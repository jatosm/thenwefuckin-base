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
#include "../cache/pf_cache.h"
#include "../cache/cb_cache.h"
#include "../cache/ops_cache.h"
#include "../globals/globals.h"
#include "../tp_handler/tp_handler.h"
#include "../functions/aim/aim.h"
#include "../functions/visual/visual.h"
#include "../functions/mics/mics.h"
#include "../functions/movement/movement.h"
#include "../functions/combat/combat.h"
#include "../functions/freecam/freecam.h"
#include "../functions/aim/viewport_silent.h"
#include "../functions/aim/magic.h"
#include "../functions/aim/pf_silent.h"
#include "../functions/aim/raycast.h"
#include "../functions/world/world.h"
#include "../cache/workspace.h"
#include "../cache/worldcache.h"
#include "../features/worldgeo/worldgeo.h"
#include "../features/preview/preview.h"
#include "../functions/backpack_widget.h"
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

namespace {
int s_dmMismatch = 0;
}

void ResetAimState() {
    Aimbot::lockedPlayerAddr = 0;
    Aimbot::hasTarget = false;
    ViewportSilent::Clear();
    MagicBullet::SetActive(false, {});
    MagicBullet::Ensure(false);
    PfSilent::SetActive(false, {});
}

void RefreshServices() {
    if (!memory->IsConnected())
        return;
    const auto base = memory->get_module_address();
    if (!base)
        return;
    const auto fake = memory->read<std::uintptr_t>(base + Offsets::FakeDataModel::Pointer);
    const auto dm = fake ? memory->read<std::uintptr_t>(fake + Offsets::FakeDataModel::RealDataModel) : 0;
    if (dm == Globals::dataModel.Addr) {
        s_dmMismatch = 0;
        return;
    }

    if (++s_dmMismatch < 3)
        return;
    s_dmMismatch = 0;
    if (!dm) {
        Globals::dataModel = RBX::RbxInstance{0};
        Globals::workspace = RBX::RbxInstance{0};
        Globals::players = RBX::RbxInstance{0};
        Globals::camera = RBX::RbxInstance{0};
        Globals::localPlayer = RBX::RbxInstance{0};
        PlayerCache::players.clear();
        PlayerCache::localRootPrim = 0;
        PfCache::players.clear();
        PfCache::workspacePlayersAddr = 0;
        CbCache::players.clear();
        CbCache::charactersAddr = 0;
        OpsCache::players.clear();
        OpsCache::viewmodelsAddr = 0;
        ResetAimState();
        return;
    }
    Globals::dataModel = RBX::RbxInstance{dm};
    Globals::workspace = Globals::dataModel.FindChildByClass("Workspace");
    Globals::players = Globals::dataModel.FindChildByClass("Players");
    Globals::camera = Globals::workspace.FindChildByClass("Camera");
    const auto local = Globals::players.Addr
        ? memory->read<std::uintptr_t>(Globals::players.Addr + Offsets::Player::LocalPlayer)
        : 0;
    Globals::localPlayer = RBX::RbxInstance{local};
    PlayerCache::players.clear();
    PlayerCache::localRootPrim = 0;
    PfCache::players.clear();
    PfCache::workspacePlayersAddr = 0;
    CbCache::players.clear();
    CbCache::charactersAddr = 0;
    OpsCache::players.clear();
    OpsCache::viewmodelsAddr = 0;
    ResetAimState();
}

