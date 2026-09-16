#pragma once
#include "../../../src/sdk/sdk.h"
#include "../globals/globals.h"
#include "../variables/variables.h"
#include "../keys/keys.h"
#include <vector>
#include <string>
#include <utility>
#include <chrono>
#include <cstdio>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace PlayerCache {
struct CachedPlayer {
    std::uintptr_t playerAddr = 0;
    std::uintptr_t characterAddr = 0;
    std::uintptr_t humanoidAddr = 0;
    std::uintptr_t rootPartAddr = 0;
    std::uintptr_t headAddr = 0;
    std::uintptr_t teamAddr = 0;
    std::string name;
    std::string tool = "None";
    std::string teamName;
    RBX::Vec3 position{};
    float health = 0.0f;
    float maxHealth = 0.0f;
    float distance = 0.0f;
    bool isValid = false;
    bool isR6 = false;
    int role = 0; 
};



inline int ScanRole(std::uintptr_t characterAddr) {
    if (!characterAddr) return 0;
    RBX::RbxInstance ch{characterAddr};
    int sawTool = 0;
    std::string firstTool;
    for (auto& child : ch.GetChildList()) {
        if (child.GetClass() != "Tool") continue;
        sawTool++;
        const std::string tn = child.GetName();
        if (firstTool.empty()) firstTool = tn;
        if (tn == "Knife") return 1;
        if (tn == "Gun") return 2;
        for (auto& sub : child.GetChildList()) {
            const std::string sn = sub.GetName();
            if (sn == "KnifeServer") return 1;
            if (sn == "GunServer") return 2;
        }
    }
    return 0;
}
inline void DbgRoleChange(std::uintptr_t playerAddr, const std::string& name, int oldR, int newR) {
    (void)playerAddr; (void)name; (void)oldR; (void)newR;
}

struct LimbAddrs {
    bool r6 = false;
    std::uintptr_t head = 0;
    std::uintptr_t hrp = 0;
    std::uintptr_t torso = 0;
    std::uintptr_t upperTorso = 0;
    std::uintptr_t lowerTorso = 0;
    std::uintptr_t lUpperArm = 0;
    std::uintptr_t lLowerArm = 0;
    std::uintptr_t lHand = 0;
    std::uintptr_t rUpperArm = 0;
    std::uintptr_t rLowerArm = 0;
    std::uintptr_t rHand = 0;
    std::uintptr_t lUpperLeg = 0;
    std::uintptr_t lLowerLeg = 0;
    std::uintptr_t lFoot = 0;
    std::uintptr_t rUpperLeg = 0;
    std::uintptr_t rLowerLeg = 0;
    std::uintptr_t rFoot = 0;
    std::uintptr_t lArm = 0;
    std::uintptr_t rArm = 0;
    std::uintptr_t lLeg = 0;
    std::uintptr_t rLeg = 0;
    std::uintptr_t humanoid = 0;
};

inline std::unordered_map<std::uintptr_t, LimbAddrs> limbCache;

