#pragma once
#include "../../../sdk/sdk.h"
#include "../../cache/pf_cache.h"
#include <windows.h>
#include <atomic>
#include <thread>
#include <cstdint>
#include <cmath>

namespace PfSilent {
inline std::atomic<bool> run{false};
inline std::atomic<bool> active{false};
inline std::atomic<std::uintptr_t> camAddr{0};
inline std::atomic<bool> camIsPart{false};
inline rbx::matrix3_t lastMat{};
inline bool started = false;

inline rbx::vector3_t CrossUp(const rbx::vector3_t& fwd) {
    rbx::vector3_t up{0.0f, 1.0f, 0.0f};
    rbx::vector3_t right{
        up.y * fwd.z - up.z * fwd.y,
        up.z * fwd.x - up.x * fwd.z,
        up.x * fwd.y - up.y * fwd.x
    };
    float m = right.magnitude();
    if (m < 1e-6f)
        return right;
    return right * (1.0f / m);
}

inline rbx::matrix3_t MatrixFromForward(const rbx::vector3_t& fwdIn) {
    rbx::vector3_t fwd = fwdIn;
    float m = fwd.magnitude();
    if (m < 1e-6f)
        return rbx::matrix3_t{};
    fwd = fwd * (1.0f / m);
    rbx::vector3_t right = CrossUp(fwd);
    float rm = right.magnitude();
    if (rm < 1e-6f)
        return rbx::matrix3_t{};
    right = right * (1.0f / rm);
    rbx::vector3_t up = fwd.cross(right);
    rbx::matrix3_t mat{};
    mat.data[0] = -right.x;
    mat.data[1] = up.x;
    mat.data[2] = -fwd.x;
    mat.data[3] = right.y;
    mat.data[4] = 0.01f;
    mat.data[5] = -fwd.y;
    mat.data[6] = -right.z;
    mat.data[7] = up.z;
    mat.data[8] = -fwd.z;
    return mat;
}

inline std::uintptr_t PartPrim(std::uintptr_t part) {
    if (!part)
        return 0;
    return memory->read<std::uintptr_t>(part + Offsets::BasePart::Primitive);
}

inline bool ReadCurrentRot(std::uintptr_t addr, bool isPart, rbx::matrix3_t& out) {
    if (!addr)
        return false;
    if (isPart) {
        const auto prim = PartPrim(addr);
        if (!prim)
            return false;
        out = memory->read<rbx::matrix3_t>(prim + Offsets::Primitive::Rotation);
    } else {
        out = memory->read<rbx::matrix3_t>(addr + Offsets::Camera::Rotation);
    }
    return true;
}

inline rbx::matrix3_t SmoothMatrix(const rbx::matrix3_t& cur, const rbx::vector3_t& wantFwd, float smoothing) {
    float k = 1.0f / (smoothing <= 0.01f ? 1.0f : smoothing);
    if (k < 0.01f)
        k = 0.01f;
    if (k > 1.0f)
        k = 1.0f;
    rbx::vector3_t curFwd{-cur.data[2], -cur.data[5], -cur.data[8]};
    if (curFwd.magnitude() < 1e-6f)
        return MatrixFromForward(wantFwd);
    curFwd = curFwd.normalize();
    rbx::vector3_t look = curFwd + (wantFwd - curFwd) * k;
    if (look.magnitude() < 1e-6f)
        return MatrixFromForward(wantFwd);
    return MatrixFromForward(look.normalize());
}

inline rbx::vector3_t PartPosOf(std::uintptr_t part) {
    const auto prim = PartPrim(part);
    if (!prim)
        return rbx::vector3_t{};
    return memory->read<rbx::vector3_t>(prim + Offsets::Primitive::Position);
}

inline void ApplyRot(std::uintptr_t addr, const rbx::matrix3_t& mt, bool isPart) {
    if (!addr)
        return;
    if (isPart) {
        const auto prim = PartPrim(addr);
        if (!prim)
            return;
        memory->write_raw(prim + Offsets::Primitive::Rotation, mt.data, sizeof(mt.data));
    } else {
        memory->write_raw(addr + Offsets::Camera::Rotation, mt.data, sizeof(mt.data));
    }
}

inline void EnsureWriter() {
    if (started)
        return;
    started = true;
    run = true;
    std::thread([] {
        while (run) {
            if (active) {
                const auto addr = camAddr.load();
                const bool isPart = camIsPart.load();
                const rbx::matrix3_t mt = lastMat;
                ApplyRot(addr, mt, isPart);
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
                    ApplyRot(addr, mt, isPart);
            }
            Sleep((active && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) ? 1 : 10);
        }
    }).detach();
}

inline bool ResolveCamera(std::uintptr_t& outCam, bool& outIsPart) {
    outCam = 0;
    outIsPart = false;
    if (!Globals::workspace.Addr)
        return false;
    std::uintptr_t camParent = 0;
    for (auto& ch : Globals::workspace.GetChildList()) {
        if (ch.GetClass() == "Camera") {
            camParent = ch.Addr;
            break;
        }
    }
    if (!camParent)
        camParent = memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::CurrentCamera);
    if (!camParent)
        return false;
    for (auto& ch : RBX::RbxInstance(camParent).GetChildList()) {
        if (ch.GetName() == "Part") {
            outCam = ch.Addr;
            outIsPart = true;
            return true;
        }
    }
    outCam = camParent;
    outIsPart = false;
    return true;
}

inline void SetActive(bool on, const RBX::Vec3& target) {
    if (!on) {
        active = false;
        return;
    }
    if (!PfCache::workspacePlayersAddr) {
        active = false;
        return;
    }
    if (target.X == 0 && target.Y == 0 && target.Z == 0)
        return;
    std::uintptr_t cam = 0;
    bool isPart = false;
    if (!ResolveCamera(cam, isPart)) {
        active = false;
        return;
    }
    rbx::vector3_t pos{};
    if (isPart) {
        pos = PartPosOf(cam);
    } else {
        const RBX::Vec3 cp = memory->read<RBX::Vec3>(cam + Offsets::Camera::Position);
        pos = rbx::vector3_t{cp.X, cp.Y, cp.Z};
    }
    if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)
        return;
    rbx::vector3_t tgt{target.X, target.Y, target.Z};
    rbx::vector3_t wantFwd{tgt.x - pos.x, tgt.y - pos.y, tgt.z - pos.z};
    if (wantFwd.magnitude() < 1e-6f)
        return;
    wantFwd = wantFwd.normalize();
    rbx::matrix3_t mt{};
    rbx::matrix3_t cur{};
    if (ReadCurrentRot(cam, isPart, cur))
        mt = SmoothMatrix(cur, wantFwd, variables::Aimbot::smoothing);
    else
        mt = MatrixFromForward(wantFwd);
    ApplyRot(cam, mt, isPart);
    lastMat = mt;
    camAddr = cam;
    camIsPart = isPart;
    EnsureWriter();
    active = true;
}

inline void Shutdown() {
    active = false;
    run = false;
}
}
