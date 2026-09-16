#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "explorer.h"
#include "../../globals/globals.h"
#include "../../../memory/memory.h"
#include "../../../sdk/offsets.h"
#include "../../../sdk/sdk.h"
#include "../../../../ext/imgui/imgui.h"
#include "../../../../ext/imgui/imgui_internal.h"
#include "../../../render/menu/library.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "../../../../ext/stb/stb_image.h"
#include "assets/dex_icons.h"

#include <d3d11.h>
#include <Windows.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace Explorer {
namespace {

bool g_open = false;
ID3D11Device* g_device = nullptr;

struct IconTex {
    ID3D11ShaderResourceView* tex = nullptr;
    int w = 0, h = 0;
};
std::unordered_map<std::string, IconTex> g_iconCache;
bool g_texturesLoaded = false;
std::mutex g_iconMutex;

bool LoadTextureFromMemory(const unsigned char* data, unsigned int data_size, const std::string& name) {
    if (!g_device)
        return false;
    int width, height, channels;
    unsigned char* image_data = stbi_load_from_memory(data, (int)data_size, &width, &height, &channels, 4);
    if (!image_data)
        return false;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width; desc.Height = height; desc.MipLevels = 1; desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA sub{};
    sub.pSysMem = image_data; sub.SysMemPitch = width * 4;
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = g_device->CreateTexture2D(&desc, &sub, &texture);
    if (FAILED(hr) || !texture) {
        stbi_image_free(image_data);
        return false;
    }
    ID3D11ShaderResourceView* srv = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    hr = g_device->CreateShaderResourceView(texture, &srvDesc, &srv);
    texture->Release();
    stbi_image_free(image_data);
    if (FAILED(hr) || !srv)
        return false;
    std::lock_guard<std::mutex> lock(g_iconMutex);
    g_iconCache[name] = IconTex{ srv, width, height };
    return true;
}

#define LOAD_ICON(name) LoadTextureFromMemory(name##_png, name##_png_len, #name)
void LoadAllIcons() {
    if (g_texturesLoaded || !g_device)
        return;
#include "assets/dex_icons_loader.inc"
    g_texturesLoaded = true;
}

IconTex* GetIcon(const std::string& classname) {
    std::lock_guard<std::mutex> lock(g_iconMutex);
    auto it = g_iconCache.find(classname);
    if (it != g_iconCache.end())
        return &it->second;
    auto fb = g_iconCache.find("Folder");
    if (fb != g_iconCache.end())
        return &fb->second;
    return nullptr;
}

std::string GetInstancePath(RBX::RbxInstance inst) {
    if (inst.Addr == 0)
        return "";
    RBX::RbxInstance parent = inst.GetParent();
    if (parent.Addr == 0 || parent.Addr == inst.Addr || parent.GetName() == "Game")
        return inst.GetName();
    std::string parent_path = GetInstancePath(parent);
    std::string name = inst.GetName();
    if (name.empty())
        name = "Unnamed";
    if (parent_path.empty())
        return name;
    return parent_path + "." + name;
}

bool FastContainsCI(const std::string& hay, const std::string& needleLower) {
    if (needleLower.empty())
        return true;
    if (hay.size() < needleLower.size())
        return false;
    const size_t nLen = needleLower.size(), hLen = hay.size();
    for (size_t i = 0; i <= hLen - nLen; ++i) {
        bool match = true;
        for (size_t j = 0; j < nLen; ++j) {
            char c1 = hay[i + j];
            if (c1 >= 'A' && c1 <= 'Z')
                c1 += 32;
            if (c1 != needleLower[j]) {
                match = false;
                break;
            }
        }
        if (match)
            return true;
    }
    return false;
}

struct CachedNode {
    std::string name;
    std::string cls;
    std::vector<uintptr_t> childAddrs;
    bool hasChildren = false;
    std::chrono::steady_clock::time_point ts{};
};

std::unordered_map<uintptr_t, CachedNode> s_cache;
std::mutex s_cacheMutex;

CachedNode GetCached(RBX::RbxInstance inst) {
    auto now = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lk(s_cacheMutex);
        auto it = s_cache.find(inst.Addr);
        if (it != s_cache.end() &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.ts).count() < 1500)
            return it->second;
    }
    CachedNode ec;
    ec.name = inst.GetName();
    if (ec.name.empty())
        ec.name = "Unnamed";
    ec.cls = inst.GetClass();
    auto cl = inst.GetChildList();
    ec.childAddrs.reserve(cl.size() > 200 ? 200 : cl.size());
    for (size_t i = 0; i < cl.size() && i < 200; i++)
        ec.childAddrs.push_back(cl[i].Addr);
    ec.hasChildren = !ec.childAddrs.empty();
    ec.ts = now;
    {
        std::lock_guard<std::mutex> lk(s_cacheMutex);
        s_cache[inst.Addr] = ec;
        if (s_cache.size() > 4000)
            s_cache.clear();
    }
    return ec;
}

