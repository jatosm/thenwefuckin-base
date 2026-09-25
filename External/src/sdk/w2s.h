// discord.gg/thenwefuckin
#pragma once
#include "sdk.h"

namespace W2S {
inline float ScreenW() {
    static float w = 0.0f;
    if (w <= 0.0f)
        w = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
    return w;
}

inline float ScreenH() {
    static float h = 0.0f;
    if (h <= 0.0f)
        h = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
    return h;
}

inline RBX::Vec2 WorldToScreen(const RBX::Vec3& world, const RBX::Mat4& view) {
    RBX::Vec2 screen{};
    const float w = world.X * view.data[12] + world.Y * view.data[13] + world.Z * view.data[14] + view.data[15];
    if (w < 0.1f)
        return screen;
    const float x = world.X * view.data[0] + world.Y * view.data[1] + world.Z * view.data[2] + view.data[3];
    const float y = world.X * view.data[4] + world.Y * view.data[5] + world.Z * view.data[6] + view.data[7];
    const float sw = ScreenW();
    const float sh = ScreenH();
    screen.X = (sw * 0.5f * (x / w)) + (sw * 0.5f);
    screen.Y = -(sh * 0.5f * (y / w)) + (sh * 0.5f);
    return screen;
}
}
