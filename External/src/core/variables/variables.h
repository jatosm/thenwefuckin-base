#pragma once
#include <Windows.h>
#include <string>
#include "../../../ext/imgui/imgui.h"

namespace variables {
inline bool menuOpen = false;
inline int selectedTab = 0;
inline bool waitingForKey = false;
inline int* keyToRebind = nullptr;
inline bool teamCheck = false;
inline int teamCheckKey = 0;
inline int teamCheckKeyMode = 0;

namespace Aimbot {
inline bool enabled = false;
inline bool showFOV = false;
inline float fovRadius = 100.0f;
inline float smoothing = 5.0f;
inline int aimTarget = 0;
inline int aimMethod = 0;
inline int aimbotKey = 2;
inline ImVec4 fovColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
inline bool visibleCheck = false;
inline bool useDeadzone = false;
inline float deadzone = 5.0f;
inline bool triggerbot = false;
inline int triggerKey = 6;
inline int triggerDelay = 100;
inline bool prediction = false;
inline bool fallen_prediction = false;
inline float fallen_bv_override = 0.0f;
inline float fallen_grav_mult = 0.0f;
inline std::string detected_weapon_name = "None";
inline float prediction_ping = 0.0f;
inline float target_velocity_scale = 1.0f;
inline float target_gravity_comp = 0.0f;
inline int selected_weapon_index = 0;
inline bool includeNPC = false;
inline bool silentTracer = false;
inline ImVec4 silentTracerColor = ImVec4(0.2f, 1.0f, 0.6f, 1.0f);
inline float silentTracerThickness = 1.5f;
inline bool predictionLine = false;
inline ImVec4 predictionLineColor = ImVec4(1.0f, 0.85f, 0.2f, 1.0f);
inline float predictionLineThickness = 1.5f;

}

namespace ESP {inline float visualScroll = 0.f;
inline bool enabled = false;
inline bool boxes = false;
inline bool names = false;
inline bool distance = false;
inline bool healthBar = false;
inline bool skeleton = false;
inline float skeletonThickness = 2.0f;
inline bool skeletonOutline = true;
inline bool deadCheck = true;
inline bool localPlayer = false;
inline bool tool = false;
inline ImVec4 toolColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
inline bool flags = false;
inline ImVec4 flagsColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
inline bool flagSel[8] = {true, true, true, true, true, true, true, true};
inline bool headDot = false;
inline ImVec4 headDotColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
inline float headDotSize = 4.0f;
inline bool viewDirection = false;
inline ImVec4 viewDirColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
inline float viewDirLength = 8.0f;
inline ImVec4 boxColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
inline int boxMode = 0;
inline bool boxFilled = false;
inline bool boxFillGradient = false;
inline ImVec4 boxFillColor = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
inline ImVec4 boxFillColor2 = ImVec4(0.2f, 0.4f, 1.0f, 0.25f);
inline ImVec4 nameColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
inline ImVec4 distanceColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
inline ImVec4 healthColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
inline ImVec4 skeletonColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

namespace Local {
inline bool jumpEnabled = false;
inline float jumpPower = 50.0f;
inline int jumpKey = 0;
inline int jumpKeyMode = 0;
}

namespace Misc {
inline bool streamProof = false;
inline int streamKey = 0;
inline int streamKeyMode = 0;
inline bool watermark = true;
inline int watermarkKey = 0;
inline int watermarkKeyMode = 0;
inline bool keybinds = true;
inline int keybindsKey = 0;
inline int keybindsKeyMode = 0;
inline bool vsync = false;
inline int fpsLimit = 0;
inline int priority = 1;
inline float menuFontSize = 1.0f;
inline float espFontSize = 13.0f;
}

namespace Movement {
inline bool fov = false;
inline int fovKey = 0;
inline int fovKeyMode = 0;
inline float fovValue = 70.0f;
inline bool fly = false;
inline int flyKey = 0;
inline int flyKeyMode = 0;
inline int flyMethod = 0;
inline float flySpeed = 60.0f;
inline float flyVerticalBoost = 1.0f;
inline bool noclip = false;
inline int noclipKey = 0;
inline int noclipKeyMode = 0;
inline int noclipMode = 0;
inline float flyDamping = 10.0f;
inline bool flyCheckTyping = false;
inline bool bunnyHop = false;
inline int bunnyHopKey = 0;
inline int bunnyHopKeyMode = 0;
inline float bunnyHopSpeed = 32.0f;
inline bool hipHeight = false;
inline float hipHeightValue = 2.0f;
}

namespace World {
inline bool enabled = false;
inline bool name = false;
inline bool distance = false;
inline bool ores = false;
inline bool oresSel[3] = {true, true, true};
inline ImVec4 oresColor[3] = {ImVec4(0.6f,0.6f,0.6f,1.0f), ImVec4(1.0f,0.85f,0.2f,1.0f), ImVec4(0.8f,0.4f,0.2f,1.0f)};
inline bool plants = false;
inline bool plantsSel[7] = {true, true, true, true, true, true, true};
inline ImVec4 plantsColor[7] = {ImVec4(0.85f,0.85f,0.85f,1.0f), ImVec4(0.3f,0.5f,1.0f,1.0f), ImVec4(1.0f,0.2f,0.3f,1.0f), ImVec4(1.0f,1.0f,0.3f,1.0f), ImVec4(1.0f,0.85f,0.2f,1.0f), ImVec4(1.0f,0.5f,0.1f,1.0f), ImVec4(1.0f,0.3f,0.3f,1.0f)};
inline bool animals = false;
inline bool animalsSel[3] = {true, true, true};
inline ImVec4 animalsColor[3] = {ImVec4(0.6f,0.8f,0.6f,1.0f), ImVec4(0.8f,0.6f,0.4f,1.0f), ImVec4(0.7f,0.7f,0.7f,1.0f)};
inline bool animalsBox = false;
inline bool animalsHealth = false;
inline bool soldiers = false;
inline bool soldiersSel[4] = {true, true, true, true};
inline ImVec4 soldiersColor[4] = {ImVec4(1.0f,0.3f,0.3f,1.0f), ImVec4(0.3f,0.6f,1.0f,1.0f), ImVec4(1.0f,0.6f,0.2f,1.0f), ImVec4(0.8f,0.8f,0.8f,1.0f)};
inline bool soldiersBox = false;
inline bool soldiersHealth = false;
inline ImVec4 murderColor = ImVec4(1.0f, 0.15f, 0.15f, 1.0f);
inline ImVec4 sheriffColor = ImVec4(0.2f, 0.5f, 1.0f, 1.0f);
inline ImVec4 innocentColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
inline bool tools = false;
inline bool toolsSel[7] = {true, true, true, true, true, true, true};
inline ImVec4 toolsColor[7] = {ImVec4(0.6f,0.8f,1.0f,1.0f), ImVec4(0.6f,0.8f,1.0f,1.0f), ImVec4(0.6f,0.8f,1.0f,1.0f), ImVec4(0.6f,0.8f,1.0f,1.0f), ImVec4(0.6f,0.8f,1.0f,1.0f), ImVec4(0.6f,0.8f,1.0f,1.0f), ImVec4(0.6f,0.8f,1.0f,1.0f)};
inline bool toolsBox = false;
inline float worldScroll = 0.f;
}

namespace Freecam {
inline bool enabled = false;
inline int key = 0;
inline int keyMode = 0;
inline float sensitivity = 0.008f;
inline float speed = 60.0f;
inline int shiftKey = VK_SHIFT;
inline float shiftMultiplier = 2.0f;
inline bool azerty = false;
inline bool freezeCharacter = true;
}

namespace Theme {
inline ImVec4 background = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
inline ImVec4 panels = ImVec4(0.1529f, 0.1529f, 0.1529f, 1.0f);
inline ImVec4 controls = ImVec4(0.1843f, 0.1843f, 0.1843f, 1.0f);
inline ImVec4 accent = ImVec4(0.3490f, 0.8118f, 0.8275f, 1.0f);
inline ImVec4 text = ImVec4(0.7600f, 0.7600f, 0.7600f, 1.0f);
inline ImVec4 textBright = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}
}
