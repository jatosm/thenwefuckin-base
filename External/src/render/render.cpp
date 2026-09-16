#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "render.h"
#include "menu/CascadiaMonoBL.c"
#include <dwmapi.h>
#include <dxgi1_2.h>
#include <cmath>
#include <algorithm>
#include "menu/library.h"
#include "../../ext/imgui/imgui_impl_win32.h"
#include "../../ext/imgui/imgui_impl_dx11.h"
#include "../../src/core/variables/variables.h"
#include "../../src/core/functions/settings/settings.h"
#include "../../src/core/features/mesh/shader/MeshDxShader.h"
#include "../../src/core/functions/explorer/explorer.h"
#include "../../src/core/functions/mics/mics.h"
#include "../../src/core/functions/aim/aim.h"
#include "../../src/core/keys/keys.h"

#include "../../src/core/functions/visual/visual.h"
#include "../../src/core/cache/cache.h"
#include "../../src/core/net/ping.h"
#include "../../src/sdk/offsets.h"
#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static OverlayWindow* g_overlayWnd = nullptr;

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    if (msg == WM_SIZE) {
        if (g_overlayWnd && wParam != SIZE_MINIMIZED)
            g_overlayWnd->ResizeBuffers((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

OverlayWindow::OverlayWindow() : windowHandle(nullptr), d3dDevice(nullptr), d3dContext(nullptr), swapChain(nullptr), renderTarget(nullptr) {
    ZeroMemory(&windowClass, sizeof(windowClass));
}

void OverlayWindow::SetupD3D11(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL obtainedLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2, D3D11_SDK_VERSION, &sd, &swapChain, &d3dDevice, &obtainedLevel, &d3dContext);
    if (hr == DXGI_ERROR_UNSUPPORTED) {
        D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, levels, 2, D3D11_SDK_VERSION, &sd, &swapChain, &d3dDevice, &obtainedLevel, &d3dContext);
    }
    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (backBuffer) {
        d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
        backBuffer->Release();
    }
    IDXGIDevice1* dxgi1 = nullptr;
    if (SUCCEEDED(d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi1)) && dxgi1) {
        dxgi1->SetMaximumFrameLatency(1);
        dxgi1->Release();
    }
}

void OverlayWindow::ResizeBuffers(UINT w, UINT h) {
    if (!swapChain || !d3dDevice || w == 0 || h == 0)
        return;
    if (renderTarget) { renderTarget->Release(); renderTarget = nullptr; }
    if (FAILED(swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0)))
        return;
    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))) || !backBuffer)
        return;
    d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
    backBuffer->Release();
    Cheat::Visuals::MeshDxShader::Resize(w, h);
}

void OverlayWindow::CleanupD3D11() {
    Cheat::Visuals::MeshDxShader::Shutdown();
    if (renderTarget) { renderTarget->Release(); renderTarget = nullptr; }
    if (swapChain) { swapChain->Release(); swapChain = nullptr; }
    if (d3dContext) { d3dContext->Release(); d3dContext = nullptr; }
    if (d3dDevice) { d3dDevice->Release(); d3dDevice = nullptr; }
}

bool OverlayWindow::Initialize() {
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = OverlayWndProc;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.lpszClassName = L"jatos";
    if (!RegisterClassExW(&windowClass))
        return false;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    windowHandle = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW, windowClass.lpszClassName, L"jatos", WS_POPUP, 0, 0, screenW, screenH, nullptr, nullptr, windowClass.hInstance, nullptr);
    if (!windowHandle)
        return false;
    SetLayeredWindowAttributes(windowHandle, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(windowHandle, &margins);
    ShowWindow(windowHandle, SW_SHOW);
    UpdateWindow(windowHandle);
        SetupD3D11(windowHandle);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    font_cfg.OversampleH = 3;
    font_cfg.OversampleV = 3;
    font_cfg.PixelSnapH = true;
    ImFont* menuFont = io.Fonts->AddFontFromMemoryTTF((void*)CascadiaMonoBL, (int)sizeof(CascadiaMonoBL), 12.0f, &font_cfg);
    io.FontGlobalScale = 1.0f;
    imGuiCustom::Initialize(menuFont);
    ImGui_ImplWin32_Init(windowHandle);
    ImGui_ImplDX11_Init(d3dDevice, d3dContext);
    Cheat::Visuals::MeshDxShader::Init(d3dDevice, d3dContext);
    Cheat::Visuals::MeshDxShader::Resize((unsigned)screenW, (unsigned)screenH);
    g_overlayWnd = this;
    return true;
}

