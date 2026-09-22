#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "render.h"
#include "../core/logger/logger.h"
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
#include "../../src/core/functions/players_widget.h"
#include "../../src/core/functions/backpack_widget.h"
#include "../../src/core/keys/keys.h"

#include "../../src/core/functions/visual/visual.h"
#include "../../src/core/cache/cache.h"
#include "../../src/core/cache/pf_cache.h"
#include "../../src/core/cache/cb_cache.h"
#include "../../src/core/cache/ops_cache.h"
#include "../../src/core/net/ping.h"
#include "../../src/sdk/offsets.h"
#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "../../ext/stb/stb_image.h"
#include "assets/lucide_warn.h"
#include "../core/features/preview/preview.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")
#ifndef DXGI_PRESENT_ALLOW_TEARING
#define DXGI_PRESENT_ALLOW_TEARING 0x00000200U
#endif
#ifndef DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
#define DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING 512
#endif
#ifndef DXGI_PRESENT_DO_NOT_WAIT
#define DXGI_PRESENT_DO_NOT_WAIT 0x00000001U
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static OverlayWindow* g_overlayWnd = nullptr;

static ID3D11ShaderResourceView* s_warnTex = nullptr;
static int s_warnW = 0, s_warnH = 0;

ImTextureID OverlayWarnIcon() {
    return (ImTextureID)s_warnTex;
}

