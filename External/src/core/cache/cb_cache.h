#pragma once
#include "../../../src/sdk/sdk.h"
#include "../globals/globals.h"
#include "../variables/variables.h"
#include "cache.h"
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>

namespace CbCache {
inline std::vector<PlayerCache::CachedPlayer> players;
inline std::uintptr_t charactersAddr = 0;
inline std::uintptr_t localTeam = 0;
inline RBX::Vec3 localPos{};
inline std::chrono::steady_clock::time_point lastFull{};

inline std::uintptr_t FindCharactersFolder() {
    if (!Globals::workspace.Addr)
        return 0;
    for (auto& c : Globals::workspace.GetChildList()) {
        if (c.GetClass() == "Folder" && c.GetName() == "Characters")
            return c.Addr;
    }
    return 0;
}

inline std::unordered_set<std::string> GetRadarTeammates() {
    std::unordered_set<std::string> mates;
    if (!Globals::localPlayer.Addr)
        return mates;
    auto pg = Globals::localPlayer.FindChild("PlayerGui");
    if (!pg.Addr)
        return mates;
    auto mg = pg.FindChild("MainGui");
    if (!mg.Addr)
        return mates;
    auto gp = mg.FindChild("Gameplay");
    if (!gp.Addr)
        return mates;
    auto mid = gp.FindChild("Middle");
    if (!mid.Addr)
        return mates;
    auto radar = mid.FindChild("Radar");
    if (!radar.Addr)
        return mates;
    auto radarInner = radar.FindChild("Radar");
    auto targetFolder = radarInner.Addr ? radarInner : radar;
    for (auto& c : targetFolder.GetChildList()) {
        std::string name = c.GetName();
        if (name.empty() || name == "LocalPlayer" || name == "Direction" ||
            name == "Indicator" || name == "Map" || name == "A_Icon" ||
            name == "B_Icon" || name == "RunningCircle" || name == "DamageIndicator" ||
            name == "NoiseProximity" || name == "BuyMenu")
            continue;
        if (name.size() > 5 && name.substr(name.size() - 5) == "_Dead")
            name = name.substr(0, name.size() - 5);
        mates.insert(name);
    }
    return mates;
}

inline void RefreshFast() {
    if (Globals::camera.Addr)
        localPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);
    for (auto& p : players) {
        if (!p.isValid)
            continue;
        const auto cp = memory->read<std::uintptr_t>(p.characterAddr + Offsets::Instance::Parent);
        if (!cp || (charactersAddr && cp != charactersAddr && cp != p.teamAddr)) {
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
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFull).count() < 100) {
        RefreshFast();
        return;
    }
    lastFull = now;

    players.clear();
    charactersAddr = 0;
    localTeam = 0;

    if (!Globals::workspace.Addr)
        return;
    const auto chars = FindCharactersFolder();
    if (!chars)
        return;
    charactersAddr = chars;

    std::string localName;
    if (Globals::localPlayer.Addr)
        localName = Globals::localPlayer.GetName();

    const auto radarTeammates = GetRadarTeammates();
    if (!radarTeammates.empty()) {
        localTeam = 1;
    }

    RBX::RbxInstance ws{chars};
    auto childList = ws.GetChildList();

    auto processModel = [&](RBX::RbxInstance& model, std::uintptr_t folderTeamAddr, const std::string& folderTeamName) {
        if (model.GetClass() != "Model")
            return;
        const std::string name = model.GetName();
        if (name.empty())
            return;
        if (!localName.empty() && name == localName) {
            if (folderTeamAddr)
                localTeam = folderTeamAddr;
            return;
        }

        const auto charParent = memory->read<std::uintptr_t>(model.Addr + Offsets::Instance::Parent);
        if (!charParent || (folderTeamAddr ? (charParent != folderTeamAddr) : (charParent != charactersAddr)))
            return;

        RBX::RbxInstance m{model.Addr};
        auto head = m.FindChild("Head");
        auto hrp = m.FindChild("HumanoidRootPart");
        if (!head.Addr || !hrp.Addr)
            return;

        auto hum = m.FindChildByClass("Humanoid");
        float hp = 100.0f;
        float maxHp = 100.0f;
        if (hum.Addr) {
            hp = memory->read<float>(hum.Addr + Offsets::Humanoid::Health);
            maxHp = memory->read<float>(hum.Addr + Offsets::Humanoid::MaxHealth);
            if (hp <= 0.0f)
                return;
        }

        PlayerCache::CachedPlayer c{};
        c.playerAddr = model.Addr;
        c.characterAddr = model.Addr;
        c.humanoidAddr = hum.Addr;
        c.rootPartAddr = hrp.Addr;
        c.headAddr = head.Addr;
        c.name = name;
        c.health = hp;
        c.maxHealth = maxHp;
        c.isR6 = false;
        c.isValid = true;
        c.position = hrp.GetPos();

        if (folderTeamAddr) {
            c.teamAddr = folderTeamAddr;
            c.teamName = folderTeamName;
        } else if (!radarTeammates.empty()) {
            if (radarTeammates.count(name)) {
                c.teamAddr = 1;
                c.teamName = "Teammate";
            } else {
                c.teamAddr = 2;
                c.teamName = "Enemy";
            }
        } else {
            std::uintptr_t pTeam = 0;
            if (Globals::players.Addr) {
                auto plr = Globals::players.FindChild(name);
                if (plr.Addr)
                    pTeam = memory->read<std::uintptr_t>(plr.Addr + Offsets::Player::Team);
            }
            c.teamAddr = pTeam;
            c.teamName = pTeam ? RBX::RbxInstance(pTeam).GetName() : std::string{};
        }

        c.tool = "None";
        for (auto& child : m.GetChildList()) {
            if (child.GetClass() == "Tool") {
                c.tool = child.GetName();
                break;
            }
        }
        if (c.tool == "None") {
            for (auto& child : m.GetChildList()) {
                if (child.GetClass() == "Model" && child.GetName() != "CharacterArmor") {
                    c.tool = child.GetName();
                    break;
                }
            }
        }

        PlayerCache::GetLimbs(model.Addr, false);

        players.push_back(std::move(c));
    };

    for (auto& item : childList) {
        const std::string itemClass = item.GetClass();
        if (itemClass == "Model") {
            processModel(item, 0, "");
        } else if (itemClass == "Folder") {
            if (item.GetName() == "Hostages")
                continue;
            for (auto& model : item.GetChildList()) {
                processModel(model, item.Addr, item.GetName());
            }
        }
    }

    if (Globals::camera.Addr)
        localPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);
}
}