struct SearchResult {
    uintptr_t addr = 0;
    std::string name;
    std::string className;
    std::string path;
};

std::vector<SearchResult> s_results;
std::mutex s_resultsMutex;
std::string s_lastQuery;
int s_lastFilter = -1;
std::atomic<bool> s_searchRunning{ false };
ULONGLONG s_lastTrigger = 0;

void TriggerSearch(const std::string& query, int filter_idx) {
    if (s_searchRunning)
        return;
    s_searchRunning = true;
    s_lastQuery = query;
    s_lastFilter = filter_idx;
    std::thread([query, filter_idx]() {
        std::string qLower = query;
        for (char& c : qLower)
            c = (char)tolower((unsigned char)c);
        std::vector<SearchResult> results;
        results.reserve(300);
        std::vector<uintptr_t> stack;
        if (Globals::dataModel.Addr)
            stack.push_back(Globals::dataModel.Addr);
        size_t visited = 0;
        while (!stack.empty() && results.size() < 2000 && visited < 100000) {
            uintptr_t addr = stack.back();
            stack.pop_back();
            visited++;
            RBX::RbxInstance inst(addr);
            CachedNode ec = GetCached(inst);
            if (addr != Globals::dataModel.Addr) {
                bool matches = true;
                if (!qLower.empty())
                    matches = FastContainsCI(ec.name, qLower) || FastContainsCI(ec.cls, qLower);
                if (matches && filter_idx != 0) {
                    if (filter_idx == 1 && ec.cls.find("Script") == std::string::npos)
                        matches = false;
                    else if (filter_idx == 2 && ec.cls.find("Part") == std::string::npos && ec.cls != "MeshPart")
                        matches = false;
                    else if (filter_idx == 3 && ec.cls != "Folder")
                        matches = false;
                    else if (filter_idx == 4 && ec.cls != "RemoteEvent" && ec.cls != "RemoteFunction" &&
                             ec.cls != "BindableEvent" && ec.cls != "BindableFunction")
                        matches = false;
                }
                if (matches) {
                    SearchResult res;
                    res.addr = addr;
                    res.name = ec.name;
                    res.className = ec.cls;
                    res.path = GetInstancePath(inst);
                    results.push_back(std::move(res));
                }
            }
            for (auto it = ec.childAddrs.rbegin(); it != ec.childAddrs.rend(); ++it)
                stack.push_back(*it);
            if (addr == Globals::dataModel.Addr && Globals::workspace.Addr) {
                
                std::vector<uintptr_t> rest;
                std::vector<uintptr_t> wsv;
                rest.reserve(stack.size());
                while (!stack.empty()) {
                    uintptr_t a = stack.back();
                    stack.pop_back();
                    if (a == Globals::workspace.Addr)
                        wsv.push_back(a);
                    else
                        rest.push_back(a);
                }
                for (auto a : wsv)
                    stack.push_back(a);
                for (auto it = rest.rbegin(); it != rest.rend(); ++it)
                    stack.push_back(*it);
            }
        }
        {
            std::lock_guard<std::mutex> lk(s_resultsMutex);
            s_results = std::move(results);
        }
        s_searchRunning = false;
    }).detach();
}

int g_rendered = 0;

std::atomic<bool> s_dumping{ false };

