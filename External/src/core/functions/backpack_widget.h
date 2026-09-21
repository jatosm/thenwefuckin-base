#pragma once
#include <d3d11.h>
struct ImDrawList;
namespace BackpackWidget{ void SetOpen(bool o); bool IsOpen(); void RenderWindow(ID3D11Device* d); void RenderOverlay(ImDrawList* dl, ID3D11Device* dev); }