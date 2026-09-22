#pragma once
#include "../../../sdk/w2s.h"
#include "../../../core/cache/cache.h"
#include "../../../core/variables/variables.h"
#include "../../../sdk/sdk.h"
#include "../../../../ext/imgui/imgui.h"
#include <windows.h>
#include <cstdint>
#include <vector>

namespace Aimbot {
inline std::uintptr_t lockedPlayerAddr = 0;
inline RBX::Vec2 lastTarget{0.0f, 0.0f};
inline bool hasTarget = false;

inline bool aimActiveCached = false;
inline bool trigActiveCached = false;

void MoveMouse(float x, float y);
void AutoClick();
float GetDistance2D(const RBX::Vec2& a, const RBX::Vec2& b);
bool IsAimKeyDown(int vk);

bool AimKeyActive(int key, int mode, bool& tog, bool& was);
bool IsTargetVisible(const RBX::Vec3& worldPos);
void CollectHitboxParts(std::uintptr_t characterAddr, int hitbox, std::vector<std::uintptr_t>& out);
RBX::Vec3 PartWorldPos(std::uintptr_t partAddr);
void WriteMemoryAngles(const RBX::Vec3& targetWorld);
void RenderTracer(ImDrawList* dl);
void RenderPredictionLine(ImDrawList* dl);
void RunAimbot(const RBX::Mat4& view);
bool Is360Active();
float EffectiveFov(float base);
void ApplyGlobalPrediction(RBX::Vec3& world, std::uintptr_t rootPartAddr);
}
