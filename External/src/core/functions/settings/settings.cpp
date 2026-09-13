#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "settings.h"
#include "../../variables/variables.h"
#include "../../../render/menu/library.h"
#include "../aim/fallen_prediction.h"
#include <windows.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace Settings {
void RenderAimMenu() {
    float ay = 46.0f;
    imGuiCustom::Checkbox("Enable Aimbot", &variables::Aimbot::enabled, ImVec2(12.0f, ay));
    imGuiCustom::Keybind("aim_key", &variables::Aimbot::aimbotKey, ImVec2(222.0f, ay - 1.0f), ImVec2(68.0f, 13.0f), nullptr);
    ay += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Show FOV", &variables::Aimbot::showFOV, ImVec2(12.0f, ay));
    imGuiCustom::ColorSquare("fov_color", &variables::Aimbot::fovColor, ImVec2(264.0f, ay + 1.0f));
    ay += imGuiCustom::CheckStep();
    imGuiCustom::SliderFloat("fov_radius", &variables::Aimbot::fovRadius, 10.0f, 500.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "FOV Radius", "%.0f");
    ay += imGuiCustom::SliderTop() + 15.0f;
    imGuiCustom::SliderFloat("smoothing", &variables::Aimbot::smoothing, 1.0f, 20.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "Smoothing", "%.1f");
    ay += imGuiCustom::SliderTop() + 15.0f;
    const char* targets[] = {"Head", "Torso", "Left Arm", "Right Arm", "Left Leg", "Right Leg", "HumanoidRootPart", "Closest"};
    ay += imGuiCustom::ComboTop();
    imGuiCustom::Combo("aim_target", &variables::Aimbot::aimTarget, targets, 8, ImVec2(12.0f, ay), 158.0f, "Aim Target:");
    ay += imGuiCustom::ComboStep();
    const char* methods[] = {"Mouse", "Memory", "Viewport"};
    imGuiCustom::Combo("aim_method", &variables::Aimbot::aimMethod, methods, 3, ImVec2(12.0f, ay + imGuiCustom::ComboTop()), 158.0f, "Aim Method:");
    ay += imGuiCustom::ComboTop() + 22.0f;
    imGuiCustom::Checkbox("Prediction", &variables::Aimbot::prediction, ImVec2(12.0f, ay));
    ay += imGuiCustom::CheckStep();
    if (variables::Aimbot::prediction) {
        imGuiCustom::Checkbox("Fallen Prediction##fallen_pred", &variables::Aimbot::fallen_prediction, ImVec2(12.0f, ay));
        ay += imGuiCustom::CheckStep();
        if (variables::Aimbot::fallen_prediction) {
            static std::vector<const char*> weapon_items;
            if (weapon_items.empty()) { for (auto& w : FALLEN_WEAPON_LIST) weapon_items.push_back(w.c_str()); }
            ay += imGuiCustom::ComboTop();
            imGuiCustom::Combo("fallen_weapon##fallen_weapon", &variables::Aimbot::selected_weapon_index, weapon_items.data(), (int)weapon_items.size(), ImVec2(12.0f, ay), 158.0f, "Weapon:");
            ay += imGuiCustom::ComboStep();
            {
                ImDrawList* wdl = ImGui::GetWindowDrawList();
                ImVec2 base = ImGui::GetWindowPos();
                ImFont* wfont = imGuiCustom::GetFonts().CascadiaMonoBL ? imGuiCustom::GetFonts().CascadiaMonoBL : ImGui::GetFont();
                float wfs = 12.0f * imGuiCustom::g_fontScale;
                char detBuf[128]; snprintf(detBuf, sizeof(detBuf), "Detected: %s", variables::Aimbot::detected_weapon_name.c_str());
                wdl->AddText(wfont, wfs, ImVec2(base.x + 12.0f, base.y + ay), imGuiCustom::ColorU32(imGuiCustom::GetTheme().Text), detBuf);
                ay += imGuiCustom::CheckStep();
                const FallenWeaponBallistics* ball = nullptr;
                if (variables::Aimbot::selected_weapon_index > 0 && variables::Aimbot::selected_weapon_index < (int)FALLEN_WEAPON_LIST.size())
                    ball = fallen_get_weapon_data(FALLEN_WEAPON_LIST[variables::Aimbot::selected_weapon_index]);
                else if (!variables::Aimbot::detected_weapon_name.empty() && variables::Aimbot::detected_weapon_name != "None")
                    ball = fallen_get_weapon_data(variables::Aimbot::detected_weapon_name);
                if (ball) {
                    char bvBuf[128]; snprintf(bvBuf, sizeof(bvBuf), "BV: %.0f  Gravity: %.2f", ball->bv, ball->grav);
                    wdl->AddText(wfont, wfs, ImVec2(base.x + 12.0f, base.y + ay), imGuiCustom::ColorU32(imGuiCustom::GetTheme().Text), bvBuf);
                    ay += imGuiCustom::CheckStep();
                }
            }
            imGuiCustom::SliderFloat("fallen_bv", &variables::Aimbot::fallen_bv_override, 0.0f, 3000.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "Bullet Vel Override", "%.0f");
            ay += imGuiCustom::SliderTop() + 15.0f;
        }
    }
    imGuiCustom::Checkbox("Include NPC", &variables::Aimbot::includeNPC, ImVec2(12.0f, ay));
    ay += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Silent Tracer", &variables::Aimbot::silentTracer, ImVec2(12.0f, ay));
    imGuiCustom::ColorSquare("tracer_color", &variables::Aimbot::silentTracerColor, ImVec2(264.0f, ay + 1.0f));
    ay += imGuiCustom::CheckStep();
    if (variables::Aimbot::silentTracer) {
        imGuiCustom::SliderFloat("tracer_thick", &variables::Aimbot::silentTracerThickness, 0.5f, 5.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "Tracer Thickness", "%.1f");
        ay += imGuiCustom::SliderTop() + 15.0f;
    }
    imGuiCustom::Checkbox("Prediction Line", &variables::Aimbot::predictionLine, ImVec2(12.0f, ay));
    imGuiCustom::ColorSquare("predline_color", &variables::Aimbot::predictionLineColor, ImVec2(264.0f, ay + 1.0f));
    ay += imGuiCustom::CheckStep();
    if (variables::Aimbot::predictionLine) {
        imGuiCustom::SliderFloat("predline_thick", &variables::Aimbot::predictionLineThickness, 0.5f, 5.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "Line Thickness", "%.1f");
        ay += imGuiCustom::SliderTop() + 15.0f;
    }
    float by = 46.0f;
    imGuiCustom::Checkbox("Visible Check", &variables::Aimbot::visibleCheck, ImVec2(311.0f, by));
    by += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Deadzone", &variables::Aimbot::useDeadzone, ImVec2(311.0f, by));
    by += imGuiCustom::CheckStep();
    if (variables::Aimbot::useDeadzone) {
        imGuiCustom::SliderFloat("deadzone", &variables::Aimbot::deadzone, 0.0f, 50.0f, ImVec2(311.0f, by + imGuiCustom::SliderTop()), 272.0f, "Deadzone", "%.0f");
        by += imGuiCustom::SliderTop() + 15.0f;
    }
    imGuiCustom::Checkbox("Triggerbot", &variables::Aimbot::triggerbot, ImVec2(311.0f, by));
    imGuiCustom::Keybind("trigger_key", &variables::Aimbot::triggerKey, ImVec2(505.0f, by - 1.0f), ImVec2(68.0f, 13.0f), nullptr);
    by += imGuiCustom::CheckStep();
    if (variables::Aimbot::triggerbot) {
        float trigDelayMs = (float)variables::Aimbot::triggerDelay;
        imGuiCustom::SliderFloat("trigger_delay", &trigDelayMs, 0.0f, 500.0f, ImVec2(311.0f, by + imGuiCustom::SliderTop()), 272.0f, "Trigger Delay", "%.0f");
        variables::Aimbot::triggerDelay = (int)trigDelayMs;
        by += imGuiCustom::SliderTop() + 15.0f;
    }
}

