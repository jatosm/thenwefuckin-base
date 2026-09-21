
#pragma once
#include "imgui.h"
#ifndef IMGUI_DISABLE

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11SamplerState;
struct ID3D11Buffer;

IMGUI_IMPL_API bool     ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context);
IMGUI_IMPL_API void     ImGui_ImplDX11_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplDX11_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data);

IMGUI_IMPL_API bool     ImGui_ImplDX11_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplDX11_InvalidateDeviceObjects();

IMGUI_IMPL_API void     ImGui_ImplDX11_UpdateTexture(ImTextureData* tex);

struct ImGui_ImplDX11_RenderState
{
    ID3D11Device*           Device;
    ID3D11DeviceContext*    DeviceContext;
    ID3D11SamplerState*     SamplerLinear;
    ID3D11SamplerState*     SamplerNearest;
    ID3D11Buffer*           VertexConstantBuffer;
};

#endif
