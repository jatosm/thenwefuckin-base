// discord.gg/thenwefuckin
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "visual.h"
#include "../../../render/menu/library.h"
#include "../../cache/workspace.h"
#include "../../cache/worldcache.h"
#include "../../cache/pf_cache.h"
#include "../../cache/cb_cache.h"
#include "../../cache/ml_cache.h"
#include "../../cache/ops_cache.h"
#include "../players/players.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "../../features/mesh/cache/MeshCache.h"
#include "../../features/mesh/chams/MeshChams.h"
#include "../../features/mesh/shader/MeshDxShader.h"

namespace Visuals {
ImFont* EspFont() {
    const imGuiCustom::Fonts& fonts = imGuiCustom::GetFonts();
    return fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
}

float EspSize() {
    return variables::Misc::espFontSize;
}

ImU32 ToU32(const ImVec4& c) {
    return IM_COL32(static_cast<int>(c.x * 255.0f), static_cast<int>(c.y * 255.0f), static_cast<int>(c.z * 255.0f), static_cast<int>(c.w * 255.0f));
}

void DrawOutlinedText(ImDrawList* dl, const ImVec2& pos, const std::string& text, ImU32 col) {

    ImFont* font = EspFont();
    const float s = EspSize();
    dl->AddText(font, s, ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 255), text.c_str());
    dl->AddText(font, s, pos, col, text.c_str());
}

void DrawLine(ImDrawList* dl, const ImVec2& a, const ImVec2& b, ImU32 col, float t, bool outline) {
    if (outline)
        dl->AddLine(a, b, IM_COL32(0, 0, 0, 255), t + 0.5f);
    dl->AddLine(a, b, col, t);
}

bool ToScreen(const RBX::Vec3& w, const RBX::Mat4& v, ImVec2& out) {
    const auto s = W2S::WorldToScreen(w, v);
    if (s.X == 0 && s.Y == 0)
        return false;
    out = ImVec2(s.X, s.Y);
    return true;
}

void Bone(ImDrawList* dl, const RBX::Vec3& a, const RBX::Vec3& b, const RBX::Mat4& v) {
    ImVec2 sa, sb;
    if (!ToScreen(a, v, sa) || !ToScreen(b, v, sb))
        return;
    DrawLine(dl, sa, sb, ToU32(variables::ESP::skeletonColor), variables::ESP::skeletonThickness, variables::ESP::skeletonOutline);
}

void BoneCached(ImDrawList* dl, const RBX::Vec3& a, const RBX::Vec3& b, const RBX::Mat4& v, ImU32 col, float thickness, bool outline) {
    ImVec2 sa, sb;
    if (!ToScreen(a, v, sa) || !ToScreen(b, v, sb))
        return;
    DrawLine(dl, sa, sb, col, thickness, outline);
}

RBX::Vec3 PartPosRaw(std::uintptr_t partAddr) {
    if (!partAddr)
        return {};
    const uintptr_t prim = memory->read<uintptr_t>(partAddr + Offsets::BasePart::Primitive);
    if (!prim)
        return {};
    return memory->read<RBX::Vec3>(prim + Offsets::Primitive::Position);
}

inline std::unordered_map<std::uintptr_t, RBX::Vec3>& PosCache() {
    static std::unordered_map<std::uintptr_t, RBX::Vec3> cache;
    return cache;
}
inline bool& PosCacheActive() {
    static bool active = false;
    return active;
}
struct PosFrame {
    PosFrame() {
        auto& c = PosCache();
        if (c.size() > 512)
            c.clear();
        PosCacheActive() = true;
        c.clear();
    }
    ~PosFrame() {
        PosCacheActive() = false;
        PosCache().clear();
    }
};

RBX::Vec3 PartPos(std::uintptr_t partAddr) {
    if (!partAddr)
        return {};
    if (!PosCacheActive())
        return PartPosRaw(partAddr);
    auto& c = PosCache();
    auto it = c.find(partAddr);
    if (it != c.end())
        return it->second;
    const RBX::Vec3 p = PartPosRaw(partAddr);
    if (c.size() < 1024)
        c.emplace(partAddr, p);
    return p;
}

void BoneAddr(ImDrawList* dl, std::uintptr_t a, std::uintptr_t b, const RBX::Mat4& v) {
    if (!a || !b)
        return;
    const RBX::Vec3 pa = PartPos(a);
    const RBX::Vec3 pb = PartPos(b);
    if ((pa.X == 0 && pa.Y == 0 && pa.Z == 0) || (pb.X == 0 && pb.Y == 0 && pb.Z == 0))
        return;
    Bone(dl, pa, pb, v);
}

bool ProjectPoint(const RBX::Vec3& w, const RBX::Mat4& v, RBX::Vec2& out) {
    const float ww = w.X * v.data[12] + w.Y * v.data[13] + w.Z * v.data[14] + v.data[15];
    if (!(ww >= 1.0f))
        return false;
    const auto s = W2S::WorldToScreen(w, v);
    if (s.X == 0 && s.Y == 0)
        return false;
    out = s;
    return true;
}

float EspFocal(float viewH) {
    float fov = 70.0f * 0.0174533f;
    if (Globals::camera.Addr) {
        const float live = memory->read<float>(Globals::camera.Addr + Offsets::Camera::FieldOfView);
        if (std::isfinite(live) && live > 0.2f && live < 3.0f)
            fov = live;
    }
    const float t = tanf(fov * 0.5f);
    if (!(t > 0.05f))
        return viewH * 0.7f;
    return (viewH * 0.5f) / t;
}