void RenderVisualMenu() {
    
    ImVec2 vBase = ImGui::GetWindowPos();
    ImVec2 vLMin = vBase + ImVec2(6.0f,40.0f);
    ImVec2 vLMax = vBase + ImVec2(6.0f+290.0f,40.0f+340.0f);
    ImVec2 vMp = ImGui::GetIO().MousePos;
    bool vHover = (vMp.x>=vLMin.x && vMp.x<=vLMax.x && vMp.y>=vLMin.y && vMp.y<=vLMax.y) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
    if (vHover) {
        float wh = ImGui::GetIO().MouseWheel;
        if (wh!=0.f && !imGuiCustom::PopupBlocking()) variables::ESP::visualScroll -= wh * 22.0f;
    }
    
    float vContent = 0;
    vContent += imGuiCustom::CheckStep()*2; 
    if(variables::ESP::boxes){ vContent += imGuiCustom::ComboStep() + imGuiCustom::CheckStep()*2; if(variables::ESP::boxFilled) vContent += imGuiCustom::CheckStep() + (variables::ESP::boxFillGradient?imGuiCustom::CheckStep():0); }
    vContent += imGuiCustom::CheckStep()*4; 
    vContent += imGuiCustom::CheckStep(); 
    if(variables::ESP::flags) vContent += imGuiCustom::ComboStep()+imGuiCustom::ComboTop();
    vContent += imGuiCustom::CheckStep(); 
    if(variables::ESP::headDot) vContent += imGuiCustom::SliderTop()+15.0f;
    vContent += imGuiCustom::CheckStep(); 
    if(variables::ESP::viewDirection) vContent += imGuiCustom::SliderTop()+15.0f;
    vContent += imGuiCustom::CheckStep(); 
    if(variables::ESP::skeleton) vContent += imGuiCustom::SliderTop()+15.0f;
    vContent += 10;
    float vMaxScroll = vContent - 340.0f + 6.0f; if(vMaxScroll<0) vMaxScroll=0;
    if(variables::ESP::visualScroll<0) variables::ESP::visualScroll=0;
    if(variables::ESP::visualScroll>vMaxScroll) variables::ESP::visualScroll=vMaxScroll;
    ImDrawList* vFg = ImGui::GetWindowDrawList();
    vFg->PushClipRect(vLMin, vLMax, true);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
    float ly = 46.0f - variables::ESP::visualScroll;
    imGuiCustom::Checkbox("Enable ESP", &variables::ESP::enabled, ImVec2(12.0f, ly));
    ly += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Boxes", &variables::ESP::boxes, ImVec2(12.0f, ly));
    imGuiCustom::ColorSquare("box_color", &variables::ESP::boxColor, ImVec2(264.0f, ly + 1.0f));
    ly += imGuiCustom::CheckStep();
        if (variables::ESP::boxes) {
            const char* boxModes[] = {"Static", "Dynamic"};
            imGuiCustom::Combo("box_mode", &variables::ESP::boxMode, boxModes, 2, ImVec2(12.0f, ly + imGuiCustom::ComboTop()), 158.0f, "Box Mode:");
            ly += imGuiCustom::ComboTop() + 22.0f;
        imGuiCustom::Checkbox("Box Filled", &variables::ESP::boxFilled, ImVec2(12.0f, ly));
        imGuiCustom::ColorSquare("box_fill_color", &variables::ESP::boxFillColor, ImVec2(264.0f, ly + 1.0f));
        ly += imGuiCustom::CheckStep();
        if (variables::ESP::boxFilled) {
            imGuiCustom::Checkbox("Fill Gradient", &variables::ESP::boxFillGradient, ImVec2(12.0f, ly));
            if (variables::ESP::boxFillGradient)
                imGuiCustom::ColorSquare("box_fill_color2", &variables::ESP::boxFillColor2, ImVec2(264.0f, ly + 1.0f));
            ly += imGuiCustom::CheckStep();
        }
    }
    imGuiCustom::Checkbox("Names", &variables::ESP::names, ImVec2(12.0f, ly));
    imGuiCustom::ColorSquare("name_color", &variables::ESP::nameColor, ImVec2(264.0f, ly + 1.0f));
    ly += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Distance", &variables::ESP::distance, ImVec2(12.0f, ly));
    imGuiCustom::ColorSquare("distance_color", &variables::ESP::distanceColor, ImVec2(264.0f, ly + 1.0f));
    ly += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Health Bar", &variables::ESP::healthBar, ImVec2(12.0f, ly));
    imGuiCustom::ColorSquare("health_color", &variables::ESP::healthColor, ImVec2(264.0f, ly + 1.0f));
    ly += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Tool", &variables::ESP::tool, ImVec2(12.0f, ly));
    imGuiCustom::ColorSquare("tool_color", &variables::ESP::toolColor, ImVec2(264.0f, ly + 1.0f));
    ly += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Flags", &variables::ESP::flags, ImVec2(12.0f, ly));
    imGuiCustom::ColorSquare("flags_color", &variables::ESP::flagsColor, ImVec2(264.0f, ly + 1.0f));
    ly += imGuiCustom::CheckStep();
    if (variables::ESP::flags) {
        const char* flagItems[] = {"State", "Rig", "Health", "Tool", "Distance", "Velocity", "Team", "Role"};
        ly += imGuiCustom::ComboTop();
        imGuiCustom::MultiCombo("flags_sel", variables::ESP::flagSel, flagItems, 8, ImVec2(12.0f, ly), 158.0f, "Flags:");
        ly += imGuiCustom::ComboStep();
    }
    imGuiCustom::Checkbox("Head Dot", &variables::ESP::headDot, ImVec2(12.0f, ly));
    imGuiCustom::ColorSquare("headdot_color", &variables::ESP::headDotColor, ImVec2(264.0f, ly + 1.0f));
    ly += imGuiCustom::CheckStep();
    if (variables::ESP::headDot) {
        imGuiCustom::SliderFloat("headdot_size", &variables::ESP::headDotSize, 1.0f, 10.0f, ImVec2(12.0f, ly + imGuiCustom::SliderTop()), 272.0f, "Dot Size", "%.0f");
        ly += imGuiCustom::SliderTop() + 15.0f;
    }
    imGuiCustom::Checkbox("View Direction", &variables::ESP::viewDirection, ImVec2(12.0f, ly));
    imGuiCustom::ColorSquare("viewdir_color", &variables::ESP::viewDirColor, ImVec2(264.0f, ly + 1.0f));
    ly += imGuiCustom::CheckStep();
    if (variables::ESP::viewDirection) {
        imGuiCustom::SliderFloat("viewdir_len", &variables::ESP::viewDirLength, 1.0f, 30.0f, ImVec2(12.0f, ly + imGuiCustom::SliderTop()), 272.0f, "Length", "%.0f");
        ly += imGuiCustom::SliderTop() + 15.0f;
    }
    imGuiCustom::Checkbox("Skeleton", &variables::ESP::skeleton, ImVec2(12.0f, ly));
    imGuiCustom::ColorSquare("skeleton_color", &variables::ESP::skeletonColor, ImVec2(264.0f, ly + 1.0f));
    ly += imGuiCustom::CheckStep();
    if (variables::ESP::skeleton) {
        imGuiCustom::SliderFloat("skeleton_thick", &variables::ESP::skeletonThickness, 0.5f, 5.0f, ImVec2(12.0f, ly + imGuiCustom::SliderTop()), 272.0f, "Skeleton Thickness", "%.1f");
        ly += imGuiCustom::SliderTop() + 15.0f;
    }
    vFg->PopClipRect();
    ImGui::PopStyleVar();
    float sy = 46.0f;
    imGuiCustom::Checkbox("Dead Check", &variables::ESP::deadCheck, ImVec2(311.0f, sy));
    sy += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Local Player", &variables::ESP::localPlayer, ImVec2(311.0f, sy));
    sy += imGuiCustom::CheckStep();
    
    {
        ImVec2 base = ImGui::GetWindowPos();
        ImVec2 pMin = base + ImVec2(305.0f,133.0f);
        ImVec2 pMax = base + ImVec2(305.0f+290.0f,133.0f+247.0f);
        ImVec2 mp = ImGui::GetIO().MousePos;
        bool hoverPanel = (mp.x>=pMin.x && mp.x<=pMax.x && mp.y>=pMin.y && mp.y<=pMax.y) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        if (hoverPanel) {
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel!=0.f && !imGuiCustom::PopupBlocking()) {
                variables::World::worldScroll -= wheel * 22.0f;
            }
        }
        
        auto contentH = [&]()->float{
            float h=0; h+=imGuiCustom::CheckStep(); 
            if(variables::World::enabled){ h+=imGuiCustom::CheckStep()*2; }
            h+=imGuiCustom::CheckStep(); 
            if(variables::World::ores) h+=imGuiCustom::ComboStep()+imGuiCustom::ComboTop();
            h+=imGuiCustom::CheckStep(); 
            if(variables::World::plants) h+=imGuiCustom::ComboStep()+imGuiCustom::ComboTop();
            h+=imGuiCustom::CheckStep(); 
            if(variables::World::animals) h+=imGuiCustom::ComboStep()+imGuiCustom::ComboTop()+imGuiCustom::CheckStep()*2;
            h+=imGuiCustom::CheckStep(); 
            if(variables::World::soldiers) h+=imGuiCustom::ComboStep()+imGuiCustom::ComboTop()+imGuiCustom::CheckStep()*2;
            h+=imGuiCustom::CheckStep(); 
            if(variables::World::tools) h+=imGuiCustom::ComboStep()+imGuiCustom::ComboTop()+imGuiCustom::CheckStep();
            return h + 10.0f;
        }();
        float maxScroll = contentH - 247.0f + 6.0f;
        if (maxScroll < 0) maxScroll = 0;
        if (variables::World::worldScroll < 0) variables::World::worldScroll = 0;
        if (variables::World::worldScroll > maxScroll) variables::World::worldScroll = maxScroll;
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
        ImDrawList* fg = ImGui::GetWindowDrawList();
        fg->PushClipRect(pMin, pMax, true);
        
        float wy = 139.0f - variables::World::worldScroll;
        
        ImGui::GetStyle().ScrollbarSize = 0.0f;
        imGuiCustom::Checkbox("Enable world##world_enabled", &variables::World::enabled, ImVec2(311.0f, wy));
        wy += imGuiCustom::CheckStep();
        if (variables::World::enabled) {
            imGuiCustom::Checkbox("Name##world_name", &variables::World::name, ImVec2(311.0f, wy));
            wy += imGuiCustom::CheckStep();
            imGuiCustom::Checkbox("Distance##world_distance", &variables::World::distance, ImVec2(311.0f, wy));
            wy += imGuiCustom::CheckStep();
        }
        imGuiCustom::Checkbox("Ores##world_ores", &variables::World::ores, ImVec2(311.0f, wy));
        wy += imGuiCustom::CheckStep();
        if (variables::World::ores) {
            const char* oreItems[] = {"Stone", "Phosphate", "Metal"};
            wy += imGuiCustom::ComboTop();
            imGuiCustom::MultiCombo("world_ores_sel##world_ores", variables::World::oresSel, oreItems, 3, ImVec2(311.0f, wy), 158.0f, "Ores:");
            wy += imGuiCustom::ComboStep();
        }
        imGuiCustom::Checkbox("Plants##world_plants_cb", &variables::World::plants, ImVec2(311.0f, wy));
        wy += imGuiCustom::CheckStep();
        if (variables::World::plants) {
            const char* plantItems[] = {"Wool Plant", "Blueberry Plant", "Raspberry Plant", "Lemon Plant", "Corn Plant", "Pumpkin Plant", "Tomato Plant"};
            wy += imGuiCustom::ComboTop();
            imGuiCustom::MultiCombo("world_plants_sel##world_plants", variables::World::plantsSel, plantItems, 7, ImVec2(311.0f, wy), 158.0f, "Plants:");
            wy += imGuiCustom::ComboStep();
        }
        imGuiCustom::Checkbox("Animals##world_animals", &variables::World::animals, ImVec2(311.0f, wy));
        wy += imGuiCustom::CheckStep();
        if (variables::World::animals) {
            const char* animalItems[] = {"Deer", "WildBoar", "Wolf"};
            wy += imGuiCustom::ComboTop();
            imGuiCustom::MultiCombo("world_animals_sel##world_animals", variables::World::animalsSel, animalItems, 3, ImVec2(311.0f, wy), 158.0f, "Animals:");
            wy += imGuiCustom::ComboStep();
            imGuiCustom::Checkbox("Box##world_animals_box", &variables::World::animalsBox, ImVec2(311.0f, wy));
            wy += imGuiCustom::CheckStep();
            imGuiCustom::Checkbox("Health##world_animals_health", &variables::World::animalsHealth, ImVec2(311.0f, wy));
            wy += imGuiCustom::CheckStep();
        }
        imGuiCustom::Checkbox("Soldiers##world_soldiers", &variables::World::soldiers, ImVec2(311.0f, wy));
        wy += imGuiCustom::CheckStep();
        if (variables::World::soldiers) {
            const char* soldierItems[] = {"Boris", "Bruno", "Brutus", "Soldier"};
            wy += imGuiCustom::ComboTop();
            imGuiCustom::MultiCombo("world_soldiers_sel##world_soldiers", variables::World::soldiersSel, soldierItems, 4, ImVec2(311.0f, wy), 158.0f, "Soldiers:");
            wy += imGuiCustom::ComboStep();
            imGuiCustom::Checkbox("Box##world_soldiers_box", &variables::World::soldiersBox, ImVec2(311.0f, wy));
            wy += imGuiCustom::CheckStep();
            imGuiCustom::Checkbox("Health##world_soldiers_health", &variables::World::soldiersHealth, ImVec2(311.0f, wy));
            wy += imGuiCustom::CheckStep();
        }
        imGuiCustom::Checkbox("Tools##world_tools", &variables::World::tools, ImVec2(311.0f, wy));
        wy += imGuiCustom::CheckStep();
        if (variables::World::tools) {
            const char* toolItems[] = {"Base Cabinet","Small Storage Box","Large Storage Box","Anvil","Furnace","Storage Cabinet","Sleeping Bag"};
            wy += imGuiCustom::ComboTop();
            imGuiCustom::MultiCombo("world_tools_sel##world_tools", variables::World::toolsSel, toolItems, 7, ImVec2(311.0f, wy), 158.0f, "Tools:");
            wy += imGuiCustom::ComboStep();
            imGuiCustom::Checkbox("3D Box##world_tools_box", &variables::World::toolsBox, ImVec2(311.0f, wy));
            wy += imGuiCustom::CheckStep();
        }
        fg->PopClipRect();
        ImGui::PopStyleVar();
    }
}

