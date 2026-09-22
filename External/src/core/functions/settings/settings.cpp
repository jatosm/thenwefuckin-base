#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "settings.h"
#include "../../variables/variables.h"
#include "../../../render/menu/library.h"
#include "../../../render/render.h"
#include "../aim/fallen_prediction.h"
#include "../explorer/explorer.h"
#include "../players_widget.h"
#include "../backpack_widget.h"
#include "../../features/mesh/chams/MeshChams.h"
#include "../../features/mesh/shader/MeshDxShader.h"
#include <windows.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <functional>
#include <unordered_map>

namespace Settings {
void RenderAimMenu() {
    ImVec2 aBase = ImGui::GetWindowPos();
    ImVec2 aLMin = aBase + ImVec2(6.0f,40.0f);
    ImVec2 aLMax = aBase + ImVec2(6.0f+290.0f,40.0f+340.0f);
    ImVec2 aRMin = aBase + ImVec2(305.0f,40.0f);
    ImVec2 aRMax = aBase + ImVec2(305.0f+290.0f,40.0f+340.0f);
    ImVec2 aMp = ImGui::GetIO().MousePos;
    bool aHoverL = (aMp.x>=aLMin.x && aMp.x<=aLMax.x && aMp.y>=aLMin.y && aMp.y<=aLMax.y) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
    bool aHoverR = (aMp.x>=aRMin.x && aMp.x<=aRMax.x && aMp.y>=aRMin.y && aMp.y<=aRMax.y) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
    static float aimScrollL = 0.f, aimScrollR = 0.f;
    if (aHoverL) {
        float wh = ImGui::GetIO().MouseWheel;
        if (wh!=0.f && !imGuiCustom::PopupBlocking()) aimScrollL -= wh * 22.0f;
    }
    if (aHoverR) {
        float wh = ImGui::GetIO().MouseWheel;
        if (wh!=0.f && !imGuiCustom::PopupBlocking()) aimScrollR -= wh * 22.0f;
    }
    if (aimScrollL < 0.f) aimScrollL = 0.f;
    if (aimScrollR < 0.f) aimScrollR = 0.f;
    ImDrawList* aFg = ImGui::GetWindowDrawList();
    aFg->PushClipRect(aLMin, aLMax, true);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
    float ay = 46.0f - aimScrollL;
    imGuiCustom::Checkbox("Enable Aimbot", &variables::Aimbot::enabled, ImVec2(12.0f, ay));
    imGuiCustom::Keybind("aim_key", &variables::Aimbot::aimbotKey, ImVec2(222.0f, ay - 1.0f), ImVec2(68.0f, 13.0f), &variables::Aimbot::aimbotKeyMode);
    ay += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Show FOV", &variables::Aimbot::showFOV, ImVec2(12.0f, ay));
    imGuiCustom::ColorSquare("fov_color", &variables::Aimbot::fovColor, ImVec2(264.0f, ay + 1.0f));
    ay += imGuiCustom::CheckStep();
    imGuiCustom::SliderFloat("fov_radius", &variables::Aimbot::fovRadius, 10.0f, 500.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "FOV Radius", "%.0f");
    ay += imGuiCustom::SliderTop() + 15.0f;
    if (variables::Aimbot::showFOV) {
        imGuiCustom::Checkbox("Fill FOV", &variables::Aimbot::fillFov, ImVec2(12.0f, ay));
        imGuiCustom::ColorSquare("fov_fill_color", &variables::Aimbot::fovFillColor, ImVec2(264.0f, ay + 1.0f));
        ay += imGuiCustom::CheckStep();
    }
    imGuiCustom::Checkbox("360 Mode", &variables::Aimbot::mode360, ImVec2(12.0f, ay));
    imGuiCustom::Keybind("mode360_key", &variables::Aimbot::mode360Key, ImVec2(222.0f, ay - 1.0f), ImVec2(68.0f, 13.0f), &variables::Aimbot::mode360KeyMode);
    ay += imGuiCustom::CheckStep();
    imGuiCustom::SliderFloat("smoothing", &variables::Aimbot::smoothing, 1.0f, 20.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "Smoothing", "%.1f");
    ay += imGuiCustom::SliderTop() + 15.0f;
    const char* targets[] = {"Head", "Torso", "Left Arm", "Right Arm", "Left Leg", "Right Leg", "HumanoidRootPart", "Closest"};
    ay += imGuiCustom::ComboTop();
    imGuiCustom::Combo("aim_target", &variables::Aimbot::aimTarget, targets, 8, ImVec2(12.0f, ay), 158.0f, "Aim Target:");
    ay += imGuiCustom::ComboStep();
    imGuiCustom::Checkbox("Auto Switch Target", &variables::Aimbot::autoSwitch, ImVec2(12.0f, ay));
    ay += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Sticky Aim", &variables::Aimbot::stickyAim, ImVec2(12.0f, ay));
    ay += imGuiCustom::CheckStep();
    const char* methods[] = {"Memory", "Viewport", "Raycast", "PF Silent"};
    imGuiCustom::Combo("aim_method", &variables::Aimbot::aimMethod, methods, 4, ImVec2(12.0f, ay + imGuiCustom::ComboTop()), 158.0f, "Aim Method:");
    ay += imGuiCustom::ComboTop() + 22.0f;
    imGuiCustom::Checkbox("Magic Bullet", &variables::Aimbot::magicBullet, ImVec2(12.0f, ay));
    ay += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Prediction", &variables::Aimbot::prediction, ImVec2(12.0f, ay));
    ay += imGuiCustom::CheckStep();
    if (variables::Aimbot::prediction) {
        fallen_update_weapon_auto();
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
            imGuiCustom::SliderFloat("fallen_bv", &variables::Aimbot::fallen_bv_override, -1.0f, 3000.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "Bullet Vel Override (-1=auto)", "%.0f");
            ay += imGuiCustom::SliderTop() + 15.0f;
            imGuiCustom::SliderFloat("fallen_grav", &variables::Aimbot::fallen_grav_mult, 0.0f, 2.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "Gravity Mult", "%.2f");
            ay += imGuiCustom::SliderTop() + 15.0f;
        }
    }
    imGuiCustom::Checkbox("Global Prediction", &variables::Aimbot::globalPrediction, ImVec2(12.0f, ay));
    ay += imGuiCustom::CheckStep();
    if (variables::Aimbot::globalPrediction) {
        imGuiCustom::SliderFloat("pred_x", &variables::Aimbot::predX, 0.0f, 10.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "Pred X", "%.1f");
        ay += imGuiCustom::SliderTop() + 15.0f;
        imGuiCustom::SliderFloat("pred_y", &variables::Aimbot::predY, 0.0f, 10.0f, ImVec2(12.0f, ay + imGuiCustom::SliderTop()), 272.0f, "Pred Y", "%.1f");
        ay += imGuiCustom::SliderTop() + 15.0f;
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
    float contentL = ay + aimScrollL - 46.0f;
    float maxScrollL = contentL - 340.0f + 6.0f; if (maxScrollL < 0.f) maxScrollL = 0.f;
    if (aimScrollL > maxScrollL) aimScrollL = maxScrollL;
    aFg->PopClipRect();
    aFg->PushClipRect(aRMin, aRMax, true);
    float by = 46.0f - aimScrollR;
    imGuiCustom::Checkbox("Visible Check", &variables::Aimbot::visibleCheck, ImVec2(311.0f, by));
    by += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Knock Check", &variables::Aimbot::knockCheck, ImVec2(311.0f, by));
    by += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Forcefield Check", &variables::Aimbot::forcefieldCheck, ImVec2(311.0f, by));
    by += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Spectate Check", &variables::Aimbot::spectateCheck, ImVec2(311.0f, by));
    by += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Min Health Check", &variables::Aimbot::healthCheck, ImVec2(311.0f, by));
    by += imGuiCustom::CheckStep();
    if (variables::Aimbot::healthCheck) {
        imGuiCustom::SliderFloat("min_health", &variables::Aimbot::minHealth, 0.0f, 100.0f, ImVec2(311.0f, by + imGuiCustom::SliderTop()), 272.0f, "Min Health", "%.0f");
        by += imGuiCustom::SliderTop() + 15.0f;
    }
    imGuiCustom::Checkbox("Spread Modifier", &variables::Aimbot::useSpread, ImVec2(311.0f, by));
    by += imGuiCustom::CheckStep();
    if (variables::Aimbot::useSpread) {
        imGuiCustom::SliderFloat("spread_amount", &variables::Aimbot::spreadModifier, 0.0f, 10.0f, ImVec2(311.0f, by + imGuiCustom::SliderTop()), 272.0f, "Spread Amount", "%.2f");
        by += imGuiCustom::SliderTop() + 15.0f;
        imGuiCustom::SliderFloat("hit_chance", &variables::Aimbot::hitChance, 0.0f, 100.0f, ImVec2(311.0f, by + imGuiCustom::SliderTop()), 272.0f, "Hit Chance", "%.0f%%");
        by += imGuiCustom::SliderTop() + 15.0f;
    }
    imGuiCustom::Checkbox("Deadzone", &variables::Aimbot::useDeadzone, ImVec2(311.0f, by));
    by += imGuiCustom::CheckStep();
    if (variables::Aimbot::useDeadzone) {
        imGuiCustom::SliderFloat("deadzone", &variables::Aimbot::deadzone, 0.0f, 50.0f, ImVec2(311.0f, by + imGuiCustom::SliderTop()), 272.0f, "Deadzone", "%.0f");
        by += imGuiCustom::SliderTop() + 15.0f;
    }
    imGuiCustom::Checkbox("Triggerbot", &variables::Aimbot::triggerbot, ImVec2(311.0f, by));
    imGuiCustom::Keybind("trigger_key", &variables::Aimbot::triggerKey, ImVec2(505.0f, by - 1.0f), ImVec2(68.0f, 13.0f), &variables::Aimbot::triggerKeyMode);
    by += imGuiCustom::CheckStep();
    if (variables::Aimbot::triggerbot) {
        float trigDelayMs = (float)variables::Aimbot::triggerDelay;
        imGuiCustom::SliderFloat("trigger_delay", &trigDelayMs, 0.0f, 500.0f, ImVec2(311.0f, by + imGuiCustom::SliderTop()), 272.0f, "Trigger Delay", "%.0f");
        variables::Aimbot::triggerDelay = (int)trigDelayMs;
        by += imGuiCustom::SliderTop() + 15.0f;
    }
    float contentR = by + aimScrollR - 46.0f;
    float maxScrollR = contentR - 340.0f + 6.0f; if (maxScrollR < 0.f) maxScrollR = 0.f;
    if (aimScrollR > maxScrollR) aimScrollR = maxScrollR;
    aFg->PopClipRect();
    ImGui::PopStyleVar();
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
    vContent += imGuiCustom::CheckStep();
    if(variables::ESP::meshChams) vContent += (imGuiCustom::ComboStep()+imGuiCustom::ComboTop())*3;
    vContent += 0;
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
    imGuiCustom::Checkbox("Mesh Chams", &variables::ESP::meshChams, ImVec2(12.0f, ly));
    ly += imGuiCustom::CheckStep();
    if (variables::ESP::meshChams) {
        static int chamsMeshFlat = 0;
        static int chamsShaderType = 0;
        const char* flatOnly[] = {"Flat"};
        const char* shaderOnly[] = {"Shader"};
        chamsMeshFlat = 0;
        chamsShaderType = 0;
        ly += imGuiCustom::ComboTop();
        imGuiCustom::Combo("chams_mesh", &chamsMeshFlat, flatOnly, 1, ImVec2(12.0f, ly), 158.0f, "Mesh:");
        ly += imGuiCustom::ComboStep();
        ly += imGuiCustom::ComboTop();
        imGuiCustom::Combo("chams_shadertype", &chamsShaderType, shaderOnly, 1, ImVec2(12.0f, ly), 158.0f, "Shaders type:");
        ly += imGuiCustom::ComboStep();
        ly += imGuiCustom::ComboTop();
        imGuiCustom::Combo("chams_shaders", &variables::ESP::meshChamsDxMode, Cheat::Visuals::MeshDxShader::ModeNames(), Cheat::Visuals::MeshDxShader::ModeNameCount(), ImVec2(12.0f, ly), 158.0f, "Shaders:");
        ly += imGuiCustom::ComboStep();
        variables::ESP::meshChamsOccludedDxMode = variables::ESP::meshChamsDxMode;
    }

    vFg->PopClipRect();
    ImGui::PopStyleVar();
    float sy = 46.0f;
    imGuiCustom::Checkbox("Dead Check", &variables::ESP::deadCheck, ImVec2(311.0f, sy));
    sy += imGuiCustom::CheckStep();
    imGuiCustom::Checkbox("Local Player", &variables::ESP::localPlayer, ImVec2(311.0f, sy));

    {
        ImVec2 base = ImGui::GetWindowPos();
        ImVec2 pMin = base + ImVec2(305.0f + imGuiCustom::g_contentOffset.x, 96.0f + imGuiCustom::g_contentOffset.y);
        ImVec2 pMax = pMin + ImVec2(290.0f, 138.0f);
        ImVec2 mp = ImGui::GetIO().MousePos;
        bool hoverPanel = (mp.x >= pMin.x && mp.x <= pMax.x && mp.y >= pMin.y && mp.y <= pMax.y) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        if (hoverPanel) {
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.f && !imGuiCustom::PopupBlocking()) {
                variables::World::worldScroll -= wheel * 22.0f;
            }
        }

        auto contentH = [&]()->float{
            float h = 0; h += imGuiCustom::CheckStep();
            if (variables::World::enabled) { h += imGuiCustom::CheckStep() * 2; }
            h += imGuiCustom::CheckStep();
            if (variables::World::ores) h += imGuiCustom::ComboStep() + imGuiCustom::ComboTop();
            h += imGuiCustom::CheckStep();
            if (variables::World::plants) h += imGuiCustom::ComboStep() + imGuiCustom::ComboTop();
            h += imGuiCustom::CheckStep();
            if (variables::World::animals) h += imGuiCustom::ComboStep() + imGuiCustom::ComboTop() + imGuiCustom::CheckStep() * 2;
            h += imGuiCustom::CheckStep();
            if (variables::World::soldiers) h += imGuiCustom::ComboStep() + imGuiCustom::ComboTop() + imGuiCustom::CheckStep() * 2;
            h += imGuiCustom::CheckStep();
            if (variables::World::tools) h += imGuiCustom::ComboStep() + imGuiCustom::ComboTop() + imGuiCustom::CheckStep();
            return h + 10.0f;
        }();
        float maxScroll = contentH - 138.0f + 6.0f;
        if (maxScroll < 0) maxScroll = 0;
        if (variables::World::worldScroll < 0) variables::World::worldScroll = 0;
        if (variables::World::worldScroll > maxScroll) variables::World::worldScroll = maxScroll;
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
        ImDrawList* fg = ImGui::GetWindowDrawList();
        fg->PushClipRect(pMin + ImVec2(1.0f, 1.0f), pMax - ImVec2(1.0f, 1.0f), true);

        float wy = 102.0f - variables::World::worldScroll;

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
    {
        ImVec2 base = ImGui::GetWindowPos();
        static float s_qContent = 120.0f;
        ImVec2 qMin = base + ImVec2(305.0f + imGuiCustom::g_contentOffset.x, 242.0f + imGuiCustom::g_contentOffset.y);
        ImVec2 qMax = qMin + ImVec2(290.0f, 138.0f);
        ImVec2 mp = ImGui::GetIO().MousePos;
        bool qHover = (mp.x >= qMin.x && mp.x <= qMax.x && mp.y >= qMin.y && mp.y <= qMax.y) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        if (qHover) {
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.f && !imGuiCustom::PopupBlocking()) {
                variables::World::lightScroll -= wheel * 22.0f;
            }
        }
        float qMaxScroll = s_qContent - 138.0f + 6.0f;
        if (qMaxScroll < 0) qMaxScroll = 0;
        if (variables::World::lightScroll < 0) variables::World::lightScroll = 0;
        if (variables::World::lightScroll > qMaxScroll) variables::World::lightScroll = qMaxScroll;
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
        ImDrawList* qg = ImGui::GetWindowDrawList();
        qg->PushClipRect(qMin + ImVec2(1.0f, 1.0f), qMax - ImVec2(1.0f, 1.0f), true);
        float qy = 248.0f - variables::World::lightScroll;
        ImGui::GetStyle().ScrollbarSize = 0.0f;
        auto qLabel = [&](const char* text, float rowY) {
            ImFont* lfont = imGuiCustom::GetFonts().CascadiaMonoBL ? imGuiCustom::GetFonts().CascadiaMonoBL : ImGui::GetFont();
            float lfs = 12.0f * imGuiCustom::g_fontScale;
            qg->AddText(lfont, lfs, ImVec2(base.x + 311.0f + imGuiCustom::g_contentOffset.x, base.y + rowY + 2.0f + imGuiCustom::g_contentOffset.y), imGuiCustom::ColorU32(imGuiCustom::GetTheme().Text), text);
        };
        imGuiCustom::Checkbox("Clock Time##light_clock", &variables::World::clockTimeEnabled, ImVec2(311.0f, qy));
        qy += imGuiCustom::CheckStep();
        if (variables::World::clockTimeEnabled) {
            imGuiCustom::SliderFloat("light_clocktime", &variables::World::clockTimeValue, 0.0f, 24.0f, ImVec2(311.0f, qy + imGuiCustom::SliderTop()), 272.0f, "Time", "%.1f");
            qy += imGuiCustom::SliderTop() + 15.0f;
        }
        imGuiCustom::Checkbox("Brightness##light_bright", &variables::World::brightnessEnabled, ImVec2(311.0f, qy));
        qy += imGuiCustom::CheckStep();
        if (variables::World::brightnessEnabled) {
            imGuiCustom::SliderFloat("light_brightness", &variables::World::brightnessValue, 0.0f, 5.0f, ImVec2(311.0f, qy + imGuiCustom::SliderTop()), 272.0f, "Brightness", "%.2f");
            qy += imGuiCustom::SliderTop() + 15.0f;
        }
        imGuiCustom::Checkbox("Ambient##light_amb", &variables::World::ambientEnabled, ImVec2(311.0f, qy));
        qy += imGuiCustom::CheckStep();
        if (variables::World::ambientEnabled) {
            qLabel("Ambient", qy);
            imGuiCustom::ColorSquare("light_ambientcolor", &variables::World::ambientColor, ImVec2(573.0f, qy + 1.0f));
            qy += imGuiCustom::CheckStep();
            qLabel("Outdoor", qy);
            imGuiCustom::ColorSquare("light_outdoorcolor", &variables::World::outdoorAmbientColor, ImVec2(573.0f, qy + 1.0f));
            qy += imGuiCustom::CheckStep();
        }
        imGuiCustom::Checkbox("Fog##light_fog", &variables::World::fogEnabled, ImVec2(311.0f, qy));
        qy += imGuiCustom::CheckStep();
        if (variables::World::fogEnabled) {
            qLabel("Fog", qy);
            imGuiCustom::ColorSquare("light_fogcolor", &variables::World::fogColor, ImVec2(573.0f, qy + 1.0f));
            qy += imGuiCustom::CheckStep();
            imGuiCustom::SliderFloat("light_fogstart", &variables::World::fogStart, 0.0f, 10000.0f, ImVec2(311.0f, qy + imGuiCustom::SliderTop()), 272.0f, "Fog Start", "%.0f");
            qy += imGuiCustom::SliderTop() + 15.0f;
            imGuiCustom::SliderFloat("light_fogend", &variables::World::fogEnd, 0.0f, 100000.0f, ImVec2(311.0f, qy + imGuiCustom::SliderTop()), 272.0f, "Fog End", "%.0f");
            qy += imGuiCustom::SliderTop() + 15.0f;
        }
        imGuiCustom::Checkbox("Exposure##light_exp", &variables::World::exposureEnabled, ImVec2(311.0f, qy));
        qy += imGuiCustom::CheckStep();
        if (variables::World::exposureEnabled) {
            imGuiCustom::SliderFloat("light_exposure", &variables::World::exposureValue, -5.0f, 5.0f, ImVec2(311.0f, qy + imGuiCustom::SliderTop()), 272.0f, "Exposure", "%.2f");
            qy += imGuiCustom::SliderTop() + 15.0f;
        }
        imGuiCustom::Checkbox("Skybox##light_sky", &variables::World::skyboxEnabled, ImVec2(311.0f, qy));
        qy += imGuiCustom::CheckStep();
        if (variables::World::skyboxEnabled) {
            const char* skyPresets[] = {"Clear", "Custom"};
            qy += imGuiCustom::ComboTop();
            imGuiCustom::Combo("light_skypreset", &variables::World::skyboxPreset, skyPresets, 2, ImVec2(311.0f, qy), 158.0f, "Preset:");
            qy += imGuiCustom::ComboStep();
            if (variables::World::skyboxPreset == 1) {
                static char skyIds[1024] = {};
                ImVec2 wpos = ImGui::GetWindowPos();
                ImGui::PushID("light_skyids");
                ImGui::SetCursorScreenPos(ImVec2(wpos.x + 311.0f + imGuiCustom::g_contentOffset.x, wpos.y + qy + imGuiCustom::g_contentOffset.y));
                ImGui::PushItemWidth(272.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, imGuiCustom::ColorU32(imGuiCustom::GetTheme().ControlBg));
                ImGui::PushStyleColor(ImGuiCol_Text, imGuiCustom::ColorU32(imGuiCustom::GetTheme().TextBright));
                ImGui::PushStyleColor(ImGuiCol_Border, imGuiCustom::ColorU32(ImVec4(0, 0, 0, 0)));
                ImGui::InputTextWithHint("##in", "ID, or 6 IDs separated by commas", skyIds, sizeof(skyIds));
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();
                ImGui::PopItemWidth();
                ImDrawList* idl = ImGui::GetWindowDrawList();
                const ImVec2 bmin = ImGui::GetItemRectMin(), bmax = ImGui::GetItemRectMax();
                idl->AddRect(bmin, bmax, imGuiCustom::OutlineBlack(), 0.f, 0, 1.f);
                idl->AddRect(bmin + ImVec2(1, 1), bmax - ImVec2(1, 1), imGuiCustom::OutlineInner(), 0.f, 0, 1.f);
                ImGui::PopID();
                qy += 24.0f;
                std::vector<std::string> toks;
                std::string cur;
                for (const char* p = skyIds; *p; ++p) {
                    char ch = *p;
                    if (ch == ',' || ch == ';' || ch == '\n' || ch == ' ') {
                        if (!cur.empty()) { toks.push_back(cur); cur.clear(); }
                    } else {
                        cur.push_back(ch);
                    }
                }
                if (!cur.empty()) toks.push_back(cur);
                if (toks.size() == 1) {
                    variables::World::skyIdBk = toks[0];
                    variables::World::skyIdDn = toks[0];
                    variables::World::skyIdFt = toks[0];
                    variables::World::skyIdLf = toks[0];
                    variables::World::skyIdRt = toks[0];
                    variables::World::skyIdUp = toks[0];
                } else {
                    variables::World::skyIdBk = toks.size() > 0 ? toks[0] : "";
                    variables::World::skyIdDn = toks.size() > 1 ? toks[1] : "";
                    variables::World::skyIdFt = toks.size() > 2 ? toks[2] : "";
                    variables::World::skyIdLf = toks.size() > 3 ? toks[3] : "";
                    variables::World::skyIdRt = toks.size() > 4 ? toks[4] : "";
                    variables::World::skyIdUp = toks.size() > 5 ? toks[5] : "";
                }
            }
        }
        s_qContent = qy + variables::World::lightScroll - 248.0f + 10.0f;
        qg->PopClipRect();
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
        if (imGuiCustom::ButtonPos("Explorer##open_exp", ImVec2(12.0f, gy), ImVec2(158.0f, 20.0f), false))
            Explorer::SetOpen(!Explorer::IsOpen());
        gy += 26.0f;
        if (imGuiCustom::ButtonPos("Players##open_players", ImVec2(12.0f, gy), ImVec2(158.0f, 20.0f), false))
            PlayersWidget::SetOpen(!PlayersWidget::IsOpen());
        gy += 26.0f;
        if (imGuiCustom::ButtonPos("Hotbar##open_back", ImVec2(12.0f, gy), ImVec2(158.0f, 20.0f), false))
            BackpackWidget::SetOpen(!BackpackWidget::IsOpen());
        gy += 26.0f;
    }

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
            auto b = [&](const char* k, bool v){ o<<k<<"="<<(v?1:0)<<"\n"; };
            auto i = [&](const char* k, int v){ o<<k<<"="<<v<<"\n"; };
            auto f = [&](const char* k, float v){ o<<k<<"="<<v<<"\n"; };
            auto c = [&](const char* k, const ImVec4& v){ o<<k<<"="<<v.x<<","<<v.y<<","<<v.z<<","<<v.w<<"\n"; };
            auto s = [&](const char* k, const std::string& v){ o<<k<<"="<<v<<"\n"; };
            b("teamCheck", variables::teamCheck);
            i("teamCheckKey", variables::teamCheckKey);
            i("teamCheckKeyMode", variables::teamCheckKeyMode);
            b("Aimbot.enabled", variables::Aimbot::enabled);
            b("Aimbot.showFOV", variables::Aimbot::showFOV);
            f("Aimbot.fovRadius", variables::Aimbot::fovRadius);
            f("Aimbot.smoothing", variables::Aimbot::smoothing);
            i("Aimbot.aimTarget", variables::Aimbot::aimTarget);
            i("Aimbot.aimMethod", variables::Aimbot::aimMethod);
            b("Aimbot.magicBullet", variables::Aimbot::magicBullet);
            i("Aimbot.aimbotKey", variables::Aimbot::aimbotKey);
            i("Aimbot.aimbotKeyMode", variables::Aimbot::aimbotKeyMode);
            c("Aimbot.fovColor", variables::Aimbot::fovColor);
            b("Aimbot.visibleCheck", variables::Aimbot::visibleCheck);
            b("Aimbot.playerPreview", variables::Aimbot::playerPreview);
            b("Aimbot.useDeadzone", variables::Aimbot::useDeadzone);
            f("Aimbot.deadzone", variables::Aimbot::deadzone);
            b("Aimbot.useSpread", variables::Aimbot::useSpread);
            f("Aimbot.spreadModifier", variables::Aimbot::spreadModifier);
            f("Aimbot.hitChance", variables::Aimbot::hitChance);
            b("Aimbot.triggerbot", variables::Aimbot::triggerbot);
            i("Aimbot.triggerKey", variables::Aimbot::triggerKey);
            i("Aimbot.triggerKeyMode", variables::Aimbot::triggerKeyMode);
            i("Aimbot.triggerDelay", variables::Aimbot::triggerDelay);
            b("Aimbot.prediction", variables::Aimbot::prediction);
            b("Aimbot.fallen_prediction", variables::Aimbot::fallen_prediction);
            f("Aimbot.fallen_bv_override", variables::Aimbot::fallen_bv_override);
            f("Aimbot.fallen_grav_mult", variables::Aimbot::fallen_grav_mult);
            i("Aimbot.selected_weapon_index", variables::Aimbot::selected_weapon_index);
            b("Aimbot.includeNPC", variables::Aimbot::includeNPC);
            b("Aimbot.silentTracer", variables::Aimbot::silentTracer);
            c("Aimbot.silentTracerColor", variables::Aimbot::silentTracerColor);
            f("Aimbot.silentTracerThickness", variables::Aimbot::silentTracerThickness);
            b("Aimbot.predictionLine", variables::Aimbot::predictionLine);
            c("Aimbot.predictionLineColor", variables::Aimbot::predictionLineColor);
            f("Aimbot.predictionLineThickness", variables::Aimbot::predictionLineThickness);
            b("ESP.enabled", variables::ESP::enabled);
            b("ESP.boxes", variables::ESP::boxes);
            i("ESP.boxMode", variables::ESP::boxMode);
            c("ESP.boxColor", variables::ESP::boxColor);
            b("ESP.boxFilled", variables::ESP::boxFilled);
            b("ESP.boxFillGradient", variables::ESP::boxFillGradient);
            c("ESP.boxFillColor", variables::ESP::boxFillColor);
            c("ESP.boxFillColor2", variables::ESP::boxFillColor2);
            b("ESP.names", variables::ESP::names);
            c("ESP.nameColor", variables::ESP::nameColor);
            b("ESP.distance", variables::ESP::distance);
            c("ESP.distanceColor", variables::ESP::distanceColor);
            b("ESP.healthBar", variables::ESP::healthBar);
            c("ESP.healthColor", variables::ESP::healthColor);
            b("ESP.skeleton", variables::ESP::skeleton);
            c("ESP.skeletonColor", variables::ESP::skeletonColor);
            f("ESP.skeletonThickness", variables::ESP::skeletonThickness);
            b("ESP.skeletonOutline", variables::ESP::skeletonOutline);
            b("ESP.meshChams", variables::ESP::meshChams);
            c("ESP.chamsFillColor", variables::ESP::chamsFillColor);
            c("ESP.meshChamsOccludedColor", variables::ESP::meshChamsOccludedColor);
            i("ESP.meshChamsDxMode", variables::ESP::meshChamsDxMode);
            i("ESP.meshChamsOccludedDxMode", variables::ESP::meshChamsOccludedDxMode);
            b("ESP.deadCheck", variables::ESP::deadCheck);
            b("ESP.localPlayer", variables::ESP::localPlayer);
            b("ESP.tool", variables::ESP::tool);
            c("ESP.toolColor", variables::ESP::toolColor);
            b("ESP.flags", variables::ESP::flags);
            c("ESP.flagsColor", variables::ESP::flagsColor);
            for(int k=0;k<8;++k) b((std::string("ESP.flagSel")+std::to_string(k)).c_str(), variables::ESP::flagSel[k]);
            b("ESP.headDot", variables::ESP::headDot);
            c("ESP.headDotColor", variables::ESP::headDotColor);
            f("ESP.headDotSize", variables::ESP::headDotSize);
            b("ESP.viewDirection", variables::ESP::viewDirection);
            c("ESP.viewDirColor", variables::ESP::viewDirColor);
            f("ESP.viewDirLength", variables::ESP::viewDirLength);
            b("Local.jumpEnabled", variables::Local::jumpEnabled);
            f("Local.jumpPower", variables::Local::jumpPower);
            i("Local.jumpKey", variables::Local::jumpKey);
            i("Local.jumpKeyMode", variables::Local::jumpKeyMode);
            b("Misc.streamProof", variables::Misc::streamProof);
            i("Misc.streamKey", variables::Misc::streamKey);
            i("Misc.streamKeyMode", variables::Misc::streamKeyMode);
            b("Misc.watermark", variables::Misc::watermark);
            i("Misc.watermarkKey", variables::Misc::watermarkKey);
            i("Misc.watermarkKeyMode", variables::Misc::watermarkKeyMode);
            b("Misc.keybinds", variables::Misc::keybinds);
            i("Misc.keybindsKey", variables::Misc::keybindsKey);
            i("Misc.keybindsKeyMode", variables::Misc::keybindsKeyMode);
            b("Misc.vsync", variables::Misc::vsync);
            i("Misc.fpsLimit", variables::Misc::fpsLimit);
            i("Misc.priority", variables::Misc::priority);
            b("Misc.walkSpeed", variables::Misc::walkSpeed);
            f("Misc.walkSpeedValue", variables::Misc::walkSpeedValue);
            b("Misc.gravity", variables::Misc::gravity);
            f("Misc.gravityValue", variables::Misc::gravityValue);
            f("Misc.menuFontSize", variables::Misc::menuFontSize);
            f("Misc.espFontSize", variables::Misc::espFontSize);
            b("Movement.fov", variables::Movement::fov);
            i("Movement.fovKey", variables::Movement::fovKey);
            i("Movement.fovKeyMode", variables::Movement::fovKeyMode);
            f("Movement.fovValue", variables::Movement::fovValue);
            b("Movement.fly", variables::Movement::fly);
            i("Movement.flyKey", variables::Movement::flyKey);
            i("Movement.flyKeyMode", variables::Movement::flyKeyMode);
            i("Movement.flyMethod", variables::Movement::flyMethod);
            f("Movement.flySpeed", variables::Movement::flySpeed);
            f("Movement.flyVerticalBoost", variables::Movement::flyVerticalBoost);
            f("Movement.flyDamping", variables::Movement::flyDamping);
            b("Movement.noclip", variables::Movement::noclip);
            i("Movement.noclipKey", variables::Movement::noclipKey);
            i("Movement.noclipKeyMode", variables::Movement::noclipKeyMode);
            i("Movement.noclipMode", variables::Movement::noclipMode);
            b("Movement.bunnyHop", variables::Movement::bunnyHop);
            i("Movement.bunnyHopKey", variables::Movement::bunnyHopKey);
            i("Movement.bunnyHopKeyMode", variables::Movement::bunnyHopKeyMode);
            f("Movement.bunnyHopSpeed", variables::Movement::bunnyHopSpeed);
            b("Movement.hipHeight", variables::Movement::hipHeight);
            f("Movement.hipHeightValue", variables::Movement::hipHeightValue);
            b("World.enabled", variables::World::enabled);
            b("World.name", variables::World::name);
            b("World.distance", variables::World::distance);
            b("World.ores", variables::World::ores);
            for(int k=0;k<3;++k) b((std::string("World.oresSel")+std::to_string(k)).c_str(), variables::World::oresSel[k]);
            for(int k=0;k<3;++k) c((std::string("World.oresColor")+std::to_string(k)).c_str(), variables::World::oresColor[k]);
            b("World.plants", variables::World::plants);
            for(int k=0;k<7;++k) b((std::string("World.plantsSel")+std::to_string(k)).c_str(), variables::World::plantsSel[k]);
            for(int k=0;k<7;++k) c((std::string("World.plantsColor")+std::to_string(k)).c_str(), variables::World::plantsColor[k]);
            b("World.animals", variables::World::animals);
            for(int k=0;k<3;++k) b((std::string("World.animalsSel")+std::to_string(k)).c_str(), variables::World::animalsSel[k]);
            for(int k=0;k<3;++k) c((std::string("World.animalsColor")+std::to_string(k)).c_str(), variables::World::animalsColor[k]);
            b("World.animalsBox", variables::World::animalsBox);
            b("World.animalsHealth", variables::World::animalsHealth);
            b("World.soldiers", variables::World::soldiers);
            for(int k=0;k<4;++k) b((std::string("World.soldiersSel")+std::to_string(k)).c_str(), variables::World::soldiersSel[k]);
            for(int k=0;k<4;++k) c((std::string("World.soldiersColor")+std::to_string(k)).c_str(), variables::World::soldiersColor[k]);
            b("World.soldiersBox", variables::World::soldiersBox);
            b("World.soldiersHealth", variables::World::soldiersHealth);
            c("World.murderColor", variables::World::murderColor);
            c("World.sheriffColor", variables::World::sheriffColor);
            c("World.innocentColor", variables::World::innocentColor);
            b("World.tools", variables::World::tools);
            for(int k=0;k<7;++k) b((std::string("World.toolsSel")+std::to_string(k)).c_str(), variables::World::toolsSel[k]);
            for(int k=0;k<7;++k) c((std::string("World.toolsColor")+std::to_string(k)).c_str(), variables::World::toolsColor[k]);
            b("World.toolsBox", variables::World::toolsBox);
            b("World.wireframe", variables::World::wireframe);
            b("World.clockTimeEnabled", variables::World::clockTimeEnabled);
            f("World.clockTimeValue", variables::World::clockTimeValue);
            b("World.brightnessEnabled", variables::World::brightnessEnabled);
            f("World.brightnessValue", variables::World::brightnessValue);
            b("World.ambientEnabled", variables::World::ambientEnabled);
            c("World.ambientColor", variables::World::ambientColor);
            c("World.outdoorAmbientColor", variables::World::outdoorAmbientColor);
            b("World.fogEnabled", variables::World::fogEnabled);
            f("World.fogStart", variables::World::fogStart);
            f("World.fogEnd", variables::World::fogEnd);
            c("World.fogColor", variables::World::fogColor);
            b("World.exposureEnabled", variables::World::exposureEnabled);
            f("World.exposureValue", variables::World::exposureValue);
            b("World.skyboxEnabled", variables::World::skyboxEnabled);
            i("World.skyboxPreset", variables::World::skyboxPreset);
            s("World.skyIdBk", variables::World::skyIdBk);
            s("World.skyIdDn", variables::World::skyIdDn);
            s("World.skyIdFt", variables::World::skyIdFt);
            s("World.skyIdLf", variables::World::skyIdLf);
            s("World.skyIdRt", variables::World::skyIdRt);
            s("World.skyIdUp", variables::World::skyIdUp);
            b("Freecam.enabled", variables::Freecam::enabled);
            i("Freecam.key", variables::Freecam::key);
            i("Freecam.keyMode", variables::Freecam::keyMode);
            f("Freecam.sensitivity", variables::Freecam::sensitivity);
            f("Freecam.speed", variables::Freecam::speed);
            i("Freecam.shiftKey", variables::Freecam::shiftKey);
            f("Freecam.shiftMultiplier", variables::Freecam::shiftMultiplier);
            b("Freecam.azerty", variables::Freecam::azerty);
            b("Freecam.freezeCharacter", variables::Freecam::freezeCharacter);
            c("Theme.background", variables::Theme::background);
            c("Theme.panels", variables::Theme::panels);
            c("Theme.controls", variables::Theme::controls);
            c("Theme.accent", variables::Theme::accent);
            c("Theme.text", variables::Theme::text);
            c("Theme.textBright", variables::Theme::textBright);
            return o.str();
        };
        auto parseStr = [](const std::string& s){
            using Handler = std::function<void(const std::string&)>;
            auto toB = [](const std::string& v)->bool{ return v=="1"||v=="true"; };
            auto toI = [](const std::string& v)->int{ try{ return stoi(v); }catch(...){ return 0; } };
            auto toF = [](const std::string& v)->float{ try{ return stof(v); }catch(...){ return 0.f; } };
            auto toC = [](const std::string& v)->ImVec4{
                ImVec4 r(0,0,0,1); float x,y,z,w;
                if(sscanf_s(v.c_str(), "%f,%f,%f,%f", &x, &y, &z, &w)==4) r = ImVec4(x,y,z,w);
                return r;
            };
            auto clampI = [](int v, int lo, int hi){ return v<lo?lo:(v>hi?hi:v); };
            static const auto table = [&](){
                std::unordered_map<std::string, Handler> m;
                auto B = [&](const char* k, bool& v){ m[k] = [&v,toB](const std::string& s){ v = toB(s); }; };
                auto I = [&](const char* k, int& v){ m[k] = [&v,toI](const std::string& s){ v = toI(s); }; };
                auto Ic = [&](const char* k, int& v, int lo, int hi){ m[k] = [&v,toI,clampI,lo,hi](const std::string& s){ v = clampI(toI(s),lo,hi); }; };
                auto F = [&](const char* k, float& v){ m[k] = [&v,toF](const std::string& s){ v = toF(s); }; };
                auto C = [&](const char* k, ImVec4& v){ m[k] = [&v,toC](const std::string& s){ v = toC(s); }; };
                auto S = [&](const char* k, std::string& v){ m[k] = [&v](const std::string& s){ v = s; }; };
                B("teamCheck", variables::teamCheck);
                I("teamCheckKey", variables::teamCheckKey);
                Ic("teamCheckKeyMode", variables::teamCheckKeyMode, 0, 2);
                B("Aimbot.enabled", variables::Aimbot::enabled);
                B("Aimbot.showFOV", variables::Aimbot::showFOV);
                F("Aimbot.fovRadius", variables::Aimbot::fovRadius);
                F("Aimbot.smoothing", variables::Aimbot::smoothing);
                Ic("Aimbot.aimTarget", variables::Aimbot::aimTarget, 0, 7);
                Ic("Aimbot.aimMethod", variables::Aimbot::aimMethod, 0, 3);
                B("Aimbot.magicBullet", variables::Aimbot::magicBullet);
                I("Aimbot.aimbotKey", variables::Aimbot::aimbotKey);
                Ic("Aimbot.aimbotKeyMode", variables::Aimbot::aimbotKeyMode, 0, 2);
                C("Aimbot.fovColor", variables::Aimbot::fovColor);
                B("Aimbot.visibleCheck", variables::Aimbot::visibleCheck);
                B("Aimbot.playerPreview", variables::Aimbot::playerPreview);
                B("Aimbot.useDeadzone", variables::Aimbot::useDeadzone);
                F("Aimbot.deadzone", variables::Aimbot::deadzone);
                B("Aimbot.useSpread", variables::Aimbot::useSpread);
                F("Aimbot.spreadModifier", variables::Aimbot::spreadModifier);
                F("Aimbot.hitChance", variables::Aimbot::hitChance);
                B("Aimbot.triggerbot", variables::Aimbot::triggerbot);
                I("Aimbot.triggerKey", variables::Aimbot::triggerKey);
                Ic("Aimbot.triggerKeyMode", variables::Aimbot::triggerKeyMode, 0, 2);
                I("Aimbot.triggerDelay", variables::Aimbot::triggerDelay);
                B("Aimbot.prediction", variables::Aimbot::prediction);
                B("Aimbot.fallen_prediction", variables::Aimbot::fallen_prediction);
                F("Aimbot.fallen_bv_override", variables::Aimbot::fallen_bv_override);
                F("Aimbot.fallen_grav_mult", variables::Aimbot::fallen_grav_mult);
                I("Aimbot.selected_weapon_index", variables::Aimbot::selected_weapon_index);
                B("Aimbot.includeNPC", variables::Aimbot::includeNPC);
                B("Aimbot.silentTracer", variables::Aimbot::silentTracer);
                C("Aimbot.silentTracerColor", variables::Aimbot::silentTracerColor);
                F("Aimbot.silentTracerThickness", variables::Aimbot::silentTracerThickness);
                B("Aimbot.predictionLine", variables::Aimbot::predictionLine);
                C("Aimbot.predictionLineColor", variables::Aimbot::predictionLineColor);
                F("Aimbot.predictionLineThickness", variables::Aimbot::predictionLineThickness);
                B("ESP.enabled", variables::ESP::enabled);
                B("ESP.boxes", variables::ESP::boxes);
                Ic("ESP.boxMode", variables::ESP::boxMode, 0, 1);
                C("ESP.boxColor", variables::ESP::boxColor);
                B("ESP.boxFilled", variables::ESP::boxFilled);
                B("ESP.boxFillGradient", variables::ESP::boxFillGradient);
                C("ESP.boxFillColor", variables::ESP::boxFillColor);
                C("ESP.boxFillColor2", variables::ESP::boxFillColor2);
                B("ESP.names", variables::ESP::names);
                C("ESP.nameColor", variables::ESP::nameColor);
                B("ESP.distance", variables::ESP::distance);
                C("ESP.distanceColor", variables::ESP::distanceColor);
                B("ESP.healthBar", variables::ESP::healthBar);
                C("ESP.healthColor", variables::ESP::healthColor);
                B("ESP.skeleton", variables::ESP::skeleton);
                C("ESP.skeletonColor", variables::ESP::skeletonColor);
                F("ESP.skeletonThickness", variables::ESP::skeletonThickness);
                B("ESP.skeletonOutline", variables::ESP::skeletonOutline);
                B("ESP.meshChams", variables::ESP::meshChams);
                C("ESP.chamsFillColor", variables::ESP::chamsFillColor);
                C("ESP.meshChamsOccludedColor", variables::ESP::meshChamsOccludedColor);
                Ic("ESP.meshChamsDxMode", variables::ESP::meshChamsDxMode, 0, 16);
                Ic("ESP.meshChamsOccludedDxMode", variables::ESP::meshChamsOccludedDxMode, 0, 16);
                B("ESP.deadCheck", variables::ESP::deadCheck);
                B("ESP.localPlayer", variables::ESP::localPlayer);
                B("ESP.tool", variables::ESP::tool);
                C("ESP.toolColor", variables::ESP::toolColor);
                B("ESP.flags", variables::ESP::flags);
                C("ESP.flagsColor", variables::ESP::flagsColor);
                B("ESP.headDot", variables::ESP::headDot);
                C("ESP.headDotColor", variables::ESP::headDotColor);
                F("ESP.headDotSize", variables::ESP::headDotSize);
                B("ESP.viewDirection", variables::ESP::viewDirection);
                C("ESP.viewDirColor", variables::ESP::viewDirColor);
                F("ESP.viewDirLength", variables::ESP::viewDirLength);
                B("Local.jumpEnabled", variables::Local::jumpEnabled);
                F("Local.jumpPower", variables::Local::jumpPower);
                I("Local.jumpKey", variables::Local::jumpKey);
                Ic("Local.jumpKeyMode", variables::Local::jumpKeyMode, 0, 2);
                B("Misc.streamProof", variables::Misc::streamProof);
                I("Misc.streamKey", variables::Misc::streamKey);
                Ic("Misc.streamKeyMode", variables::Misc::streamKeyMode, 0, 2);
                B("Misc.watermark", variables::Misc::watermark);
                I("Misc.watermarkKey", variables::Misc::watermarkKey);
                Ic("Misc.watermarkKeyMode", variables::Misc::watermarkKeyMode, 0, 2);
                B("Misc.keybinds", variables::Misc::keybinds);
                I("Misc.keybindsKey", variables::Misc::keybindsKey);
                Ic("Misc.keybindsKeyMode", variables::Misc::keybindsKeyMode, 0, 2);
                B("Misc.vsync", variables::Misc::vsync);
                I("Misc.fpsLimit", variables::Misc::fpsLimit);
                Ic("Misc.priority", variables::Misc::priority, 0, 3);
                B("Misc.walkSpeed", variables::Misc::walkSpeed);
                F("Misc.walkSpeedValue", variables::Misc::walkSpeedValue);
                B("Misc.gravity", variables::Misc::gravity);
                F("Misc.gravityValue", variables::Misc::gravityValue);
                F("Misc.menuFontSize", variables::Misc::menuFontSize);
                F("Misc.espFontSize", variables::Misc::espFontSize);
                B("Movement.fov", variables::Movement::fov);
                I("Movement.fovKey", variables::Movement::fovKey);
                Ic("Movement.fovKeyMode", variables::Movement::fovKeyMode, 0, 2);
                F("Movement.fovValue", variables::Movement::fovValue);
                B("Movement.fly", variables::Movement::fly);
                I("Movement.flyKey", variables::Movement::flyKey);
                Ic("Movement.flyKeyMode", variables::Movement::flyKeyMode, 0, 2);
                I("Movement.flyMethod", variables::Movement::flyMethod);
                F("Movement.flySpeed", variables::Movement::flySpeed);
                F("Movement.flyVerticalBoost", variables::Movement::flyVerticalBoost);
                F("Movement.flyDamping", variables::Movement::flyDamping);
                B("Movement.noclip", variables::Movement::noclip);
                I("Movement.noclipKey", variables::Movement::noclipKey);
                Ic("Movement.noclipKeyMode", variables::Movement::noclipKeyMode, 0, 2);
                Ic("Movement.noclipMode", variables::Movement::noclipMode, 0, 1);
                B("Movement.bunnyHop", variables::Movement::bunnyHop);
                I("Movement.bunnyHopKey", variables::Movement::bunnyHopKey);
                Ic("Movement.bunnyHopKeyMode", variables::Movement::bunnyHopKeyMode, 0, 2);
                F("Movement.bunnyHopSpeed", variables::Movement::bunnyHopSpeed);
                B("Movement.hipHeight", variables::Movement::hipHeight);
                F("Movement.hipHeightValue", variables::Movement::hipHeightValue);
                B("World.enabled", variables::World::enabled);
                B("World.name", variables::World::name);
                B("World.distance", variables::World::distance);
                B("World.ores", variables::World::ores);
                B("World.plants", variables::World::plants);
                B("World.animals", variables::World::animals);
                B("World.animalsBox", variables::World::animalsBox);
                B("World.animalsHealth", variables::World::animalsHealth);
                B("World.soldiers", variables::World::soldiers);
                B("World.soldiersBox", variables::World::soldiersBox);
                B("World.soldiersHealth", variables::World::soldiersHealth);
                C("World.murderColor", variables::World::murderColor);
                C("World.sheriffColor", variables::World::sheriffColor);
                C("World.innocentColor", variables::World::innocentColor);
                B("World.tools", variables::World::tools);
                B("World.toolsBox", variables::World::toolsBox);
                B("World.wireframe", variables::World::wireframe);
                B("World.clockTimeEnabled", variables::World::clockTimeEnabled);
                F("World.clockTimeValue", variables::World::clockTimeValue);
                B("World.brightnessEnabled", variables::World::brightnessEnabled);
                F("World.brightnessValue", variables::World::brightnessValue);
                B("World.ambientEnabled", variables::World::ambientEnabled);
                C("World.ambientColor", variables::World::ambientColor);
                C("World.outdoorAmbientColor", variables::World::outdoorAmbientColor);
                B("World.fogEnabled", variables::World::fogEnabled);
                F("World.fogStart", variables::World::fogStart);
                F("World.fogEnd", variables::World::fogEnd);
                C("World.fogColor", variables::World::fogColor);
                B("World.exposureEnabled", variables::World::exposureEnabled);
                F("World.exposureValue", variables::World::exposureValue);
                B("World.skyboxEnabled", variables::World::skyboxEnabled);
                Ic("World.skyboxPreset", variables::World::skyboxPreset, 0, 1);
                S("World.skyIdBk", variables::World::skyIdBk);
                S("World.skyIdDn", variables::World::skyIdDn);
                S("World.skyIdFt", variables::World::skyIdFt);
                S("World.skyIdLf", variables::World::skyIdLf);
                S("World.skyIdRt", variables::World::skyIdRt);
                S("World.skyIdUp", variables::World::skyIdUp);
                B("Freecam.enabled", variables::Freecam::enabled);
                I("Freecam.key", variables::Freecam::key);
                Ic("Freecam.keyMode", variables::Freecam::keyMode, 0, 2);
                F("Freecam.sensitivity", variables::Freecam::sensitivity);
                F("Freecam.speed", variables::Freecam::speed);
                I("Freecam.shiftKey", variables::Freecam::shiftKey);
                F("Freecam.shiftMultiplier", variables::Freecam::shiftMultiplier);
                B("Freecam.azerty", variables::Freecam::azerty);
                B("Freecam.freezeCharacter", variables::Freecam::freezeCharacter);
                C("Theme.background", variables::Theme::background);
                C("Theme.panels", variables::Theme::panels);
                C("Theme.controls", variables::Theme::controls);
                C("Theme.accent", variables::Theme::accent);
                C("Theme.text", variables::Theme::text);
                C("Theme.textBright", variables::Theme::textBright);
                return m;
            }();
            auto selIdx = [](const std::string& key, const char* pre, int n)->int{
                if(key.rfind(pre,0)!=0 || key.size()!=(std::string(pre).size()+1)) return -1;
                int d = key.back()-'0'; return (d>=0&&d<n)?d:-1;
            };
            auto selB = [&](const std::string& key, const char* pre, bool* arr, int n, const std::string& v){
                int d = selIdx(key, pre, n); if(d>=0) arr[d] = (v=="1"||v=="true");
            };
            auto selC = [&](const std::string& key, const char* pre, ImVec4* arr, int n, const std::string& v){
                int d = selIdx(key, pre, n); if(d>=0) arr[d] = toC(v);
            };
            std::istringstream iss(s); std::string line;
            while(std::getline(iss,line)){
                auto p = line.find('=');
                if(p==std::string::npos || p==0) continue;
                std::string key = line.substr(0,p), val = line.substr(p+1);
                auto it = table.find(key);
                if(it != table.end()){ it->second(val); continue; }
                selB(key, "ESP.flagSel", variables::ESP::flagSel, 8, val);
                selB(key, "World.oresSel", variables::World::oresSel, 3, val);
                selC(key, "World.oresColor", variables::World::oresColor, 3, val);
                selB(key, "World.plantsSel", variables::World::plantsSel, 7, val);
                selC(key, "World.plantsColor", variables::World::plantsColor, 7, val);
                selB(key, "World.animalsSel", variables::World::animalsSel, 3, val);
                selC(key, "World.animalsColor", variables::World::animalsColor, 3, val);
                selB(key, "World.soldiersSel", variables::World::soldiersSel, 4, val);
                selC(key, "World.soldiersColor", variables::World::soldiersColor, 4, val);
                selB(key, "World.toolsSel", variables::World::toolsSel, 7, val);
                selC(key, "World.toolsColor", variables::World::toolsColor, 7, val);
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
            return imGuiCustom::ButtonPos(label, off, ImVec2(75.0f, 20.0f));
        };
        std::string path = getDesktop() + std::string(cfgName) + ".config";
        if(cfgBtn("Save", ImVec2(12.0f, gy))){
            std::ofstream f(path); f<<buildStr();
        }
        if(cfgBtn("Load", ImVec2(95.0f, gy))){
            std::ifstream f(path); if(f){ std::stringstream ss; ss<<f.rdbuf(); parseStr(ss.str()); }
        }
        gy += 22.0f;
        if(cfgBtn("Export", ImVec2(12.0f, gy))){
            ImGui::SetClipboardText(buildStr().c_str());
        }
        if(cfgBtn("Import", ImVec2(95.0f, gy))){
            const char* cb = ImGui::GetClipboardText(); if(cb){ parseStr(std::string(cb)); }
        }
        gy += 26.0f;
    }

    {
        ImDrawList* edl = ImGui::GetWindowDrawList();
        ImVec2 base = ImGui::GetWindowPos();
        ImTextureID warn = OverlayWarnIcon();
        const float iconSz = 14.0f;
        if (warn)
            edl->AddImage(warn, ImVec2(base.x + 12.0f, base.y + gy + 1.0f), ImVec2(base.x + 12.0f + iconSz, base.y + gy + 1.0f + iconSz), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 70, 70, 255));
        imGuiCustom::Checkbox("Wireframe##settings_wireframe", &variables::World::wireframe, ImVec2(warn ? 30.0f : 12.0f, gy));
        gy += imGuiCustom::CheckStep();
        imGuiCustom::Checkbox("Player Preview", &variables::Aimbot::playerPreview, ImVec2(12.0f, gy));
        gy += imGuiCustom::CheckStep();
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
    if (imGuiCustom::ButtonPos("Reset Theme##reset_th", ImVec2(311.0f, ty), ImVec2(158.0f, 20.0f))) {
        variables::Theme::background = ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f);
        variables::Theme::panels = ImVec4(0.1529f, 0.1529f, 0.1529f, 1.0f);
        variables::Theme::controls = ImVec4(0.1843f, 0.1843f, 0.1843f, 1.0f);
        variables::Theme::accent = ImVec4(0.3490f, 0.8118f, 0.8275f, 1.0f);
        variables::Theme::text = ImVec4(0.7600f, 0.7600f, 0.7600f, 1.0f);
        variables::Theme::textBright = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}
}