static bool LoadWarnIcon(ID3D11Device* dev) {
    if (s_warnTex || !dev)
        return s_warnTex != nullptr;
    int w = 0, h = 0, ch = 0;
    unsigned char* img = stbi_load_from_memory(lucide_warn_png, (int)lucide_warn_png_len, &w, &h, &ch, 4);
    if (!img || w <= 0 || h <= 0)
        return false;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = (UINT)w;
    desc.Height = (UINT)h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA sub{};
    sub.pSysMem = img;
    sub.SysMemPitch = (UINT)(w * 4);
    ID3D11Texture2D* tex = nullptr;
    HRESULT hr = dev->CreateTexture2D(&desc, &sub, &tex);
    stbi_image_free(img);
    if (FAILED(hr) || !tex)
        return false;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.Format = desc.Format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    hr = dev->CreateShaderResourceView(tex, &srvd, &s_warnTex);
    tex->Release();
    if (FAILED(hr) || !s_warnTex)
        return false;
    s_warnW = w;
    s_warnH = h;
    return true;
}

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_NCPAINT:
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_CLOSE:
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE: {
        if (g_overlayWnd && wParam != SIZE_MINIMIZED)
            g_overlayWnd->ResizeBuffers((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
        return 0;
    }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

OverlayWindow::OverlayWindow() : windowHandle(nullptr), d3dDevice(nullptr), d3dContext(nullptr), swapChain(nullptr), renderTarget(nullptr) {
    ZeroMemory(&windowClass, sizeof(windowClass));
}

void OverlayWindow::ReleasePartialD3D() {
    if (renderTarget) { renderTarget->Release(); renderTarget = nullptr; }
    if (swapChain) { swapChain->Release(); swapChain = nullptr; }
    if (d3dContext) { d3dContext->Release(); d3dContext = nullptr; }
    if (d3dDevice) { d3dDevice->Release(); d3dDevice = nullptr; }
}

static HRESULT SafeD3D11Create(DXGI_SWAP_CHAIN_DESC* sd, D3D_FEATURE_LEVEL* levels, IDXGISwapChain** sc, ID3D11Device** dev, D3D_FEATURE_LEVEL* obtained, ID3D11DeviceContext** ctx, D3D_DRIVER_TYPE type) {
    __try {
        return D3D11CreateDeviceAndSwapChain(nullptr, type, nullptr, 0, levels, 2, D3D11_SDK_VERSION, sd, sc, dev, obtained, ctx);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return E_FAIL;
    }
}

bool OverlayWindow::SetupD3D11(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL obtainedLevel;
    HRESULT hr = E_FAIL;

    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    hr = SafeD3D11Create(&sd, levels, &swapChain, &d3dDevice, &obtainedLevel, &d3dContext, D3D_DRIVER_TYPE_HARDWARE);
    Logger::logf("RENDER", "D3D11 HARDWARE+TEARING hr=0x%X", (unsigned)hr);

    if (FAILED(hr) || !swapChain || !d3dDevice || !d3dContext) {
        ReleasePartialD3D();
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        hr = SafeD3D11Create(&sd, levels, &swapChain, &d3dDevice, &obtainedLevel, &d3dContext, D3D_DRIVER_TYPE_HARDWARE);
        Logger::logf("RENDER", "D3D11 HARDWARE hr=0x%X", (unsigned)hr);
    }

    if (FAILED(hr) || !swapChain || !d3dDevice || !d3dContext) {
        ReleasePartialD3D();
        hr = SafeD3D11Create(&sd, levels, &swapChain, &d3dDevice, &obtainedLevel, &d3dContext, D3D_DRIVER_TYPE_WARP);
        Logger::logf("RENDER", "D3D11 WARP hr=0x%X", (unsigned)hr);
    }
    if (FAILED(hr) || !swapChain || !d3dDevice || !d3dContext) {
        ReleasePartialD3D();
        return false;
    }
    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))) || !backBuffer)
        return false;
    if (FAILED(d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTarget))) {
    backBuffer->Release();
    IDXGIDevice1* dxgi1 = nullptr;
    if (SUCCEEDED(d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi1)) && dxgi1) {
        dxgi1->SetMaximumFrameLatency(1);
        dxgi1->Release();
    }
    return true;
}
    backBuffer->Release();
    IDXGIDevice1* dxgi1 = nullptr;
    if (SUCCEEDED(d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi1)) && dxgi1) {
        dxgi1->SetMaximumFrameLatency(1);
        dxgi1->Release();
    }
    return true;
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
    Logger::log("RENDER", "Initialize start");
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_CLASSDC;
    windowClass.lpfnWndProc = OverlayWndProc;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.lpszClassName = L"jatos";
    if (!RegisterClassExW(&windowClass)) {
        Logger::log("RENDER", "RegisterClassExW failed");
        return false;
    }
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    Logger::logf("RENDER", "screen %dx%d", screenW, screenH);
    windowHandle = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW, windowClass.lpszClassName, L"jatos", WS_POPUP, 0, 0, screenW, screenH, nullptr, nullptr, windowClass.hInstance, nullptr);
    if (!windowHandle) {
        Logger::log("RENDER", "CreateWindowExW failed");
        return false;
    }
    SetLayeredWindowAttributes(windowHandle, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(windowHandle, &margins);
    ShowWindow(windowHandle, SW_SHOW);
    UpdateWindow(windowHandle);
    Logger::log("RENDER", "SetupD3D11...");
    if (!SetupD3D11(windowHandle)) {
        Logger::log("RENDER", "SetupD3D11 failed");
        return false;
    }
    Logger::log("RENDER", "SetupD3D11 OK");
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
    LoadWarnIcon(d3dDevice);
    Preview::Init(d3dDevice);
    Cheat::Visuals::MeshDxShader::Init(d3dDevice, d3dContext);
    Cheat::Visuals::MeshDxShader::Resize((unsigned)screenW, (unsigned)screenH);
    g_overlayWnd = this;
    return true;
}