void DumpAll() {
    if (s_dumping.exchange(true))
        return;
    std::thread([]() {
        char* up = nullptr;
        size_t len = 0;
        _dupenv_s(&up, &len, "USERPROFILE");
        std::string desk = up ? std::string(up) : std::string("C:\\Users\\Public");
        if (up)
            free(up);
        std::string dir = desk + "\\Desktop\\jatos dumps\\";
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        std::time_t t = std::time(nullptr);
        std::tm tm{};
        localtime_s(&tm, &t);
        char stamp[32];
        std::snprintf(stamp, sizeof(stamp), "dump_%04d%02d%02d_%02d%02d%02d.txt",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        std::string path = dir + stamp;
        FILE* f = nullptr;
        fopen_s(&f, path.c_str(), "w");
        if (!f) {
            s_dumping.store(false);
            return;
        }
        const auto root = Globals::dataModel.Addr;
        std::string rn = RBX::RbxInstance(root).GetName();
        std::string rc = RBX::RbxInstance(root).GetClass();
        std::fprintf(f, "=== ROBLOX INSTANCE DUMP ===\nRoot Address: 0x%llX\nRoot Name: %s\nRoot Class: %s\n\n",
            (unsigned long long)root, rn.c_str(), rc.c_str());
        struct Item {
            std::uintptr_t addr;
            int depth;
        };
        std::vector<Item> stack;
        if (root)
            stack.push_back({ root, 0 });
        std::size_t count = 0;
        const std::size_t kCap = 300000;
        char line[512];
        while (!stack.empty() && count < kCap) {
            Item cur = stack.back();
            stack.pop_back();
            count++;
            RBX::RbxInstance inst(cur.addr);
            std::string name = inst.GetName();
            std::string cls = inst.GetClass();
            if (name.size() > 120)
                name.resize(120);
            if (cls.size() > 64)
                cls.resize(64);
            std::string pad((std::size_t)(cur.depth > 12 ? 12 : cur.depth) * 2, ' ');
            std::snprintf(line, sizeof(line), "%s- [%s] %s (0x%llX)\n", pad.c_str(),
                cls.c_str(), name.c_str(), (unsigned long long)cur.addr);
            std::fputs(line, f);
            auto kids = inst.GetChildList();
            for (auto it = kids.rbegin(); it != kids.rend(); ++it) {
                if (it->Addr && count + stack.size() < kCap)
                    stack.push_back({ it->Addr, cur.depth + 1 });
            }
        }
        std::fprintf(f, "\nTotal: %zu instances\n", count);
        std::fclose(f);
        printf("[Explorer] dumped %zu instances to %s\n", count, path.c_str());
        s_dumping.store(false);
    }).detach();
}

void DrawNode(RBX::RbxInstance inst, RBX::RbxInstance& selected, float iconSize) {
    if (inst.Addr == 0 || g_rendered > 1500)
        return;
    if (ImGui::GetCurrentWindow() && ImGui::GetCurrentWindow()->SkipItems)
        return;
    CachedNode ec = GetCached(inst);
    if (!g_texturesLoaded)
        LoadAllIcons();
    IconTex* icon = GetIcon(ec.cls);
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                               ImGuiTreeNodeFlags_SpanAvailWidth;
    if (selected.Addr == inst.Addr)
        flags |= ImGuiTreeNodeFlags_Selected;
    if (!ec.hasChildren)
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    char buf[256];
    snprintf(buf, sizeof(buf), "%s [%s]", ec.name.c_str(), ec.cls.c_str());
    bool open = ImGui::TreeNodeEx((void*)(intptr_t)inst.Addr, flags, "%s", "");
    if (ImGui::IsItemClicked())
        selected = inst;
    if (icon && icon->tex) {
        ImGui::SameLine(0, 0.0f);
        ImGui::Image((ImTextureID)icon->tex, ImVec2(iconSize, iconSize));
        ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x);
    }
    ImGui::TextUnformatted(buf);
    g_rendered++;
    if (open) {
        const size_t limit = 300;
        const size_t total = ec.childAddrs.size();
        const size_t shown = total > limit ? limit : total;
        for (size_t i = 0; i < shown; i++)
            DrawNode(RBX::RbxInstance(ec.childAddrs[i]), selected, iconSize);
        if (total > limit) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 24.0f);
            ImGui::TextDisabled("... %zu more (use search)", total - limit);
        }
        if (ec.hasChildren)
            ImGui::TreePop();
    }
}

