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
#include <winhttp.h>
#include <algorithm>
#pragma comment(lib, "winhttp.lib")
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
        s_dumping.store(false);
    }).detach();
}

std::atomic<bool> s_imgDumping{ false };
std::atomic<int> s_imgDone{ 0 };
std::atomic<int> s_imgTotal{ 0 };

std::string ReadImgStr(std::uintptr_t addr, std::uintptr_t off) {
    if (!addr) return {};
    for (std::uintptr_t o : {off, off + 8, off - 8, off + 0x10, off - 0x10}) {
        std::string s = memory->read_string(addr + o);
        if (!s.empty() && s.find("rbxassetid://") != std::string::npos) return s;
        std::uintptr_t ptr = memory->read<std::uintptr_t>(addr + o);
        if (ptr) {
            s = memory->read_string(ptr);
            if (!s.empty() && s.find("rbxassetid://") != std::string::npos) return s;
        }
    }
    return {};
}

std::string ImgHttpGet(const std::wstring& host, const std::wstring& path) {
    std::string out;
    HINTERNET hS = WinHttpOpen(L"jatos/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
    if (!hS) return out;
    HINTERNET hC = WinHttpConnect(hS, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hC) { WinHttpCloseHandle(hS); return out; }
    HINTERNET hR = WinHttpOpenRequest(hC, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hR) { WinHttpCloseHandle(hC); WinHttpCloseHandle(hS); return out; }
    if (!WinHttpSendRequest(hR, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) { WinHttpCloseHandle(hR); WinHttpCloseHandle(hC); WinHttpCloseHandle(hS); return out; }
    if (!WinHttpReceiveResponse(hR, nullptr)) { WinHttpCloseHandle(hR); WinHttpCloseHandle(hC); WinHttpCloseHandle(hS); return out; }
    std::vector<char> buf;
    for (;;) {
        DWORD avail = 0;
        if (!WinHttpQueryDataAvailable(hR, &avail)) break;
        if (!avail) break;
        size_t old = buf.size();
        buf.resize(old + avail);
        DWORD rd = 0;
        if (!WinHttpReadData(hR, buf.data() + old, avail, &rd)) break;
        buf.resize(old + rd);
        if (!rd) break;
    }
    out.assign(buf.begin(), buf.end());
    WinHttpCloseHandle(hR); WinHttpCloseHandle(hC); WinHttpCloseHandle(hS);
    return out;
}

std::string ImgAssetUrl(const std::string& json) {
    auto p = json.find("imageUrl");
    if (p == std::string::npos) return {};
    auto q = json.find("http", p);
    if (q == std::string::npos) return {};
    auto e = json.find('"', q);
    if (e == std::string::npos) e = json.size();
    std::string url = json.substr(q, e - q);
    size_t pos = url.find("\\u0026");
    while (pos != std::string::npos) { url.replace(pos, 6, "&"); pos = url.find("\\u0026", pos + 1); }
    return url;
}

void DumpImages() {
    if (s_imgDumping.exchange(true))
        return;
    std::thread([]() {
        char* up = nullptr;
        size_t len = 0;
        _dupenv_s(&up, &len, "USERPROFILE");
        std::string desk = up ? std::string(up) : std::string("C:\\Users\\Public");
        if (up) free(up);
        std::string dir = desk + "\\Desktop\\fallenimages\\";
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        struct Found { std::string id; std::string name; };
        std::vector<Found> items;
        std::unordered_map<std::string, bool> seen;
        const auto root = Globals::dataModel.Addr;
        std::vector<std::uintptr_t> stack;
        if (root) stack.push_back(root);
        std::size_t vis = 0;
        std::size_t labelCount = 0;
        std::size_t emptyCount = 0;
        const std::size_t kCap = 300000;
        while (!stack.empty() && vis < kCap && items.size() < 5000) {
            std::uintptr_t addr = stack.back();
            stack.pop_back();
            vis++;
            RBX::RbxInstance inst(addr);
            std::string cls = inst.GetClass();
            std::string asset;
            if (cls == "ImageLabel" || cls == "ImageButton" || cls == "Decal" || cls == "Texture") {
                labelCount++;
                if (cls == "Decal" || cls == "Texture")
                    asset = ReadImgStr(addr, 0x1E0);
                else
                    asset = ReadImgStr(addr, 0x988);
                if (asset.empty()) emptyCount++;
            }
            if (!asset.empty()) {
                std::string id = asset;
                if (id.rfind("rbxassetid://", 0) == 0) id = id.substr(13);
                auto at = id.find_first_not_of("0123456789");
                if (at != std::string::npos) id = id.substr(0, at);
                if (!id.empty() && !seen[id]) {
                    seen[id] = true;
                    std::string nm = inst.GetName();
                    if (nm.empty()) nm = cls;
                    items.push_back({id, nm});
                }
            }
            auto kids = inst.GetChildList();
            for (auto it = kids.rbegin(); it != kids.rend(); ++it) {
                if (it->Addr && vis + stack.size() < kCap)
                    stack.push_back(it->Addr);
            }
        }
        (void)vis; (void)labelCount; (void)emptyCount;
        s_imgTotal.store((int)items.size());
        s_imgDone.store(0);
        FILE* idx = nullptr;
        fopen_s(&idx, (dir + "images_index.txt").c_str(), "w");
        int done = 0, fail = 0;
        std::unordered_map<std::string, int> nameCount;
        auto assetState = [](const std::string& json) -> std::string {
            auto p = json.find("\"state\"");
            if (p == std::string::npos) return "no-state";
            auto q = json.find('"', p + 7);
            if (q == std::string::npos) return "bad-state";
            auto e = json.find('"', q + 1);
            if (e == std::string::npos) return "bad-state";
            return json.substr(q + 1, e - q - 1);
        };
        for (auto& it : items) {
            std::wstring q = L"/v1/assets?assetIds=" + std::wstring(it.id.begin(), it.id.end()) + L"&size=420x420&format=Png";
            std::string json = ImgHttpGet(L"thumbnails.roblox.com", q);
            std::string imgUrl = ImgAssetUrl(json);
            if (imgUrl.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                json = ImgHttpGet(L"thumbnails.roblox.com", q);
                imgUrl = ImgAssetUrl(json);
            }
            if (imgUrl.empty()) {
                fail++;
                if (idx) fprintf(idx, "FAIL %s %s no-url state=%s jsonlen=%zu\n", it.id.c_str(), it.name.c_str(), assetState(json).c_str(), json.size());
                s_imgDone.store(done + fail);
                continue;
            }
            std::string tmp = imgUrl;
            if (tmp.rfind("https://", 0) == 0) tmp = tmp.substr(8);
            else if (tmp.rfind("http://", 0) == 0) tmp = tmp.substr(7);
            std::string host, path;
            auto sl = tmp.find('/');
            if (sl != std::string::npos) { host = tmp.substr(0, sl); path = tmp.substr(sl); }
            else { host = tmp; path = "/"; }
            std::string data = ImgHttpGet(std::wstring(host.begin(), host.end()), std::wstring(path.begin(), path.end()));
            if (data.size() < 100) {
                fail++;
                if (idx) fprintf(idx, "FAIL %s %s no-data bytes=%zu\n", it.id.c_str(), it.name.c_str(), data.size());
                s_imgDone.store(done + fail);
                continue;
            }
            std::string safe;
            for (char c : it.name) {
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-') safe.push_back(c);
                else if (c == ' ') safe.push_back('_');
                if (safe.size() >= 48) break;
            }
            if (safe.empty()) safe = "image";
            int n = nameCount[safe + "_" + it.id]++;
            std::string fname = safe + "_" + it.id + (n ? "_" + std::to_string(n) : "") + ".png";
            FILE* f = nullptr;
            fopen_s(&f, (dir + fname).c_str(), "wb");
            if (f) {
                fwrite(data.data(), 1, data.size(), f); fclose(f); done++;
                if (idx) fprintf(idx, "OK %s %s %s\n", it.id.c_str(), it.name.c_str(), fname.c_str());
            } else {
                fail++;
                if (idx) fprintf(idx, "FAIL %s %s save-fail\n", it.id.c_str(), it.name.c_str());
            }
            s_imgDone.store(done + fail);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        if (idx) fclose(idx);
        s_imgDumping.store(false);
    }).detach();
}

struct DecompiledScript {
    std::string name;
    std::string text;
    bool open = true;
};
std::vector<DecompiledScript> s_scripts;
std::mutex s_scriptsMutex;

std::vector<uint8_t> ReadScriptBytes(std::uintptr_t scriptAddr) {
    std::vector<uint8_t> out;
    if (!scriptAddr) return out;
    const std::uintptr_t bc = memory->read<std::uintptr_t>(scriptAddr + Offsets::LocalScript::ByteCode);
    if (!bc) return out;
    const std::uintptr_t ptr = memory->read<std::uintptr_t>(bc + Offsets::ByteCode::Pointer);
    const std::uint32_t sz = memory->read<std::uint32_t>(bc + Offsets::ByteCode::Size);
    if (!ptr || sz == 0 || sz > 8 * 1024 * 1024) return out;
    out.resize(sz);
    if (!memory->read_raw(ptr, out.data(), sz)) out.clear();
    return out;
}

std::string HexDisassemble(const std::vector<uint8_t>& bytes, const std::string& name) {
    std::string s;
    s.reserve(bytes.size() * 3 + 256);
    char h[128];
    std::snprintf(h, sizeof(h), "-- disassembly of %s (%zu bytes)\n-- raw luau bytecode hex dump\n\n", name.c_str(), bytes.size());
    s += h;
    for (size_t i = 0; i < bytes.size(); i += 16) {
        char off[16];
        std::snprintf(off, sizeof(off), "%08zX  ", i);
        s += off;
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < bytes.size()) {
                char b[4];
                std::snprintf(b, sizeof(b), "%02X ", bytes[i + j]);
                s += b;
            } else s += "   ";
        }
        s += " |";
        for (size_t j = 0; j < 16 && i + j < bytes.size(); ++j) {
            char c = (char)bytes[i + j];
            s += (c >= 32 && c < 127) ? c : '.';
        }
        s += "|\n";
        if (s.size() > 200000) { s += "\n-- truncated --\n"; break; }
    }
    return s;
}

void SaveAndShowScript(const std::string& name, const std::vector<uint8_t>& bytes, bool disassemble) {
    char* up = nullptr;
    size_t len = 0;
    _dupenv_s(&up, &len, "USERPROFILE");
    std::string dir = up ? std::string(up) : std::string("C:\\Users\\Public");
    if (up) free(up);
    dir += "\\Desktop\\jatos dumps\\scripts\\";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    std::string safe;
    for (char c : name) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-') safe.push_back(c);
        else if (c == ' ') safe.push_back('_');
        if (safe.size() >= 48) break;
    }
    if (safe.empty()) safe = "script";
    if (disassemble) {
        std::string txt = HexDisassemble(bytes, name);
        FILE* f = nullptr;
        fopen_s(&f, (dir + safe + ".dis.txt").c_str(), "w");
        if (f) { std::fwrite(txt.data(), 1, txt.size(), f); std::fclose(f); }
        std::lock_guard<std::mutex> lk(s_scriptsMutex);
        s_scripts.push_back({name + " [disassembly]", txt, true});
    } else {
        FILE* f = nullptr;
        fopen_s(&f, (dir + safe + ".luauc").c_str(), "wb");
        if (f) { std::fwrite(bytes.data(), 1, bytes.size(), f); std::fclose(f); }
        std::string txt = "-- decompile of " + name + " (" + std::to_string(bytes.size()) + " bytes)\n";
        txt += "-- NOTE: no luau decompiler backend linked; raw bytecode saved to jatos dumps\\scripts\\" + safe + ".luauc\n";
        txt += "-- printable strings found in bytecode:\n\n";
        std::string cur;
        for (uint8_t b : bytes) {
            if (b >= 32 && b < 127) cur.push_back((char)b);
            else {
                if (cur.size() >= 4) { txt += "\"" + cur + "\"\n"; }
                cur.clear();
            }
            if (txt.size() > 200000) { txt += "\n-- truncated --\n"; break; }
        }
        if (cur.size() >= 4) txt += "\"" + cur + "\"\n";
        std::lock_guard<std::mutex> lk(s_scriptsMutex);
        s_scripts.push_back({name + " [decompile]", txt, true});
    }
}

