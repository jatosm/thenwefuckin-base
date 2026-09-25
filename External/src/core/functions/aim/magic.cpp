// discord.gg/thenwefuckin
#include "magic.h"
#include "raycast.h"

namespace MagicBullet {
bool Install() {
    return RaycastSilent::Install();
}

void Remove() {
    RaycastSilent::Remove();
}

void Ensure(bool want) {
    RaycastSilent::Ensure(want);
}

void SetActive(bool on, const RBX::Vec3& world_target) {
    RaycastSilent::SetActive(on, world_target, true);
}

bool Ready() {
    return RaycastSilent::Ready();
}

bool Aiming() {
    return RaycastSilent::Aiming() && RaycastSilent::WallbangMode();
}
}
