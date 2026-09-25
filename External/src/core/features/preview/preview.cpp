// discord.gg/thenwefuckin
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "preview.h"
#include "imgui_internal.h"
#include "core/variables/variables.h"
#include "core/functions/visual/visual.h"
#include "core/globals/globals.h"
#include "core/functions/players/players.h"
#include "memory/memory.h"
#include "sdk/offsets.h"
#include "render/menu/library.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <windows.h>
#include <winhttp.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "../../../../ext/stb/stb_image.h"

#pragma comment(lib, "winhttp.lib")

namespace Preview {
namespace {

constexpr float kPanelW = 196.0f;

ID3D11Device* g_dev = nullptr;

std::mutex g_logMtx;
namespace {
constexpr WORD LC_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
constexpr WORD LC_DIM = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
constexpr WORD LC_CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
constexpr const char* LC_INDENT = "       ";
}
void PvLog(const std::string& msg) {
    (void)msg;
}

struct Thumb {
    ID3D11ShaderResourceView* tex = nullptr;
    int w = 0, h = 0;
    bool loading = false;
    float cu0 = 0.0f, cv0 = 0.0f, cu1 = 1.0f, cv1 = 1.0f;
};
std::unordered_map<long long, Thumb> g_thumbs;
std::mutex g_thumbMtx;
std::unordered_set<long long> g_pending;

void HttpFail(const std::wstring& host, const std::wstring& path, const char* method, const char* stage) {
    const DWORD err = GetLastError();
    std::string h;
    h.reserve(host.size());
    for (wchar_t c : host)
        h.push_back((char)c);
    std::string p;
    p.reserve(path.size());
    for (wchar_t c : path)
        p.push_back((char)c);
    if (p.size() > 80)
        p = p.substr(0, 80) + "...";
    char b[320];
    snprintf(b, sizeof(b), "http %s %s%s -> %s err=%lu", method, h.c_str(), p.c_str(), stage, (unsigned long)err);
    PvLog(b);
}

std::string HttpGet(const std::wstring& host, const std::wstring& path, const std::wstring& extraHeaders = L"") {
    std::string out;
    HINTERNET hS = WinHttpOpen(L"jatos/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
    if (!hS) {
        HttpFail(host, path, "GET", "WinHttpOpen");
        return out;
    }
    HINTERNET hC = WinHttpConnect(hS, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hC) {
        HttpFail(host, path, "GET", "WinHttpConnect");
        WinHttpCloseHandle(hS);
        return out;
    }
    HINTERNET hR = WinHttpOpenRequest(hC, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hR) {
        HttpFail(host, path, "GET", "WinHttpOpenRequest");
        WinHttpCloseHandle(hC);
        WinHttpCloseHandle(hS);
        return out;
    }
    LPCWSTR hdrs = extraHeaders.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : extraHeaders.c_str();
    DWORD hdrLen = extraHeaders.empty() ? 0 : (DWORD)extraHeaders.size();
    if (!WinHttpSendRequest(hR, hdrs, hdrLen, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        HttpFail(host, path, "GET", "WinHttpSendRequest");
        WinHttpCloseHandle(hR);
        WinHttpCloseHandle(hC);
        WinHttpCloseHandle(hS);
        return out;
    }
    if (!WinHttpReceiveResponse(hR, nullptr)) {
        HttpFail(host, path, "GET", "WinHttpReceiveResponse");
        WinHttpCloseHandle(hR);
        WinHttpCloseHandle(hC);
        WinHttpCloseHandle(hS);
        return out;
    }
    std::vector<char> buf;
    for (;;) {
        DWORD avail = 0;
        if (!WinHttpQueryDataAvailable(hR, &avail))
            break;
        if (!avail)
            break;
        const size_t old = buf.size();
        buf.resize(old + avail);
        DWORD got = 0;
        if (!WinHttpReadData(hR, buf.data() + old, avail, &got))
            break;
        buf.resize(old + got);
        if (got == 0)
            break;
    }
    out.assign(buf.begin(), buf.end());
    WinHttpCloseHandle(hR);
    WinHttpCloseHandle(hC);
    WinHttpCloseHandle(hS);
    return out;
}

std::string HttpPost(const std::wstring& host, const std::wstring& path, const std::string& body) {
    std::string out;
    HINTERNET hS = WinHttpOpen(L"jatos/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
    if (!hS) {
        HttpFail(host, path, "POST", "WinHttpOpen");
        return out;
    }
    HINTERNET hC = WinHttpConnect(hS, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hC) {
        HttpFail(host, path, "POST", "WinHttpConnect");
        WinHttpCloseHandle(hS);
        return out;
    }
    HINTERNET hR = WinHttpOpenRequest(hC, L"POST", path.c_str(), nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hR) {
        HttpFail(host, path, "POST", "WinHttpOpenRequest");
        WinHttpCloseHandle(hC);
        WinHttpCloseHandle(hS);
        return out;
    }
    static const wchar_t* kCt = L"Content-Type: application/json\r\n";
    SIZE_T blen = body.size();
    const DWORD blen32 = (DWORD)blen;
    if (!WinHttpSendRequest(hR, kCt, (DWORD)wcslen(kCt), (LPVOID)body.data(), blen32, blen32, 0)) {
        HttpFail(host, path, "POST", "WinHttpSendRequest");
        WinHttpCloseHandle(hR);
        WinHttpCloseHandle(hC);
        WinHttpCloseHandle(hS);
        return out;
    }
    if (!WinHttpReceiveResponse(hR, nullptr)) {
        HttpFail(host, path, "POST", "WinHttpReceiveResponse");
        WinHttpCloseHandle(hR);
        WinHttpCloseHandle(hC);
        WinHttpCloseHandle(hS);
        return out;
    }
    std::vector<char> buf;
    for (;;) {
        DWORD avail = 0;
        if (!WinHttpQueryDataAvailable(hR, &avail))
            break;
        if (!avail)
            break;
        const size_t old = buf.size();
        buf.resize(old + avail);
        DWORD got = 0;
        if (!WinHttpReadData(hR, buf.data() + old, avail, &got))
            break;
        buf.resize(old + got);
        if (got == 0)
            break;
    }
    out.assign(buf.begin(), buf.end());
    WinHttpCloseHandle(hR);
    WinHttpCloseHandle(hC);
    WinHttpCloseHandle(hS);
    return out;
}

std::string g_apiKey;

void LoadApiKey() {
    if (!g_apiKey.empty())
        return;
    char exe[MAX_PATH]{};
    DWORD n = GetModuleFileNameA(nullptr, exe, MAX_PATH);
    std::string dir(exe, exe + (n ? n : 0));
    const auto p = dir.find_last_of("\\/");
    if (p != std::string::npos)
        dir.resize(p);
    std::ifstream f(dir + "\\avatar3d_key.txt", std::ios::binary);
    if (!f)
        return;
    std::string key((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    while (!key.empty() && (key.back() == '\r' || key.back() == '\n' || key.back() == ' ' || key.back() == '\t'))
        key.pop_back();
    while (!key.empty() && (key.front() == ' ' || key.front() == '\t' || key.front() == '\r' || key.front() == '\n'))
        key.erase(key.begin());
    if (!key.empty())
        g_apiKey = key;
}

std::wstring ApiKeyHeader() {
    if (g_apiKey.empty())
        return L"";
    std::wstring w(g_apiKey.begin(), g_apiKey.end());
    return L"x-api-key: " + w + L"\r\n";
}

std::unordered_map<std::string, long long> g_nameUid;
std::unordered_set<std::string> g_namePending;
std::unordered_set<std::string> g_nameTried;

void RequestUidByName(const std::string& name) {
    if (name.empty())
        return;
    {
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        if (g_nameUid.find(name) != g_nameUid.end() || g_namePending.find(name) != g_namePending.end() ||
            g_nameTried.find(name) != g_nameTried.end())
            return;
        g_namePending.insert(name);
        g_nameTried.insert(name);
    }
    std::thread([name] {
        long long uid = 0;
        const std::string resp =
            HttpPost(L"users.roblox.com", L"/v1/usernames/users",
                "{\"usernames\":[\"" + name + "\"],\"excludeBannedUsers\":false}");
        if (const auto p = resp.find("\"id\""); p != std::string::npos) {
            if (const auto c = resp.find(':', p); c != std::string::npos)
                uid = std::strtoll(resp.c_str() + c + 1, nullptr, 10);
        }
        {
            char b[256];
            std::string head = resp.substr(0, 120);
            snprintf(b, sizeof(b), "username lookup name='%s' respBytes=%llu parsedUid=%lld resp=%.120s",
                name.c_str(), (unsigned long long)resp.size(), uid, head.c_str());
            PvLog(b);
        }
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        if (uid > 0)
            g_nameUid[name] = uid;
        g_namePending.erase(name);
    }).detach();
}

std::string ExtractImageUrl(const std::string& json) {
    const auto p = json.find("imageUrl");
    if (p == std::string::npos)
        return {};
    const auto q = json.find("http", p);
    if (q == std::string::npos)
        return {};
    auto e = json.find('"', q);
    if (e == std::string::npos)
        e = json.size();
    std::string url = json.substr(q, e - q);
    size_t pos = url.find("\\u0026");
    while (pos != std::string::npos) {
        url.replace(pos, 6, "&");
        pos = url.find("\\u0026", pos + 1);
    }
    return url;
}

void RequestThumb(long long uid) {
    if (uid <= 0)
        return;
    {
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        if (g_thumbs.find(uid) != g_thumbs.end() || g_pending.find(uid) != g_pending.end())
            return;
        g_pending.insert(uid);
        g_thumbs[uid] = {nullptr, 0, 0, true};
    }
    {
        char b[128];
        snprintf(b, sizeof(b), "thumb request uid=%lld", uid);
        PvLog(b);
    }
    std::thread([uid] {
        const std::string json = HttpGet(L"thumbnails.roblox.com",
            L"/v1/users/avatar?userIds=" + std::to_wstring(uid) +
                L"&size=720x720&format=Png&isCircular=false");
        const std::string imgUrl = ExtractImageUrl(json);
        auto fail = [&](const char* stage, unsigned long long imgBytes) {
            char b[192];
            snprintf(b, sizeof(b), "thumb FAIL uid=%lld stage=%s jsonBytes=%llu imgBytes=%llu",
                uid, stage, (unsigned long long)json.size(), imgBytes);
            PvLog(b);
            std::lock_guard<std::mutex> lk(g_thumbMtx);
            g_pending.erase(uid);
            if (auto it = g_thumbs.find(uid); it != g_thumbs.end())
                it->second.loading = false;
        };
        if (imgUrl.empty()) {
            fail("no-imageUrl-in-json", 0);
            return;
        }
        std::string host, path;
        {
            std::string tmp = imgUrl;
            if (tmp.rfind("https://", 0) == 0)
                tmp = tmp.substr(8);
            else if (tmp.rfind("http://", 0) == 0)
                tmp = tmp.substr(7);
            const auto sl = tmp.find('/');
            if (sl != std::string::npos) {
                host = tmp.substr(0, sl);
                path = tmp.substr(sl);
            } else {
                host = tmp;
                path = "/";
            }
        }
        const std::wstring whost(host.begin(), host.end()), wpath(path.begin(), path.end());
        const std::string imgData = HttpGet(whost, wpath);
        if (imgData.empty()) {
            fail("image-download-empty", 0);
            return;
        }
        int w = 0, h = 0, ch = 0;
        unsigned char* pix = stbi_load_from_memory(
            (const unsigned char*)imgData.data(), (int)imgData.size(), &w, &h, &ch, 4);
        if (!pix || !g_dev) {
            if (pix)
                stbi_image_free(pix);
            fail(!pix ? "png-decode-failed" : "no-d3d-device", (unsigned long long)imgData.size());
            return;
        }
        float cu0 = 0.0f, cv0 = 0.0f, cu1 = 1.0f, cv1 = 1.0f;
        {
            int x0 = w, y0 = h, x1 = -1, y1 = -1;
            for (int yy = 0; yy < h; ++yy) {
                const unsigned char* row = pix + (size_t)yy * w * 4;
                for (int xx = 0; xx < w; ++xx) {
                    if (row[(size_t)xx * 4 + 3] > 8) {
                        if (xx < x0)
                            x0 = xx;
                        if (xx > x1)
                            x1 = xx;
                        if (yy < y0)
                            y0 = yy;
                        if (yy > y1)
                            y1 = yy;
                    }
                }
            }
            if (x1 >= x0 && y1 >= y0) {
                const int pad = 4;
                x0 = (std::max)(x0 - pad, 0);
                y0 = (std::max)(y0 - pad, 0);
                x1 = (std::min)(x1 + pad, w - 1);
                y1 = (std::min)(y1 + pad, h - 1);
                cu0 = (float)x0 / (float)w;
                cv0 = (float)y0 / (float)h;
                cu1 = (float)(x1 + 1) / (float)w;
                cv1 = (float)(y1 + 1) / (float)h;
            }
        }
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = (UINT)w;
        desc.Height = (UINT)h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA sd{};
        sd.pSysMem = pix;
        sd.SysMemPitch = (UINT)(w * 4);
        ID3D11Texture2D* tex = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;
        if (SUCCEEDED(g_dev->CreateTexture2D(&desc, &sd, &tex))) {
            D3D11_SHADER_RESOURCE_VIEW_DESC vd{};
            vd.Format = desc.Format;
            vd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            vd.Texture2D.MipLevels = 1;
            if (SUCCEEDED(g_dev->CreateShaderResourceView(tex, &vd, &srv))) {
                std::lock_guard<std::mutex> lk(g_thumbMtx);
                auto& t = g_thumbs[uid];
                if (t.tex)
                    t.tex->Release();
                t.tex = srv;
                t.w = w;
                t.h = h;
                t.loading = false;
                t.cu0 = cu0;
                t.cv0 = cv0;
                t.cu1 = cu1;
                t.cv1 = cv1;
                char b[160];
                snprintf(b, sizeof(b), "thumb OK uid=%lld %dx%d imgBytes=%llu", uid, w, h,
                    (unsigned long long)imgData.size());
                PvLog(b);
            } else {
                char b[128];
                snprintf(b, sizeof(b), "thumb FAIL uid=%lld stage=srv-create-failed", uid);
                PvLog(b);
            }
            if (tex)
                tex->Release();
        } else {
            char b[128];
            snprintf(b, sizeof(b), "thumb FAIL uid=%lld stage=texture-create-failed %dx%d", uid, w, h);
            PvLog(b);
        }
        stbi_image_free(pix);
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        g_pending.erase(uid);
    }).detach();
}


struct MeshVert {
    float x, y, z, u, v, cr, cg, cb;
};
struct MeshMat {
    float r = 1.0f, g = 1.0f, b = 1.0f;
    int tex = -1;
};
struct AvatarMesh {
    std::vector<MeshVert> verts;
    std::vector<std::uint32_t> idx;
    std::vector<int> triMat;
    std::vector<char> triUV;
    std::vector<MeshMat> mats;
    std::vector<ID3D11ShaderResourceView*> texs;
    bool ready = false;
    bool loading = false;
};
std::unordered_map<long long, AvatarMesh> g_meshes;
std::unordered_set<long long> g_meshPending;

std::string AbsUrl(const std::string& u) {
    if (u.rfind("http://", 0) == 0 || u.rfind("https://", 0) == 0)
        return u;
    if (!u.empty() && u[0] == '/')
        return "https://thumbnails.roblox.com" + u;
    return u;
}

void SplitHostPath(const std::string& url, std::string& host, std::string& path) {
    std::string tmp = url;
    if (tmp.rfind("https://", 0) == 0)
        tmp = tmp.substr(8);
    else if (tmp.rfind("http://", 0) == 0)
        tmp = tmp.substr(7);
    const auto sl = tmp.find('/');
    if (sl != std::string::npos) {
        host = tmp.substr(0, sl);
        path = tmp.substr(sl);
    } else {
        host = tmp;
        path = "/";
    }
}

std::string UrlDir(const std::string& url) {
    const auto q = url.find_last_of('/');
    if (q == std::string::npos || q < 8)
        return url;
    return url.substr(0, q);
}

std::string TrimCp(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n'))
        ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n'))
        --b;
    return s.substr(a, b - a);
}

std::string LowerCp(std::string s) {
    for (char& c : s)
        c = (char)std::tolower((unsigned char)c);
    return s;
}

std::string ExtractMapFile(const std::string& trimmedLine) {
    const auto sp = trimmedLine.find_first_of(" \t");
    if (sp == std::string::npos)
        return {};
    std::string rest = TrimCp(trimmedLine.substr(sp + 1));
    size_t last = rest.find_last_of(" \t");
    std::string file = (last == std::string::npos) ? rest : TrimCp(rest.substr(last + 1));
    if (file.size() >= 2 && file.front() == '"' && file.back() == '"')
        file = file.substr(1, file.size() - 2);
    if (const auto bs = file.find_last_of('\\'); bs != std::string::npos)
        file = file.substr(bs + 1);
    return file;
}

bool IsMapKey(const std::string& trimmedLine, const char* key) {
    const size_t kl = std::strlen(key);
    if (trimmedLine.size() <= kl || trimmedLine.compare(0, kl, key) != 0)
        return false;
    const char c = trimmedLine[kl];
    return c == ' ' || c == '\t';
}

std::string DownloadUrl(const std::string& url, const std::wstring& headers = L"");
int FetchTexture(const std::string& url, AvatarMesh& mesh,
    std::unordered_map<std::string, int>& texByName,
    const std::unordered_map<std::string, std::string>* preloaded = nullptr) {
    if (url.empty() || mesh.texs.size() >= 8)
        return -1;
    auto it = texByName.find(url);
    if (it != texByName.end())
        return it->second;
    std::string data;
    if (preloaded) {
        if (auto pit = preloaded->find(url); pit != preloaded->end())
            data = pit->second;
    }
    if (data.empty())
        data = DownloadUrl(url);
    if (data.empty() || data.size() > 16 * 1024 * 1024)
        return -1;
    int w = 0, h = 0, ch = 0;
    unsigned char* pix =
        stbi_load_from_memory((const unsigned char*)data.data(), (int)data.size(), &w, &h, &ch, 4);
    if (!pix || w <= 0 || h <= 0 || !g_dev)
        return -1;
    D3D11_TEXTURE2D_DESC td{};
    td.Width = (UINT)w;
    td.Height = (UINT)h;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = pix;
    sd.SysMemPitch = (UINT)(w * 4);
    ID3D11Texture2D* tex = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    if (SUCCEEDED(g_dev->CreateTexture2D(&td, &sd, &tex))) {
        D3D11_SHADER_RESOURCE_VIEW_DESC vd{};
        vd.Format = td.Format;
        vd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        vd.Texture2D.MipLevels = 1;
        if (FAILED(g_dev->CreateShaderResourceView(tex, &vd, &srv)))
            srv = nullptr;
        if (tex)
            tex->Release();
    }
    stbi_image_free(pix);
    if (!srv)
        return -1;
    const int id = (int)mesh.texs.size();
    mesh.texs.push_back(srv);
    texByName[url] = id;
    return id;
}

bool ParseAvatarObj(const std::string& text, const std::string& mtlText, const std::string& texBase,
    AvatarMesh& mesh, const std::unordered_map<std::string, std::string>* preloaded = nullptr) {
    struct Mtl {
        float r = 1.0f, g = 1.0f, b = 1.0f;
        std::string mapKd;
    };
    std::unordered_map<std::string, Mtl> mtls;
    std::unordered_map<std::string, std::string> mtlLower;
    {
        std::istringstream mf(mtlText);
        std::string ml, cur;
        while (std::getline(mf, ml)) {
            const std::string t = TrimCp(ml);
            if (t.compare(0, 7, "newmtl ") == 0 || (t.size() > 6 && t.compare(0, 6, "newmtl") == 0 &&
                    (t[6] == ' ' || t[6] == '\t'))) {
                cur = TrimCp(t.substr(6));
                if (mtls.find(cur) == mtls.end()) {
                    mtls[cur] = Mtl{};
                    mtlLower[LowerCp(cur)] = cur;
                }
            } else if (t.compare(0, 3, "Kd ") == 0 && !cur.empty()) {
                float r = 1, g = 1, b = 1;
                std::sscanf(t.c_str() + 3, "%f %f %f", &r, &g, &b);
                mtls[cur].r = r;
                mtls[cur].g = g;
                mtls[cur].b = b;
            } else if (IsMapKey(t, "map_Kd") && !cur.empty()) {
                const std::string file = ExtractMapFile(t);
                if (!file.empty())
                    mtls[cur].mapKd = file;
            }
        }
    }
    std::vector<float> pos, uv;
    struct RawTri {
        int v[3], t[3];
        std::string mat;
    };
    std::vector<RawTri> rawTris;
    std::string curMtl;
    {
        std::istringstream f(text);
        std::string line;
        auto triple = [](const std::string& tok, int& vi, int& vti) {
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
            if (line.compare(0, 2, "v ") == 0) {
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
                curMtl = TrimCp(line.substr(7));
            } else if (line.compare(0, 2, "f ") == 0) {
                std::istringstream ss(line.substr(2));
                std::string tok;
                std::vector<int> vis, vts;
                while (ss >> tok) {
                    int vi, vti;
                    triple(tok, vi, vti);
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
    }
    if (rawTris.empty())
        return false;
    mesh.mats.clear();
    mesh.mats.push_back(MeshMat{});
    std::unordered_map<std::string, int> matIds;
    std::unordered_map<std::string, int> texByName;
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
    constexpr int kMaxVerts = 65536;
    for (auto& rt : rawTris) {
        int mi = 0;
        if (!rt.mat.empty()) {
            auto mii = matIds.find(rt.mat);
            if (mii == matIds.end()) {
                mi = (int)mesh.mats.size();
                matIds[rt.mat] = mi;
                MeshMat m;
                if (auto mit = mtls.find(rt.mat); mit != mtls.end()) {
                    m.r = mit->second.r;
                    m.g = mit->second.g;
                    m.b = mit->second.b;
                    if (!mit->second.mapKd.empty()) {
                        const std::string t = mit->second.mapKd;
                        const std::string full = (t.rfind("http://", 0) == 0 || t.rfind("https://", 0) == 0)
                            ? t
                            : texBase + "/" + t;
                        m.tex = FetchTexture(full, mesh, texByName, preloaded);
                    }
                } else if (auto lit = mtlLower.find(LowerCp(rt.mat)); lit != mtlLower.end()) {
                    if (auto mit2 = mtls.find(lit->second); mit2 != mtls.end()) {
                        m.r = mit2->second.r;
                        m.g = mit2->second.g;
                        m.b = mit2->second.b;
                        if (!mit2->second.mapKd.empty()) {
                            const std::string t = mit2->second.mapKd;
                            const std::string full = (t.rfind("http://", 0) == 0 || t.rfind("https://", 0) == 0)
                                ? t
                                : texBase + "/" + t;
                            m.tex = FetchTexture(full, mesh, texByName, preloaded);
                        }
                    }
                }
                mesh.mats.push_back(m);
            } else {
                mi = mii->second;
            }
        }
        std::uint32_t out[3]{};
        bool ok = true;
        for (int k = 0; k < 3; ++k) {
            const Key kk{rt.v[k], rt.t[k], mi};
            if (auto it = dedup.find(kk); it != dedup.end()) {
                out[k] = it->second;
                continue;
            }
            if ((int)mesh.verts.size() >= kMaxVerts) {
                ok = false;
                break;
            }
            MeshVert w{};
            w.x = pos[(std::size_t)rt.v[k] * 3 + 0];
            w.y = pos[(std::size_t)rt.v[k] * 3 + 1];
            w.z = pos[(std::size_t)rt.v[k] * 3 + 2];
            if (rt.t[k] >= 0) {
                w.u = uv[(std::size_t)rt.t[k] * 2 + 0];
                w.v = 1.0f - uv[(std::size_t)rt.t[k] * 2 + 1];
            }
            const MeshMat& mm = mesh.mats[(std::size_t)mi];
            w.cr = mm.r;
            w.cg = mm.g;
            w.cb = mm.b;
            out[k] = (std::uint32_t)mesh.verts.size();
            mesh.verts.push_back(w);
            dedup[kk] = out[k];
        }
        if (!ok)
            continue;
        mesh.idx.push_back(out[0]);
        mesh.idx.push_back(out[1]);
        mesh.idx.push_back(out[2]);
        mesh.triMat.push_back(mi);
        mesh.triUV.push_back((rt.t[0] >= 0 && rt.t[1] >= 0 && rt.t[2] >= 0) ? 1 : 0);
    }
    if (mesh.verts.empty() || mesh.idx.empty())
        return false;
    float mnx = 1e9f, mny = 1e9f, mnz = 1e9f, mxx = -1e9f, mxy = -1e9f, mxz = -1e9f;
    for (auto& w : mesh.verts) {
        if (w.x < mnx)
            mnx = w.x;
        if (w.y < mny)
            mny = w.y;
        if (w.z < mnz)
            mnz = w.z;
        if (w.x > mxx)
            mxx = w.x;
        if (w.y > mxy)
            mxy = w.y;
        if (w.z > mxz)
            mxz = w.z;
    }
    const float cx = (mnx + mxx) * 0.5f, cy = (mny + mxy) * 0.5f, cz = (mnz + mxz) * 0.5f;
    const float ext = (std::max)((mxx - mnx), (std::max)((mxy - mny), (mxz - mnz)));
    if (ext < 1e-6f)
        return false;
    const float s = 1.15f / ext;
    for (auto& w : mesh.verts) {
        w.x = (w.x - cx) * s;
        w.y = (w.y - cy) * s;
        w.z = (w.z - cz) * s;
    }
    return true;
}

std::string ExtractJsonStrAt(const std::string& json, const char* key, size_t from) {
    std::string pat = std::string("\"") + key + "\"";
    const auto p = json.find(pat, from);
    if (p == std::string::npos)
        return {};
    const auto c = json.find(':', p + pat.size());
    if (c == std::string::npos)
        return {};
    const auto q1 = json.find('"', c);
    if (q1 == std::string::npos)
        return {};
    const auto q2 = json.find('"', q1 + 1);
    if (q2 == std::string::npos)
        return {};
    return json.substr(q1 + 1, q2 - q1 - 1);
}

std::string ExtractJsonStr(const std::string& json, const char* key) {
    return ExtractJsonStrAt(json, key, 0);
}

constexpr wchar_t kVercelKeyW[] = L"bWefKrE-p6ItjXw0fUqO61t3YcsVkver2r";
constexpr char kVercelKeyA[] = "bWefKrE-p6ItjXw0fUqO61t3YcsVkver2r";
constexpr char kVercelPassA[] = "a6707d0df1f04daa0f036a1d";

std::wstring VercelHeaders(const std::wstring& cookie) {
    std::wstring h = L"x-api-key: ";
    h += kVercelKeyW;
    h += L"\r\n";
    if (!cookie.empty())
        h += L"Cookie: " + cookie + L"\r\n";
    return h;
}

std::wstring LoginVercel() {
    std::wstring cookie;
    const std::string body = std::string("{\"password\":\"") + kVercelPassA + "\"}";
    HINTERNET hS =
        WinHttpOpen(L"jatos/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
    if (!hS)
        return cookie;
    HINTERNET hC = WinHttpConnect(hS, L"rblxapi.vercel.app", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hC) {
        WinHttpCloseHandle(hS);
        return cookie;
    }
    HINTERNET hR = WinHttpOpenRequest(hC, L"POST", L"/api/auth/login", nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hR) {
        WinHttpCloseHandle(hC);
        WinHttpCloseHandle(hS);
        return cookie;
    }
    static const wchar_t* kCt = L"Content-Type: application/json\r\n";
    const DWORD blen = (DWORD)body.size();
    if (WinHttpSendRequest(hR, kCt, (DWORD)wcslen(kCt), (LPVOID)body.data(), blen, blen, 0) &&
        WinHttpReceiveResponse(hR, nullptr)) {
        DWORD dwSize = 0;
        WinHttpQueryHeaders(hR, WINHTTP_QUERY_SET_COOKIE, WINHTTP_HEADER_NAME_BY_INDEX,
            WINHTTP_NO_OUTPUT_BUFFER, &dwSize, WINHTTP_NO_HEADER_INDEX);
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && dwSize > 2) {
            std::vector<wchar_t> buf(dwSize / sizeof(wchar_t) + 1, L'\0');
            DWORD rd = (DWORD)buf.size() * (DWORD)sizeof(wchar_t);
            if (WinHttpQueryHeaders(hR, WINHTTP_QUERY_SET_COOKIE, WINHTTP_HEADER_NAME_BY_INDEX,
                    buf.data(), &rd, WINHTTP_NO_HEADER_INDEX)) {
                std::wstring full(buf.data());
                const auto semi = full.find(L';');
                cookie = (semi == std::wstring::npos) ? full : full.substr(0, semi);
            }
        }
        for (;;) {
            DWORD avail = 0;
            if (!WinHttpQueryDataAvailable(hR, &avail) || !avail)
                break;
            std::vector<char> tmp(avail);
            DWORD got = 0;
            if (!WinHttpReadData(hR, tmp.data(), avail, &got) || !got)
                break;
        }
    }
    WinHttpCloseHandle(hR);
    WinHttpCloseHandle(hC);
    WinHttpCloseHandle(hS);
    return cookie;
}

std::unordered_map<std::string, std::string> ParseVercelTextures(const std::string& meta) {
    std::unordered_map<std::string, std::string> out;
    const auto tp = meta.find("\"textures\"");
    if (tp == std::string::npos)
        return out;
    size_t pos = meta.find('[', tp);
    if (pos == std::string::npos)
        return out;
    int depth = 0;
    while (pos < meta.size()) {
        const char c = meta[pos];
        if (c == '[') {
            ++depth;
        } else if (c == ']') {
            if (--depth <= 0)
                break;
        } else if (c == '{') {
            const auto end = meta.find('}', pos);
            if (end == std::string::npos)
                break;
            const std::string obj = meta.substr(pos, end - pos);
            const std::string id = ExtractJsonStr(obj, "id");
            const std::string url = ExtractJsonStr(obj, "url");
            if (!id.empty() && !url.empty() && out.find(id) == out.end())
                out[id] = url;
            pos = end;
        }
        ++pos;
    }
    return out;
}

std::string RewriteMtlIds(const std::string& mtlText,
    const std::unordered_map<std::string, std::string>& idToUrl) {
    std::istringstream in(mtlText);
    std::ostringstream out;
    std::string line;
    const char* keys[] = {"map_Kd", "map_Ka", "map_Ks"};
    while (std::getline(in, line)) {
        std::string t = TrimCp(line);
        if (t.rfind("map_d ", 0) == 0 || (t.size() > 5 && t.compare(0, 5, "map_d") == 0 &&
                (t[5] == ' ' || t[5] == '\t')))
            continue;
        for (auto k : keys) {
            if (IsMapKey(t, k)) {
                const std::string id = ExtractMapFile(t);
                if (auto it = idToUrl.find(id); it != idToUrl.end())
                    t = std::string(k) + " " + it->second;
                break;
            }
        }
        out << t << "\n";
    }
    return out.str();
}

std::string DownloadUrl(const std::string& url, const std::wstring& headers) {
    if (url.empty())
        return {};
    std::string host, path;
    SplitHostPath(url, host, path);
    const std::wstring wh(host.begin(), host.end()), wp(path.begin(), path.end());
    return HttpGet(wh, wp, headers);
}

std::vector<std::string> CollectMapKdUrls(const std::string& mtlText) {
    std::vector<std::string> out;
    std::istringstream in(mtlText);
    std::string line;
    while (out.size() < 8 && std::getline(in, line)) {
        const std::string t = TrimCp(line);
        if (IsMapKey(t, "map_Kd")) {
            const std::string u = ExtractMapFile(t);
            if ((u.rfind("http://", 0) == 0 || u.rfind("https://", 0) == 0) &&
                std::find(out.begin(), out.end(), u) == out.end())
                out.push_back(u);
        }
    }
    return out;
}

void RequestAvatarMesh(long long uid) {
    if (uid <= 0)
        return;
    {
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        if (g_meshes.find(uid) != g_meshes.end() || g_meshPending.find(uid) != g_meshPending.end())
            return;
        g_meshPending.insert(uid);
        g_meshes[uid] = AvatarMesh{};
        g_meshes[uid].loading = true;
    }
    {
        char b[128];
        snprintf(b, sizeof(b), "mesh request uid=%lld", uid);
        PvLog(b);
    }
    std::thread([uid] {
        auto fail = [&](const char* stage) {
            char b[160];
            snprintf(b, sizeof(b), "mesh FAIL uid=%lld stage=%s", uid, stage);
            PvLog(b);
            std::lock_guard<std::mutex> lk(g_thumbMtx);
            g_meshPending.erase(uid);
            if (auto it = g_meshes.find(uid); it != g_meshes.end())
                it->second.loading = false;
        };
        std::string objText, mtlText, texBase;
        std::unordered_map<std::string, std::string> preloaded;
        std::wstring cookie;
        std::string meta, state;
        bool reauthed = false;
        for (int attempt = 0; attempt < 40; ++attempt) {
            meta = HttpGet(L"rblxapi.vercel.app", L"/api/avatar?userId=" + std::to_wstring(uid),
                VercelHeaders(cookie));
            if (meta.empty()) {
                fail("meta-empty");
                return;
            }
            state = ExtractJsonStr(meta, "state");
            if (state == "Completed")
                break;
            if (meta.find("\"error\"") != std::string::npos) {
                if (!reauthed) {
                    reauthed = true;
                    cookie.clear();
                    cookie = LoginVercel();
                    {
                        char b[96];
                        snprintf(b, sizeof(b), "mesh reauth uid=%lld cookie=%s", uid,
                            cookie.empty() ? "none" : "ok");
                        PvLog(b);
                    }
                    continue;
                }
                char b[288];
                std::string head = meta.substr(0, 160);
                snprintf(b, sizeof(b), "mesh api-error uid=%lld resp=%.160s", uid, head.c_str());
                PvLog(b);
                fail("api-error");
                return;
            }
            {
                static std::string lastPoll;
                char b[160];
                snprintf(b, sizeof(b), "mesh poll uid=%lld state=%s", uid,
                    state.empty() ? "?" : state.c_str());
                if (lastPoll != b) {
                    lastPoll = b;
                    PvLog(b);
                }
            }
            if (state.empty() && attempt == 0) {
                char b[288];
                std::string head = meta.substr(0, 160);
                snprintf(b, sizeof(b), "mesh meta-head uid=%lld bytes=%llu resp=%.160s", uid,
                    (unsigned long long)meta.size(), head.c_str());
                PvLog(b);
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (state != "Completed") {
            fail("poll-timeout");
            return;
        }
        {
            char b[288];
            std::string head = meta.substr(0, 160);
            snprintf(b, sizeof(b), "mesh meta uid=%lld bytes=%llu resp=%.160s", uid,
                (unsigned long long)meta.size(), head.c_str());
            PvLog(b);
        }
        const std::string objUrl = ExtractJsonStr(meta, "objUrl");
        const std::string mtlUrl = ExtractJsonStr(meta, "mtlUrl");
        if (objUrl.empty()) {
            fail("no-obj-url");
            return;
        }
        const auto texIdToUrl = ParseVercelTextures(meta);
        {
            const std::string objU = objUrl, mtlU = mtlUrl;
            std::thread tObj([&] { objText = DownloadUrl(objU); });
            std::thread tMtl([&] {
                mtlText = mtlU.empty() ? std::string() : DownloadUrl(mtlU);
                texBase = mtlU.empty() ? UrlDir(objU) : UrlDir(mtlU);
            });
            tObj.join();
            tMtl.join();
            char b[160];
            snprintf(b, sizeof(b), "mesh obj+mtl uid=%lld objBytes=%llu mtlBytes=%llu", uid,
                (unsigned long long)objText.size(), (unsigned long long)mtlText.size());
            PvLog(b);
        }
        if (objText.empty()) {
            fail("obj-download-empty");
            return;
        }
        mtlText = RewriteMtlIds(mtlText, texIdToUrl);
        {
            const auto texUrls = CollectMapKdUrls(mtlText);
            std::mutex preMtx;
            std::vector<std::thread> th;
            for (const auto& u : texUrls)
                th.emplace_back([&, u] {
                    std::string d = DownloadUrl(u);
                    if (!d.empty()) {
                        std::lock_guard<std::mutex> lk(preMtx);
                        preloaded[u] = std::move(d);
                    }
                });
            for (auto& t : th)
                t.join();
            char b[128];
            snprintf(b, sizeof(b), "mesh tex uid=%lld got=%llu/%llu", uid,
                (unsigned long long)preloaded.size(), (unsigned long long)texUrls.size());
            PvLog(b);
        }
        mtlText = RewriteMtlIds(mtlText, texIdToUrl);
        AvatarMesh mesh;
        mesh.mats.push_back(MeshMat{});
        if (!ParseAvatarObj(objText, mtlText, texBase, mesh, &preloaded)) {
            fail("obj-parse-failed");
            return;
        }
        {
            std::lock_guard<std::mutex> lk(g_thumbMtx);
            auto& dst = g_meshes[uid];
            for (auto t : dst.texs)
                if (t)
                    t->Release();
            dst = std::move(mesh);
            dst.ready = true;
            dst.loading = false;
            g_meshPending.erase(uid);
        }
        {
            char b[160];
            const auto& m = g_meshes[uid];
            snprintf(b, sizeof(b), "mesh OK uid=%lld verts=%llu tris=%llu mats=%llu texs=%llu", uid,
                (unsigned long long)m.verts.size(), (unsigned long long)(m.idx.size() / 3),
                (unsigned long long)m.mats.size(), (unsigned long long)m.texs.size());
            PvLog(b);
        }
    }).detach();
}

long long ResolveSubjectUid(std::string& outName) {
    outName.clear();
    static std::string lastKey;
    static long long lastUid = 0;
    static std::string lastResolvedName;
    static auto lastT = std::chrono::steady_clock::now() - std::chrono::seconds(10);
    const std::string key = "local:" + std::to_string(Globals::localPlayer.Addr);
    const auto now = std::chrono::steady_clock::now();
    if (key == lastKey && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastT).count() < 1000) {
        outName = lastResolvedName;
        return lastUid;
    }
    lastKey = key;
    lastT = now;
    std::string resolved;
    std::uintptr_t addr = 0;
    if (Globals::localPlayer.Addr) {
        addr = Globals::localPlayer.Addr;
        resolved = Globals::localPlayer.GetName();
    }
    long long uid = 0;
    if (addr && memory)
        uid = memory->read<long long>(addr + Offsets::Player::UserId);
    const long long memUid = uid;
    if (uid <= 0 && !resolved.empty()) {
        RequestUidByName(resolved);
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        if (auto it = g_nameUid.find(resolved); it != g_nameUid.end())
            uid = it->second;
    }
    lastUid = uid > 0 ? uid : 0;
    lastResolvedName = resolved;
    {
        static std::string lastLogged;
        char b[256];
        snprintf(b, sizeof(b), "resolve localAddr=0x%llX name='%s' memUid=%lld finalUid=%lld",
            (unsigned long long)addr, resolved.c_str(), memUid, lastUid);
        if (lastLogged != b) {
            lastLogged = b;
            PvLog(b);
        }
    }
    outName = resolved;
    return lastUid;
}

bool GetLocalHealth(float& hp, float& maxHp) {
    static std::uintptr_t s_hum = 0;
    static auto s_lastT = std::chrono::steady_clock::now() - std::chrono::seconds(10);
    const auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastT).count() > 500) {
        s_lastT = now;
        s_hum = 0;
        if (memory && Globals::localPlayer.Addr) {
            if (auto ch = Globals::localPlayer.GetModelRef(); ch.Addr)
                s_hum = ch.FindChildByClass("Humanoid").Addr;
        }
    }
    if (!s_hum || !memory)
        return false;
    hp = memory->read<float>(s_hum + Offsets::Humanoid::Health);
    maxHp = memory->read<float>(s_hum + Offsets::Humanoid::MaxHealth);
    return maxHp > 0.0f && hp >= 0.0f;
}

}

bool Init(ID3D11Device* dev) {
    if (dev)
        g_dev = dev;
    LoadApiKey();
    PvLog("avatar3d: proxy mode");
    return g_dev != nullptr;
}

void Shutdown() {
    std::lock_guard<std::mutex> lk(g_thumbMtx);
    for (auto& kv : g_thumbs)
        if (kv.second.tex)
            kv.second.tex->Release();
    g_thumbs.clear();
    g_pending.clear();
    for (auto& kv : g_meshes)
        for (auto t : kv.second.texs)
            if (t)
                t->Release();
    g_meshes.clear();
    g_meshPending.clear();
    g_dev = nullptr;
}

bool Ready() {
    return g_dev != nullptr;
}

struct MeshView {
    float ca = 1.0f, sa = 0.0f, cp = 1.0f, sp = 0.0f;
    float scale = 1.0f, cx = 0.0f, cy = 0.0f;
    bool valid = false;
};

bool DrawMesh(ImDrawList* dl, const AvatarMesh& m, const ImVec2& rmin, const ImVec2& rmax,
    ImVec2& outMin, ImVec2& outMax, MeshView* outView = nullptr) {
    if (m.verts.empty() || m.idx.empty() || m.triMat.size() * 3 != m.idx.size())
        return false;
    const ImVec2 mp = ImGui::GetIO().MousePos;
    const bool hovered =
        variables::menuOpen && mp.x >= rmin.x && mp.x <= rmax.x && mp.y >= rmin.y && mp.y <= rmax.y;
    static bool dragging = false;
    static float yawD = 0.0f, pitch = 0.0f, zoom = 1.0f;
    constexpr float kCenterYaw = 0.0f;
    static bool wasSpin = true;
    if (wasSpin && !variables::previewSpin) {
        yawD = kCenterYaw;
        pitch = 0.0f;
        zoom = 1.0f;
        dragging = false;
    }
    wasSpin = variables::previewSpin;
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        dragging = true;
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        dragging = false;
    if (dragging) {
        yawD += ImGui::GetIO().MouseDelta.x * 0.015f;
        pitch += -ImGui::GetIO().MouseDelta.y * 0.015f;
        pitch = (std::clamp)(pitch, -1.0f, 1.0f);
    } else if (variables::previewSpin) {
        float dt = ImGui::GetIO().DeltaTime;
        if (dt <= 0.0f || dt > 0.1f)
            dt = 0.016f;
        yawD -= dt * 0.7f;
    }
    if (hovered) {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f)
            zoom = (std::clamp)(zoom + wheel * 0.10f, 0.4f, 2.5f);
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            yawD = kCenterYaw;
            pitch = 0.0f;
            zoom = 1.0f;
        }
    }
    const float ca = cosf(yawD), sa = sinf(yawD);
    const float cp = cosf(pitch), sp = sinf(pitch);
    const float rw = rmax.x - rmin.x;
    const float rh = rmax.y - rmin.y;
    float ext = 0.0f;
    for (auto& w : m.verts) {
        const float dx = w.x >= 0 ? w.x : -w.x;
        const float dy = w.y >= 0 ? w.y : -w.y;
        const float dz = w.z >= 0 ? w.z : -w.z;
        const float mm = dx > dy ? (dx > dz ? dx : dz) : (dy > dz ? dy : dz);
        if (mm > ext)
            ext = mm;
    }
    if (ext < 1e-6f)
        ext = 1.0f;
    const float scale = (std::min)(rw, rh) * 0.60f / ext * zoom;
    const float cx = rmin.x + rw * 0.5f;
    const float cy = rmin.y + rh * 0.47f;
    if (outView) {
        outView->ca = ca;
        outView->sa = sa;
        outView->cp = cp;
        outView->sp = sp;
        outView->scale = scale;
        outView->cx = cx;
        outView->cy = cy;
        outView->valid = true;
    }

    struct PT {
        float x, y, d;
    };
    static std::vector<PT> proj;
    proj.clear();
    if (proj.capacity() < m.verts.size())
        proj.reserve(m.verts.size());
    float bMinX = 1e9f, bMaxX = -1e9f, bMinY = 1e9f, bMaxY = -1e9f;
    for (auto& w : m.verts) {
        const float rx = w.x * ca + w.z * sa;
        const float rz = -w.x * sa + w.z * ca;
        const float ry = w.y * cp + rz * sp;
        const float rz2 = -w.y * sp + rz * cp;
        const float px = cx + rx * scale;
        const float py = cy - ry * scale;
        proj.push_back({px, py, rz2});
        if (px < bMinX)
            bMinX = px;
        if (px > bMaxX)
            bMaxX = px;
        if (py < bMinY)
            bMinY = py;
        if (py > bMaxY)
            bMaxY = py;
    }
    const float pad = 4.0f;
    outMin = ImVec2(bMinX - pad, bMinY - pad);
    outMax = ImVec2(bMaxX + pad, bMaxY + pad);

    struct RT {
        float d;
        std::uint32_t t;
    };
    static std::vector<RT> order;
    order.clear();
    const std::size_t ntris = m.idx.size() / 3;
    if (order.capacity() < ntris)
        order.reserve(ntris);
    for (std::size_t ti = 0; ti < ntris; ++ti) {
        const std::uint32_t i0 = m.idx[ti * 3 + 0];
        const std::uint32_t i1 = m.idx[ti * 3 + 1];
        const std::uint32_t i2 = m.idx[ti * 3 + 2];
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

    const ImVec2 whiteUv = ImGui::GetFontTexUvWhitePixel();
    ImTextureID curTex = (ImTextureID)0;
    bool pushed = false;
    for (auto& r : order) {
        const std::uint32_t i0 = m.idx[(std::size_t)r.t * 3 + 0];
        const std::uint32_t i1 = m.idx[(std::size_t)r.t * 3 + 1];
        const std::uint32_t i2 = m.idx[(std::size_t)r.t * 3 + 2];
        const MeshVert& v0 = m.verts[i0];
        const MeshVert& v1 = m.verts[i1];
        const MeshVert& v2 = m.verts[i2];
        const PT& a = proj[i0];
        const PT& b = proj[i1];
        const PT& c = proj[i2];
        const int mi = (r.t < m.triMat.size()) ? m.triMat[(std::size_t)r.t] : 0;
        const MeshMat& mm = (mi >= 0 && (std::size_t)mi < m.mats.size()) ? m.mats[(std::size_t)mi] : m.mats[0];

        const float nx = (v1.y - v0.y) * (v2.z - v0.z) - (v1.z - v0.z) * (v2.y - v0.y);
        const float ny = (v1.z - v0.z) * (v2.x - v0.x) - (v1.x - v0.x) * (v2.z - v0.z);
        const float nz = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
        float lambert = 0.85f;
        if (const float nl = std::sqrt(nx * nx + ny * ny + nz * nz); nl > 1e-9f) {
            const float lx = nx / nl, ly = ny / nl, lz = nz / nl;
            const float lrx = lx * ca + lz * sa;
            const float lrz = -lx * sa + lz * ca;
            const float lry = ly * cp + lrz * sp;
            float ndl = lrx * 0.25f + lry * 0.45f + (ly * sp - lrz * cp) * 0.75f;
            if (ndl < 0.0f)
                ndl = -ndl * 0.35f;
            lambert = 0.65f + 0.35f * ndl;
        }

        ImTextureID want = (ImTextureID)0;
        if (mm.tex >= 0 && (std::size_t)mm.tex < m.texs.size() && m.texs[(std::size_t)mm.tex] &&
            r.t < m.triUV.size() && m.triUV[(std::size_t)r.t])
            want = (ImTextureID)m.texs[(std::size_t)mm.tex];
        if (want != curTex) {
            if (pushed) {
                dl->PopTexture();
                pushed = false;
            }
            curTex = want;
            if (curTex) {
                dl->PushTexture(curTex);
                pushed = true;
            }
        }
        ImVec2 uv0 = whiteUv, uv1 = whiteUv, uv2 = whiteUv;
        ImU32 col;
        if (want) {
            uv0 = ImVec2(v0.u, v0.v);
            uv1 = ImVec2(v1.u, v1.v);
            uv2 = ImVec2(v2.u, v2.v);
            const int l = (int)((std::clamp)(lambert, 0.0f, 1.0f) * 255.0f);
            col = IM_COL32(l, l, l, 255);
        } else {
            const float lr = (std::min)(mm.r * lambert + 0.08f, 1.0f);
            const float lg = (std::min)(mm.g * lambert + 0.08f, 1.0f);
            const float lb = (std::min)(mm.b * lambert + 0.08f, 1.0f);
            col = IM_COL32((int)(lr * 255.0f), (int)(lg * 255.0f), (int)(lb * 255.0f), 255);
        }
        dl->PrimReserve(3, 3);
        dl->PrimVtx(ImVec2(a.x, a.y), uv0, col);
        dl->PrimVtx(ImVec2(b.x, b.y), uv1, col);
        dl->PrimVtx(ImVec2(c.x, c.y), uv2, col);
    }
    if (pushed)
        dl->PopTexture();
    return true;
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

    static float ax = 690.0f, ay = 80.0f;
    static float sw = 0.0f;
    if (sw <= 0.0f)
        sw = (float)GetSystemMetrics(SM_CXSCREEN);
    std::string subject;
    const long long uid = ResolveSubjectUid(subject);
    if (uid > 0)
        RequestThumb(uid);
    Thumb thumb{};
    bool hasThumb = false, thumbLoading = false;
    {
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        if (auto it = g_thumbs.find(uid); it != g_thumbs.end()) {
            thumbLoading = it->second.loading;
            if (it->second.tex) {
                thumb = it->second;
                hasThumb = true;
            }
        }
    }
    {
        static long long lastStUid = -1;
        static int lastStCode = -1;
        const int code = hasThumb ? 2 : (thumbLoading ? 1 : 0);
        if (uid != lastStUid || code != lastStCode) {
            lastStUid = uid;
            lastStCode = code;
            char b[192];
            snprintf(b, sizeof(b), "panel uid=%lld state=%s name='%s'", uid,
                hasThumb ? "SHOWING" : (thumbLoading ? "LOADING" : (uid <= 0 ? "NO-PLAYER" : "NO-THUMBNAIL")),
                subject.c_str());
            PvLog(b);
        }
    }
    if (uid > 0)
        RequestAvatarMesh(uid);
    bool meshReady = false;
    {
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        if (auto it = g_meshes.find(uid); it != g_meshes.end())
            meshReady = it->second.ready && !it->second.verts.empty();
    }
    float h = 300.0f;
    if (meshReady)
        h = 452.0f;
    else if (hasThumb && thumb.w > 0 && thumb.h > 0) {
        const float cw = (thumb.cu1 - thumb.cu0) * (float)thumb.w;
        const float ch = (thumb.cv1 - thumb.cv0) * (float)thumb.h;
        if (cw > 1.0f && ch > 1.0f) {
            const float viewH = (std::clamp)((kPanelW - 16.0f) * ch / cw, 120.0f, 460.0f);
            h = 54.0f + viewH;
        }
    }
    if (ImGuiWindow* mw = ImGui::FindWindowByName("jatos")) {
        if (!mw->Collapsed) {
            const float nx = std::floor(mw->Pos.x) + std::floor(mw->Size.x) + 8.0f;
            if (nx + kPanelW <= sw - 8.0f) {
                ax = nx;
            } else {
                ax = std::floor(mw->Pos.x) - kPanelW - 8.0f;
            }
            ay = std::floor(mw->Pos.y);
        }
    }
    const float x = std::floor(ax);
    const float y = std::floor(ay);
    h = std::floor(h);

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
    const ImVec2 rmax(std::floor(pmin.x + kPanelW - 6.0f), std::floor(pmin.y + h - 32.0f));
    dl->AddRectFilled(rmin, rmax, imGuiCustom::ColorU32(theme.CardBg), 0.0f);
    dl->AddRect(rmin, rmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    dl->AddRect(rmin + ImVec2(1.0f, 1.0f), rmax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);

    static float pvZoom = 1.0f;
    static long long pvUid = -1;
    if (pvUid != uid) {
        pvUid = uid;
        pvZoom = 1.0f;
    }
    const ImVec2 mp = ImGui::GetIO().MousePos;
    const bool hovered = variables::menuOpen && mp.x >= rmin.x && mp.x <= rmax.x && mp.y >= rmin.y && mp.y <= rmax.y;
    if (hovered) {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            pvZoom += wheel * 0.12f;
            pvZoom = (std::clamp)(pvZoom, 1.0f, 3.0f);
        }
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            pvZoom = 1.0f;
    }

    dl->PushClipRect(rmin + ImVec2(2.0f, 2.0f), rmax - ImVec2(2.0f, 2.0f), true);
    ImVec2 dmin, dmax;
    bool meshDrawn = false;
    MeshView meshView;
    if (meshReady) {
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        if (auto it = g_meshes.find(uid); it != g_meshes.end())
            meshDrawn = DrawMesh(dl, it->second, rmin + ImVec2(2.0f, 2.0f),
                rmax - ImVec2(2.0f, 2.0f), dmin, dmax, &meshView);
    }
    if (!meshDrawn && hasThumb && thumb.tex && thumb.w > 0 && thumb.h > 0) {
        const float cw = (thumb.cu1 - thumb.cu0) * (float)thumb.w;
        const float ch = (thumb.cv1 - thumb.cv0) * (float)thumb.h;
        const float rw = rmax.x - rmin.x - 4.0f;
        const float rh = rmax.y - rmin.y - 4.0f;
        const float fit = (std::min)(rw / cw, rh / ch);
        constexpr float kModelScale = 0.7f;
        const float dw = cw * fit * kModelScale;
        const float dh = ch * fit * kModelScale;
        dmin = ImVec2(rmin.x + 2.0f + (rw - dw) * 0.5f, rmin.y + 2.0f + (rh - dh) * 0.5f);
        dmax = ImVec2(dmin.x + dw, dmin.y + dh);
        const float zs = 1.0f / pvZoom;
        const float uw = thumb.cu1 - thumb.cu0;
        const float uh = thumb.cv1 - thumb.cv0;
        const ImVec2 uvMin(thumb.cu0 + uw * (0.5f - 0.5f * zs), thumb.cv0 + uh * (0.5f - 0.5f * zs));
        const ImVec2 uvMax(thumb.cu0 + uw * (0.5f + 0.5f * zs), thumb.cv0 + uh * (0.5f + 0.5f * zs));
        dl->AddImage((ImTextureID)thumb.tex, dmin, dmax, uvMin, uvMax);
    }
    if (meshDrawn || (hasThumb && thumb.tex && thumb.w > 0 && thumb.h > 0)) {
        const float bx0 = dmin.x, by0 = dmin.y, bx1 = dmax.x, by1 = dmax.y;
        if (variables::ESP::enabled) {
            if (variables::ESP::boxes) {
                Visuals::DrawBoxFill(dl, bx0, by0, bx1, by1);
                const ImU32 bcol = imGuiCustom::ColorU32(variables::ESP::boxColor);
                if (variables::ESP::boxMode == 0) {
                    Visuals::DrawBoxCol(dl, bx0, by0, bx1, by1, bcol);
                } else {
                    const float cw = (bx1 - bx0) * 0.25f;
                    const float chh = (by1 - by0) * 0.20f;
                    auto corner = [&](ImVec2 p1, ImVec2 c, ImVec2 p2) {
                        dl->AddLine(p1, c, IM_COL32(0, 0, 0, 255), 3.0f);
                        dl->AddLine(c, p2, IM_COL32(0, 0, 0, 255), 3.0f);
                        dl->AddLine(p1, c, bcol, 1.5f);
                        dl->AddLine(c, p2, bcol, 1.5f);
                    };
                    corner(ImVec2(bx0, by0 + chh), ImVec2(bx0, by0), ImVec2(bx0 + cw, by0));
                    corner(ImVec2(bx1 - cw, by0), ImVec2(bx1, by0), ImVec2(bx1, by0 + chh));
                    corner(ImVec2(bx0, by1 - chh), ImVec2(bx0, by1), ImVec2(bx0 + cw, by1));
                    corner(ImVec2(bx1 - cw, by1), ImVec2(bx1, by1), ImVec2(bx1, by1 - chh));
                }
            }
            if (variables::ESP::healthBar) {
                float hphp = 0.0f, hpmax = 0.0f;
                const bool haveReal = GetLocalHealth(hphp, hpmax);
                float showHp = hphp, showMax = hpmax;
                if (variables::previewAnimateHealth) {
                    if (showMax <= 0.0f)
                        showMax = 100.0f;
                    showHp = showMax * (0.5f - 0.5f * std::cos((float)ImGui::GetTime() * 1.5f));
                } else if (!haveReal || showMax <= 0.0f) {
                    showHp = 1.0f;
                    showMax = 1.0f;
                }
                const float pct = (std::clamp)(showHp / showMax, 0.0f, 1.0f);
                const float hx0 = std::floor(bx0 - 6.0f);
                Visuals::DrawHealthBar(
                    dl, hx0, hx0 + 3.0f, by0, by1, pct, imGuiCustom::ColorU32(variables::ESP::healthColor));
            }
            struct SJ {
                float x, y, z;
            };
            const SJ jHead3 = {0.00f, 0.42f, 0.0f}, jNeck3 = {0.00f, 0.30f, 0.0f},
                     jPelvis3 = {0.00f, -0.02f, 0.0f};
            const SJ jLSh3 = {-0.19f, 0.27f, 0.0f}, jLElb3 = {-0.23f, 0.12f, 0.0f},
                     jLWr3 = {-0.24f, -0.03f, 0.0f};
            const SJ jRSh3 = {0.19f, 0.27f, 0.0f}, jRElb3 = {0.23f, 0.12f, 0.0f},
                     jRWr3 = {0.24f, -0.03f, 0.0f};
            const SJ jLHip3 = {-0.09f, -0.04f, 0.0f}, jLKnee3 = {-0.10f, -0.28f, 0.0f},
                     jLAnk3 = {-0.10f, -0.52f, 0.0f};
            const SJ jRHip3 = {0.09f, -0.04f, 0.0f}, jRKnee3 = {0.10f, -0.28f, 0.0f},
                     jRAnk3 = {0.10f, -0.52f, 0.0f};
            ImVec2 pHead, pNeck, pPelvis, pLSh, pLElb, pLWr, pRSh, pRElb, pRWr, pLHip, pLKnee,
                pLAnk, pRHip, pRKnee, pRAnk;
            ImVec2 vdDir(0.0f, -1.0f);
            float vdLen = 20.0f;
            if (meshDrawn && meshView.valid) {
                auto MP = [&](const SJ& j) {
                    const float rx = j.x * meshView.ca + j.z * meshView.sa;
                    const float rz = -j.x * meshView.sa + j.z * meshView.ca;
                    const float ry = j.y * meshView.cp + rz * meshView.sp;
                    return ImVec2(meshView.cx + rx * meshView.scale, meshView.cy - ry * meshView.scale);
                };
                pHead = MP(jHead3);
                pNeck = MP(jNeck3);
                pPelvis = MP(jPelvis3);
                pLSh = MP(jLSh3);
                pLElb = MP(jLElb3);
                pLWr = MP(jLWr3);
                pRSh = MP(jRSh3);
                pRElb = MP(jRElb3);
                pRWr = MP(jRWr3);
                pLHip = MP(jLHip3);
                pLKnee = MP(jLKnee3);
                pLAnk = MP(jLAnk3);
                pRHip = MP(jRHip3);
                pRKnee = MP(jRKnee3);
                pRAnk = MP(jRAnk3);
                const float frx = -meshView.sa;
                const float fry = -meshView.ca * meshView.sp;
                const float fl = std::sqrt(frx * frx + fry * fry);
                if (fl > 1e-5f)
                    vdDir = ImVec2(frx / fl, -fry / fl);
                vdLen = (variables::ESP::viewDirLength > 0.0f) ? variables::ESP::viewDirLength *
                                                                      meshView.scale * 0.05f :
                                                                meshView.scale * 0.15f;
            } else {
                const float bw = bx1 - bx0, bh = by1 - by0;
                auto BP = [&](float fx, float fy) { return ImVec2(bx0 + fx * bw, by0 + fy * bh); };
                pHead = BP(0.50f, 0.16f);
                pNeck = BP(0.50f, 0.27f);
                pPelvis = BP(0.50f, 0.55f);
                pLSh = BP(0.36f, 0.30f);
                pLElb = BP(0.34f, 0.42f);
                pLWr = BP(0.35f, 0.53f);
                pRSh = BP(0.64f, 0.30f);
                pRElb = BP(0.66f, 0.42f);
                pRWr = BP(0.65f, 0.53f);
                pLHip = BP(0.44f, 0.56f);
                pLKnee = BP(0.44f, 0.75f);
                pLAnk = BP(0.44f, 0.94f);
                pRHip = BP(0.56f, 0.56f);
                pRKnee = BP(0.56f, 0.75f);
                pRAnk = BP(0.56f, 0.94f);
                vdLen = (variables::ESP::viewDirLength > 0.0f) ? variables::ESP::viewDirLength * bh * 0.05f :
                                                                bh * 0.15f;
            }
            if (variables::ESP::skeleton) {
                auto bone = [&](const ImVec2& a, const ImVec2& b) {
                    if (variables::ESP::skeletonOutline)
                        dl->AddLine(a, b, IM_COL32(0, 0, 0, 255),
                            variables::ESP::skeletonThickness + 1.0f);
                    dl->AddLine(a, b, imGuiCustom::ColorU32(variables::ESP::skeletonColor),
                        variables::ESP::skeletonThickness);
                };
                bone(pHead, pNeck);
                bone(pNeck, pPelvis);
                bone(pNeck, pLSh);
                bone(pLSh, pLElb);
                bone(pLElb, pLWr);
                bone(pNeck, pRSh);
                bone(pRSh, pRElb);
                bone(pRElb, pRWr);
                bone(pPelvis, pLHip);
                bone(pLHip, pLKnee);
                bone(pLKnee, pLAnk);
                bone(pPelvis, pRHip);
                bone(pRHip, pRKnee);
                bone(pRKnee, pRAnk);
            }
            if (variables::ESP::headDot) {
                const float r = (std::max)(2.0f, variables::ESP::headDotSize);
                dl->AddCircleFilled(pHead, r, imGuiCustom::ColorU32(variables::ESP::headDotColor), 16);
                dl->AddCircle(pHead, r + 1.0f, IM_COL32(0, 0, 0, 200), 16, 1.0f);
            }
            if (variables::ESP::viewDirection) {
                const ImVec2 endP(pHead.x + vdDir.x * vdLen, pHead.y + vdDir.y * vdLen);
                const ImU32 vdCol = imGuiCustom::ColorU32(variables::ESP::viewDirColor);
                dl->AddLine(pHead, endP, IM_COL32(0, 0, 0, 255), 3.0f);
                dl->AddLine(pHead, endP, vdCol, 2.0f);
                dl->AddCircleFilled(endP, 2.5f, vdCol, 8);
            }
            if (variables::ESP::names && !subject.empty()) {
                ImFont* espF = Visuals::EspFont();
                const float espSz = variables::Misc::espFontSize;
                const ImVec2 ts = espF->CalcTextSizeA(espSz, FLT_MAX, 0.0f, subject.c_str());
                Visuals::DrawOutlinedText(dl,
                    ImVec2((bx0 + bx1) * 0.5f - ts.x * 0.5f, by0 - ts.y - 2.0f), subject,
                    imGuiCustom::ColorU32(variables::ESP::nameColor));
            }
        }
    } else {
        const char* txt = uid <= 0 ? "No player" : (thumbLoading ? "Loading..." : "No thumbnail");
        const float fs = 12.0f * imGuiCustom::g_fontScale;
        const ImVec2 tsz = title_font->CalcTextSizeA(fs, FLT_MAX, 0.0f, txt);
        const float rw = rmax.x - rmin.x;
        const float rh = rmax.y - rmin.y;
        dl->AddText(title_font, fs,
            ImVec2(rmin.x + (rw - tsz.x) * 0.5f, rmin.y + (rh - tsz.y) * 0.5f),
            imGuiCustom::ColorU32(theme.Text), txt);
    }
    dl->PopClipRect();

    dl->AddRect(rmin, rmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    dl->AddRect(rmin + ImVec2(1.0f, 1.0f), rmax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
    dl->AddRectFilled(pmin, pmin + ImVec2(kPanelW, 1.5f), imGuiCustom::ColorU32(ImVec4(0.5373f, 0.7647f, 0.7490f, 1.0f)));

    imGuiCustom::Checkbox("Spin", &variables::previewSpin, ImVec2(8.0f, h - 24.0f));
    imGuiCustom::Checkbox("Animate Health", &variables::previewAnimateHealth, ImVec2(58.0f, h - 24.0f));

    ImGui::End();
    ImGui::PopStyleVar();
}

}