bool SanityBox(float dist, float x0, float y0, float x1, float y1, float focal) {
    const float w = x1 - x0, h = y1 - y0;
    if (!(h > 3.0f) || !(w > 2.0f))
        return false;
    if (!(dist >= 1.0f))
        dist = 1.0f;
    const float expH = 5.5f * focal / dist;
    const float expW = 2.4f * focal / dist;
    if (h > expH * 3.0f + 30.0f)
        return false;
    if (w > expW * 4.0f + 30.0f)
        return false;
    return true;
}

bool DynamicBounds(const PlayerCache::LimbAddrs& l, bool r6, const RBX::Mat4& v, float& x0, float& x1, float& y0, float& y1) {
    if (!l.head || !l.hrp)
        return false;
    const RBX::Vec3 hp = PartPos(l.head);
    const RBX::Vec3 rp = PartPos(l.hrp);
    if ((hp.X == 0 && hp.Y == 0 && hp.Z == 0) || (rp.X == 0 && rp.Y == 0 && rp.Z == 0))
        return false;
    return DynamicBoundsEx(l, r6, v, hp, rp, x0, x1, y0, y1);
}

bool DynamicBoundsEx(const PlayerCache::LimbAddrs& l, bool r6, const RBX::Mat4& v, const RBX::Vec3& hp, const RBX::Vec3& rp, float& x0, float& x1, float& y0, float& y1) {
    RBX::Vec3 pts[20];
    int n = 0;
    pts[n++] = {hp.X, hp.Y + 0.6f, hp.Z};
    pts[n++] = hp;
    const std::uintptr_t extra[] = {
        r6 ? l.torso : l.upperTorso, r6 ? l.lArm : l.lUpperArm, r6 ? l.rArm : l.rUpperArm,
        r6 ? l.lLeg : l.lUpperLeg, r6 ? l.rLeg : l.rUpperLeg, r6 ? 0 : l.lowerTorso,
        r6 ? 0 : l.lLowerArm, r6 ? 0 : l.lHand, r6 ? 0 : l.rLowerArm, r6 ? 0 : l.rHand,
        r6 ? 0 : l.lLowerLeg, r6 ? 0 : l.lFoot, r6 ? 0 : l.rLowerLeg, r6 ? 0 : l.rFoot
    };
    for (auto addr : extra) {
        if (!addr || n >= 17)
            continue;
        const RBX::Vec3 p = PartPos(addr);
        if (p.X == 0 && p.Y == 0 && p.Z == 0)
            continue;
        pts[n++] = p;
    }
    pts[n++] = rp;
    pts[n++] = {rp.X, rp.Y - (r6 ? 3.0f : 2.5f), rp.Z};
    bool any = false;
    float mnX = 1e9f, mxX = -1e9f, mnY = 1e9f, mxY = -1e9f;
    for (int i = 0; i < n; ++i) {
        RBX::Vec2 s{};
        if (!ProjectPoint(pts[i], v, s))
            continue;
        any = true;
        if (s.X < mnX) mnX = s.X;
        if (s.X > mxX) mxX = s.X;
        if (s.Y < mnY) mnY = s.Y;
        if (s.Y > mxY) mxY = s.Y;
    }
    if (!any)
        return false;
    const float pad = std::clamp((mxY - mnY) * 0.06f, 3.0f, 10.0f);
    x0 = mnX - pad * 0.8f;
    x1 = mxX + pad * 0.8f;
    y0 = mnY - pad;
    y1 = mxY + pad;
    return x1 > x0 && y1 > y0;
}

void DrawBoxFill(ImDrawList* dl, float x0, float y0, float x1, float y1) {
    if (!variables::ESP::boxFilled)
        return;
    if (variables::ESP::boxFillGradient)
        dl->AddRectFilledMultiColor(ImVec2(x0, y0), ImVec2(x1, y1), ToU32(variables::ESP::boxFillColor), ToU32(variables::ESP::boxFillColor), ToU32(variables::ESP::boxFillColor2), ToU32(variables::ESP::boxFillColor2));
    else
        dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), ToU32(variables::ESP::boxFillColor));
}

