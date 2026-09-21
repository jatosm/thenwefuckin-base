#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "preview.h"
#include "imgui_internal.h"
#include "core/variables/variables.h"
#include "core/functions/visual/visual.h"
#include "render/menu/library.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "../../../../ext/stb/stb_image.h"

namespace Preview {
namespace {

struct Vert {
    float x, y, z;
    float u, v;
    float cr, cg, cb;
};

constexpr int kMaxVerts = 32768;
constexpr float kPanelW = 196.0f;
constexpr float kTitleH = 22.0f;

std::vector<Vert> g_verts;
std::vector<std::uint32_t> g_idx;
std::vector<char> g_triUV;
ID3D11ShaderResourceView* g_tex = nullptr;
int g_texW = 0, g_texH = 0;
bool g_ready = false;
const char* g_fail = "not initialized";

float g_rx = 0.0f, g_ry = 0.0f, g_rw = 0.0f, g_rh = 0.0f;
float s_yawAuto = 0.0f, s_yawDrag = 0.0f, s_pitch = 0.0f, s_zoom = 1.0f;
float g_floorY = -0.55f;
std::string g_texPathOut;

std::string ExeDir() {
    char path[MAX_PATH]{};
    DWORD n = GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string s(path, path + (n ? n : 0));
    const auto p = s.find_last_of("\\/");
    if (p != std::string::npos)
        s.resize(p);
    return s;
}

bool FileExists(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return (bool)f;
}

struct Mtl {
    float r = 1.0f, g = 1.0f, b = 1.0f;
    std::string mapKd;
};

bool LoadObj(const std::string& objPath, const std::string& objDir) {
    std::ifstream f(objPath);
    if (!f) {
        g_fail = "player.obj missing";
        return false;
    }
    std::vector<float> pos, uv;
    std::unordered_map<std::string, Mtl> mtls;
    std::string curMtl;
    std::string line;
    struct RawTri {
        int v[3], t[3];
        std::string mat;
    };
    std::vector<RawTri> rawTris;
    std::string mtlFile;
    auto parseTriple = [](const std::string& tok, int& vi, int& vti) {
        vi = vti = -1;
        int a = 0, b = 0, c = 0;
        if (std::sscanf(tok.c_str(), "%d/%d/%d", &a, &b, &c) == 3) {
            vi = a - 1;
            vti = b - 1;
        } else if (std::sscanf(tok.c_str(), "%d//%d", &a, &c) == 2) {
            vi = a - 1;
        } else if (std::sscanf(tok.c_str(), "%d/%d", &a, &b) == 2) {
            vi = a - 1;
            vti = b - 1;
        } else if (std::sscanf(tok.c_str(), "%d", &a) == 1) {
            vi = a - 1;
        }
    };
    while (std::getline(f, line)) {
        if (line.compare(0, 7, "mtllib ") == 0) {
            mtlFile = line.substr(7);
            while (!mtlFile.empty() && (mtlFile.back() == '\r' || mtlFile.back() == ' ' || mtlFile.back() == '\t'))
                mtlFile.pop_back();
        } else if (line.compare(0, 2, "v ") == 0) {
            float x = 0, y = 0, z = 0;
            std::sscanf(line.c_str() + 2, "%f %f %f", &x, &y, &z);
            pos.push_back(x);
            pos.push_back(y);
            pos.push_back(z);
        } else if (line.compare(0, 3, "vt ") == 0) {
            float u = 0, v = 0;
            std::sscanf(line.c_str() + 3, "%f %f", &u, &v);
            uv.push_back(u);
            uv.push_back(v);
        } else if (line.compare(0, 7, "usemtl ") == 0) {
            curMtl = line.substr(7);
            while (!curMtl.empty() &&
                   (curMtl.back() == '\r' || curMtl.back() == ' ' || curMtl.back() == '\t'))
                curMtl.pop_back();
        } else if (line.compare(0, 2, "f ") == 0) {
            std::istringstream ss(line.substr(2));
            std::string tok;
            std::vector<int> vis, vts;
            while (ss >> tok) {
                int vi, vti;
                parseTriple(tok, vi, vti);
                if (vi < 0 || (std::size_t)vi * 3 + 2 >= pos.size())
                    continue;
                if (vti >= 0 && (std::size_t)vti * 2 + 1 >= uv.size())
                    vti = -1;
                vis.push_back(vi);
                vts.push_back(vti);
            }
            for (std::size_t i = 1; i + 1 < vis.size(); ++i) {
                RawTri rt{};
                rt.v[0] = vis[0];
                rt.v[1] = vis[i];
                rt.v[2] = vis[i + 1];
                rt.t[0] = vts[0];
                rt.t[1] = vts[i];
                rt.t[2] = vts[i + 1];
                rt.mat = curMtl;
                rawTris.push_back(rt);
            }
        }
    }
    if (rawTris.empty()) {
        g_fail = "obj parse failed";
        return false;
    }
    std::unordered_map<std::string, int> matIds;
    std::vector<Mtl> matList(1);
    if (!mtlFile.empty()) {
        std::ifstream mf(objDir + "\\" + mtlFile);
        std::string ml, cur;
        while (std::getline(mf, ml)) {
            if (ml.compare(0, 7, "newmtl ") == 0) {
                cur = ml.substr(7);
                while (!cur.empty() && (cur.back() == '\r' || cur.back() == ' ' || cur.back() == '\t'))
                    cur.pop_back();
                if (mtls.find(cur) == mtls.end())
                    mtls[cur] = Mtl{};
            } else if (ml.compare(0, 3, "Kd ") == 0 && !cur.empty()) {
                float r = 1, g = 1, b = 1;
                std::sscanf(ml.c_str() + 3, "%f %f %f", &r, &g, &b);
                mtls[cur].r = r;
                mtls[cur].g = g;
                mtls[cur].b = b;
            } else if (ml.compare(0, 7, "map_Kd ") == 0 && !cur.empty()) {
                std::string t = ml.substr(7);
                while (!t.empty() && (t.back() == '\r' || t.back() == ' ' || t.back() == '\t'))
                    t.pop_back();
                mtls[cur].mapKd = t;
            }
        }
    }
    std::string firstTex;
    for (auto& kv : mtls) {
        if (!kv.second.mapKd.empty() && firstTex.empty())
            firstTex = kv.second.mapKd;
    }
    struct Key {
        int v, t, m;
        bool operator==(const Key& o) const { return v == o.v && t == o.t && m == o.m; }
    };
    struct KeyHash {
        std::size_t operator()(const Key& k) const {
            return ((std::size_t)k.v * 73856093ull) ^ ((std::size_t)(k.t + 1) * 19349663ull) ^
                   ((std::size_t)(k.m + 1) * 83492791ull);
        }
    };
    std::unordered_map<Key, std::uint32_t, KeyHash> dedup;
    std::vector<Vert> verts;
    std::vector<std::uint32_t> idx;
    std::vector<char> triUV;
    verts.reserve(4096);
    idx.reserve(8192);
    int triNo = 0;
    for (auto& rt : rawTris) {
        ++triNo;
        (void)triNo;
        Mtl mt;
        int mi = 0;
        if (!rt.mat.empty()) {
            auto mii = matIds.find(rt.mat);
            if (mii == matIds.end()) {
                mi = (int)matIds.size() + 1;
                matIds[rt.mat] = mi;
            } else {
                mi = mii->second;
            }
            auto mit = mtls.find(rt.mat);
            if (mit != mtls.end())
                mt = mit->second;
        }
        std::uint32_t out[3]{};
        bool ok = true;
        for (int k = 0; k < 3; ++k) {
            Key kk{rt.v[k], rt.t[k], mi};
            auto it = dedup.find(kk);
            if (it != dedup.end()) {
                out[k] = it->second;
                continue;
            }
            if ((int)verts.size() >= kMaxVerts) {
                ok = false;
                break;
            }
            Vert w{};
            w.x = pos[(std::size_t)rt.v[k] * 3 + 0];
            w.y = pos[(std::size_t)rt.v[k] * 3 + 1];
            w.z = pos[(std::size_t)rt.v[k] * 3 + 2];
            if (rt.t[k] >= 0) {
                w.u = uv[(std::size_t)rt.t[k] * 2 + 0];
                w.v = 1.0f - uv[(std::size_t)rt.t[k] * 2 + 1];
            }
            w.cr = mt.r;
            w.cg = mt.g;
            w.cb = mt.b;
            out[k] = (std::uint32_t)verts.size();
            verts.push_back(w);
            dedup[kk] = out[k];
        }
        if (!ok)
            continue;
        idx.push_back(out[0]);
        idx.push_back(out[1]);
        idx.push_back(out[2]);
        triUV.push_back((rt.t[0] >= 0 && rt.t[1] >= 0 && rt.t[2] >= 0) ? 1 : 0);
    }
    if (verts.empty() || idx.empty()) {
        g_fail = "obj parse failed";
        return false;
    }
    float mnx = 1e9f, mny = 1e9f, mnz = 1e9f, mxx = -1e9f, mxy = -1e9f, mxz = -1e9f;
    for (auto& w : verts) {
        if (w.x < mnx) mnx = w.x;
        if (w.y < mny) mny = w.y;
        if (w.z < mnz) mnz = w.z;
        if (w.x > mxx) mxx = w.x;
        if (w.y > mxy) mxy = w.y;
        if (w.z > mxz) mxz = w.z;
    }
    const float cx = (mnx + mxx) * 0.5f, cy = (mny + mxy) * 0.5f, cz = (mnz + mxz) * 0.5f;
    const float ext = (std::max)((mxx - mnx), (std::max)((mxy - mny), (mxz - mnz)));
    if (ext < 1e-6f) {
        g_fail = "obj parse failed";
        return false;
    }
    const float s = 1.15f / ext;
    float floor_y = 1e9f;
    for (auto& w : verts) {
        w.x = (w.x - cx) * s;
        w.y = (w.y - cy) * s;
        w.z = (w.z - cz) * s;
        if (w.y < floor_y)
            floor_y = w.y;
    }
    g_floorY = floor_y;
    g_verts = std::move(verts);
    g_idx = std::move(idx);
    g_triUV = std::move(triUV);
    if (!firstTex.empty())
        g_texPathOut = objDir + "\\" + firstTex;
    return true;
}

bool LoadTexture(ID3D11Device* dev, const std::string& texPath) {
    std::ifstream tf(texPath, std::ios::binary);
    if (!tf) {
        g_fail = "texture file missing";
        return false;
    }
    tf.seekg(0, std::ios::end);
    const std::size_t tlen = (std::size_t)tf.tellg();
    tf.seekg(0, std::ios::beg);
    if (tlen == 0 || tlen > 32 * 1024 * 1024) {
        g_fail = "texture decode failed";
        return false;
    }
    std::vector<unsigned char> tbuf(tlen);
    tf.read((char*)tbuf.data(), tlen);
    tf.close();
    int tw = 0, th = 0, tc = 0;
    unsigned char* img = stbi_load_from_memory(tbuf.data(), (int)tbuf.size(), &tw, &th, &tc, 4);
    if (!img || tw <= 0 || th <= 0) {
        g_fail = "texture decode failed";
        return false;
    }
    D3D11_TEXTURE2D_DESC td{};
    td.Width = (UINT)tw;
    td.Height = (UINT)th;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA sub{};
    sub.pSysMem = img;
    sub.SysMemPitch = (UINT)(tw * 4);
    ID3D11Texture2D* tex = nullptr;
    if (FAILED(dev->CreateTexture2D(&td, &sub, &tex)) || !tex) {
        stbi_image_free(img);
        g_fail = "gpu init failed";
        return false;
    }
    stbi_image_free(img);
    D3D11_SHADER_RESOURCE_VIEW_DESC svd{};
    svd.Format = td.Format;
    svd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    svd.Texture2D.MipLevels = 1;
    if (FAILED(dev->CreateShaderResourceView(tex, &svd, &g_tex)) || !g_tex) {
        tex->Release();
        g_fail = "gpu init failed";
        return false;
    }
    tex->Release();
    g_texW = tw;
    g_texH = th;
    return true;
}

}

bool Init(ID3D11Device* dev) {
    if (g_ready || !dev)
        return g_ready;
    std::string objPath;
    std::string objDir;
    {
        const std::string exeObj = ExeDir() + "\\player.obj";
        const std::string deskObj = "C:\\Users\\admin\\Desktop\\Ny mappe (2)\\player.obj";
        if (FileExists(exeObj)) {
            objPath = exeObj;
            objDir = ExeDir();
        } else if (FileExists(deskObj)) {
            objPath = deskObj;
            objDir = "C:\\Users\\admin\\Desktop\\Ny mappe (2)";
        } else {
            g_fail = "player.obj missing";
            return false;
        }
    }
    g_texPathOut.clear();
    if (!LoadObj(objPath, objDir)) {
        return false;
    }
    if (g_texPathOut.empty() || !LoadTexture(dev, g_texPathOut)) {
        g_fail = "texture fallback";
    }
    g_ready = true;
    g_fail = "ok";
    return true;
}

void Shutdown() {
    if (g_tex) {
        g_tex->Release();
        g_tex = nullptr;
    }
    g_verts.clear();
    g_verts.shrink_to_fit();
    g_idx.clear();
    g_idx.shrink_to_fit();
    g_triUV.clear();
    g_triUV.shrink_to_fit();
    g_texW = g_texH = 0;
    g_ready = false;
}

bool Ready() {
    return g_ready && !g_verts.empty() && !g_idx.empty();
}

void DrawPanel() {
    float dt = ImGui::GetIO().DeltaTime;
    if (dt <= 0.0f || dt > 0.1f)
        dt = 0.016f;
    static float previewFade = 0.0f;
    if (variables::menuOpen && variables::Aimbot::playerPreview) {
        previewFade += dt * 8.0f;
        if (previewFade > 1.0f)
            previewFade = 1.0f;
    } else {
        previewFade -= dt * 8.0f;
        if (previewFade < 0.0f)
            previewFade = 0.0f;
    }
    if (previewFade <= 0.001f)
        return;

    static float ax = 690.0f, ay = 80.0f, ah = 390.0f;
    static float sw = 0.0f;
    if (sw <= 0.0f)
        sw = (float)GetSystemMetrics(SM_CXSCREEN);
    if (ImGuiWindow* mw = ImGui::FindWindowByName("jatos")) {
        if (!mw->Collapsed) {
            const float nx = std::floor(mw->Pos.x) + std::floor(mw->Size.x) + 8.0f;
            if (nx + kPanelW <= sw - 8.0f) {
                ax = nx;
            } else {
                ax = std::floor(mw->Pos.x) - kPanelW - 8.0f;
            }
            ay = std::floor(mw->Pos.y);
            if (mw->Size.y > 100.0f)
                ah = std::floor(mw->Size.y);
        }
    }
    const float x = std::floor(ax);
    const float y = std::floor(ay);
    const float h = std::floor(ah);

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, previewFade);
    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kPanelW, h), ImGuiCond_Always);
    if (!ImGui::Begin("##player_preview", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove)) {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    const imGuiCustom::Theme& theme = imGuiCustom::GetTheme();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 pmin = ImVec2(std::floor(ImGui::GetWindowPos().x), std::floor(ImGui::GetWindowPos().y));
    const ImVec2 pmax = pmin + ImVec2(kPanelW, h);

    dl->AddRectFilled(pmin, pmax, imGuiCustom::ColorU32(theme.WindowBg), 0.0f);
    dl->AddRect(pmin, pmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    dl->AddRect(pmin + ImVec2(1.0f, 1.0f), pmax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
    dl->AddRectFilled(pmin, pmin + ImVec2(kPanelW, 1.5f), imGuiCustom::ColorU32(ImVec4(0.5373f, 0.7647f, 0.7490f, 1.0f)));

    const imGuiCustom::Fonts& fonts = imGuiCustom::GetFonts();
    ImFont* title_font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
    const float title_fs = 12.0f * imGuiCustom::g_fontScale;
    const ImVec2 title_sz = title_font->CalcTextSizeA(title_fs, FLT_MAX, 0.0f, "player preview");
    dl->AddText(title_font, title_fs, pmin + ImVec2(std::floor((kPanelW - title_sz.x) * 0.5f), 3.5f),
                imGuiCustom::ColorU32(theme.TextBright), "player preview");

    const ImVec2 rmin(std::floor(pmin.x + 6.0f), std::floor(pmin.y + 22.0f));
    const ImVec2 rmax(std::floor(pmin.x + kPanelW - 6.0f), std::floor(pmin.y + h - 10.0f));
    g_rx = rmin.x;
    g_ry = rmin.y;
    g_rw = rmax.x - rmin.x;
    g_rh = rmax.y - rmin.y;
    if (g_rh < 40.0f)
        g_rh = 40.0f;

    if (!g_ready) {
        dl->AddText(title_font, 12.0f * imGuiCustom::g_fontScale, ImVec2(std::floor(pmin.x + 8.0f), std::floor(g_ry + 8.0f)),
                    IM_COL32(255, 90, 90, 255), g_fail);
        g_rw = 0.0f;
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    dl->AddRectFilled(rmin, rmax, imGuiCustom::ColorU32(theme.CardBg), 0.0f);
    dl->AddRect(rmin, rmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    dl->AddRect(rmin + ImVec2(1.0f, 1.0f), rmax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);

    const bool menuOpen = variables::menuOpen;
    const ImVec2 mp = ImGui::GetIO().MousePos;
    const bool isHovered = menuOpen && (mp.x >= rmin.x && mp.x <= rmax.x && mp.y >= rmin.y && mp.y <= rmax.y);
    static bool s_isDragging = false;
    if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        s_isDragging = true;
    }
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        s_isDragging = false;
    }
    if (s_isDragging) {
        s_yawDrag += ImGui::GetIO().MouseDelta.x * 0.015f;
        s_pitch += -ImGui::GetIO().MouseDelta.y * 0.015f;
        if (s_pitch < -1.0f)
            s_pitch = -1.0f;
        if (s_pitch > 1.0f)
            s_pitch = 1.0f;
    }
    if (isHovered) {
        if (ImGui::GetIO().MouseWheel != 0.0f) {
            s_zoom += ImGui::GetIO().MouseWheel * 0.10f;
            if (s_zoom < 0.4f)
                s_zoom = 0.4f;
            if (s_zoom > 2.5f)
                s_zoom = 2.5f;
        }
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            s_yawDrag = 0.0f;
            s_pitch = 0.0f;
            s_zoom = 1.0f;
        }
    }
    const float yaw = s_yawAuto + s_yawDrag;
    const float ca = cosf(yaw), sa = sinf(yaw);
    const float cp = cosf(s_pitch), sp = sinf(s_pitch);
    const float sw2 = g_rw, sh2 = g_rh;
    float ext = 0.0f;
    for (auto& w : g_verts) {
        const float dx = w.x >= 0 ? w.x : -w.x;
        const float dy = w.y >= 0 ? w.y : -w.y;
        const float dz = w.z >= 0 ? w.z : -w.z;
        const float m = dx > dy ? (dx > dz ? dx : dz) : (dy > dz ? dy : dz);
        if (m > ext)
            ext = m;
    }
    if (ext < 1e-6f)
        ext = 1.0f;
    const float scale = (std::min)(sw2, sh2) * 0.44f / ext * s_zoom;
    const float cx = rmin.x + sw2 * 0.5f;
    const float cy = rmin.y + sh2 * 0.47f;

    const ImVec2 clipMin = rmin + ImVec2(2.0f, 2.0f);
    const ImVec2 clipMax = rmax - ImVec2(2.0f, 2.0f);
    dl->PushClipRect(clipMin, clipMax, true);

    struct PT {
        float x, y, d;
    };
    static std::vector<PT> proj;
    proj.clear();
    if (proj.capacity() < g_verts.size())
        proj.reserve(g_verts.size());
    for (auto& w : g_verts) {
        const float rx = w.x * ca + w.z * sa;
        const float rz = -w.x * sa + w.z * ca;
        const float ry = w.y * cp + rz * sp;
        const float rz2 = -w.y * sp + rz * cp;
        proj.push_back({cx + rx * scale, cy - ry * scale, rz2});
    }

    struct RT {
        float d;
        std::uint32_t t;
    };
    static std::vector<RT> order;
    order.clear();
    const std::size_t ntris = g_idx.size() / 3;
    if (order.capacity() < ntris)
        order.reserve(ntris);
    for (std::size_t ti = 0; ti < ntris; ++ti) {
        const std::uint32_t i0 = g_idx[ti * 3 + 0];
        const std::uint32_t i1 = g_idx[ti * 3 + 1];
        const std::uint32_t i2 = g_idx[ti * 3 + 2];
        if (i0 >= proj.size() || i1 >= proj.size() || i2 >= proj.size())
            continue;
        const PT& a = proj[i0];
        const PT& b = proj[i1];
        const PT& c = proj[i2];
        const float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        if (area > -0.01f && area < 0.01f)
            continue;
        order.push_back({(a.d + b.d + c.d) * 0.333333f, (std::uint32_t)ti});
    }

    std::sort(order.begin(), order.end(), [](const RT& a, const RT& b) { return a.d > b.d; });

    const bool useChams = variables::ESP::meshChams;
    const bool hasTex = (g_tex != nullptr) && !useChams;
    if (hasTex)
        dl->PushTexture((ImTextureID)g_tex);
    const ImVec2 whiteUv = ImGui::GetFontTexUvWhitePixel();

    for (auto& r : order) {
        const std::uint32_t i0 = g_idx[(std::size_t)r.t * 3 + 0];
        const std::uint32_t i1 = g_idx[(std::size_t)r.t * 3 + 1];
        const std::uint32_t i2 = g_idx[(std::size_t)r.t * 3 + 2];
        const Vert& v0 = g_verts[i0];
        const Vert& v1 = g_verts[i1];
        const Vert& v2 = g_verts[i2];
        const PT& a = proj[i0];
        const PT& b = proj[i1];
        const PT& c = proj[i2];

        const float nx = (v1.y - v0.y) * (v2.z - v0.z) - (v1.z - v0.z) * (v2.y - v0.y);
        const float ny = (v1.z - v0.z) * (v2.x - v0.x) - (v1.x - v0.x) * (v2.z - v0.z);
        const float nz = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
        float nl = std::sqrt(nx * nx + ny * ny + nz * nz);
        float lambert = 0.85f;
        if (nl > 1e-9f) {
            const float lx = nx / nl, ly = ny / nl, lz = nz / nl;
            const float lrx = lx * ca + lz * sa;
            const float lrz = -lx * sa + lz * ca;
            const float lry = ly * cp + lrz * sp;
            const float lrz2 = -ly * sp + lrz * cp;
            float ndl = lrx * 0.25f + lry * 0.45f + (-lrz2) * 0.75f;
            if (ndl < 0.0f)
                ndl = -ndl * 0.35f;
            lambert = 0.65f + 0.35f * ndl;
        }

        const bool triHasUV = (g_triUV[r.t] != 0 && hasTex);
        ImVec2 uv0 = whiteUv, uv1 = whiteUv, uv2 = whiteUv;
        ImU32 triCol0, triCol1, triCol2;

        if (useChams) {
            const ImVec4& cc = variables::ESP::chamsFillColor;
            const float cr = (std::min)(cc.x * lambert, 1.0f);
            const float cg = (std::min)(cc.y * lambert, 1.0f);
            const float cb = (std::min)(cc.z * lambert, 1.0f);
            const int caVal = (int)(cc.w * 255.0f);
            triCol0 = triCol1 = triCol2 = IM_COL32((int)(cr * 255.0f), (int)(cg * 255.0f), (int)(cb * 255.0f), caVal > 0 ? caVal : 255);
        } else if (triHasUV) {
            uv0 = ImVec2(v0.u, v0.v);
            uv1 = ImVec2(v1.u, v1.v);
            uv2 = ImVec2(v2.u, v2.v);
            const int lVal = (int)(std::clamp(lambert, 0.0f, 1.0f) * 255.0f);
            triCol0 = triCol1 = triCol2 = IM_COL32(lVal, lVal, lVal, 255);
        } else {
            const float lr = (std::min)((v0.cr + v1.cr + v2.cr) * 0.333333f * lambert + 0.08f, 1.0f);
            const float lg = (std::min)((v0.cg + v1.cg + v2.cg) * 0.333333f * lambert + 0.08f, 1.0f);
            const float lb = (std::min)((v0.cb + v1.cb + v2.cb) * 0.333333f * lambert + 0.08f, 1.0f);
            triCol0 = triCol1 = triCol2 = IM_COL32((int)(lr * 255.0f), (int)(lg * 255.0f), (int)(lb * 255.0f), 255);
        }

        dl->PrimReserve(3, 3);
        dl->PrimVtx(ImVec2(a.x, a.y), uv0, triCol0);
        dl->PrimVtx(ImVec2(b.x, b.y), uv1, triCol1);
        dl->PrimVtx(ImVec2(c.x, c.y), uv2, triCol2);
    }

    if (hasTex)
        dl->PopTexture();

    float bMinX = 1e9f, bMaxX = -1e9f, bMinY = 1e9f, bMaxY = -1e9f;
    for (const auto& pt : proj) {
        if (pt.x < bMinX) bMinX = pt.x;
        if (pt.x > bMaxX) bMaxX = pt.x;
        if (pt.y < bMinY) bMinY = pt.y;
        if (pt.y > bMaxY) bMaxY = pt.y;
    }
    const float pad = 4.0f;
    const float bx0 = bMinX - pad;
    const float bx1 = bMaxX + pad;
    const float by0 = bMinY - pad;
    const float by1 = bMaxY + pad;

    if (variables::ESP::boxes) {
        Visuals::DrawBoxFill(dl, bx0, by0, bx1, by1);
        if (variables::ESP::boxMode == 0) {
            Visuals::DrawBoxCol(dl, bx0, by0, bx1, by1, imGuiCustom::ColorU32(variables::ESP::boxColor));
        } else {
            const float cw = (bx1 - bx0) * 0.25f;
            const float ch = (by1 - by0) * 0.20f;
            const ImU32 col = imGuiCustom::ColorU32(variables::ESP::boxColor);
            auto drawCorner = [&](ImVec2 p1, ImVec2 corner, ImVec2 p2) {
                dl->AddLine(p1, corner, IM_COL32(0, 0, 0, 255), 3.0f);
                dl->AddLine(corner, p2, IM_COL32(0, 0, 0, 255), 3.0f);
                dl->AddLine(p1, corner, col, 1.5f);
                dl->AddLine(corner, p2, col, 1.5f);
            };
            drawCorner(ImVec2(bx0, by0 + ch), ImVec2(bx0, by0), ImVec2(bx0 + cw, by0));
            drawCorner(ImVec2(bx1 - cw, by0), ImVec2(bx1, by0), ImVec2(bx1, by0 + ch));
            drawCorner(ImVec2(bx0, by1 - ch), ImVec2(bx0, by1), ImVec2(bx0 + cw, by1));
            drawCorner(ImVec2(bx1 - cw, by1), ImVec2(bx1, by1), ImVec2(bx1, by1 - ch));
        }
    }

    if (variables::ESP::healthBar) {
        const float hx0 = std::floor(bx0 - 5.0f);
        const float hx1 = hx0 + 2.0f;
        Visuals::DrawHealthBar(dl, hx0, hx1, by0, by1, 1.0f, imGuiCustom::ColorU32(variables::ESP::healthColor));
    }

    if (variables::ESP::skeleton) {
        struct Joint { float x, y, z; };
        const Joint jHead   = {  0.00f,  0.38f,  0.00f };
        const Joint jNeck   = {  0.00f,  0.22f,  0.00f };
        const Joint jPelvis = {  0.00f, -0.05f,  0.00f };
        const Joint jLSh    = { -0.22f,  0.20f,  0.00f };
        const Joint jLElb   = { -0.32f,  0.08f,  0.00f };
        const Joint jLWrist = { -0.34f, -0.06f,  0.00f };
        const Joint jRSh    = {  0.22f,  0.20f,  0.00f };
        const Joint jRElb   = {  0.32f,  0.08f,  0.00f };
        const Joint jRWrist = {  0.34f, -0.06f,  0.00f };
        const Joint jLHip   = { -0.11f, -0.05f,  0.00f };
        const Joint jLKnee  = { -0.11f, -0.28f,  0.00f };
        const Joint jLAnkle = { -0.11f, -0.52f,  0.00f };
        const Joint jRHip   = {  0.11f, -0.05f,  0.00f };
        const Joint jRKnee  = {  0.11f, -0.28f,  0.00f };
        const Joint jRAnkle = {  0.11f, -0.52f,  0.00f };

        auto projJoint = [&](const Joint& j) -> ImVec2 {
            const float rx = j.x * ca + j.z * sa;
            const float rz = -j.x * sa + j.z * ca;
            const float ry = j.y * cp + rz * sp;
            return ImVec2(cx + rx * scale, cy - ry * scale);
        };

        auto drawBone = [&](const Joint& j1, const Joint& j2) {
            ImVec2 p1 = projJoint(j1);
            ImVec2 p2 = projJoint(j2);
            if (variables::ESP::skeletonOutline)
                dl->AddLine(p1, p2, IM_COL32(0, 0, 0, 255), variables::ESP::skeletonThickness + 1.0f);
            dl->AddLine(p1, p2, imGuiCustom::ColorU32(variables::ESP::skeletonColor), variables::ESP::skeletonThickness);
        };

        drawBone(jHead, jNeck);
        drawBone(jNeck, jPelvis);
        drawBone(jNeck, jLSh);
        drawBone(jLSh, jLElb);
        drawBone(jLElb, jLWrist);
        drawBone(jNeck, jRSh);
        drawBone(jRSh, jRElb);
        drawBone(jRElb, jRWrist);
        drawBone(jPelvis, jLHip);
        drawBone(jLHip, jLKnee);
        drawBone(jLKnee, jLAnkle);
        drawBone(jPelvis, jRHip);
        drawBone(jRHip, jRKnee);
        drawBone(jRKnee, jRAnkle);
    }

    if (variables::ESP::headDot) {
        const float rx = 0.00f * ca + 0.00f * sa;
        const float rz = -0.00f * sa + 0.00f * ca;
        const float ry = 0.38f * cp + rz * sp;
        const ImVec2 hp = ImVec2(cx + rx * scale, cy - ry * scale);
        const float r = (std::max)(2.0f, variables::ESP::headDotSize);
        dl->AddCircleFilled(hp, r, imGuiCustom::ColorU32(variables::ESP::headDotColor), 16);
        dl->AddCircle(hp, r + 1.0f, IM_COL32(0, 0, 0, 200), 16, 1.0f);
    }

    if (variables::ESP::viewDirection) {
        const float rx = 0.00f * ca + 0.00f * sa;
        const float rz = -0.00f * sa + 0.00f * ca;
        const float ry = 0.38f * cp + rz * sp;
        const ImVec2 hp = ImVec2(cx + rx * scale, cy - ry * scale);

        const float lookX = 0.0f, lookY = 0.0f, lookZ = -0.45f;
        const float lrx = lookX * ca + lookZ * sa;
        const float lrz = -lookX * sa + lookZ * ca;
        const float lry = lookY * cp + lrz * sp;
        const ImVec2 endP = ImVec2(hp.x + lrx * scale, hp.y - lry * scale);

        const ImU32 vdCol = imGuiCustom::ColorU32(variables::ESP::viewDirColor);
        dl->AddLine(hp, endP, IM_COL32(0, 0, 0, 255), 3.0f);
        dl->AddLine(hp, endP, vdCol, 2.0f);
        dl->AddCircleFilled(endP, 2.5f, vdCol, 8);
    }

    if (variables::ESP::names) {
        ImFont* espF = Visuals::EspFont();
        const float espSz = variables::Misc::espFontSize;
        const ImVec2 ts = espF->CalcTextSizeA(espSz, FLT_MAX, 0.0f, "Player");
        Visuals::DrawOutlinedText(dl, ImVec2((bx0 + bx1) * 0.5f - ts.x * 0.5f, by0 - ts.y - 2.0f), "Player", imGuiCustom::ColorU32(variables::ESP::nameColor));
    }

    if (variables::ESP::distance) {
        ImFont* espF = Visuals::EspFont();
        const float espSz = variables::Misc::espFontSize;
        const ImVec2 ts = espF->CalcTextSizeA(espSz, FLT_MAX, 0.0f, "15m");
        Visuals::DrawOutlinedText(dl, ImVec2((bx0 + bx1) * 0.5f - ts.x * 0.5f, by1 + 2.0f), "15m", imGuiCustom::ColorU32(variables::ESP::distanceColor));
    }

    if (variables::ESP::tool) {
        ImFont* espF = Visuals::EspFont();
        const float espSz = variables::Misc::espFontSize;
        const ImVec2 ts = espF->CalcTextSizeA(espSz, FLT_MAX, 0.0f, "Sword");
        float ty = by1 + 2.0f;
        if (variables::ESP::distance)
            ty += espSz + 3.0f;
        Visuals::DrawOutlinedText(dl, ImVec2((bx0 + bx1) * 0.5f - ts.x * 0.5f, ty), "Sword", imGuiCustom::ColorU32(variables::ESP::toolColor));
    }

    if (variables::ESP::flags) {
        const ImU32 flagsCol = imGuiCustom::ColorU32(variables::ESP::flagsColor);
        const float espSz = variables::Misc::espFontSize;
        float fx = bx1 + 4.0f;
        float fy = by0;
        auto flagText = [&](const std::string& t) {
            Visuals::DrawOutlinedText(dl, ImVec2(fx, fy), t, flagsCol);
            fy += espSz + 2.0f;
        };
        if (variables::ESP::flagSel[0]) flagText("Idle");
        if (variables::ESP::flagSel[1]) flagText("R6");
        if (variables::ESP::flagSel[2]) flagText("100% HP");
        if (variables::ESP::flagSel[3] && variables::ESP::tool) flagText("Sword");
        if (variables::ESP::flagSel[4]) flagText("15m");
    }

    dl->PopClipRect();

    dl->AddRect(rmin, rmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    dl->AddRect(rmin + ImVec2(1.0f, 1.0f), rmax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);

    dl->AddRectFilled(pmin, pmin + ImVec2(kPanelW, 1.5f), imGuiCustom::ColorU32(ImVec4(0.5373f, 0.7647f, 0.7490f, 1.0f)));

    ImGui::End();
    ImGui::PopStyleVar();
}

}
