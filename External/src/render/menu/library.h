// discord.gg/thenwefuckin
#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../../../ext/imgui/imgui.h"
#include "../../../ext/imgui/imgui_internal.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>

namespace imGuiCustom
{
struct Theme
{
    ImVec4 WindowBg;
    ImVec4 CardBg;
    ImVec4 ControlBg;
    ImVec4 ControlInactive;
    ImVec4 Border;
    ImVec4 Accent;
    ImVec4 AccentText;
    ImVec4 Text;
    ImVec4 TextBright;
    ImVec4 KeybindBg;
};

struct Fonts
{
    ImFont* CascadiaMonoBL = nullptr;
};

inline Theme& GetThemeMutable()
{
    static Theme g_Theme = {
        ImVec4(0.1176f, 0.1176f, 0.1176f, 1.0f),
        ImVec4(0.1529f, 0.1529f, 0.1529f, 1.0f),
        ImVec4(0.1843f, 0.1843f, 0.1843f, 1.0f),
        ImVec4(0.2157f, 0.2157f, 0.2157f, 1.0f),
        ImVec4(0.1608f, 0.1608f, 0.1608f, 1.0f),
        ImVec4(0.3490f, 0.8118f, 0.8275f, 1.0f),
        ImVec4(0.3490f, 0.6314f, 0.6392f, 1.0f),
        ImVec4(0.7600f, 0.7600f, 0.7600f, 1.0f),
        ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
        ImVec4(0.1800f, 0.1800f, 0.1800f, 1.0f),
    };
    return g_Theme;
}

inline const Theme& GetTheme() { return GetThemeMutable(); }

inline Fonts& GetFontsMutable()
{
    static Fonts g_Fonts = {};
    return g_Fonts;
}

inline const Fonts& GetFonts() { return GetFontsMutable(); }

inline void ApplyStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    const Theme& g_Theme = GetTheme();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(0.0f, 0.0f);
    style.FramePadding = ImVec2(0.0f, 0.0f);
    style.ItemSpacing = ImVec2(0.0f, 0.0f);
    style.ItemInnerSpacing = ImVec2(0.0f, 0.0f);
    style.AntiAliasedLines = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill = true;
    style.Colors[ImGuiCol_WindowBg] = g_Theme.WindowBg;
    style.Colors[ImGuiCol_PopupBg] = g_Theme.CardBg;
    style.Colors[ImGuiCol_Text] = g_Theme.Text;
    style.Colors[ImGuiCol_Button] = g_Theme.ControlBg;
    style.Colors[ImGuiCol_ButtonHovered] = g_Theme.ControlInactive;
    style.Colors[ImGuiCol_ButtonActive] = g_Theme.ControlInactive;
    style.Colors[ImGuiCol_Header] = g_Theme.ControlBg;
    style.Colors[ImGuiCol_HeaderHovered] = g_Theme.ControlInactive;
    style.Colors[ImGuiCol_HeaderActive] = g_Theme.Accent;
}

inline void Initialize(ImFont* cascadiaMonoBL)
{
    GetFontsMutable().CascadiaMonoBL = cascadiaMonoBL;
    ApplyStyle();
}

inline float AnimateFloat(ImGuiID id, bool enabled, float speed = 12.0f)
{
    static std::unordered_map<ImGuiID, float> g_Animations;
    ImGuiIO& io = ImGui::GetIO();
    float& value = g_Animations[id];
    const float target = enabled ? 1.0f : 0.0f;
    value = ImLerp(value, target, ImClamp(io.DeltaTime * speed, 0.0f, 1.0f));
    return value;
}

inline ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(ImLerp(a.x, b.x, t), ImLerp(a.y, b.y, t), ImLerp(a.z, b.z, t), ImLerp(a.w, b.w, t));
}

inline ImU32 OutlineBlack() { return ImGui::GetColorU32(IM_COL32(0, 0, 0, 255)); }
inline ImU32 OutlineInner() { return ImGui::GetColorU32(IM_COL32(52, 52, 56, 255)); }

inline ImVec2 g_contentOffset = ImVec2(0.0f, 0.0f);
inline float g_fontScale = 1.0f;
inline float SliderTop() { return 12.0f * g_fontScale + 5.0f; }
inline float SliderStep() { return 12.0f * g_fontScale + 5.0f + 15.0f; }
inline float ComboTop() { return 12.0f * g_fontScale + 3.0f; }
inline float ComboStep() { return 22.0f; }
inline float CheckStep() { return 13.5f * g_fontScale + 5.0f; }

inline ImGuiID& ComboOpenId() { static ImGuiID v = 0; return v; }
inline int& ComboClosedFrame() { static int f = -100000; return f; }
inline bool PopupBlocking() {
    if (ImGui::GetFrameCount() == ComboClosedFrame())
        return true;
    return ComboOpenId() != 0;
}

inline ImU32 ColorU32(const ImVec4& color, float alpha_mul = 1.0f)
{
    ImVec4 c = color;
    c.w *= alpha_mul;
    return ImGui::GetColorU32(c);
}

inline void AddTextWithOutline(ImDrawList* draw_list, ImFont* font, float font_size, const ImVec2& pos, ImU32 text_col, const char* text)
{
    const ImU32 outline_col = OutlineBlack();
    static const ImVec2 offsets[8] = {
        ImVec2(-1.0f, -1.0f), ImVec2(0.0f, -1.0f), ImVec2(1.0f, -1.0f),
        ImVec2(-1.0f,  0.0f),                      ImVec2(1.0f,  0.0f),
        ImVec2(-1.0f,  1.0f), ImVec2(0.0f,  1.0f), ImVec2(1.0f,  1.0f)
    };
    if (font)
    {
        for (int i = 0; i < 8; ++i)
            draw_list->AddText(font, font_size, ImVec2(pos.x + offsets[i].x, pos.y + offsets[i].y), outline_col, text);
        draw_list->AddText(font, font_size, pos, text_col, text);
    }
    else
    {
        for (int i = 0; i < 8; ++i)
            draw_list->AddText(ImVec2(pos.x + offsets[i].x, pos.y + offsets[i].y), outline_col, text);
        draw_list->AddText(pos, text_col, text);
    }
}

