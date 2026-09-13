#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "../../ext/imgui/imgui.h"

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class OverlayWindow {
private:
    HWND windowHandle;
    WNDCLASSEXW windowClass;
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTarget;

    void SetupD3D11(HWND hwnd);
    void CleanupD3D11();

public:
    OverlayWindow();
    bool Initialize();
    void BeginFrame();
    void RenderMenu();
    void render(ImDrawList* drawList);
    void EndFrame();
    void Cleanup();
    HWND GetWindowHandle() const;
};
