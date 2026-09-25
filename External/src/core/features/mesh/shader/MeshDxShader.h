// discord.gg/thenwefuckin
#pragma once

#include "sdk/MeshMath.h"

#include <d3d11.h>
#include <cstdint>
#include <string>

namespace Cheat {
namespace Visuals {
namespace MeshDxShader {

bool Init(ID3D11Device* device, ID3D11DeviceContext* context);
void Shutdown();
void Resize(unsigned width, unsigned height);

void BeginFrame(const Mesh::Matrix4x4& view, const Mesh::Vector3& camera, float time);
void QueueMesh(const std::string& mesh_id, const Mesh::Matrix4x4& world);
void QueueBox(const Mesh::Matrix4x4& world);
void Flush(ID3D11RenderTargetView* rtv);
bool IsFrameValid();

const char* const* ModeNames();
int ModeNameCount();

}
}
}