inline bool Checkbox(const char* label, bool* value, const ImVec2& pos)
{
    const char* display = label;
    const char* hash = strstr(label, "##");
    std::string displayStr;
    if (hash) displayStr.assign(label, hash - label), display = displayStr.c_str();
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2 base = window->Pos;
    const ImVec2 min = ImVec2(std::floor(base.x + pos.x + g_contentOffset.x), std::floor(base.y + pos.y + g_contentOffset.y));
    const ImVec2 box_size(10.0f, 10.0f);
    ImGui::SetCursorScreenPos(min);
    const ImVec2 text_size = ImGui::CalcTextSize(display);
    ImGui::PushID(label);
    const bool pressed = ImGui::InvisibleButton("##checkbox", ImVec2(20.0f + text_size.x, 13.0f));
    if (pressed && !PopupBlocking())
        *value = !*value;

    const ImGuiID id = ImGui::GetItemID();
    const float check = AnimateFloat(id, *value, 14.0f);
    const Theme& theme = GetTheme();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(min, min + box_size, ColorU32(LerpColor(theme.ControlInactive, theme.Accent, check)), 0.0f);
    draw->AddRect(min, min + box_size, OutlineBlack(), 0.0f, 0, 1.0f);

    const Fonts& fonts = GetFonts();
    ImFont* font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
    ImVec2 _ts = font->CalcTextSizeA(13.5f * g_fontScale, FLT_MAX, 0.0f, display);
    draw->AddText(font, 13.5f * g_fontScale, ImVec2(min.x + 14.0f, min.y + (box_size.y - _ts.y) * 0.5f), ColorU32(theme.Text), display);
    ImGui::PopID();
    return pressed;
}

inline bool ButtonCore(const char* label, const ImVec2& min, const ImVec2& size, bool active = false)
{
    const char* display = label;
    const char* hash = strstr(label, "##");
    std::string displayStr;
    if (hash) displayStr.assign(label, hash - label), display = displayStr.c_str();

    ImGui::PushID(label);
    ImGui::SetCursorScreenPos(min);
    const bool pressed = ImGui::InvisibleButton("##btn", size);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    const ImGuiID id = ImGui::GetItemID();

    const float hover_anim = AnimateFloat(id, hovered || held, 16.0f);
    const float active_anim = AnimateFloat(id + 1, active, 16.0f);

    const Theme& theme = GetTheme();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    ImVec4 bg = LerpColor(theme.ControlBg, theme.ControlInactive, hover_anim * 0.45f);
    if (active_anim > 0.01f) {
        ImVec4 activeBg = LerpColor(bg, theme.Accent, 0.22f);
        bg = LerpColor(bg, activeBg, active_anim);
    }
    draw->AddRectFilled(min, min + size, ColorU32(bg), 0.0f);
    draw->AddRect(min, min + size, OutlineBlack(), 0.0f, 0, 1.0f);

    const ImU32 innerCol = (active_anim > 0.01f)
        ? ColorU32(LerpColor(ImVec4(0.204f, 0.204f, 0.220f, 1.0f), theme.Accent, active_anim))
        : OutlineInner();
    draw->AddRect(min + ImVec2(1.0f, 1.0f), min + size - ImVec2(1.0f, 1.0f), innerCol, 0.0f, 0, 1.0f);

    const Fonts& fonts = GetFonts();
    ImFont* font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
    const float font_size = 12.0f * g_fontScale;
    const ImVec2 text_sz = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, display);
    const ImVec2 text_pos(
        std::floor(min.x + (size.x - text_sz.x) * 0.5f),
        std::floor(min.y + (size.y - text_sz.y) * 0.5f)
    );

    const ImVec4 base_text = active ? theme.Accent : theme.Text;
    const ImVec4 text_col = LerpColor(base_text, theme.TextBright, hover_anim * 0.35f);
    draw->AddText(font, font_size, text_pos, ColorU32(text_col), display);

    ImGui::PopID();
    return pressed && !PopupBlocking();
}

inline bool Button(const char* label, const ImVec2& size = ImVec2(0.0f, 22.0f), bool active = false)
{
    ImVec2 actual_size = size;
    if (actual_size.x <= 0.0f)
        actual_size.x = ImGui::GetContentRegionAvail().x;
    if (actual_size.y <= 0.0f)
        actual_size.y = 22.0f;

    const ImVec2 min = ImVec2(std::floor(ImGui::GetCursorScreenPos().x), std::floor(ImGui::GetCursorScreenPos().y));
    return ButtonCore(label, min, actual_size, active);
}

inline bool ButtonPos(const char* label, const ImVec2& pos, const ImVec2& size = ImVec2(120.0f, 22.0f), bool active = false)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2 base = window->Pos;
    const ImVec2 min = ImVec2(std::floor(base.x + pos.x + g_contentOffset.x), std::floor(base.y + pos.y + g_contentOffset.y));
    return ButtonCore(label, min, size, active);
}

inline bool ParseHexColor(const char* text, ImVec4& out) {
    if (!text)
        return false;
    while (*text == ' ' || *text == '\t')
        ++text;
    if (*text == '#')
        ++text;
    size_t len = 0;
    while (text[len] != '\0' && len < 9)
        ++len;
    if ((len != 6 && len != 8) || text[len] != '\0')
        return false;
    auto hexVal = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    unsigned int v[8];
    for (size_t i = 0; i < len; ++i) {
        int h = hexVal(text[i]);
        if (h < 0)
            return false;
        v[i] = (unsigned int)h;
    }
    out.x = (float)(v[0] * 16 + v[1]) / 255.0f;
    out.y = (float)(v[2] * 16 + v[3]) / 255.0f;
    out.z = (float)(v[4] * 16 + v[5]) / 255.0f;
    out.w = (len == 8) ? (float)(v[6] * 16 + v[7]) / 255.0f : out.w;
    return true;
}

inline bool SliderFloat(const char* label, float* value, float min_value, float max_value, const ImVec2& pos, float width, const char* text_label, const char* format);

