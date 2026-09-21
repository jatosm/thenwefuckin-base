#pragma once
#include <windows.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../../../ext/imgui/imgui.h"
#include "../variables/variables.h"

namespace Keys {
inline bool IsKeyPressed(int key) {
    if (key <= 0)
        return false;
    if (key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END) {
        if (ImGui::IsKeyDown((ImGuiKey)key)) return true;
        int vk = 0;
        if (key >= ImGuiKey_A && key <= ImGuiKey_Z) vk = 'A' + (key - ImGuiKey_A);
        else if (key >= ImGuiKey_0 && key <= ImGuiKey_9) vk = '0' + (key - ImGuiKey_0);
        else if (key >= ImGuiKey_F1 && key <= ImGuiKey_F12) vk = VK_F1 + (key - ImGuiKey_F1);
        else if (key == ImGuiKey_Space) vk = VK_SPACE;
        else if (key == ImGuiKey_Tab) vk = VK_TAB;
        else if (key == ImGuiKey_Escape) vk = VK_ESCAPE;
        else if (key == ImGuiKey_Enter) vk = VK_RETURN;
        else if (key == ImGuiKey_LeftShift || key == ImGuiKey_RightShift) vk = VK_SHIFT;
        else if (key == ImGuiKey_LeftCtrl || key == ImGuiKey_RightCtrl) vk = VK_CONTROL;
        else if (key == ImGuiKey_LeftAlt || key == ImGuiKey_RightAlt) vk = VK_MENU;
        else if (key == ImGuiKey_CapsLock) vk = VK_CAPITAL;
        else if (key == ImGuiKey_Backspace) vk = VK_BACK;
        else if (key == ImGuiKey_LeftArrow) vk = VK_LEFT;
        else if (key == ImGuiKey_RightArrow) vk = VK_RIGHT;
        else if (key == ImGuiKey_UpArrow) vk = VK_UP;
        else if (key == ImGuiKey_DownArrow) vk = VK_DOWN;
        if (vk != 0) return (GetAsyncKeyState(vk) & 0x8000) != 0;
        return false;
    }
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

template <typename T>
void ProcessKeybind(int key, int mode, T& feature, bool& last) {
    if (key == 0)
        return;
    const bool down = IsKeyPressed(key);
    if (mode == 2) {
        feature = true;
        last = down;
        return;
    }
    if (down != last) {
        if (mode == 1) {
            if (down)
                feature = !feature;
        } else {
            feature = down;
        }
        last = down;
    }
}

inline bool Gate(int key, int mode, bool& tog, bool& was) {
    if (mode == 2) return true;
    if (key == 0) return true;

    static bool toggle_states[512]{};
    static bool last_states[512]{};
    int idx = key;
    if (key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END) {
        if (key >= ImGuiKey_A && key <= ImGuiKey_Z) idx = 'A' + (key - ImGuiKey_A);
        else if (key >= ImGuiKey_0 && key <= ImGuiKey_9) idx = '0' + (key - ImGuiKey_0);
        else if (key >= ImGuiKey_F1 && key <= ImGuiKey_F12) idx = VK_F1 + (key - ImGuiKey_F1);
        else if (key == ImGuiKey_Space) idx = VK_SPACE;
        else if (key == ImGuiKey_LeftShift || key == ImGuiKey_RightShift) idx = VK_SHIFT;
        else if (key == ImGuiKey_LeftCtrl || key == ImGuiKey_RightCtrl) idx = VK_CONTROL;
        else if (key == ImGuiKey_LeftAlt || key == ImGuiKey_RightAlt) idx = VK_MENU;
        else idx = key % 512;
    }
    if (idx < 0 || idx >= 512) idx = key % 512;
    bool down = IsKeyPressed(key);
    if (mode == 1) {
        if (down && !last_states[idx]) toggle_states[idx] = !toggle_states[idx];
        last_states[idx] = down;
        tog = toggle_states[idx];
        was = last_states[idx];
        return toggle_states[idx];
    }
    last_states[idx] = down;
    was = down;
    tog = down;
    return down;
}

inline bool TeamCheckOn() {
    static bool tog = false;
    static bool was = false;
    if (!variables::teamCheck) {
        tog = false;
        was = false;
        return false;
    }
    return Gate(variables::teamCheckKey, variables::teamCheckKeyMode, tog, was);
}

inline bool StreamProofOn() {
    static bool tog = false;
    static bool was = false;
    if (!variables::Misc::streamProof) {
        tog = false;
        was = false;
        return false;
    }
    return Gate(variables::Misc::streamKey, variables::Misc::streamKeyMode, tog, was);
}

inline bool WatermarkOn() {
    static bool tog = false;
    static bool was = false;
    if (!variables::Misc::watermark) {
        tog = false;
        was = false;
        return false;
    }
    return Gate(variables::Misc::watermarkKey, variables::Misc::watermarkKeyMode, tog, was);
}

inline bool KeybindsOn() {
    static bool tog = false;
    static bool was = false;
    if (!variables::Misc::keybinds) {
        tog = false;
        was = false;
        return false;
    }
    return Gate(variables::Misc::keybindsKey, variables::Misc::keybindsKeyMode, tog, was);
}
}
