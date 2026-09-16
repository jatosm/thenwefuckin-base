#pragma once

struct ID3D11Device;

namespace Explorer {
void SetOpen(bool open);
bool IsOpen();
void RenderWindow(ID3D11Device* device);
}
