#include "movement.h"
#include "../../variables/variables.h"
#include "../../globals/globals.h"
#include "../../cache/cache.h"
#include "../../keys/keys.h"
#include "../../../memory/memory.h"
#include "../../../sdk/offsets.h"
#include "../../../sdk/sdk.h"
#include "../../../sdk/math.h"
#include <windows.h>
#include <thread>
#include <chrono>
#include <cmath>

namespace Movement {
namespace {
bool KeyActive(int key, int mode, bool& tog, bool& was) {
    return Keys::Gate(key, mode, tog, was);
}

bool RobloxFocused() {
    HWND hwnd = FindWindowW(nullptr, L"Roblox");
    return hwnd && IsWindow(hwnd) && GetForegroundWindow() == hwnd;
}

void SetVel(std::uintptr_t prim, const rbx::vector3_t& vel) {
    if (!prim)
        return;
    memory->write<rbx::vector3_t>(prim + Offsets::Primitive::AssemblyLinearVelocity, vel);
    memory->write<rbx::vector3_t>(prim + Offsets::Primitive::AssemblyAngularVelocity, rbx::vector3_t(0.0f, 0.0f, 0.0f));
}

static std::atomic<bool> fly_active_state{false};
void TickFly(bool& gravOver, float& gravBackup) {
    static bool was_active = false;
    static rbx::vector3_t curVel{0,0,0};
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - last_time).count();
    if (dt < 0.0001f) dt = 0.0001f;
    last_time = now;

    bool roblox_active = RobloxFocused();
    bool allow_toggle = roblox_active;

    
    static bool flyTog=false, flyWas=false;
    bool key_active = false;
    if (allow_toggle) key_active = KeyActive(variables::Movement::flyKey, variables::Movement::flyKeyMode, flyTog, flyWas);
    bool fly_active = variables::Movement::fly && (variables::Movement::flyKey==0 ? true : key_active);
    
    fly_active_state.store(fly_active);

    auto localChar = Globals::localPlayer.GetModelRef();
    std::uintptr_t prim = 0;
    std::uintptr_t cam = 0;
    if (localChar.Addr) {
        const auto& limbs = PlayerCache::GetLimbs(localChar.Addr);
        if (limbs.hrp) prim = memory->read<std::uintptr_t>(limbs.hrp + Offsets::BasePart::Primitive);
    }
    if (Globals::workspace.Addr) cam = memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::CurrentCamera);

    if (!prim || !cam || !localChar.Addr) {
        if (was_active && prim) { SetVel(prim, rbx::vector3_t(0,0,0)); curVel={0,0,0}; }
        was_active = false;
        if (gravOver && Globals::workspace.Addr) {
            auto world = memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::World);
            if (world) memory->write<float>(world + Offsets::World::Gravity, gravBackup);
            gravOver=false;
        }
        return;
    }
    if (!fly_active) {
        if (was_active) { SetVel(prim, rbx::vector3_t(0,0,0)); curVel={0,0,0}; }
        was_active=false;
        if (gravOver && Globals::workspace.Addr) {
            auto world = memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::World);
            if (world) memory->write<float>(world + Offsets::World::Gravity, gravBackup);
            gravOver=false;
        }
        return;
    }
    was_active=true;
    
    if (!gravOver && Globals::workspace.Addr) {
        auto world = memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::World);
        if (world){ gravBackup = memory->read<float>(world + Offsets::World::Gravity); gravOver=true; }
    }
    if (gravOver && Globals::workspace.Addr) {
        auto world = memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::World);
        if (world) memory->write<float>(world + Offsets::World::Gravity, 0.0f);
    }
    rbx::matrix3_t rot{}; for(int i=0;i<9;++i) rot.data[i]=memory->read<float>(cam + Offsets::Camera::Rotation + (uint64_t)i*sizeof(float));
    rbx::vector3_t fwd(-rot.data[2], -rot.data[5], -rot.data[8]);
    rbx::vector3_t right(-rot.data[0], rot.data[3], -rot.data[6]);
    if (fwd.magnitude()<1e-6f) fwd=rbx::vector3_t(0,0,1); else fwd=fwd.normalize();
    if (right.magnitude()<1e-6f) right=rbx::vector3_t(1,0,0); else right=right.normalize();
    
    
    
    fwd.y = 0.0f;
    if (fwd.magnitude()<1e-6f) fwd=rbx::vector3_t(0,0,1); else fwd=fwd.normalize();
    bool allow_move = roblox_active;
    rbx::vector3_t target{0,0,0};
    float spd = (std::max)(variables::Movement::flySpeed,0.0f);
    float vspd = spd * variables::Movement::flyVerticalBoost;
    if (allow_move) {
        auto kd=[&](int vk){ return (GetAsyncKeyState(vk)&0x8000)!=0; };
        if(kd('W')) target = target + fwd * spd;
        if(kd('S')) target = target - fwd * spd;
        if(kd('A')) target = target + right * spd;
        if(kd('D')) target = target - right * spd;
        float vert=0; if(kd(VK_SPACE)) vert+=1; if(kd(VK_CONTROL)) vert-=1;
        if(vert!=0) target = target + rbx::vector3_t(0,1,0) * (vert * vspd);
    }
    float damping = (std::max)(variables::Movement::flyDamping,0.0f);
    float cdt = std::clamp(dt,0.001f,0.05f);
    if (damping>0) {
        float alpha = 1.0f - std::exp(-damping * cdt);
        curVel = curVel + (target - curVel) * alpha;
    } else curVel = target;
    SetVel(prim, curVel);
}

