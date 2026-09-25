// discord.gg/thenwefuckin
#pragma once
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../../../sdk/sdk.h"
#include "../../cache/cache.h"
#include "../../cache/pf_cache.h"
#include "../../cache/cb_cache.h"
#include "../../cache/ml_cache.h"
#include "../../cache/ops_cache.h"
#include "../../globals/globals.h"
#include "../../../../ext/imgui/imgui.h"
#include "../../../render/menu/library.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

namespace PlayersTab {
struct Entry {
    std::string name;
    std::uintptr_t head = 0;
    std::uintptr_t root = 0;
    int src = 0;
};

inline std::string selected;
inline std::string target;
inline std::unordered_set<std::string> friends;

inline bool IsFriend(const std::string& n) {
    return !n.empty() && friends.find(n) != friends.end();
}

inline bool IsMarked(const std::string& n) {
    return !target.empty() && n == target;
}

inline std::vector<Entry> Gather() {
    std::vector<Entry> out;
    std::string localName;
    if (Globals::localPlayer.Addr)
        localName = Globals::localPlayer.GetName();

    auto push = [&](const std::string& name, std::uintptr_t head, std::uintptr_t root, int src) {
        if (name.empty() || (!localName.empty() && name == localName))
            return;
        for (auto& e : out) {
            if (e.name == name) {
                if (!e.head && head) e.head = head;
                if (!e.root && root) e.root = root;
                return;
            }
        }
        out.push_back(Entry{name, head, root, src});
    };

    for (auto& p : PlayerCache::players) {
        push(p.name, p.headAddr, p.rootPartAddr, 0);
    }

    for (auto& p : CbCache::players) {
        push(p.name, p.headAddr, p.rootPartAddr, 1);
    }

    for (auto& p : MlCache::players) {
        push(p.name, p.headAddr, p.rootPartAddr, 1);
    }

    for (auto& p : PfCache::players) {
        push(p.name, p.headAddr, p.torsoAddr, 2);
    }

    for (auto& p : OpsCache::players) {
        push(p.name, p.headAddr, p.rootPartAddr, 3);
    }

    if (Globals::players.Addr) {
        for (auto& plr : Globals::players.GetChildList()) {
            if (plr.Addr == Globals::localPlayer.Addr)
                continue;
            const std::string name = plr.GetName();
            if (name.empty())
                continue;
            std::uintptr_t head = 0, root = 0;
            auto ch = plr.GetModelRef();
            if (ch.Addr) {
                const auto& limbs = PlayerCache::GetLimbs(ch.Addr, false);
                head = limbs.head;
                root = limbs.hrp;
            }
            push(name, head, root, 0);
        }
    }
    return out;
}

inline const Entry* Find(const std::vector<Entry>& list, const std::string& name) {
    for (auto& e : list) {
        if (e.name == name)
            return &e;
    }
    return nullptr;
}

inline RBX::Vec3 PartPosOf(std::uintptr_t partAddr) {
    if (!partAddr)
        return {};
    const auto prim = memory->read<std::uintptr_t>(partAddr + Offsets::BasePart::Primitive);
    if (!prim)
        return {};
    return memory->read<RBX::Vec3>(prim + Offsets::Primitive::Position);
}

inline std::uintptr_t LocalRootPrim() {
    if (PlayerCache::localRootPrim)
        return PlayerCache::localRootPrim;
    if (Globals::localPlayer.Addr) {
        auto ch = Globals::localPlayer.GetModelRef();
        if (!ch.Addr && Globals::workspace.Addr) {
            ch = Globals::workspace.FindChild(Globals::localPlayer.GetName());
        }
        if (!ch.Addr && CbCache::charactersAddr) {
            ch = RBX::RbxInstance(CbCache::charactersAddr).FindChild(Globals::localPlayer.GetName());
        }
        if (ch.Addr) {
            auto hrp = ch.FindChild("HumanoidRootPart");
            if (hrp.Addr)
                return memory->read<std::uintptr_t>(hrp.Addr + Offsets::BasePart::Primitive);
        }
    }
    return 0;
}

inline RBX::Vec3 ResolveTargetPos(const Entry& e) {
    RBX::Vec3 p = PartPosOf(e.root ? e.root : e.head);
    if (p.X != 0 || p.Y != 0 || p.Z != 0)
        return p;

    for (auto& cp : PlayerCache::players) {
        if (cp.name == e.name) {
            p = PartPosOf(cp.rootPartAddr ? cp.rootPartAddr : cp.headAddr);
            if (p.X != 0 || p.Y != 0 || p.Z != 0)
                return p;
        }
    }

    for (auto& cp : CbCache::players) {
        if (cp.name == e.name) {
            p = PartPosOf(cp.rootPartAddr ? cp.rootPartAddr : cp.headAddr);
            if (p.X != 0 || p.Y != 0 || p.Z != 0)
                return p;
        }
    }

    for (auto& cp : MlCache::players) {
        if (cp.name == e.name) {
            p = PartPosOf(cp.rootPartAddr ? cp.rootPartAddr : cp.headAddr);
            if (p.X != 0 || p.Y != 0 || p.Z != 0)
                return p;
        }
    }

    if (Globals::players.Addr) {
        auto plr = Globals::players.FindChild(e.name);
        if (plr.Addr) {
            auto ch = plr.GetModelRef();
            if (!ch.Addr && CbCache::charactersAddr) {
                ch = RBX::RbxInstance(CbCache::charactersAddr).FindChild(e.name);
            }
            if (ch.Addr) {
                auto hrp = ch.FindChild("HumanoidRootPart");
                if (hrp.Addr) {
                    p = PartPosOf(hrp.Addr);
                    if (p.X != 0 || p.Y != 0 || p.Z != 0)
                        return p;
                }
                auto head = ch.FindChild("Head");
                if (head.Addr)
                    return PartPosOf(head.Addr);
            }
        }
    }
    return {};
}

inline void TeleportTo(const RBX::Vec3& dst) {
    const auto prim = LocalRootPrim();
    if (!prim)
        return;
    if (dst.X == 0 && dst.Y == 0 && dst.Z == 0)
        return;
    RBX::Vec3 safeDst = dst;
    safeDst.Y += 3.5f;
    memory->write<RBX::Vec3>(prim + Offsets::Primitive::Position, safeDst);
    const RBX::Vec3 zero{};
    memory->write<RBX::Vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity, zero);
}

