#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "aim.h"
#include "fallen_prediction.h"
#include "movement_history.h"
#include "viewport_silent.h"
#include "magic.h"
#include "pf_silent.h"
#include "../../cache/workspace.h"
#include "../../cache/worldcache.h"
#include "../../cache/pf_cache.h"
#include "../../cache/cb_cache.h"
#include "../../cache/ops_cache.h"
#include "../players/players.h"
#include <mutex>
#include <vector>
#include "../../keys/keys.h"
#include "../../net/ping.h"
#include "../../globals/globals.h"
#include "../../../memory/memory.h"
#include "../../../sdk/offsets.h"
#include "../../../../ext/imgui/imgui.h"
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#include <random>

namespace {
bool PfPartScreen(const PfCache::PfPlayer& p, const RBX::Mat4& view, const RBX::Vec2& center, float maxDist, const RBX::Vec3& camPos, bool checkVis, RBX::Vec2& outScreen, RBX::Vec3& outWorld) {
    static float sw = 0.0f;
    static float sh = 0.0f;
    if (sw <= 0.0f) {
        sw = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        sh = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
    }
    const std::uintptr_t cands[2] = {p.headAddr, p.torsoAddr};
    for (int i = 0; i < 2; ++i) {
        if (!cands[i])
            continue;
        const RBX::Vec3 w = Aimbot::PartWorldPos(cands[i]);
        if (w.X == 0 && w.Y == 0 && w.Z == 0)
            continue;
        const RBX::Vec2 s = W2S::WorldToScreen(w, view);
        const bool isMarkedOrInfinite = (maxDist >= 1e8f);
        if (!isMarkedOrInfinite) {
            if (s.X == 0 && s.Y == 0)
                continue;
            if (s.X < 0 || s.Y < 0 || s.X > sw || s.Y > sh)
                continue;
            if (Aimbot::GetDistance2D(center, s) >= maxDist)
                continue;
        }
        if (checkVis && !WorkspaceCache::IsVisible(camPos, w))
            continue;
        outScreen = s;
        outWorld = w;
        return true;
    }
    return false;
}
RBX::Vec3 ApplyRaycastSpread(const RBX::Vec3& origin, const RBX::Vec3& target, float spreadAmount) {
    if (spreadAmount <= 0.001f)
        return target;

    static std::mt19937 rng(1337);
    std::uniform_real_distribution<float> distAngle(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> distRadius(0.0f, 1.0f);

    const float angle = distAngle(rng);
    const float radius = std::sqrt(distRadius(rng)) * spreadAmount;

    RBX::Vec3 forward = {target.X - origin.X, target.Y - origin.Y, target.Z - origin.Z};
    const float len = std::sqrt(forward.X * forward.X + forward.Y * forward.Y + forward.Z * forward.Z);
    if (len < 1e-4f)
        return target;
    forward.X /= len;
    forward.Y /= len;
    forward.Z /= len;

    const RBX::Vec3 upRef = (std::abs(forward.Y) < 0.99f) ? RBX::Vec3{0.0f, 1.0f, 0.0f} : RBX::Vec3{1.0f, 0.0f, 0.0f};

    RBX::Vec3 right = {
        forward.Y * upRef.Z - forward.Z * upRef.Y,
        forward.Z * upRef.X - forward.X * upRef.Z,
        forward.X * upRef.Y - forward.Y * upRef.X
    };
    const float rlen = std::sqrt(right.X * right.X + right.Y * right.Y + right.Z * right.Z);
    if (rlen > 1e-4f) {
        right.X /= rlen;
        right.Y /= rlen;
        right.Z /= rlen;
    }

    RBX::Vec3 up = {
        right.Y * forward.Z - right.Z * forward.Y,
        right.Z * forward.X - right.X * forward.Z,
        right.X * forward.Y - right.Y * forward.X
    };

    const float offsetX = std::cos(angle) * radius;
    const float offsetY = std::sin(angle) * radius;

    return {
        target.X + right.X * offsetX + up.X * offsetY,
        target.Y + right.Y * offsetX + up.Y * offsetY,
        target.Z + right.Z * offsetX + up.Z * offsetY
    };
}
}

namespace Aimbot {
void MoveMouse(float x, float y) {
    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = MOUSEEVENTF_MOVE;
    in.mi.dx = static_cast<LONG>(x);
    in.mi.dy = static_cast<LONG>(y);
    SendInput(1, &in, sizeof(in));
}

void AutoClick() {
    INPUT in[2]{};
    in[0].type = INPUT_MOUSE;
    in[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    in[1].type = INPUT_MOUSE;
    in[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, in, sizeof(INPUT));
}

float GetDistance2D(const RBX::Vec2& a, const RBX::Vec2& b) {
    const float dx = a.X - b.X;
    const float dy = a.Y - b.Y;
    return sqrtf(dx * dx + dy * dy);
}

bool IsAimKeyDown(int vk) {
    if (vk <= 0)
        return false;
    if (vk >= ImGuiKey_NamedKey_BEGIN && vk < ImGuiKey_NamedKey_END) {
        if (ImGui::IsKeyDown((ImGuiKey)vk)) return true;
        int m = 0;
        if (vk >= ImGuiKey_A && vk <= ImGuiKey_Z) m = 'A' + (vk - ImGuiKey_A);
        else if (vk >= ImGuiKey_0 && vk <= ImGuiKey_9) m = '0' + (vk - ImGuiKey_0);
        else if (vk >= ImGuiKey_F1 && vk <= ImGuiKey_F12) m = VK_F1 + (vk - ImGuiKey_F1);
        else if (vk == ImGuiKey_Space) m = VK_SPACE;
        if (m) return (GetAsyncKeyState(m) & 0x8000) != 0;
        return false;
    }
    if (vk == 1)
        return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (vk == 2)
        return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (vk == 4)
        return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
    if (vk == 5)
        return (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
    if (vk == 6)
        return (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

bool AimKeyActive(int key, int mode, bool& tog, bool& was) {
    if (key <= 0)
        return mode == 2;
    if (mode == 2)
        return true;
    const bool down = IsAimKeyDown(key);
    if (mode == 1) {
        if (down && !was)
            tog = !tog;
        was = down;
        return tog;
    }
    was = down;
    return down;
}

bool IsTargetVisible(const RBX::Vec3& worldPos) {
    if (!variables::Aimbot::visibleCheck)
        return true;
    if (!Globals::camera.Addr)
        return true;
    const RBX::Vec3 cam = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);
    if (cam.X == 0 && cam.Y == 0 && cam.Z == 0)
        return true;
    return WorkspaceCache::IsVisible(cam, worldPos);
}

void CollectHitboxParts(std::uintptr_t characterAddr, int hitbox, std::vector<std::uintptr_t>& out) {
    if (!characterAddr)
        return;
    RBX::RbxInstance ch{characterAddr};
    auto addFallback = [&](std::initializer_list<const char*> names) {
        for (const char* n : names) {
            auto part = ch.FindChild(n);
            if (part.Addr) {
                out.push_back(part.Addr);
                return;
            }
        }
    };
    switch (hitbox) {
    case 0: addFallback({"Head"}); break;
    case 1: addFallback({"Torso", "UpperTorso", "LowerTorso"}); break;
    case 2: addFallback({"Left Arm", "LeftUpperArm", "LeftLowerArm", "LeftHand"}); break;
    case 3: addFallback({"Right Arm", "RightUpperArm", "RightLowerArm", "RightHand"}); break;
    case 4: addFallback({"Left Leg", "LeftUpperLeg", "LeftLowerLeg", "LeftFoot"}); break;
    case 5: addFallback({"Right Leg", "RightUpperLeg", "RightLowerLeg", "RightFoot"}); break;
    case 6: addFallback({"HumanoidRootPart"}); break;
    default: break;
    }
    if (hitbox == 7 || out.empty()) {
        if (hitbox != 7)
            out.clear();

        static std::unordered_map<std::uintptr_t, std::pair<std::vector<std::uintptr_t>, std::chrono::steady_clock::time_point>> s_hitboxCache;
        const std::uintptr_t key = characterAddr ^ (std::uintptr_t)(hitbox + 1) * (std::uintptr_t)0x9e3779b9ull;
        const auto nowH = std::chrono::steady_clock::now();
        auto hit = s_hitboxCache.find(key);
        if (hit != s_hitboxCache.end() &&
            std::chrono::duration_cast<std::chrono::milliseconds>(nowH - hit->second.second).count() < 1000 &&
            !hit->second.first.empty()) {
            out.insert(out.end(), hit->second.first.begin(), hit->second.first.end());
            return;
        }
        std::vector<std::uintptr_t> stack{characterAddr};
        while (!stack.empty() && out.size() < 64) {
            const uintptr_t cur = stack.back();
            stack.pop_back();
            const uintptr_t prim = memory->read<uintptr_t>(cur + Offsets::BasePart::Primitive);
            if (prim && prim != 0xFFFFFFFFFFFFFFFFull && prim > 0x10000 && prim < 0x7FFFFFFF0000ull)
                out.push_back(cur);
            const uintptr_t start = memory->read<uintptr_t>(cur + Offsets::Instance::ChildrenStart);
            if (!start)
                continue;
            const uintptr_t end = memory->read<uintptr_t>(start + Offsets::Instance::ChildrenEnd);
            uintptr_t it = memory->read<uintptr_t>(start);
            if (!end || !it || end < it)
                continue;
            for (uintptr_t p = it, n = 0; p < end && n < 64; p += 0x10, ++n) {
                const uintptr_t child = memory->read<uintptr_t>(p);
                if (child)
                    stack.push_back(child);
            }
        }
        if (!out.empty()) {
            if (s_hitboxCache.size() >= 256)
                s_hitboxCache.clear();
            s_hitboxCache[key] = {out, nowH};
        }
    }
}

RBX::Vec3 PartWorldPos(std::uintptr_t partAddr) {
    if (!partAddr)
        return {};
    const uintptr_t prim = memory->read<uintptr_t>(partAddr + Offsets::BasePart::Primitive);
    if (!prim)
        return {};
    return memory->read<RBX::Vec3>(prim + Offsets::Primitive::Position);
}

void WriteMemoryAngles(const RBX::Vec3& targetWorld) {
    if (!Globals::camera.Addr)
        return;

    const RBX::CFrame cf = memory->read<RBX::CFrame>(Globals::camera.Addr + Offsets::Camera::Rotation);
    rbx::matrix3_t curRot;
    curRot.data[0] = cf.data[0]; curRot.data[1] = cf.data[1]; curRot.data[2] = cf.data[2];
    curRot.data[3] = cf.data[3]; curRot.data[4] = cf.data[4]; curRot.data[5] = cf.data[5];
    curRot.data[6] = cf.data[6]; curRot.data[7] = cf.data[7]; curRot.data[8] = cf.data[8];
    rbx::vector3_t camPos{cf.data[9], cf.data[10], cf.data[11]};
    if (camPos.x == 0.0f && camPos.y == 0.0f && camPos.z == 0.0f)
        return;
    rbx::vector3_t want(targetWorld.X - camPos.x, targetWorld.Y - camPos.y, targetWorld.Z - camPos.z);
    if (want.magnitude() < 1e-6f)
        return;
    want = want.normalize();
    rbx::vector3_t curLook(-curRot.data[2], -curRot.data[5], -curRot.data[8]);
    if (curLook.magnitude() < 1e-6f)
        curLook = want;
    curLook = curLook.normalize();
    float k = 1.0f / (variables::Aimbot::smoothing <= 0.01f ? 1.0f : variables::Aimbot::smoothing);
    k = std::clamp(k, 0.01f, 1.0f);
    rbx::vector3_t look = curLook + (want - curLook) * k;
    if (look.magnitude() < 1e-6f)
        return;
    look = look.normalize();
    rbx::vector3_t worldUp(0.0f, 1.0f, 0.0f);
    rbx::vector3_t right = look.cross(worldUp);
    if (right.magnitude() < 1e-6f) {
        worldUp = rbx::vector3_t(0.0f, 0.0f, 1.0f);
        right = look.cross(worldUp);
        if (right.magnitude() < 1e-6f)
            return;
    }
    right = right.normalize();
    rbx::vector3_t up = right.cross(look).normalize();
    rbx::vector3_t back = look * -1.0f;
    rbx::matrix3_t newRot;
    newRot.data[0] = right.x; newRot.data[1] = up.x; newRot.data[2] = back.x;
    newRot.data[3] = right.y; newRot.data[4] = up.y; newRot.data[5] = back.y;
    newRot.data[6] = right.z; newRot.data[7] = up.z; newRot.data[8] = back.z;
    memory->write<rbx::matrix3_t>(Globals::camera.Addr + Offsets::Camera::Rotation, newRot);
}

namespace {
bool BestPartScreen(const PlayerCache::CachedPlayer& plr, const RBX::Mat4& view, const RBX::Vec2& center, float maxDist, const RBX::Vec3& camPos, bool checkVis, RBX::Vec2& outScreen, RBX::Vec3& outWorld) {
    static float sw = 0.0f;
    static float sh = 0.0f;
    if (sw <= 0.0f) {
        sw = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        sh = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
    }
    const int mode = variables::Aimbot::aimTarget;
    if (mode != 7) {
        std::uintptr_t primary = 0;
        if (mode == 0)
            primary = plr.headAddr;
        else if (mode == 6)
            primary = plr.rootPartAddr;
        else {
            const PlayerCache::LimbAddrs& limbs = PlayerCache::GetLimbs(plr.characterAddr, false);
            if (mode == 1)
                primary = limbs.r6 ? limbs.torso : limbs.upperTorso;
            else if (mode == 2)
                primary = limbs.r6 ? limbs.lArm : limbs.lUpperArm;
            else if (mode == 3)
                primary = limbs.r6 ? limbs.rArm : limbs.rUpperArm;
            else if (mode == 4)
                primary = limbs.r6 ? limbs.lLeg : limbs.lUpperLeg;
            else if (mode == 5)
                primary = limbs.r6 ? limbs.rLeg : limbs.rUpperLeg;
        }
        std::uintptr_t cands[4];
        int n = 0;
        auto push = [&](std::uintptr_t a) {
            if (!a || n >= 4)
                return;
            for (int i = 0; i < n; ++i) {
                if (cands[i] == a)
                    return;
            }
            cands[n++] = a;
        };
        push(primary);
        if (n < 4) {
            const PlayerCache::LimbAddrs& limbs = PlayerCache::GetLimbs(plr.characterAddr, false);
            push(plr.headAddr);
            push(limbs.r6 ? limbs.torso : limbs.upperTorso);
            push(plr.rootPartAddr);
        }
        if (n == 0)
            return false;
        for (int i = 0; i < n; ++i) {
            const RBX::Vec3 w = PartWorldPos(cands[i]);
            if (w.X == 0 && w.Y == 0 && w.Z == 0)
                continue;
            const RBX::Vec2 s = W2S::WorldToScreen(w, view);
            const bool isMarkedOrInfinite = (maxDist >= 1e8f);
            if (!isMarkedOrInfinite) {
                if (s.X == 0 && s.Y == 0)
                    continue;
                if (s.X < 0 || s.Y < 0 || s.X > sw || s.Y > sh)
                    continue;
                if (GetDistance2D(center, s) >= maxDist)
                    continue;
            }
            if (checkVis && !WorkspaceCache::IsVisible(camPos, w))
                continue;
            outScreen = s;
            outWorld = w;
            return true;
        }
        return false;
    }
    std::vector<std::uintptr_t> parts;
    CollectHitboxParts(plr.characterAddr, 7, parts);
    if (parts.empty())
        return false;
    float best = maxDist;
    bool foundVis = false;
    bool found = false;
    for (auto addr : parts) {
        const RBX::Vec3 w = PartWorldPos(addr);
        if (w.X == 0 && w.Y == 0 && w.Z == 0)
            continue;
        const RBX::Vec2 s = W2S::WorldToScreen(w, view);
        const bool isMarkedOrInfinite = (maxDist >= 1e8f);
        if (!isMarkedOrInfinite) {
            if (s.X == 0 && s.Y == 0)
                continue;
            if (s.X < 0 || s.Y < 0 || s.X > sw || s.Y > sh)
                continue;
            const float d = GetDistance2D(center, s);
            if (d >= best)
                continue;
        }
        const bool vis = !checkVis || WorkspaceCache::IsVisible(camPos, w);
        if (vis && !foundVis) {
            best = isMarkedOrInfinite ? 0.0f : GetDistance2D(center, s);
            outScreen = s;
            outWorld = w;
            foundVis = true;
            found = true;
        } else if (vis == foundVis) {
            best = isMarkedOrInfinite ? 0.0f : GetDistance2D(center, s);
            outScreen = s;
            outWorld = w;
            found = true;
        }
    }
    if (found && variables::Aimbot::visibleCheck && !foundVis)
        return false;
    return found;
}
}

void RenderTracer(ImDrawList* dl) {
    if (!variables::Aimbot::silentTracer || !hasTarget)
        return;
    POINT mp{};
    GetCursorPos(&mp);
    const ImVec2 from(static_cast<float>(mp.x), static_cast<float>(mp.y));
    ImVec2 to(lastTarget.X, lastTarget.Y);
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;
    const float dist = sqrtf(dx * dx + dy * dy);
    if (dist > variables::Aimbot::fovRadius)
        return;
    const ImU32 col = IM_COL32((int)(variables::Aimbot::silentTracerColor.x * 255.0f), (int)(variables::Aimbot::silentTracerColor.y * 255.0f), (int)(variables::Aimbot::silentTracerColor.z * 255.0f), (int)(variables::Aimbot::silentTracerColor.w * 255.0f));
    dl->AddLine(from, to, IM_COL32(0, 0, 0, 255), variables::Aimbot::silentTracerThickness + 2.0f);
    dl->AddLine(from, to, col, variables::Aimbot::silentTracerThickness);
    dl->AddCircleFilled(to, 3.0f, col, 12);
}

void RenderPredictionLine(ImDrawList* dl) {
    if (!variables::Aimbot::predictionLine || !hasTarget || !variables::Aimbot::prediction)
        return;
    POINT mp{};
    GetCursorPos(&mp);
    const ImVec2 from(static_cast<float>(mp.x), static_cast<float>(mp.y));
    ImVec2 to(lastTarget.X, lastTarget.Y);
    const ImU32 col = IM_COL32((int)(variables::Aimbot::predictionLineColor.x * 255.0f), (int)(variables::Aimbot::predictionLineColor.y * 255.0f), (int)(variables::Aimbot::predictionLineColor.z * 255.0f), (int)(variables::Aimbot::predictionLineColor.w * 255.0f));
    dl->AddLine(from, to, IM_COL32(0, 0, 0, 255), variables::Aimbot::predictionLineThickness + 2.0f);
    dl->AddLine(from, to, col, variables::Aimbot::predictionLineThickness);
    dl->AddCircleFilled(to, 3.0f, col, 12);
    dl->AddCircle(to, 5.0f, col, 16, 1.0f);
}

void RunAimbot(const RBX::Mat4& view) {
    POINT mp{};
    GetCursorPos(&mp);
    const RBX::Vec2 center{static_cast<float>(mp.x), static_cast<float>(mp.y)};

    RBX::Vec3 camPos{};
    bool haveCam = false;
    if (variables::Aimbot::visibleCheck || variables::Aimbot::prediction || variables::Aimbot::useSpread) {
        if (Globals::camera.Addr) {
            camPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);
            haveCam = !(camPos.X == 0 && camPos.Y == 0 && camPos.Z == 0);
        }
    }
    const bool checkVis = variables::Aimbot::visibleCheck && haveCam;
    const rbx::vector3_t camPosRbx{camPos.X, camPos.Y, camPos.Z};

    static bool aimTog = false, aimWas = false, trigTog = false, trigWas = false;
    const bool trigHeld = AimKeyActive(variables::Aimbot::triggerKey, variables::Aimbot::triggerKeyMode, trigTog, trigWas);
    const bool aimHeld = AimKeyActive(variables::Aimbot::aimbotKey, variables::Aimbot::aimbotKeyMode, aimTog, aimWas);
    trigActiveCached = variables::Aimbot::triggerbot && trigHeld;
    aimActiveCached = variables::Aimbot::enabled && aimHeld;

    using AimClock = std::chrono::steady_clock;
    static auto lastTrigScan = AimClock::now() - std::chrono::seconds(10);
    static auto lastAcqScan = AimClock::now() - std::chrono::seconds(10);
    const auto nowAim = AimClock::now();
    const bool doTrigScan = std::chrono::duration_cast<std::chrono::milliseconds>(nowAim - lastTrigScan).count() >= 8;
    const bool doAcqScan = std::chrono::duration_cast<std::chrono::milliseconds>(nowAim - lastAcqScan).count() >= 8;

    if (variables::Aimbot::triggerbot && trigHeld) {
        static bool pending = false;
        static auto armedAt = std::chrono::steady_clock::now();
        if (doTrigScan) {
            lastTrigScan = nowAim;
            RBX::Vec2 s{};
            RBX::Vec3 w{};
            bool has = false;
            if (!PlayersTab::target.empty()) {
                for (auto& p : PlayerCache::players) {
                    if (!p.isValid || !PlayersTab::IsMarked(p.name))
                        continue;
                    if (BestPartScreen(p, view, center, variables::Aimbot::fovRadius, camPos, checkVis, s, w)) {
                        has = true;
                        break;
                    }
                }
            }
            if (!has) {
                if (OpsCache::viewmodelsAddr) {
                    for (auto& p : OpsCache::players) {
                        if (!p.isValid || (p.maxHealth > 0.0f && p.health <= 0.0f))
                            continue;
                        if (PlayersTab::IsFriend(p.name))
                            continue;
                        if (Keys::TeamCheckOn() && p.teamAddr && p.teamAddr == OpsCache::localTeam)
                            continue;
                        if (!BestPartScreen(p, view, center, variables::Aimbot::fovRadius, camPos, checkVis, s, w))
                            continue;
                        has = true;
                        break;
                    }
                } else if (CbCache::charactersAddr) {
                    for (auto& p : CbCache::players) {
                        if (!p.isValid || (p.maxHealth > 0.0f && p.health <= 0.0f))
                            continue;
                        if (PlayersTab::IsFriend(p.name))
                            continue;
                        if (Keys::TeamCheckOn() && p.teamAddr && p.teamAddr == CbCache::localTeam)
                            continue;
                        if (!BestPartScreen(p, view, center, variables::Aimbot::fovRadius, camPos, checkVis, s, w))
                            continue;
                        has = true;
                        break;
                    }
                } else {
                    for (auto& p : PlayerCache::players) {
                        if (!p.isValid || (p.maxHealth > 0.0f && p.health <= 0.0f))
                            continue;
                        if (PlayersTab::IsFriend(p.name))
                            continue;
                        if (Keys::TeamCheckOn() && p.teamAddr && p.teamAddr == PlayerCache::localPlayerTeam)
                            continue;
                        if (!BestPartScreen(p, view, center, variables::Aimbot::fovRadius, camPos, checkVis, s, w))
                            continue;
                        has = true;
                        break;
                    }
                }
            }
            if (has) {
                if (!pending) {
                    pending = true;
                    armedAt = std::chrono::steady_clock::now();
                } else if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - armedAt).count() >= variables::Aimbot::triggerDelay) {
                    AutoClick();
                    pending = false;
                }
            } else {
                pending = false;
            }
        }
    }

    if (!variables::Aimbot::enabled) {
        hasTarget = false;
        ViewportSilent::Clear();
        MagicBullet::SetActive(false, {});
        MagicBullet::Ensure(false);
        PfSilent::SetActive(false, {});
        return;
    }
    if (!aimHeld) {
        lockedPlayerAddr = 0;
        hasTarget = false;
        ViewportSilent::Clear();
        MagicBullet::SetActive(false, {});
        MagicBullet::Ensure(false);
        PfSilent::SetActive(false, {});
        return;
    }

    if (!PlayersTab::target.empty()) {
        std::uintptr_t targetAddr = 0;
        for (auto& p : PlayerCache::players) {
            if (p.isValid && PlayersTab::IsMarked(p.name)) {
                targetAddr = p.playerAddr;
                break;
            }
        }
        if (!targetAddr) {
            for (auto& p : CbCache::players) {
                if (p.isValid && PlayersTab::IsMarked(p.name)) {
                    targetAddr = p.playerAddr;
                    break;
                }
            }
        }
        if (!targetAddr) {
            for (auto& p : OpsCache::players) {
                if (p.isValid && PlayersTab::IsMarked(p.name)) {
                    targetAddr = p.playerAddr;
                    break;
                }
            }
        }
        if (targetAddr != 0) {
            lockedPlayerAddr = targetAddr;
        }
    }

    if (PfCache::workspacePlayersAddr && !PfCache::players.empty()) {
        const bool tc = Keys::TeamCheckOn();
        if (lockedPlayerAddr != 0) {
            bool stillValid = false;
            for (auto& p : PfCache::players) {
                if (!p.isValid || p.modelAddr != lockedPlayerAddr)
                    continue;
                if (PlayersTab::IsFriend(p.name))
                    break;
                if (tc && PfCache::IsTeammate(p))
                    break;
                RBX::Vec2 s{};
                RBX::Vec3 w{};
                if (!PfPartScreen(p, view, center, 999999.0f, camPos, PlayersTab::IsMarked(p.name) ? false : checkVis, s, w))
                    break;
                stillValid = true;
                lastTarget = s;
                hasTarget = true;
                if (variables::Aimbot::aimMethod == 3) {
                    PfSilent::SetActive(true, w);
                    ViewportSilent::Clear();
                    MagicBullet::SetActive(false, {});
                    MagicBullet::Ensure(false);
                } else if (variables::Aimbot::aimMethod == 2 || variables::Aimbot::magicBullet) {
                    PfSilent::SetActive(false, {});
                    ViewportSilent::Clear();
                    MagicBullet::Ensure(true);
                    MagicBullet::SetActive(true, w);
                } else if (variables::Aimbot::aimMethod == 1) {
                    PfSilent::SetActive(false, {});
                    ViewportSilent::SetTarget(w);
                    MagicBullet::SetActive(false, {});
                    MagicBullet::Ensure(false);
                } else {
                    PfSilent::SetActive(false, {});
                    WriteMemoryAngles(w);
                    ViewportSilent::Clear();
                    MagicBullet::SetActive(false, {});
                    MagicBullet::Ensure(false);
                }
                return;
            }
            if (!stillValid)
                lockedPlayerAddr = 0;
        }
        float best = variables::Aimbot::fovRadius;
        RBX::Vec2 bestS{};
        RBX::Vec3 bestW{};
        std::uintptr_t bestAddr = 0;
        for (auto& p : PfCache::players) {
            if (!p.isValid)
                continue;
            if (PlayersTab::IsFriend(p.name))
                continue;
            if (tc && PfCache::IsTeammate(p))
                continue;
            const bool marked = PlayersTab::IsMarked(p.name);
            RBX::Vec2 s{};
            RBX::Vec3 w{};
            if (!PfPartScreen(p, view, center, marked ? 1e9f : best, camPos, marked ? false : checkVis, s, w))
                continue;
            best = GetDistance2D(center, s);
            bestS = s;
            bestW = w;
            bestAddr = p.modelAddr;
            if (marked)
                best = -1.0f;
        }
        lockedPlayerAddr = bestAddr;
        if (!bestAddr) {
            hasTarget = false;
            PfSilent::SetActive(false, {});
            ViewportSilent::Clear();
            MagicBullet::SetActive(false, {});
            return;
        }
        lastTarget = bestS;
        hasTarget = true;
        if (variables::Aimbot::aimMethod == 3) {
            PfSilent::SetActive(true, bestW);
            ViewportSilent::Clear();
            MagicBullet::SetActive(false, {});
            MagicBullet::Ensure(false);
        } else if (variables::Aimbot::aimMethod == 2 || variables::Aimbot::magicBullet) {
            PfSilent::SetActive(false, {});
            ViewportSilent::Clear();
            MagicBullet::Ensure(true);
            MagicBullet::SetActive(true, bestW);
        } else if (variables::Aimbot::aimMethod == 1) {
            PfSilent::SetActive(false, {});
            ViewportSilent::SetTarget(bestW);
            MagicBullet::SetActive(false, {});
            MagicBullet::Ensure(false);
        } else {
            PfSilent::SetActive(false, {});
            WriteMemoryAngles(bestW);
            ViewportSilent::Clear();
            MagicBullet::SetActive(false, {});
            MagicBullet::Ensure(false);
        }
        return;
    }
    PfSilent::SetActive(false, {});

    if (lockedPlayerAddr == 0) {
        if (!doAcqScan) {
            hasTarget = false;
            return;
        }
        lastAcqScan = nowAim;
        float best = variables::Aimbot::fovRadius;
        std::uintptr_t bestAddr = 0;
        auto consider = [&](PlayerCache::CachedPlayer& p) {
            if (!p.isValid || (p.maxHealth > 0.0f && p.health <= 0.0f))
                return;
            if (PlayersTab::IsFriend(p.name))
                return;
            if (Keys::TeamCheckOn() && p.teamAddr && (p.teamAddr == PlayerCache::localPlayerTeam || p.teamAddr == CbCache::localTeam || p.teamAddr == OpsCache::localTeam))
                return;
            const bool marked = PlayersTab::IsMarked(p.name);
            RBX::Vec2 s{};
            RBX::Vec3 w{};
            if (!BestPartScreen(p, view, center, marked ? 1e9f : best, camPos, marked ? false : checkVis, s, w))
                return;
            best = GetDistance2D(center, s);
            bestAddr = p.playerAddr;
            if (marked)
                best = -1.0f;
        };
        if (OpsCache::viewmodelsAddr) {
            for (auto& p : OpsCache::players)
                consider(p);
        } else if (CbCache::charactersAddr) {
            for (auto& p : CbCache::players)
                consider(p);
        } else {
            for (auto& p : PlayerCache::players)
                consider(p);
        }
        lockedPlayerAddr = bestAddr;
    }

    RBX::Vec2 dst{0, 0};
    RBX::Vec3 dstWorld{0, 0, 0};
    bool found = false;
    bool npcTarget = false;
    if (lockedPlayerAddr == 0 && variables::Aimbot::includeNPC) {

        RBX::Vec3 nbPos{};
        RBX::Vec3 nbVel{};
        bool haveNpc = false;
        {
            std::lock_guard<std::mutex> lk(WorldCache::mtx);
            float nbest = variables::Aimbot::fovRadius;
            for (auto& e : WorldCache::entries) {
                if (e.category != "soldier" && e.category != "animal") continue;
                if (e.pos.X == 0 && e.pos.Y == 0 && e.pos.Z == 0) continue;
                const RBX::Vec2 s = W2S::WorldToScreen(e.pos, view);
                if (s.X == 0 && s.Y == 0) continue;
                const float d = GetDistance2D(center, s);
                if (d >= nbest) continue;
                nbest = d;
                nbPos = e.pos;
                nbVel = e.vel;
                haveNpc = true;
            }
        }
        if (haveNpc) {
            dstWorld = nbPos;
            RBX::Vec3 npVel = nbVel;
            MovementHistory::Push(1, nbPos.X, nbPos.Y, nbPos.Z);
            {
                MovementHistory::Result nhr{};
                if (MovementHistory::Sample(1, nhr)) {
                    npVel.X = nhr.vx * nhr.damp;
                    npVel.Y = nhr.vy;
                    npVel.Z = nhr.vz * nhr.damp;
                }
            }
            if (variables::Aimbot::prediction) {
                fallen_update_weapon_auto();
                if (variables::Aimbot::fallen_prediction) {
                    float bv = fallen_get_bullet_velocity(fallen_get_local_weapon());
                    if (bv > 0.f && haveCam) {
                        rbx::vector3_t target_vel = { npVel.X, npVel.Y, npVel.Z };
                        rbx::vector3_t target_pos = { dstWorld.X, dstWorld.Y, dstWorld.Z };
                        rbx::vector3_t local_vel = fallen_get_local_velocity();
                        fallen_predict(target_pos, camPosRbx, target_vel, bv, local_vel);
                        dstWorld.X = target_pos.x; dstWorld.Y = target_pos.y; dstWorld.Z = target_pos.z;
                    }
                } else {
                    const int pingMs = Ping::GetMs();
                    const float t = (pingMs > 0 ? (float)pingMs / 1000.0f : 0.0f) + 0.016f;
                    dstWorld.X += npVel.X * t;
                    dstWorld.Y += npVel.Y * t;
                    dstWorld.Z += npVel.Z * t;
                }
            }
            const RBX::Vec2 proj = W2S::WorldToScreen(dstWorld, view);
            if (proj.X != 0 || proj.Y != 0) { dst = proj; npcTarget = true; }
        }
    }
    if (lockedPlayerAddr == 0 && !npcTarget)
        return;

    auto trackOne = [&](PlayerCache::CachedPlayer& p) -> bool {
        if (!p.isValid || p.playerAddr != lockedPlayerAddr || (p.maxHealth > 0.0f && p.health <= 0.0f))
            return false;
        if (PlayersTab::IsFriend(p.name)) {
            lockedPlayerAddr = 0;
            hasTarget = false;
            return false;
        }
        const bool marked = PlayersTab::IsMarked(p.name);
        if (!marked && Keys::TeamCheckOn() && p.teamAddr && (p.teamAddr == PlayerCache::localPlayerTeam || p.teamAddr == CbCache::localTeam || p.teamAddr == OpsCache::localTeam)) {
            lockedPlayerAddr = 0;
            hasTarget = false;
            return false;
        }
        if (!BestPartScreen(p, view, center, 999999.0f, camPos, marked ? false : checkVis, dst, dstWorld))
            return false;
        MovementHistory::Push(p.playerAddr, dstWorld.X, dstWorld.Y, dstWorld.Z);
        RBX::Vec3 histVel{};
        bool haveHist = false;
        {
            MovementHistory::Result hr{};
            if (MovementHistory::Sample(p.playerAddr, hr)) {
                histVel = { hr.vx * hr.damp, hr.vy, hr.vz * hr.damp };
                haveHist = true;
            }
        }
        if (variables::Aimbot::prediction) {
            fallen_update_weapon_auto();
            if (variables::Aimbot::fallen_prediction) {
                float bv = fallen_get_bullet_velocity(fallen_get_local_weapon());
                if (bv > 0.f) {
                    const std::uintptr_t prim = memory->read<std::uintptr_t>(p.rootPartAddr + Offsets::BasePart::Primitive);
                    if (prim) {
                        rbx::vector3_t target_vel = memory->read<rbx::vector3_t>(prim + Offsets::Primitive::AssemblyLinearVelocity);
                        if (haveHist) {
                            target_vel.x = histVel.X;
                            target_vel.y = histVel.Y;
                            target_vel.z = histVel.Z;
                        }
                        rbx::vector3_t target_pos = { dstWorld.X, dstWorld.Y, dstWorld.Z };
                        rbx::vector3_t local_vel = fallen_get_local_velocity();
                        fallen_predict(target_pos, camPosRbx, target_vel, bv, local_vel);
                        dstWorld.X = target_pos.x; dstWorld.Y = target_pos.y; dstWorld.Z = target_pos.z;
                        const RBX::Vec2 proj = W2S::WorldToScreen(dstWorld, view);
                        if (proj.X != 0 || proj.Y != 0)
                            dst = proj;
                    }
                }
            } else {
                const std::uintptr_t prim = memory->read<std::uintptr_t>(p.rootPartAddr + Offsets::BasePart::Primitive);
                if (prim) {
                    RBX::Vec3 vel = memory->read<RBX::Vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity);
                    if (haveHist)
                        vel = histVel;
                    const int pingMs = Ping::GetMs();
                    const float t = (pingMs > 0 ? (float)pingMs / 1000.0f : 0.0f) + 0.016f;
                    dstWorld.X += vel.X * t;
                    dstWorld.Y += vel.Y * t;
                    dstWorld.Z += vel.Z * t;
                    const RBX::Vec2 proj = W2S::WorldToScreen(dstWorld, view);
                    if (proj.X != 0 || proj.Y != 0)
                        dst = proj;
                }
            }
        }
        found = true;
        return true;
    };
    if (!npcTarget) {
        if (OpsCache::viewmodelsAddr) {
            for (auto& p : OpsCache::players) {
                if (trackOne(p))
                    break;
            }
        } else if (CbCache::charactersAddr) {
            for (auto& p : CbCache::players) {
                if (trackOne(p))
                    break;
            }
        } else {
            for (auto& p : PlayerCache::players) {
                if (trackOne(p))
                    break;
            }
        }
    }
    if (!found && !npcTarget) {
        lockedPlayerAddr = 0;
        hasTarget = false;
        ViewportSilent::Clear();
        MagicBullet::SetActive(false, {});
        return;
    }
    lastTarget = dst;
    hasTarget = true;
    const float dx = dst.X - static_cast<float>(mp.x);
    const float dy = dst.Y - static_cast<float>(mp.y);
    if (variables::Aimbot::useDeadzone) {
        const float dist = sqrtf(dx * dx + dy * dy);
        if (dist <= variables::Aimbot::deadzone) {
            ViewportSilent::Clear();
            MagicBullet::SetActive(false, {});
            return;
        }
    }
    if (variables::Aimbot::aimMethod == 0) {
        WriteMemoryAngles(dstWorld);
        ViewportSilent::Clear();
        MagicBullet::SetActive(false, {});
        MagicBullet::Ensure(false);
        return;
    }
    if (variables::Aimbot::aimMethod == 1) {
        ViewportSilent::SetTarget(dstWorld);
        MagicBullet::SetActive(false, {});
        MagicBullet::Ensure(false);
        return;
    }
    if (variables::Aimbot::aimMethod == 2 || variables::Aimbot::magicBullet) {
        ViewportSilent::Clear();
        MagicBullet::Ensure(true);

        RBX::Vec3 finalTarget = dstWorld;
        if (variables::Aimbot::useSpread && variables::Aimbot::spreadModifier > 0.001f) {
            static std::mt19937 hitRng(42);
            std::uniform_real_distribution<float> pct(0.0f, 100.0f);
            if (pct(hitRng) > variables::Aimbot::hitChance) {
                finalTarget = ApplyRaycastSpread(camPos, dstWorld, variables::Aimbot::spreadModifier);
            }
        }

        MagicBullet::SetActive(true, finalTarget);
        return;
    }
    ViewportSilent::Clear();
    MagicBullet::SetActive(false, {});
    MagicBullet::Ensure(false);
    return;
}
}
