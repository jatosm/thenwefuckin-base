// discord.gg/thenwefuckin
#pragma once
#include "../../../sdk/sdk.h"

namespace MagicBullet {

bool Install();
void Remove();
void Ensure(bool want);
void SetActive(bool on, const RBX::Vec3& world_target);
bool Ready();
bool Aiming();

}
