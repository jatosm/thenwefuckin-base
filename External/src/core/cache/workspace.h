#pragma once
#include "../../sdk/sdk.h"
#include <vector>
#include <thread>
#include <atomic>
namespace WorkspaceCache {
inline bool IsVisible(const RBX::Vec3& camPos, const RBX::Vec3& targetPos) { (void)camPos; (void)targetPos; return true; }
inline void Loop() { while (true) std::this_thread::sleep_for(std::chrono::seconds(1)); }
}