inline void RenderMenu() {
    const ImVec2 base = ImGui::GetWindowPos();
    const auto& theme = imGuiCustom::GetTheme();
    const auto& fonts = imGuiCustom::GetFonts();
    ImFont* font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
    const float font_size = 12.0f * imGuiCustom::g_fontScale;
    auto entries = Gather();

    if (selected.empty() && !entries.empty())
        selected = entries.front().name;

    const ImVec2 listPos = base + ImVec2(12.0f + imGuiCustom::g_contentOffset.x, 46.0f);
    const ImVec2 listSize(278.0f, 326.0f);
    ImGui::SetCursorScreenPos(listPos);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, imGuiCustom::ColorU32(theme.CardBg));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));
    ImGui::BeginChild("plist", listSize, false, ImGuiWindowFlags_NoScrollbar);

    for (size_t i = 0; i < entries.size(); ++i) {
        auto& e = entries[i];
        const bool isSel = (e.name == selected);
        const bool isTgt = IsMarked(e.name);
        const bool isFrnd = IsFriend(e.name);

        ImGui::PushID(static_cast<int>(i));
        const ImVec2 rowMin = ImGui::GetCursorScreenPos();
        const ImVec2 rowSize(listSize.x - 8.0f, 20.0f);

        const bool rowPressed = ImGui::InvisibleButton("##row", rowSize);
        const bool rowHovered = ImGui::IsItemHovered();

        if (rowPressed)
            selected = e.name;

        ImDrawList* dl = ImGui::GetWindowDrawList();

        if (isSel) {
            dl->AddRectFilled(rowMin, rowMin + rowSize, imGuiCustom::ColorU32(theme.ControlInactive), 0.0f);
            dl->AddRect(rowMin, rowMin + rowSize, imGuiCustom::ColorU32(theme.Accent), 0.0f, 0, 1.0f);
        } else if (rowHovered) {
            dl->AddRectFilled(rowMin, rowMin + rowSize, imGuiCustom::ColorU32(theme.ControlBg), 0.0f);
        }

        float textOffsetX = rowMin.x + 6.0f;
        if (isTgt) {
            dl->AddText(font, font_size, ImVec2(textOffsetX, rowMin.y + 3.0f), imGuiCustom::ColorU32(theme.Accent), "[T]");
            textOffsetX += font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, "[T] ").x;
        }
        if (isFrnd) {
            dl->AddText(font, font_size, ImVec2(textOffsetX, rowMin.y + 3.0f), IM_COL32(100, 220, 120, 255), "[F]");
            textOffsetX += font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, "[F] ").x;
        }

        ImVec4 nameCol = isSel ? theme.TextBright : (rowHovered ? theme.TextBright : theme.Text);
        dl->AddText(font, font_size, ImVec2(textOffsetX, rowMin.y + 3.0f), imGuiCustom::ColorU32(nameCol), e.name.c_str());

        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    const ImVec2 actPos = base + ImVec2(311.0f + imGuiCustom::g_contentOffset.x, 46.0f);
    const ImVec2 actSize(278.0f, 326.0f);
    ImGui::SetCursorScreenPos(actPos);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, imGuiCustom::ColorU32(theme.CardBg));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 5.0f));
    ImGui::BeginChild("pact", actSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    if (selected.empty()) {
        ImVec2 textPos = ImGui::GetCursorScreenPos() + ImVec2(4.0f, 4.0f);
        dl->AddText(font, font_size, textPos, imGuiCustom::ColorU32(theme.Text), "Select a player from the list.");
    } else {
        const Entry* e = Find(entries, selected);
        const bool isMarkedSel = IsMarked(selected);
        const bool isFriendSel = IsFriend(selected);

        ImVec2 curPos = ImGui::GetCursorScreenPos();
        dl->AddText(font, font_size + 1.0f, curPos + ImVec2(2.0f, 2.0f), imGuiCustom::ColorU32(theme.TextBright), selected.c_str());

        if (isMarkedSel || isFriendSel) {
            std::string tags;
            if (isMarkedSel) tags += "[TARGET LOCKED] ";
            if (isFriendSel) tags += "[FRIEND]";
            ImVec2 tagPos = curPos + ImVec2(2.0f, 18.0f);
            dl->AddText(font, font_size - 1.0f, tagPos, isMarkedSel ? imGuiCustom::ColorU32(theme.Accent) : IM_COL32(100, 220, 120, 255), tags.c_str());
            ImGui::Dummy(ImVec2(actSize.x - 16.0f, 30.0f));
        } else {
            ImGui::Dummy(ImVec2(actSize.x - 16.0f, 18.0f));
        }

        ImVec2 sepPos = ImGui::GetCursorScreenPos();
        dl->AddLine(sepPos, sepPos + ImVec2(actSize.x - 16.0f, 0.0f), imGuiCustom::OutlineInner(), 1.0f);
        ImGui::Dummy(ImVec2(actSize.x - 16.0f, 4.0f));

        const PlayerCache::CachedPlayer* cp = nullptr;
        int gameSource = 0;
        for (auto& p : PlayerCache::players) {
            if (p.name == selected) {
                cp = &p;
                gameSource = 0;
                break;
            }
        }
        if (!cp) {
            for (auto& p : CbCache::players) {
                if (p.name == selected) {
                    cp = &p;
                    gameSource = 1;
                    break;
                }
            }
        }
        if (!cp) {
            for (auto& p : MlCache::players) {
                if (p.name == selected) {
                    cp = &p;
                    gameSource = 1;
                    break;
                }
            }
        }

        auto drawInfoRow = [&](const char* label, const char* val, ImU32 valCol = 0) {
            ImVec2 p = ImGui::GetCursorScreenPos();
            dl->AddText(font, font_size, p + ImVec2(2.0f, 0.0f), imGuiCustom::ColorU32(theme.Text), label);
            const ImVec2 valSz = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, val);
            const float rightX = p.x + (actSize.x - 18.0f) - valSz.x;
            dl->AddText(font, font_size, ImVec2(rightX, p.y), valCol ? valCol : imGuiCustom::ColorU32(theme.TextBright), val);
            ImGui::Dummy(ImVec2(actSize.x - 16.0f, 16.0f));
        };

        if (cp && cp->isValid) {
            drawInfoRow("Status", "Alive", IM_COL32(100, 220, 120, 255));
            if (cp->maxHealth > 0.0f) {
                char hBuf[32];
                snprintf(hBuf, sizeof(hBuf), "%.0f / %.0f", cp->health, cp->maxHealth);
                drawInfoRow("Health", hBuf);
            }
            RBX::Vec3 myPos = (gameSource == 1) ? CbCache::localPos : PlayerCache::localPlayerPos;
            RBX::Vec3 theirPos = cp->position;
            if (theirPos.X == 0 && theirPos.Y == 0 && theirPos.Z == 0) {
                theirPos = PartPosOf(cp->rootPartAddr ? cp->rootPartAddr : cp->headAddr);
            }
            const float dist = sqrtf(
                (theirPos.X - myPos.X) * (theirPos.X - myPos.X) +
                (theirPos.Y - myPos.Y) * (theirPos.Y - myPos.Y) +
                (theirPos.Z - myPos.Z) * (theirPos.Z - myPos.Z)
            );
            char dBuf[32];
            snprintf(dBuf, sizeof(dBuf), "%.0f studs", dist);
            drawInfoRow("Distance", dBuf);

            if (!cp->teamName.empty())
                drawInfoRow("Team", cp->teamName.c_str());
            if (cp->tool != "None" && !cp->tool.empty())
                drawInfoRow("Weapon", cp->tool.c_str());
        } else {
            drawInfoRow("Status", "In Server", imGuiCustom::ColorU32(theme.Text));
        }

        ImVec2 sep2 = ImGui::GetCursorScreenPos();
        dl->AddLine(sep2, sep2 + ImVec2(actSize.x - 16.0f, 0.0f), imGuiCustom::OutlineInner(), 1.0f);
        ImGui::Dummy(ImVec2(actSize.x - 16.0f, 4.0f));

        const float btnW = std::floor((actSize.x - 16.0f - 6.0f) * 0.5f);
        const float btnH = 20.0f;

        if (imGuiCustom::Button(isMarkedSel ? "Untarget##tgt" : "Target##tgt", ImVec2(btnW, btnH), isMarkedSel)) {
            if (isMarkedSel)
                target.clear();
            else
                target = selected;
        }
        ImGui::SameLine(0.0f, 6.0f);
        if (imGuiCustom::Button(isFriendSel ? "Unfriend##frnd" : "Friend##frnd", ImVec2(btnW, btnH), isFriendSel)) {
            if (isFriendSel)
                friends.erase(selected);
            else
                friends.insert(selected);
        }

        ImGui::Dummy(ImVec2(actSize.x - 16.0f, 2.0f));

        if (imGuiCustom::Button("Teleport to Player##tp", ImVec2(actSize.x - 16.0f, btnH))) {
            if (e)
                TeleportTo(ResolveTargetPos(*e));
            else {
                Entry temp{selected, 0, 0, 0};
                TeleportTo(ResolveTargetPos(temp));
            }
        }
    }

    if (!target.empty()) {
        ImVec2 sep3 = ImGui::GetCursorScreenPos();
        dl->AddLine(sep3, sep3 + ImVec2(actSize.x - 16.0f, 0.0f), imGuiCustom::OutlineInner(), 1.0f);
        ImGui::Dummy(ImVec2(actSize.x - 16.0f, 4.0f));

        char tgtBuf[64];
        snprintf(tgtBuf, sizeof(tgtBuf), "Active Target: %s", target.c_str());
        ImVec2 tp = ImGui::GetCursorScreenPos();
        dl->AddText(font, font_size, tp + ImVec2(2.0f, 0.0f), imGuiCustom::ColorU32(theme.Accent), tgtBuf);
        ImGui::Dummy(ImVec2(actSize.x - 16.0f, 16.0f));

        if (imGuiCustom::Button("Clear Target##clr_tgt", ImVec2(actSize.x - 16.0f, 20.0f))) {
            target.clear();
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}
}
