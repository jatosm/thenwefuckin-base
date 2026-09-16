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

void UpdateOnce() {
    if (!variables::World::enabled) {
        std::lock_guard<std::mutex> lk(mtx);
        if (!entries.empty()) {
            entries.clear();
            lastCount = 0;
            printf("[World] disabled -> cleared cache\n");
        }
        return;
    }
    if (!Globals::workspace.Addr || !memory->IsConnected()) return;

    auto now = std::chrono::steady_clock::now();
    if (now - lastScan < std::chrono::milliseconds(50)) return;
    lastScan = now;

    
    bool needPlants = variables::World::plants;
    bool needOres = variables::World::ores;
    bool needAnimals = variables::World::animals;
    bool needSoldiers = variables::World::soldiers;
    bool needTools = variables::World::tools;
    if (!needPlants && !needOres && !needAnimals && !needSoldiers && !needTools) {
        std::lock_guard<std::mutex> lk(mtx);
        if (!entries.empty()) entries.clear();
        return;
    }

    std::vector<Entry> newEntries;
    newEntries.reserve(256);

    
    std::queue<RBX::RbxInstance> q;
    q.push(Globals::workspace);
    size_t scanned = 0;
    const size_t kMaxScan = 12000;
    int dbgPlants=0, dbgOres=0, dbgTools=0, dbgSoldiers=0, dbgAnimals=0;

    
    RBX::Vec3 localPos{};
    if (PlayerCache::localRootPrim)
        localPos = memory->read<RBX::Vec3>(PlayerCache::localRootPrim + Offsets::Primitive::Position);
    else if (PlayerCache::players.size()>0)
        localPos = PlayerCache::localPlayerPos;

    while (!q.empty() && scanned < kMaxScan) {
        auto cur = q.front(); q.pop();
        ++scanned;
        std::string name = cur.GetName();
        std::string cls = cur.GetClass();

        bool isPlant = false, isOre=false, isTool=false, isSoldier=false, isAnimal=false;
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
            for(int i=0;i<4;++i) if(name==kSoldierNames[i]) { if(isSoldierSelected(name)) { isSoldier=true; cat="soldier"; } break; }
        }
        
        if (!isPlant && !isOre && !isTool && !isSoldier && needAnimals) {
            for(int i=0;i<3;++i) if(name==kAnimalNames[i]) { if(isAnimalSelected(name)) { isAnimal=true; cat="animal"; } break; }
        }

        if (isPlant||isOre||isTool||isSoldier||isAnimal) {
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
                    if(isPlant) dbgPlants++; else if(isOre) dbgOres++; else if(isTool) dbgTools++; else if(isSoldier) dbgSoldiers++; else if(isAnimal) dbgAnimals++;
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
    
    static auto lastPrint = std::chrono::steady_clock::now() - std::chrono::seconds(5);
    static int lastPrintCount = -1;
    if (lastPrintCount != lastCount || now - lastPrint > std::chrono::seconds(2)) {
        lastPrint = now; lastPrintCount = lastCount;
        printf("[World] scan %zu instances -> cached %d (plants %d ores %d tools %d soldiers %d animals %d) | enabled=%d plants=%d ores=%d\n",
            scanned, lastCount, dbgPlants, dbgOres, dbgTools, dbgSoldiers, dbgAnimals,
            (int)variables::World::enabled, (int)variables::World::plants, (int)variables::World::ores);
    }
}

void Loop() {
    printf("[World] cache thread started\n");
    while (running && Globals::running) {
        UpdateOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
    printf("[World] cache thread stopped\n");
}

}