void OverlayWindow::PumpMessages() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void OverlayWindow::BeginFrame() {
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
    PumpMessages();

    {
        static LONG lastStyle = 0;
        static bool firstStyle = true;
        const LONG want = variables::menuOpen
            ? (WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW)
            : (WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        if (firstStyle || want != lastStyle) {
            SetWindowLong(windowHandle, GWL_EXSTYLE, want);
            lastStyle = want;
            firstStyle = false;
        }
    }
    {
        static DWORD lastAffinity = 0xFFFFFFFF;
        static bool firstAff = true;
        const DWORD wantAff = Keys::StreamProofOn() ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE;
        if (firstAff || wantAff != lastAffinity) {
            SetWindowDisplayAffinity(windowHandle, wantAff);
            lastAffinity = wantAff;
            firstAff = false;
        }
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
    if (!variables::menuOpen && menuFade <= 0.001f) {

        return;
    }
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
    if (variables::menuOpen) {
        Explorer::RenderWindow(this->d3dDevice);
        PlayersWidget::RenderWindow(this->d3dDevice);
    } else {
        Explorer::SetOpen(false);
        PlayersWidget::SetOpen(false);
    }
    ImGui::PopStyleVar();
}

void OverlayWindow::render(ImDrawList* drawList) {
    if (variables::Aimbot::enabled && variables::Aimbot::showFOV) {
        POINT p;
        GetCursorPos(&p);
        ImVec2 center = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
        const ImU32 fc = imGuiCustom::ColorU32(variables::Aimbot::fovColor);
        const int seg = (int)std::clamp(variables::Aimbot::fovRadius * 0.4f, 24.0f, 64.0f);
        if (variables::Aimbot::fillFov)
            drawList->AddCircleFilled(center, variables::Aimbot::fovRadius, imGuiCustom::ColorU32(variables::Aimbot::fovFillColor), seg);
        drawList->AddCircle(center, variables::Aimbot::fovRadius, IM_COL32(0, 0, 0, 255), seg, 2.0f);
        drawList->AddCircle(center, variables::Aimbot::fovRadius, fc, seg, 1.0f);
    }
    if (Keys::KeybindsOn()) {
        struct KeyRow {
            const char* name;
            bool on;
            int key;
            int mode;
            bool* tog;
            bool* was;
            const bool* cached;
        };
        static bool fovTog=false,fovWas=false,flyTog=false,flyWas=false,noclipTog=false,noclipWas=false;
        KeyRow rows[] = {
            {"Aimbot", variables::Aimbot::enabled, variables::Aimbot::aimbotKey, variables::Aimbot::aimbotKeyMode, nullptr, nullptr, &Aimbot::aimActiveCached},
            {"Triggerbot", variables::Aimbot::triggerbot, variables::Aimbot::triggerKey, variables::Aimbot::triggerKeyMode, nullptr, nullptr, &Aimbot::trigActiveCached},
            {"FOV", variables::Movement::fov, variables::Movement::fovKey, variables::Movement::fovKeyMode, &fovTog, &fovWas, nullptr},
            {"Fly", variables::Movement::fly, variables::Movement::flyKey, variables::Movement::flyKeyMode, &flyTog, &flyWas, nullptr},
            {"Noclip", variables::Movement::noclip, variables::Movement::noclipKey, variables::Movement::noclipKeyMode, &noclipTog, &noclipWas, nullptr},
        };
        int shown = 0;
        for (auto& r : rows) {
            if (r.on)
                ++shown;
        }
        if (shown > 0) {

            ImFont* kfont = Visuals::EspFont();
            const float kfs = 13.0f;
            const float kpad = 8.0f;
            const float krowH = 17.0f;
            const float kw = 196.0f;
            const float kh = kpad * 2.0f + 22.0f + (float)shown * krowH;
            const ImVec2 kmin(12.0f, 120.0f);
            const ImVec2 kmax(kmin.x + kw, kmin.y + kh);
            drawList->AddRectFilled(kmin, kmax, imGuiCustom::ColorU32(imGuiCustom::GetTheme().CardBg, 0.85f), 0.0f);
            drawList->AddRect(kmin, kmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
            drawList->AddRect(kmin + ImVec2(1.0f, 1.0f), kmax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
            drawList->AddRectFilled(kmin, ImVec2(kmax.x, kmin.y + 1.5f), imGuiCustom::ColorU32(imGuiCustom::GetTheme().Accent, 0.85f), 0.0f);
            ImVec2 cur = kmin + ImVec2(kpad, 10.0f);
            drawList->AddText(kfont, kfs, cur, imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright), "keybinds");
            cur.y += 20.0f;
            for (auto& r : rows) {
                if (!r.on)
                    continue;
                bool down;
                if (r.mode == 2) down = true;
                else if (r.mode == 1 && r.cached) down = *r.cached;
                else if (r.tog) down = Keys::Gate(r.key, r.mode, *r.tog, *r.was);
                else down = (r.key == 0) ? true : Aimbot::IsAimKeyDown(r.key);

                const char* keyName = (r.key == 0) ? "Always" : imGuiCustom::KeyName(r.key);
                const char* state = down ? "on" : "off";
                ImU32 stateCol = down ? IM_COL32(120, 255, 120, 255) : imGuiCustom::ColorU32(ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
                drawList->AddText(kfont, kfs, cur, imGuiCustom::ColorU32(imGuiCustom::GetTheme().Text), r.name);
                ImVec2 st = kfont->CalcTextSizeA(kfs, FLT_MAX, 0.0f, state);
                ImVec2 kn = kfont->CalcTextSizeA(kfs, FLT_MAX, 0.0f, keyName);
                drawList->AddText(kfont, kfs, ImVec2(kmax.x - kpad - st.x, cur.y), stateCol, state);
                drawList->AddText(kfont, kfs, ImVec2(kmax.x - kpad - st.x - 6.0f - kn.x, cur.y), imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright), keyName);
                cur.y += krowH;
            }
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

        static char cachedClock[16] = "--:--:--";
        static auto lastClock = std::chrono::steady_clock::now() - std::chrono::seconds(10);
        const auto ccNow = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(ccNow - lastClock).count() >= 500) {
            const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm tm{};
            localtime_s(&tm, &t);
            strftime(cachedClock, sizeof(cachedClock), "%H:%M:%S", &tm);
            lastClock = ccNow;
        }
        std::memcpy(clock, cachedClock, sizeof(clock));
    }
    const int ping = Ping::GetMs();
    char text[256];
    if (ping >= 0)
        std::snprintf(text, sizeof(text), "jatos | %d fps | %s | players %d pf %d cb %d ops %d | %dms | %s", fps, Offsets::ClientVersion.c_str(), (int)PlayerCache::players.size(), (int)PfCache::players.size(), (int)CbCache::players.size(), (int)OpsCache::players.size(), ping, clock);
    else
        std::snprintf(text, sizeof(text), "jatos | %d fps | %s | players %d pf %d cb %d ops %d | -- | %s", fps, Offsets::ClientVersion.c_str(), (int)PlayerCache::players.size(), (int)PfCache::players.size(), (int)CbCache::players.size(), (int)OpsCache::players.size(), clock);

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
    static float cachedSw = 0.0f;
    if (cachedSw <= 0.0f)
        cachedSw = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
    const float sw = cachedSw;
    const ImVec2 wmin(std::floor(sw - w - 10.0f), 10.0f);
    const ImVec2 wmax(std::floor(wmin.x + w), std::floor(wmin.y + h));
    drawList->AddRectFilled(wmin, wmax, imGuiCustom::ColorU32(imGuiCustom::GetTheme().CardBg), 0.0f);
    drawList->AddRect(wmin, wmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
    drawList->AddRect(wmin + ImVec2(1.0f, 1.0f), wmax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
    drawList->AddRectFilled(wmin, ImVec2(wmax.x, wmin.y + 1.5f), IM_COL32(0, 150, 255, 255), 0.0f);
    drawList->AddText(font, fontSize, ImVec2(std::floor(wmin.x + pad), std::floor(wmin.y + pad)), imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright), text);
    BackpackWidget::RenderOverlay(drawList, this->d3dDevice);
}

void OverlayWindow::EndFrame() {
    ImGui::Render();
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    d3dContext->OMSetRenderTargets(1, &renderTarget, nullptr);
    d3dContext->ClearRenderTargetView(renderTarget, clearColor);
    if (variables::ESP::meshChams)
        Cheat::Visuals::MeshDxShader::Flush(renderTarget);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    if (variables::Misc::vsync) {
        swapChain->Present(1, 0);
    } else {
        HRESULT pr = swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING | DXGI_PRESENT_DO_NOT_WAIT);
        if (pr == DXGI_ERROR_INVALID_CALL)
            pr = swapChain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);
        if (pr == DXGI_ERROR_INVALID_CALL)
            swapChain->Present(0, 0);
    }
    if (variables::Misc::vsync)
        DwmFlush();
}

void OverlayWindow::Cleanup() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    Preview::Shutdown();
    if (s_warnTex) {
        s_warnTex->Release();
        s_warnTex = nullptr;
    }
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
