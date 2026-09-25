// discord.gg/thenwefuckin
#pragma once
#include "../../../src/sdk/sdk.h"
#include "../globals/globals.h"
#include "../variables/variables.h"
#include "cache.h"
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>
#include <unordered_map>

namespace MlCache {
inline std::vector<PlayerCache::CachedPlayer> players;
inline std::uintptr_t charactersAddr = 0;
inline std::uintptr_t localTeam = 0;
inline RBX::Vec3 localPos{};
inline std::chrono::steady_clock::time_point lastFull{};

inline void RefreshFast() {
    if (Globals::camera.Addr)
        localPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);

    for (auto& p : players) {
        if (!p.isValid)
            continue;

        const auto cp = memory->read<std::uintptr_t>(p.characterAddr + Offsets::Instance::Parent);
        if (!cp || (Globals::workspace.Addr && cp != Globals::workspace.Addr)) {
            p.isValid = false;
            continue;
        }

        const auto rp = memory->read<std::uintptr_t>(p.rootPartAddr + Offsets::Instance::Parent);
        if (rp != p.characterAddr) {
            p.isValid = false;
            continue;
        }

        if (p.humanoidAddr) {
            const auto hpParent = memory->read<std::uintptr_t>(p.humanoidAddr + Offsets::Instance::Parent);
            if (hpParent != p.characterAddr) {
                p.isValid = false;
                continue;
            }
            p.health = memory->read<float>(p.humanoidAddr + Offsets::Humanoid::Health);
            if (p.health <= 0.0f) {
                p.isValid = false;
                continue;
            }
        }

        if (p.rootPartAddr) {
            p.position = RBX::RbxInstance(p.rootPartAddr).GetPos();
        }
    }
}

inline void update() {
    const auto now = std::chrono::steady_clock::now();
    if (lastFull != std::chrono::steady_clock::time_point{} &&
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFull).count() < 500) {
        RefreshFast();
        return;
    }
    lastFull = now;

    players.clear();
    charactersAddr = 0;
    localTeam = 0;

    if (!Globals::workspace.Addr)
        return;
    if (Globals::camera.Addr)
        localPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);

    struct Owner {
        std::uintptr_t playerAddr = 0;
        std::string name;
        std::uintptr_t team = 0;
    };
    std::unordered_map<std::uintptr_t, Owner> owners;
    std::string localName;
    std::uintptr_t localCharAddr = 0;
    if (Globals::players.Addr) {
        if (Globals::localPlayer.Addr) {
            localName = Globals::localPlayer.GetName();
            localCharAddr = Globals::localPlayer.GetModelRef().Addr;
        }
        for (auto& plr : Globals::players.GetChildList()) {
            if (!plr.Addr || plr.Addr == Globals::localPlayer.Addr)
                continue;
            const auto ch = plr.GetModelRef().Addr;
            if (!ch)
                continue;
            Owner o;
            o.playerAddr = plr.Addr;
            o.name = plr.GetName();
            o.team = memory->read<std::uintptr_t>(plr.Addr + Offsets::Player::Team);
            owners[ch] = std::move(o);
        }
    }

    std::uintptr_t camSubject = 0;
    if (Globals::camera.Addr)
        camSubject = memory->read<std::uintptr_t>(Globals::camera.Addr + Offsets::Camera::CameraSubject);

    int scanned = 0;
    int foldersScanned = 0;
    auto processMale = [&](RBX::RbxInstance& item) {
        if (scanned >= 400)
            return;
        if (item.GetClass() != "Model" || item.GetName() != "Male")
            return;
        ++scanned;

        std::uintptr_t headAddr = 0, rootAddr = 0, humAddr = 0;
        std::string tool = "None";
        for (auto& child : RBX::RbxInstance(item.Addr).GetChildList()) {
            const std::string cn = child.GetName();
            const std::string cc = child.GetClass();
            if (!headAddr && (cn == "Head" || cn == "FakeHead") &&
                (cc == "MeshPart" || cc == "Part"))
                headAddr = child.Addr;
            else if (!rootAddr && cn == "Root" && (cc == "Part" || cc == "MeshPart"))
                rootAddr = child.Addr;
            else if (!humAddr && cc == "Humanoid")
                humAddr = child.Addr;
            else if (tool == "None" && cc == "Tool")
                tool = cn;
        }
        if (!headAddr || !rootAddr || !humAddr)
            return;

        const float hp = memory->read<float>(humAddr + Offsets::Humanoid::Health);
        if (!std::isfinite(hp) || hp <= 0.0f)
            return;
        const float maxHp = memory->read<float>(humAddr + Offsets::Humanoid::MaxHealth);

        if (item.Addr == localCharAddr || item.Addr == camSubject || humAddr == camSubject)
            return;

        PlayerCache::CachedPlayer c{};
        c.playerAddr = item.Addr;
        c.characterAddr = item.Addr;
        c.humanoidAddr = humAddr;
        c.rootPartAddr = rootAddr;
        c.headAddr = headAddr;
        c.health = hp;
        c.maxHealth = (std::isfinite(maxHp) && maxHp > 0.0f) ? maxHp : 100.0f;
        c.isR6 = false;
        c.isValid = true;
        c.position = RBX::RbxInstance(rootAddr).GetPos();
        c.tool = tool;

        if (auto oit = owners.find(item.Addr); oit != owners.end()) {
            c.playerAddr = oit->second.playerAddr;
            c.name = oit->second.name;
            c.teamAddr = oit->second.team;
            if (c.teamAddr)
                c.teamName = RBX::RbxInstance(c.teamAddr).GetName();
        } else {
            c.name = "Male";
        }

        PlayerCache::GetLimbs(item.Addr, false);
        players.push_back(std::move(c));
    };

    for (auto& item : Globals::workspace.GetChildList()) {
        if (scanned >= 400)
            break;
        if (item.GetClass() == "Model") {
            processMale(item);
        } else if (item.GetClass() == "Folder" && foldersScanned < 40) {
            ++foldersScanned;
            for (auto& sub : RBX::RbxInstance(item.Addr).GetChildList()) {
                if (scanned >= 400)
                    break;
                if (sub.GetClass() == "Model")
                    processMale(sub);
            }
        }
    }

    if (!players.empty())
        charactersAddr = Globals::workspace.Addr;
}
}
