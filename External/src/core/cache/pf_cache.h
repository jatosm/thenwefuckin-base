#pragma once
#include "../../../src/sdk/sdk.h"
#include "../globals/globals.h"
#include "../variables/variables.h"
#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <chrono>

namespace PfCache {
inline constexpr std::uint64_t kPlaceId = 292439477ull;

struct PfPlayer {
    std::uintptr_t modelAddr = 0;
    std::uintptr_t teamFolderAddr = 0;
    std::uintptr_t headAddr = 0;
    std::uintptr_t torsoAddr = 0;
    std::uintptr_t labelAddr = 0;
    std::uintptr_t healthBarAddr = 0;
    std::vector<std::uintptr_t> limbAddrs;
    std::string name;
    float health = 1.0f;
    bool isEnemy = true;
    bool colorValid = false;
    bool isValid = false;
};

inline std::vector<PfPlayer> players;
inline std::uintptr_t localCharacterAddr = 0;
inline std::uintptr_t localTeamFolderAddr = 0;
inline std::uintptr_t workspacePlayersAddr = 0;
inline RBX::Vec3 localPos{};

struct PfDebug {
    std::uint64_t placeIdRead = 0;
    int wsChildren = -1;
    int teamFolders = 0;
    int modelsScanned = 0;
    int modelsKept = 0;
    int modelsNoHeadTorso = 0;
    int modelsEmptyName = 0;
    int hpFrames = 0;
    int hpZero = 0;
    int hpAllZeroFix = 0;
    int enemies = 0;
    float hpSample = -1.0f;
    const char* failStage = "not run";
};
inline PfDebug dbg;
inline std::chrono::steady_clock::time_point lastFull{};

inline std::uint64_t ReadPlaceId() {
    if (!Globals::dataModel.Addr)
        return 0;
    return memory->read<std::uint64_t>(Globals::dataModel.Addr + Offsets::DataModel::PlaceId);
}

inline bool IsActivePlace() {
    return ReadPlaceId() == kPlaceId;
}

inline bool LooksLikePf(std::uintptr_t wsPlayers) {
    if (!wsPlayers)
        return false;
    RBX::RbxInstance ws{wsPlayers};
    for (auto& folder : ws.GetChildList()) {
        if (folder.GetClass() != "Folder")
            continue;
        for (auto& model : folder.GetChildList()) {
            if (model.GetClass() != "Model")
                continue;
            for (auto& part : model.GetChildList()) {
                const std::string pc = part.GetClass();
                if (pc != "Part" && pc != "MeshPart")
                    continue;
                for (auto& c : part.GetChildList()) {
                    if (c.GetClass() == "BillboardGui")
                        return true;
                }
            }
        }
    }
    return false;
}

inline std::uintptr_t FindWorkspacePlayersFolder() {
    if (!Globals::workspace.Addr)
        return 0;
    for (auto& c : Globals::workspace.GetChildList()) {
        if (c.GetClass() == "Folder" && c.GetName() == "Players")
            return c.Addr;
    }
    return 0;
}

inline std::string ReadLabelText(std::uintptr_t labelAddr) {
    if (!labelAddr)
        return {};
    return memory->read_string(labelAddr + Offsets::TextLabel::Text);
}

inline bool ReadLabelColor(std::uintptr_t labelAddr, rbx::color3_t& out) {
    if (!labelAddr)
        return false;
    out = memory->read<rbx::color3_t>(labelAddr + Offsets::TextLabel::TextColor3);
    if (!std::isfinite(out.r) || !std::isfinite(out.g) || !std::isfinite(out.b))
        return false;
    if (out.r < 0.0f || out.r > 1.0f || out.g < 0.0f || out.g > 1.0f || out.b < 0.0f || out.b > 1.0f)
        return false;
    return true;
}

inline bool IsEnemyLabelColor(const rbx::color3_t& c) {
    const int r = (int)(c.r * 255.0f + 0.5f);
    const int g = (int)(c.g * 255.0f + 0.5f);
    const int b = (int)(c.b * 255.0f + 0.5f);
    return std::abs(r - 255) <= 8 && std::abs(g - 10) <= 12 && std::abs(b - 20) <= 12;
}

inline std::uintptr_t FindBillboardLabel(std::uintptr_t modelAddr) {
    RBX::RbxInstance model{modelAddr};
    for (auto& part : model.GetChildList()) {
        const std::string pc = part.GetClass();
        if (pc != "Part" && pc != "MeshPart")
            continue;
        for (auto& c : part.GetChildList()) {
            if (c.GetClass() != "BillboardGui")
                continue;
            for (auto& bc : c.GetChildList()) {
                if (bc.GetClass() == "TextLabel")
                    return bc.Addr;
            }
        }
    }
    return 0;
}

inline std::string ReadModelName(std::uintptr_t modelAddr) {
    const auto label = FindBillboardLabel(modelAddr);
    if (label)
        return ReadLabelText(label);
    return {};
}

inline std::uintptr_t FindHealthPercent(std::uintptr_t modelAddr) {
    RBX::RbxInstance model{modelAddr};
    for (auto& part : model.GetChildList()) {
        const std::string pc = part.GetClass();
        if (pc != "Part" && pc != "MeshPart")
            continue;
        for (auto& c : part.GetChildList()) {
            if (c.GetClass() != "BillboardGui")
                continue;
            for (auto& bc : c.GetChildList()) {
                if (bc.GetClass() != "TextLabel")
                    continue;
                for (auto& f : bc.GetChildList()) {
                    if (f.GetClass() != "Frame" || f.GetName() != "Health")
                        continue;
                    for (auto& pf : f.GetChildList()) {
                        if (pf.GetClass() == "Frame" && pf.GetName() == "Percent")
                            return pf.Addr;
                    }
                }
            }
        }
    }
    return 0;
}

inline float ReadHealthFrac(std::uintptr_t percentAddr) {
    if (!percentAddr)
        return 1.0f;
    const auto sz = memory->read<rbx::udim2_t>(percentAddr + Offsets::GuiObject::Size);
    float f = sz.x.scale;
    if (!std::isfinite(f) || f < 0.0f || f > 1.0f)
        return 1.0f;
    return f;
}

inline std::vector<std::uintptr_t> CollectLimbs(std::uintptr_t modelAddr, std::uintptr_t head, std::uintptr_t torso) {
    std::vector<std::uintptr_t> out;
    RBX::RbxInstance model{modelAddr};
    for (auto& part : model.GetChildList()) {
        const std::string pc = part.GetClass();
        if (pc != "Part" && pc != "MeshPart")
            continue;
        if (part.Addr == head || part.Addr == torso)
            continue;
        out.push_back(part.Addr);
    }
    return out;
}

inline void ResolveCharacter(std::uintptr_t modelAddr, std::uintptr_t& outHead, std::uintptr_t& outTorso) {
    outHead = 0;
    outTorso = 0;
    RBX::RbxInstance model{modelAddr};
    for (auto& part : model.GetChildList()) {
        const std::string pc = part.GetClass();
        if (pc != "Part" && pc != "MeshPart")
            continue;
        bool isHead = false;
        bool isTorso = false;
        for (auto& c : part.GetChildList()) {
            const std::string cc = c.GetClass();
            if (cc == "BillboardGui") {
                isHead = true;
                break;
            }
            if (cc == "SpotLight") {
                isTorso = true;
                break;
            }
        }
        if (isHead && !outHead)
            outHead = part.Addr;
        else if (isTorso && !outTorso)
            outTorso = part.Addr;
    }
}

inline std::uintptr_t ResolveLocalCharacter(std::uintptr_t wsPlayers) {
    if (Globals::localPlayer.Addr) {
        const auto mi = memory->read<std::uintptr_t>(Globals::localPlayer.Addr + Offsets::Player::ModelInstance);
        if (mi) {
            RBX::RbxInstance parent{memory->read<std::uintptr_t>(mi + Offsets::Instance::Parent)};
            if (parent.Addr && parent.GetClass() == "Folder") {
                RBX::RbxInstance grand{memory->read<std::uintptr_t>(parent.Addr + Offsets::Instance::Parent)};
                if (grand.Addr == wsPlayers)
                    return mi;
            }
            return mi;
        }
    }
    return 0;
}

inline int ScoreFolderEnemy(std::uintptr_t folderAddr) {
    int enemy = 0;
    int friendly = 0;
    RBX::RbxInstance folder{folderAddr};
    for (auto& model : folder.GetChildList()) {
        if (model.GetClass() != "Model")
            continue;
        const auto label = FindBillboardLabel(model.Addr);
        rbx::color3_t col{};
        if (!ReadLabelColor(label, col))
            continue;
        if (IsEnemyLabelColor(col))
            ++enemy;
        else
            ++friendly;
    }
    return enemy - friendly;
}

inline bool IsTeammate(const PfPlayer& p) {
    if (p.colorValid)
        return !p.isEnemy;
    if (localTeamFolderAddr && p.teamFolderAddr)
        return p.teamFolderAddr == localTeamFolderAddr;
    return false;
}

inline void RefreshFast() {
    if (Globals::camera.Addr)
        localPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);
    int zeros = 0;
    for (auto& p : players) {
        if (!p.isValid)
            continue;
        if (p.healthBarAddr)
            p.health = ReadHealthFrac(p.healthBarAddr);
        if (p.health <= 0.0f)
            ++zeros;
    }
    if (!players.empty() && zeros == (int)players.size()) {
        for (auto& p : players)
            p.health = 1.0f;
        dbg.hpAllZeroFix = 1;
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
    localCharacterAddr = 0;
    localTeamFolderAddr = 0;
    workspacePlayersAddr = 0;
    dbg = PfDebug{};

    dbg.placeIdRead = ReadPlaceId();
    if (!Globals::workspace.Addr) {
        dbg.failStage = "no workspace";
        return;
    }

    const auto wsPlayers = FindWorkspacePlayersFolder();
    if (!wsPlayers) {
        dbg.failStage = "no Workspace.Players folder";
        return;
    }
    workspacePlayersAddr = wsPlayers;

    if (dbg.placeIdRead != kPlaceId && !LooksLikePf(wsPlayers)) {
        dbg.failStage = "placeid mismatch + no pf structure";
        return;
    }

    std::string localName;
    if (Globals::localPlayer.Addr)
        localName = Globals::localPlayer.GetName();
    localCharacterAddr = ResolveLocalCharacter(wsPlayers);

    dbg.wsChildren = (int)RBX::RbxInstance(wsPlayers).GetChildList().size();

    std::uintptr_t folders[2]{};
    int nf = 0;
    RBX::RbxInstance ws{wsPlayers};
    for (auto& folder : ws.GetChildList()) {
        if (folder.GetClass() != "Folder")
            continue;
        if (nf < 2)
            folders[nf++] = folder.Addr;
        dbg.teamFolders++;
    }
    if (nf == 2) {
        const int s0 = ScoreFolderEnemy(folders[0]);
        const int s1 = ScoreFolderEnemy(folders[1]);
        if (s0 > 0 && s1 <= 0)
            localTeamFolderAddr = folders[1];
        else if (s1 > 0 && s0 <= 0)
            localTeamFolderAddr = folders[0];
        else if (s0 < 0 && s1 >= 0)
            localTeamFolderAddr = folders[0];
        else if (s1 < 0 && s0 >= 0)
            localTeamFolderAddr = folders[1];
    }
    if (!localTeamFolderAddr && localCharacterAddr) {
        RBX::RbxInstance parent{memory->read<std::uintptr_t>(localCharacterAddr + Offsets::Instance::Parent)};
        RBX::RbxInstance grand{parent.Addr ? memory->read<std::uintptr_t>(parent.Addr + Offsets::Instance::Parent) : 0};
        if (grand.Addr == wsPlayers)
            localTeamFolderAddr = parent.Addr;
    }

    for (auto& folder : ws.GetChildList()) {
        if (folder.GetClass() != "Folder")
            continue;
        for (auto& model : folder.GetChildList()) {
            if (model.GetClass() != "Model")
                continue;
            dbg.modelsScanned++;
            if (model.Addr == localCharacterAddr)
                continue;

            std::uintptr_t head = 0, torso = 0;
            ResolveCharacter(model.Addr, head, torso);
            if (!head && !torso) {
                dbg.modelsNoHeadTorso++;
                continue;
            }

            const auto label = FindBillboardLabel(model.Addr);
            rbx::color3_t col{};
            const bool colOk = ReadLabelColor(label, col);
            const auto percent = FindHealthPercent(model.Addr);
            const float hp = ReadHealthFrac(percent);
            if (percent)
                dbg.hpFrames++;
            if (hp <= 0.0f)
                dbg.hpZero++;
            if (dbg.hpSample < 0.0f && percent)
                dbg.hpSample = hp;
            if (variables::ESP::deadCheck && percent && hp <= 0.0f)
                continue;

            PfPlayer p{};
            p.modelAddr = model.Addr;
            p.teamFolderAddr = folder.Addr;
            p.headAddr = head;
            p.torsoAddr = torso;
            p.labelAddr = label;
            p.healthBarAddr = percent;
            p.limbAddrs = CollectLimbs(model.Addr, head, torso);
            p.health = hp;
            p.colorValid = colOk;
            p.isEnemy = colOk ? IsEnemyLabelColor(col) : true;
            if (p.isEnemy)
                dbg.enemies++;
            p.name = ReadModelName(model.Addr);
            if (p.name.empty())
                p.name = model.GetName();
            if (p.name.empty()) {
                dbg.modelsEmptyName++;
                char buf[32];
                std::snprintf(buf, sizeof(buf), "pf_%llx", (unsigned long long)model.Addr);
                p.name = buf;
            }
            if (!localName.empty() && p.name == localName)
                continue;
            p.isValid = true;
            players.push_back(std::move(p));
            dbg.modelsKept++;
        }
    }

    int zeros = 0;
    for (auto& p : players) {
        if (p.health <= 0.0f)
            ++zeros;
    }
    if (!players.empty() && zeros == (int)players.size()) {
        for (auto& p : players)
            p.health = 1.0f;
        dbg.hpAllZeroFix = 1;
    }

    dbg.failStage = players.empty() ? "ok but empty" : "ok";

    if (Globals::camera.Addr)
        localPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);
}

inline void PrintDebug() {}
}
