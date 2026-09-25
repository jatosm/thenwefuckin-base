// discord.gg/thenwefuckin
#include "worldcache.h"
#include "cache.h"
#include "../../memory/memory.h"
#include <iostream>
#include <queue>
#include <unordered_set>
#include <algorithm>

namespace WorldCache {

static RBX::Vec3 GetInstancePos(const RBX::RbxInstance& inst) {

    auto prim = inst.GetPrimitivePtr();
    if (prim) {
        auto p = memory->read<RBX::Vec3>(prim + Offsets::Primitive::Position);
        if (p.X!=0 || p.Y!=0 || p.Z!=0) return p;
    }

    auto ppAddr = memory->read<std::uintptr_t>(inst.Addr + Offsets::Model::PrimaryPart);
    if (ppAddr) {
        auto ppPrim = memory->read<std::uintptr_t>(ppAddr + Offsets::BasePart::Primitive);
        if (ppPrim) {
            auto p = memory->read<RBX::Vec3>(ppPrim + Offsets::Primitive::Position);
            if (p.X!=0||p.Y!=0||p.Z!=0) return p;
        }
    }

    for (auto &c : inst.GetChildList()) {
        auto cp = c.GetPrimitivePtr();
        if (cp) {
            auto p = memory->read<RBX::Vec3>(cp + Offsets::Primitive::Position);
            if (p.X!=0||p.Y!=0||p.Z!=0) return p;
        }

        for (auto &cc : c.GetChildList()) {
            auto ccp = cc.GetPrimitivePtr();
            if (ccp) {
                auto p = memory->read<RBX::Vec3>(ccp + Offsets::Primitive::Position);
                if (p.X!=0||p.Y!=0||p.Z!=0) return p;
            }
        }
    }
    return {};
}

static RBX::Vec3 PartPosOf(std::uintptr_t partAddr) {
    if (!partAddr)
        return {};
    const auto prim = memory->read<std::uintptr_t>(partAddr + Offsets::BasePart::Primitive);
    if (!prim)
        return {};
    return memory->read<RBX::Vec3>(prim + Offsets::Primitive::Position);
}

void UpdateOnce() {
    if (!variables::World::enabled) {
        std::lock_guard<std::mutex> lk(mtx);
        if (!entries.empty()) {
            entries.clear();
            lastCount = 0;
            (void)0;
        }
        return;
    }
    if (!Globals::workspace.Addr || !memory->IsConnected()) return;

    auto now = std::chrono::steady_clock::now();
    if (now - lastScan < std::chrono::milliseconds(50)) return;
    lastScan = now;

    {
        std::lock_guard<std::mutex> lk(mtx);
        if (!entries.empty()) {
            RBX::Vec3 localPos{};
            if (PlayerCache::localRootPrim)
                localPos = memory->read<RBX::Vec3>(PlayerCache::localRootPrim + Offsets::Primitive::Position);
            else if (Globals::camera.Addr)
                localPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);
            for (auto it = entries.begin(); it != entries.end();) {
                Entry& e = *it;
                if ((e.category != "soldier" && e.category != "animal" && e.category != "npc") || !e.instAddr) {
                    ++it;
                    continue;
                }
                const auto parent = memory->read<std::uintptr_t>(e.instAddr + Offsets::Instance::Parent);
                const RBX::Vec3 root = PartPosOf(e.rootPartAddr);
                const RBX::Vec3 head = PartPosOf(e.headPartAddr);
                const bool rootZero = (root.X == 0 && root.Y == 0 && root.Z == 0);
                const bool headZero = (head.X == 0 && head.Y == 0 && head.Z == 0);
                if (!parent || (rootZero && headZero)) {
                    if (++e.zeroStreak >= 5) {
                        it = entries.erase(it);
                        continue;
                    }
                    ++it;
                    continue;
                }
                e.zeroStreak = 0;
                if (!rootZero) {
                    e.rootPos = root;
                    e.pos = root;
                }
                if (!headZero)
                    e.headPos = head;
                if (e.humanoidAddr) {
                    const float hp = memory->read<float>(e.humanoidAddr + Offsets::Humanoid::Health);
                    if (!std::isfinite(hp) || hp <= 0.0f) {
                        it = entries.erase(it);
                        continue;
                    }
                    e.health = hp;
                    const float mhp = memory->read<float>(e.humanoidAddr + Offsets::Humanoid::MaxHealth);
                    if (std::isfinite(mhp) && mhp > 0.0f)
                        e.maxHealth = mhp;
                    if (e.rootPartAddr) {
                        const auto prim = memory->read<std::uintptr_t>(e.rootPartAddr + Offsets::BasePart::Primitive);
                        if (prim)
                            e.vel = memory->read<RBX::Vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity);
                    }
                }
                const float dx = e.pos.X - localPos.X, dy = e.pos.Y - localPos.Y, dz = e.pos.Z - localPos.Z;
                e.dist = sqrtf(dx * dx + dy * dy + dz * dz);
                ++it;
            }
            lastCount = (int)entries.size();
        }
    }

    if (now - lastFullScan < std::chrono::milliseconds(2000)) return;
    lastFullScan = now;

    bool needPlants = variables::World::plants;
    bool needOres = variables::World::ores;
    bool needAnimals = variables::World::animals;
    bool needSoldiers = variables::World::soldiers;
    bool needTools = variables::World::tools;
    bool needCrates = variables::World::crates;
    bool needDrops = variables::World::drops;
    bool needKeycards = variables::World::keycards;
    if (!needPlants && !needOres && !needAnimals && !needSoldiers && !needTools &&
        !needCrates && !needDrops && !needKeycards) {
        std::lock_guard<std::mutex> lk(mtx);
        if (!entries.empty()) entries.clear();
        return;
    }

    std::vector<Entry> newEntries;
    newEntries.reserve(256);

    RBX::Vec3 localPos{};
    if (PlayerCache::localRootPrim)
        localPos = memory->read<RBX::Vec3>(PlayerCache::localRootPrim + Offsets::Primitive::Position);
    else if (PlayerCache::players.size() > 0)
        localPos = PlayerCache::localPlayerPos;
    else if (Globals::camera.Addr)
        localPos = memory->read<RBX::Vec3>(Globals::camera.Addr + Offsets::Camera::Position);

    if (needSoldiers) {
        uintptr_t deltaContainerAddr = 0;

        if (Globals::localPlayer.Addr) {
            auto localChar = Globals::localPlayer.GetModelRef();
            if (localChar.Addr) {
                uintptr_t p = memory->read<uintptr_t>(localChar.Addr + Offsets::Instance::Parent);
                if (p) {
                    RBX::RbxInstance parentInst{p};
                    if (parentInst.FindChild("NPCs").Addr || parentInst.FindChild("thermalTemplate").Addr)
                        deltaContainerAddr = p;
                }
            }
        }

        if (!deltaContainerAddr && Globals::players.Addr) {
            for (auto& plr : Globals::players.GetChildList()) {
                auto c = plr.GetModelRef();
                if (c.Addr) {
                    uintptr_t p = memory->read<uintptr_t>(c.Addr + Offsets::Instance::Parent);
                    if (p) {
                        RBX::RbxInstance parentInst{p};
                        if (parentInst.FindChild("NPCs").Addr || parentInst.FindChild("thermalTemplate").Addr) {
                            deltaContainerAddr = p;
                            break;
                        }
                    }
                }
            }
        }

        if (!deltaContainerAddr && Globals::workspace.Addr) {
            for (auto& c : Globals::workspace.GetChildList()) {
                if (c.GetClass() == "Model") {
                    if (c.FindChild("NPCs").Addr || c.FindChild("thermalTemplate").Addr) {
                        deltaContainerAddr = c.Addr;
                        break;
                    }
                }
            }
        }

        if (!deltaContainerAddr && Globals::workspace.Addr) {
            auto npcsFolder = Globals::workspace.FindChild("NPCs");
            if (npcsFolder.Addr)
                deltaContainerAddr = npcsFolder.Addr;
        }

        if (deltaContainerAddr) {
            RBX::RbxInstance container{deltaContainerAddr};

            std::unordered_set<std::string> playerNames;
            std::unordered_set<uintptr_t> playerCharAddrs;
            if (Globals::localPlayer.Addr) {
                playerNames.insert(Globals::localPlayer.GetName());
                auto lc = Globals::localPlayer.GetModelRef();
                if (lc.Addr) playerCharAddrs.insert(lc.Addr);
            }
            if (Globals::players.Addr) {
                for (auto& plr : Globals::players.GetChildList()) {
                    playerNames.insert(plr.GetName());
                    auto c = plr.GetModelRef();
                    if (c.Addr) playerCharAddrs.insert(c.Addr);
                }
            }

            for (auto& c : container.GetChildList()) {
                if (c.GetClass() != "Model")
                    continue;

                std::string cname = c.GetName();
                if (cname.empty() || cname == "NPCs" || cname == "thermalTemplate")
                    continue;

                if (playerCharAddrs.count(c.Addr) || playerNames.count(cname))
                    continue;

                auto hum = c.FindChildByClass("Humanoid");
                if (!hum.Addr)
                    continue;

                float hp = memory->read<float>(hum.Addr + Offsets::Humanoid::Health);
                float maxHp = memory->read<float>(hum.Addr + Offsets::Humanoid::MaxHealth);
                if (hp <= 0.0f)
                    continue;

                auto head = c.FindChild("Head");
                auto hrp = c.FindChild("HumanoidRootPart");
                RBX::Vec3 headPos = head.Addr ? head.GetPos() : RBX::Vec3{};
                RBX::Vec3 rootPos = hrp.Addr ? hrp.GetPos() : RBX::Vec3{};
                RBX::Vec3 pos = (rootPos.X != 0 || rootPos.Y != 0 || rootPos.Z != 0) ? rootPos :
                                (headPos.X != 0 || headPos.Y != 0 || headPos.Z != 0) ? headPos :
                                GetInstancePos(c);

                if (pos.X == 0 && pos.Y == 0 && pos.Z == 0)
                    continue;

                if (headPos.X == 0 && headPos.Y == 0 && headPos.Z == 0) headPos = pos;
                if (rootPos.X == 0 && rootPos.Y == 0 && rootPos.Z == 0) rootPos = pos;

                RBX::Vec3 vel{};
                if (hrp.Addr) {
                    auto hrpPrim = hrp.GetPrimitivePtr();
                    if (hrpPrim) vel = memory->read<RBX::Vec3>(hrpPrim + Offsets::Primitive::AssemblyLinearVelocity);
                }

                float dx = pos.X - localPos.X, dy = pos.Y - localPos.Y, dz = pos.Z - localPos.Z;
                float dist = sqrtf(dx * dx + dy * dy + dz * dz);

                if (dist < 2500.f) {
                    Entry e;
                    e.name = cname;
                    e.pos = pos;
                    e.headPos = headPos;
                    e.rootPos = rootPos;
                    e.vel = vel;
                    e.health = hp;
                    e.maxHealth = maxHp;
                    e.dist = dist;
                    e.category = "npc";
                    e.humanoidAddr = hum.Addr;
                    e.instAddr = c.Addr;
                    e.headPartAddr = head.Addr;
                    e.rootPartAddr = hrp.Addr;
                    newEntries.push_back(std::move(e));
                }
            }
        }
    }

    const bool needFallen = needPlants || needOres || needAnimals || needSoldiers ||
                            needTools || needCrates || needDrops || needKeycards;

    std::queue<RBX::RbxInstance> q;
    if (needFallen) {
        q.push(Globals::workspace);
    }
    size_t scanned = 0;
    const size_t kMaxScan = 12000;
    int dbgPlants=0, dbgOres=0, dbgTools=0, dbgSoldiers=0, dbgAnimals=0, dbgCrates=0, dbgDrops=0, dbgKeycards=0;

    while (!q.empty() && scanned < kMaxScan) {
        auto cur = q.front(); q.pop();
        ++scanned;
        std::string name = cur.GetName();
        std::string cls = cur.GetClass();

        bool isPlant = false, isOre=false, isTool=false, isSoldier=false, isAnimal=false;
        bool isCrate=false;
        std::string cat;

        if (needPlants) {
            for(int i=0;i<7;++i) if(name==kPlantNames[i]) { isPlant=true; cat="plant"; break; }
            if (isPlant && !isPlantSelected(name)) isPlant=false;
        }

        if (!isPlant && needOres) {
            bool oreMatch=false;
            for(int i=0;i<3;++i) if(name==kOreNames[i]) { oreMatch=true; break; }
            if (oreMatch) {
                if (isOreSelected(name)) { isOre=true; cat="ore"; }
            }
        }

        if (!isPlant && !isOre && !isTool && needTools) {
            for(int i=0;i<7;++i) if(name.find(kToolNames[i])!=std::string::npos) { if(isToolSelected(kToolNames[i])) { isTool=true; cat="tool"; break; } }
        }

        if (!isPlant && !isOre && !isTool && needSoldiers) {
            if (isSoldierSelected(name)) { isSoldier = true; cat = "soldier"; }
        }

        if (!isPlant && !isOre && !isTool && !isSoldier && needAnimals) {
            for(int i=0;i<3;++i) if(name==kAnimalNames[i]) { if(isAnimalSelected(name)) { isAnimal=true; cat="animal"; } break; }
        }

        if (!isPlant && !isOre && !isTool && !isSoldier && !isAnimal && needCrates) {
            if (isCrateSelected(name, cls)) { isCrate = true; cat = "crate"; }
        }

        if (isPlant||isOre||isTool||isSoldier||isAnimal||isCrate) {
            RBX::Vec3 pos = GetInstancePos(cur);
            if (pos.X!=0 || pos.Y!=0 || pos.Z!=0) {
                std::string dispName = name;
                if (cat=="ore") dispName = OreDisplayName(name);
                else if (cat=="animal") dispName = AnimalDisplayName(name);
                else if (cat=="tool") {

                    for(int i=0;i<6;++i) if(name.find(kToolNames[i])!=std::string::npos) { dispName=kToolNames[i]; break; }
                }
                Entry e; e.name=dispName; e.pos=pos; e.category=cat;
                if(isPlant) e.plantIdx = getPlantIndex(name);
                if(isOre) e.oreIdx = getOreIndex(name);
                if(isAnimal) e.animalIdx = getAnimalIndex(name);
                if(isSoldier) e.soldierIdx = getSoldierIndex(name);
                if(isCrate) e.crateIdx = getCrateIndex(name);
                if(isTool) {
                    for(int i=0;i<7;++i) if(name.find(kToolNames[i])!=std::string::npos) { e.toolIdx=i; break; }

                    auto prim = cur.GetPrimitivePtr();
                    if (!prim) {
                        auto ppAddr = memory->read<std::uintptr_t>(cur.Addr + Offsets::Model::PrimaryPart);
                        if (ppAddr) prim = memory->read<std::uintptr_t>(ppAddr + Offsets::BasePart::Primitive);
                        if (!prim) {
                            for(auto &c: cur.GetChildList()) { prim=c.GetPrimitivePtr(); if(prim) break; }
                        }
                    }
                    if (prim) {
                        auto sz = memory->read<RBX::Vec3>(prim + Offsets::Primitive::Size);
                        if (sz.X>0.1f && sz.Y>0.1f && sz.Z>0.1f) e.size = sz;
                        else e.size = {2,1.5f,2};
                    } else e.size = {2,1.5f,2};
                }

                if (isSoldier || isAnimal) {
                    auto hum = cur.FindChildByClass("Humanoid");
                    if (hum.Addr) {
                        e.health = memory->read<float>(hum.Addr + Offsets::Humanoid::Health);
                        e.maxHealth = memory->read<float>(hum.Addr + Offsets::Humanoid::MaxHealth);
                        auto head = cur.FindChild("Head");
                        e.headPos = head.Addr ? head.GetPos() : pos;
                        auto hrp = cur.FindChild("HumanoidRootPart");
                        e.rootPos = hrp.Addr ? hrp.GetPos() : pos;
                        e.instAddr = cur.Addr;
                        e.humanoidAddr = hum.Addr;
                        e.headPartAddr = head.Addr;
                        e.rootPartAddr = hrp.Addr;
                        if (hrp.Addr) {
                            auto hrpPrim = hrp.GetPrimitivePtr();
                            if (hrpPrim) e.vel = memory->read<RBX::Vec3>(hrpPrim + Offsets::Primitive::AssemblyLinearVelocity);
                        }
                        if (e.headPos.X==0 && e.headPos.Y==0 && e.headPos.Z==0) e.headPos = pos;
                        if (e.rootPos.X==0 && e.rootPos.Y==0 && e.rootPos.Z==0) e.rootPos = pos;
                    } else {
                        e.headPos = pos; e.rootPos = pos;
                    }
                }
                float dx=pos.X-localPos.X, dy=pos.Y-localPos.Y, dz=pos.Z-localPos.Z;
                e.dist = sqrtf(dx*dx+dy*dy+dz*dz);

                if (e.dist < 2500.f) {
                    newEntries.push_back(std::move(e));
                    if(isPlant) dbgPlants++; else if(isOre) dbgOres++; else if(isTool) dbgTools++; else if(isSoldier) dbgSoldiers++; else if(isAnimal) dbgAnimals++; else if(isCrate) dbgCrates++;
                }
            }
        }

        if (needDrops && ((cls == "Folder" && name == "Drops") ||
            (cls == "Model" && (name == "DeadDrop" || name == "itemPickupModel")))) {
            auto pushDrop = [&](const std::string& dname, const RBX::Vec3& pos) {
                if (dname.empty() || (pos.X == 0 && pos.Y == 0 && pos.Z == 0))
                    return;
                Entry e;
                e.name = dname;
                e.pos = pos;
                e.category = "drop";
                const float dx = pos.X - localPos.X, dy = pos.Y - localPos.Y, dz = pos.Z - localPos.Z;
                e.dist = sqrtf(dx * dx + dy * dy + dz * dz);
                if (e.dist < 2500.f) {
                    newEntries.push_back(std::move(e));
                    ++dbgDrops;
                }
            };
            if (cls == "Model") {
                pushDrop(name, GetInstancePos(cur));
            } else {
                for (auto& sub : cur.GetChildList()) {
                    if (sub.GetClass() != "Model")
                        continue;
                    pushDrop(sub.GetName(), GetInstancePos(sub));
                }
            }
        }

        if (needKeycards && cls == "Folder" && name != "Keycards" && name.find("Keycard") != std::string::npos) {
            if (isKeycardSelected(name)) {
                RBX::Vec3 pos{};
                std::vector<std::pair<RBX::RbxInstance, int>> st;
                st.emplace_back(cur, 0);
                int vis = 0;
                while (!st.empty() && vis < 60) {
                    auto [node, depth] = st.back();
                    st.pop_back();
                    ++vis;
                    if (node.GetName() == "HumanoidRootPart" &&
                        (node.GetClass() == "Part" || node.GetClass() == "MeshPart")) {
                        pos = GetInstancePos(node);
                        if (pos.X != 0 || pos.Y != 0 || pos.Z != 0)
                            break;
                        pos = {};
                    }
                    if (depth < 3) {
                        for (auto& c : node.GetChildList())
                            st.emplace_back(c, depth + 1);
                    }
                }
                if (pos.X != 0 || pos.Y != 0 || pos.Z != 0) {
                    Entry e;
                    e.name = name;
                    e.pos = pos;
                    e.category = "keycard";
                    e.keycardIdx = getKeycardIndex(name);
                    const float dx = pos.X - localPos.X, dy = pos.Y - localPos.Y, dz = pos.Z - localPos.Z;
                    e.dist = sqrtf(dx * dx + dy * dy + dz * dz);
                    if (e.dist < 2500.f) {
                        newEntries.push_back(std::move(e));
                        ++dbgKeycards;
                    }
                }
            }
        }

        if (cls=="Workspace" || cls=="Folder" || cls=="Model") {
            auto kids = cur.GetChildList();

            for (auto &k : kids) {
                if (q.size() > 8000) break;
                q.push(k);
            }
        } else if (cls=="Tool") {

        }
    }

    {
        std::lock_guard<std::mutex> lk(mtx);
        entries.swap(newEntries);
        lastCount = (int)entries.size();
    }

    (void)scanned; (void)dbgPlants; (void)dbgOres; (void)dbgTools; (void)dbgSoldiers; (void)dbgAnimals; (void)dbgCrates; (void)dbgDrops; (void)dbgKeycards;
}

void Loop() {
    while (running && Globals::running) {
        UpdateOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}

}