void DrawBox(ImDrawList* dl, float x0, float y0, float x1, float y1) {
    const ImU32 col = ToU32(variables::ESP::boxColor);

    dl->AddRect(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
    dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col, 0.0f, 0, 1.0f);
    dl->AddRect(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
}
void DrawBoxCol(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col) {
    dl->AddRect(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
    dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col, 0.0f, 0, 1.0f);
    dl->AddRect(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
}
void DrawWorldBox(ImDrawList* dl, float x0, float y0, float x1, float y1, ImU32 col) {
    dl->AddRect(ImVec2(x0 - 1.0f, y0 - 1.0f), ImVec2(x1 + 1.0f, y1 + 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
    dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col, 0.0f, 0, 1.0f);
    dl->AddRect(ImVec2(x0 + 1.0f, y0 + 1.0f), ImVec2(x1 - 1.0f, y1 - 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
}
void DrawHealthBar(ImDrawList* dl, float bx0, float bx1, float by0, float by1, float frac, ImU32 fillCol) {
    if (by1 < by0) std::swap(by0, by1);

    bx0 = std::floor(bx0 + 0.5f); bx1 = bx0 + 2.0f;
    by0 = std::floor(by0 + 0.5f); by1 = std::floor(by1 + 0.5f);
    float h = by1 - by0;
    if (h <= 1.0f) return;
    if (frac < 0.f) frac = 0.f;
    if (frac > 1.f) frac = 1.f;

    dl->AddRectFilled(ImVec2(bx0, by0), ImVec2(bx1, by1), IM_COL32(0, 0, 0, 200));

    float fillTop = by1 - h * frac;
    if (fillTop < by0) fillTop = by0;
    if (frac > 0.001f)
        dl->AddRectFilled(ImVec2(bx0, fillTop), ImVec2(bx1, by1), fillCol);

    const ImU32 ob = IM_COL32(0, 0, 0, 255);
    dl->AddRectFilled(ImVec2(bx0 - 1, by0 - 1), ImVec2(bx1 + 1, by0), ob);
    dl->AddRectFilled(ImVec2(bx0 - 1, by1), ImVec2(bx1 + 1, by1 + 1), ob);
    dl->AddRectFilled(ImVec2(bx0 - 1, by0), ImVec2(bx0, by1), ob);
    dl->AddRectFilled(ImVec2(bx1, by0), ImVec2(bx1 + 1, by1), ob);
}

void RenderSkeleton(ImDrawList* dl, const PlayerCache::LimbAddrs& l, const RBX::Mat4& v) {
    if (l.r6) {
        if (!l.head || !l.torso)
            return;
        const auto hp = PartPos(l.head);
        const auto tp = PartPos(l.torso);
        Bone(dl, hp, tp, v);
        const std::uintptr_t limbs[] = {l.lArm, l.rArm, l.lLeg, l.rLeg};
        for (auto addr : limbs) {
            if (!addr)
                continue;
            Bone(dl, tp, PartPos(addr), v);
        }
        return;
    }
    if (!l.head || !l.upperTorso)
        return;
    const auto hp = PartPos(l.head);
    const auto up = PartPos(l.upperTorso);
    Bone(dl, hp, up, v);
    if (l.lowerTorso) {
        const auto lp = PartPos(l.lowerTorso);
        Bone(dl, up, lp, v);
        const std::uintptr_t leftArm[] = {l.lUpperArm, l.lLowerArm, l.lHand};
        const std::uintptr_t rightArm[] = {l.rUpperArm, l.rLowerArm, l.rHand};
        const std::uintptr_t leftLeg[] = {l.lUpperLeg, l.lLowerLeg, l.lFoot};
        const std::uintptr_t rightLeg[] = {l.rUpperLeg, l.rLowerLeg, l.rFoot};
        auto chain = [&](const std::uintptr_t* c) {
            std::uintptr_t prev = 0;
            RBX::Vec3 prevPos{};
            for (int i = 0; i < 3; ++i) {
                if (!c[i])
                    return;
                const RBX::Vec3 p = PartPos(c[i]);
                if (prev)
                    Bone(dl, prevPos, p, v);
                prev = c[i];
                prevPos = p;
            }
        };
        BoneAddr(dl, l.upperTorso, l.lUpperArm, v);
        BoneAddr(dl, l.upperTorso, l.rUpperArm, v);
        chain(leftArm);
        chain(rightArm);
        BoneAddr(dl, l.lowerTorso, l.lUpperLeg, v);
        BoneAddr(dl, l.lowerTorso, l.rUpperLeg, v);
        chain(leftLeg);
        chain(rightLeg);
    }
}

void BoneAddrCached(ImDrawList* dl, const RBX::Vec3& pa, const RBX::Vec3& pb, const RBX::Mat4& v, ImU32 col, float thickness, bool outline) {
    if ((pa.X == 0 && pa.Y == 0 && pa.Z == 0) || (pb.X == 0 && pb.Y == 0 && pb.Z == 0))
        return;
    BoneCached(dl, pa, pb, v, col, thickness, outline);
}

void RenderSkeletonCached(ImDrawList* dl, const PlayerCache::LimbAddrs& l, const RBX::Mat4& v, ImU32 col, float thickness, bool outline) {
    if (l.r6) {
        if (!l.head || !l.torso)
            return;
        const auto hp = PartPos(l.head);
        const auto tp = PartPos(l.torso);
        BoneCached(dl, hp, tp, v, col, thickness, outline);
        const std::uintptr_t limbs[] = {l.lArm, l.rArm, l.lLeg, l.rLeg};
        for (auto addr : limbs) {
            if (!addr)
                continue;
            BoneCached(dl, tp, PartPos(addr), v, col, thickness, outline);
        }
        return;
    }
    if (!l.head || !l.upperTorso)
        return;
    const auto hp = PartPos(l.head);
    const auto up = PartPos(l.upperTorso);
    BoneCached(dl, hp, up, v, col, thickness, outline);
    if (l.lowerTorso) {
        const auto lp = PartPos(l.lowerTorso);
        BoneCached(dl, up, lp, v, col, thickness, outline);
        const std::uintptr_t leftArm[] = {l.lUpperArm, l.lLowerArm, l.lHand};
        const std::uintptr_t rightArm[] = {l.rUpperArm, l.rLowerArm, l.rHand};
        const std::uintptr_t leftLeg[] = {l.lUpperLeg, l.lLowerLeg, l.lFoot};
        const std::uintptr_t rightLeg[] = {l.rUpperLeg, l.rLowerLeg, l.rFoot};
        auto chain = [&](const std::uintptr_t* c) {
            std::uintptr_t prev = 0;
            RBX::Vec3 prevPos{};
            for (int i = 0; i < 3; ++i) {
                if (!c[i])
                    return;
                const RBX::Vec3 p = PartPos(c[i]);
                if (prev)
                    BoneCached(dl, prevPos, p, v, col, thickness, outline);
                prev = c[i];
                prevPos = p;
            }
        };
        if (l.lUpperArm) BoneAddrCached(dl, up, PartPos(l.lUpperArm), v, col, thickness, outline);
        if (l.rUpperArm) BoneAddrCached(dl, up, PartPos(l.rUpperArm), v, col, thickness, outline);
        chain(leftArm);
        chain(rightArm);
        if (l.lUpperLeg) BoneAddrCached(dl, lp, PartPos(l.lUpperLeg), v, col, thickness, outline);
        if (l.rUpperLeg) BoneAddrCached(dl, lp, PartPos(l.rUpperLeg), v, col, thickness, outline);
        chain(leftLeg);
        chain(rightLeg);
    }
}

void RenderOnePlayer(ImDrawList* dl, const RBX::Mat4& v, const PlayerCache::CachedPlayer& p, const RBX::Vec3& lp,
    ImFont* font, float espSize, const ImVec2& disp,
    ImU32 boxCol, ImU32 nameCol, ImU32 distCol, ImU32 toolCol, ImU32 flagsCol,
    ImU32 headDotCol, ImU32 viewDirCol, ImU32 skelCol,
    ImU32 murderCol, ImU32 sheriffCol, ImU32 innocentCol,
    bool needFlagsVel, bool needPrim) {
    if (!p.isValid || !p.headAddr || !p.rootPartAddr)
        return;
    if (p.maxHealth > 0.0f && p.health <= 0.0f)
        return;
    if (p.humanoidAddr) {
        const float liveHp = memory->read<float>(p.humanoidAddr + Offsets::Humanoid::Health);
        if (!std::isfinite(liveHp) || liveHp <= 0.0f)
            return;
    } else if (!std::isfinite(p.health)) {
        return;
    }
    const auto cp = memory->read<std::uintptr_t>(p.characterAddr + Offsets::Instance::Parent);
    if (!cp)
        return;
    if (PlayersTab::IsFriend(p.name))
        return;
    const bool isTarget = PlayersTab::IsMarked(p.name);
    if (isTarget) {
        boxCol = IM_COL32(89, 207, 211, 255);
        nameCol = IM_COL32(89, 207, 211, 255);
    }
    const PlayerCache::LimbAddrs& limbs = PlayerCache::GetLimbs(p.characterAddr, false);
    const bool r6 = p.isR6;
    const RBX::Vec3 hp = PartPos(p.headAddr);
    const RBX::Vec3 rp = PartPos(p.rootPartAddr);
    if ((hp.X == 0 && hp.Y == 0 && hp.Z == 0) || (rp.X == 0 && rp.Y == 0 && rp.Z == 0))
        return;
    if (memory->read<std::uintptr_t>(p.headAddr + Offsets::Instance::Parent) != p.characterAddr)
        return;
    if (memory->read<std::uintptr_t>(p.rootPartAddr + Offsets::Instance::Parent) != p.characterAddr)
        return;
    const float dist = sqrtf((rp.X - lp.X) * (rp.X - lp.X) + (rp.Y - lp.Y) * (rp.Y - lp.Y) + (rp.Z - lp.Z) * (rp.Z - lp.Z));
    float x0, x1, y0, y1;
    if (variables::ESP::boxMode == 1) {
        if (!DynamicBoundsEx(limbs, r6, v, hp, rp, x0, x1, y0, y1))
            return;
    } else {
        RBX::Vec3 top3{hp.X, hp.Y + 0.5f, hp.Z};
        RBX::Vec3 bot3{rp.X, rp.Y - (r6 ? 3.0f : 2.5f), rp.Z};
        RBX::Vec2 top{}, bot{};
        if (!ProjectPoint(top3, v, top) || !ProjectPoint(bot3, v, bot))
            return;
        const float h = bot.Y - top.Y;
        const float w = h * 0.55f;
        x0 = top.X - w * 0.5f;
        x1 = top.X + w * 0.5f;
        y0 = top.Y;
        y1 = bot.Y;
    }
    if (!SanityBox(dist, x0, y0, x1, y1, EspFocal(disp.y)))
        return;
    if (x0 < -500 || y0 < -500 || x1 > disp.x + 500 || y1 > disp.y + 500)
        return;
    if (variables::ESP::boxes)
        DrawBoxFill(dl, x0, y0, x1, y1);
    if (variables::ESP::skeleton)
        RenderSkeletonCached(dl, limbs, v, skelCol, variables::ESP::skeletonThickness, variables::ESP::skeletonOutline);
    if (variables::ESP::boxes)
        DrawBoxCol(dl, x0, y0, x1, y1, boxCol);
    if (variables::ESP::healthBar && p.maxHealth > 0) {
        float f = (p.maxHealth > 0.01f) ? (p.health / p.maxHealth) : 1.f;
        if (!std::isfinite(f)) f = 1.f;
        f = std::clamp(f, 0.0f, 1.0f);
        ImU32 hcol = IM_COL32((int)(255.0f * (1.0f - f)), (int)(255.0f * f), 0, 255);
        const float bx0 = std::floor(x0 - 4.0f);
        const float bx1 = bx0 + 2.0f;
        DrawHealthBar(dl, bx0, bx1, y0, y1, f, hcol);
    }
    if (variables::ESP::names) {
        const ImVec2 ts = font->CalcTextSizeA(espSize, FLT_MAX, 0.0f, p.name.c_str());
        DrawOutlinedText(dl, ImVec2((x0 + x1) * 0.5f - ts.x * 0.5f, y0 - ts.y - 2), p.name, nameCol);
    }
    if (isTarget) {
        const std::string tgtTag = "[TARGET]";
        const ImVec2 tts = font->CalcTextSizeA(espSize, FLT_MAX, 0.0f, tgtTag.c_str());
        float ty = y0 - tts.y - 2.0f;
        if (variables::ESP::names) {
            const ImVec2 nts = font->CalcTextSizeA(espSize, FLT_MAX, 0.0f, p.name.c_str());
            ty -= (nts.y + 2.0f);
        }
        DrawOutlinedText(dl, ImVec2((x0 + x1) * 0.5f - tts.x * 0.5f, ty), tgtTag, IM_COL32(89, 207, 211, 255));
    }
    if (variables::ESP::distance) {
        std::string dt = std::to_string(static_cast<int>(dist)) + "m";
        const ImVec2 ts = font->CalcTextSizeA(espSize, FLT_MAX, 0.0f, dt.c_str());
        DrawOutlinedText(dl, ImVec2((x0 + x1) * 0.5f - ts.x * 0.5f, y1 + 2), dt, distCol);
    }
    if (variables::ESP::tool && p.tool != "None" && !p.tool.empty()) {
        const ImVec2 ts = font->CalcTextSizeA(espSize, FLT_MAX, 0.0f, p.tool.c_str());
        float ty = y1 + 2;
        if (variables::ESP::distance)
            ty += ts.y + 3.0f;
        DrawOutlinedText(dl, ImVec2((x0 + x1) * 0.5f - ts.x * 0.5f, ty), p.tool, toolCol);
    }
    std::uintptr_t rootPrim = 0;
    RBX::Vec3 vel{};
    bool haveVel = false;
    RBX::CFrame cf{};
    bool haveCf = false;
    if (needPrim) {
        rootPrim = memory->read<std::uintptr_t>(p.rootPartAddr + Offsets::BasePart::Primitive);
        if (rootPrim) {
            if (needFlagsVel) {
                vel = memory->read<RBX::Vec3>(rootPrim + Offsets::Primitive::AssemblyLinearVelocity);
                haveVel = true;
            }
            if (variables::ESP::viewDirection) {
                cf = memory->read<RBX::CFrame>(rootPrim + Offsets::Primitive::Rotation);
                haveCf = true;
            }
        }
    }
    if (variables::ESP::flags) {
        const float speedH = haveVel ? sqrtf(vel.X * vel.X + vel.Z * vel.Z) : 0.0f;
        const bool* sel = variables::ESP::flagSel;
        float fx = x1 + 4.0f;
        float fy = y0;
        auto flagText = [&](const std::string& t) {
            DrawOutlinedText(dl, ImVec2(fx, fy), t, flagsCol);
            fy += espSize + 2.0f;
        };
        if (sel[0]) {
            if (vel.Y > 2.0f)
                flagText("Jumping");
            else if (vel.Y < -2.0f)
                flagText("Falling");
            else if (speedH > 1.5f)
                flagText("Running");
            else
                flagText("Idle");
        }
        if (sel[1])
            flagText(p.isR6 ? "R6" : "R15");
        if (sel[2] && p.maxHealth > 0)
            flagText(std::to_string((int)(p.health / p.maxHealth * 100.0f)) + "% HP");
        if (sel[3] && p.tool != "None" && !p.tool.empty())
            flagText(p.tool);
        if (sel[4])
            flagText(std::to_string((int)dist) + "m");
        if (sel[5]) {
            char vbuf[32];
            std::snprintf(vbuf, sizeof(vbuf), "%.1f u/s", sqrtf(vel.X * vel.X + vel.Y * vel.Y + vel.Z * vel.Z));
            flagText(vbuf);
        }
        if (sel[6] && !p.teamName.empty())
            flagText("Team: " + p.teamName);
        if (sel[7]) {
            const char* rt = p.role == 1 ? "Murder" : p.role == 2 ? "Sheriff" : "Innocent";
            ImU32 rc = p.role == 1 ? murderCol : p.role == 2 ? sheriffCol : innocentCol;
            DrawOutlinedText(dl, ImVec2(fx, fy), rt, rc);
            fy += espSize + 2.0f;
        }
    }
    if (variables::ESP::headDot) {
        ImVec2 hs{};
        if (ToScreen(hp, v, hs)) {
            float r = variables::ESP::headDotSize * std::clamp(200.0f / (std::max)(dist, 10.0f), 0.6f, 1.4f);
            dl->AddCircleFilled(ImVec2(hs.x, hs.y), r, headDotCol, 12);
            dl->AddCircle(ImVec2(hs.x, hs.y), r + 1.0f, IM_COL32(0, 0, 0, 180), 12, 1.0f);
        }
    }
    if (variables::ESP::viewDirection) {
        if (rootPrim && haveCf) {
            const float* rot = cf.data;
            RBX::Vec3 look{-rot[2], -rot[5], -rot[8]};
            const float lenSq = look.X * look.X + look.Y * look.Y + look.Z * look.Z;
            if (lenSq > 1e-6f) {
                const float inv = 1.0f / sqrtf(lenSq);
                look.X *= inv;
                look.Y *= inv;
                look.Z *= inv;
                const float L = std::clamp(variables::ESP::viewDirLength, 1.0f, 50.0f);
                RBX::Vec3 endW{hp.X + look.X * L, hp.Y + look.Y * L, hp.Z + look.Z * L};
                ImVec2 s0{};
                ImVec2 s1{};
                if (ToScreen(hp, v, s0) && ToScreen(endW, v, s1)) {
                    DrawLine(dl, s0, s1, viewDirCol, 2.0f, true);
                    dl->AddCircleFilled(s1, 2.5f, viewDirCol, 8);
                }
            }
        }
    }
}

void RenderESP(ImDrawList* dl, const RBX::Mat4& v) {
    if (!variables::ESP::enabled)
        return;

    if (!variables::ESP::boxes && !variables::ESP::names && !variables::ESP::distance &&
        !variables::ESP::healthBar && !variables::ESP::skeleton && !variables::ESP::tool &&
        !variables::ESP::flags && !variables::ESP::headDot && !variables::ESP::viewDirection &&
        !variables::ESP::localPlayer)
        return;
    const PosFrame posFrame;
    const ImVec2 disp = ImGui::GetIO().DisplaySize;

    const ImU32 boxCol = ToU32(variables::ESP::boxColor);
    const ImU32 nameCol = ToU32(variables::ESP::nameColor);
    const ImU32 distCol = ToU32(variables::ESP::distanceColor);
    const ImU32 toolCol = ToU32(variables::ESP::toolColor);
    const ImU32 flagsCol = ToU32(variables::ESP::flagsColor);
    const ImU32 headDotCol = ToU32(variables::ESP::headDotColor);
    const ImU32 viewDirCol = ToU32(variables::ESP::viewDirColor);
    const ImU32 skelCol = ToU32(variables::ESP::skeletonColor);
    const ImU32 murderCol = ToU32(variables::World::murderColor);
    const ImU32 sheriffCol = ToU32(variables::World::sheriffColor);
    const ImU32 innocentCol = ToU32(variables::World::innocentColor);
    const bool needFlagsVel = variables::ESP::flags && (variables::ESP::flagSel[0] || variables::ESP::flagSel[5]);
    const bool needPrim = needFlagsVel || variables::ESP::viewDirection;
    ImFont* font = EspFont();
    const float espSize = EspSize();
    std::unordered_set<std::string> drawn;
    auto alreadyDrawn = [&](const std::string& n) {
        return !n.empty() && drawn.find(n) != drawn.end();
    };
    if (OpsCache::viewmodelsAddr) {
        for (auto& p : OpsCache::players) {
            if (!p.isValid)
                continue;
            if (alreadyDrawn(p.name))
                continue;
            if (Keys::TeamCheckOn() && p.teamAddr && p.teamAddr == OpsCache::localTeam && !PlayersTab::IsMarked(p.name))
                continue;
            RenderOnePlayer(dl, v, p, OpsCache::localPos, font, espSize, disp,
                boxCol, nameCol, distCol, toolCol, flagsCol, headDotCol, viewDirCol, skelCol,
                murderCol, sheriffCol, innocentCol, needFlagsVel, needPrim);
            if (!p.name.empty())
                drawn.insert(p.name);
        }
    } else if (CbCache::charactersAddr) {
        for (auto& p : CbCache::players) {
            if (!p.isValid)
                continue;
            if (alreadyDrawn(p.name))
                continue;
            if (Keys::TeamCheckOn() && p.teamAddr && p.teamAddr == CbCache::localTeam && !PlayersTab::IsMarked(p.name))
                continue;
            RenderOnePlayer(dl, v, p, CbCache::localPos, font, espSize, disp,
                boxCol, nameCol, distCol, toolCol, flagsCol, headDotCol, viewDirCol, skelCol,
                murderCol, sheriffCol, innocentCol, needFlagsVel, needPrim);
            if (!p.name.empty())
                drawn.insert(p.name);
        }
    } else if (MlCache::charactersAddr) {
        for (auto& p : MlCache::players) {
            if (!p.isValid)
                continue;
            if (alreadyDrawn(p.name))
                continue;
            if (Keys::TeamCheckOn() && p.teamAddr && p.teamAddr == MlCache::localTeam && !PlayersTab::IsMarked(p.name))
                continue;
            RenderOnePlayer(dl, v, p, MlCache::localPos, font, espSize, disp,
                boxCol, nameCol, distCol, toolCol, flagsCol, headDotCol, viewDirCol, skelCol,
                murderCol, sheriffCol, innocentCol, needFlagsVel, needPrim);
            if (!p.name.empty())
                drawn.insert(p.name);
        }
    } else {
        for (auto& p : PlayerCache::players) {
            if (!p.isValid)
                continue;
            if (alreadyDrawn(p.name))
                continue;
            RenderOnePlayer(dl, v, p, PlayerCache::localPlayerPos, font, espSize, disp,
                boxCol, nameCol, distCol, toolCol, flagsCol, headDotCol, viewDirCol, skelCol,
                murderCol, sheriffCol, innocentCol, needFlagsVel, needPrim);
            if (!p.name.empty())
                drawn.insert(p.name);
        }
    }
    if (!PfCache::players.empty()) {
        const bool tc = Keys::TeamCheckOn();
        const RBX::Vec3 lp = PfCache::localPos;
        const float pfFocal = EspFocal(disp.y);
        for (auto& p : PfCache::players) {
            if (!p.isValid || !p.headAddr || !p.torsoAddr)
                continue;
            if (alreadyDrawn(p.name))
                continue;
            if (!std::isfinite(p.health) || p.health <= 0.0f)
                continue;
            if (PlayersTab::IsFriend(p.name))
                continue;
            if (tc && PfCache::IsTeammate(p) && !PlayersTab::IsMarked(p.name))
                continue;
            const RBX::Vec3 hp = PartPos(p.headAddr);
            const RBX::Vec3 rp = PartPos(p.torsoAddr);
            if ((hp.X == 0 && hp.Y == 0 && hp.Z == 0) || (rp.X == 0 && rp.Y == 0 && rp.Z == 0))
                continue;
            const float dx = rp.X - lp.X, dy = rp.Y - lp.Y, dz = rp.Z - lp.Z;
            const float dist = sqrtf(dx * dx + dy * dy + dz * dz);
            float x0, x1, y0, y1;
            if (variables::ESP::boxMode == 1) {
                RBX::Vec3 pts[20];
                int n = 0;
                pts[n++] = {hp.X, hp.Y + 0.6f, hp.Z};
                pts[n++] = hp;
                pts[n++] = rp;
                for (auto addr : p.limbAddrs) {
                    if (!addr || n >= 17)
                        continue;
                    const RBX::Vec3 q = PartPos(addr);
                    if (q.X == 0 && q.Y == 0 && q.Z == 0)
                        continue;
                    pts[n++] = q;
                }
                pts[n++] = {rp.X, rp.Y - 3.0f, rp.Z};
                bool any = false;
                float mnX = 1e9f, mxX = -1e9f, mnY = 1e9f, mxY = -1e9f;
                for (int i = 0; i < n; ++i) {
                    RBX::Vec2 s{};
                    if (!ProjectPoint(pts[i], v, s))
                        continue;
                    any = true;
                    if (s.X < mnX) mnX = s.X;
                    if (s.X > mxX) mxX = s.X;
                    if (s.Y < mnY) mnY = s.Y;
                    if (s.Y > mxY) mxY = s.Y;
                }
                if (!any)
                    continue;
                const float pad = std::clamp((mxY - mnY) * 0.06f, 3.0f, 10.0f);
                x0 = mnX - pad * 0.8f;
                x1 = mxX + pad * 0.8f;
                y0 = mnY - pad;
                y1 = mxY + pad;
                if (!(x1 > x0 && y1 > y0))
                    continue;
            } else {
                RBX::Vec3 top3{hp.X, hp.Y + 0.5f, hp.Z};
                RBX::Vec3 bot3{rp.X, rp.Y - 3.0f, rp.Z};
                RBX::Vec2 top{}, bot{};
                if (!ProjectPoint(top3, v, top) || !ProjectPoint(bot3, v, bot))
                    continue;
                const float h = bot.Y - top.Y;
                const float w = h * 0.55f;
                x0 = top.X - w * 0.5f;
                x1 = top.X + w * 0.5f;
                y0 = top.Y;
                y1 = bot.Y;
            }
            if (!SanityBox(dist, x0, y0, x1, y1, pfFocal))
                continue;
            if (x0 < -500 || y0 < -500 || x1 > disp.x + 500 || y1 > disp.y + 500)
                continue;
            if (variables::ESP::boxes)
                DrawBoxFill(dl, x0, y0, x1, y1);
            if (variables::ESP::skeleton) {
                BoneCached(dl, hp, rp, v, skelCol, variables::ESP::skeletonThickness, variables::ESP::skeletonOutline);
                for (auto addr : p.limbAddrs) {
                    if (!addr)
                        continue;
                    BoneCached(dl, rp, PartPos(addr), v, skelCol, variables::ESP::skeletonThickness, variables::ESP::skeletonOutline);
                }
            }
            if (variables::ESP::boxes)
                DrawBoxCol(dl, x0, y0, x1, y1, boxCol);
            if (variables::ESP::healthBar) {
                float f = std::clamp(p.health, 0.0f, 1.0f);
                ImU32 hcol = IM_COL32((int)(255.0f * (1.0f - f)), (int)(255.0f * f), 0, 255);
                const float bx0 = std::floor(x0 - 4.0f);
                const float bx1 = bx0 + 2.0f;
                DrawHealthBar(dl, bx0, bx1, y0, y1, f, hcol);
            }
            if (variables::ESP::names) {
                const ImVec2 ts = font->CalcTextSizeA(espSize, FLT_MAX, 0.0f, p.name.c_str());
                DrawOutlinedText(dl, ImVec2((x0 + x1) * 0.5f - ts.x * 0.5f, y0 - ts.y - 2), p.name, nameCol);
            }
            if (variables::ESP::distance) {
                std::string dt = std::to_string(static_cast<int>(dist)) + "m";
                const ImVec2 ts = font->CalcTextSizeA(espSize, FLT_MAX, 0.0f, dt.c_str());
                DrawOutlinedText(dl, ImVec2((x0 + x1) * 0.5f - ts.x * 0.5f, y1 + 2), dt, distCol);
            }
            if (variables::ESP::headDot) {
                ImVec2 hs{};
                if (ToScreen(hp, v, hs)) {
                    float r = variables::ESP::headDotSize * std::clamp(200.0f / (std::max)(dist, 10.0f), 0.6f, 1.4f);
                    dl->AddCircleFilled(ImVec2(hs.x, hs.y), r, headDotCol, 12);
                    dl->AddCircle(ImVec2(hs.x, hs.y), r + 1.0f, IM_COL32(0, 0, 0, 180), 12, 1.0f);
                }
            }
            if (!p.name.empty())
                drawn.insert(p.name);
        }
    }
    if (variables::ESP::localPlayer) {
        auto localChar = Globals::localPlayer.GetModelRef();
        if (!localChar.Addr)
            return;
        const PlayerCache::LimbAddrs& limbs = PlayerCache::GetLimbs(localChar.Addr, false);
        if (!limbs.head || !limbs.hrp)
            return;
        const bool r6 = limbs.r6;
        auto hum = RBX::RbxInstance{limbs.humanoid};
        const RBX::Vec3 hp = PartPos(limbs.head);
        const RBX::Vec3 rp = PartPos(limbs.hrp);
        if ((hp.X == 0 && hp.Y == 0 && hp.Z == 0) || (rp.X == 0 && rp.Y == 0 && rp.Z == 0))
            return;
        {
            float x0, x1, y0, y1;
            if (variables::ESP::boxMode == 1) {
                if (!DynamicBoundsEx(limbs, r6, v, hp, rp, x0, x1, y0, y1))
                    return;
            } else {
                RBX::Vec3 top3{hp.X, hp.Y + 0.5f, hp.Z};
                RBX::Vec3 bot3{rp.X, rp.Y - (r6 ? 3.0f : 2.5f), rp.Z};
                RBX::Vec2 top{}, bot{};
                if (!ProjectPoint(top3, v, top) || !ProjectPoint(bot3, v, bot))
                    return;
                const float h = bot.Y - top.Y;
                const float w = h * 0.55f;
                x0 = top.X - w * 0.5f;
                x1 = top.X + w * 0.5f;
                y0 = top.Y;
                y1 = bot.Y;
            }
            if (!SanityBox(0.0f, x0, y0, x1, y1, EspFocal(disp.y)))
                return;
            {
                if (variables::ESP::boxes)
                    DrawBoxFill(dl, x0, y0, x1, y1);
                if (variables::ESP::healthBar && hum.Addr) {
                    const float maxHp = memory->read<float>(hum.Addr + Offsets::Humanoid::MaxHealth);
                    if (maxHp > 0.01f) {
                        const float curHp = memory->read<float>(hum.Addr + Offsets::Humanoid::Health);
                        float f = curHp / maxHp;
                        if (!std::isfinite(f)) f = 1.f;
                        f = std::clamp(f, 0.0f, 1.0f);
                        ImU32 hcol = IM_COL32((int)(255.0f * (1.0f - f)), (int)(255.0f * f), 0, 255);
                        const float bx0 = std::floor(x0 - 4.0f);
                        const float bx1 = bx0 + 2.0f;
                        DrawHealthBar(dl, bx0, bx1, y0, y1, f, hcol);
                    }
                }
                if (variables::ESP::skeleton)
                    RenderSkeletonCached(dl, limbs, v, skelCol, variables::ESP::skeletonThickness, variables::ESP::skeletonOutline);
                if (variables::ESP::boxes)
                    DrawBoxCol(dl, x0, y0, x1, y1, boxCol);
                if (variables::ESP::names) {
                    const std::string name = Globals::localPlayer.GetName();
                    const ImVec2 ts = font->CalcTextSizeA(espSize, FLT_MAX, 0.0f, name.c_str());
                    DrawOutlinedText(dl, ImVec2((x0 + x1) * 0.5f - ts.x * 0.5f, y0 - ts.y - 2), name, nameCol);
                }
                if (variables::ESP::distance) {
                    std::string dt("0m");
                    const ImVec2 ts = font->CalcTextSizeA(espSize, FLT_MAX, 0.0f, dt.c_str());
                    DrawOutlinedText(dl, ImVec2((x0 + x1) * 0.5f - ts.x * 0.5f, y1 + 2), dt, distCol);
                }
            }
    }
}
}
}

void Visuals::RenderMeshChams(ImDrawList* dl, const RBX::Mat4& v) {
    if (!variables::ESP::enabled || !variables::ESP::meshChams)
        return;
    Cheat::Visuals::MeshCache::Get().Refresh();
    Mesh::Matrix4x4 view{};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            view.m[i][j] = v.data[i * 4 + j];
    RBX::Vec3 cp{};
    if (Globals::camera.Addr)
        cp = Globals::camera.GetPos();
    Mesh::Vector3 cam{cp.X, cp.Y, cp.Z};
    const ImVec2 disp = ImGui::GetIO().DisplaySize;
    Mesh::Vector2 vp{disp.x, disp.y};
    const float t = (float)(GetTickCount64() / 1000.0);
    Cheat::Visuals::MeshDxShader::BeginFrame(view, cam, t);

    {}
    const ImVec4& fc = variables::ESP::chamsFillColor;
    const ImU32 fill = IM_COL32((int)(fc.x * 255.0f), (int)(fc.y * 255.0f), (int)(fc.z * 255.0f), (int)(fc.w * 255.0f));
    auto drawOne = [&](const PlayerCache::CachedPlayer& p, const RBX::Vec3& lp) {
        if (!p.isValid || !p.characterAddr || !p.headAddr)
            return;

        const RBX::Vec3 hp = PartPos(p.headAddr);
        if (hp.X == 0 && hp.Y == 0 && hp.Z == 0)
            return;
        const auto s = W2S::WorldToScreen(hp, v);
        if ((s.X == 0 && s.Y == 0) || s.X < -100 || s.Y < -100 || s.X > disp.x + 100 || s.Y > disp.y + 100)
            return;

        const float dx = hp.X - lp.X;
        const float dy = hp.Y - lp.Y;
        const float dz = hp.Z - lp.Z;
        const float d2 = dx * dx + dy * dy + dz * dz;
        const bool fullDetail = d2 < 120.0f * 120.0f;
        Cheat::Visuals::MeshChams::Draw(dl, p.characterAddr, view, vp, 1.0f, 1.0f, fill, fullDetail);
    };

    if (OpsCache::viewmodelsAddr) {
        for (auto& p : OpsCache::players)
            drawOne(p, OpsCache::localPos);
    } else if (CbCache::charactersAddr) {
        for (auto& p : CbCache::players)
            drawOne(p, CbCache::localPos);
    } else if (MlCache::charactersAddr) {
        for (auto& p : MlCache::players)
            drawOne(p, MlCache::localPos);
    } else {
        for (auto& p : PlayerCache::players)
            drawOne(p, PlayerCache::localPlayerPos);
    }
}