void RenderScriptWindows() {
    std::lock_guard<std::mutex> lk(s_scriptsMutex);
    for (auto& sc : s_scripts) {
        if (!sc.open) continue;
        ImGui::SetNextWindowSize(ImVec2(560, 420), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(sc.name.c_str(), &sc.open)) {
            ImGui::TextDisabled("%zu chars", sc.text.size());
            ImGui::BeginChild("##scripttxt", ImVec2(0, 0), true);
            ImGui::TextUnformatted(sc.text.c_str());
            ImGui::EndChild();
        }
        ImGui::End();
    }
    s_scripts.erase(std::remove_if(s_scripts.begin(), s_scripts.end(),
        [](const DecompiledScript& d) { return !d.open; }), s_scripts.end());
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

    ImGui::SetNextWindowSize(ImVec2(640.f, 480.f), ImGuiCond_Always);
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
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
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
            bool doImgs = false;
            ImGui::PushID("exp_images");
            ImGui::SetCursorScreenPos(ImVec2(px, py + 26.f));
            ImGui::PushStyleColor(ImGuiCol_Button, imGuiCustom::ColorU32(theme.ControlBg));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imGuiCustom::ColorU32(theme.ControlInactive));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, imGuiCustom::ColorU32(theme.ControlInactive));
            ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(theme.TextBright));
            char imgsLabel[32];
            if (s_imgDumping.load() && s_imgTotal.load() > 0)
                snprintf(imgsLabel, sizeof(imgsLabel), "Images %d/%d", s_imgDone.load(), s_imgTotal.load());
            else if (s_imgDumping.load())
                snprintf(imgsLabel, sizeof(imgsLabel), "Images...");
            else
                snprintf(imgsLabel, sizeof(imgsLabel), "Images");
            doImgs = ImGui::Button(imgsLabel, ImVec2(pw, 20.f));
            ImDrawList* idl = ImGui::GetWindowDrawList();
            const ImVec2 imin = ImGui::GetItemRectMin(), imax = ImGui::GetItemRectMax();
            idl->AddRect(imin, imax, imGuiCustom::OutlineBlack(), 0.f, 0, 1.f);
            idl->AddRect(imin + ImVec2(1, 1), imax - ImVec2(1, 1), imGuiCustom::OutlineInner(), 0.f, 0, 1.f);
            ImGui::PopStyleColor(4);
            ImGui::PopID();
            if (doDump)
                DumpAll();
            if (doImgs)
                DumpImages();
            {
                const std::string cls = selected.Addr ? selected.GetClass() : "";
                const bool isScript = cls.find("Script") != std::string::npos;
                bool doDecomp = false, doDis = false;
                ImGui::PushID("exp_decomp");
                ImGui::SetCursorScreenPos(ImVec2(px, py + 52.f));
                ImGui::PushStyleColor(ImGuiCol_Button, imGuiCustom::ColorU32(theme.ControlBg));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imGuiCustom::ColorU32(theme.ControlInactive));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, imGuiCustom::ColorU32(theme.ControlInactive));
                ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(theme.TextBright));
                ImGui::BeginDisabled(!isScript);
                doDecomp = ImGui::Button("Decompile", ImVec2(bw, 20.f));
                ImGui::EndDisabled();
                ImGui::PopStyleColor(4);
                ImGui::PopID();
                ImGui::PushID("exp_disas");
                ImGui::SetCursorScreenPos(ImVec2(px + bw + 6.f, py + 52.f));
                ImGui::PushStyleColor(ImGuiCol_Button, imGuiCustom::ColorU32(theme.ControlBg));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imGuiCustom::ColorU32(theme.ControlInactive));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, imGuiCustom::ColorU32(theme.ControlInactive));
                ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(theme.TextBright));
                ImGui::BeginDisabled(!isScript);
                doDis = ImGui::Button("Disassemble", ImVec2(bw, 20.f));
                ImGui::EndDisabled();
                ImGui::PopStyleColor(4);
                ImGui::PopID();
                if (!isScript) {
                    ImGui::SetCursorScreenPos(ImVec2(px, py + 78.f));
                    ImGui::TextDisabled("Select a Script for decompiler");
                }
                if ((doDecomp || doDis) && selected.Addr) {
                    std::string nm = selected.GetName();
                    if (nm.empty()) nm = cls;
                    std::vector<uint8_t> bytes = ReadScriptBytes(selected.Addr);
                    if (bytes.empty()) {
                        std::lock_guard<std::mutex> lk(s_scriptsMutex);
                        s_scripts.push_back({nm + (doDis ? " [disassembly]" : " [decompile]"),
                            "-- failed to read bytecode (empty or unreadable) --", true});
                    } else {
                        SaveAndShowScript(nm, bytes, doDis);
                    }
                }
            }
            RenderScriptWindows();
            ImGui::SetCursorScreenPos(ImVec2(px, py + 104.f));
            ImGui::Dummy(ImVec2(1, 1));
        }
    }
    ImGui::EndChild();
    ImGui::PopFont();
    ImGui::End();
}

}
