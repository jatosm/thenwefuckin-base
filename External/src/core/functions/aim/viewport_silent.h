// discord.gg/thenwefuckin
#pragma once
#include "../../../sdk/sdk.h"

namespace ViewportSilent {
    void SetTarget(const RBX::Vec3& world);
    void Clear();
    void Shutdown();
    bool IsAiming();
}