void ThemedInput(const char* id, char* buf, std::size_t cap, const ImVec2& size, const char* hint = nullptr) {
    const imGuiCustom::Theme& theme = imGuiCustom::GetTheme();
    ImGui::PushID(id);
    ImGui::PushItemWidth(size.x);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, imGuiCustom::ColorU32(theme.ControlBg));
    ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(theme.TextBright));
    ImGui::PushStyleColor(ImGuiCol_Border, imGuiCustom::ColorU32(ImVec4(0, 0, 0, 0)));
    if (hint)
        ImGui::InputTextWithHint("##in", hint, buf, cap);
    else
        ImGui::InputText("##in", buf, cap);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::PopItemWidth();
    ImDrawList* idl = ImGui::GetWindowDrawList();
    const ImVec2 bmin = ImGui::GetItemRectMin(), bmax = ImGui::GetItemRectMax();
    idl->AddRect(bmin, bmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    idl->AddRect(bmin + ImVec2(1, 1), bmax - ImVec2(1, 1), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
    ImGui::PopID();
}

} 

void SetOpen(bool open) {
    g_open = open;
}

bool IsOpen() {
    return g_open;
}

void RenderWindow(ID3D11Device* device) {
    if (!g_open)
        return;
    if (device)
        g_device = device;
    const imGuiCustom::Theme& theme = imGuiCustom::GetTheme();
    ImFont* font = imGuiCustom::GetFonts().CascadiaMonoBL
        ? imGuiCustom::GetFonts().CascadiaMonoBL
        : ImGui::GetFont();
    const float fs = 12.f * imGuiCustom::g_fontScale;

    ImGui::SetNextWindowSize(ImVec2(640.f, 480.f), ImGuiCond_FirstUseEver);
    {
        ImVec2 pos(700.f, 80.f);
        ImGuiWindow* mainW = ImGui::FindWindowByName("jatos");
        if (mainW) {
            pos.x = mainW->Pos.x + mainW->Size.x + 20.f;
            pos.y = mainW->Pos.y;
            const ImVec2 disp = ImGui::GetIO().DisplaySize;
            if (pos.x + 640.f > disp.x)
                pos.x = disp.x - 640.f - 10.f;
            if (pos.y + 480.f > disp.y)
                pos.y = disp.y - 480.f - 10.f;
            if (pos.x < 0.f)
                pos.x = 10.f;
            if (pos.y < 0.f)
                pos.y = 10.f;
        }
        ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
    }
    if (!ImGui::Begin("##explorer", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::End();
        return;
    }
    const ImVec2 origin = ImVec2(std::floor(ImGui::GetWindowPos().x), std::floor(ImGui::GetWindowPos().y));
    const ImVec2 winMax = origin + ImVec2(640.f, 480.f);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(origin, winMax, imGuiCustom::ColorU32(theme.WindowBg), 0.0f);
    draw->AddRect(origin, winMax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    draw->AddRect(origin + ImVec2(1.f, 1.f), winMax - ImVec2(1.f, 1.f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
    draw->AddRectFilled(origin, origin + ImVec2(640.f, 1.5f),
        imGuiCustom::ColorU32(ImVec4(0.5373f, 0.7647f, 0.7490f, 1.0f)), 0.0f);
    const ImVec2 title_sz = font->CalcTextSizeA(fs, FLT_MAX, 0.f, "Explorer");
    draw->AddText(font, fs, origin + ImVec2(std::floor((640.f - title_sz.x) * 0.5f), 3.5f),
        imGuiCustom::ColorU32(theme.TextBright), "Explorer");

    ImGui::PushFont(font);
    static RBX::RbxInstance selected(0);
    static char search_buf[64] = "";
    static int filter_idx = 0;
    const char* filters[] = { "All", "Scripts", "Parts", "Folders", "Remotes" };

    {
        ImDrawList* capdl = ImGui::GetWindowDrawList();
        ImFont* capfont = imGuiCustom::GetFonts().CascadiaMonoBL ? imGuiCustom::GetFonts().CascadiaMonoBL : ImGui::GetFont();
        float capfs = 12.f * imGuiCustom::g_fontScale;
        capdl->AddText(capfont, capfs, origin + ImVec2(12.f, 28.f),
            imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright), "Search:");
        capdl->AddText(capfont, capfs, origin + ImVec2(322.f, 28.f),
            imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright), "Filter:");
    }
    ImGui::SetCursorScreenPos(origin + ImVec2(12.f, 44.f));
    ThemedInput("exp_search", search_buf, sizeof(search_buf), ImVec2(300.f, 0), "Search");
    {
        ImVec2 bpos = origin + ImVec2(322.f, 44.f);
        ImGui::SetCursorScreenPos(bpos);
        ImVec2 wpos = ImGui::GetWindowPos();
        ImVec2 rel = ImVec2(bpos.x - wpos.x, bpos.y - wpos.y);
        imGuiCustom::Combo("exp_filter", &filter_idx, filters, 5, rel, 180.f, "");
        ImGui::SetCursorScreenPos(ImVec2(bpos.x, bpos.y + imGuiCustom::ComboStep()));
    }

    const float listTop = 68.f;
    const float listH = 480.f - listTop - 12.f;
    const float colW = (640.f - 24.f - 8.f) * 0.5f;

    bool isSearching = (search_buf[0] != '\0' || filter_idx != 0);
    if (isSearching) {
        ULONGLONG now = GetTickCount64();
        if (s_lastQuery != search_buf || s_lastFilter != filter_idx) {
            if (now - s_lastTrigger > 100) {
                s_lastTrigger = now;
                TriggerSearch(search_buf, filter_idx);
            }
        }
    }

    ImGui::SetCursorScreenPos(origin + ImVec2(12.f, listTop));
    if (ImGui::BeginChild("exp_tree", ImVec2(colW, listH), false, ImGuiWindowFlags_NoScrollbar)) {
        if (isSearching) {
            std::vector<SearchResult> copy;
            {
                std::lock_guard<std::mutex> lk(s_resultsMutex);
                copy = s_results;
            }
            if (s_searchRunning && copy.empty()) {
                ImGui::TextDisabled("Searching instances...");
            } else if (copy.empty()) {
                ImGui::TextDisabled("No matching instances found.");
            } else {
                ImGui::TextDisabled("%zu results found", copy.size());
                ImGuiListClipper clipper;
                clipper.Begin((int)copy.size());
                while (clipper.Step()) {
                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                        auto& item = copy[i];
                        ImGui::PushID(i);
                        bool sel = (selected.Addr == item.addr);
                        float itemH = ImGui::GetTextLineHeightWithSpacing() + 16.f;
                        ImVec2 cur = ImGui::GetCursorScreenPos();
                        bool clicked = ImGui::Selectable("##res", sel,
                            ImGuiSelectableFlags_SpanAvailWidth, ImVec2(0, itemH));
                        if (clicked)
                            selected = RBX::RbxInstance(item.addr);
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        IconTex* icon = GetIcon(item.className);
                        float iconSize = ImGui::GetFontSize();
                        if (icon && icon->tex)
                            dl->AddImage((ImTextureID)icon->tex, cur + ImVec2(4.f, 4.f),
                                cur + ImVec2(4.f + iconSize, 4.f + iconSize));
                        char topBuf[256];
                        snprintf(topBuf, sizeof(topBuf), "%s [%s]", item.name.c_str(), item.className.c_str());
                        dl->AddText(cur + ImVec2(iconSize + 10.f, 2.f),
                            imGuiCustom::ColorU32(theme.TextBright), topBuf);
                        dl->AddText(cur + ImVec2(iconSize + 10.f, ImGui::GetFontSize() + 4.f),
                            imGuiCustom::ColorU32(theme.Text), item.path.c_str());
                        ImGui::PopID();
                    }
                }
            }
        } else if (Globals::dataModel.Addr != 0) {
            g_rendered = 0;
            DrawNode(Globals::dataModel, selected, ImGui::GetFontSize());
        } else {
            ImGui::TextDisabled("DataModel not initialized");
        }
    }
    ImGui::EndChild();

    ImGui::SetCursorScreenPos(origin + ImVec2(12.f + colW + 8.f, listTop));
    if (ImGui::BeginChild("exp_props", ImVec2(colW, listH), false, ImGuiWindowFlags_NoScrollbar)) {
        auto row = [&](const char* name, const char* value) {
            ImGui::TextUnformatted(name);
            ImGui::SameLine(110.f);
            ImGui::TextColored(theme.TextBright, "%s", value);
        };
        ImGui::TextDisabled("Information");
        if (selected.Addr != 0) {
            char addr_buf[32], child_buf[16];
            snprintf(addr_buf, sizeof(addr_buf), "0x%llX", (unsigned long long)selected.Addr);
            snprintf(child_buf, sizeof(child_buf), "%d", (int)selected.GetChildList().size());
            row("Path", GetInstancePath(selected).c_str());
            row("Name", selected.GetName().c_str());
            row("Class", selected.GetClass().c_str());
            std::string pn = selected.GetParent().GetName();
            row("Parent", pn.empty() ? "None" : pn.c_str());
            row("Address", addr_buf);
            row("Children", child_buf);
        } else {
            row("Path", "None selected");
            row("Name", "None");
            row("Class", "None");
            row("Parent", "None");
            row("Address", "0x0");
            row("Children", "0");
        }
        ImGui::Spacing();
        if (selected.Addr != 0) {
            const std::string selCls = selected.GetClass();
            const std::uint64_t va = selected.Addr + Offsets::Misc::Value;
            ImGui::TextDisabled("Value");
            
            auto themedSlider = [&](const char* id, float* fv, float lo, float hi, const char* label, const char* fmt) {
                ImVec2 wpos = ImGui::GetWindowPos();
                ImVec2 cur = ImGui::GetCursorScreenPos();
                cur.y += imGuiCustom::SliderTop();
                ImVec2 rel(cur.x - wpos.x, cur.y - wpos.y);
                float w = ImGui::GetContentRegionAvail().x;
                if (w < 50.f)
                    w = 50.f;
                bool changed = imGuiCustom::SliderFloat(id, fv, lo, hi, rel, w, label, fmt);
                ImGui::SetCursorScreenPos(ImVec2(cur.x, cur.y + 15.0f));
                return changed;
            };
            if (selCls == "NumberValue") {
                static std::uint64_t lastNum = 0;
                static double numV = 0;
                static float numF = 0, numLo = -1, numHi = 1;
                static bool numActive = false;
                const bool wasActive = numActive;
                if (lastNum != selected.Addr) {
                    lastNum = selected.Addr;
                    numV = memory->read<double>(va);
                    numF = (float)numV;
                    double span = std::fabs(numV) * 2.0;
                    if (span < 1.0)
                        span = 1.0;
                    numLo = (float)(numV < 0.0 ? numV : 0.0) - (float)span;
                    numHi = (float)(numV > 0.0 ? numV : 0.0) + (float)span;
                } else if (!wasActive) {
                    numV = memory->read<double>(va);
                    if (numV < numLo || numV > numHi) {
                        double span = std::fabs(numV) * 2.0;
                        if (span < 1.0)
                            span = 1.0;
                        numLo = (float)(numV < 0.0 ? numV : 0.0) - (float)span;
                        numHi = (float)(numV > 0.0 ? numV : 0.0) + (float)span;
                    }
                    numF = (float)numV;
                }
                if (themedSlider("exp_num", &numF, numLo, numHi, "Value", "%.4f")) {
                    numV = (double)numF;
                    memory->write<double>(va, numV);
                }
                numActive = ImGui::IsItemActive();
            } else if (selCls == "IntValue") {
                static std::uint64_t lastInt = 0;
                static int intV = 0;
                static float intF = 0, intLo = -10, intHi = 10;
                if (lastInt != selected.Addr) {
                    lastInt = selected.Addr;
                    intV = memory->read<int>(va);
                    intF = (float)intV;
                    float span = std::fabs((float)intV) * 2.0f;
                    if (span < 10.0f)
                        span = 10.0f;
                    intLo = ((float)intV < 0.0f ? (float)intV : 0.0f) - span;
                    intHi = ((float)intV > 0.0f ? (float)intV : 0.0f) + span;
                } else if (!ImGui::IsItemActive()) {
                    intV = memory->read<int>(va);
                    intF = (float)intV;
                }
                if (themedSlider("exp_int", &intF, intLo, intHi, "Value", "%.0f")) {
                    intV = (int)intF;
                    memory->write<int>(va, intV);
                }
            } else if (selCls == "BoolValue") {
                bool bV = memory->read<std::uint8_t>(va) != 0;
                ImVec2 wpos = ImGui::GetWindowPos();
                ImVec2 cur = ImGui::GetCursorScreenPos();
                ImVec2 rel(cur.x - wpos.x, cur.y - wpos.y);
                imGuiCustom::Checkbox("Value", &bV, rel);
                ImGui::SetCursorScreenPos(ImVec2(cur.x, cur.y + imGuiCustom::CheckStep()));
                memory->write<std::uint8_t>(va, bV ? 1 : 0);
            }
            ImGui::Spacing();
        }
        auto copyBtn = [&](const char* id, const char* label, const ImVec2& pos, const ImVec2& size, const std::string& v) {
            ImGui::PushID(id);
            ImGui::SetCursorScreenPos(pos);
            ImGui::PushStyleColor(ImGuiCol_Button, imGuiCustom::ColorU32(theme.ControlBg));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imGuiCustom::ColorU32(theme.ControlInactive));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, imGuiCustom::ColorU32(theme.ControlInactive));
            ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(theme.TextBright));
            bool r = ImGui::Button(label, size);
            ImDrawList* bdl = ImGui::GetWindowDrawList();
            const ImVec2 bmin = ImGui::GetItemRectMin(), bmax = ImGui::GetItemRectMax();
            bdl->AddRect(bmin, bmax, imGuiCustom::OutlineBlack(), 0.f, 0, 1.f);
            bdl->AddRect(bmin + ImVec2(1, 1), bmax - ImVec2(1, 1), imGuiCustom::OutlineInner(), 0.f, 0, 1.f);
            ImGui::PopStyleColor(4);
            if (r && !v.empty())
                ImGui::SetClipboardText(v.c_str());
            ImGui::PopID();
        };
        {
            ImVec2 pcmin = ImGui::GetWindowPos();
            float px = pcmin.x + ImGui::GetCursorPosX();
            float py = ImGui::GetCursorScreenPos().y;
            float pw = ImGui::GetContentRegionAvail().x;
            float bw = (pw - 6.f) * 0.5f;
            if (selected.Addr != 0) {
                char addr_buf[32];
                snprintf(addr_buf, sizeof(addr_buf), "0x%llX", (unsigned long long)selected.Addr);
                copyBtn("cp_name", "Copy name", ImVec2(px, py), ImVec2(bw, 20.f), selected.GetName());
                copyBtn("cp_class", "Copy Classname", ImVec2(px + bw + 6.f, py), ImVec2(bw, 20.f), selected.GetClass());
                py += 26.f;
                copyBtn("cp_path", "Copy Path", ImVec2(px, py), ImVec2(bw, 20.f), GetInstancePath(selected));
                copyBtn("cp_addr", "Copy Address", ImVec2(px + bw + 6.f, py), ImVec2(bw, 20.f), addr_buf);
                py += 26.f;
            }
            bool doDump = false;
            ImGui::PushID("exp_dump");
            ImGui::SetCursorScreenPos(ImVec2(px, py));
            ImGui::PushStyleColor(ImGuiCol_Button, imGuiCustom::ColorU32(theme.ControlBg));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imGuiCustom::ColorU32(theme.ControlInactive));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, imGuiCustom::ColorU32(theme.ControlInactive));
            ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(theme.TextBright));
            doDump = ImGui::Button(s_dumping.load() ? "Dumping..." : "Dump", ImVec2(pw, 20.f));
            ImDrawList* ddl = ImGui::GetWindowDrawList();
            const ImVec2 dmin = ImGui::GetItemRectMin(), dmax = ImGui::GetItemRectMax();
            ddl->AddRect(dmin, dmax, imGuiCustom::OutlineBlack(), 0.f, 0, 1.f);
            ddl->AddRect(dmin + ImVec2(1, 1), dmax - ImVec2(1, 1), imGuiCustom::OutlineInner(), 0.f, 0, 1.f);
            ImGui::PopStyleColor(4);
            ImGui::PopID();
            if (doDump)
                DumpAll();
            ImGui::SetCursorScreenPos(ImVec2(px, py + 26.f));
            ImGui::Dummy(ImVec2(1, 1));
        }
    }
    ImGui::EndChild();
    ImGui::PopFont();
    ImGui::End();
}

} 
