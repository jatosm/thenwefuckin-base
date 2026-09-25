// discord.gg/thenwefuckin
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <Windows.h>
#include "../../../sdk/sdk.h"
#include "../../variables/variables.h"
#include "../../cache/cache.h"
#include "../../../memory/memory.h"
#include "../../../sdk/offsets.h"

struct FallenWeaponBallistics {
    float bv;
    float grav;
    float spd_sprint;
    float spd_walk;
    float spd_ads;
};
static const std::unordered_map<std::string, FallenWeaponBallistics> FALLEN_WEAPONS = {
    {"Military M4A1",        {2100.f, 0.55f, 16.2f, 9.9f, 7.9f}},
    {"Bruno's M4A1",         {2100.f, 0.55f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged AK47",        {2100.f, 0.55f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged AK74U",       {1800.f, 0.60f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged AK74u",       {1800.f, 0.60f, 16.2f, 9.9f, 7.9f}},
    {"Military PKM",         {2400.f, 0.55f, 16.2f, 9.9f, 7.9f}},
    {"Military MP7",         {1900.f, 0.60f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged SMG",         {1800.f, 0.60f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged Skorpion",    {1600.f, 0.60f, 16.2f, 9.9f, 7.9f}},
    {"Military Barrett",     {2500.f, 0.55f, 16.2f, 9.9f, 7.9f}},
    {"Military M39",         {2400.f, 0.52f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged Sniper",      {2400.f, 0.55f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged M14",         {2100.f, 0.55f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged Pipe Rifle",  {1700.f, 0.60f, 16.2f, 9.9f, 7.9f}},
    {"Military USP",         {1500.f, 0.60f, 17.1f, 10.4f, 8.4f}},
    {"Salvaged Python",      {1800.f, 0.60f, 17.1f, 10.4f, 8.4f}},
    {"Salvaged P250",        {1400.f, 0.60f, 17.1f, 10.4f, 8.4f}},
    {"Military AA12",        { 600.f, 0.60f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged Pump Action", { 650.f, 0.60f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged Break Action",{ 550.f, 0.60f, 17.1f, 10.4f, 8.4f}},
    {"Salvaged Shotgun",     { 400.f, 0.60f, 17.1f, 10.4f, 8.4f}},
    {"Nail Gun",             { 165.f, 0.18f, 18.0f, 11.0f, 8.8f}},
    {"Crossbow",             { 420.f, 0.17f, 17.1f, 10.4f, 8.4f}},
    {"Wooden Bow",           { 280.f, 0.17f, 18.0f, 11.0f, 8.8f}},
    {"Salvaged RPG",               {100.f, 0.12f, 16.2f, 9.9f, 7.9f}},
    {"Pumpkin Launcher",           { 80.f, 0.16f, 16.2f, 9.9f, 7.9f}},
    {"Military Grenade Launcher",  { 85.f, 0.15f, 16.2f, 9.9f, 7.9f}},
    {"Salvaged Grenade Launcher",  { 85.f, 0.15f, 16.2f, 9.9f, 7.9f}},
};
static const std::vector<std::string> FALLEN_WEAPON_LIST = {
    "Auto Detect",
    "Military M4A1",
    "Bruno's M4A1",
    "Salvaged AK47",
    "Salvaged AK74U",
    "Salvaged AK74u",
    "Military PKM",
    "Military MP7",
    "Salvaged SMG",
    "Salvaged Skorpion",
    "Military Barrett",
    "Military M39",
    "Salvaged Sniper",
    "Salvaged M14",
    "Salvaged Pipe Rifle",
    "Military USP",
    "Salvaged Python",
    "Salvaged P250",
    "Military AA12",
    "Salvaged Pump Action",
    "Salvaged Break Action",
    "Salvaged Shotgun",
    "Nail Gun",
    "Crossbow",
    "Wooden Bow",
    "Salvaged RPG",
    "Pumpkin Launcher",
    "Military Grenade Launcher",
    "Salvaged Grenade Launcher",
    "Custom / Manual"
};
static constexpr float FALLEN_GRAVITY = 196.2f;
inline float fallen_get_base_bv(const std::string& weapon) {
    auto it = FALLEN_WEAPONS.find(weapon);
    return (it != FALLEN_WEAPONS.end()) ? it->second.bv : 0.f;
}
inline float fallen_get_base_grav(const std::string& weapon) {
    auto it = FALLEN_WEAPONS.find(weapon);
    return (it != FALLEN_WEAPONS.end()) ? it->second.grav : 0.f;
}
inline const FallenWeaponBallistics* fallen_get_weapon_data(const std::string& weapon) {
    auto it = FALLEN_WEAPONS.find(weapon);
    return (it != FALLEN_WEAPONS.end()) ? &it->second : nullptr;
}
inline float fallen_get_bullet_velocity(const std::string& weapon) {
    if (variables::Aimbot::fallen_bv_override > 0.f) return variables::Aimbot::fallen_bv_override;
    return fallen_get_base_bv(weapon);
}
inline std::string fallen_detect_weapon(uintptr_t character, const std::string& playerName) {
    if (!character) return "";
    try {
        std::string vmWeapon = PlayerCache::GetWeaponFromViewModels(playerName);
        if (vmWeapon != "None" && !vmWeapon.empty()) {
            return vmWeapon;
        }
        RBX::RbxInstance charInst(character);
        auto children = charInst.GetChildList();
        for (auto& child : children) {
            if (child.Addr == 0) continue;
            std::string cls = child.GetClass();
            if (cls == "Tool") {
                return child.GetName();
            }
            if (cls == "Model") {
                auto handle = child.FindChild("Handle");
                if (handle.Addr == 0) handle = child.FindChild("Main");
                if (handle.Addr == 0) continue;
                std::string name = child.GetName();
                if (name == "HolsterModel") continue;
                return name;
            }
        }
    } catch (...) {}
    return "";
}
inline std::string fallen_get_local_weapon() {
    try {
        if (Globals::localPlayer.Addr == 0) return "";
        auto lp_char = Globals::localPlayer.GetModelRef();
        if (lp_char.Addr == 0) return "";
        return fallen_detect_weapon(lp_char.Addr, Globals::localPlayer.GetName());
    } catch (...) {}
    return "";
}
inline rbx::vector3_t fallen_get_local_velocity() {
    try {

        if (PlayerCache::localRootPrim)
            return memory->read<rbx::vector3_t>(PlayerCache::localRootPrim + Offsets::Primitive::AssemblyLinearVelocity);
        if (Globals::localPlayer.Addr == 0) return {};
        auto lp_char = Globals::localPlayer.GetModelRef();
        if (lp_char.Addr == 0) return {};
        auto hrp = lp_char.FindChild("HumanoidRootPart");
        if (hrp.Addr == 0) return {};
        uintptr_t prim = hrp.GetPrimitivePtr();
        if (prim == 0) return {};
        return memory->read<rbx::vector3_t>(prim + Offsets::Primitive::AssemblyLinearVelocity);
    } catch (...) {}
    return {};
}

inline void fallen_clamp_velocity(rbx::vector3_t& vel, float max_speed) {
    if (max_speed <= 0.f) return;
    float mag_xz = sqrtf(vel.x * vel.x + vel.z * vel.z);
    if (mag_xz > max_speed && mag_xz > 0.01f) {
        float scale = max_speed / mag_xz;
        vel.x *= scale;
        vel.z *= scale;
    }
}

static inline std::string g_fallen_cached_wpn;
static inline float       g_fallen_cached_base_bv = 0;
static inline DWORD       g_fallen_last_wpn_check = 0;
inline void fallen_update_weapon_auto()
{
    DWORD now = GetTickCount();
    if (now - g_fallen_last_wpn_check < 500) return;
    g_fallen_last_wpn_check = now;
    if (variables::Aimbot::selected_weapon_index == 0) {
        std::string new_wpn = fallen_get_local_weapon();
        if (new_wpn.empty()) return;
        float new_base = fallen_get_base_bv(new_wpn);
        if (new_wpn != g_fallen_cached_wpn) {
            variables::Aimbot::fallen_bv_override = new_base;
            variables::Aimbot::fallen_grav_mult = fallen_get_base_grav(new_wpn);
            variables::Aimbot::detected_weapon_name = new_wpn;
        }
        g_fallen_cached_wpn = new_wpn;
        g_fallen_cached_base_bv = new_base;
    }
    else if (variables::Aimbot::selected_weapon_index > 0 &&
             variables::Aimbot::selected_weapon_index < (int)FALLEN_WEAPON_LIST.size() - 1) {
        static int lastSel = -1;
        if (variables::Aimbot::selected_weapon_index != lastSel) {
            lastSel = variables::Aimbot::selected_weapon_index;
            std::string selected_wpn = FALLEN_WEAPON_LIST[variables::Aimbot::selected_weapon_index];
            variables::Aimbot::fallen_bv_override = fallen_get_base_bv(selected_wpn);
            variables::Aimbot::fallen_grav_mult = fallen_get_base_grav(selected_wpn);
            variables::Aimbot::detected_weapon_name = selected_wpn;
        }
    }
    else {
        variables::Aimbot::detected_weapon_name = "Custom / Manual";
    }
}
inline void fallen_predict(rbx::vector3_t& target_pos, const rbx::vector3_t& cam_pos,
                           const rbx::vector3_t& target_vel, float bullet_vel,
                           const rbx::vector3_t& local_vel = {}) {
    if (bullet_vel <= 0.f) return;
    float dist = sqrtf(
        (target_pos.x - cam_pos.x) * (target_pos.x - cam_pos.x) +
        (target_pos.y - cam_pos.y) * (target_pos.y - cam_pos.y) +
        (target_pos.z - cam_pos.z) * (target_pos.z - cam_pos.z));
    if (dist < 1.f) return;
    float ox = target_pos.x, oy = target_pos.y, oz = target_pos.z;
    for (int i = 0; i < 3; i++) {
        float dx = target_pos.x - cam_pos.x;
        float dy = target_pos.y - cam_pos.y;
        float dz = target_pos.z - cam_pos.z;
        float inv_dist = (dist > 0.1f) ? 1.f / dist : 0.f;
        float dir_x = dx * inv_dist, dir_y = dy * inv_dist, dir_z = dz * inv_dist;
        float vel_proj = local_vel.x * dir_x + local_vel.y * dir_y + local_vel.z * dir_z;
        float eff_bv = bullet_vel + vel_proj;
        if (eff_bv < 10.f) eff_bv = 10.f;
        float ft = dist / eff_bv;
        float px = ox + target_vel.x * ft;
        float pz = oz + target_vel.z * ft;
        float py = oy + target_vel.y * ft + 0.5f * FALLEN_GRAVITY * variables::Aimbot::fallen_grav_mult * ft * ft;
        dx = px - cam_pos.x;
        dy = py - cam_pos.y;
        dz = pz - cam_pos.z;
        dist = sqrtf(dx * dx + dy * dy + dz * dz);
        target_pos.x = px;
        target_pos.y = py;
        target_pos.z = pz;
    }
}