inline bool ColorSquare(const char* id_text, ImVec4* color, const ImVec2& pos)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    const ImVec2 base = window->Pos;
    const ImVec2 min = ImVec2(std::floor(base.x + pos.x + g_contentOffset.x), std::floor(base.y + pos.y + g_contentOffset.y));
    const ImVec2 size(12.0f, 9.0f);

    ImGui::SetCursorScreenPos(min);

    const bool pressed = ImGui::InvisibleButton(id_text, size);
    static bool colorDragging = false;
    static ImVec2 colorGrabOff{};

    ImDrawList* draw = ImGui::GetWindowDrawList();

    draw->AddRectFilled(min, min + size, ColorU32(*color), 0.0f);
    draw->AddRect(min, min + size, OutlineBlack(), 0.0f, 0, 1.0f);
    if (pressed)
        ImGui::OpenPopup(id_text);

    if (ImGui::BeginPopup(id_text))
    {
        ImVec2 popPos = ImGui::GetWindowPos();
        ImVec2 popSize = ImGui::GetWindowSize();
        ImGui::SetCursorScreenPos(popPos);
        ImGui::PushID("color_drag");
        ImGui::InvisibleButton("##color_drag", ImVec2(popSize.x, 10.0f));
        ImGuiID dragId = ImGui::GetItemID();
        float dragHov = AnimateFloat(dragId, ImGui::IsItemHovered(), 18.0f);
        ImDrawList* dragDraw = ImGui::GetWindowDrawList();
        for (int i = 0; i < 3; ++i) {
            float dx = popPos.x + popSize.x * 0.5f + (float)(i - 1) * 8.0f;
            dragDraw->AddCircleFilled(ImVec2(dx, popPos.y + 5.0f), 1.2f, ColorU32(LerpColor(GetTheme().Text, GetTheme().TextBright, dragHov)), 8);
        }
        if (ImGui::IsItemActivated()) {
            colorGrabOff = ImGui::GetIO().MousePos - popPos;
            colorDragging = true;
        }
        if (colorDragging) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                ImGui::SetWindowPos(ImGui::GetIO().MousePos - colorGrabOff, ImGuiCond_Always);
            else
                colorDragging = false;
        }
        ImGui::PopID();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, GetTheme().CardBg);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, GetTheme().ControlInactive);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, GetTheme().ControlInactive);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, GetTheme().Accent);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, GetTheme().Accent);
        ImGui::PushStyleColor(ImGuiCol_Button, GetTheme().ControlBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GetTheme().ControlInactive);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, GetTheme().ControlInactive);
        ImGui::PushStyleColor(ImGuiCol_Header, GetTheme().CardBg);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, GetTheme().ControlInactive);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, GetTheme().ControlInactive);
        ImGui::ColorPicker4("##picker", (float*)color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_PickerHueBar);
        ImGui::PopStyleColor(11);
        const Theme& ptheme = GetTheme();
        ImFont* pfont = GetFonts().CascadiaMonoBL ? GetFonts().CascadiaMonoBL : ImGui::GetFont();
        ImGuiWindow* pwin = ImGui::GetCurrentWindow();
        ImVec2 prel = ImGui::GetCursorScreenPos() - pwin->Pos + ImVec2(0.0f, 16.0f);
        SliderFloat("hex_opacity", &color->w, 0.0f, 1.0f, prel, 180.0f, "Opacity", "%.2f");
        if (color->w < 0.0f) color->w = 0.0f;
        if (color->w > 1.0f) color->w = 1.0f;
        char hex[16];
        ImFormatString(hex, IM_ARRAYSIZE(hex), "#%02X%02X%02X%02X",
            (int)(ImClamp(color->x, 0.0f, 1.0f) * 255.0f),
            (int)(ImClamp(color->y, 0.0f, 1.0f) * 255.0f),
            (int)(ImClamp(color->z, 0.0f, 1.0f) * 255.0f),
            (int)(ImClamp(color->w, 0.0f, 1.0f) * 255.0f));
        ImVec2 hexRel = prel + ImVec2(0.0f, 15.0f);
        ImVec2 hexMin = pwin->Pos + hexRel;
        ImVec2 hexTs = pfont->CalcTextSizeA(12.0f * g_fontScale, FLT_MAX, 0.0f, hex);
        ImGui::SetCursorScreenPos(hexMin);
        ImGui::PushID("hex_ctx_btn");
        ImGui::InvisibleButton("##hex", ImVec2(hexTs.x + 6.0f, 15.0f));
        ImGuiID hexId = ImGui::GetItemID();
        float hexHov = AnimateFloat(hexId, ImGui::IsItemHovered(), 18.0f);
        ImGui::GetWindowDrawList()->AddText(pfont, 12.0f * g_fontScale, hexMin, ColorU32(LerpColor(ptheme.Text, ptheme.TextBright, hexHov)), hex);
        static ImGuiID hexCtxOpen = 0;
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            hexCtxOpen = (hexCtxOpen == hexId) ? 0 : hexId;
        bool hexCtx = (hexCtxOpen == hexId);
        float hexCtxAnim = AnimateFloat(hexId + 40, hexCtx, 18.0f);
        const float hexRowH = 16.0f;
        const float hexPad = 3.0f;
        const float hexPopW = 110.0f;
        const float hexFullH = hexPad * 2.0f + hexRowH * 2.0f;
        ImVec2 hexPopMin(hexMin.x, hexMin.y + 16.0f);
        if (hexCtx && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImRect(hexPopMin, ImVec2(hexPopMin.x + hexPopW, hexPopMin.y + hexFullH)).Contains(ImGui::GetIO().MousePos))
            hexCtxOpen = 0;
        if (hexCtxAnim > 0.01f) {
            ImDrawList* hexFg = ImGui::GetForegroundDrawList();
            hexFg->PushClipRect(hexPopMin, ImVec2(hexPopMin.x + hexPopW, hexPopMin.y + hexFullH * hexCtxAnim), true);
            ImVec2 hexBoxMax = ImVec2(hexPopMin.x + hexPopW, hexPopMin.y + hexFullH);
            hexFg->AddRectFilled(hexPopMin, hexBoxMax, ColorU32(ptheme.ControlBg, hexCtxAnim), 0.0f);
            hexFg->AddRect(hexPopMin, hexBoxMax, ImGui::GetColorU32(IM_COL32(0, 0, 0, (int)(255 * hexCtxAnim))), 0.0f, 0, 1.0f);
            const char* hexOpts[2] = {"Copy Hex", "Paste Hex"};
            for (int i = 0; i < 2; ++i) {
                ImVec2 iMin(hexPopMin.x + 2.0f, hexPopMin.y + hexPad + hexRowH * i);
                ImVec2 iMax(hexPopMin.x + hexPopW - 2.0f, iMin.y + hexRowH);
                bool hov = ImRect(iMin, iMax).Contains(ImGui::GetIO().MousePos);
                ImGui::PushID(100 + i);
                ImGuiID iid = ImGui::GetID("hex_opt");
                float ih = AnimateFloat(iid, hov, 18.0f);
                ImVec4 parsed{};
                bool valid = (i == 1) ? ParseHexColor(ImGui::GetClipboardText(), parsed) : true;
                if (ih > 0.01f)
                    hexFg->AddRectFilled(iMin, iMax, ColorU32(LerpColor(ptheme.ControlBg, ptheme.ControlInactive, ih * 0.8f), hexCtxAnim), 0.0f);
                hexFg->AddText(pfont, 12.0f * g_fontScale, ImVec2(iMin.x + 4.0f, iMin.y + 2.0f), ColorU32(valid ? LerpColor(ptheme.Text, ptheme.TextBright, ih * 0.35f) : ImVec4(0.45f, 0.45f, 0.45f, 1.0f), hexCtxAnim), hexOpts[i]);
                if (hexCtx && hexCtxAnim > 0.70f && hov && valid && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    if (i == 0)
                        ImGui::SetClipboardText(hex);
                    else
                        *color = parsed;
                    hexCtxOpen = 0;
                }
                ImGui::PopID();
            }
            hexFg->PopClipRect();
        }
        ImGui::PopID();
        popPos = ImGui::GetWindowPos();
        popSize = ImGui::GetWindowSize();
        ImGui::EndPopup();
        ImDrawList* popFg = ImGui::GetForegroundDrawList();
        popFg->AddRect(popPos, popPos + popSize, OutlineBlack(), 0.0f, 0, 1.0f);
        popFg->AddRect(popPos + ImVec2(1.0f, 1.0f), popPos + popSize - ImVec2(1.0f, 1.0f), OutlineInner(), 0.0f, 0, 1.0f);
    } else {
        colorDragging = false;
    }

    return pressed;
}