void SetCollide(std::uintptr_t partAddr, bool collide) {
    if (!partAddr)
        return;
    const uintptr_t prim = memory->read<uintptr_t>(partAddr + Offsets::BasePart::Primitive);
    if (!prim)
        return;
    uint8_t flags = memory->read<uint8_t>(prim + Offsets::Primitive::Flags);
    const uint8_t bit = (uint8_t)Offsets::PrimitiveFlags::CanCollide;
    const uint8_t next = collide ? (uint8_t)(flags | bit) : (uint8_t)(flags & ~bit);
    if (next != flags)
        memory->write<uint8_t>(prim + Offsets::Primitive::Flags, next);
}

void TickNoclip(bool& wasOn) {
    static bool tog = false;
    static bool was = false;
    const bool on = variables::Movement::noclip && KeyActive(variables::Movement::noclipKey, variables::Movement::noclipKeyMode, tog, was);
    auto localChar = Globals::localPlayer.GetModelRef();
    if (!on) {
        if (wasOn && localChar.Addr) {
            const auto& limbs = PlayerCache::GetLimbs(localChar.Addr);
            const std::uintptr_t parts[] = {limbs.hrp, limbs.head, limbs.torso, limbs.upperTorso, limbs.lowerTorso, limbs.lUpperArm, limbs.lLowerArm, limbs.lHand, limbs.rUpperArm, limbs.rLowerArm, limbs.rHand, limbs.lUpperLeg, limbs.lLowerLeg, limbs.lFoot, limbs.rUpperLeg, limbs.rLowerLeg, limbs.rFoot, limbs.lArm, limbs.rArm, limbs.lLeg, limbs.rLeg};
            for (auto p : parts)
                SetCollide(p, true);
        }
        wasOn = false;
        return;
    }
    if (!localChar.Addr)
        return;
    wasOn = true;
    if (variables::Movement::noclipMode == 1) {
        const auto& limbs = PlayerCache::GetLimbs(localChar.Addr);
        SetCollide(limbs.hrp, false);
        return;
    }
    const auto& limbs = PlayerCache::GetLimbs(localChar.Addr);
    const std::uintptr_t parts[] = {limbs.hrp, limbs.head, limbs.torso, limbs.upperTorso, limbs.lowerTorso, limbs.lUpperArm, limbs.lLowerArm, limbs.lHand, limbs.rUpperArm, limbs.rLowerArm, limbs.rHand, limbs.lUpperLeg, limbs.lLowerLeg, limbs.lFoot, limbs.rUpperLeg, limbs.rLowerLeg, limbs.rFoot, limbs.lArm, limbs.rArm, limbs.lLeg, limbs.rLeg};
    for (auto p : parts)
        SetCollide(p, false);
}

