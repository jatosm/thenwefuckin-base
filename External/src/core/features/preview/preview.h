#pragma once

#include "imgui.h"

#include <d3d11.h>

namespace Preview {

bool Init(ID3D11Device* dev);
void Shutdown();
bool Ready();

void DrawPanel();

}
