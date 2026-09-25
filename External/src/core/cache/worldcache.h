// discord.gg/thenwefuckin
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
    int crateIdx = -1;
    int keycardIdx = -1;
    float health = 0.f;
    float maxHealth = 0.f;
    RBX::Vec3 vel{};
    RBX::Vec3 headPos{};
    RBX::Vec3 rootPos{};
    RBX::Vec3 size{2,2,2};
    std::uintptr_t humanoidAddr = 0;
    std::uintptr_t instAddr = 0;
    std::uintptr_t headPartAddr = 0;
    std::uintptr_t rootPartAddr = 0;
    int zeroStreak = 0;
};

inline std::vector<Entry> entries;
inline std::mutex mtx;
inline std::atomic<bool> running{true};
inline std::chrono::steady_clock::time_point lastScan{};
inline std::chrono::steady_clock::time_point lastFullScan{};
inline int lastCount = 0;

inline const char* kPlantNames[7] = {"Wool Plant","Blueberry Plant","Raspberry Plant","Lemon Plant","Corn Plant","Pumpkin Plant","Tomato Plant"};
inline const char* kOreNames[3] = {"Stone_Node","Phosphate_Node","Metal_Node"};
inline const char* kAnimalNames[3] = {"PREFAB_ANIMAL_DEER","PREFAB_ANIMAL_WILDBOAR","PREFAB_ANIMAL_WOLF"};
inline constexpr int kSoldierCount = 69;
inline const char* kSoldierNames[kSoldierCount] = {"Boris","Bruno","Brutus","Soldier",
"Alek","Alexei","Andrei","Artyom","Bravo","Cervus","Clutch","Cougar","Denis","Dmitri",
"Fyodor","Gambit","Gennady","Grigory","Gunner","Hawk","Ilian","Ilya","Ivan","Jackal",
"Kirill","Konstantin","Kostya","Leonid","Maxim","Mikhail","Miro","Nazar","Nikolai","Oleg",
"Overkill","Panther","Pyotr","Ranger","Reaper","Redline","Roman","Rostislav","Ruslan","Semyon",
"Sergei","Specter","Stalemate","Talon","Timofey","Vadim","Vitaly","Vlad","Yegor","Yuri","Zakhar",
"[Sniper] Artyom","[Sniper] Denis","[Sniper] Fyodor","[Sniper] Ilian","[Sniper] Miro",
"[Sniper] Nikolai","[Sniper] Pavel","[Sniper] Semyon","[Sniper] Stanislav","[Sniper] Vadim",
"[Sniper] Vlad","[Sniper] Yegor","[Sniper] Yuri","[Sniper] Zakhar"};
inline const char* kToolNames[7] = {"Base Cabinet","Small Storage Box","Large Storage Box","Anvil","Furnace","Storage Cabinet","Sleeping Bag"};
inline const char* kKeycardNames[5] = {"Yellow","Purple","Red","Pink","Black"};

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
    for(int i=0;i<kSoldierCount;++i) if(n==kSoldierNames[i]) return variables::World::soldiersSel[i];
    return false;
}
inline int getSoldierIndex(const std::string& n) {
    for(int i=0;i<kSoldierCount;++i) if(n==kSoldierNames[i]) return i;
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
inline bool isKeycardSelected(const std::string& n) {
    for(int i=0;i<5;++i) if(n.find(kKeycardNames[i])!=std::string::npos) return variables::World::keycardsSel[i];
    return false;
}
inline int getKeycardIndex(const std::string& n) {
    for(int i=0;i<5;++i) if(n.find(kKeycardNames[i])!=std::string::npos) return i;
    return -1;
}
inline constexpr int kCrateCount = 34;
inline const char* kCrateNames[kCrateCount] = {"Care Package","Food Crate","Wooden Crate",
"Locked Wooden Crate","Locked Steel Crate","Locked Metal Crate","Timed Crate","Brutus Locker",
"Medium Wooden Crate","XMedium Wooden Crate","Complex Crate","Crate4","Military Crate",
"Cabinet","File Cabinet","XFile Cabinet","XFileCabinet","XCabinet","HospitalCabinet",
"Military File Cabinet","Locker","Weapon Locker","XWeapon Locker","Safe","Fridge",
"TallFridge","XFridge","XTallFridge","Package","Foodcrate_A1","Foodcrate_A2","DeadDrop",
"XRifle Case","XCloset"};
inline bool isCrateSelected(const std::string& n, const std::string& cls) {
    if (cls != "Model" && cls != "MeshPart")
        return false;
    for(int i=0;i<kCrateCount;++i) if(n==kCrateNames[i]) return variables::World::cratesSel[i];
    return false;
}
inline int getCrateIndex(const std::string& n) {
    for(int i=0;i<kCrateCount;++i) if(n==kCrateNames[i]) return i;
    return -1;
}

void UpdateOnce();
void Loop();
}
