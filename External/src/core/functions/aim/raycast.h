// discord.gg/thenwefuckin
#pragma once
#include "../../../sdk/sdk.h"
#include <cstdint>

namespace RaycastSilent {

bool Install();
void Remove();
void Ensure(bool want);
void SetActive(bool on, const RBX::Vec3& world_target, bool wallbang);
bool Ready();
bool Aiming();
bool WallbangMode();
std::uint64_t Calls();
std::uintptr_t OriginalHandler();

}
