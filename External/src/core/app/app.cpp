#include "app.h"
#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include <thread>
#include <chrono>
#include "../../memory/memory.h"
#include "../../sdk/offsets.h"
#include "../../sdk/sdk.h"
#include "../cache/cache.h"
#include "../globals/globals.h"
#include "../tp_handler/tp_handler.h"
#include "../functions/aim/aim.h"
#include "../functions/visual/visual.h"
#include "../functions/mics/mics.h"
#include "../functions/movement/movement.h"
#include "../functions/freecam/freecam.h"
#include "../functions/aim/viewport_silent.h"
#include "../functions/world/world.h"
#include "../cache/workspace.h"
#include "../cache/worldcache.h"
#include "../net/ping.h"
#include "../../render/render.h"

#pragma comment(lib, "winmm.lib")

namespace {
constexpr const char* kProc = "RobloxPlayerBeta.exe";
constexpr const wchar_t* kTitle = L"Roblox";
}

namespace App {
bool game_open() {
    return FindWindowW(nullptr, kTitle) != nullptr;
}

bool init() {
    if (!memory->find_process_id(kProc)) {
        printf("unable to get pid.\nmake sure roblox is running.\n");
        system("pause");
        return false;
    }
    if (!memory->attach_to_process(kProc)) {
        printf("unable to attach to roblox.");
        return false;
    }
    if (!memory->find_module_address(kProc)) {
        printf("unable to find main module address!");
        return false;
    }
    const auto base = memory->get_module_address();
    if (!base) {
        printf("base address is null.");
        return false;
    }
    const auto fake = memory->read<std::uintptr_t>(base + Offsets::FakeDataModel::Pointer);
    if (!fake) {
        printf("fake datamodel pointer is null.");
        return false;
    }
    const auto dm = memory->read<std::uintptr_t>(fake + Offsets::FakeDataModel::RealDataModel);
    if (!dm) {
        printf("datamodel pointer is null.");
        return false;
    }
    const auto ve = memory->read<std::uintptr_t>(base + Offsets::VisualEngine::Pointer);
    if (!ve) {
        printf("visualengine pointer is null.");
        return false;
    }
    Globals::dataModel = RBX::RbxInstance{dm};
    Globals::renderEngine = RBX::RenderEngine{ve};
    Globals::workspace = Globals::dataModel.FindChildByClass("Workspace");
    Globals::players = Globals::dataModel.FindChildByClass("Players");
    Globals::camera = Globals::workspace.FindChildByClass("Camera");
    const auto local = memory->read<std::uintptr_t>(Globals::players.Addr + Offsets::Player::LocalPlayer);
    Globals::localPlayer = RBX::RbxInstance{local};
    system("cls");
    return true;
}

std::int32_t Run() {
    if (!init())
        return 1;
    OverlayWindow overlay;
    if (!overlay.Initialize()) {
        std::cout << "[!] failed to initialize overlay\n";
        return -1;
    }
    std::cout << "[+] overlay initialized\n[*] press insert to toggle menu\n\n";
    timeBeginPeriod(1);
    std::thread tpThread(Core::tp_handler::thread);
    std::thread localThread(Mics::Loop);
    std::thread wsThread(WorkspaceCache::Loop);
    std::thread worldThread(WorldCache::Loop);
    std::thread pingThread(Ping::Loop);
    std::thread moveThread(Movement::Loop);
    Freecam::Start();
    int frame = 0;
    while (memory->IsConnected() && Globals::running) {
        const auto frameStart = std::chrono::steady_clock::now();
        if (!game_open())
            break;
        if (GetAsyncKeyState(VK_INSERT) & 1)
            variables::menuOpen = !variables::menuOpen;
        if (!Globals::renderEngine.Addr || !Globals::players.Addr || !Globals::localPlayer.Addr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        if (frame % 3 == 0)
            PlayerCache::updateplayers();
        ++frame;
        Freecam::Update();
        overlay.BeginFrame();
        overlay.RenderMenu();

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        overlay.render(dl);
        const auto vm = Globals::renderEngine.GetViewMat();
        Aimbot::RunAimbot(vm);
        Visuals::RenderESP(dl, vm);
        WorldVisuals::Render(dl, vm);
        Aimbot::RenderTracer(dl);
        Aimbot::RenderPredictionLine(dl);
        overlay.EndFrame();
        const int limit = variables::Misc::fpsLimit;
        if (limit >= 60) {
            const auto budget = std::chrono::microseconds(1000000 / limit);
            auto spent = std::chrono::steady_clock::now() - frameStart;
            if (spent < budget) {
                auto remain = budget - spent;
                if (remain > std::chrono::milliseconds(2))
                    std::this_thread::sleep_for(remain - std::chrono::milliseconds(1));
                while (std::chrono::steady_clock::now() - frameStart < budget) {
                }
            }
        }
    }
    timeEndPeriod(1);
    Globals::running = false;
    ViewportSilent::Shutdown();
    if (tpThread.joinable())
        tpThread.join();
    if (localThread.joinable())
        localThread.join();
    WorldCache::running = false;
    Freecam::Stop();
    if (wsThread.joinable())
        wsThread.join();
    if (worldThread.joinable())
        worldThread.join();
    if (pingThread.joinable())
        pingThread.join();
    if (moveThread.joinable())
        moveThread.join();
    overlay.Cleanup();
    return 0;
}
}