bool init() {
    std::uint32_t pid = memory->find_process_id(kProc);
    if (!pid) {
        return false;
    }
    if (!memory->attach_to_process(kProc)) {
        return false;
    }
    if (!memory->find_module_address(kProc)) {
        return false;
    }
    const auto base = memory->get_module_address();
    if (!base) {
        return false;
    }
    const auto fake = memory->read<std::uintptr_t>(base + Offsets::FakeDataModel::Pointer);
    if (!fake) {
        return false;
    }
    const auto dm = memory->read<std::uintptr_t>(fake + Offsets::FakeDataModel::RealDataModel);
    if (!dm) {
        return false;
    }
    const auto ve = memory->read<std::uintptr_t>(base + Offsets::VisualEngine::Pointer);
    if (!ve) {
        return false;
    }
    Globals::dataModel = RBX::RbxInstance{dm};
    Globals::renderEngine = RBX::RenderEngine{ve};
    Globals::workspace = Globals::dataModel.FindChildByClass("Workspace");
    Globals::players = Globals::dataModel.FindChildByClass("Players");
    Globals::camera = Globals::workspace.FindChildByClass("Camera");
    const auto local = memory->read<std::uintptr_t>(Globals::players.Addr + Offsets::Player::LocalPlayer);
    Globals::localPlayer = RBX::RbxInstance{local};
    return true;
}

