#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "../../ext/imgui/imgui.h"

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ImTextureID OverlayWarnIcon();

class OverlayWindow {
private:
    HWND windowHandle;
    WNDCLASSEXW windowClass;
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTarget;

    void ReleasePartialD3D();
    bool SetupD3D11(HWND hwnd);
    void CleanupD3D11();

public:
    void ResizeBuffers(UINT w, UINT h);

public:
    OverlayWindow();
    bool Initialize();
    void PumpMessages();
    void BeginFrame();
    void RenderMenu();
    void render(ImDrawList* drawList);
    void EndFrame();
    void Cleanup();
    HWND GetWindowHandle() const;
};