inline const char* KeyName(int key)
{
    switch (key)
    {
    case 0x01: return "Left Mouse";
    case 0x02: return "Right Mouse";
    case 0x04: return "Middle Mouse";
    case 0x05: return "Mouse X1";
    case 0x06: return "Mouse X2";
    case 0x10: return "Shift";
    case 0x11: return "Ctrl";
    case 0x12: return "Alt";
    case 0x20: return "Space";
    default: break;
    }
    if (key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END)
        return ImGui::GetKeyName((ImGuiKey)key);

    static char name[16];

    if (key >= 'A' && key <= 'Z')
        ImFormatString(name, IM_ARRAYSIZE(name), "%c", key);
    else
        ImFormatString(name, IM_ARRAYSIZE(name), "Key %d", key);

    return name;
}

inline bool Keybind(const char* label, int* key, const ImVec2& pos, const ImVec2& size = ImVec2(68.0f, 13.0f), int* mode = nullptr)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2 base = window->Pos;
    const ImVec2 min = ImVec2(std::floor(base.x + pos.x + g_contentOffset.x), std::floor(base.y + pos.y + g_contentOffset.y));
    ImGui::SetCursorScreenPos(min);
    const bool pressed = ImGui::InvisibleButton(label, size);
    const ImGuiID id = ImGui::GetItemID();
    const bool right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

    static std::unordered_map<ImGuiID, int> s_keybind_modes;
    int& current_mode = mode ? *mode : s_keybind_modes[id];

    static ImGuiID context_open_id = 0;
    if (right_clicked)
    {
        context_open_id = (context_open_id == id) ? 0 : id;
    }

    static ImGuiID waiting_id = 0;
    static bool wait_mouse_release = false;
    if (pressed && !PopupBlocking())
    {
        context_open_id = 0;
        waiting_id = id;
        wait_mouse_release = true;
        ImGui::SetActiveID(id, window);
    }

    const bool active = waiting_id == id;
    if (active)
    {
        ImGuiIO& io = ImGui::GetIO();
        bool any_mouse_down = false;
        for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); ++i)
            any_mouse_down |= io.MouseDown[i];
        if (!any_mouse_down)
            wait_mouse_release = false;

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            *key = 0;
            waiting_id = 0;
            ImGui::ClearActiveID();
        }

        for (int key_code = ImGuiKey_NamedKey_BEGIN; key_code < ImGuiKey_NamedKey_END; ++key_code)
        {
            ImGuiKey imgui_key = (ImGuiKey)key_code;
            if (ImGui::IsKeyPressed(imgui_key) && imgui_key != ImGuiKey_Escape)
            {
                *key = key_code;
                waiting_id = 0;
                ImGui::ClearActiveID();
                break;
            }
        }

        if (waiting_id == id && !wait_mouse_release)
        {
            for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); ++i)
            {
                if (ImGui::IsMouseClicked(i))
                {
                    *key = i == 0 ? 0x01 : i == 1 ? 0x02 : i == 2 ? 0x04 : i == 3 ? 0x05 : 0x06;
                    waiting_id = 0;
                    ImGui::ClearActiveID();
                    break;
                }
            }
        }
    }

    const float active_anim = AnimateFloat(id, active || ImGui::IsItemHovered(), 16.0f);
    const Theme& theme = GetTheme();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(min, min + size, ColorU32(LerpColor(theme.KeybindBg, theme.ControlInactive, active_anim * 0.35f)), 0.0f);
    draw->AddRect(min, min + size, OutlineBlack(), 0.0f, 0, 1.0f);
    draw->AddRect(min + ImVec2(1.0f, 1.0f), min + size - ImVec2(1.0f, 1.0f), OutlineInner(), 0.0f, 0, 1.0f);

    const Fonts& fonts = GetFonts();
    ImFont* font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
    const float font_size = 12.0f * g_fontScale;
    const char* text = active ? "..." : (current_mode == 2 && *key == 0 ? "Always" : KeyName(*key));
    const ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text);
    draw->AddText(font, font_size, ImVec2(min.x + (size.x - text_size.x) * 0.5f, min.y + (size.y - text_size.y) * 0.5f), ColorU32(ImVec4(0.839f, 0.839f, 0.839f, 1.0f)), text);

    const bool context_open = (context_open_id == id);
    const float context_anim = AnimateFloat(id + 20, context_open, 18.0f);
    const float popup_w = size.x;
    const float row_height = 16.0f;
    const float popup_padding = 3.0f;
    const float full_height = popup_padding * 2.0f + row_height * 3.0f;
    const float visible_height = full_height * context_anim;

    const ImVec2 popup_min(min.x, min.y + size.y + 2.0f);
    const ImVec2 popup_max(popup_min.x + popup_w, popup_min.y + visible_height);
    const ImRect total_rect(min, ImVec2(min.x + popup_w, popup_min.y + full_height));

    if (context_open && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !total_rect.Contains(ImGui::GetIO().MousePos))
    {
        context_open_id = 0;
    }

    if (context_anim > 0.01f)
    {
        ImDrawList* overlay = ImGui::GetForegroundDrawList();
        overlay->PushClipRect(popup_min, popup_max, true);
        const ImVec2 popup_box_max = ImVec2(popup_min.x + popup_w, popup_min.y + full_height);
        overlay->AddRectFilled(popup_min, popup_box_max, ColorU32(theme.ControlBg, context_anim), 0.0f);

        overlay->AddRect(popup_min, popup_box_max, ImGui::GetColorU32(IM_COL32(0, 0, 0, (int)(255 * context_anim))), 0.0f, 0, 1.0f);
        overlay->AddRect(popup_min + ImVec2(1.0f, 1.0f), popup_box_max - ImVec2(1.0f, 1.0f), ImGui::GetColorU32(IM_COL32(52, 52, 56, (int)(255 * context_anim))), 0.0f, 0, 1.0f);

        struct ModeDef { const char* label; int value; };
        static const ModeDef mode_list[3] = {
            { "Toggle", 1 },
            { "Hold",   0 },
            { "Always", 2 }
        };

        for (int i = 0; i < 3; ++i)
        {
            const ImVec2 item_min(popup_min.x + 2.0f, popup_min.y + popup_padding + row_height * i);
            const ImVec2 item_max(popup_min.x + popup_w - 2.0f, item_min.y + row_height);
            const ImRect item_rect(item_min, item_max);
            const bool item_hovered = item_rect.Contains(ImGui::GetIO().MousePos);
            const bool item_pressed = context_open && context_anim > 0.70f && item_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

            ImGui::PushID(i);
            const ImGuiID item_id = ImGui::GetID("kb_mode");
            const float item_hover = AnimateFloat(item_id, item_hovered, 18.0f);
            const bool is_current = (current_mode == mode_list[i].value);
            const float item_selected = AnimateFloat(item_id + 1, is_current, 18.0f);
            const float item_appear = ImClamp((context_anim - i * 0.08f) / 0.45f, 0.0f, 1.0f);

            const ImVec4 row_color = LerpColor(theme.ControlBg, theme.ControlInactive, item_hover * 0.8f + item_selected * 0.4f);
            if (item_hover > 0.01f || item_selected > 0.01f)
                overlay->AddRectFilled(item_min, item_max, ColorU32(row_color, context_anim), 0.0f);

            const ImVec4 base_text_color = is_current ? theme.Accent : theme.Text;
            const ImVec4 text_color = LerpColor(base_text_color, theme.TextBright, item_hover * 0.35f);
            const ImVec2 text_sz = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, mode_list[i].label);
            const ImVec2 text_pos(item_min.x + 4.0f, item_min.y + (row_height - text_sz.y) * 0.5f);
            overlay->AddText(font, font_size, text_pos, ColorU32(text_color, item_appear), mode_list[i].label);

            if (item_pressed)
            {
                current_mode = mode_list[i].value;
                if (mode) *mode = mode_list[i].value;
                context_open_id = 0;
                ComboClosedFrame() = ImGui::GetFrameCount();
            }
            ImGui::PopID();
        }

        overlay->PopClipRect();
    }
    if (context_open_id == id)
        ComboOpenId() = id;
    else if (ComboOpenId() == id)
        ComboOpenId() = 0;
    return pressed;
}

