#pragma once
#include "../../sdk/sdk.h"
#include "../variables/variables.h"
#include "../globals/globals.h"
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <chrono>

namespace WorldCache {
struct Entry {
    std::string name;
    RBX::Vec3 pos{};
    float dist = 0.f;
    std::string category;
    int plantIdx = -1;
    int oreIdx = -1;
    int animalIdx = -1;
    int soldierIdx = -1;
    int toolIdx = -1;
    float health = 0.f;
    float maxHealth = 0.f;
    RBX::Vec3 vel{};
    RBX::Vec3 headPos{};
    RBX::Vec3 rootPos{};
    RBX::Vec3 size{2,2,2};
};

inline std::vector<Entry> entries;
inline std::mutex mtx;
inline std::atomic<bool> running{true};
inline std::chrono::steady_clock::time_point lastScan{};
inline int lastCount = 0;

inline const char* kPlantNames[7] = {"Wool Plant","Blueberry Plant","Raspberry Plant","Lemon Plant","Corn Plant","Pumpkin Plant","Tomato Plant"};
inline const char* kOreNames[3] = {"Stone_Node","Phosphate_Node","Metal_Node"};
inline const char* kAnimalNames[3] = {"PREFAB_ANIMAL_DEER","PREFAB_ANIMAL_WILDBOAR","PREFAB_ANIMAL_WOLF"};
inline const char* kSoldierNames[4] = {"Boris","Bruno","Brutus","Soldier"};
inline const char* kToolNames[7] = {"Base Cabinet","Small Storage Box","Large Storage Box","Anvil","Furnace","Storage Cabinet","Sleeping Bag"};

inline bool isPlantSelected(const std::string& n) {
    for (int i=0;i<7;++i) if (n==kPlantNames[i]) return variables::World::plantsSel[i];
    return false;
}
inline int getPlantIndex(const std::string& n) {
    for (int i=0;i<7;++i) if (n==kPlantNames[i]) return i;
    return -1;
}
inline bool isOreSelected(const std::string& n) {
    for (int i=0;i<3;++i) if (n==kOreNames[i]) return variables::World::oresSel[i];
    return false;
}
inline int getOreIndex(const std::string& n) {
    for (int i=0;i<3;++i) if (n==kOreNames[i]) return i;
    return -1;
}
inline std::string OreDisplayName(const std::string& n) {
    if (n=="Stone_Node") return "Stone";
    if (n=="Phosphate_Node") return "Phosphate";
    if (n=="Metal_Node") return "Metal";
    return n;
}
inline bool isAnimalSelected(const std::string& n) {
    for(int i=0;i<3;++i) if(n==kAnimalNames[i]) return variables::World::animalsSel[i];
    return false;
}
inline int getAnimalIndex(const std::string& n) {
    for(int i=0;i<3;++i) if(n==kAnimalNames[i]) return i;
    return -1;
}
inline std::string AnimalDisplayName(const std::string& n) {
    if(n=="PREFAB_ANIMAL_DEER") return "Deer";
    if(n=="PREFAB_ANIMAL_WILDBOAR") return "WildBoar";
    if(n=="PREFAB_ANIMAL_WOLF") return "Wolf";
    return n;
}
inline bool isSoldierSelected(const std::string& n) {
    for(int i=0;i<4;++i) if(n==kSoldierNames[i]) return variables::World::soldiersSel[i];
    return false;
}
inline int getSoldierIndex(const std::string& n) {
    for(int i=0;i<4;++i) if(n==kSoldierNames[i]) return i;
    return -1;
}
inline bool isToolSelected(const std::string& n) {
    for(int i=0;i<7;++i) if(n==kToolNames[i]) return variables::World::toolsSel[i];
    return false;
}
inline int getToolIndex(const std::string& n) {
    for(int i=0;i<7;++i) if(n==kToolNames[i]) return i;
    return -1;
}

void UpdateOnce();
void Loop();
}
