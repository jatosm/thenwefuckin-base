#pragma once

#include "sdk/MeshMath.h"
#include "imgui.h"

#include <cstdint>

namespace Cheat {
namespace Visuals {
namespace MeshChams {

void Draw(
	ImDrawList* dl,
	std::uint64_t character,
	const Mesh::Matrix4x4& view,
	const Mesh::Vector2& viewport,
	float scale_x,
	float scale_y,
	ImU32 fill_col);

const char* const* OutlineStyleNames();
int OutlineStyleNameCount();

bool ExpandBounds(
	std::uint64_t character,
	const Mesh::Matrix4x4& view,
	const Mesh::Vector2& viewport,
	float scale_x,
	float scale_y,
	float& min_x, float& max_x,
	float& min_y, float& max_y,
	Mesh::Vector3& wmin, Mesh::Vector3& wmax);

}
}
}