void RenderSettingsMenu() {
    float gy = 46.0f;
    imGuiCustom::Checkbox("VSync", &variables::Misc::vsync, ImVec2(12.0f, gy));
    gy += imGuiCustom::CheckStep();
    float fps = (float)variables::Misc::fpsLimit;
    imGuiCustom::SliderFloat("fps_limit", &fps, 0.0f, 240.0f, ImVec2(12.0f, gy + imGuiCustom::SliderTop()), 272.0f, fps < 0.5f ? "FPS Limit: Unlimited" : "FPS Limit", "%.0f");
    gy += imGuiCustom::SliderTop() + 15.0f;
    int snapped = (int)(fps + 0.5f);
    if (snapped > 0 && snapped < 60)
        snapped = 60;
    variables::Misc::fpsLimit = snapped;
    const char* priorities[] = {"Low", "Normal", "High", "Realtime"};
    gy += imGuiCustom::ComboTop();
    if (imGuiCustom::Combo("priority", &variables::Misc::priority, priorities, 4, ImVec2(12.0f, gy), 158.0f, "Priority:")) {
        DWORD cls = NORMAL_PRIORITY_CLASS;
        if (variables::Misc::priority == 0)
            cls = IDLE_PRIORITY_CLASS;
        else if (variables::Misc::priority == 2)
            cls = HIGH_PRIORITY_CLASS;
        else if (variables::Misc::priority == 3)
            cls = REALTIME_PRIORITY_CLASS;
        SetPriorityClass(GetCurrentProcess(), cls);
    }
    gy += imGuiCustom::ComboStep();
    
    {
        ImDrawList* cdl = ImGui::GetWindowDrawList();
        ImVec2 base = ImGui::GetWindowPos();
        ImFont* cfont = imGuiCustom::GetFonts().CascadiaMonoBL ? imGuiCustom::GetFonts().CascadiaMonoBL : ImGui::GetFont();
        float cfs = 12.0f * imGuiCustom::g_fontScale;
        cdl->AddText(cfont, cfs, ImVec2(base.x+12.0f, base.y+gy+2.0f), imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright), "Config");
        gy += 18.0f;
        static char cfgName[64] = "default";
        auto getDesktop = []()->std::string{
            char* up = nullptr; size_t len=0; _dupenv_s(&up,&len,"USERPROFILE");
            std::string p = up ? std::string(up) : std::string("C:\\Users\\Public");
            if(up) free(up);
            p += "\\Desktop\\";
            return p;
        };
        auto buildStr = []()->std::string{
            std::ostringstream o;
            o<<"Aimbot.enabled="<<(variables::Aimbot::enabled?1:0)<<"\n";
            o<<"Aimbot.fovRadius="<<variables::Aimbot::fovRadius<<"\n";
            o<<"Aimbot.smoothing="<<variables::Aimbot::smoothing<<"\n";
            o<<"Aimbot.aimTarget="<<variables::Aimbot::aimTarget<<"\n";
            o<<"Aimbot.aimbotKey="<<variables::Aimbot::aimbotKey<<"\n";
            o<<"Aimbot.fallen_prediction="<<(variables::Aimbot::fallen_prediction?1:0)<<"\n";
            o<<"Aimbot.selected_weapon_index="<<variables::Aimbot::selected_weapon_index<<"\n";
            o<<"Aimbot.fallen_bv_override="<<variables::Aimbot::fallen_bv_override<<"\n";
            o<<"Aimbot.includeNPC="<<(variables::Aimbot::includeNPC?1:0)<<"\n";
            o<<"Aimbot.predictionLine="<<(variables::Aimbot::predictionLine?1:0)<<"\n";
            o<<"ESP.enabled="<<(variables::ESP::enabled?1:0)<<"\n";
            o<<"ESP.boxes="<<(variables::ESP::boxes?1:0)<<"\n";
            o<<"ESP.names="<<(variables::ESP::names?1:0)<<"\n";
            o<<"ESP.distance="<<(variables::ESP::distance?1:0)<<"\n";
            o<<"ESP.healthBar="<<(variables::ESP::healthBar?1:0)<<"\n";
            o<<"ESP.skeleton="<<(variables::ESP::skeleton?1:0)<<"\n";
            o<<"World.enabled="<<(variables::World::enabled?1:0)<<"\n";
            o<<"World.name="<<(variables::World::name?1:0)<<"\n";
            o<<"World.distance="<<(variables::World::distance?1:0)<<"\n";
            o<<"World.ores="<<(variables::World::ores?1:0)<<"\n";
            for(int i=0;i<3;++i) o<<"World.oresSel"<<i<<"="<<(variables::World::oresSel[i]?1:0)<<"\n";
            o<<"World.plants="<<(variables::World::plants?1:0)<<"\n";
            for(int i=0;i<7;++i) o<<"World.plantsSel"<<i<<"="<<(variables::World::plantsSel[i]?1:0)<<"\n";
            o<<"World.animals="<<(variables::World::animals?1:0)<<"\n";
            for(int i=0;i<3;++i) o<<"World.animalsSel"<<i<<"="<<(variables::World::animalsSel[i]?1:0)<<"\n";
            o<<"World.soldiers="<<(variables::World::soldiers?1:0)<<"\n";
            for(int i=0;i<4;++i) o<<"World.soldiersSel"<<i<<"="<<(variables::World::soldiersSel[i]?1:0)<<"\n";
            o<<"World.tools="<<(variables::World::tools?1:0)<<"\n";
            for(int i=0;i<7;++i) o<<"World.toolsSel"<<i<<"="<<(variables::World::toolsSel[i]?1:0)<<"\n";
            o<<"World.toolsBox="<<(variables::World::toolsBox?1:0)<<"\n";
            o<<"World.animalsBox="<<(variables::World::animalsBox?1:0)<<"\n";
            o<<"World.soldiersBox="<<(variables::World::soldiersBox?1:0)<<"\n";
            return o.str();
        };
        auto parseStr = [](const std::string& s){
            std::istringstream iss(s); std::string line;
            auto getVal = [](const std::string& l)->std::string{ auto p=l.find('='); return p==std::string::npos?"":l.substr(p+1); };
            while(std::getline(iss,line)){
                if(line.rfind("Aimbot.enabled=",0)==0) variables::Aimbot::enabled = stoi(getVal(line))!=0;
                else if(line.rfind("Aimbot.fovRadius=",0)==0) variables::Aimbot::fovRadius = stof(getVal(line));
                else if(line.rfind("Aimbot.smoothing=",0)==0) variables::Aimbot::smoothing = stof(getVal(line));
                else if(line.rfind("Aimbot.aimTarget=",0)==0) variables::Aimbot::aimTarget = stoi(getVal(line));
                else if(line.rfind("Aimbot.aimbotKey=",0)==0) variables::Aimbot::aimbotKey = stoi(getVal(line));
                else if(line.rfind("Aimbot.fallen_prediction=",0)==0) variables::Aimbot::fallen_prediction = stoi(getVal(line))!=0;
                else if(line.rfind("Aimbot.selected_weapon_index=",0)==0) variables::Aimbot::selected_weapon_index = stoi(getVal(line));
                else if(line.rfind("Aimbot.fallen_bv_override=",0)==0) variables::Aimbot::fallen_bv_override = stof(getVal(line));
                else if(line.rfind("Aimbot.includeNPC=",0)==0) variables::Aimbot::includeNPC = stoi(getVal(line))!=0;
                else if(line.rfind("Aimbot.predictionLine=",0)==0) variables::Aimbot::predictionLine = stoi(getVal(line))!=0;
                else if(line.rfind("ESP.enabled=",0)==0) variables::ESP::enabled = stoi(getVal(line))!=0;
                else if(line.rfind("ESP.boxes=",0)==0) variables::ESP::boxes = stoi(getVal(line))!=0;
                else if(line.rfind("ESP.names=",0)==0) variables::ESP::names = stoi(getVal(line))!=0;
                else if(line.rfind("ESP.distance=",0)==0) variables::ESP::distance = stoi(getVal(line))!=0;
                else if(line.rfind("ESP.healthBar=",0)==0) variables::ESP::healthBar = stoi(getVal(line))!=0;
                else if(line.rfind("ESP.skeleton=",0)==0) variables::ESP::skeleton = stoi(getVal(line))!=0;
                else if(line.rfind("World.enabled=",0)==0) variables::World::enabled = stoi(getVal(line))!=0;
                else if(line.rfind("World.name=",0)==0) variables::World::name = stoi(getVal(line))!=0;
                else if(line.rfind("World.distance=",0)==0) variables::World::distance = stoi(getVal(line))!=0;
                else if(line.rfind("World.ores=",0)==0) variables::World::ores = stoi(getVal(line))!=0;
                else if(line.rfind("World.oresSel",0)==0){ int idx=line[12]-'0'; if(idx>=0&&idx<3) variables::World::oresSel[idx]=stoi(getVal(line))!=0; }
                else if(line.rfind("World.plants=",0)==0) variables::World::plants = stoi(getVal(line))!=0;
                else if(line.rfind("World.plantsSel",0)==0){ int idx=line[14]-'0'; if(idx>=0&&idx<7) variables::World::plantsSel[idx]=stoi(getVal(line))!=0; }
                else if(line.rfind("World.animals=",0)==0) variables::World::animals = stoi(getVal(line))!=0;
                else if(line.rfind("World.animalsSel",0)==0){ int idx=line[15]-'0'; if(idx>=0&&idx<3) variables::World::animalsSel[idx]=stoi(getVal(line))!=0; }
                else if(line.rfind("World.soldiers=",0)==0) variables::World::soldiers = stoi(getVal(line))!=0;
                else if(line.rfind("World.soldiersSel",0)==0){ int idx=line[16]-'0'; if(idx>=0&&idx<4) variables::World::soldiersSel[idx]=stoi(getVal(line))!=0; }
                else if(line.rfind("World.tools=",0)==0) variables::World::tools = stoi(getVal(line))!=0;
                else if(line.rfind("World.toolsSel",0)==0){ int idx=line[12]-'0'; if(idx>=0&&idx<7) variables::World::toolsSel[idx]=stoi(getVal(line))!=0; }
                else if(line.rfind("World.toolsBox=",0)==0) variables::World::toolsBox = stoi(getVal(line))!=0;
                else if(line.rfind("World.animalsBox=",0)==0) variables::World::animalsBox = stoi(getVal(line))!=0;
                else if(line.rfind("World.soldiersBox=",0)==0) variables::World::soldiersBox = stoi(getVal(line))!=0;
            }
        };
        ImVec2 inpPos = base + ImVec2(12.0f, gy + imGuiCustom::g_contentOffset.y);
        ImGui::SetCursorScreenPos(inpPos);
        ImGui::PushItemWidth(158.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,2));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, imGuiCustom::ColorU32(imGuiCustom::GetTheme().ControlBg));
        ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright));
        ImGui::PushStyleColor(ImGuiCol_Border, imGuiCustom::ColorU32(ImVec4(0,0,0,0)));
        ImGui::InputText("##cfgName", cfgName, sizeof(cfgName));
        ImGui::PopStyleColor(3); ImGui::PopStyleVar(); ImGui::PopItemWidth();
        {
            ImDrawList* idl = ImGui::GetWindowDrawList();
            ImVec2 bmin = ImGui::GetItemRectMin(), bmax = ImGui::GetItemRectMax();
            idl->AddRect(bmin, bmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
            idl->AddRect(bmin+ImVec2(1,1), bmax-ImVec2(1,1), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
        }
        gy += 24.0f;
        
        {
            static std::vector<std::string> cfgFiles;
            static std::vector<const char*> cfgPtrs;
            static int cfgSel = 0;
            static double lastScan = 0;
            double now = ImGui::GetTime();
            if (now - lastScan > 1.0) {
                lastScan = now;
                cfgFiles.clear();
                std::string desk = getDesktop();
                namespace fs = std::filesystem;
                try {
                    for (auto &p : fs::directory_iterator(desk)) {
                        if (p.is_regular_file() && p.path().extension()==".config") {
                            cfgFiles.push_back(p.path().stem().string());
                        }
                    }
                } catch(...) {}
                cfgPtrs.clear(); for(auto &s: cfgFiles) cfgPtrs.push_back(s.c_str());
                if(cfgSel >= (int)cfgPtrs.size()) cfgSel = 0;
            }
            if (!cfgPtrs.empty()) {
                gy += imGuiCustom::ComboTop();
                if (imGuiCustom::Combo("cfg_select##cfg_select", &cfgSel, cfgPtrs.data(), (int)cfgPtrs.size(), ImVec2(12.0f, gy), 158.0f, "Configs:")) {
                    strncpy_s(cfgName, sizeof(cfgName), cfgPtrs[cfgSel], _TRUNCATE);
                }
                gy += imGuiCustom::ComboStep();
            } else {
                gy += 2.0f;
            }
        }
        auto cfgBtn = [&](const char* label, ImVec2 off){
            ImVec2 p = base + off + imGuiCustom::g_contentOffset;
            ImGui::SetCursorScreenPos(p);
            ImGui::PushStyleColor(ImGuiCol_Button, imGuiCustom::ColorU32(imGuiCustom::GetTheme().ControlBg));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imGuiCustom::ColorU32(imGuiCustom::GetTheme().ControlInactive));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, imGuiCustom::ColorU32(imGuiCustom::GetTheme().ControlInactive));
            ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright));
            bool r = ImGui::Button(label, ImVec2(75.0f,18.0f));
            ImDrawList* bdl = ImGui::GetWindowDrawList(); ImVec2 bmin=ImGui::GetItemRectMin(), bmax=ImGui::GetItemRectMax();
            bdl->AddRect(bmin,bmax,imGuiCustom::OutlineBlack(),0.0f,0,1.0f); bdl->AddRect(bmin+ImVec2(1,1),bmax-ImVec2(1,1),imGuiCustom::OutlineInner(),0.0f,0,1.0f);
            ImGui::PopStyleColor(4);
            return r;
        };
        std::string path = getDesktop() + std::string(cfgName) + ".config";
        if(cfgBtn("Save", ImVec2(12.0f, gy))){
            std::ofstream f(path); f<<buildStr(); printf("[Config] saved %s\n", path.c_str());
        }
        if(cfgBtn("Load", ImVec2(95.0f, gy))){
            std::ifstream f(path); if(f){ std::stringstream ss; ss<<f.rdbuf(); parseStr(ss.str()); printf("[Config] loaded %s\n", path.c_str()); } else printf("[Config] not found %s\n", path.c_str());
        }
        gy += 22.0f;
        if(cfgBtn("Export", ImVec2(12.0f, gy))){
            ImGui::SetClipboardText(buildStr().c_str()); printf("[Config] exported to clipboard\n");
        }
        if(cfgBtn("Import", ImVec2(95.0f, gy))){
            const char* cb = ImGui::GetClipboardText(); if(cb){ parseStr(std::string(cb)); printf("[Config] imported from clipboard\n"); }
        }
        gy += 26.0f;
    }
    float hy = 46.0f;
    imGuiCustom::SliderFloat("menu_font", &variables::Misc::menuFontSize, 0.8f, 1.5f, ImVec2(311.0f, hy + imGuiCustom::SliderTop()), 272.0f, "Menu Font Size", "%.2f");
    if (variables::Misc::menuFontSize < 0.8f)
        variables::Misc::menuFontSize = 0.8f;
    if (variables::Misc::menuFontSize > 1.5f)
        variables::Misc::menuFontSize = 1.5f;
    hy += imGuiCustom::SliderTop() + 15.0f;
    imGuiCustom::SliderFloat("esp_font", &variables::Misc::espFontSize, 10.0f, 20.0f, ImVec2(311.0f, hy + imGuiCustom::SliderTop()), 272.0f, "ESP Font Size", "%.0f");
    if (variables::Misc::espFontSize < 10.0f)
        variables::Misc::espFontSize = 10.0f;
    if (variables::Misc::espFontSize > 20.0f)
        variables::Misc::espFontSize = 20.0f;
    hy += imGuiCustom::SliderTop() + 15.0f;
    float ty = hy + 6.0f;
    auto themeRow = [&](const char* id, const char* label, ImVec4* col) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 base = ImGui::GetWindowPos();
        ImFont* font = imGuiCustom::GetFonts().CascadiaMonoBL ? imGuiCustom::GetFonts().CascadiaMonoBL : ImGui::GetFont();
        const float fs = 12.0f * imGuiCustom::g_fontScale;
        dl->AddText(font, fs, ImVec2(base.x + 311.0f, base.y + ty), imGuiCustom::ColorU32(imGuiCustom::GetTheme().Text), label);
        imGuiCustom::ColorSquare(id, col, ImVec2(563.0f, ty + 1.0f));
        ty += imGuiCustom::CheckStep();
    };
    themeRow("theme_bg", "Background", &variables::Theme::background);
    themeRow("theme_panels", "Panels", &variables::Theme::panels);
    themeRow("theme_controls", "Controls", &variables::Theme::controls);
    themeRow("theme_accent", "Accent", &variables::Theme::accent);
    themeRow("theme_text", "Text", &variables::Theme::text);
    themeRow("theme_textbright", "Text Bright", &variables::Theme::textBright);
    ImGui::SetCursorScreenPos(ImGui::GetWindowPos() + ImVec2(311.0f, ty));
    ImGui::PushStyleColor(ImGuiCol_Button, imGuiCustom::ColorU32(imGuiCustom::GetTheme().ControlBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, imGuiCustom::ColorU32(imGuiCustom::GetTheme().ControlInactive));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, imGuiCustom::ColorU32(imGuiCustom::GetTheme().ControlInactive));
    ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright));
    if (ImGui::Button("Reset Theme", ImVec2(158.0f, 18.0f))) {
        variables::Theme::background = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
        variables::Theme::panels = ImVec4(0.1529f, 0.1529f, 0.1529f, 1.0f);
        variables::Theme::controls = ImVec4(0.1843f, 0.1843f, 0.1843f, 1.0f);
        variables::Theme::accent = ImVec4(0.3490f, 0.8118f, 0.8275f, 1.0f);
        variables::Theme::text = ImVec4(0.7600f, 0.7600f, 0.7600f, 1.0f);
        variables::Theme::textBright = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    {
        ImDrawList* bdl = ImGui::GetWindowDrawList();
        const ImVec2 bmin = ImGui::GetItemRectMin();
        const ImVec2 bmax = ImGui::GetItemRectMax();
        bdl->AddRect(bmin, bmax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);
        bdl->AddRect(bmin + ImVec2(1.0f, 1.0f), bmax - ImVec2(1.0f, 1.0f), imGuiCustom::OutlineInner(), 0.0f, 0, 1.0f);
    }
    ImGui::PopStyleColor(4);
}
}