inline bool SliderFloat(const char* label, float* value, float min_value, float max_value, const ImVec2& pos, float width, const char* text_label, const char* format = "%.0f")
{
    const float font_size = 12.0f * g_fontScale;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2 base = window->Pos;
    const ImVec2 origin = ImVec2(std::floor(base.x + pos.x + g_contentOffset.x), std::floor(base.y + pos.y + g_contentOffset.y));
    const float height = 7.0f;

    const float step_w = 11.0f;
    const float step_gap = 4.0f;
    const ImVec2 minus_min(origin.x, origin.y);
    const ImVec2 minus_max(origin.x + step_w, origin.y + height);
    const ImVec2 track_min(origin.x + step_w + step_gap, origin.y);
    const float track_w = width - (step_w + step_gap) * 2.0f;
    const ImVec2 track_max(track_min.x + track_w, origin.y + height);
    const ImVec2 plus_min(track_max.x + step_gap, origin.y);
    const ImVec2 plus_max(plus_min.x + step_w, origin.y + height);

    ImGui::PushID(label);
    ImGui::SetCursorScreenPos(origin);
    const bool pressed = ImGui::InvisibleButton("##slider_total", ImVec2(width, height + 4.0f));
    const ImGuiID id = ImGui::GetItemID();

    const ImVec2 mouse = ImGui::GetIO().MousePos;
    const bool minus_hovered = (mouse.x >= minus_min.x - 2.0f && mouse.x <= minus_max.x + 2.0f &&
                                mouse.y >= origin.y - 4.0f && mouse.y <= origin.y + height + 4.0f);
    const bool plus_hovered = (mouse.x >= plus_min.x - 2.0f && mouse.x <= plus_max.x + 2.0f &&
                               mouse.y >= origin.y - 4.0f && mouse.y <= origin.y + height + 4.0f);
    const bool track_hovered = (mouse.x >= track_min.x && mouse.x <= track_max.x &&
                                mouse.y >= origin.y - 3.0f && mouse.y <= origin.y + height + 3.0f);

    const bool active = ImGui::IsItemActive();
    bool changed = false;

    const float step = (max_value - min_value <= 5.0f) ? 0.1f : 1.0f;
    if (!PopupBlocking() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (minus_hovered)
        {
            *value = ImClamp(*value - step, min_value, max_value);
            changed = true;
        }
        else if (plus_hovered)
        {
            *value = ImClamp(*value + step, min_value, max_value);
            changed = true;
        }
    }
    const float down_dur = ImGui::GetIO().MouseDownDuration[0];
    if (down_dur > 0.35f && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        static float last_time = 0.0f;
        const float cur_time = (float)ImGui::GetTime();
        if (cur_time - last_time > 0.08f)
        {
            if (minus_hovered)
            {
                *value = ImClamp(*value - step, min_value, max_value);
                changed = true;
                last_time = cur_time;
            }
            else if (plus_hovered)
            {
                *value = ImClamp(*value + step, min_value, max_value);
                changed = true;
                last_time = cur_time;
            }
        }
    }

    if (active && !minus_hovered && !plus_hovered && !PopupBlocking())
    {
        float t = (mouse.x - track_min.x) / track_w;
        t = ImClamp(t, 0.0f, 1.0f);
        const float new_value = min_value + (max_value - min_value) * t;
        if (*value != new_value)
        {
            *value = new_value;
            changed = true;
        }
    }

    const float target_t = ImClamp((*value - min_value) / (max_value - min_value), 0.0f, 1.0f);
    static std::unordered_map<ImGuiID, float> slider_values;
    float& animated_t = slider_values[id];
    animated_t = ImLerp(animated_t, target_t, ImClamp(ImGui::GetIO().DeltaTime * 16.0f, 0.0f, 1.0f));
    const float hover = AnimateFloat(id + 1, track_hovered || active, 16.0f);
    const float minus_anim = AnimateFloat(id + 2, minus_hovered, 16.0f);
    const float plus_anim = AnimateFloat(id + 3, plus_hovered, 16.0f);

    const Theme& theme = GetTheme();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    draw->AddRectFilled(track_min, track_max, ColorU32(theme.ControlInactive), 0.0f);

    if (animated_t > 0.001f)
    {
        draw->AddRectFilled(track_min, ImVec2(track_min.x + track_w * animated_t, track_max.y),
                            ColorU32(LerpColor(theme.Accent, theme.TextBright, hover * 0.08f)), 0.0f);
    }

    draw->AddRect(track_min, track_max, OutlineBlack(), 0.0f, 0, 1.0f);
    draw->AddRect(track_min + ImVec2(1.0f, 1.0f), track_max - ImVec2(1.0f, 1.0f), OutlineInner(), 0.0f, 0, 1.0f);

    const Fonts& fonts = GetFonts();
    ImFont* font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();

    const ImVec4 muted_col(0.60f, 0.60f, 0.60f, 1.0f);
    const ImVec4 active_col(1.0f, 1.0f, 1.0f, 1.0f);
    const ImU32 minus_col = ColorU32(LerpColor(muted_col, active_col, minus_anim));
    const ImU32 plus_col = ColorU32(LerpColor(muted_col, active_col, plus_anim));

    const ImVec2 minus_ts = font->CalcTextSizeA(12.0f * g_fontScale, FLT_MAX, 0.0f, "-");
    const ImVec2 plus_ts = font->CalcTextSizeA(12.0f * g_fontScale, FLT_MAX, 0.0f, "+");
    const ImVec2 minus_pos(std::floor(minus_min.x + (step_w - minus_ts.x) * 0.5f), std::floor(origin.y + (height - minus_ts.y) * 0.5f - 1.0f));
    const ImVec2 plus_pos(std::floor(plus_min.x + (step_w - plus_ts.x) * 0.5f), std::floor(origin.y + (height - plus_ts.y) * 0.5f - 1.0f));

    AddTextWithOutline(draw, font, font_size, minus_pos, minus_col, "-");
    AddTextWithOutline(draw, font, font_size, plus_pos, plus_col, "+");

    char value_text[64];
    char prefix_text[128];
    ImFormatString(value_text, IM_ARRAYSIZE(value_text), format, *value);
    ImFormatString(prefix_text, IM_ARRAYSIZE(prefix_text), "%s: ", text_label);
    const ImVec2 label_pos(origin.x + 1.0f, origin.y - font_size - 5.0f);
    const ImVec2 prefix_sz = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, prefix_text);
    const ImVec2 value_sz = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, value_text);
    const ImVec2 value_pos(label_pos.x + prefix_sz.x, label_pos.y);

    static ImGuiID s_editId = 0;
    static char s_editBuf[64] = {};
    static bool s_editFocus = false;
    const ImVec2 box_pad(5.0f, 2.0f);
    const float box_w = (std::max)(value_sz.x + box_pad.x * 2.0f, 52.0f);
    const float box_h = font_size + box_pad.y * 2.0f;
    const ImVec2 box_min(value_pos.x - box_pad.x, label_pos.y - box_pad.y);
    const ImVec2 box_max(box_min.x + box_w, box_min.y + box_h);
    draw->AddText(font, font_size, label_pos, ColorU32(theme.Text), prefix_text);
    if (s_editId != id)
    {
        draw->AddRectFilled(box_min, box_max, ColorU32(theme.ControlBg), 0.0f);
        draw->AddRect(box_min, box_max, OutlineBlack(), 0.0f, 0, 1.0f);
        draw->AddRect(box_min + ImVec2(1.0f, 1.0f), box_max - ImVec2(1.0f, 1.0f), OutlineInner(), 0.0f, 0, 1.0f);
        draw->AddText(font, font_size, ImVec2(box_min.x + box_pad.x, box_min.y + box_pad.y), ColorU32(theme.TextBright), value_text);
        ImGui::SetCursorScreenPos(box_min);
        ImGui::InvisibleButton("##slider_edit", box_max - box_min);
        if (!PopupBlocking() && ImGui::IsItemHovered() &&
            (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Left)) &&
            s_editId == 0)
        {
            ImFormatString(s_editBuf, IM_ARRAYSIZE(s_editBuf), "%g", (double)*value);
            s_editId = id;
            s_editFocus = true;
        }
    }
    else
    {
        ImGui::SetCursorScreenPos(box_min);
        ImGui::SetNextItemWidth(box_w - 4.0f);
        if (s_editFocus)
        {
            ImGui::SetKeyboardFocusHere();
            s_editFocus = false;
        }
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ColorU32(theme.ControlBg));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ColorU32(theme.ControlBg));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ColorU32(theme.ControlBg));
        ImGui::PushStyleColor(ImGuiCol_Text, ColorU32(theme.TextBright));
        ImGui::PushStyleColor(ImGuiCol_Border, ColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.0f)));
        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ColorU32(theme.ControlInactive));
        bool done = ImGui::InputText("##slider_input", s_editBuf, IM_ARRAYSIZE(s_editBuf),
                                     ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
        ImGui::PopStyleColor(6);
        ImGui::PopStyleVar();
        ImDrawList* fdl = ImGui::GetWindowDrawList();
        const ImVec2 bmin = ImGui::GetItemRectMin(), bmax = ImGui::GetItemRectMax();
        fdl->AddRect(bmin, bmax, OutlineBlack(), 0.0f, 0, 1.0f);
        fdl->AddRect(bmin + ImVec2(1.0f, 1.0f), bmax - ImVec2(1.0f, 1.0f), OutlineInner(), 0.0f, 0, 1.0f);
        const bool esc = ImGui::IsKeyPressed(ImGuiKey_Escape);
        if (done)
        {
            *value = ImClamp((float)atof(s_editBuf), min_value, max_value);
            changed = true;
            s_editId = 0;
        }
        else if (esc)
        {
            s_editId = 0;
        }
        else if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) &&
                 !ImGui::IsItemActive() && !ImGui::IsItemHovered())
        {
            s_editId = 0;
        }
    }

    ImGui::PopID();
    return pressed || changed;
}