void OverlayWindow::BeginFrame() {
    MSG msg;
    imGuiCustom::g_fontScale = variables::Misc::menuFontSize;
    imGuiCustom::Theme& theme = imGuiCustom::GetThemeMutable();
    theme.WindowBg = variables::Theme::background;
    theme.CardBg = variables::Theme::panels;
    theme.ControlBg = variables::Theme::controls;
    theme.ControlInactive = imGuiCustom::LerpColor(variables::Theme::controls, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 0.12f);
    theme.Accent = variables::Theme::accent;
    theme.AccentText = variables::Theme::accent;
    theme.Text = variables::Theme::text;
    theme.TextBright = variables::Theme::textBright;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (variables::menuOpen) {
        SetWindowLong(windowHandle, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
    } else {
        SetWindowLong(windowHandle, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
    }
        if (Keys::StreamProofOn()) {
        SetWindowDisplayAffinity(windowHandle, WDA_EXCLUDEFROMCAPTURE);
    } else {
        SetWindowDisplayAffinity(windowHandle, WDA_NONE);
    }
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

namespace {
constexpr const char* kTabs[] = {"Aim", "Visual", "Mics", "Settings"};

void DrawPanel(const char* id, const ImVec2& pos, const ImVec2& size) {
    ImGui::PushID(id);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 winPos = ImGui::GetWindowPos();
    const ImVec2 min = ImVec2(std::floor(winPos.x + pos.x + imGuiCustom::g_contentOffset.x), std::floor(winPos.y + pos.y + imGuiCustom::g_contentOffset.y));
    const ImVec2 max = ImVec2(min.x + std::floor(size.x), min.y + std::floor(size.y));
    draw->AddRectFilled(min, max, imGuiCustom::ColorU32(imGuiCustom::GetTheme().CardBg), 0.0f);
    draw->AddRect(min, max, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    draw->AddRect(min + ImVec2(1.0f, 1.0f), max - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
    ImGui::PopID();
}

bool DrawTab(const char* label, bool active, const ImVec2& pos, const ImVec2& size) {
    const ImVec2 winPos = ImGui::GetWindowPos();
    const ImVec2 min = ImVec2(std::floor(winPos.x + pos.x), std::floor(winPos.y + pos.y));
    const ImVec2 max = ImVec2(min.x + std::floor(size.x), min.y + std::floor(size.y));
    ImGui::PushID(label);
    ImGui::SetCursorScreenPos(min);
    const bool pressed = ImGui::InvisibleButton(label, size);
    const bool hovered = ImGui::IsItemHovered();
    const ImGuiID id = ImGui::GetItemID();
    const float selected = imGuiCustom::AnimateFloat(id, active, 14.0f);
    const float hover = imGuiCustom::AnimateFloat(id + 1, hovered, 14.0f);
    const imGuiCustom::Theme& theme = imGuiCustom::GetTheme();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec4 current_bg = imGuiCustom::LerpColor(imGuiCustom::LerpColor(theme.WindowBg, imGuiCustom::LerpColor(theme.WindowBg, theme.CardBg, 0.45f), hover), theme.CardBg, selected);
    draw->AddRectFilled(min, max, imGuiCustom::ColorU32(current_bg), 0.0f);
    draw->AddRect(min, max, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    draw->AddRect(min + ImVec2(1.0f, 1.0f), max - ImVec2(1.0f, 1.0f), imGuiCustom::ColorU32(imGuiCustom::LerpColor(ImVec4(52.0f / 255.0f, 52.0f / 255.0f, 56.0f / 255.0f, 1.0f), ImVec4(72.0f / 255.0f, 72.0f / 255.0f, 78.0f / 255.0f, 1.0f), selected)), 0.0f, 0, 1.0f);
    const imGuiCustom::Fonts& fonts = imGuiCustom::GetFonts();
    ImFont* font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
    const float font_size = 12.5f * imGuiCustom::g_fontScale;
    const ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, label);
    draw->AddText(font, font_size, ImVec2(std::floor(min.x + (size.x - text_size.x) * 0.5f), std::floor(min.y + (size.y - text_size.y) * 0.5f)), imGuiCustom::ColorU32(imGuiCustom::LerpColor(theme.Text, theme.TextBright, selected)), label);
    ImGui::PopID();
    return pressed;
}
}

void OverlayWindow::RenderMenu() {
    float dt = ImGui::GetIO().DeltaTime;
    if (dt <= 0.0f || dt > 0.1f)
        dt = 0.016f;
    static float menuFade = 0.0f;
    static float tabAnim = 1.0f;
    static int prevTab = 0;
    static float slideDir = 1.0f;
    if (variables::menuOpen) {
        menuFade += dt * 8.0f;
        if (menuFade > 1.0f)
            menuFade = 1.0f;
    } else {
        menuFade -= dt * 8.0f;
        if (menuFade < 0.0f)
            menuFade = 0.0f;
    }
    if (!variables::menuOpen && menuFade <= 0.001f)
        return;
    if (variables::selectedTab != prevTab) {
        slideDir = (variables::selectedTab > prevTab) ? 1.0f : -1.0f;
        prevTab = variables::selectedTab;
        tabAnim = 0.0f;
    }
    tabAnim = (std::min)(1.0f, tabAnim + dt * 8.0f);
    const float eased = 1.0f - std::pow(1.0f - tabAnim, 3.0f);
    imGuiCustom::g_contentOffset = ImVec2((1.0f - eased) * 22.0f * slideDir, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menuFade);
    ImGui::SetNextWindowSize(ImVec2(601.0f, 390.0f), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(80.0f, 80.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("jatos", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    const imGuiCustom::Theme& theme = imGuiCustom::GetTheme();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 origin = ImVec2(std::floor(ImGui::GetWindowPos().x), std::floor(ImGui::GetWindowPos().y));
    const ImVec2 winMax = origin + ImVec2(601.0f, 390.0f);
    draw->AddRectFilled(origin, winMax, imGuiCustom::ColorU32(theme.WindowBg), 0.0f);
    draw->AddRect(origin, winMax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    draw->AddRect(origin + ImVec2(1.0f, 1.0f), winMax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
    draw->AddRectFilled(origin, origin + ImVec2(601.0f, 1.5f), imGuiCustom::ColorU32(ImVec4(0.5373f, 0.7647f, 0.7490f, 1.0f)));
    const imGuiCustom::Fonts& fonts = imGuiCustom::GetFonts();
    ImFont* title_font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
    const ImVec2 title_sz = title_font->CalcTextSizeA(12.0f * imGuiCustom::g_fontScale, FLT_MAX, 0.0f, "jatos");
    draw->AddText(title_font, 12.0f * imGuiCustom::g_fontScale, origin + ImVec2(std::floor((601.0f - title_sz.x) * 0.5f), 3.5f), imGuiCustom::ColorU32(theme.TextBright), "jatos");
    const float tab_widths[] = {145.0f, 145.0f, 145.0f, 146.0f};
    float cur_x = 6.0f;
    for (int i = 0; i < 4; ++i) {
        if (DrawTab(kTabs[i], variables::selectedTab == i, ImVec2(cur_x, 16.0f), ImVec2(tab_widths[i], 18.0f)))
            variables::selectedTab = i;
        cur_x += tab_widths[i] + 3.0f;
    }
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menuFade * eased);
    draw->PushClipRect(origin + ImVec2(6.0f, 40.0f), origin + ImVec2(595.0f, 380.0f), true);
    if (variables::selectedTab == 0) {
        DrawPanel("aim_left", ImVec2(6.0f, 40.0f), ImVec2(290.0f, 340.0f));
        DrawPanel("aim_right", ImVec2(305.0f, 40.0f), ImVec2(290.0f, 340.0f));
        Settings::RenderAimMenu();
    } else if (variables::selectedTab == 1) {
        DrawPanel("visual_left", ImVec2(6.0f, 40.0f), ImVec2(290.0f, 340.0f));
        DrawPanel("visual_right_top", ImVec2(305.0f, 40.0f), ImVec2(290.0f, 48.0f));
        DrawPanel("visual_world", ImVec2(305.0f, 96.0f), ImVec2(290.0f, 138.0f));
        DrawPanel("visual_lighting", ImVec2(305.0f, 242.0f), ImVec2(290.0f, 138.0f));
        Settings::RenderVisualMenu();
    } else if (variables::selectedTab == 2) {
        DrawPanel("mics_left", ImVec2(6.0f, 40.0f), ImVec2(290.0f, 340.0f));
        DrawPanel("mics_right", ImVec2(305.0f, 40.0f), ImVec2(290.0f, 340.0f));
        Mics::RenderLocalMenu();
        Mics::RenderMiscMenu();
    } else {
        DrawPanel("settings_left", ImVec2(6.0f, 40.0f), ImVec2(290.0f, 340.0f));
        DrawPanel("settings_right", ImVec2(305.0f, 40.0f), ImVec2(290.0f, 340.0f));
        Settings::RenderSettingsMenu();
    }
    draw->PopClipRect();
    imGuiCustom::g_contentOffset = ImVec2(0.0f, 0.0f);
    ImGui::PopStyleVar();
    ImGui::End();
    if (variables::menuOpen)
        Explorer::RenderWindow(this->d3dDevice);
    else
        Explorer::SetOpen(false);
    ImGui::PopStyleVar();
}



void OverlayWindow::render(ImDrawList* drawList) {
    if (variables::Aimbot::enabled && variables::Aimbot::showFOV) {
        POINT p;
        GetCursorPos(&p);
        ImVec2 center = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
        const ImU32 fc = imGuiCustom::ColorU32(variables::Aimbot::fovColor);
        drawList->AddCircle(center, variables::Aimbot::fovRadius, IM_COL32(0, 0, 0, 255), 64, 2.0f);
        drawList->AddCircle(center, variables::Aimbot::fovRadius, fc, 64, 1.0f);
    }
    if (Keys::KeybindsOn()) {
        struct KeyRow {
            const char* name;
            bool on;
            int key;
            int mode;
            bool* tog;
            bool* was;
        };
        static bool fovTog=false,fovWas=false,flyTog=false,flyWas=false,noclipTog=false,noclipWas=false;
        KeyRow rows[] = {
            {"Aimbot", variables::Aimbot::enabled, variables::Aimbot::aimbotKey, 0, nullptr, nullptr},
            {"Triggerbot", variables::Aimbot::triggerbot, variables::Aimbot::triggerKey, 0, nullptr, nullptr},
            {"FOV", variables::Movement::fov, variables::Movement::fovKey, variables::Movement::fovKeyMode, &fovTog, &fovWas},
            {"Fly", variables::Movement::fly, variables::Movement::flyKey, variables::Movement::flyKeyMode, &flyTog, &flyWas},
            {"Noclip", variables::Movement::noclip, variables::Movement::noclipKey, variables::Movement::noclipKeyMode, &noclipTog, &noclipWas},
        };
        int shown = 0;
        for (auto& r : rows) {
            if (r.on)
                ++shown;
        }
        if (shown > 0) {
            ImGui::SetNextWindowPos(ImVec2(12.0f, 120.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(196.0f, 0.0f), ImGuiCond_Always);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.85f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, imGuiCustom::ColorU32(imGuiCustom::GetTheme().WindowBg));
            ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(imGuiCustom::GetTheme().Text));
            ImGui::Begin("keybinds", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
            ImFont* kfont = Visuals::EspFont();
            const float kfs = 13.0f;
            ImDrawList* kdl = ImGui::GetWindowDrawList();
            const ImVec2 kpos = ImGui::GetWindowPos();
            const ImVec2 ksize = ImGui::GetWindowSize();
            kdl->AddRectFilled(kpos, kpos + ksize, imGuiCustom::ColorU32(imGuiCustom::GetTheme().CardBg, 0.85f), 0.0f);
            kdl->AddRect(kpos, kpos + ksize, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
            kdl->AddRect(kpos + ImVec2(1.0f, 1.0f), kpos + ksize - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
            kdl->AddRectFilled(kpos, ImVec2(kpos.x + ksize.x, kpos.y + 1.5f), imGuiCustom::ColorU32(imGuiCustom::GetTheme().Accent, 0.85f), 0.0f);
            ImVec2 cur = kpos + ImVec2(8.0f, 10.0f);
            kdl->AddText(kfont, kfs, cur, imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright), "keybinds");
            cur.y += 20.0f;
            for (auto& r : rows) {
                if (!r.on)
                    continue;
                bool down;
                if (r.tog) down = Keys::Gate(r.key, r.mode, *r.tog, *r.was);
                else down = (r.key == 0) ? true : Aimbot::IsAimKeyDown(r.key);
                
                const char* keyName = (r.key == 0) ? "Always" : imGuiCustom::KeyName(r.key);
                const char* state = down ? "on" : "off";
                ImU32 stateCol = down ? IM_COL32(120, 255, 120, 255) : imGuiCustom::ColorU32(ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
                kdl->AddText(kfont, kfs, cur, imGuiCustom::ColorU32(imGuiCustom::GetTheme().Text), r.name);
                ImVec2 st = kfont->CalcTextSizeA(kfs, FLT_MAX, 0.0f, state);
                ImVec2 kn = kfont->CalcTextSizeA(kfs, FLT_MAX, 0.0f, keyName);
                kdl->AddText(kfont, kfs, ImVec2(kpos.x + ksize.x - 8.0f - st.x, cur.y), stateCol, state);
                kdl->AddText(kfont, kfs, ImVec2(kpos.x + ksize.x - 8.0f - st.x - 6.0f - kn.x, cur.y), imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright), keyName);
                cur.y += 17.0f;
            }
            ImGui::Dummy(ImVec2(180.0f, (float)shown * 17.0f + 22.0f));
            ImGui::End();
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(2);
        }
    }
    if (!Keys::WatermarkOn())
        return;
    static auto lastTime = std::chrono::steady_clock::now();
    static int frameCount = 0;
    static int fps = 0;
    ++frameCount;
    const auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count() >= 100) {
        fps = frameCount * 10;
        frameCount = 0;
        lastTime = now;
    }
    char clock[16] = "--:--:--";
    {
        const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm tm{};
        localtime_s(&tm, &t);
        strftime(clock, sizeof(clock), "%H:%M:%S", &tm);
    }
    const int ping = Ping::GetMs();
    char text[256];
    if (ping >= 0)
        std::snprintf(text, sizeof(text), "jatos | %d fps | %s | players %d | %dms | %s", fps, Offsets::ClientVersion.c_str(), (int)PlayerCache::players.size(), ping, clock);
    else
        std::snprintf(text, sizeof(text), "jatos | %d fps | %s | players %d | -- | %s", fps, Offsets::ClientVersion.c_str(), (int)PlayerCache::players.size(), clock);
    
    if (Keys::KeybindsOn() && Keys::WatermarkOn()) {
        static bool wFovTog=false,wFovWas=false,wFlyTog=false,wFlyWas=false,wNoclipTog=false,wNoclipWas=false;
        struct WRow{const char* n; bool on; int k; int m; bool* t; bool* w;};
        WRow wrows[]={{"FOV",variables::Movement::fov,variables::Movement::fovKey,variables::Movement::fovKeyMode,&wFovTog,&wFovWas},{"Fly",variables::Movement::fly,variables::Movement::flyKey,variables::Movement::flyKeyMode,&wFlyTog,&wFlyWas},{"Noclip",variables::Movement::noclip,variables::Movement::noclipKey,variables::Movement::noclipKeyMode,&wNoclipTog,&wNoclipWas}};
        char binds[96]=""; bool first=true;
        for(auto &rr: wrows){ if(!rr.on) continue; bool a = Keys::Gate(rr.k, rr.m, *rr.t, *rr.w); char tmp[32]; std::snprintf(tmp,sizeof(tmp),"%s%s %s",first?" | ":" , ", rr.n, a?"on":"off"); strncat_s(text,sizeof(text),tmp,_TRUNCATE); first=false; }
    }
    ImFont* font = Visuals::EspFont();
    const float fontSize = 13.0f;
    const ImVec2 ts = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    const float pad = 8.0f;
    const float w = ts.x + pad * 2.0f;
    const float h = ts.y + pad * 2.0f;
    const float sw = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
    const ImVec2 wmin(std::floor(sw - w - 10.0f), 10.0f);
    const ImVec2 wmax(std::floor(wmin.x + w), std::floor(wmin.y + h));
    drawList->AddRectFilled(wmin, wmax, imGuiCustom::ColorU32(imGuiCustom::GetTheme().CardBg), 0.0f);
    drawList->AddRect(wmin, wmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    drawList->AddRect(wmin + ImVec2(1.0f, 1.0f), wmax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
    drawList->AddRectFilled(wmin, ImVec2(wmax.x, wmin.y + 1.5f), IM_COL32(0, 150, 255, 255), 0.0f);
    drawList->AddText(font, fontSize, ImVec2(std::floor(wmin.x + pad), std::floor(wmin.y + pad)), imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright), text);
}

void OverlayWindow::EndFrame() {
    ImGui::Render();
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    d3dContext->OMSetRenderTargets(1, &renderTarget, nullptr);
    d3dContext->ClearRenderTargetView(renderTarget, clearColor);
    if (variables::ESP::meshChams)
        Cheat::Visuals::MeshDxShader::Flush(renderTarget);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swapChain->Present(variables::Misc::vsync ? 1 : 0, 0);
}

void OverlayWindow::Cleanup() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupD3D11();
    if (windowHandle) {
        DestroyWindow(windowHandle);
        windowHandle = nullptr;
    }
    UnregisterClassW(windowClass.lpszClassName, windowClass.hInstance);
}

HWND OverlayWindow::GetWindowHandle() const {
    return windowHandle;
}