inline const LimbAddrs& GetLimbs(std::uintptr_t characterAddr) {
    static const LimbAddrs kEmpty{};
    if (!characterAddr)
        return kEmpty;
    auto it = limbCache.find(characterAddr);
    if (it != limbCache.end()) {
        
        
        LimbAddrs& e = it->second;
        bool ok = e.hrp && e.humanoid;
        if (ok) {
            const auto hp = memory->read<std::uintptr_t>(e.hrp + Offsets::Instance::Parent);
            const auto hh = memory->read<std::uintptr_t>(e.humanoid + Offsets::Instance::Parent);
            ok = (hp == characterAddr && hh == characterAddr);
        }
        if (!ok) {
            limbCache.erase(it);
        } else {
            
            
            
            RBX::RbxInstance ch{characterAddr};
            if (!e.head)
                e.head = ch.FindChild("Head").Addr;
            if (!e.torso) {
                e.torso = ch.FindChild("Torso").Addr;
                if (e.torso)
                    e.r6 = true;
            }
            if (e.r6) {
                if (!e.lArm) e.lArm = ch.FindChild("Left Arm").Addr;
                if (!e.rArm) e.rArm = ch.FindChild("Right Arm").Addr;
                if (!e.lLeg) e.lLeg = ch.FindChild("Left Leg").Addr;
                if (!e.rLeg) e.rLeg = ch.FindChild("Right Leg").Addr;
            } else {
                if (!e.upperTorso) e.upperTorso = ch.FindChild("UpperTorso").Addr;
                if (!e.lowerTorso) e.lowerTorso = ch.FindChild("LowerTorso").Addr;
                if (!e.lUpperArm) e.lUpperArm = ch.FindChild("LeftUpperArm").Addr;
                if (!e.lLowerArm) e.lLowerArm = ch.FindChild("LeftLowerArm").Addr;
                if (!e.lHand) e.lHand = ch.FindChild("LeftHand").Addr;
                if (!e.rUpperArm) e.rUpperArm = ch.FindChild("RightUpperArm").Addr;
                if (!e.rLowerArm) e.rLowerArm = ch.FindChild("RightLowerArm").Addr;
                if (!e.rHand) e.rHand = ch.FindChild("RightHand").Addr;
                if (!e.lUpperLeg) e.lUpperLeg = ch.FindChild("LeftUpperLeg").Addr;
                if (!e.lLowerLeg) e.lLowerLeg = ch.FindChild("LeftLowerLeg").Addr;
                if (!e.lFoot) e.lFoot = ch.FindChild("LeftFoot").Addr;
                if (!e.rUpperLeg) e.rUpperLeg = ch.FindChild("RightUpperLeg").Addr;
                if (!e.rLowerLeg) e.rLowerLeg = ch.FindChild("RightLowerLeg").Addr;
                if (!e.rFoot) e.rFoot = ch.FindChild("RightFoot").Addr;
            }
            return e;
        }
    }
    if (limbCache.size() >= 256)
        limbCache.clear();
    LimbAddrs l{};
    RBX::RbxInstance ch{characterAddr};
    auto head = ch.FindChild("Head");
    auto hrp = ch.FindChild("HumanoidRootPart");
    l.head = head.Addr;
    l.hrp = hrp.Addr;
    auto torso = ch.FindChild("Torso");
    l.r6 = torso.Addr != 0;
    l.torso = torso.Addr;
    if (l.r6) {
        l.lArm = ch.FindChild("Left Arm").Addr;
        l.rArm = ch.FindChild("Right Arm").Addr;
        l.lLeg = ch.FindChild("Left Leg").Addr;
        l.rLeg = ch.FindChild("Right Leg").Addr;
    } else {
        l.upperTorso = ch.FindChild("UpperTorso").Addr;
        l.lowerTorso = ch.FindChild("LowerTorso").Addr;
        l.lUpperArm = ch.FindChild("LeftUpperArm").Addr;
        l.lLowerArm = ch.FindChild("LeftLowerArm").Addr;
        l.lHand = ch.FindChild("LeftHand").Addr;
        l.rUpperArm = ch.FindChild("RightUpperArm").Addr;
        l.rLowerArm = ch.FindChild("RightLowerArm").Addr;
        l.rHand = ch.FindChild("RightHand").Addr;
        l.lUpperLeg = ch.FindChild("LeftUpperLeg").Addr;
        l.lLowerLeg = ch.FindChild("LeftLowerLeg").Addr;
        l.lFoot = ch.FindChild("LeftFoot").Addr;
        l.rUpperLeg = ch.FindChild("RightUpperLeg").Addr;
        l.rLowerLeg = ch.FindChild("RightLowerLeg").Addr;
        l.rFoot = ch.FindChild("RightFoot").Addr;
    }
    l.humanoid = ch.FindChildByClass("Humanoid").Addr;
    if (!l.hrp || !l.humanoid)
        return kEmpty; 
    auto res = limbCache.emplace(characterAddr, l);
    return res.first->second;
}

inline std::string GetWeaponFromViewModels(const std::string& playerName) {
    if (!Globals::workspace.Addr) return "None";
    auto viewModels = Globals::workspace.FindChild("ViewModels");
    if (!viewModels.Addr) return "None";
    for (auto& child : viewModels.GetChildList()) {
        const std::string className = child.GetClass();
        const std::string name = child.GetName();
        if (className == "Model" && name.rfind(playerName + " - ", 0) == 0) {
            size_t firstDash = name.find(" - ");
            if (firstDash != std::string::npos) {
                size_t secondDash = name.find(" - ", firstDash + 3);
                if (secondDash != std::string::npos)
                    return name.substr(firstDash + 3, secondDash - (firstDash + 3));
                return name.substr(firstDash + 3);
            }
        }
        if (name == "FirstPerson") {
            for (auto& fpChild : child.GetChildList()) {
                const std::string fpClass = fpChild.GetClass();
                const std::string fpName = fpChild.GetName();
                if (fpClass == "Model" && fpName.rfind(playerName + " - ", 0) == 0) {
                    size_t firstDash = fpName.find(" - ");
                    if (firstDash != std::string::npos) {
                        size_t secondDash = fpName.find(" - ", firstDash + 3);
                        if (secondDash != std::string::npos)
                            return fpName.substr(firstDash + 3, secondDash - (firstDash + 3));
                        return fpName.substr(firstDash + 3);
                    }
                }
            }
        }
    }
    return "None";
}

inline void PruneLimbs(const std::unordered_set<std::uintptr_t>& alive) {
    for (auto it = limbCache.begin(); it != limbCache.end();) {
        if (alive.find(it->first) == alive.end())
            it = limbCache.erase(it);
        else
            ++it;
    }
}

inline std::vector<CachedPlayer> players;
inline RBX::Vec3 localPlayerPos{};
inline std::uintptr_t localPlayerTeam = 0;
inline std::uintptr_t localRootPrim = 0;