struct ComboTextAnimation
{
    std::string Previous;
    std::string Current;
    float Blend = 1.0f;
};

inline bool Combo(const char* label, int* current_item, const char* const items[], int items_count, const ImVec2& pos, float width, const char* text_label)
{
    const float font_size = 12.0f * g_fontScale;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2 base = window->Pos;
    const ImVec2 min = ImVec2(std::floor(base.x + pos.x + g_contentOffset.x), std::floor(base.y + pos.y + g_contentOffset.y));
    const ImVec2 size(width, 18.0f);
    ImGui::PushID(label);
    ImGui::SetCursorScreenPos(min);
    const bool pressed = ImGui::InvisibleButton("##combo_preview", size);

    const bool hovered = ImGui::IsItemHovered();
    const ImGuiID id = ImGui::GetItemID();
    static ImGuiID open_id = 0;
    if (pressed)
        open_id = open_id == id ? 0 : id;

    const bool open = open_id == id;
    const float hover = AnimateFloat(id, hovered, 18.0f);
    const float open_anim = AnimateFloat(id + 10, open, 18.0f);
    const Theme& theme = GetTheme();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(min, min + size, ColorU32(LerpColor(theme.ControlBg, theme.ControlInactive, hover * 0.35f)), 0.0f);
    draw->AddRect(min, min + size, OutlineBlack(), 0.0f, 0, 1.0f);
    draw->AddRect(min + ImVec2(1.0f, 1.0f), min + size - ImVec2(1.0f, 1.0f), OutlineInner(), 0.0f, 0, 1.0f);

    const Fonts& fonts = GetFonts();
    ImFont* font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
    if (text_label)
        draw->AddText(font, font_size, ImVec2(min.x, min.y - font_size - 3.0f), ColorU32(theme.Text), text_label);
    const char* preview = (*current_item >= 0 && *current_item < items_count) ? items[*current_item] : "";
    static std::unordered_map<ImGuiID, ComboTextAnimation> text_animations;
    ComboTextAnimation& text_anim = text_animations[id];
    if (text_anim.Current.empty())
        text_anim.Current = preview;
    if (text_anim.Current != preview)
    {
        text_anim.Previous = text_anim.Current;
        text_anim.Current = preview;
        text_anim.Blend = 0.0f;
    }
    text_anim.Blend = ImLerp(text_anim.Blend, 1.0f, ImClamp(ImGui::GetIO().DeltaTime * 14.0f, 0.0f, 1.0f));
    ImVec2 preview_sz = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text_anim.Current.c_str());
    const ImVec2 preview_pos(min.x + 3.0f, min.y + (size.y - preview_sz.y) * 0.5f);
    if (!text_anim.Previous.empty() && text_anim.Blend < 0.98f)
        draw->AddText(font, font_size, preview_pos, ColorU32(theme.Text, 1.0f - text_anim.Blend), text_anim.Previous.c_str());
    draw->AddText(font, font_size, preview_pos, ColorU32(theme.Text, text_anim.Blend), text_anim.Current.c_str());

    bool changed = false;
    const float row_height = 16.0f;
    const float popup_padding = 3.0f;
    const float full_height = popup_padding * 2.0f + row_height * items_count;
    const float visible_height = full_height * open_anim;
    ImVec2 boxMin(min.x, min.y + size.y + 2.0f);
    if (boxMin.y + full_height > ImGui::GetIO().DisplaySize.y && min.y - 2.0f - full_height >= 0.0f)
        boxMin.y = min.y - 2.0f - full_height;
    const ImVec2 popup_max(boxMin.x + width, boxMin.y + visible_height);
    const float totalTop = (std::min)(boxMin.y, min.y);
    const float totalBottom = (std::max)(boxMin.y + full_height, min.y + size.y);
    const ImRect total_rect(ImVec2(min.x, totalTop), ImVec2(min.x + width, totalBottom));

    if (open && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !total_rect.Contains(ImGui::GetIO().MousePos)) {
        open_id = 0;
        ComboClosedFrame() = ImGui::GetFrameCount();
    }

    if (open_anim > 0.01f)
    {
        ImDrawList* overlay = ImGui::GetForegroundDrawList();
        overlay->PushClipRect(boxMin, popup_max, true);
        const ImVec2 popup_box_max = ImVec2(boxMin.x + width, boxMin.y + full_height);
        overlay->AddRectFilled(boxMin, popup_box_max, ColorU32(theme.ControlBg, open_anim), 0.0f);
        overlay->AddRect(boxMin, popup_box_max, ImGui::GetColorU32(IM_COL32(0, 0, 0, (int)(255 * open_anim))), 0.0f, 0, 1.0f);
        overlay->AddRect(boxMin + ImVec2(1.0f, 1.0f), popup_box_max - ImVec2(1.0f, 1.0f), ImGui::GetColorU32(IM_COL32(52, 52, 56, (int)(255 * open_anim))), 0.0f, 0, 1.0f);

        for (int i = 0; i < items_count; ++i)
        {
            const ImVec2 item_min(boxMin.x + 2.0f, boxMin.y + popup_padding + row_height * i);
            const ImVec2 item_max(boxMin.x + width - 2.0f, item_min.y + row_height);
            const ImRect item_rect(item_min, item_max);
            const bool item_hovered = item_rect.Contains(ImGui::GetIO().MousePos);
            const bool item_pressed = open && open_anim > 0.70f && item_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
            ImGui::PushID(i);
            const ImGuiID item_id = ImGui::GetID("combo_item");
            const float item_hover = AnimateFloat(item_id, item_hovered, 18.0f);
            const float item_selected = AnimateFloat(item_id + 1, i == *current_item, 18.0f);
            const float item_appear = ImClamp((open_anim - (float)i / (float)items_count * 0.55f) / 0.45f, 0.0f, 1.0f);
            const ImVec4 row_color = LerpColor(theme.ControlBg, theme.ControlInactive, item_hover * 0.8f + item_selected * 0.4f);
            if (item_hover > 0.01f || item_selected > 0.01f)
                overlay->AddRectFilled(item_min, item_max, ColorU32(row_color, open_anim), 0.0f);
            const ImVec4 text_color = LerpColor(theme.Text, theme.TextBright, item_hover * 0.35f + item_selected * 0.55f);
            overlay->AddText(font, font_size, ImVec2(item_min.x + 2.0f, item_min.y + 2.0f), ColorU32(text_color, item_appear), items[i]);
            if (item_pressed)
            {
                *current_item = i;
                changed = true;
                open_id = 0;
            }
            ImGui::PopID();
        }
        overlay->PopClipRect();
    }
    if (open_id == id)
        ComboOpenId() = id;
    else if (ComboOpenId() == id) {
        ComboOpenId() = 0;
        ComboClosedFrame() = ImGui::GetFrameCount();
    }
    ImGui::PopID();
    return changed;
}

