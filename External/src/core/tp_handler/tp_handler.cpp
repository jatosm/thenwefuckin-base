// discord.gg/thenwefuckin
#include "tp_handler.h"
#include <chrono>
#include <thread>
#include <windows.h>
#include "../globals/globals.h"
#include "../../memory/memory.h"
#include "../../sdk/offsets.h"
#include "../../sdk/sdk.h"

namespace {
void reset_globals() {
    Globals::dataModel = RBX::RbxInstance{};
    Globals::renderEngine = RBX::RenderEngine{0};
    Globals::workspace = RBX::RbxInstance{};
    Globals::players = RBX::RbxInstance{};
    Globals::camera = RBX::RbxInstance{};
    Globals::localPlayer = RBX::RbxInstance{};
}

bool refresh() {
    const auto base = memory->get_module_address();
    if (!base)
        return false;
    const auto fake = memory->read<std::uintptr_t>(base + Offsets::FakeDataModel::Pointer);
    if (!fake)
        return false;
    const auto dm = memory->read<std::uintptr_t>(fake + Offsets::FakeDataModel::RealDataModel);
    const auto ve = memory->read<std::uintptr_t>(base + Offsets::VisualEngine::Pointer);
    if (!dm || !ve)
        return false;
    Globals::dataModel = RBX::RbxInstance{dm};
    Globals::renderEngine = RBX::RenderEngine{ve};
    Globals::workspace = Globals::dataModel.FindChildByClass("Workspace");
    Globals::players = Globals::dataModel.FindChildByClass("Players");
    Globals::camera = Globals::workspace.FindChildByClass("Camera");
    if (!Globals::workspace.Addr || !Globals::players.Addr || !Globals::camera.Addr) {
        Globals::localPlayer = RBX::RbxInstance{};
        return false;
    }
    const auto local = memory->read<std::uintptr_t>(Globals::players.Addr + Offsets::Player::LocalPlayer);
    Globals::localPlayer = RBX::RbxInstance{local};
    return true;
}
}

void Core::tp_handler::thread() {
    using namespace std::chrono_literals;
    while (Globals::running) {
        if (!memory->IsConnected()) {
            Globals::running = false;
            break;
        }
        if (!refresh())
            reset_globals();
        std::this_thread::sleep_for(100ms);
    }
}
