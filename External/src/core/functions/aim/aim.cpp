#include "aim.h"
#include "fallen_prediction.h"
#include "viewport_silent.h"
#include "magic.h"
#include "../../cache/workspace.h"
#include "../../cache/worldcache.h"
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
    rbx::matrix3_t curRot = memory->read<rbx::matrix3_t>(Globals::camera.Addr + Offsets::Camera::Rotation);
    rbx::vector3_t camPos = memory->read<rbx::vector3_t>(Globals::camera.Addr + Offsets::Camera::Position);
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
bool BestPartScreen(const PlayerCache::CachedPlayer& plr, const RBX::Mat4& view, const RBX::Vec2& center, float maxDist, RBX::Vec2& outScreen, RBX::Vec3& outWorld) {
    static float sw = 0.0f;
    static float sh = 0.0f;
    if (sw <= 0.0f) {
        sw = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        sh = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
    }
    const int mode = variables::Aimbot::aimTarget;
    if (mode != 7) {
        std::uintptr_t addr = 0;
        if (mode == 0)
            addr = plr.headAddr;
        else if (mode == 6)
            addr = plr.rootPartAddr;
        else {
            const PlayerCache::LimbAddrs& limbs = PlayerCache::GetLimbs(plr.characterAddr);
            if (mode == 1)
                addr = limbs.r6 ? limbs.torso : limbs.upperTorso;
            else if (mode == 2)
                addr = limbs.r6 ? limbs.lArm : limbs.lUpperArm;
            else if (mode == 3)
                addr = limbs.r6 ? limbs.rArm : limbs.rUpperArm;
            else if (mode == 4)
                addr = limbs.r6 ? limbs.lLeg : limbs.lUpperLeg;
            else if (mode == 5)
                addr = limbs.r6 ? limbs.rLeg : limbs.rUpperLeg;
        }
        if (!addr)
            return false;
        const RBX::Vec3 w = PartWorldPos(addr);
        if (w.X == 0 && w.Y == 0 && w.Z == 0)
            return false;
        const RBX::Vec2 s = W2S::WorldToScreen(w, view);
        if (s.X == 0 && s.Y == 0)
            return false;
        if (s.X < 0 || s.Y < 0 || s.X > sw || s.Y > sh)
            return false;
        if (GetDistance2D(center, s) >= maxDist)
            return false;
        if (!IsTargetVisible(w))
            return false;
        outScreen = s;
        outWorld = w;
        return true;
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
        if (s.X == 0 && s.Y == 0)
            continue;
        if (s.X < 0 || s.Y < 0 || s.X > sw || s.Y > sh)
            continue;
        const float d = GetDistance2D(center, s);
        if (d >= best)
            continue;
        const bool vis = IsTargetVisible(w);
        if (vis && !foundVis) {
            best = d;
            outScreen = s;
            outWorld = w;
            foundVis = true;
            found = true;
        } else if (vis == foundVis) {
            best = d;
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

    if (variables::Aimbot::triggerbot && IsAimKeyDown(variables::Aimbot::triggerKey)) {
        static bool pending = false;
        static auto armedAt = std::chrono::steady_clock::now();
        RBX::Vec2 s{};
        RBX::Vec3 w{};
        bool has = false;
        for (auto& p : PlayerCache::players) {
            if (!p.isValid)
                continue;
            if (Keys::TeamCheckOn() && p.teamAddr && p.teamAddr == PlayerCache::localPlayerTeam)
                continue;
            if (!BestPartScreen(p, view, center, variables::Aimbot::fovRadius, s, w))
                continue;
            has = true;
            break;
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

    if (!variables::Aimbot::enabled) {
        hasTarget = false;
        ViewportSilent::Clear();
        MagicBullet::SetActive(false, {});
        MagicBullet::Ensure(false);
        return;
    }
    if (!IsAimKeyDown(variables::Aimbot::aimbotKey)) {
        lockedPlayerAddr = 0;
        hasTarget = false;
        ViewportSilent::Clear();
        MagicBullet::SetActive(false, {});
        MagicBullet::Ensure(false);
        return;
    }

    if (lockedPlayerAddr == 0) {
        float best = variables::Aimbot::fovRadius;
        std::uintptr_t bestAddr = 0;
        for (auto& p : PlayerCache::players) {
            if (!p.isValid)
                continue;
            if (Keys::TeamCheckOn() && p.teamAddr && p.teamAddr == PlayerCache::localPlayerTeam)
                continue;
            RBX::Vec2 s{};
            RBX::Vec3 w{};
            if (!BestPartScreen(p, view, center, best, s, w))
                continue;
            best = GetDistance2D(center, s);
            bestAddr = p.playerAddr;
        }
        lockedPlayerAddr = bestAddr;
    }

    RBX::Vec2 dst{0, 0};
    RBX::Vec3 dstWorld{0, 0, 0};
    bool found = false;
    bool npcTarget = false;
    if (lockedPlayerAddr == 0 && variables::Aimbot::includeNPC) {
        std::vector<std::pair<RBX::Vec3, RBX::Vec3>> npcs;
        {
            std::lock_guard<std::mutex> lk(WorldCache::mtx);
            for (auto& e : WorldCache::entries) {
                if (e.category != "soldier" && e.category != "animal") continue;
                if (e.pos.X == 0 && e.pos.Y == 0 && e.pos.Z == 0) continue;
                npcs.emplace_back(e.pos, e.vel);
            }
        }
        float best = variables::Aimbot::fovRadius;
        RBX::Vec3 bestVel{};
        for (auto& np : npcs) {
            const RBX::Vec2 s = W2S::WorldToScreen(np.first, view);
            if (s.X == 0 && s.Y == 0) continue;
            const float d = GetDistance2D(center, s);
            if (d >= best) continue;
            best = d;
            dstWorld = np.first;
            bestVel = np.second;
        }
        if (dstWorld.X != 0 || dstWorld.Y != 0 || dstWorld.Z != 0) {
            if (variables::Aimbot::prediction) {
                fallen_update_weapon_auto();
                if (variables::Aimbot::fallen_prediction) {
                    float bv = fallen_get_bullet_velocity(fallen_get_local_weapon());
                    if (bv > 0.f && Globals::camera.Addr) {
                        const int pingMs = Ping::GetMs();
                        variables::Aimbot::prediction_ping = pingMs > 0 ? (float)pingMs : 0.f;
                        rbx::vector3_t target_vel = { bestVel.X, bestVel.Y, bestVel.Z };
                        rbx::vector3_t target_pos = { dstWorld.X, dstWorld.Y, dstWorld.Z };
                        rbx::vector3_t camPos = memory->read<rbx::vector3_t>(Globals::camera.Addr + Offsets::Camera::Position);
                        rbx::vector3_t local_vel = fallen_get_local_velocity();
                        fallen_predict(target_pos, camPos, target_vel, bv, local_vel);
                        dstWorld.X = target_pos.x; dstWorld.Y = target_pos.y; dstWorld.Z = target_pos.z;
                    }
                } else {
                    const int pingMs = Ping::GetMs();
                    const float t = (pingMs > 0 ? (float)pingMs / 1000.0f : 0.0f) + 0.016f;
                    dstWorld.X += bestVel.X * t;
                    dstWorld.Y += bestVel.Y * t;
                    dstWorld.Z += bestVel.Z * t;
                }
            }
            const RBX::Vec2 proj = W2S::WorldToScreen(dstWorld, view);
            if (proj.X != 0 || proj.Y != 0) { dst = proj; npcTarget = true; }
        }
    }
    if (lockedPlayerAddr == 0 && !npcTarget)
        return;

    for (auto& p : PlayerCache::players) {
        if (npcTarget) break;
        if (!p.isValid || p.playerAddr != lockedPlayerAddr)
            continue;
        if (Keys::TeamCheckOn() && p.teamAddr && p.teamAddr == PlayerCache::localPlayerTeam) {
            lockedPlayerAddr = 0;
            hasTarget = false;
            return;
        }
        if (!BestPartScreen(p, view, center, 999999.0f, dst, dstWorld))
            break;
        if (variables::Aimbot::prediction) {
            fallen_update_weapon_auto();
            if (variables::Aimbot::fallen_prediction) {
                std::string my_weapon = fallen_get_local_weapon();
                float bv = fallen_get_bullet_velocity(my_weapon);
                if (bv > 0.f) {
                    const std::uintptr_t prim = memory->read<std::uintptr_t>(p.rootPartAddr + Offsets::BasePart::Primitive);
                    if (prim) {
                        const int pingMs = Ping::GetMs();
                        variables::Aimbot::prediction_ping = pingMs > 0 ? (float)pingMs : 0.f;
                        rbx::vector3_t target_vel = memory->read<rbx::vector3_t>(prim + Offsets::Primitive::AssemblyLinearVelocity);
                        rbx::vector3_t target_pos = { dstWorld.X, dstWorld.Y, dstWorld.Z };
                        rbx::vector3_t camPos = memory->read<rbx::vector3_t>(Globals::camera.Addr + Offsets::Camera::Position);
                        rbx::vector3_t local_vel = fallen_get_local_velocity();
                        fallen_predict(target_pos, camPos, target_vel, bv, local_vel);
                        dstWorld.X = target_pos.x; dstWorld.Y = target_pos.y; dstWorld.Z = target_pos.z;
                        const RBX::Vec2 proj = W2S::WorldToScreen(dstWorld, view);
                        if (proj.X != 0 || proj.Y != 0)
                            dst = proj;
                    }
                }
            } else {
                const std::uintptr_t prim = memory->read<std::uintptr_t>(p.rootPartAddr + Offsets::BasePart::Primitive);
                if (prim) {
                    const RBX::Vec3 vel = memory->read<RBX::Vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity);
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
        break;
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
    if (variables::Aimbot::aimMethod == 1) {
        WriteMemoryAngles(dstWorld);
        ViewportSilent::Clear();
        MagicBullet::SetActive(false, {});
        MagicBullet::Ensure(false);
        return;
    }
    if (variables::Aimbot::aimMethod == 2) {
        ViewportSilent::SetTarget(dstWorld);
        MagicBullet::SetActive(false, {});
        MagicBullet::Ensure(false);
        return;
    }
    if (variables::Aimbot::aimMethod == 3 || variables::Aimbot::magicBullet) {
        ViewportSilent::Clear();
        MagicBullet::Ensure(true);
        MagicBullet::SetActive(true, dstWorld);
        return;
    }
    ViewportSilent::Clear();
    MagicBullet::SetActive(false, {});
    MagicBullet::Ensure(false);
    const float k = 1.0f / (variables::Aimbot::smoothing <= 0.01f ? 1.0f : variables::Aimbot::smoothing);
    MoveMouse(dx * k, dy * k);
}
}
