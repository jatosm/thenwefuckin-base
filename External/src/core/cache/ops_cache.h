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
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <mutex>

namespace OpsCache {
inline std::vector<PlayerCache::CachedPlayer> players;
inline std::uintptr_t viewmodelsAddr = 0;
inline std::uintptr_t localTeam = 0;
inline RBX::Vec3 localPos{};
inline std::chrono::steady_clock::time_point lastFull{};

inline std::uintptr_t FindViewmodelsFolder() {
    if (!Globals::workspace.Addr)
        return 0;
    for (auto& c : Globals::workspace.GetChildList()) {
        if (c.GetClass() == "Folder" && (c.GetName() == "Viewmodels" || c.GetName() == "ViewModels"))
            return c.Addr;
    }
    return 0;
}

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
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFull).count() < 100) {
        RefreshFast();
        return;
    }
    lastFull = now;

    viewmodelsAddr = FindViewmodelsFolder();
    if (!viewmodelsAddr) {
        players.clear();
        return;
    }

    std::string localName;
    if (Globals::localPlayer.Addr) {
        localName = Globals::localPlayer.GetName();
        localTeam = memory->read<std::uintptr_t>(Globals::localPlayer.Addr + Offsets::Player::Team);
    }
    if (Globals::camera.Addr)
        localPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);

    struct VmData {
        std::uintptr_t modelAddr = 0;
        std::uintptr_t head = 0;
        std::uintptr_t torso = 0;
        std::uintptr_t arm1 = 0;
        std::uintptr_t arm2 = 0;
        std::uintptr_t leg1 = 0;
        std::uintptr_t leg2 = 0;
        std::uintptr_t shoulder1 = 0;
        std::uintptr_t shoulder2 = 0;
        std::uintptr_t hip1 = 0;
        std::uintptr_t hip2 = 0;
        RBX::Vec3 torsoPos{};
        std::string username;
        std::string weapon;
        bool isLocal = false;
    };

    std::vector<VmData> vmList;
    RBX::RbxInstance vmFolder{viewmodelsAddr};
    for (auto& child : vmFolder.GetChildList()) {
        if (child.GetClass() != "Model")
            continue;
        const std::string vmName = child.GetName();
        auto head = child.FindChild("head");
        auto torso = child.FindChild("torso");
        if (!head.Addr || !torso.Addr)
            continue;

        VmData vmd{};
        vmd.modelAddr = child.Addr;
        vmd.head = head.Addr;
        vmd.torso = torso.Addr;
        vmd.arm1 = child.FindChild("arm1").Addr;
        vmd.arm2 = child.FindChild("arm2").Addr;
        vmd.leg1 = child.FindChild("leg1").Addr;
        vmd.leg2 = child.FindChild("leg2").Addr;
        vmd.shoulder1 = child.FindChild("shoulder1").Addr;
        vmd.shoulder2 = child.FindChild("shoulder2").Addr;
        vmd.hip1 = child.FindChild("hip1").Addr;
        vmd.hip2 = child.FindChild("hip2").Addr;
        vmd.torsoPos = torso.GetPos();
        vmd.isLocal = (vmName == "LocalViewmodel");

        auto unGui = head.FindChild("Username");
        if (unGui.Addr) {
            auto unLabel = unGui.FindChild("Username");
            if (unLabel.Addr) {
                std::string txt = memory->read_string(unLabel.Addr + Offsets::TextLabel::Text);
                if (!txt.empty())
                    vmd.username = txt;
            }
        }

        for (auto& sub : child.GetChildList()) {
            if (sub.GetClass() == "Model") {
                const std::string sn = sub.GetName();
                if (sn != "shoulder1" && sn != "shoulder2" && sn != "torso" && sn != "head" &&
                    sn != "hip1" && sn != "hip2" && sn != "leg1" && sn != "leg2" &&
                    sn != "arm1" && sn != "arm2" && sn != "Model") {
                    vmd.weapon = sn;
                    break;
                }
            }
        }

        vmList.push_back(vmd);
    }

    if (!localName.empty() && Globals::localPlayer.Addr) {
        auto localChar = Globals::localPlayer.GetModelRef();
        if (!localChar.Addr && Globals::workspace.Addr)
            localChar = Globals::workspace.FindChild(localName);
        if (localChar.Addr) {
            for (const auto& vm : vmList) {
                if (vm.isLocal) {
                    auto lhrp = localChar.FindChild("HumanoidRootPart");
                    PlayerCache::LimbAddrs l{};
                    l.r6 = false;
                    l.head = vm.head;
                    l.hrp = lhrp.Addr;
                    l.upperTorso = vm.torso;
                    l.lowerTorso = vm.torso;
                    l.lUpperArm = vm.shoulder1 ? vm.shoulder1 : vm.arm1;
                    l.lLowerArm = vm.arm1;
                    l.rUpperArm = vm.shoulder2 ? vm.shoulder2 : vm.arm2;
                    l.rLowerArm = vm.arm2;
                    l.lUpperLeg = vm.hip1 ? vm.hip1 : vm.leg1;
                    l.lLowerLeg = vm.leg1;
                    l.rUpperLeg = vm.hip2 ? vm.hip2 : vm.leg2;
                    l.rLowerLeg = vm.leg2;
                    l.humanoid = localChar.FindChildByClass("Humanoid").Addr;

                    std::lock_guard<std::mutex> lk(PlayerCache::limbMutex);
                    PlayerCache::limbCache[localChar.Addr] = l;
                    break;
                }
            }
        }
    }

    std::vector<PlayerCache::CachedPlayer> newPlayers;
    if (!Globals::players.Addr)
        return;

    std::vector<bool> vmUsed(vmList.size(), false);

    for (auto& plr : Globals::players.GetChildList()) {
        if (plr.Addr == Globals::localPlayer.Addr)
            continue;
        const std::string name = plr.GetName();
        if (name.empty())
            continue;

        RBX::RbxInstance charModel = plr.GetModelRef();
        if (!charModel.Addr && Globals::workspace.Addr)
            charModel = Globals::workspace.FindChild(name);
        if (!charModel.Addr)
            continue;

        const auto cp = memory->read<std::uintptr_t>(charModel.Addr + Offsets::Instance::Parent);
        if (!cp || cp != Globals::workspace.Addr)
            continue;

        auto hrp = charModel.FindChild("HumanoidRootPart");
        if (!hrp.Addr)
            continue;
        const auto rpParent = memory->read<std::uintptr_t>(hrp.Addr + Offsets::Instance::Parent);
        if (rpParent != charModel.Addr)
            continue;

        auto hum = charModel.FindChildByClass("Humanoid");
        float hp = 100.0f;
        float maxHp = 100.0f;
        if (hum.Addr) {
            hp = memory->read<float>(hum.Addr + Offsets::Humanoid::Health);
            maxHp = memory->read<float>(hum.Addr + Offsets::Humanoid::MaxHealth);
            if (hp <= 0.0f)
                continue;
        }

        const RBX::Vec3 hrpPos = hrp.GetPos();
        if (hrpPos.X == 0 && hrpPos.Y == 0 && hrpPos.Z == 0)
            continue;

        int matchIdx = -1;

        for (size_t i = 0; i < vmList.size(); ++i) {
            if (vmUsed[i] || vmList[i].isLocal)
                continue;
            if (!vmList[i].username.empty() && vmList[i].username == name) {
                matchIdx = static_cast<int>(i);
                break;
            }
        }

        if (matchIdx == -1) {
            float bestDistSq = 36.0f;
            for (size_t i = 0; i < vmList.size(); ++i) {
                if (vmUsed[i] || vmList[i].isLocal)
                    continue;
                const float dx = hrpPos.X - vmList[i].torsoPos.X;
                const float dy = hrpPos.Y - vmList[i].torsoPos.Y;
                const float dz = hrpPos.Z - vmList[i].torsoPos.Z;
                const float d2 = dx * dx + dy * dy + dz * dz;
                if (d2 < bestDistSq) {
                    bestDistSq = d2;
                    matchIdx = static_cast<int>(i);
                }
            }
        }

        std::uintptr_t headAddr = 0;
        std::uintptr_t torsoAddr = 0;
        std::string weaponName = "None";

        if (matchIdx != -1) {
            vmUsed[matchIdx] = true;
            const auto& vm = vmList[matchIdx];
            headAddr = vm.head;
            torsoAddr = vm.torso;
            weaponName = vm.weapon.empty() ? "None" : vm.weapon;

            PlayerCache::LimbAddrs l{};
            l.r6 = false;
            l.head = vm.head;
            l.hrp = hrp.Addr;
            l.upperTorso = vm.torso;
            l.lowerTorso = vm.torso;
            l.lUpperArm = vm.shoulder1 ? vm.shoulder1 : vm.arm1;
            l.lLowerArm = vm.arm1;
            l.rUpperArm = vm.shoulder2 ? vm.shoulder2 : vm.arm2;
            l.rLowerArm = vm.arm2;
            l.lUpperLeg = vm.hip1 ? vm.hip1 : vm.leg1;
            l.lLowerLeg = vm.leg1;
            l.rUpperLeg = vm.hip2 ? vm.hip2 : vm.leg2;
            l.rLowerLeg = vm.leg2;
            l.humanoid = hum.Addr;

            std::lock_guard<std::mutex> lk(PlayerCache::limbMutex);
            PlayerCache::limbCache[charModel.Addr] = l;
        } else {
            headAddr = hrp.Addr;
            torsoAddr = hrp.Addr;
        }

        const auto team = memory->read<std::uintptr_t>(plr.Addr + Offsets::Player::Team);

        PlayerCache::CachedPlayer c{};
        c.playerAddr = plr.Addr;
        c.characterAddr = charModel.Addr;
        c.humanoidAddr = hum.Addr;
        c.rootPartAddr = hrp.Addr;
        c.headAddr = headAddr;
        c.teamAddr = team;
        c.teamName = team ? RBX::RbxInstance(team).GetName() : std::string{};
        c.name = name;
        c.tool = weaponName;
        c.health = hp;
        c.maxHealth = maxHp;
        c.isR6 = false;
        c.isValid = true;
        c.position = hrpPos;

        newPlayers.push_back(std::move(c));
    }

    players = std::move(newPlayers);
}
}
