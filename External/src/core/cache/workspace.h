#pragma once
#include "../../sdk/sdk.h"
#include "../features/worldgeo/worldgeo.h"
#include <vector>
#include <thread>
#include <atomic>
namespace WorkspaceCache {
inline bool IsVisible(const RBX::Vec3& camPos, const RBX::Vec3& targetPos) {
    return WorldGeo::IsVisible(camPos, targetPos);
}
inline void Loop() { while (true) std::this_thread::sleep_for(std::chrono::seconds(1)); }
}
