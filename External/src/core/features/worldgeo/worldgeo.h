#pragma once

#include "imgui.h"
#include "sdk/sdk.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace WorldGeo {

struct Box {
    float px = 0.0f, py = 0.0f, pz = 0.0f;
    float r[9]{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float sx = 0.0f, sy = 0.0f, sz = 0.0f;
};

struct Snapshot {
    std::vector<Box> boxes;

    std::unordered_map<std::int64_t, std::vector<int>> grid;
};

inline std::atomic<bool> running{true};

void Loop();

void Render(ImDrawList* dl, const RBX::Mat4& view);

bool IsVisible(const RBX::Vec3& cam, const RBX::Vec3& target);

}