struct MultiComboTextAnimation
{
    std::string Previous;
    std::string Current;
    float Blend = 1.0f;
};

inline bool MultiCombo(const char* label, bool values[], const char* const items[], int items_count, const ImVec2& pos, float width, const char* text_label)
{
    const float font_size = 12.0f * g_fontScale;
    std::string preview;
    int selected_count = 0;
    for (int i = 0; i < items_count; ++i)
    {
        if (!values[i])
            continue;
        ++selected_count;
        if (!preview.empty())
            preview += " , ";
        preview += items[i];
    }
    if (selected_count > 3)
        preview = std::to_string(selected_count) + " Selected";
    if (preview.empty())
        preview = "Select...";

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2 base = window->Pos;
    const ImVec2 min = ImVec2(std::floor(base.x + pos.x + g_contentOffset.x), std::floor(base.y + pos.y + g_contentOffset.y));
    const ImVec2 size(width, 18.0f);
    const ImVec2 hit_size(width, 20.0f);
    ImGui::PushID(label);
    ImGui::SetCursorScreenPos(min - ImVec2(0.0f, 0.0f));
    const bool pressed = ImGui::InvisibleButton("##multicombo_preview", hit_size);

    const bool hovered = ImGui::IsItemHovered();
    const ImGuiID id = ImGui::GetItemID();
    static ImGuiID open_id = 0;
    if (pressed)
        open_id = open_id == id ? 0 : id;

    const bool open = open_id == id;
    const float hover = AnimateFloat(id, hovered, 16.0f);
    const float open_anim = AnimateFloat(id + 10, open, 18.0f);
    const Theme& theme = GetTheme();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(min, min + size, ColorU32(LerpColor(theme.ControlBg, theme.ControlInactive, hover * 0.35f)), 0.0f);
    draw->AddRect(min, min + size, OutlineBlack(), 0.0f, 0, 1.0f);
    draw->AddRect(min + ImVec2(1.0f, 1.0f), min + size - ImVec2(1.0f, 1.0f), OutlineInner(), 0.0f, 0, 1.0f);

    const Fonts& fonts = GetFonts();
    ImFont* font = fonts.CascadiaMonoBL ? fonts.CascadiaMonoBL : ImGui::GetFont();
    if (text_label)
        draw->AddText(font, font_size, ImVec2(min.x, min.y - font_size - 3.0f), ColorU32(theme.Text), text_label);
    static std::unordered_map<ImGuiID, MultiComboTextAnimation> text_animations;
    MultiComboTextAnimation& text_anim = text_animations[id];
    if (text_anim.Current.empty())
        text_anim.Current = preview;
    if (text_anim.Current != preview)
    {
        text_anim.Previous = text_anim.Current;
        text_anim.Current = preview;
        text_anim.Blend = 0.0f;
    }
    text_anim.Blend = ImLerp(text_anim.Blend, 1.0f, ImClamp(ImGui::GetIO().DeltaTime * 14.0f, 0.0f, 1.0f));
    ImVec2 preview_sz = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text_anim.Current.c_str());
    const ImVec2 preview_pos(min.x + 3.0f, min.y + (size.y - preview_sz.y) * 0.5f);
    if (!text_anim.Previous.empty() && text_anim.Blend < 0.98f)
        draw->AddText(font, font_size, preview_pos, ColorU32(theme.Text, 1.0f - text_anim.Blend), text_anim.Previous.c_str());
    draw->AddText(font, font_size, preview_pos, ColorU32(theme.Text, text_anim.Blend), text_anim.Current.c_str());

    bool changed = false;
    const float row_height = 17.0f;
    const float popup_padding = 3.0f;
    constexpr int kMaxRows = 10;
    const int shown_rows = items_count < kMaxRows ? items_count : kMaxRows;
    const float list_h = row_height * (float)items_count;
    const float box_list_h = row_height * (float)shown_rows;
    const float box_h = popup_padding * 2.0f + box_list_h;
    const float visible_height = box_h * open_anim;
    const ImVec2 popup_min(min.x, min.y + size.y + 2.0f);
    const ImVec2 popup_max(popup_min.x + width, popup_min.y + visible_height);
    const ImVec2 box_max(popup_min.x + width, popup_min.y + box_h);
    const ImRect total_rect(min, ImVec2(min.x + width, popup_min.y + box_h));

    static std::unordered_map<ImGuiID, float> s_multiScroll;
    float& mscroll = s_multiScroll[id];
    const float maxScroll = list_h > box_list_h ? (list_h - box_list_h) : 0.0f;
    if (mscroll < 0.0f)
        mscroll = 0.0f;
    if (mscroll > maxScroll)
        mscroll = maxScroll;
    if (open && ImGui::GetIO().MouseWheel != 0.0f &&
        ImRect(popup_min, box_max).Contains(ImGui::GetIO().MousePos)) {
        mscroll -= ImGui::GetIO().MouseWheel * 22.0f;
        if (mscroll < 0.0f)
            mscroll = 0.0f;
        if (mscroll > maxScroll)
            mscroll = maxScroll;
    }

    if (open && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !total_rect.Contains(ImGui::GetIO().MousePos)) {
        open_id = 0;
        ComboClosedFrame() = ImGui::GetFrameCount();
    }

    if (open_anim > 0.01f)
    {
        ImDrawList* overlay = ImGui::GetForegroundDrawList();
        overlay->PushClipRect(popup_min, popup_max, true);
        overlay->AddRectFilled(popup_min, box_max, ColorU32(theme.ControlBg, open_anim), 0.0f);
        overlay->AddRect(popup_min, box_max, ImGui::GetColorU32(IM_COL32(0, 0, 0, (int)(255 * open_anim))), 0.0f, 0, 1.0f);
        overlay->AddRect(popup_min + ImVec2(1.0f, 1.0f), box_max - ImVec2(1.0f, 1.0f), ImGui::GetColorU32(IM_COL32(52, 52, 56, (int)(255 * open_anim))), 0.0f, 0, 1.0f);

        for (int i = 0; i < items_count; ++i)
        {
            const ImVec2 item_min(popup_min.x + 2.0f, popup_min.y + popup_padding + row_height * i - mscroll);
            const ImVec2 item_max(popup_min.x + width - 2.0f, item_min.y + row_height);
            const ImRect item_rect(item_min, item_max);
            const bool item_hovered = item_rect.Contains(ImGui::GetIO().MousePos);
            const bool item_pressed = open && open_anim > 0.70f && item_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
            ImGui::PushID(i);
            const ImGuiID item_id = ImGui::GetID("multicombo_item");
            const float item_hover = AnimateFloat(item_id, item_hovered, 18.0f);
            const float item_selected = AnimateFloat(item_id + 1, values[i], 18.0f);
            const float item_appear = (items_count > 12) ? open_anim : ImClamp((open_anim - i * 0.08f) / 0.45f, 0.0f, 1.0f);
            const ImVec4 row_color = LerpColor(theme.ControlBg, theme.ControlInactive, item_hover * 0.8f + item_selected * 0.4f);
            if (item_hover > 0.01f || item_selected > 0.01f)
                overlay->AddRectFilled(item_min, item_max, ColorU32(row_color, open_anim), 0.0f);
            const ImVec4 text_color = LerpColor(theme.Text, theme.TextBright, item_hover * 0.35f + item_selected * 0.55f);
            overlay->AddText(font, font_size, ImVec2(item_min.x + 2.0f, item_min.y + 2.0f), ColorU32(text_color, item_appear), items[i]);
            if (item_pressed)
            {
                values[i] = !values[i];
                changed = true;
            }
            ImGui::PopID();
        }
        overlay->PopClipRect();
    }
    if (open_id == id)
        ComboOpenId() = id;
    else if (ComboOpenId() == id) {
        ComboOpenId() = 0;
        ComboClosedFrame() = ImGui::GetFrameCount();
    }
    ImGui::PopID();
    return changed;
}
}
