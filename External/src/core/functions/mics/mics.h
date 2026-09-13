#pragma once
#include "../../../sdk/sdk.h"
#include "../../variables/variables.h"
#include "../../globals/globals.h"
#include "../../../memory/memory.h"
#include "../../../sdk/offsets.h"
#include "../../../../ext/imgui/imgui.h"
#include "../../../sdk/structs.h"

namespace Mics {
void Tick();
void Loop();
void RenderLocalMenu();
void RenderMiscMenu();
}