void TickBhop() {
    static bool was = false;
    static float backup = 16.0f;
    static std::uintptr_t backupHum = 0;
    static bool tog = false;
    static bool keyWas = false;
    auto localChar = Globals::localPlayer.GetModelRef();
    std::uintptr_t hum = localChar.Addr ? PlayerCache::GetLimbs(localChar.Addr).humanoid : 0;
    if (variables::Movement::bunnyHop && hum && KeyActive(variables::Movement::bunnyHopKey, variables::Movement::bunnyHopKeyMode, tog, keyWas)) {
        if (!was || backupHum != hum) {
            backup = memory->read<float>(hum + Offsets::Humanoid::Walkspeed);
            if (!std::isfinite(backup) || backup <= 0.0f)
                backup = 16.0f;
            backupHum = hum;
        }
        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0) {
            memory->write<bool>(hum + Offsets::Humanoid::Jump, true);
            const float spd = std::clamp(variables::Movement::bunnyHopSpeed, 16.0f, 250.0f);
            memory->write<float>(hum + Offsets::Humanoid::Walkspeed, spd);
            memory->write<float>(hum + Offsets::Humanoid::WalkspeedCheck, spd);
        }
        was = true;
    } else if (was) {
        if (backupHum) {
            memory->write<float>(backupHum + Offsets::Humanoid::Walkspeed, backup);
            memory->write<float>(backupHum + Offsets::Humanoid::WalkspeedCheck, backup);
        }
        was = false;
    }
}

}

void TickGravity() {
    static bool was = false;
    static float backup = 196.2f;
    std::uintptr_t world = 0;
    if (Globals::workspace.Addr)
        world = memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::World);
    if (variables::Misc::gravity && world) {
        if (!was) {
            backup = memory->read<float>(world + Offsets::World::Gravity);
            if (!std::isfinite(backup) || backup <= 0.0f)
                backup = 196.2f;
        }
        memory->write<float>(world + Offsets::World::Gravity, variables::Misc::gravityValue);
        was = true;
    } else if (was) {
        if (world)
            memory->write<float>(world + Offsets::World::Gravity, backup);
        was = false;
    }
}

void TickWalk() {
    static bool was = false;
    static float backup = 16.0f;
    static std::uintptr_t backupHum = 0;
    auto localChar = Globals::localPlayer.GetModelRef();
    std::uintptr_t hum = localChar.Addr ? PlayerCache::GetLimbs(localChar.Addr).humanoid : 0;
    if (variables::Misc::walkSpeed && hum) {
        if (!was || backupHum != hum) {
            backup = memory->read<float>(hum + Offsets::Humanoid::Walkspeed);
            if (!std::isfinite(backup) || backup <= 0.0f)
                backup = 16.0f;
            backupHum = hum;
        }
        memory->write<float>(hum + Offsets::Humanoid::Walkspeed, variables::Misc::walkSpeedValue);
        memory->write<float>(hum + Offsets::Humanoid::WalkspeedCheck, variables::Misc::walkSpeedValue);
        was = true;
    } else if (was) {
        if (backupHum) {
            memory->write<float>(backupHum + Offsets::Humanoid::Walkspeed, backup);
            memory->write<float>(backupHum + Offsets::Humanoid::WalkspeedCheck, backup);
        }
        was = false;
    }
}

void TickHip() {
    static bool was = false;
    static float backup = 2.0f;
    static std::uintptr_t backupHum = 0;
    static float lastWritten = -1.0f;
    auto localChar = Globals::localPlayer.GetModelRef();
    std::uintptr_t hum = localChar.Addr ? PlayerCache::GetLimbs(localChar.Addr).humanoid : 0;
    if (variables::Movement::hipHeight && hum) {
        if (!was || backupHum != hum) {
            backup = memory->read<float>(hum + Offsets::Humanoid::HipHeight);
            if (!std::isfinite(backup) || backup < 0.0f)
                backup = 2.0f;
            backupHum = hum;
            variables::Movement::hipHeightValue = backup;
            lastWritten = backup;
        }
        if (variables::Movement::hipHeightValue != lastWritten) {
            memory->write<float>(hum + Offsets::Humanoid::HipHeight, variables::Movement::hipHeightValue);
            lastWritten = variables::Movement::hipHeightValue;
        }
        was = true;
    } else if (was) {
        if (backupHum)
            memory->write<float>(backupHum + Offsets::Humanoid::HipHeight, backup);
        was = false;
        lastWritten = -1.0f;
    }
}

void Loop() {
    using namespace std::chrono_literals;
    bool gravOver=false; float gravBackup=0;
    bool noclipOn=false;
    while (Globals::running) {
        if (memory->IsConnected() && Globals::localPlayer.Addr) {
            TickFly(gravOver,gravBackup);
            TickNoclip(noclipOn);
            TickBhop();
            TickHip();
            TickWalk();
            TickGravity();
        }
        std::this_thread::sleep_for(variables::Movement::fly ? 1ms : 8ms);
    }
    
    if (gravOver && Globals::workspace.Addr) {
        auto world = memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::World);
        if (world) memory->write<float>(world + Offsets::World::Gravity, gravBackup);
    }
}
}
