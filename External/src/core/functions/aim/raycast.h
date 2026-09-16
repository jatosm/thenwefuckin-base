#pragma once
#include "../../../sdk/sdk.h"
#include <cstdint>

namespace RaycastSilent {
bool Install();
void Remove();
void Ensure(bool want = true);
void SetActive(bool on, const RBX::Vec3& world_target = {}, bool wallbang = false);
bool Ready();
bool Aiming();
bool WallbangMode();
std::uintptr_t OriginalHandler();
}