std::int32_t Run() {
    if (!init())
        return 1;
    OverlayWindow overlay;
    if (!overlay.Initialize()) {
        return -1;
    }
    timeBeginPeriod(1);
    std::thread tpThread(Core::tp_handler::thread);
    std::thread localThread(Mics::Loop);
    std::thread wsThread(WorkspaceCache::Loop);
    std::thread worldThread(WorldCache::Loop);
    std::thread geoThread(WorldGeo::Loop);
    std::thread pingThread(Ping::Loop);
    std::thread moveThread(Movement::Loop);
    std::thread combatThread(Combat::Loop);

    Freecam::Start();
    int frame = 0;
    while (memory->IsConnected() && Globals::running) {
        const auto frameStart = std::chrono::steady_clock::now();
        static auto repStart = frameStart;
        static double tSvc = 0, tPlr = 0, tMenu = 0, tAim = 0, tEsp = 0;
        static double tWorld = 0, tMesh = 0, tMisc = 0, tEnd = 0;
        static int frames = 0;
        auto elapsedMs = [](const auto& a, const auto& b) {
            return (double)std::chrono::duration_cast<std::chrono::microseconds>(b - a).count() / 1000.0;
        };
        if (!game_open())
            break;
        if (GetAsyncKeyState(VK_INSERT) & 1)
            variables::menuOpen = !variables::menuOpen;

        const bool showWatermark = Keys::WatermarkOn();
        const bool showKeybinds = Keys::KeybindsOn();
        bool wantDraw = variables::menuOpen || showWatermark || showKeybinds || BackpackWidget::IsOpen() ||
            variables::ESP::enabled || variables::World::enabled || variables::World::wireframe ||
            (variables::Aimbot::playerPreview && Preview::Ready()) ||
            (variables::Aimbot::enabled && variables::Aimbot::showFOV) ||
            (variables::Aimbot::silentTracer && Aimbot::hasTarget) ||
            (variables::Aimbot::predictionLine && variables::Aimbot::prediction && Aimbot::hasTarget);
        {

            static auto lastMenuOpen = std::chrono::steady_clock::now() - std::chrono::seconds(10);
            if (variables::menuOpen)
                lastMenuOpen = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastMenuOpen).count() < 400)
                wantDraw = true;
        }
        static bool idleClearPending = false;
        if (!wantDraw) {
            overlay.PumpMessages();
            ++frames;
            if (idleClearPending) {
                const int idleLimit = variables::Misc::fpsLimit;
                if (idleLimit >= 60) {
                    const auto budget = std::chrono::microseconds(1000000 / idleLimit);
                    auto spent = std::chrono::steady_clock::now() - frameStart;
                    if (spent < budget)
                        std::this_thread::sleep_for(budget - spent);
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(0));
                }
                continue;
            }
            idleClearPending = true;

        } else {
            idleClearPending = false;
        }
        auto t0 = std::chrono::steady_clock::now();

        {
            static auto lastSvc = std::chrono::steady_clock::now() - std::chrono::seconds(10);
            if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastSvc).count() >= 250) {
                lastSvc = std::chrono::steady_clock::now();
                RefreshServices();
            }
        }
        tSvc += elapsedMs(t0, std::chrono::steady_clock::now());
        if (!Globals::renderEngine.Addr || !Globals::players.Addr || !Globals::localPlayer.Addr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        t0 = std::chrono::steady_clock::now();

        {
            static auto lastPlrUpd = std::chrono::steady_clock::now() - std::chrono::seconds(10);
            const auto nowP = std::chrono::steady_clock::now();
            CbCache::update();
            PfCache::update();
            OpsCache::update();
            if ((frame % 3 == 0) && std::chrono::duration_cast<std::chrono::milliseconds>(nowP - lastPlrUpd).count() >= 4) {
                lastPlrUpd = nowP;
                if (!CbCache::charactersAddr && !OpsCache::viewmodelsAddr) {
                    PlayerCache::updateplayers();
                } else {
                    PlayerCache::players.clear();
                }
            }
        }
        ++frame;
        Freecam::Update();
        tPlr += elapsedMs(t0, std::chrono::steady_clock::now());
        t0 = std::chrono::steady_clock::now();
        overlay.BeginFrame();
        overlay.RenderMenu();

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        overlay.render(dl);
        Preview::DrawPanel();
        tMenu += elapsedMs(t0, std::chrono::steady_clock::now());

        const bool needAim = variables::Aimbot::enabled || variables::Aimbot::triggerbot;
        const bool needEsp = variables::ESP::enabled;
        const bool needWorld = variables::World::enabled;
        const bool needWire = variables::World::wireframe;
        const bool needTracer = variables::Aimbot::silentTracer;
        const bool needPredLine = variables::Aimbot::predictionLine && variables::Aimbot::prediction;
        const bool needView = needAim || needEsp || needWorld || needWire || needTracer || needPredLine;
        RBX::Mat4 vm{};
        if (needView)
            vm = Globals::renderEngine.GetViewMat();
        t0 = std::chrono::steady_clock::now();
        if (needAim)
            Aimbot::RunAimbot(vm);
        tAim += elapsedMs(t0, std::chrono::steady_clock::now());
        t0 = std::chrono::steady_clock::now();
        if (needEsp)
            Visuals::RenderESP(dl, vm);
        tEsp += elapsedMs(t0, std::chrono::steady_clock::now());
        t0 = std::chrono::steady_clock::now();
        if (needWorld)
            WorldVisuals::Render(dl, vm);
        if (needWire)
            WorldGeo::Render(dl, vm);
        tWorld += elapsedMs(t0, std::chrono::steady_clock::now());
        t0 = std::chrono::steady_clock::now();
        if (needEsp && variables::ESP::meshChams)
            Visuals::RenderMeshChams(dl, vm);
        tMesh += elapsedMs(t0, std::chrono::steady_clock::now());
        t0 = std::chrono::steady_clock::now();
        if (needTracer)
            Aimbot::RenderTracer(dl);
        if (needPredLine)
            Aimbot::RenderPredictionLine(dl);
        tMisc += elapsedMs(t0, std::chrono::steady_clock::now());
        t0 = std::chrono::steady_clock::now();
        overlay.EndFrame();
        tEnd += elapsedMs(t0, std::chrono::steady_clock::now());
        ++frames;
        if (elapsedMs(repStart, std::chrono::steady_clock::now()) >= 5000.0) {
            repStart = std::chrono::steady_clock::now();
            tSvc = tPlr = tMenu = tAim = tEsp = tWorld = tMesh = tMisc = tEnd = 0;
            frames = 0;
        }
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
    PfSilent::Shutdown();
    if (tpThread.joinable())
        tpThread.join();
    if (localThread.joinable())
        localThread.join();
    WorldCache::running = false;
    WorldGeo::running = false;
    Freecam::Stop();
    if (wsThread.joinable())
        wsThread.join();
    if (worldThread.joinable())
        worldThread.join();
    if (geoThread.joinable())
        geoThread.join();
    if (pingThread.joinable())
        pingThread.join();
    if (moveThread.joinable())
        moveThread.join();
    if (combatThread.joinable())
        combatThread.join();
    overlay.Cleanup();
    return 0;
}
}
