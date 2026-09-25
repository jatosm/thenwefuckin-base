// discord.gg/thenwefuckin
#pragma once
#include "../../../../src/sdk/w2s.h"
#include "../../../../src/core/cache/cache.h"
#include "../../../../src/core/variables/variables.h"
#include "../../../../ext/imgui/imgui.h"
#include <string>

namespace Visuals {
ImFont* EspFont();
void DrawOutlinedText(ImDrawList* dl, const ImVec2& pos, const std::string& text, ImU32 col);
void DrawLine(ImDrawList* dl, const ImVec2& a, const ImVec2& b, ImU32 col, float t, bool outline);
bool ToScreen(const RBX::Vec3& w, const RBX::Mat4& v, ImVec2& out);
void Bone(ImDrawList* dl, const RBX::Vec3& a, const RBX::Vec3& b, const RBX::Mat4& v);
void BoneCached(ImDrawList* dl, const RBX::Vec3& a, const RBX::Vec3& b, const RBX::Mat4& v, ImU32 col, float thickness, bool outline);
void BoneAddrCached(ImDrawList* dl, const RBX::Vec3& pa, const RBX::Vec3& pb, const RBX::Mat4& v, ImU32 col, float thickness, bool outline);
RBX::Vec3 PartPosRaw(std::uintptr_t partAddr);
RBX::Vec3 PartPos(std::uintptr_t partAddr);
void BoneAddr(ImDrawList* dl, std::uintptr_t a, std::uintptr_t b, const RBX::Mat4& v);
bool DynamicBounds(const PlayerCache::LimbAddrs& l, bool r6, const RBX::Mat4& v, float& x0, float& x1, float& y0, float& y1);
bool DynamicBoundsEx(const PlayerCache::LimbAddrs& l, bool r6, const RBX::Mat4& v, const RBX::Vec3& hp, const RBX::Vec3& rp, float& x0, float& x1, float& y0, float& y1);
void DrawBoxFill(ImDrawList* dl, float x0, float y0, float x1, float y1);
void DrawBox(ImDrawList* dl, float x0, float y0, float x1, float y1);
void DrawBoxCol(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col);
void RenderSkeleton(ImDrawList* dl, const PlayerCache::LimbAddrs& l, const RBX::Mat4& v);
void RenderSkeletonCached(ImDrawList* dl, const PlayerCache::LimbAddrs& l, const RBX::Mat4& v, ImU32 col, float thickness, bool outline);
void RenderESP(ImDrawList* dl, const RBX::Mat4& v);
void RenderMeshChams(ImDrawList* dl, const RBX::Mat4& v);
void DrawWorldBox(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col);
void DrawHealthBar(ImDrawList* dl, float bx0, float bx1, float by0, float by1, float frac, ImU32 fillCol);
}
