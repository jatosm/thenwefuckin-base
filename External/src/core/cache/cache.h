// discord.gg/thenwefuckin
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
#include <mutex>

namespace PlayersTab {
bool IsMarked(const std::string& n);
}

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
inline std::mutex limbMutex;

inline LimbAddrs GetLimbs(std::uintptr_t characterAddr, bool validate = true) {
    static const LimbAddrs kEmpty{};
    if (!characterAddr)
        return kEmpty;
    std::lock_guard<std::mutex> lk(limbMutex);
    auto it = limbCache.find(characterAddr);
    if (it != limbCache.end()) {

        if (!validate)
            return it->second;
        LimbAddrs& e = it->second;
        bool ok = e.hrp != 0;
        if (ok) {
            const auto hp = memory->read<std::uintptr_t>(e.hrp + Offsets::Instance::Parent);
            ok = (hp == characterAddr);
            if (ok && e.humanoid) {
                const auto hh = memory->read<std::uintptr_t>(e.humanoid + Offsets::Instance::Parent);
                ok = (hh == characterAddr);
            }
        }
        if (!ok) {
            limbCache.erase(it);
        } else {

            RBX::RbxInstance ch{characterAddr};
            if (!e.head) {
                e.head = ch.FindChild("Head").Addr;
                if (!e.head)
                    e.head = ch.FindChild("FakeHead").Addr;
            }
            if (!e.hrp)
                e.hrp = ch.FindChild("Root").Addr;
            if (!e.torso) {
                e.torso = ch.FindChild("Torso").Addr;
                if (!e.torso)
                    e.torso = ch.FindChild("LowerTorso").Addr;
                if (e.torso)
                    e.r6 = (ch.FindChild("Torso").Addr != 0);
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
    if (!head.Addr)
        head = ch.FindChild("FakeHead");
    auto hrp = ch.FindChild("HumanoidRootPart");
    if (!hrp.Addr)
        hrp = ch.FindChild("Root");
    l.head = head.Addr;
    l.hrp = hrp.Addr;
    auto torso = ch.FindChild("Torso");
    if (!torso.Addr)
        torso = ch.FindChild("LowerTorso");
    l.r6 = ch.FindChild("Torso").Addr != 0;
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
    if (!l.hrp)
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
    std::lock_guard<std::mutex> lk(limbMutex);
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
            const auto cp = memory->read<std::uintptr_t>(c.characterAddr + Offsets::Instance::Parent);
            if (!cp) {
                c.isValid = false;
                continue;
            }
            const auto rp = memory->read<std::uintptr_t>(c.rootPartAddr + Offsets::Instance::Parent);
            if (rp != c.characterAddr) {
                c.isValid = false;
                continue;
            }
            if (c.humanoidAddr) {
                const auto hpParent = memory->read<std::uintptr_t>(c.humanoidAddr + Offsets::Instance::Parent);
                if (hpParent != c.characterAddr) {
                    c.isValid = false;
                    continue;
                }
                c.health = memory->read<float>(c.humanoidAddr + Offsets::Humanoid::Health);
                if (c.health <= 0.0f) {
                    c.isValid = false;
                    continue;
                }
            }
        }
    }
    using Clock = std::chrono::steady_clock;
    players.erase(std::remove_if(players.begin(), players.end(), [](const CachedPlayer& c) { return !c.isValid; }), players.end());
    static auto lastTopo = Clock::now() - std::chrono::seconds(10);
    if (Clock::now() - lastTopo < std::chrono::milliseconds(500))
        return;
    lastTopo = Clock::now();

    const bool needRole = variables::ESP::enabled && variables::ESP::flags && variables::ESP::flagSel[7];
    const bool needTool = variables::ESP::enabled && (variables::ESP::tool || (variables::ESP::flags && variables::ESP::flagSel[3]));
    static int topoTick = 0;
    ++topoTick;
    const bool rescanMeta = (topoTick % 4) == 0;

    auto localChar = Globals::localPlayer.GetModelRef();
    if (!localChar.Addr && Globals::workspace.Addr && Globals::localPlayer.Addr) {
        localChar = Globals::workspace.FindChild(Globals::localPlayer.GetName());
        if (!localChar.Addr) {
            auto chars = Globals::workspace.FindChild("Characters");
            if (chars.Addr)
                localChar = chars.FindChild(Globals::localPlayer.GetName());
        }
    }
    auto localRoot = localChar.Addr ? localChar.FindChild("HumanoidRootPart") : RBX::RbxInstance{0};
    if (localRoot.Addr) {
        localRootPrim = localRoot.GetPrimitivePtr();
        localPlayerPos = localRoot.GetPos();
    } else {
        localRootPrim = 0;
        if (Globals::camera.Addr) {
            localPlayerPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);
        }
    }
    localPlayerTeam = Globals::localPlayer.Addr ? memory->read<std::uintptr_t>(Globals::localPlayer.Addr + Offsets::Player::Team) : 0;

    auto list = Globals::players.GetChildList();
    std::unordered_set<std::uintptr_t> alive;
    alive.reserve(list.size() + 1);
    if (localChar.Addr)
        alive.insert(localChar.Addr);

    for (auto& plr : players)
        plr.isValid = false;

    for (auto& plr : list) {
        if (plr.Addr == Globals::localPlayer.Addr)
            continue;
        const auto character = plr.GetModelRef();
        if (!character.Addr)
            continue;
        const auto charParent = memory->read<std::uintptr_t>(character.Addr + Offsets::Instance::Parent);
        if (!charParent)
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
                if (Keys::TeamCheckOn() && teamNow && teamNow == localPlayerTeam && !PlayersTab::IsMarked(slot->name)) {
                    slot->isValid = false;
                    continue;
                }
                if (slot->humanoidAddr) {
                    const auto hpParent = memory->read<std::uintptr_t>(slot->humanoidAddr + Offsets::Instance::Parent);
                    if (hpParent != character.Addr) {
                        slot->isValid = false;
                        continue;
                    }
                    slot->health = memory->read<float>(slot->humanoidAddr + Offsets::Humanoid::Health);
                    if (slot->health <= 0.0f) {
                        slot->isValid = false;
                        continue;
                    }
                }
                {
                    if (needRole && rescanMeta) {
                        int nr = ScanRole(character.Addr);
                        DbgRoleChange(plr.Addr, slot->name, slot->role, nr);
                        slot->role = nr;
                    }
                    if (rescanMeta) {
                        const auto& limbs = GetLimbs(character.Addr);
                        if (limbs.head)
                            slot->headAddr = limbs.head;
                        if (limbs.hrp)
                            slot->rootPartAddr = limbs.hrp;
                        if (limbs.humanoid)
                            slot->humanoidAddr = limbs.humanoid;
                        slot->isR6 = limbs.r6;
                    }
                    if (needTool && rescanMeta) {
                        slot->tool = "None";
                        for (auto& child : RBX::RbxInstance(character.Addr).GetChildList()) {
                            if (child.GetClass() == "Tool") {
                                slot->tool = child.GetName();
                                break;
                            }
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
        if (!limbs.hrp)
            continue;
        const float hp = limbs.humanoid ? memory->read<float>(limbs.humanoid + Offsets::Humanoid::Health) : 100.0f;
        if (limbs.humanoid && hp <= 0.0f)
            continue;
        const auto team = memory->read<std::uintptr_t>(plr.Addr + Offsets::Player::Team);
        const std::string curPlrName = plr.GetName();
        if (Keys::TeamCheckOn() && team && team == localPlayerTeam && !PlayersTab::IsMarked(curPlrName))
            continue;
        CachedPlayer c{};
        c.playerAddr = plr.Addr;
        c.characterAddr = character.Addr;
        c.humanoidAddr = limbs.humanoid;
        c.rootPartAddr = limbs.hrp;
        c.headAddr = limbs.head;
        c.teamAddr = team;
        c.teamName = team ? RBX::RbxInstance(team).GetName() : std::string{};
        c.name = curPlrName;
        c.health = hp;
        c.maxHealth = limbs.humanoid ? memory->read<float>(limbs.humanoid + Offsets::Humanoid::MaxHealth) : 100.0f;
        c.isR6 = limbs.r6;
        c.isValid = true;
        c.role = needRole ? ScanRole(character.Addr) : 0;
        if (c.role != 0) DbgRoleChange(plr.Addr, c.name, 0, c.role);
        if (needTool) {
            for (auto& child : RBX::RbxInstance(character.Addr).GetChildList()) {
                if (child.GetClass() == "Tool") {
                    c.tool = child.GetName();
                    break;
                }
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