inline void updateplayers() {
    if (!Globals::players.Addr || !Globals::localPlayer.Addr) {
        players.clear();
        localRootPrim = 0;
        return;
    }
    if (localRootPrim) {
        localPlayerPos = memory->read<RBX::Vec3>(localRootPrim + Offsets::Primitive::Position);
        for (auto& c : players) {
            if (!c.isValid)
                continue;
            c.health = memory->read<float>(c.humanoidAddr + Offsets::Humanoid::Health);
            if (variables::ESP::deadCheck && c.health <= 0)
                c.isValid = false;
        }
    }
    using Clock = std::chrono::steady_clock;
    static auto lastTopo = Clock::now() - std::chrono::seconds(10);
    if (Clock::now() - lastTopo < std::chrono::milliseconds(100))
        return;
    lastTopo = Clock::now();

    auto localChar = Globals::localPlayer.GetModelRef();
    if (!localChar.Addr) {
        players.clear();
        localRootPrim = 0;
        return;
    }
    auto localRoot = localChar.FindChild("HumanoidRootPart");
    if (!localRoot.Addr) {
        players.clear();
        localRootPrim = 0;
        return;
    }
    localRootPrim = localRoot.GetPrimitivePtr();
    localPlayerPos = localRoot.GetPos();
    localPlayerTeam = memory->read<std::uintptr_t>(Globals::localPlayer.Addr + Offsets::Player::Team);

    auto list = Globals::players.GetChildList();
    std::unordered_set<std::uintptr_t> alive;
    alive.reserve(list.size() + 1);
    alive.insert(localChar.Addr);

    for (auto& plr : players)
        plr.isValid = false;

    for (auto& plr : list) {
        if (plr.Addr == Globals::localPlayer.Addr)
            continue;
        const auto character = plr.GetModelRef();
        if (!character.Addr)
            continue;
        alive.insert(character.Addr);
        CachedPlayer* slot = nullptr;
        for (auto& c : players) {
            if (c.playerAddr == plr.Addr) {
                slot = &c;
                break;
            }
        }
        if (slot && slot->characterAddr == character.Addr && slot->rootPartAddr) {
            
            
            const auto rp = memory->read<std::uintptr_t>(slot->rootPartAddr + Offsets::Instance::Parent);
            if (rp == character.Addr) {
                const auto teamNow = memory->read<std::uintptr_t>(plr.Addr + Offsets::Player::Team);
                slot->teamAddr = teamNow;
                if (Keys::TeamCheckOn() && teamNow && teamNow == localPlayerTeam) {
                    slot->isValid = false;
                    continue;
                }
                slot->health = memory->read<float>(slot->humanoidAddr + Offsets::Humanoid::Health);
                if (variables::ESP::deadCheck && slot->health <= 0) {
                    slot->isValid = false;
                    continue;
                }
                {
                    int nr = ScanRole(character.Addr);
                    DbgRoleChange(plr.Addr, slot->name, slot->role, nr);
                    slot->role = nr;
                }
                
                static unsigned toolTick = 0;
                if ((toolTick++ % 20) == 0) {
                    slot->tool = "None";
                    for (auto& child : RBX::RbxInstance(character.Addr).GetChildList()) {
                        if (child.GetClass() == "Tool") {
                            slot->tool = child.GetName();
                            break;
                        }
                    }
                }
                slot->isValid = true;
                alive.insert(character.Addr);
                
                
                if (!slot->headAddr) {
                    const auto& limbs = GetLimbs(character.Addr);
                    slot->headAddr = limbs.head;
                }
                continue;
            }
        }
        const auto& limbs = GetLimbs(character.Addr);
        if (!limbs.hrp || !limbs.humanoid)
            continue;
        const float hp = memory->read<float>(limbs.humanoid + Offsets::Humanoid::Health);
        if (variables::ESP::deadCheck && hp <= 0)
            continue;
        const auto team = memory->read<std::uintptr_t>(plr.Addr + Offsets::Player::Team);
        if (Keys::TeamCheckOn() && team && team == localPlayerTeam)
            continue;
        CachedPlayer c{};
        c.playerAddr = plr.Addr;
        c.characterAddr = character.Addr;
        c.humanoidAddr = limbs.humanoid;
        c.rootPartAddr = limbs.hrp;
        c.headAddr = limbs.head;
        c.teamAddr = team;
        c.teamName = team ? RBX::RbxInstance(team).GetName() : std::string{};
        c.name = plr.GetName();
        c.health = hp;        c.maxHealth = memory->read<float>(limbs.humanoid + Offsets::Humanoid::MaxHealth);
        c.isR6 = limbs.r6;
        c.isValid = true;
        c.role = ScanRole(character.Addr);
        if (c.role != 0) DbgRoleChange(plr.Addr, c.name, 0, c.role);
        for (auto& child : RBX::RbxInstance(character.Addr).GetChildList()) {
            if (child.GetClass() == "Tool") {
                c.tool = child.GetName();
                break;
            }
        }
        if (slot)
            *slot = std::move(c);
        else
            players.push_back(std::move(c));
    }

    players.erase(std::remove_if(players.begin(), players.end(), [](const CachedPlayer& c) { return !c.isValid; }), players.end());
    PruneLimbs(alive);
}
}
