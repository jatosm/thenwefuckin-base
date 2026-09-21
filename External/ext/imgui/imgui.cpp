
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"

#include <stdio.h>
#include <stdint.h>

#if defined(_WIN32) && !defined(_MSC_VER) && !defined(IMGUI_ENABLE_WIN32_DEFAULT_IME_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#endif

#if defined(_WIN32) && defined(IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS) && defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS) && defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS) && defined(IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#define IMGUI_DISABLE_WIN32_FUNCTIONS
#endif
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef __MINGW32__
#include <Windows.h>
#else
#include <windows.h>
#endif
#if defined(WINAPI_FAMILY) && ((defined(WINAPI_FAMILY_APP) && WINAPI_FAMILY == WINAPI_FAMILY_APP) || (defined(WINAPI_FAMILY_GAMES) && WINAPI_FAMILY == WINAPI_FAMILY_GAMES))

#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#endif
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4127)
#pragma warning (disable: 4996)
#if defined(_MSC_VER) && _MSC_VER >= 1922
#pragma warning (disable: 5054)
#endif
#pragma warning (disable: 26451)
#pragma warning (disable: 26495)
#pragma warning (disable: 26812)
#endif

#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wfloat-equal"
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#pragma clang diagnostic ignored "-Wformat-pedantic"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"
#pragma clang diagnostic ignored "-Wswitch-default"
#elif defined(__GNUC__)

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#define IMGUI_DEBUG_NAV_SCORING     0
#define IMGUI_DEBUG_NAV_RECTS       0

static const float FONT_DEFAULT_SIZE_BASE = 20.0f;

static const float NAV_WINDOWING_HIGHLIGHT_DELAY            = 0.20f;
static const float NAV_WINDOWING_LIST_APPEAR_DELAY          = 0.15f;
static const float NAV_ACTIVATE_HIGHLIGHT_TIMER             = 0.10f;
static const float NAV_ACTIVATE_INPUT_WITH_GAMEPAD_DELAY    = 0.60f;
static const float WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER = 0.04f;
static const float WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER    = 0.70f;

static const ImVec2 TOOLTIP_DEFAULT_OFFSET_MOUSE = ImVec2(16, 10);
static const ImVec2 TOOLTIP_DEFAULT_OFFSET_TOUCH = ImVec2(0, -20);
static const ImVec2 TOOLTIP_DEFAULT_PIVOT_TOUCH = ImVec2(0.5f, 1.0f);

static void             SetCurrentWindow(ImGuiWindow* window);
static ImGuiWindow*     CreateNewWindow(const char* name, ImGuiWindowFlags flags);
static ImVec2           CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window);

static void             AddWindowToSortBuffer(ImVector<ImGuiWindow*>* out_sorted_windows, ImGuiWindow* window);

static void             WindowSettingsHandler_ClearAll(ImGuiContext*, ImGuiSettingsHandler*);
static void*            WindowSettingsHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name);
static void             WindowSettingsHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
static void             WindowSettingsHandler_ApplyAll(ImGuiContext*, ImGuiSettingsHandler*);
static void             WindowSettingsHandler_WriteAll(ImGuiContext*, ImGuiSettingsHandler*, ImGuiTextBuffer* buf);

static const char*      Platform_GetClipboardTextFn_DefaultImpl(ImGuiContext* ctx);
static void             Platform_SetClipboardTextFn_DefaultImpl(ImGuiContext* ctx, const char* text);
static void             Platform_SetImeDataFn_DefaultImpl(ImGuiContext* ctx, ImGuiViewport* viewport, ImGuiPlatformImeData* data);
static bool             Platform_OpenInShellFn_DefaultImpl(ImGuiContext* ctx, const char* path);

namespace ImGui
{

static void             ItemHandleShortcut(ImGuiID id);

static int              FindWindowFocusIndex(ImGuiWindow* window);
static void             UpdateWindowInFocusOrderList(ImGuiWindow* window, bool just_created, ImGuiWindowFlags new_flags);

static void             NavUpdate();
static void             NavUpdateWindowing();
static void             NavUpdateWindowingApplyFocus(ImGuiWindow* window);
static void             NavUpdateWindowingOverlay();
static void             NavUpdateCancelRequest();
static void             NavUpdateContextMenuRequest();
static void             NavUpdateCreateMoveRequest();
static void             NavUpdateCreateTabbingRequest();
static float            NavUpdatePageUpPageDown();
static inline void      NavUpdateAnyRequestFlag();
static void             NavUpdateCreateWrappingRequest();
static void             NavEndFrame();
static bool             NavScoreItem(ImGuiNavItemData* result, const ImRect& nav_bb);
static void             NavApplyItemToResult(ImGuiNavItemData* result);
static void             NavProcessItem();
static void             NavProcessItemForTabbingRequest(ImGuiID id, ImGuiItemFlags item_flags, ImGuiNavMoveFlags move_flags);
static ImGuiInputSource NavCalcPreferredRefPosSource(ImGuiWindowFlags window_type);
static ImVec2           NavCalcPreferredRefPos(ImGuiWindowFlags window_type);
static void             NavSaveLastChildNavWindowIntoParent(ImGuiWindow* nav_window);
static ImGuiWindow*     NavRestoreLastChildNavWindow(ImGuiWindow* window);
static void             NavRestoreLayer(ImGuiNavLayer layer);

static void             ErrorCheckNewFrameSanityChecks();
static void             ErrorCheckEndFrameSanityChecks();
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
static void             UpdateDebugToolItemPicker();
static void             UpdateDebugToolItemPathQuery();
static void             UpdateDebugToolFlashStyleColor();
#endif

static void             UpdateKeyboardInputs();
static void             UpdateMouseInputs();
static void             UpdateMouseWheel();
static void             UpdateKeyRoutingTable(ImGuiKeyRoutingTable* rt);

static void             UpdateFontsNewFrame();
static void             UpdateFontsEndFrame();
static void             UpdateTexturesNewFrame();
static void             UpdateTexturesEndFrame();
static void             UpdateSettings();
static int              UpdateWindowManualResize(ImGuiWindow* window, int* border_hovered, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4], const ImRect& visibility_rect);
static void             RenderWindowOuterBorders(ImGuiWindow* window);
static void             RenderWindowDecorations(ImGuiWindow* window, const ImRect& title_bar_rect, bool title_bar_is_highlight, bool handle_borders_and_resize_grips, int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size);
static void             RenderWindowTitleBarContents(ImGuiWindow* window, const ImRect& title_bar_rect, const char* name, bool* p_open);
static void             RenderDimmedBackgroundBehindWindow(ImGuiWindow* window, ImU32 col);
static void             RenderDimmedBackgrounds();
static void             SetLastItemDataForWindow(ImGuiWindow* window, const ImRect& rect);
static void             SetLastItemDataForChildWindowItem(ImGuiWindow* window, const ImRect& rect);

const ImGuiID           IMGUI_VIEWPORT_DEFAULT_ID = 0x11111111;
static void             UpdateViewportsNewFrame();

}

#ifndef GImGui
ImGuiContext*   GImGui = NULL;
#endif

#ifndef IMGUI_DISABLE_DEFAULT_ALLOCATORS
static void*   MallocWrapper(size_t size, void* user_data)    { IM_UNUSED(user_data); return malloc(size); }
static void    FreeWrapper(void* ptr, void* user_data)        { IM_UNUSED(user_data); free(ptr); }
#else
static void*   MallocWrapper(size_t size, void* user_data)    { IM_UNUSED(user_data); IM_UNUSED(size); IM_ASSERT(0); return NULL; }
static void    FreeWrapper(void* ptr, void* user_data)        { IM_UNUSED(user_data); IM_UNUSED(ptr); IM_ASSERT(0); }
#endif
static ImGuiMemAllocFunc    GImAllocatorAllocFunc = MallocWrapper;
static ImGuiMemFreeFunc     GImAllocatorFreeFunc = FreeWrapper;
static void*                GImAllocatorUserData = NULL;

ImGuiStyle::ImGuiStyle()
{
    FontSizeBase                = 0.0f;
    FontScaleMain               = 1.0f;
    FontScaleDpi                = 1.0f;

    Alpha                       = 1.0f;
    DisabledAlpha               = 0.60f;
    WindowPadding               = ImVec2(8,8);
    WindowRounding              = 0.0f;
    WindowBorderSize            = 1.0f;
    WindowBorderHoverPadding    = 4.0f;
    WindowMinSize               = ImVec2(32,32);
    WindowTitleAlign            = ImVec2(0.0f,0.5f);
    WindowMenuButtonPosition    = ImGuiDir_Left;
    ChildRounding               = 0.0f;
    ChildBorderSize             = 1.0f;
    PopupRounding               = 0.0f;
    PopupBorderSize             = 1.0f;
    FramePadding                = ImVec2(4,3);
    FrameRounding               = 0.0f;
    FrameBorderSize             = 0.0f;
    ItemSpacing                 = ImVec2(8,4);
    ItemInnerSpacing            = ImVec2(4,4);
    CellPadding                 = ImVec2(4,2);
    TouchExtraPadding           = ImVec2(0,0);
    IndentSpacing               = 21.0f;
    ColumnsMinSpacing           = 6.0f;
    ScrollbarSize               = 14.0f;
    ScrollbarRounding           = 9.0f;
    ScrollbarPadding            = 2.0f;
    GrabMinSize                 = 12.0f;
    GrabRounding                = 0.0f;
    LogSliderDeadzone           = 4.0f;
    ImageRounding               = 0.0f;
    ImageBorderSize             = 0.0f;
    TabRounding                 = 5.0f;
    TabBorderSize               = 0.0f;
    TabMinWidthBase             = 1.0f;
    TabMinWidthShrink           = 80.0f;
    TabCloseButtonMinWidthSelected   = -1.0f;
    TabCloseButtonMinWidthUnselected = 0.0f;
    TabBarBorderSize            = 1.0f;
    TabBarOverlineSize          = 1.0f;
    TableAngledHeadersAngle     = 35.0f * (IM_PI / 180.0f);
    TableAngledHeadersTextAlign = ImVec2(0.5f,0.0f);
    TreeLinesFlags              = ImGuiTreeNodeFlags_DrawLinesNone;
    TreeLinesSize               = 1.0f;
    TreeLinesRounding           = 0.0f;
    DragDropTargetRounding      = 0.0f;
    DragDropTargetBorderSize    = 2.0f;
    DragDropTargetPadding       = 3.0f;
    ColorMarkerSize             = 3.0f;
    ColorButtonPosition         = ImGuiDir_Right;
    ButtonTextAlign             = ImVec2(0.5f,0.5f);
    SelectableTextAlign         = ImVec2(0.0f,0.0f);
    SeparatorSize               = 1.0f;
    SeparatorTextBorderSize     = 3.0f;
    SeparatorTextAlign          = ImVec2(0.0f,0.5f);
    SeparatorTextPadding        = ImVec2(20.0f,3.f);
    DisplayWindowPadding        = ImVec2(19,19);
    DisplaySafeAreaPadding      = ImVec2(3,3);
    MouseCursorScale            = 1.0f;
    AntiAliasedLines            = true;
    AntiAliasedLinesUseTex      = true;
    AntiAliasedFill             = true;
    CurveTessellationTol        = 1.25f;
    CircleTessellationMaxError  = 0.30f;

    HoverStationaryDelay        = 0.15f;
    HoverDelayShort             = 0.15f;
    HoverDelayNormal            = 0.40f;
    HoverFlagsForTooltipMouse   = ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled;
    HoverFlagsForTooltipNav     = ImGuiHoveredFlags_NoSharedDelay | ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled;

    _MainScale                  = 1.0f;
    _NextFrameFontSizeBase      = 0.0f;

    ImGui::StyleColorsDark(this);
}

void ImGuiStyle::ScaleAllSizes(float scale_factor)
{
    _MainScale *= scale_factor;
    WindowPadding = ImTrunc(WindowPadding * scale_factor);
    WindowRounding = ImTrunc(WindowRounding * scale_factor);
    WindowBorderSize = ImTrunc(WindowBorderSize * scale_factor);
    WindowMinSize = ImTrunc(WindowMinSize * scale_factor);
    WindowBorderHoverPadding = ImTrunc(WindowBorderHoverPadding * scale_factor);
    ChildRounding = ImTrunc(ChildRounding * scale_factor);
    ChildBorderSize = ImTrunc(ChildBorderSize * scale_factor);
    PopupRounding = ImTrunc(PopupRounding * scale_factor);
    PopupBorderSize = ImTrunc(PopupBorderSize * scale_factor);
    FramePadding = ImTrunc(FramePadding * scale_factor);
    FrameBorderSize = ImTrunc(FrameBorderSize * scale_factor);
    FrameRounding = ImTrunc(FrameRounding * scale_factor);
    ItemSpacing = ImTrunc(ItemSpacing * scale_factor);
    ItemInnerSpacing = ImTrunc(ItemInnerSpacing * scale_factor);
    CellPadding = ImTrunc(CellPadding * scale_factor);
    TouchExtraPadding = ImTrunc(TouchExtraPadding * scale_factor);
    IndentSpacing = ImTrunc(IndentSpacing * scale_factor);
    ColumnsMinSpacing = ImTrunc(ColumnsMinSpacing * scale_factor);
    ScrollbarSize = ImTrunc(ScrollbarSize * scale_factor);
    ScrollbarRounding = ImTrunc(ScrollbarRounding * scale_factor);
    ScrollbarPadding = ImTrunc(ScrollbarPadding * scale_factor);
    GrabMinSize = ImTrunc(GrabMinSize * scale_factor);
    GrabRounding = ImTrunc(GrabRounding * scale_factor);
    LogSliderDeadzone = ImTrunc(LogSliderDeadzone * scale_factor);
    ImageRounding = ImTrunc(ImageRounding * scale_factor);
    ImageBorderSize = ImTrunc(ImageBorderSize * scale_factor);
    TabRounding = ImTrunc(TabRounding * scale_factor);
    TabBorderSize = ImTrunc(TabBorderSize * scale_factor);
    TabMinWidthBase = ImTrunc(TabMinWidthBase * scale_factor);
    TabMinWidthShrink = ImTrunc(TabMinWidthShrink * scale_factor);
    TabCloseButtonMinWidthSelected = (TabCloseButtonMinWidthSelected > 0.0f && TabCloseButtonMinWidthSelected != FLT_MAX) ? ImTrunc(TabCloseButtonMinWidthSelected * scale_factor) : TabCloseButtonMinWidthSelected;
    TabCloseButtonMinWidthUnselected = (TabCloseButtonMinWidthUnselected > 0.0f && TabCloseButtonMinWidthUnselected != FLT_MAX) ? ImTrunc(TabCloseButtonMinWidthUnselected * scale_factor) : TabCloseButtonMinWidthUnselected;
    TabBarBorderSize = ImTrunc(TabBarBorderSize * scale_factor);
    TabBarOverlineSize = ImTrunc(TabBarOverlineSize * scale_factor);
    TreeLinesSize = ImTrunc(TreeLinesSize * scale_factor);
    TreeLinesRounding = ImTrunc(TreeLinesRounding * scale_factor);
    DragDropTargetRounding = ImTrunc(DragDropTargetRounding * scale_factor);
    DragDropTargetBorderSize = ImTrunc(DragDropTargetBorderSize * scale_factor);
    DragDropTargetPadding = ImTrunc(DragDropTargetPadding * scale_factor);
    ColorMarkerSize = ImTrunc(ColorMarkerSize * scale_factor);
    SeparatorSize = ImTrunc(SeparatorSize * scale_factor);
    SeparatorTextBorderSize = ImTrunc(SeparatorTextBorderSize * scale_factor);
    SeparatorTextPadding = ImTrunc(SeparatorTextPadding * scale_factor);
    DisplayWindowPadding = ImTrunc(DisplayWindowPadding * scale_factor);
    DisplaySafeAreaPadding = ImTrunc(DisplaySafeAreaPadding * scale_factor);
    MouseCursorScale = ImTrunc(MouseCursorScale * scale_factor);
}

ImGuiIO::ImGuiIO()
{

    memset((void*)this, 0, sizeof(*this));
    IM_STATIC_ASSERT(IM_COUNTOF(ImGuiIO::MouseDown) == ImGuiMouseButton_COUNT && IM_COUNTOF(ImGuiIO::MouseClicked) == ImGuiMouseButton_COUNT);

    ConfigFlags = ImGuiConfigFlags_None;
    BackendFlags = ImGuiBackendFlags_None;
    DisplaySize = ImVec2(-1.0f, -1.0f);
    DeltaTime = 1.0f / 60.0f;
    IniSavingRate = 5.0f;
    IniFilename = "imgui.ini";
    LogFilename = "imgui_log.txt";
    UserData = NULL;

    Fonts = NULL;
    FontDefault = NULL;
    FontAllowUserScaling = false;
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    FontGlobalScale = 1.0f;
#endif
    DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    ConfigNavSwapGamepadButtons = false;
    ConfigNavMoveSetMousePos = false;
    ConfigNavCaptureKeyboard = true;
    ConfigNavEscapeClearFocusItem = true;
    ConfigNavEscapeClearFocusWindow = false;
    ConfigNavCursorVisibleAuto = true;
    ConfigNavCursorVisibleAlways = false;

    MouseDrawCursor = false;
#ifdef __APPLE__
    ConfigMacOSXBehaviors = true;
#else
    ConfigMacOSXBehaviors = false;
#endif
    ConfigInputTrickleEventQueue = true;
    ConfigInputTextCursorBlink = true;
    ConfigInputTextEnterKeepActive = false;
    ConfigDragClickToInputText = false;
    ConfigWindowsResizeFromEdges = true;
    ConfigWindowsMoveFromTitleBarOnly = false;
    ConfigWindowsCopyContentsWithCtrlC = false;
    ConfigScrollbarScrollByPage = true;
    ConfigMemoryCompactTimer = 60.0f;
    ConfigDebugIsDebuggerPresent = false;
    ConfigDebugHighlightIdConflicts = true;
    ConfigDebugHighlightIdConflictsShowItemPicker = true;
    ConfigDebugBeginReturnValueOnce = false;
    ConfigDebugBeginReturnValueLoop = false;

    ConfigErrorRecovery = true;
    ConfigErrorRecoveryEnableAssert = true;
    ConfigErrorRecoveryEnableDebugLog = true;
    ConfigErrorRecoveryEnableTooltip = true;

    MouseDoubleClickTime = 0.30f;
    MouseDoubleClickMaxDist = 6.0f;
    MouseDragThreshold = 6.0f;
    KeyRepeatDelay = 0.275f;
    KeyRepeatRate = 0.050f;

    BackendPlatformName = BackendRendererName = NULL;
    BackendPlatformUserData = BackendRendererUserData = BackendLanguageUserData = NULL;

    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    MousePosPrev = ImVec2(-FLT_MAX, -FLT_MAX);
    MouseSource = ImGuiMouseSource_Mouse;
    for (int i = 0; i < IM_COUNTOF(MouseDownDuration); i++) MouseDownDuration[i] = MouseDownDurationPrev[i] = -1.0f;
    for (int i = 0; i < IM_COUNTOF(KeysData); i++) { KeysData[i].DownDuration = KeysData[i].DownDurationPrev = -1.0f; }
    AppAcceptingEvents = true;
}

void ImGuiIO::AddInputCharacter(unsigned int c)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    if (c == 0 || !AppAcceptingEvents)
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_Text;
    e.Source = ImGuiInputSource_Keyboard;
    e.EventId = g.InputEventsNextEventId++;
    e.Text.Char = c;
    g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddInputCharacterUTF16(ImWchar16 c)
{
    if ((c == 0 && InputQueueSurrogate == 0) || !AppAcceptingEvents)
        return;

    if ((c & 0xFC00) == 0xD800)
    {
        if (InputQueueSurrogate != 0)
            AddInputCharacter(IM_UNICODE_CODEPOINT_INVALID);
        InputQueueSurrogate = c;
        return;
    }

    ImWchar cp = c;
    if (InputQueueSurrogate != 0)
    {
        if ((c & 0xFC00) != 0xDC00)
        {
            AddInputCharacter(IM_UNICODE_CODEPOINT_INVALID);
        }
        else
        {
#if IM_UNICODE_CODEPOINT_MAX == 0xFFFF
            cp = IM_UNICODE_CODEPOINT_INVALID;
#else
            cp = (ImWchar)(((InputQueueSurrogate - 0xD800) << 10) + (c - 0xDC00) + 0x10000);
#endif
        }

        InputQueueSurrogate = 0;
    }
    AddInputCharacter((unsigned)cp);
}

void ImGuiIO::AddInputCharactersUTF8(const char* str)
{
    if (!AppAcceptingEvents)
        return;
    const char* str_end = str + strlen(str);
    while (*str != 0)
    {
        unsigned int c = 0;
        str += ImTextCharFromUtf8(&c, str, str_end);
        AddInputCharacter(c);
    }
}

void ImGuiIO::ClearEventsQueue()
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    g.InputEventsQueue.clear();
}

void ImGuiIO::ClearInputKeys()
{
    ImGuiContext& g = *Ctx;
    for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++)
    {
        if (ImGui::IsMouseKey((ImGuiKey)key))
            continue;
        ImGuiKeyData* key_data = &g.IO.KeysData[key - ImGuiKey_NamedKey_BEGIN];
        key_data->Down = false;
        key_data->DownDuration = -1.0f;
        key_data->DownDurationPrev = -1.0f;
    }
    KeyCtrl = KeyShift = KeyAlt = KeySuper = false;
    KeyMods = ImGuiMod_None;
    InputQueueCharacters.resize(0);
}

void ImGuiIO::ClearInputMouse()
{
    for (ImGuiKey key = ImGuiKey_Mouse_BEGIN; key < ImGuiKey_Mouse_END; key = (ImGuiKey)(key + 1))
    {
        ImGuiKeyData* key_data = &KeysData[key - ImGuiKey_NamedKey_BEGIN];
        key_data->Down = false;
        key_data->DownDuration = -1.0f;
        key_data->DownDurationPrev = -1.0f;
    }
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    for (int n = 0; n < IM_COUNTOF(MouseDown); n++)
    {
        MouseDown[n] = false;
        MouseDownDuration[n] = MouseDownDurationPrev[n] = -1.0f;
    }
    MouseWheel = MouseWheelH = 0.0f;
}

static ImGuiInputEvent* FindLatestInputEvent(ImGuiContext* ctx, ImGuiInputEventType type, int arg = -1)
{
    ImGuiContext& g = *ctx;
    for (int n = g.InputEventsQueue.Size - 1; n >= 0; n--)
    {
        ImGuiInputEvent* e = &g.InputEventsQueue[n];
        if (e->Type != type)
            continue;
        if (type == ImGuiInputEventType_Key && e->Key.Key != arg)
            continue;
        if (type == ImGuiInputEventType_MouseButton && e->MouseButton.Button != arg)
            continue;
        return e;
    }
    return NULL;
}

void ImGuiIO::AddKeyAnalogEvent(ImGuiKey key, bool down, float analog_value)
{

    IM_ASSERT(Ctx != NULL);
    if (key == ImGuiKey_None || !AppAcceptingEvents)
        return;
    ImGuiContext& g = *Ctx;
    IM_ASSERT(ImGui::IsNamedKeyOrMod(key));
    IM_ASSERT(ImGui::IsAliasKey(key) == false);

    if (g.IO.ConfigMacOSXBehaviors)
    {
        if (key == ImGuiMod_Super)          { key = ImGuiMod_Ctrl; }
        else if (key == ImGuiMod_Ctrl)      { key = ImGuiMod_Super; }
        else if (key == ImGuiKey_LeftSuper) { key = ImGuiKey_LeftCtrl; }
        else if (key == ImGuiKey_RightSuper){ key = ImGuiKey_RightCtrl; }
        else if (key == ImGuiKey_LeftCtrl)  { key = ImGuiKey_LeftSuper; }
        else if (key == ImGuiKey_RightCtrl) { key = ImGuiKey_RightSuper; }
    }

    const ImGuiInputEvent* latest_event = FindLatestInputEvent(&g, ImGuiInputEventType_Key, (int)key);
    const ImGuiKeyData* key_data = ImGui::GetKeyData(&g, key);
    const bool latest_key_down = latest_event ? latest_event->Key.Down : key_data->Down;
    const float latest_key_analog = latest_event ? latest_event->Key.AnalogValue : key_data->AnalogValue;
    if (latest_key_down == down && latest_key_analog == analog_value)
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_Key;
    e.Source = ImGui::IsGamepadKey(key) ? ImGuiInputSource_Gamepad : ImGuiInputSource_Keyboard;
    e.EventId = g.InputEventsNextEventId++;
    e.Key.Key = key;
    e.Key.Down = down;
    e.Key.AnalogValue = analog_value;
    g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddKeyEvent(ImGuiKey key, bool down)
{
    if (!AppAcceptingEvents)
        return;
    AddKeyAnalogEvent(key, down, down ? 1.0f : 0.0f);
}

void ImGuiIO::SetKeyEventNativeData(ImGuiKey key, int native_keycode, int native_scancode, int native_legacy_index)
{
    if (key == ImGuiKey_None)
        return;
    IM_ASSERT(ImGui::IsNamedKey(key));
    IM_ASSERT(native_legacy_index == -1 || ImGui::IsLegacyKey((ImGuiKey)native_legacy_index));
    IM_UNUSED(key);
    IM_UNUSED(native_keycode);
    IM_UNUSED(native_scancode);
    IM_UNUSED(native_legacy_index);
}

void ImGuiIO::SetAppAcceptingEvents(bool accepting_events)
{
    AppAcceptingEvents = accepting_events;
}

void ImGuiIO::AddMousePosEvent(float x, float y)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    if (!AppAcceptingEvents)
        return;

    ImVec2 pos((x > -FLT_MAX) ? ImFloor(x) : x, (y > -FLT_MAX) ? ImFloor(y) : y);

    const ImGuiInputEvent* latest_event = FindLatestInputEvent(&g, ImGuiInputEventType_MousePos);
    const ImVec2 latest_pos = latest_event ? ImVec2(latest_event->MousePos.PosX, latest_event->MousePos.PosY) : g.IO.MousePos;
    if (latest_pos.x == pos.x && latest_pos.y == pos.y)
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_MousePos;
    e.Source = ImGuiInputSource_Mouse;
    e.EventId = g.InputEventsNextEventId++;
    e.MousePos.PosX = pos.x;
    e.MousePos.PosY = pos.y;
    e.MousePos.MouseSource = g.InputEventsNextMouseSource;
    g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddMouseButtonEvent(int mouse_button, bool down)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    IM_ASSERT(mouse_button >= 0 && mouse_button < ImGuiMouseButton_COUNT);
    if (!AppAcceptingEvents)
        return;

    if (ConfigMacOSXBehaviors && mouse_button == 0 && MouseCtrlLeftAsRightClick)
    {

        mouse_button = 1;
        if (!down)
            MouseCtrlLeftAsRightClick = false;
    }

    const ImGuiInputEvent* latest_event = FindLatestInputEvent(&g, ImGuiInputEventType_MouseButton, (int)mouse_button);
    const bool latest_button_down = latest_event ? latest_event->MouseButton.Down : g.IO.MouseDown[mouse_button];
    if (latest_button_down == down)
        return;

    if (ConfigMacOSXBehaviors && mouse_button == 0 && down)
    {
        const ImGuiInputEvent* latest_super_event = FindLatestInputEvent(&g, ImGuiInputEventType_Key, (int)ImGuiMod_Super);
        if (latest_super_event ? latest_super_event->Key.Down : g.IO.KeySuper)
        {
            IMGUI_DEBUG_LOG_IO("[io] Super+Left Click aliased into Right Click\n");
            MouseCtrlLeftAsRightClick = true;
            AddMouseButtonEvent(1, true);
            return;
        }
    }

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_MouseButton;
    e.Source = ImGuiInputSource_Mouse;
    e.EventId = g.InputEventsNextEventId++;
    e.MouseButton.Button = mouse_button;
    e.MouseButton.Down = down;
    e.MouseButton.MouseSource = g.InputEventsNextMouseSource;
    g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddMouseWheelEvent(float wheel_x, float wheel_y)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;

    if (!AppAcceptingEvents || (wheel_x == 0.0f && wheel_y == 0.0f))
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_MouseWheel;
    e.Source = ImGuiInputSource_Mouse;
    e.EventId = g.InputEventsNextEventId++;
    e.MouseWheel.WheelX = wheel_x;
    e.MouseWheel.WheelY = wheel_y;
    e.MouseWheel.MouseSource = g.InputEventsNextMouseSource;
    g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddMouseSourceEvent(ImGuiMouseSource source)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    g.InputEventsNextMouseSource = source;
}

void ImGuiIO::AddFocusEvent(bool focused)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;

    const ImGuiInputEvent* latest_event = FindLatestInputEvent(&g, ImGuiInputEventType_Focus);
    const bool latest_focused = latest_event ? latest_event->AppFocused.Focused : !g.IO.AppFocusLost;
    if (latest_focused == focused || (ConfigDebugIgnoreFocusLoss && !focused))
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_Focus;
    e.EventId = g.InputEventsNextEventId++;
    e.AppFocused.Focused = focused;
    g.InputEventsQueue.push_back(e);
}

ImGuiPlatformIO::ImGuiPlatformIO()
{

    memset((void*)this, 0, sizeof(*this));
    Platform_LocaleDecimalPoint = '.';
}

ImVec2 ImBezierCubicClosestPoint(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& p, int num_segments)
{
    IM_ASSERT(num_segments > 0);
    ImVec2 p_last = p1;
    ImVec2 p_closest;
    float p_closest_dist2 = FLT_MAX;
    float t_step = 1.0f / (float)num_segments;
    for (int i_step = 1; i_step <= num_segments; i_step++)
    {
        ImVec2 p_current = ImBezierCubicCalc(p1, p2, p3, p4, t_step * i_step);
        ImVec2 p_line = ImLineClosestPoint(p_last, p_current, p);
        float dist2 = ImLengthSqr(p - p_line);
        if (dist2 < p_closest_dist2)
        {
            p_closest = p_line;
            p_closest_dist2 = dist2;
        }
        p_last = p_current;
    }
    return p_closest;
}

static void ImBezierCubicClosestPointCasteljauStep(const ImVec2& p, ImVec2& p_closest, ImVec2& p_last, float& p_closest_dist2, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy))
    {
        ImVec2 p_current(x4, y4);
        ImVec2 p_line = ImLineClosestPoint(p_last, p_current, p);
        float dist2 = ImLengthSqr(p - p_line);
        if (dist2 < p_closest_dist2)
        {
            p_closest = p_line;
            p_closest_dist2 = dist2;
        }
        p_last = p_current;
    }
    else if (level < 10)
    {
        float x12 = (x1 + x2)*0.5f,       y12 = (y1 + y2)*0.5f;
        float x23 = (x2 + x3)*0.5f,       y23 = (y2 + y3)*0.5f;
        float x34 = (x3 + x4)*0.5f,       y34 = (y3 + y4)*0.5f;
        float x123 = (x12 + x23)*0.5f,    y123 = (y12 + y23)*0.5f;
        float x234 = (x23 + x34)*0.5f,    y234 = (y23 + y34)*0.5f;
        float x1234 = (x123 + x234)*0.5f, y1234 = (y123 + y234)*0.5f;
        ImBezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
        ImBezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
    }
}

ImVec2 ImBezierCubicClosestPointCasteljau(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& p, float tess_tol)
{
    IM_ASSERT(tess_tol > 0.0f);
    ImVec2 p_last = p1;
    ImVec2 p_closest;
    float p_closest_dist2 = FLT_MAX;
    ImBezierCubicClosestPointCasteljauStep(p, p_closest, p_last, p_closest_dist2, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, tess_tol, 0);
    return p_closest;
}

ImVec2 ImLineClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& p)
{
    ImVec2 ap = p - a;
    ImVec2 ab_dir = b - a;
    float dot = ap.x * ab_dir.x + ap.y * ab_dir.y;
    if (dot < 0.0f)
        return a;
    float ab_len_sqr = ab_dir.x * ab_dir.x + ab_dir.y * ab_dir.y;
    if (dot > ab_len_sqr)
        return b;
    return a + ab_dir * dot / ab_len_sqr;
}

bool ImTriangleContainsPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p)
{
    bool b1 = ((p.x - b.x) * (a.y - b.y) - (p.y - b.y) * (a.x - b.x)) < 0.0f;
    bool b2 = ((p.x - c.x) * (b.y - c.y) - (p.y - c.y) * (b.x - c.x)) < 0.0f;
    bool b3 = ((p.x - a.x) * (c.y - a.y) - (p.y - a.y) * (c.x - a.x)) < 0.0f;
    return (b1 == b2) && (b2 == b3);
}

void ImTriangleBarycentricCoords(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p, float& out_u, float& out_v, float& out_w)
{
    ImVec2 v0 = b - a;
    ImVec2 v1 = c - a;
    ImVec2 v2 = p - a;
    const float denom = v0.x * v1.y - v1.x * v0.y;
    out_v = (v2.x * v1.y - v1.x * v2.y) / denom;
    out_w = (v0.x * v2.y - v2.x * v0.y) / denom;
    out_u = 1.0f - out_v - out_w;
}

ImVec2 ImTriangleClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p)
{
    ImVec2 proj_ab = ImLineClosestPoint(a, b, p);
    ImVec2 proj_bc = ImLineClosestPoint(b, c, p);
    ImVec2 proj_ca = ImLineClosestPoint(c, a, p);
    float dist2_ab = ImLengthSqr(p - proj_ab);
    float dist2_bc = ImLengthSqr(p - proj_bc);
    float dist2_ca = ImLengthSqr(p - proj_ca);
    float m = ImMin(dist2_ab, ImMin(dist2_bc, dist2_ca));
    if (m == dist2_ab)
        return proj_ab;
    if (m == dist2_bc)
        return proj_bc;
    return proj_ca;
}

int ImStricmp(const char* str1, const char* str2)
{
    int d;
    while ((d = ImToUpper(*str2) - ImToUpper(*str1)) == 0 && *str1) { str1++; str2++; }
    return d;
}

int ImStrnicmp(const char* str1, const char* str2, size_t count)
{
    int d = 0;
    while (count > 0 && (d = ImToUpper(*str2) - ImToUpper(*str1)) == 0 && *str1) { str1++; str2++; count--; }
    return d;
}

void ImStrncpy(char* dst, const char* src, size_t count)
{
    if (count < 1)
        return;
    if (count > 1)
        strncpy(dst, src, count - 1);
    dst[count - 1] = 0;
}

char* ImStrdup(const char* str)
{
    size_t len = ImStrlen(str);
    void* buf = IM_ALLOC(len + 1);
    return (char*)memcpy(buf, (const void*)str, len + 1);
}

void* ImMemdup(const void* src, size_t size)
{
    void* dst = IM_ALLOC(size);
    return memcpy(dst, src, size);
}

char* ImStrdupcpy(char* dst, size_t* p_dst_size, const char* src)
{
    size_t dst_buf_size = p_dst_size ? *p_dst_size : ImStrlen(dst) + 1;
    size_t src_size = ImStrlen(src) + 1;
    if (dst_buf_size < src_size)
    {
        IM_FREE(dst);
        dst = (char*)IM_ALLOC(src_size);
        if (p_dst_size)
            *p_dst_size = src_size;
    }
    return (char*)memcpy(dst, (const void*)src, src_size);
}

const char* ImStrchrRange(const char* str, const char* str_end, char c)
{
    const char* p = (const char*)ImMemchr(str, (int)c, str_end - str);
    return p;
}

int ImStrlenW(const ImWchar* str)
{

    int n = 0;
    while (*str++) n++;
    return n;
}

const char* ImStreolRange(const char* str, const char* str_end)
{
    const char* p = (const char*)ImMemchr(str, '\n', str_end - str);
    return p ? p : str_end;
}

const char* ImStrbol(const char* buf_mid_line, const char* buf_begin)
{
    IM_ASSERT_PARANOID(buf_mid_line >= buf_begin && buf_mid_line <= buf_begin + ImStrlen(buf_begin));
    while (buf_mid_line > buf_begin && buf_mid_line[-1] != '\n')
        buf_mid_line--;
    return buf_mid_line;
}

const char* ImStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end)
{
    if (!needle_end)
        needle_end = needle + ImStrlen(needle);

    const char un0 = (char)ImToUpper(*needle);
    while ((!haystack_end && *haystack) || (haystack_end && haystack < haystack_end))
    {
        if (ImToUpper(*haystack) == un0)
        {
            const char* b = needle + 1;
            for (const char* a = haystack + 1; b < needle_end; a++, b++)
                if (ImToUpper(*a) != ImToUpper(*b))
                    break;
            if (b == needle_end)
                return haystack;
        }
        haystack++;
    }
    return NULL;
}

void ImStrTrimBlanks(char* buf)
{
    char* p = buf;
    while (p[0] == ' ' || p[0] == '\t')
        p++;
    char* p_start = p;
    while (*p != 0)
        p++;
    while (p > p_start && (p[-1] == ' ' || p[-1] == '\t'))
        p--;
    if (p_start != buf)
        memmove(buf, p_start, p - p_start);
    buf[p - p_start] = 0;
}

const char* ImStrSkipBlank(const char* str)
{
    while (str[0] == ' ' || str[0] == '\t')
        str++;
    return str;
}

#ifndef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS

#ifdef IMGUI_USE_STB_SPRINTF
#ifndef IMGUI_DISABLE_STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_SPRINTF_FILENAME
#include IMGUI_STB_SPRINTF_FILENAME
#else
#include "stb_sprintf.h"
#endif
#endif

#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf _vsnprintf
#endif

int ImFormatString(char* buf, size_t buf_size, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#ifdef IMGUI_USE_STB_SPRINTF
    int w = stbsp_vsnprintf(buf, (int)buf_size, fmt, args);
#else
    int w = vsnprintf(buf, buf_size, fmt, args);
#endif
    va_end(args);
    if (buf == NULL)
        return w;
    if (w == -1 || w >= (int)buf_size)
        w = (int)buf_size - 1;
    buf[w] = 0;
    return w;
}

int ImFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args)
{
#ifdef IMGUI_USE_STB_SPRINTF
    int w = stbsp_vsnprintf(buf, (int)buf_size, fmt, args);
#else
    int w = vsnprintf(buf, buf_size, fmt, args);
#endif
    if (buf == NULL)
        return w;
    if (w == -1 || w >= (int)buf_size)
        w = (int)buf_size - 1;
    buf[w] = 0;
    return w;
}
#endif

void ImFormatStringToTempBuffer(const char** out_buf, const char** out_buf_end, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ImFormatStringToTempBufferV(out_buf, out_buf_end, fmt, args);
    va_end(args);
}

void ImFormatStringToTempBufferV(const char** out_buf, const char** out_buf_end, const char* fmt, va_list args)
{
    ImGuiContext& g = *GImGui;
    if (fmt[0] == '%' && fmt[1] == 's' && fmt[2] == 0)
    {
        const char* buf = va_arg(args, const char*);
        if (buf == NULL)
            buf = "(null)";
        *out_buf = buf;
        if (out_buf_end) { *out_buf_end = buf + ImStrlen(buf); }
    }
    else if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '*' && fmt[3] == 's' && fmt[4] == 0)
    {
        int buf_len = va_arg(args, int);
        const char* buf = va_arg(args, const char*);
        if (buf == NULL)
        {
            buf = "(null)";
            buf_len = ImMin(buf_len, 6);
        }
        *out_buf = buf;
        *out_buf_end = buf + buf_len;
    }
    else
    {
        int buf_len = ImFormatStringV(g.TempBuffer.Data, g.TempBuffer.Size, fmt, args);
        *out_buf = g.TempBuffer.Data;
        if (out_buf_end) { *out_buf_end = g.TempBuffer.Data + buf_len; }
    }
}

#ifndef IMGUI_ENABLE_SSE4_2_CRC

static const ImU32 GCrc32LookupTable[256] =
{
#ifdef IMGUI_USE_LEGACY_CRC32_ADLER

    0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
    0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
    0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
    0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
    0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
    0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
    0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
    0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
    0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
    0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
    0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
    0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
    0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
    0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
    0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
    0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
#else

    0x00000000,0xF26B8303,0xE13B70F7,0x1350F3F4,0xC79A971F,0x35F1141C,0x26A1E7E8,0xD4CA64EB,0x8AD958CF,0x78B2DBCC,0x6BE22838,0x9989AB3B,0x4D43CFD0,0xBF284CD3,0xAC78BF27,0x5E133C24,
    0x105EC76F,0xE235446C,0xF165B798,0x030E349B,0xD7C45070,0x25AFD373,0x36FF2087,0xC494A384,0x9A879FA0,0x68EC1CA3,0x7BBCEF57,0x89D76C54,0x5D1D08BF,0xAF768BBC,0xBC267848,0x4E4DFB4B,
    0x20BD8EDE,0xD2D60DDD,0xC186FE29,0x33ED7D2A,0xE72719C1,0x154C9AC2,0x061C6936,0xF477EA35,0xAA64D611,0x580F5512,0x4B5FA6E6,0xB93425E5,0x6DFE410E,0x9F95C20D,0x8CC531F9,0x7EAEB2FA,
    0x30E349B1,0xC288CAB2,0xD1D83946,0x23B3BA45,0xF779DEAE,0x05125DAD,0x1642AE59,0xE4292D5A,0xBA3A117E,0x4851927D,0x5B016189,0xA96AE28A,0x7DA08661,0x8FCB0562,0x9C9BF696,0x6EF07595,
    0x417B1DBC,0xB3109EBF,0xA0406D4B,0x522BEE48,0x86E18AA3,0x748A09A0,0x67DAFA54,0x95B17957,0xCBA24573,0x39C9C670,0x2A993584,0xD8F2B687,0x0C38D26C,0xFE53516F,0xED03A29B,0x1F682198,
    0x5125DAD3,0xA34E59D0,0xB01EAA24,0x42752927,0x96BF4DCC,0x64D4CECF,0x77843D3B,0x85EFBE38,0xDBFC821C,0x2997011F,0x3AC7F2EB,0xC8AC71E8,0x1C661503,0xEE0D9600,0xFD5D65F4,0x0F36E6F7,
    0x61C69362,0x93AD1061,0x80FDE395,0x72966096,0xA65C047D,0x5437877E,0x4767748A,0xB50CF789,0xEB1FCBAD,0x197448AE,0x0A24BB5A,0xF84F3859,0x2C855CB2,0xDEEEDFB1,0xCDBE2C45,0x3FD5AF46,
    0x7198540D,0x83F3D70E,0x90A324FA,0x62C8A7F9,0xB602C312,0x44694011,0x5739B3E5,0xA55230E6,0xFB410CC2,0x092A8FC1,0x1A7A7C35,0xE811FF36,0x3CDB9BDD,0xCEB018DE,0xDDE0EB2A,0x2F8B6829,
    0x82F63B78,0x709DB87B,0x63CD4B8F,0x91A6C88C,0x456CAC67,0xB7072F64,0xA457DC90,0x563C5F93,0x082F63B7,0xFA44E0B4,0xE9141340,0x1B7F9043,0xCFB5F4A8,0x3DDE77AB,0x2E8E845F,0xDCE5075C,
    0x92A8FC17,0x60C37F14,0x73938CE0,0x81F80FE3,0x55326B08,0xA759E80B,0xB4091BFF,0x466298FC,0x1871A4D8,0xEA1A27DB,0xF94AD42F,0x0B21572C,0xDFEB33C7,0x2D80B0C4,0x3ED04330,0xCCBBC033,
    0xA24BB5A6,0x502036A5,0x4370C551,0xB11B4652,0x65D122B9,0x97BAA1BA,0x84EA524E,0x7681D14D,0x2892ED69,0xDAF96E6A,0xC9A99D9E,0x3BC21E9D,0xEF087A76,0x1D63F975,0x0E330A81,0xFC588982,
    0xB21572C9,0x407EF1CA,0x532E023E,0xA145813D,0x758FE5D6,0x87E466D5,0x94B49521,0x66DF1622,0x38CC2A06,0xCAA7A905,0xD9F75AF1,0x2B9CD9F2,0xFF56BD19,0x0D3D3E1A,0x1E6DCDEE,0xEC064EED,
    0xC38D26C4,0x31E6A5C7,0x22B65633,0xD0DDD530,0x0417B1DB,0xF67C32D8,0xE52CC12C,0x1747422F,0x49547E0B,0xBB3FFD08,0xA86F0EFC,0x5A048DFF,0x8ECEE914,0x7CA56A17,0x6FF599E3,0x9D9E1AE0,
    0xD3D3E1AB,0x21B862A8,0x32E8915C,0xC083125F,0x144976B4,0xE622F5B7,0xF5720643,0x07198540,0x590AB964,0xAB613A67,0xB831C993,0x4A5A4A90,0x9E902E7B,0x6CFBAD78,0x7FAB5E8C,0x8DC0DD8F,
    0xE330A81A,0x115B2B19,0x020BD8ED,0xF0605BEE,0x24AA3F05,0xD6C1BC06,0xC5914FF2,0x37FACCF1,0x69E9F0D5,0x9B8273D6,0x88D28022,0x7AB90321,0xAE7367CA,0x5C18E4C9,0x4F48173D,0xBD23943E,
    0xF36E6F75,0x0105EC76,0x12551F82,0xE03E9C81,0x34F4F86A,0xC69F7B69,0xD5CF889D,0x27A40B9E,0x79B737BA,0x8BDCB4B9,0x988C474D,0x6AE7C44E,0xBE2DA0A5,0x4C4623A6,0x5F16D052,0xAD7D5351
#endif
};
#endif

ImGuiID ImHashData(const void* data_p, size_t data_size, ImGuiID seed)
{
    ImU32 crc = ~seed;
    const unsigned char* data = (const unsigned char*)data_p;
    const unsigned char *data_end = (const unsigned char*)data_p + data_size;
#ifndef IMGUI_ENABLE_SSE4_2_CRC
    const ImU32* crc32_lut = GCrc32LookupTable;
    while (data < data_end)
        crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ *data++];
    return ~crc;
#else
    while (data + 4 <= data_end)
    {
        crc = _mm_crc32_u32(crc, *(ImU32*)data);
        data += 4;
    }
    while (data < data_end)
        crc = _mm_crc32_u8(crc, *data++);
    return ~crc;
#endif
}

ImGuiID ImHashStr(const char* data_p, size_t data_size, ImGuiID seed)
{
    seed = ~seed;
    ImU32 crc = seed;
    const unsigned char* data = (const unsigned char*)data_p;
#ifndef IMGUI_ENABLE_SSE4_2_CRC
    const ImU32* crc32_lut = GCrc32LookupTable;
#endif
    if (data_size != 0)
    {
        while (data_size-- > 0)
        {
            unsigned char c = *data++;
            if (c == '#' && data_size >= 2 && data[0] == '#' && data[1] == '#')
            {
                crc = seed;
                data += 2;
                data_size -= 2;
                continue;
            }
#ifndef IMGUI_ENABLE_SSE4_2_CRC
            crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
#else
            crc = _mm_crc32_u8(crc, c);
#endif
        }
    }
    else
    {
        while (unsigned char c = *data++)
        {
            if (c == '#' && data[0] == '#' && data[1] == '#')
            {
                crc = seed;
                data += 2;
                continue;
            }
#ifndef IMGUI_ENABLE_SSE4_2_CRC
            crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
#else
            crc = _mm_crc32_u8(crc, c);
#endif
        }
    }
    return ~crc;
}

const char* ImHashSkipUncontributingPrefix(const char* label)
{
    const char* result = label;
    while (unsigned char c = *label++)
        if (c == '#' && label[0] == '#' && label[1] == '#')
            result = label + 2;
    return result;
}

#ifndef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS

ImFileHandle ImFileOpen(const char* filename, const char* mode)
{
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && (defined(__MINGW32__) || (!defined(__CYGWIN__) && !defined(__GNUC__)))

    const int filename_wsize = ::MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
    const int mode_wsize = ::MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);

    wchar_t local_temp_stack[FILENAME_MAX];
    ImVector<wchar_t> local_temp_heap;
    if (filename_wsize + mode_wsize > IM_COUNTOF(local_temp_stack))
        local_temp_heap.resize(filename_wsize + mode_wsize);
    wchar_t* filename_wbuf = local_temp_heap.Data ? local_temp_heap.Data : local_temp_stack;
    wchar_t* mode_wbuf = filename_wbuf + filename_wsize;
    ::MultiByteToWideChar(CP_UTF8, 0, filename, -1, filename_wbuf, filename_wsize);
    ::MultiByteToWideChar(CP_UTF8, 0, mode, -1, mode_wbuf, mode_wsize);
    return ::_wfopen(filename_wbuf, mode_wbuf);
#else
    return fopen(filename, mode);
#endif
}

bool    ImFileClose(ImFileHandle f)     { return fclose(f) == 0; }
ImU64   ImFileGetSize(ImFileHandle f)   { long off = 0, sz = 0; return ((off = ftell(f)) != -1 && !fseek(f, 0, SEEK_END) && (sz = ftell(f)) != -1 && !fseek(f, off, SEEK_SET)) ? (ImU64)sz : (ImU64)-1; }
ImU64   ImFileRead(void* data, ImU64 sz, ImU64 count, ImFileHandle f)           { return fread(data, (size_t)sz, (size_t)count, f); }
ImU64   ImFileWrite(const void* data, ImU64 sz, ImU64 count, ImFileHandle f)    { return fwrite(data, (size_t)sz, (size_t)count, f); }
#endif

void*   ImFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size, int padding_bytes)
{
    IM_ASSERT(filename && mode);
    if (out_file_size)
        *out_file_size = 0;

    ImFileHandle f;
    if ((f = ImFileOpen(filename, mode)) == NULL)
        return NULL;

    size_t file_size = (size_t)ImFileGetSize(f);
    if (file_size == (size_t)-1)
    {
        ImFileClose(f);
        return NULL;
    }

    void* file_data = IM_ALLOC(file_size + padding_bytes);
    if (file_data == NULL)
    {
        ImFileClose(f);
        return NULL;
    }
    if (ImFileRead(file_data, 1, file_size, f) != file_size)
    {
        ImFileClose(f);
        IM_FREE(file_data);
        return NULL;
    }
    if (padding_bytes > 0)
        memset((void*)(((char*)file_data) + file_size), 0, (size_t)padding_bytes);

    ImFileClose(f);
    if (out_file_size)
        *out_file_size = file_size;

    return file_data;
}

IM_MSVC_RUNTIME_CHECKS_OFF

int ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end)
{
    static const char lengths[32] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };
    static const int masks[]  = { 0x00, 0x7f, 0x1f, 0x0f, 0x07 };
    static const uint32_t mins[] = { 0x400000, 0, 0x80, 0x800, 0x10000 };
    static const int shiftc[] = { 0, 18, 12, 6, 0 };
    static const int shifte[] = { 0, 6, 4, 2, 0 };
    int len = lengths[*(const unsigned char*)in_text >> 3];
    int wanted = len + (len ? 0 : 1);

    if (in_text_end == NULL)
        in_text_end = in_text + wanted;

    unsigned char s[4];
    s[0] = in_text + 0 < in_text_end ? in_text[0] : 0;
    s[1] = in_text + 1 < in_text_end ? in_text[1] : 0;
    s[2] = in_text + 2 < in_text_end ? in_text[2] : 0;
    s[3] = in_text + 3 < in_text_end ? in_text[3] : 0;

    *out_char  = (uint32_t)(s[0] & masks[len]) << 18;
    *out_char |= (uint32_t)(s[1] & 0x3f) << 12;
    *out_char |= (uint32_t)(s[2] & 0x3f) <<  6;
    *out_char |= (uint32_t)(s[3] & 0x3f) <<  0;
    *out_char >>= shiftc[len];

    int e = 0;
    e  = (*out_char < mins[len]) << 6;
    e |= ((*out_char >> 11) == 0x1b) << 7;
    e |= (*out_char > IM_UNICODE_CODEPOINT_MAX) << 8;
    e |= (s[1] & 0xc0) >> 2;
    e |= (s[2] & 0xc0) >> 4;
    e |= (s[3]       ) >> 6;
    e ^= 0x2a;
    e >>= shifte[len];

    if (e)
    {

        wanted = ImMin(wanted, !!s[0] + !!s[1] + !!s[2] + !!s[3]);
        *out_char = IM_UNICODE_CODEPOINT_INVALID;
    }

    return wanted;
}

int ImTextStrFromUtf8(ImWchar* buf, int buf_size, const char* in_text, const char* in_text_end, const char** in_text_remaining)
{
    ImWchar* buf_out = buf;
    ImWchar* buf_end = buf + buf_size;
    while (buf_out < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c;
        in_text += ImTextCharFromUtf8(&c, in_text, in_text_end);
        *buf_out++ = (ImWchar)c;
    }
    *buf_out = 0;
    if (in_text_remaining)
        *in_text_remaining = in_text;
    return (int)(buf_out - buf);
}

int ImTextCountCharsFromUtf8(const char* in_text, const char* in_text_end)
{
    int char_count = 0;
    while ((!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c;
        in_text += ImTextCharFromUtf8(&c, in_text, in_text_end);
        char_count++;
    }
    return char_count;
}

static inline int ImTextCharToUtf8_inline(char* buf, int buf_size, unsigned int c)
{
    if (c < 0x80)
    {
        buf[0] = (char)c;
        return 1;
    }
    if (c < 0x800)
    {
        if (buf_size < 2) return 0;
        buf[0] = (char)(0xc0 + (c >> 6));
        buf[1] = (char)(0x80 + (c & 0x3f));
        return 2;
    }
    if (c < 0x10000)
    {
        if (buf_size < 3) return 0;
        buf[0] = (char)(0xe0 + (c >> 12));
        buf[1] = (char)(0x80 + ((c >> 6) & 0x3f));
        buf[2] = (char)(0x80 + ((c ) & 0x3f));
        return 3;
    }
    if (c <= 0x10FFFF)
    {
        if (buf_size < 4) return 0;
        buf[0] = (char)(0xf0 + (c >> 18));
        buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
        buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
        buf[3] = (char)(0x80 + ((c ) & 0x3f));
        return 4;
    }

    return 0;
}

int ImTextCharToUtf8(char out_buf[5], unsigned int c)
{
    int count = ImTextCharToUtf8_inline(out_buf, 5, c);
    out_buf[count] = 0;
    return count;
}

int ImTextCountUtf8BytesFromChar(const char* in_text, const char* in_text_end)
{
    unsigned int unused = 0;
    return ImTextCharFromUtf8(&unused, in_text, in_text_end);
}

static inline int ImTextCountUtf8BytesFromChar(unsigned int c)
{
    if (c < 0x80) return 1;
    if (c < 0x800) return 2;
    if (c < 0x10000) return 3;
    if (c <= 0x10FFFF) return 4;
    return 3;
}

int ImTextStrToUtf8(char* out_buf, int out_buf_size, const ImWchar* in_text, const ImWchar* in_text_end)
{
    char* buf_p = out_buf;
    const char* buf_end = out_buf + out_buf_size;
    while (buf_p < buf_end - 1 && (!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c = (unsigned int)(*in_text++);
        if (c < 0x80)
            *buf_p++ = (char)c;
        else
            buf_p += ImTextCharToUtf8_inline(buf_p, (int)(buf_end - buf_p - 1), c);
    }
    *buf_p = 0;
    return (int)(buf_p - out_buf);
}

int ImTextCountUtf8BytesFromStr(const ImWchar* in_text, const ImWchar* in_text_end)
{
    int bytes_count = 0;
    while ((!in_text_end || in_text < in_text_end) && *in_text)
    {
        unsigned int c = (unsigned int)(*in_text++);
        if (c < 0x80)
            bytes_count++;
        else
            bytes_count += ImTextCountUtf8BytesFromChar(c);
    }
    return bytes_count;
}

const char* ImTextFindPreviousUtf8Codepoint(const char* in_text_start, const char* in_p)
{
    while (in_p > in_text_start)
    {
        in_p--;
        if ((*in_p & 0xC0) != 0x80)
            return in_p;
    }
    return in_text_start;
}

const char* ImTextFindValidUtf8CodepointEnd(const char* in_text_start, const char* in_text_end, const char* in_p)
{
    if (in_text_start == in_p)
        return in_text_start;
    const char* prev = ImTextFindPreviousUtf8Codepoint(in_text_start, in_p);
    unsigned int prev_c;
    int prev_c_len = ImTextCharFromUtf8(&prev_c, prev, in_text_end);
    if (prev_c != IM_UNICODE_CODEPOINT_INVALID && prev_c_len <= (int)(in_p - prev))
        return in_p;
    return prev;
}

int ImTextCountLines(const char* in_text, const char* in_text_end)
{
    if (in_text_end == NULL)
        in_text_end = in_text + ImStrlen(in_text);
    int count = 0;
    while (in_text < in_text_end)
    {
        const char* line_end = (const char*)ImMemchr(in_text, '\n', in_text_end - in_text);
        in_text = line_end ? line_end + 1 : in_text_end;
        count++;
    }
    return count;
}

IM_MSVC_RUNTIME_CHECKS_RESTORE

IMGUI_API ImU32 ImAlphaBlendColors(ImU32 col_a, ImU32 col_b)
{
    float t = ((col_b >> IM_COL32_A_SHIFT) & 0xFF) / 255.f;
    int r = ImLerp((int)(col_a >> IM_COL32_R_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_R_SHIFT) & 0xFF, t);
    int g = ImLerp((int)(col_a >> IM_COL32_G_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_G_SHIFT) & 0xFF, t);
    int b = ImLerp((int)(col_a >> IM_COL32_B_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_B_SHIFT) & 0xFF, t);
    return IM_COL32(r, g, b, 0xFF);
}

ImVec4 ImGui::ColorConvertU32ToFloat4(ImU32 in)
{
    float s = 1.0f / 255.0f;
    return ImVec4(
        ((in >> IM_COL32_R_SHIFT) & 0xFF) * s,
        ((in >> IM_COL32_G_SHIFT) & 0xFF) * s,
        ((in >> IM_COL32_B_SHIFT) & 0xFF) * s,
        ((in >> IM_COL32_A_SHIFT) & 0xFF) * s);
}

ImU32 ImGui::ColorConvertFloat4ToU32(const ImVec4& in)
{
    ImU32 out;
    out  = ((ImU32)IM_F32_TO_INT8_SAT(in.x)) << IM_COL32_R_SHIFT;
    out |= ((ImU32)IM_F32_TO_INT8_SAT(in.y)) << IM_COL32_G_SHIFT;
    out |= ((ImU32)IM_F32_TO_INT8_SAT(in.z)) << IM_COL32_B_SHIFT;
    out |= ((ImU32)IM_F32_TO_INT8_SAT(in.w)) << IM_COL32_A_SHIFT;
    return out;
}

void ImGui::ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v)
{
    float K = 0.f;
    if (g < b)
    {
        ImSwap(g, b);
        K = -1.f;
    }
    if (r < g)
    {
        ImSwap(r, g);
        K = -2.f / 6.f - K;
    }

    const float chroma = r - (g < b ? g : b);
    out_h = ImFabs(K + (g - b) / (6.f * chroma + 1e-20f));
    out_s = chroma / (r + 1e-20f);
    out_v = r;
}

void ImGui::ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
    if (s == 0.0f)
    {

        out_r = out_g = out_b = v;
        return;
    }

    h = ImFmod(h, 1.0f) / (60.0f / 360.0f);
    int   i = (int)h;
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i)
    {
    case 0: out_r = v; out_g = t; out_b = p; break;
    case 1: out_r = q; out_g = v; out_b = p; break;
    case 2: out_r = p; out_g = v; out_b = t; break;
    case 3: out_r = p; out_g = q; out_b = v; break;
    case 4: out_r = t; out_g = p; out_b = v; break;
    case 5: default: out_r = v; out_g = p; out_b = q; break;
    }
}

ImGuiStoragePair* ImLowerBound(ImGuiStoragePair* in_begin, ImGuiStoragePair* in_end, ImGuiID key)
{
    ImGuiStoragePair* in_p = in_begin;
    for (size_t count = (size_t)(in_end - in_p); count > 0; )
    {
        size_t count2 = count >> 1;
        ImGuiStoragePair* mid = in_p + count2;
        if (mid->key < key)
        {
            in_p = ++mid;
            count -= count2 + 1;
        }
        else
        {
            count = count2;
        }
    }
    return in_p;
}

IM_MSVC_RUNTIME_CHECKS_OFF
static int IMGUI_CDECL PairComparerByID(const void* lhs, const void* rhs)
{

    ImGuiID lhs_v = ((const ImGuiStoragePair*)lhs)->key;
    ImGuiID rhs_v = ((const ImGuiStoragePair*)rhs)->key;
    return (lhs_v > rhs_v ? +1 : lhs_v < rhs_v ? -1 : 0);
}

void ImGuiStorage::BuildSortByKey()
{
    ImQsort(Data.Data, (size_t)Data.Size, sizeof(ImGuiStoragePair), PairComparerByID);
}

int ImGuiStorage::GetInt(ImGuiID key, int default_val) const
{
    ImGuiStoragePair* it = ImLowerBound(const_cast<ImGuiStoragePair*>(Data.Data), const_cast<ImGuiStoragePair*>(Data.Data + Data.Size), key);
    if (it == Data.Data + Data.Size || it->key != key)
        return default_val;
    return it->val_i;
}

bool ImGuiStorage::GetBool(ImGuiID key, bool default_val) const
{
    return GetInt(key, default_val ? 1 : 0) != 0;
}

float ImGuiStorage::GetFloat(ImGuiID key, float default_val) const
{
    ImGuiStoragePair* it = ImLowerBound(const_cast<ImGuiStoragePair*>(Data.Data), const_cast<ImGuiStoragePair*>(Data.Data + Data.Size), key);
    if (it == Data.Data + Data.Size || it->key != key)
        return default_val;
    return it->val_f;
}

void* ImGuiStorage::GetVoidPtr(ImGuiID key) const
{
    ImGuiStoragePair* it = ImLowerBound(const_cast<ImGuiStoragePair*>(Data.Data), const_cast<ImGuiStoragePair*>(Data.Data + Data.Size), key);
    if (it == Data.Data + Data.Size || it->key != key)
        return NULL;
    return it->val_p;
}

int* ImGuiStorage::GetIntRef(ImGuiID key, int default_val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        it = Data.insert(it, ImGuiStoragePair(key, default_val));
    return &it->val_i;
}

bool* ImGuiStorage::GetBoolRef(ImGuiID key, bool default_val)
{
    return (bool*)GetIntRef(key, default_val ? 1 : 0);
}

float* ImGuiStorage::GetFloatRef(ImGuiID key, float default_val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        it = Data.insert(it, ImGuiStoragePair(key, default_val));
    return &it->val_f;
}

void** ImGuiStorage::GetVoidPtrRef(ImGuiID key, void* default_val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        it = Data.insert(it, ImGuiStoragePair(key, default_val));
    return &it->val_p;
}

void ImGuiStorage::SetInt(ImGuiID key, int val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        Data.insert(it, ImGuiStoragePair(key, val));
    else
        it->val_i = val;
}

void ImGuiStorage::SetBool(ImGuiID key, bool val)
{
    SetInt(key, val ? 1 : 0);
}

void ImGuiStorage::SetFloat(ImGuiID key, float val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        Data.insert(it, ImGuiStoragePair(key, val));
    else
        it->val_f = val;
}

void ImGuiStorage::SetVoidPtr(ImGuiID key, void* val)
{
    ImGuiStoragePair* it = ImLowerBound(Data.Data, Data.Data + Data.Size, key);
    if (it == Data.Data + Data.Size || it->key != key)
        Data.insert(it, ImGuiStoragePair(key, val));
    else
        it->val_p = val;
}

void ImGuiStorage::SetAllInt(int v)
{
    for (int i = 0; i < Data.Size; i++)
        Data[i].val_i = v;
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

ImGuiTextFilter::ImGuiTextFilter(const char* default_filter)
{
    InputBuf[0] = 0;
    CountGrep = 0;
    if (default_filter)
    {
        ImStrncpy(InputBuf, default_filter, IM_COUNTOF(InputBuf));
        Build();
    }
}

bool ImGuiTextFilter::Draw(const char* label, float width)
{
    if (width != 0.0f)
        ImGui::SetNextItemWidth(width);
    bool value_changed = ImGui::InputText(label, InputBuf, IM_COUNTOF(InputBuf));
    if (value_changed)
        Build();
    return value_changed;
}

void ImGuiTextFilter::ImGuiTextRange::split(char separator, ImVector<ImGuiTextRange>* out) const
{
    out->resize(0);
    const char* wb = b;
    const char* we = wb;
    while (we < e)
    {
        if (*we == separator)
        {
            out->push_back(ImGuiTextRange(wb, we));
            wb = we + 1;
        }
        we++;
    }
    if (wb != we)
        out->push_back(ImGuiTextRange(wb, we));
}

void ImGuiTextFilter::Build()
{
    Filters.resize(0);
    ImGuiTextRange input_range(InputBuf, InputBuf + ImStrlen(InputBuf));
    input_range.split(',', &Filters);

    CountGrep = 0;
    for (ImGuiTextRange& f : Filters)
    {
        while (f.b < f.e && ImCharIsBlankA(f.b[0]))
            f.b++;
        while (f.e > f.b && ImCharIsBlankA(f.e[-1]))
            f.e--;
        if (f.empty())
            continue;
        if (f.b[0] != '-')
            CountGrep += 1;
    }
}

bool ImGuiTextFilter::PassFilter(const char* text, const char* text_end) const
{
    if (Filters.Size == 0)
        return true;

    if (text == NULL)
        text = text_end = "";

    for (const ImGuiTextRange& f : Filters)
    {
        if (f.b == f.e)
            continue;
        if (f.b[0] == '-')
        {

            if (ImStristr(text, text_end, f.b + 1, f.e) != NULL)
                return false;
        }
        else
        {

            if (ImStristr(text, text_end, f.b, f.e) != NULL)
                return true;
        }
    }

    if (CountGrep == 0)
        return true;

    return false;
}

#ifndef va_copy
#if defined(__GNUC__) || defined(__clang__)
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#else
#define va_copy(dest, src) (dest = src)
#endif
#endif

char ImGuiTextBuffer::EmptyString[1] = { 0 };

void ImGuiTextBuffer::append(const char* str, const char* str_end)
{
    int len = str_end ? (int)(str_end - str) : (int)ImStrlen(str);

    const int write_off = (Buf.Size != 0) ? Buf.Size : 1;
    const int needed_sz = write_off + len;
    if (write_off + len >= Buf.Capacity)
    {
        int new_capacity = Buf.Capacity * 2;
        Buf.reserve(needed_sz > new_capacity ? needed_sz : new_capacity);
    }

    Buf.resize(needed_sz);
    memcpy(&Buf[write_off - 1], str, (size_t)len);
    Buf[write_off - 1 + len] = 0;
}

void ImGuiTextBuffer::appendf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    appendfv(fmt, args);
    va_end(args);
}

void ImGuiTextBuffer::appendfv(const char* fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    int len = ImFormatStringV(NULL, 0, fmt, args);
    if (len <= 0)
    {
        va_end(args_copy);
        return;
    }

    const int write_off = (Buf.Size != 0) ? Buf.Size : 1;
    const int needed_sz = write_off + len;
    if (write_off + len >= Buf.Capacity)
    {
        int new_capacity = Buf.Capacity * 2;
        Buf.reserve(needed_sz > new_capacity ? needed_sz : new_capacity);
    }

    Buf.resize(needed_sz);
    ImFormatStringV(&Buf[write_off - 1], (size_t)len + 1, fmt, args_copy);
    va_end(args_copy);
}

IM_MSVC_RUNTIME_CHECKS_OFF
void ImGuiTextIndex::append(const char* base, int old_size, int new_size)
{
    IM_ASSERT(old_size >= 0 && new_size >= old_size && new_size >= EndOffset);
    if (old_size == new_size)
        return;
    if (EndOffset == 0 || base[EndOffset - 1] == '\n')
        Offsets.push_back(EndOffset);
    const char* base_end = base + new_size;
    for (const char* p = base + old_size; (p = (const char*)ImMemchr(p, '\n', base_end - p)) != 0; )
        if (++p < base_end)
            Offsets.push_back((int)(intptr_t)(p - base));
    EndOffset = ImMax(EndOffset, new_size);
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

static bool GetSkipItemForListClipping()
{
    ImGuiContext& g = *GImGui;
    return g.CurrentTable ? g.CurrentTable->HostSkipItems : g.CurrentWindow->SkipItems;
}

static void ImGuiListClipper_SortAndFuseRanges(ImVector<ImGuiListClipperRange>& ranges, int offset = 0)
{
    if (ranges.Size - offset <= 1)
        return;

    for (int sort_end = ranges.Size - offset - 1; sort_end > 0; --sort_end)
        for (int i = offset; i < sort_end + offset; ++i)
            if (ranges[i].Min > ranges[i + 1].Min)
                ImSwap(ranges[i], ranges[i + 1]);

    for (int i = 1 + offset; i < ranges.Size; i++)
    {
        IM_ASSERT(!ranges[i].PosToIndexConvert && !ranges[i - 1].PosToIndexConvert);
        if (ranges[i - 1].Max < ranges[i].Min)
            continue;
        ranges[i - 1].Min = ImMin(ranges[i - 1].Min, ranges[i].Min);
        ranges[i - 1].Max = ImMax(ranges[i - 1].Max, ranges[i].Max);
        ranges.erase(ranges.Data + i);
        i--;
    }
}

static void ImGuiListClipper_SeekCursorAndSetupPrevLine(ImGuiListClipper* clipper, float pos_y, float line_height)
{

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float off_y = pos_y - window->DC.CursorPos.y;
    window->DC.CursorPos.y = pos_y;
    window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, pos_y - g.Style.ItemSpacing.y);
    window->DC.CursorPosPrevLine.y = window->DC.CursorPos.y - line_height;
    window->DC.PrevLineSize.y = (line_height - g.Style.ItemSpacing.y);
    if (ImGuiOldColumns* columns = window->DC.CurrentColumns)
        columns->LineMinY = window->DC.CursorPos.y;
    if (ImGuiTable* table = g.CurrentTable)
    {
        if (table->IsInsideRow)
            ImGui::TableEndRow(table);
        const int row_increase = (int)((off_y / line_height) + 0.5f);
        if (row_increase > 0 && (clipper->Flags & ImGuiListClipperFlags_NoSetTableRowCounters) == 0)
        {
            table->CurrentRow += row_increase;
            table->RowBgColorCounter += row_increase;
        }
        table->RowPosY2 = window->DC.CursorPos.y;
    }
}

ImGuiListClipper::ImGuiListClipper()
{
    memset((void*)this, 0, sizeof(*this));
}

ImGuiListClipper::~ImGuiListClipper()
{
    End();
}

void ImGuiListClipper::Begin(int items_count, float items_height)
{
    Ctx = ImGui::GetCurrentContext();

    ImGuiContext& g = *Ctx;
    ImGuiWindow* window = g.CurrentWindow;
    IMGUI_DEBUG_LOG_CLIPPER("Clipper: Begin(%d,%.2f) in '%s'\n", items_count, items_height, window->Name);

    if (ImGuiTable* table = g.CurrentTable)
        if (table->IsInsideRow)
            ImGui::TableEndRow(table);

    StartPosY = window->DC.CursorPos.y;
    ItemsHeight = items_height;
    ItemsCount = items_count;
    DisplayStart = -1;
    DisplayEnd = 0;

    if (++g.ClipperTempDataStacked > g.ClipperTempData.Size)
        g.ClipperTempData.resize(g.ClipperTempDataStacked, ImGuiListClipperData());
    ImGuiListClipperData* data = &g.ClipperTempData[g.ClipperTempDataStacked - 1];
    data->Reset(this);
    data->LossynessOffset = window->DC.CursorStartPosLossyness.y;
    TempData = data;
    StartSeekOffsetY = data->LossynessOffset;
}

void ImGuiListClipper::End()
{
    if (ImGuiListClipperData* data = (ImGuiListClipperData*)TempData)
    {

        ImGuiContext& g = *Ctx;
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: End() in '%s'\n", g.CurrentWindow->Name);
        if (ItemsCount >= 0 && ItemsCount < INT_MAX && DisplayStart >= 0)
            SeekCursorForItem(ItemsCount);

        IM_ASSERT(data->ListClipper == this);
        data->StepNo = data->Ranges.Size;
        if (--g.ClipperTempDataStacked > 0)
        {
            data = &g.ClipperTempData[g.ClipperTempDataStacked - 1];
            data->ListClipper->TempData = data;
        }
        TempData = NULL;
    }
    DisplayStart = DisplayEnd = ItemsCount;
    ItemsCount = -1;
}

void ImGuiListClipper::IncludeItemsByIndex(int item_begin, int item_end)
{
    ImGuiListClipperData* data = (ImGuiListClipperData*)TempData;
    IM_ASSERT(DisplayStart < 0);
    IM_ASSERT(item_begin <= item_end);
    if (item_begin < item_end)
        data->Ranges.push_back(ImGuiListClipperRange::FromIndices(item_begin, item_end));
}

void ImGuiListClipper::SeekCursorForItem(int item_n)
{

    float pos_y = (float)((double)StartPosY + StartSeekOffsetY + (double)item_n * ItemsHeight);
    ImGuiListClipper_SeekCursorAndSetupPrevLine(this, pos_y, ItemsHeight);
}

static bool ImGuiListClipper_StepInternal(ImGuiListClipper* clipper)
{
    ImGuiContext& g = *clipper->Ctx;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiListClipperData* data = (ImGuiListClipperData*)clipper->TempData;
    IM_ASSERT(data != NULL && "Called ImGuiListClipper::Step() too many times, or before ImGuiListClipper::Begin() ?");

    ImGuiTable* table = g.CurrentTable;
    if (table && table->IsInsideRow)
        ImGui::TableEndRow(table);

    if (clipper->ItemsCount == 0 || GetSkipItemForListClipping())
        return false;

    if (data->StepNo == 0 && table != NULL && !table->IsUnfrozenRows)
    {
        clipper->DisplayStart = data->ItemsFrozen;
        clipper->DisplayEnd = ImMin(data->ItemsFrozen + 1, clipper->ItemsCount);
        if (clipper->DisplayStart < clipper->DisplayEnd)
            data->ItemsFrozen++;
        return true;
    }

    bool calc_clipping = false;
    if (data->StepNo == 0)
    {
        clipper->StartPosY = window->DC.CursorPos.y;
        if (clipper->ItemsHeight <= 0.0f)
        {

            data->Ranges.push_front(ImGuiListClipperRange::FromIndices(data->ItemsFrozen, data->ItemsFrozen + 1));
            clipper->DisplayStart = ImMax(data->Ranges[0].Min, data->ItemsFrozen);
            clipper->DisplayEnd = ImMin(data->Ranges[0].Max, clipper->ItemsCount);
            data->StepNo = 1;
            return true;
        }
        calc_clipping = true;
    }

    if (clipper->ItemsHeight <= 0.0f)
    {
        IM_ASSERT(data->StepNo == 1);
        if (table)
            IM_ASSERT(table->RowPosY1 == clipper->StartPosY && table->RowPosY2 == window->DC.CursorPos.y);

        bool affected_by_floating_point_precision = ImIsFloatAboveGuaranteedIntegerPrecision((float)clipper->StartPosY) || ImIsFloatAboveGuaranteedIntegerPrecision(window->DC.CursorPos.y);
        if (affected_by_floating_point_precision)
        {

            clipper->ItemsHeight = window->DC.PrevLineSize.y + g.Style.ItemSpacing.y;
            window->DC.CursorPos.y = (float)(clipper->StartPosY + clipper->ItemsHeight);
        }
        else
        {
            clipper->ItemsHeight = (float)(window->DC.CursorPos.y - clipper->StartPosY) / (float)(clipper->DisplayEnd - clipper->DisplayStart);
        }
        if (clipper->ItemsHeight == 0.0f && clipper->ItemsCount == INT_MAX)
            return false;
        IM_ASSERT(clipper->ItemsHeight > 0.0f && "Unable to calculate item height! First item hasn't moved the cursor vertically!");
        calc_clipping = true;
    }

    const int already_submitted = clipper->DisplayEnd;
    if (calc_clipping)
    {

        clipper->StartSeekOffsetY = (double)data->LossynessOffset - data->ItemsFrozen * (double)clipper->ItemsHeight;

        if (g.LogEnabled)
        {

            data->Ranges.push_back(ImGuiListClipperRange::FromIndices(0, clipper->ItemsCount));
        }
        else
        {

            const bool is_nav_request = (g.NavMoveScoringItems && g.NavWindow && g.NavWindow->RootWindowForNav == window->RootWindowForNav);
            const int nav_off_min = (is_nav_request && g.NavMoveClipDir == ImGuiDir_Up) ? -1 : 0;
            const int nav_off_max = (is_nav_request && g.NavMoveClipDir == ImGuiDir_Down) ? 1 : 0;
            if (is_nav_request)
            {
                data->Ranges.push_back(ImGuiListClipperRange::FromPositions(g.NavScoringRect.Min.y, g.NavScoringRect.Max.y, nav_off_min, nav_off_max));
                if (!g.NavScoringNoClipRect.IsInverted())
                    data->Ranges.push_back(ImGuiListClipperRange::FromPositions(g.NavScoringNoClipRect.Min.y, g.NavScoringNoClipRect.Max.y, nav_off_min, nav_off_max));
            }
            if (is_nav_request && (g.NavMoveFlags & ImGuiNavMoveFlags_IsTabbing) && g.NavTabbingDir == -1)
                data->Ranges.push_back(ImGuiListClipperRange::FromIndices(clipper->ItemsCount - 1, clipper->ItemsCount));

            ImRect nav_rect_abs = ImGui::WindowRectRelToAbs(window, window->NavRectRel[0]);
            if (g.NavId != 0 && window->NavLastIds[0] == g.NavId)
                data->Ranges.push_back(ImGuiListClipperRange::FromPositions(nav_rect_abs.Min.y, nav_rect_abs.Max.y, 0, 0));

            float min_y = window->ClipRect.Min.y;
            float max_y = window->ClipRect.Max.y;

            ImGuiBoxSelectState* bs = &g.BoxSelectState;
            if (bs->IsActive && bs->Window == window)
            {

                min_y -= g.Style.ItemSpacing.y;
                max_y += g.Style.ItemSpacing.y;

                if (bs->UnclipMode)
                    data->Ranges.push_back(ImGuiListClipperRange::FromPositions(bs->UnclipRect.Min.y, bs->UnclipRect.Max.y, 0, 0));
            }

            data->Ranges.push_back(ImGuiListClipperRange::FromPositions(min_y, max_y, nav_off_min, nav_off_max));
        }

        for (ImGuiListClipperRange& range : data->Ranges)
            if (range.PosToIndexConvert)
            {
                int m1 = (int)(((double)range.Min - window->DC.CursorPos.y - data->LossynessOffset) / clipper->ItemsHeight);
                int m2 = (int)((((double)range.Max - window->DC.CursorPos.y - data->LossynessOffset) / clipper->ItemsHeight) + 0.999999f);
                range.Min = ImClamp(already_submitted + m1 + range.PosToIndexOffsetMin, already_submitted, clipper->ItemsCount - 1);
                range.Max = ImClamp(already_submitted + m2 + range.PosToIndexOffsetMax, range.Min + 1, clipper->ItemsCount);
                range.PosToIndexConvert = false;
            }
        ImGuiListClipper_SortAndFuseRanges(data->Ranges, data->StepNo);
    }

    while (data->StepNo < data->Ranges.Size)
    {
        clipper->DisplayStart = ImMax(data->Ranges[data->StepNo].Min, already_submitted);
        clipper->DisplayEnd = ImMin(data->Ranges[data->StepNo].Max, clipper->ItemsCount);
        data->StepNo++;
        if (clipper->DisplayStart >= clipper->DisplayEnd)
            continue;
        if (clipper->DisplayStart > already_submitted)
            clipper->SeekCursorForItem(clipper->DisplayStart);
        return true;
    }

    if (clipper->ItemsCount < INT_MAX)
        clipper->SeekCursorForItem(clipper->ItemsCount);

    return false;
}

bool ImGuiListClipper::Step()
{
    ImGuiContext& g = *Ctx;
    bool need_items_height = (ItemsHeight <= 0.0f);
    bool ret = ImGuiListClipper_StepInternal(this);
    if (ret && (DisplayStart >= DisplayEnd))
        ret = false;
    if (g.CurrentTable && g.CurrentTable->IsUnfrozenRows == false)
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: Step(): inside frozen table row.\n");
    if (need_items_height && ItemsHeight > 0.0f)
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: Step(): computed ItemsHeight: %.2f.\n", ItemsHeight);
    if (ret)
    {
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: Step(): display %d to %d.\n", DisplayStart, DisplayEnd);
    }
    else
    {
        IMGUI_DEBUG_LOG_CLIPPER("Clipper: Step(): End.\n");
        End();
    }
    return ret;
}

void ImGui::CalcClipRectVisibleItemsY(const ImRect& clip_rect, const ImVec2& pos, float items_height, int* out_visible_start, int* out_visible_end)
{
    *out_visible_start = ImMax((int)((clip_rect.Min.y - pos.y) / items_height), 0);
    *out_visible_end = ImMax((int)ImCeil((clip_rect.Max.y - pos.y) / items_height), *out_visible_start);
}

ImGuiStyle& ImGui::GetStyle()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->Style;
}

ImU32 ImGui::GetColorU32(ImGuiCol idx, float alpha_mul)
{
    ImGuiStyle& style = GImGui->Style;
    ImVec4 c = style.Colors[idx];
    c.w *= style.Alpha * alpha_mul;
    return ColorConvertFloat4ToU32(c);
}

ImU32 ImGui::GetColorU32(const ImVec4& col)
{
    ImGuiStyle& style = GImGui->Style;
    ImVec4 c = col;
    c.w *= style.Alpha;
    return ColorConvertFloat4ToU32(c);
}

const ImVec4& ImGui::GetStyleColorVec4(ImGuiCol idx)
{
    ImGuiStyle& style = GImGui->Style;
    return style.Colors[idx];
}

ImU32 ImGui::GetColorU32(ImU32 col, float alpha_mul)
{
    ImGuiStyle& style = GImGui->Style;
    alpha_mul *= style.Alpha;
    if (alpha_mul >= 1.0f)
        return col;
    ImU32 a = (col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT;
    a = (ImU32)(a * alpha_mul);
    return (col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
}

void ImGui::PushStyleColor(ImGuiCol idx, ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = g.Style.Colors[idx];
    g.ColorStack.push_back(backup);
    if (g.DebugFlashStyleColorIdx != idx)
        g.Style.Colors[idx] = ColorConvertU32ToFloat4(col);
}

void ImGui::PushStyleColor(ImGuiCol idx, const ImVec4& col)
{
    ImGuiContext& g = *GImGui;
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = g.Style.Colors[idx];
    g.ColorStack.push_back(backup);
    if (g.DebugFlashStyleColorIdx != idx)
        g.Style.Colors[idx] = col;
}

void ImGui::PopStyleColor(int count)
{
    ImGuiContext& g = *GImGui;
    if (g.ColorStack.Size < count)
    {
        IM_ASSERT_USER_ERROR(0, "Calling PopStyleColor() too many times!");
        count = g.ColorStack.Size;
    }
    while (count > 0)
    {
        ImGuiColorMod& backup = g.ColorStack.back();
        g.Style.Colors[backup.Col] = backup.BackupValue;
        g.ColorStack.pop_back();
        count--;
    }
}

static const ImGuiStyleVarInfo GStyleVarsInfo[] =
{
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, Alpha) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, DisabledAlpha) },
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowPadding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowRounding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowBorderSize) },
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowMinSize) },
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, WindowTitleAlign) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ChildRounding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ChildBorderSize) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, PopupRounding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, PopupBorderSize) },
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, FramePadding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, FrameRounding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, FrameBorderSize) },
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ItemSpacing) },
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ItemInnerSpacing) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, IndentSpacing) },
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, CellPadding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ScrollbarSize) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ScrollbarRounding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ScrollbarPadding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, GrabMinSize) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, GrabRounding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ImageRounding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ImageBorderSize) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabRounding) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabBorderSize) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabMinWidthBase) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabMinWidthShrink) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabBarBorderSize) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TabBarOverlineSize) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TableAngledHeadersAngle)},
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TableAngledHeadersTextAlign)},
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TreeLinesSize)},
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, TreeLinesRounding)},
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, ButtonTextAlign) },
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SelectableTextAlign) },
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorSize)},
    { 1, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorTextBorderSize)},
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorTextAlign) },
    { 2, ImGuiDataType_Float, (ImU32)offsetof(ImGuiStyle, SeparatorTextPadding) },
};

const ImGuiStyleVarInfo* ImGui::GetStyleVarInfo(ImGuiStyleVar idx)
{
    IM_ASSERT(idx >= 0 && idx < ImGuiStyleVar_COUNT);
    IM_STATIC_ASSERT(IM_COUNTOF(GStyleVarsInfo) == ImGuiStyleVar_COUNT);
    return &GStyleVarsInfo[idx];
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, float val)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 1, "Calling PushStyleVar() variant with wrong type!");
    float* pvar = (float*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    *pvar = val;
}

void ImGui::PushStyleVarX(ImGuiStyleVar idx, float val_x)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 2, "Calling PushStyleVar() variant with wrong type!");
    ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    pvar->x = val_x;
}

void ImGui::PushStyleVarY(ImGuiStyleVar idx, float val_y)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 2, "Calling PushStyleVar() variant with wrong type!");
    ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    pvar->y = val_y;
}

void ImGui::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(idx);
    IM_ASSERT_USER_ERROR_RET(var_info->DataType == ImGuiDataType_Float && var_info->Count == 2, "Calling PushStyleVar() variant with wrong type!");
    ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&g.Style);
    g.StyleVarStack.push_back(ImGuiStyleMod(idx, *pvar));
    *pvar = val;
}

void ImGui::PopStyleVar(int count)
{
    ImGuiContext& g = *GImGui;
    if (g.StyleVarStack.Size < count)
    {
        IM_ASSERT_USER_ERROR(0, "Calling PopStyleVar() too many times!");
        count = g.StyleVarStack.Size;
    }
    while (count > 0)
    {

        ImGuiStyleMod& backup = g.StyleVarStack.back();
        const ImGuiStyleVarInfo* var_info = GetStyleVarInfo(backup.VarIdx);
        void* data = var_info->GetVarPtr(&g.Style);
        if (var_info->DataType == ImGuiDataType_Float && var_info->Count == 1)      { ((float*)data)[0] = backup.BackupFloat[0]; }
        else if (var_info->DataType == ImGuiDataType_Float && var_info->Count == 2) { ((float*)data)[0] = backup.BackupFloat[0]; ((float*)data)[1] = backup.BackupFloat[1]; }
        g.StyleVarStack.pop_back();
        count--;
    }
}

const char* ImGui::GetStyleColorName(ImGuiCol idx)
{

    switch (idx)
    {
    case ImGuiCol_Text: return "Text";
    case ImGuiCol_TextDisabled: return "TextDisabled";
    case ImGuiCol_WindowBg: return "WindowBg";
    case ImGuiCol_ChildBg: return "ChildBg";
    case ImGuiCol_PopupBg: return "PopupBg";
    case ImGuiCol_Border: return "Border";
    case ImGuiCol_BorderShadow: return "BorderShadow";
    case ImGuiCol_FrameBg: return "FrameBg";
    case ImGuiCol_FrameBgHovered: return "FrameBgHovered";
    case ImGuiCol_FrameBgActive: return "FrameBgActive";
    case ImGuiCol_TitleBg: return "TitleBg";
    case ImGuiCol_TitleBgActive: return "TitleBgActive";
    case ImGuiCol_TitleBgCollapsed: return "TitleBgCollapsed";
    case ImGuiCol_MenuBarBg: return "MenuBarBg";
    case ImGuiCol_ScrollbarBg: return "ScrollbarBg";
    case ImGuiCol_ScrollbarGrab: return "ScrollbarGrab";
    case ImGuiCol_ScrollbarGrabHovered: return "ScrollbarGrabHovered";
    case ImGuiCol_ScrollbarGrabActive: return "ScrollbarGrabActive";
    case ImGuiCol_CheckMark: return "CheckMark";
    case ImGuiCol_SliderGrab: return "SliderGrab";
    case ImGuiCol_SliderGrabActive: return "SliderGrabActive";
    case ImGuiCol_Button: return "Button";
    case ImGuiCol_ButtonHovered: return "ButtonHovered";
    case ImGuiCol_ButtonActive: return "ButtonActive";
    case ImGuiCol_Header: return "Header";
    case ImGuiCol_HeaderHovered: return "HeaderHovered";
    case ImGuiCol_HeaderActive: return "HeaderActive";
    case ImGuiCol_Separator: return "Separator";
    case ImGuiCol_SeparatorHovered: return "SeparatorHovered";
    case ImGuiCol_SeparatorActive: return "SeparatorActive";
    case ImGuiCol_ResizeGrip: return "ResizeGrip";
    case ImGuiCol_ResizeGripHovered: return "ResizeGripHovered";
    case ImGuiCol_ResizeGripActive: return "ResizeGripActive";
    case ImGuiCol_InputTextCursor: return "InputTextCursor";
    case ImGuiCol_TabHovered: return "TabHovered";
    case ImGuiCol_Tab: return "Tab";
    case ImGuiCol_TabSelected: return "TabSelected";
    case ImGuiCol_TabSelectedOverline: return "TabSelectedOverline";
    case ImGuiCol_TabDimmed: return "TabDimmed";
    case ImGuiCol_TabDimmedSelected: return "TabDimmedSelected";
    case ImGuiCol_TabDimmedSelectedOverline: return "TabDimmedSelectedOverline";
    case ImGuiCol_PlotLines: return "PlotLines";
    case ImGuiCol_PlotLinesHovered: return "PlotLinesHovered";
    case ImGuiCol_PlotHistogram: return "PlotHistogram";
    case ImGuiCol_PlotHistogramHovered: return "PlotHistogramHovered";
    case ImGuiCol_TableHeaderBg: return "TableHeaderBg";
    case ImGuiCol_TableBorderStrong: return "TableBorderStrong";
    case ImGuiCol_TableBorderLight: return "TableBorderLight";
    case ImGuiCol_TableRowBg: return "TableRowBg";
    case ImGuiCol_TableRowBgAlt: return "TableRowBgAlt";
    case ImGuiCol_TextLink: return "TextLink";
    case ImGuiCol_TextSelectedBg: return "TextSelectedBg";
    case ImGuiCol_TreeLines: return "TreeLines";
    case ImGuiCol_DragDropTarget: return "DragDropTarget";
    case ImGuiCol_DragDropTargetBg: return "DragDropTargetBg";
    case ImGuiCol_UnsavedMarker: return "UnsavedMarker";
    case ImGuiCol_NavCursor: return "NavCursor";
    case ImGuiCol_NavWindowingHighlight: return "NavWindowingHighlight";
    case ImGuiCol_NavWindowingDimBg: return "NavWindowingDimBg";
    case ImGuiCol_ModalWindowDimBg: return "ModalWindowDimBg";
    }
    IM_ASSERT(0);
    return "Unknown";
}

const char* ImGui::FindRenderedTextEnd(const char* text, const char* text_end)
{
    const char* text_display_end = text;
    if (!text_end)
        text_end = (const char*)-1;

    while (text_display_end < text_end && *text_display_end != '\0' && (text_display_end[0] != '#' || text_display_end[1] != '#'))
        text_display_end++;
    return text_display_end;
}

void ImGui::RenderText(ImVec2 pos, const char* text, const char* text_end, bool hide_text_after_hash)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const char* text_display_end;
    if (hide_text_after_hash)
    {
        text_display_end = FindRenderedTextEnd(text, text_end);
    }
    else
    {
        if (!text_end)
            text_end = text + ImStrlen(text);
        text_display_end = text_end;
    }

    if (text != text_display_end)
    {
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_display_end);
        if (g.LogEnabled)
            LogRenderedText(&pos, text, text_display_end);
    }
}

void ImGui::RenderTextWrapped(ImVec2 pos, const char* text, const char* text_end, float wrap_width)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    if (!text_end)
        text_end = text + ImStrlen(text);

    if (text != text_end)
    {
        window->DrawList->AddText(g.Font, g.FontSize, pos, GetColorU32(ImGuiCol_Text), text, text_end, wrap_width);
        if (g.LogEnabled)
            LogRenderedText(&pos, text, text_end);
    }
}

void ImGui::RenderTextClippedEx(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_display_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{

    ImVec2 pos = pos_min;
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : CalcTextSize(text, text_display_end, false, 0.0f);

    const ImVec2* clip_min = clip_rect ? &clip_rect->Min : &pos_min;
    const ImVec2* clip_max = clip_rect ? &clip_rect->Max : &pos_max;
    bool need_clipping = (pos.x + text_size.x >= clip_max->x) || (pos.y + text_size.y >= clip_max->y);
    if (clip_rect)
        need_clipping |= (pos.x < clip_min->x) || (pos.y < clip_min->y);

    if (align.x > 0.0f) pos.x = ImMax(pos.x, pos.x + (pos_max.x - pos.x - text_size.x) * align.x);
    if (align.y > 0.0f) pos.y = ImMax(pos.y, pos.y + (pos_max.y - pos.y - text_size.y) * align.y);

    if (need_clipping)
    {
        ImVec4 fine_clip_rect(clip_min->x, clip_min->y, clip_max->x, clip_max->y);
        draw_list->AddText(NULL, 0.0f, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, &fine_clip_rect);
    }
    else
    {
        draw_list->AddText(NULL, 0.0f, pos, GetColorU32(ImGuiCol_Text), text, text_display_end, 0.0f, NULL);
    }
}

void ImGui::RenderTextClipped(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align, const ImRect* clip_rect)
{

    const char* text_display_end = FindRenderedTextEnd(text, text_end);
    const int text_len = (int)(text_display_end - text);
    if (text_len == 0)
        return;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    RenderTextClippedEx(window->DrawList, pos_min, pos_max, text, text_display_end, text_size_if_known, align, clip_rect);
    if (g.LogEnabled)
        LogRenderedText(&pos_min, text, text_display_end);
}

void ImGui::RenderTextEllipsis(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, float ellipsis_max_x, const char* text, const char* text_end_full, const ImVec2* text_size_if_known)
{
    ImGuiContext& g = *GImGui;
    if (text_end_full == NULL)
        text_end_full = FindRenderedTextEnd(text);
    const ImVec2 text_size = text_size_if_known ? *text_size_if_known : CalcTextSize(text, text_end_full, false, 0.0f);

    if (text_size.x > pos_max.x - pos_min.x)
    {

        ImFont* font = draw_list->_Data->Font;
        const float font_size = draw_list->_Data->FontSize;
        const float font_scale = draw_list->_Data->FontScale;
        const char* text_end_ellipsis = NULL;
        ImFontBaked* baked = font->GetFontBaked(font_size);
        const float ellipsis_width = baked->GetCharAdvance(font->EllipsisChar) * font_scale;

        const float text_avail_width = ImMax((ImMax(pos_max.x, ellipsis_max_x) - ellipsis_width) - pos_min.x, 1.0f);
        const float text_size_clipped_x = font->CalcTextSizeA(font_size, text_avail_width, 0.0f, text, text_end_full, &text_end_ellipsis).x;

        RenderTextClippedEx(draw_list, pos_min, pos_max, text, text_end_ellipsis, &text_size, ImVec2(0.0f, 0.0f));
        ImVec4 cpu_fine_clip_rect(pos_min.x, pos_min.y, pos_max.x, pos_max.y);
        ImVec2 ellipsis_pos = ImTrunc(ImVec2(pos_min.x + text_size_clipped_x, pos_min.y));
        font->RenderChar(draw_list, font_size, ellipsis_pos, GetColorU32(ImGuiCol_Text), font->EllipsisChar, &cpu_fine_clip_rect);
    }
    else
    {
        RenderTextClippedEx(draw_list, pos_min, pos_max, text, text_end_full, &text_size, ImVec2(0.0f, 0.0f));
    }

    if (g.LogEnabled)
        LogRenderedText(&pos_min, text, text_end_full);
}

void ImGui::RenderFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool borders, float rounding)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding);
    const float border_size = g.Style.FrameBorderSize;
    if (borders && border_size > 0.0f)
    {
        window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, 0, border_size);
        window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, 0, border_size);
    }
}

void ImGui::RenderFrameBorder(ImVec2 p_min, ImVec2 p_max, float rounding)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const float border_size = g.Style.FrameBorderSize;
    if (border_size > 0.0f)
    {
        window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, 0, border_size);
        window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, 0, border_size);
    }
}

void ImGui::RenderColorComponentMarker(const ImRect& bb, ImU32 col, float rounding)
{
    if (bb.Min.x + 1 >= bb.Max.x)
        return;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    RenderRectFilledInRangeH(window->DrawList, bb, col, bb.Min.x, ImMin(bb.Min.x + g.Style.ColorMarkerSize, bb.Max.x), rounding);
}

void ImGui::RenderNavCursor(const ImRect& bb, ImGuiID id, ImGuiNavRenderCursorFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (id != g.NavId)
        return;
    if (!g.NavCursorVisible && !(flags & ImGuiNavRenderCursorFlags_AlwaysDraw))
        return;
    if (id == g.LastItemData.ID && (g.LastItemData.ItemFlags & ImGuiItemFlags_NoNav))
        return;

    ImGuiWindow* window = g.CurrentWindow;
    if (window->DC.NavHideHighlightOneFrame)
        return;

    float rounding = (flags & ImGuiNavRenderCursorFlags_NoRounding) ? 0.0f : g.Style.FrameRounding;
    ImRect display_rect = bb;
    display_rect.ClipWith(window->ClipRect);
    const float thickness = 2.0f;
    if (flags & ImGuiNavRenderCursorFlags_Compact)
    {
        window->DrawList->AddRect(display_rect.Min, display_rect.Max, GetColorU32(ImGuiCol_NavCursor), rounding, 0, thickness);
    }
    else
    {
        const float distance = 3.0f + thickness * 0.5f;
        display_rect.Expand(ImVec2(distance, distance));
        bool fully_visible = window->ClipRect.Contains(display_rect);
        if (!fully_visible)
            window->DrawList->PushClipRect(display_rect.Min, display_rect.Max);
        window->DrawList->AddRect(display_rect.Min, display_rect.Max, GetColorU32(ImGuiCol_NavCursor), rounding, 0, thickness);
        if (!fully_visible)
            window->DrawList->PopClipRect();
    }
}

void ImGui::RenderMouseCursor(ImVec2 base_pos, float base_scale, ImGuiMouseCursor mouse_cursor, ImU32 col_fill, ImU32 col_border, ImU32 col_shadow)
{
    ImGuiContext& g = *GImGui;
    if (mouse_cursor <= ImGuiMouseCursor_None || mouse_cursor >= ImGuiMouseCursor_COUNT)
        mouse_cursor = ImGuiMouseCursor_Arrow;
    ImFontAtlas* font_atlas = g.DrawListSharedData.FontAtlas;
    for (ImGuiViewportP* viewport : g.Viewports)
    {

        ImVec2 offset, size, uv[4];
        if (!ImFontAtlasGetMouseCursorTexData(font_atlas, mouse_cursor, &offset, &size, &uv[0], &uv[2]))
            continue;
        const ImVec2 pos = base_pos - offset;
        const float scale = base_scale;
        if (!viewport->GetMainRect().Overlaps(ImRect(pos, pos + ImVec2(size.x + 2, size.y + 2) * scale)))
            continue;
        ImDrawList* draw_list = GetForegroundDrawList(viewport);
        ImTextureRef tex_ref = font_atlas->TexRef;
        draw_list->PushTexture(tex_ref);
        draw_list->AddImage(tex_ref, pos + ImVec2(1, 0) * scale, pos + (ImVec2(1, 0) + size) * scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_ref, pos + ImVec2(2, 0) * scale, pos + (ImVec2(2, 0) + size) * scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_ref, pos,                        pos + size * scale,                  uv[2], uv[3], col_border);
        draw_list->AddImage(tex_ref, pos,                        pos + size * scale,                  uv[0], uv[1], col_fill);
        if (mouse_cursor == ImGuiMouseCursor_Wait || mouse_cursor == ImGuiMouseCursor_Progress)
        {
            float a_min = ImFmod((float)g.Time * 5.0f, 2.0f * IM_PI);
            float a_max = a_min + IM_PI * 1.65f;
            draw_list->PathArcTo(pos + ImVec2(14, -1) * scale, 6.0f * scale, a_min, a_max);
            draw_list->PathStroke(col_fill, ImDrawFlags_None, 3.0f * scale);
        }
        draw_list->PopTexture();
    }
}

ImGuiContext* ImGui::GetCurrentContext()
{
    return GImGui;
}

void ImGui::SetCurrentContext(ImGuiContext* ctx)
{
#ifdef IMGUI_SET_CURRENT_CONTEXT_FUNC
    IMGUI_SET_CURRENT_CONTEXT_FUNC(ctx);
#else
    GImGui = ctx;
#endif
}

void ImGui::SetAllocatorFunctions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void* user_data)
{
    GImAllocatorAllocFunc = alloc_func;
    GImAllocatorFreeFunc = free_func;
    GImAllocatorUserData = user_data;
}

void ImGui::GetAllocatorFunctions(ImGuiMemAllocFunc* p_alloc_func, ImGuiMemFreeFunc* p_free_func, void** p_user_data)
{
    *p_alloc_func = GImAllocatorAllocFunc;
    *p_free_func = GImAllocatorFreeFunc;
    *p_user_data = GImAllocatorUserData;
}

ImGuiContext* ImGui::CreateContext(ImFontAtlas* shared_font_atlas)
{
    ImGuiContext* prev_ctx = GetCurrentContext();
    ImGuiContext* ctx = IM_NEW(ImGuiContext)(shared_font_atlas);
    SetCurrentContext(ctx);
    Initialize();
    if (prev_ctx != NULL)
        SetCurrentContext(prev_ctx);
    return ctx;
}

void ImGui::DestroyContext(ImGuiContext* ctx)
{
    ImGuiContext* prev_ctx = GetCurrentContext();
    if (ctx == NULL)
        ctx = prev_ctx;
    SetCurrentContext(ctx);
    Shutdown();
    SetCurrentContext((prev_ctx != ctx) ? prev_ctx : NULL);
    IM_DELETE(ctx);
}

static const ImGuiLocEntry GLocalizationEntriesEnUS[] =
{
    { ImGuiLocKey_VersionStr,           "Dear ImGui " IMGUI_VERSION " (" IM_STRINGIFY(IMGUI_VERSION_NUM) ")" },
    { ImGuiLocKey_TableSizeOne,         "Size column to fit###SizeOne"          },
    { ImGuiLocKey_TableSizeAllFit,      "Size all columns to fit###SizeAll"     },
    { ImGuiLocKey_TableSizeAllDefault,  "Size all columns to default###SizeAll" },
    { ImGuiLocKey_TableResetOrder,      "Reset order###ResetOrder"              },
    { ImGuiLocKey_WindowingMainMenuBar, "(Main menu bar)"                       },
    { ImGuiLocKey_WindowingPopup,       "(Popup)"                               },
    { ImGuiLocKey_WindowingUntitled,    "(Untitled)"                            },
    { ImGuiLocKey_OpenLink_s,           "Open '%s'"                             },
    { ImGuiLocKey_CopyLink,             "Copy Link###CopyLink"                  },
};

ImGuiContext::ImGuiContext(ImFontAtlas* shared_font_atlas)
{
    IO.Ctx = this;
    InputTextState.Ctx = this;

    Initialized = false;
    WithinFrameScope = WithinFrameScopeWithImplicitWindow = false;
    TestEngineHookItems = false;
    FrameCount = 0;
    FrameCountEnded = FrameCountRendered = -1;
    Time = 0.0f;
    memset(ContextName, 0, sizeof(ContextName));

    Font = NULL;
    FontBaked = NULL;
    FontSize = FontSizeBase = FontBakedScale = CurrentDpiScale = 0.0f;
    FontRasterizerDensity = 1.0f;
    IO.Fonts = shared_font_atlas ? shared_font_atlas : IM_NEW(ImFontAtlas)();
    if (shared_font_atlas == NULL)
        IO.Fonts->OwnerContext = this;
    WithinEndChildID = 0;
    TestEngine = NULL;

    InputEventsNextMouseSource = ImGuiMouseSource_Mouse;
    InputEventsNextEventId = 1;

    WindowsActiveCount = 0;
    WindowsBorderHoverPadding = 0.0f;
    CurrentWindow = NULL;
    HoveredWindow = NULL;
    HoveredWindowUnderMovingWindow = NULL;
    HoveredWindowBeforeClear = NULL;
    MovingWindow = NULL;
    WheelingWindow = NULL;
    WheelingWindowStartFrame = WheelingWindowScrolledFrame = -1;
    WheelingWindowReleaseTimer = 0.0f;

    DebugDrawIdConflictsId = 0;
    DebugHookIdInfoId = 0;
    HoveredId = HoveredIdPreviousFrame = 0;
    HoveredIdPreviousFrameItemCount = 0;
    HoveredIdAllowOverlap = false;
    HoveredIdIsDisabled = false;
    HoveredIdTimer = HoveredIdNotActiveTimer = 0.0f;
    ItemUnclipByLog = false;
    ActiveId = 0;
    ActiveIdIsAlive = 0;
    ActiveIdTimer = 0.0f;
    ActiveIdIsJustActivated = false;
    ActiveIdAllowOverlap = false;
    ActiveIdNoClearOnFocusLoss = false;
    ActiveIdHasBeenPressedBefore = false;
    ActiveIdHasBeenEditedBefore = false;
    ActiveIdHasBeenEditedThisFrame = false;
    ActiveIdFromShortcut = false;
    ActiveIdClickOffset = ImVec2(-1, -1);
    ActiveIdSource = ImGuiInputSource_None;
    ActiveIdWindow = NULL;
    ActiveIdMouseButton = -1;
    ActiveIdDisabledId = 0;
    ActiveIdPreviousFrame = 0;
    memset(&DeactivatedItemData, 0, sizeof(DeactivatedItemData));
    memset(&ActiveIdValueOnActivation, 0, sizeof(ActiveIdValueOnActivation));
    LastActiveId = 0;
    LastActiveIdTimer = 0.0f;

    LastKeyboardKeyPressTime = LastKeyModsChangeTime = LastKeyModsChangeFromNoneTime = -1.0;

    ActiveIdUsingNavDirMask = 0x00;
    ActiveIdUsingAllKeyboardKeys = false;

    CurrentFocusScopeId = 0;
    CurrentItemFlags = ImGuiItemFlags_None;
    DebugShowGroupRects = false;
    GcCompactAll = false;

    NavCursorVisible = false;
    NavHighlightItemUnderNav = false;
    NavMousePosDirty = false;
    NavIdIsAlive = false;
    NavId = 0;
    NavWindow = NULL;
    NavFocusScopeId = NavActivateId = NavActivateDownId = NavActivatePressedId = 0;
    NavLayer = ImGuiNavLayer_Main;
    NavIdItemFlags = ImGuiItemFlags_None;
    NavOpenContextMenuItemId = NavOpenContextMenuWindowId = 0;
    NavNextActivateId = 0;
    NavActivateFlags = NavNextActivateFlags = ImGuiActivateFlags_None;
    NavHighlightActivatedId = 0;
    NavHighlightActivatedTimer = 0.0f;
    NavInputSource = ImGuiInputSource_Keyboard;
    NavLastValidSelectionUserData = ImGuiSelectionUserData_Invalid;
    NavCursorHideFrames = 0;

    NavAnyRequest = false;
    NavInitRequest = false;
    NavInitRequestFromMove = false;
    NavMoveSubmitted = false;
    NavMoveScoringItems = false;
    NavMoveForwardToNextFrame = false;
    NavMoveFlags = ImGuiNavMoveFlags_None;
    NavMoveScrollFlags = ImGuiScrollFlags_None;
    NavMoveKeyMods = ImGuiMod_None;
    NavMoveDir = NavMoveDirForDebug = NavMoveClipDir = ImGuiDir_None;
    NavScoringDebugCount = 0;
    NavTabbingDir = 0;
    NavTabbingCounter = 0;

    NavJustMovedFromFocusScopeId = NavJustMovedToId = NavJustMovedToFocusScopeId = 0;
    NavJustMovedToKeyMods = ImGuiMod_None;
    NavJustMovedToIsTabbing = false;
    NavJustMovedToHasSelectionData = false;

    ConfigNavEnableTabbing = true;
    ConfigNavWindowingWithGamepad = true;
    ConfigNavWindowingKeyNext = IO.ConfigMacOSXBehaviors ? (ImGuiMod_Super | ImGuiKey_Tab) : (ImGuiMod_Ctrl | ImGuiKey_Tab);
    ConfigNavWindowingKeyPrev = IO.ConfigMacOSXBehaviors ? (ImGuiMod_Super | ImGuiMod_Shift | ImGuiKey_Tab) : (ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Tab);
    NavWindowingTarget = NavWindowingTargetAnim = NavWindowingListWindow = NULL;
    NavWindowingInputSource = ImGuiInputSource_None;
    NavWindowingTimer = NavWindowingHighlightAlpha = 0.0f;
    NavWindowingToggleLayer = false;
    NavWindowingToggleKey = ImGuiKey_None;

    DimBgRatio = 0.0f;

    DragDropActive = DragDropWithinSource = DragDropWithinTarget = false;
    DragDropSourceFlags = ImGuiDragDropFlags_None;
    DragDropSourceFrameCount = -1;
    DragDropMouseButton = -1;
    DragDropTargetId = 0;
    DragDropTargetFullViewport = 0;
    DragDropAcceptFlagsCurr = DragDropAcceptFlagsPrev = ImGuiDragDropFlags_None;
    DragDropAcceptIdCurrRectSurface = 0.0f;
    DragDropAcceptIdPrev = DragDropAcceptIdCurr = 0;
    DragDropAcceptFrameCount = -1;
    DragDropHoldJustPressedId = 0;
    memset(DragDropPayloadBufLocal, 0, sizeof(DragDropPayloadBufLocal));

    ClipperTempDataStacked = 0;

    CurrentTable = NULL;
    TablesTempDataStacked = 0;
    CurrentTabBar = NULL;
    CurrentMultiSelect = NULL;
    MultiSelectTempDataStacked = 0;

    HoverItemDelayId = HoverItemDelayIdPreviousFrame = HoverItemUnlockedStationaryId = HoverWindowUnlockedStationaryId = 0;
    HoverItemDelayTimer = HoverItemDelayClearTimer = 0.0f;

    MouseCursor = ImGuiMouseCursor_Arrow;
    MouseStationaryTimer = 0.0f;

    InputTextPasswordFontBackupFlags = ImFontFlags_None;
    InputTextReactivateId = 0;
    TempInputId = 0;
    memset(&DataTypeZeroValue, 0, sizeof(DataTypeZeroValue));
    BeginMenuDepth = BeginComboDepth = 0;
    ColorEditOptions = ImGuiColorEditFlags_DefaultOptions_;
    ColorEditCurrentID = ColorEditSavedID = 0;
    ColorEditSavedHue = ColorEditSavedSat = 0.0f;
    ColorEditSavedColor = 0;
    WindowResizeRelativeMode = false;
    ScrollbarSeekMode = 0;
    ScrollbarClickDeltaToGrabCenter = 0.0f;
    SliderGrabClickOffset = 0.0f;
    SliderCurrentAccum = 0.0f;
    SliderCurrentAccumDirty = false;
    DragCurrentAccumDirty = false;
    DragCurrentAccum = 0.0f;
    DragSpeedDefaultRatio = 1.0f / 100.0f;
    DisabledAlphaBackup = 0.0f;
    DisabledStackSize = 0;
    TooltipOverrideCount = 0;
    TooltipPreviousWindow = NULL;

    PlatformImeData.InputPos = ImVec2(0.0f, 0.0f);
    PlatformImeDataPrev.InputPos = ImVec2(-1.0f, -1.0f);

    SettingsLoaded = false;
    SettingsDirtyTimer = 0.0f;
    HookIdNext = 0;
    DemoMarkerCallback = NULL;

    memset(LocalizationTable, 0, sizeof(LocalizationTable));

    LogEnabled = false;
    LogLineFirstItem = false;
    LogFlags = ImGuiLogFlags_None;
    LogWindow = NULL;
    LogNextPrefix = LogNextSuffix = NULL;
    LogFile = NULL;
    LogLinePosY = FLT_MAX;
    LogDepthRef = 0;
    LogDepthToExpand = LogDepthToExpandDefault = 2;

    ErrorCallback = NULL;
    ErrorCallbackUserData = NULL;
    ErrorFirst = true;
    ErrorCountCurrentFrame = 0;
    StackSizesInBeginForCurrentWindow = NULL;

    DebugDrawIdConflictsCount = 0;
    DebugLogFlags = ImGuiDebugLogFlags_EventError | ImGuiDebugLogFlags_OutputToTTY;
    DebugLocateId = 0;
    DebugLogSkippedErrors = 0;
    DebugLogAutoDisableFlags = ImGuiDebugLogFlags_None;
    DebugLogAutoDisableFrames = 0;
    DebugLocateFrames = 0;
    DebugBeginReturnValueCullDepth = -1;
    DebugItemPickerActive = false;
    DebugItemPickerMouseButton = ImGuiMouseButton_Left;
    DebugItemPickerBreakId = 0;
    DebugFlashStyleColorTime = 0.0f;
    DebugFlashStyleColorIdx = ImGuiCol_COUNT;

    DebugBreakInWindow = 0;
    DebugBreakInTable = 0;
    DebugBreakInLocateId = false;
    DebugBreakKeyChord = ImGuiKey_Pause;
    DebugBreakInShortcutRouting = ImGuiKey_None;

    memset(FramerateSecPerFrame, 0, sizeof(FramerateSecPerFrame));
    FramerateSecPerFrameIdx = FramerateSecPerFrameCount = 0;
    FramerateSecPerFrameAccum = 0.0f;
    WantCaptureMouseNextFrame = WantCaptureKeyboardNextFrame = WantTextInputNextFrame = -1;
    memset(TempKeychordName, 0, sizeof(TempKeychordName));
}

ImGuiContext::~ImGuiContext()
{
    IM_ASSERT(Initialized == false && "Forgot to call DestroyContext()?");
}

void ImGui::Initialize()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(!g.Initialized && !g.SettingsLoaded);

    {
        ImGuiSettingsHandler ini_handler;
        ini_handler.TypeName = "Window";
        ini_handler.TypeHash = ImHashStr("Window");
        ini_handler.ClearAllFn = WindowSettingsHandler_ClearAll;
        ini_handler.ReadOpenFn = WindowSettingsHandler_ReadOpen;
        ini_handler.ReadLineFn = WindowSettingsHandler_ReadLine;
        ini_handler.ApplyAllFn = WindowSettingsHandler_ApplyAll;
        ini_handler.WriteAllFn = WindowSettingsHandler_WriteAll;
        AddSettingsHandler(&ini_handler);
    }
    TableSettingsAddSettingsHandler();

    LocalizeRegisterEntries(GLocalizationEntriesEnUS, IM_COUNTOF(GLocalizationEntriesEnUS));

    g.PlatformIO.Platform_GetClipboardTextFn = Platform_GetClipboardTextFn_DefaultImpl;
    g.PlatformIO.Platform_SetClipboardTextFn = Platform_SetClipboardTextFn_DefaultImpl;
    g.PlatformIO.Platform_OpenInShellFn = Platform_OpenInShellFn_DefaultImpl;
    g.PlatformIO.Platform_SetImeDataFn = Platform_SetImeDataFn_DefaultImpl;

    ImGuiViewportP* viewport = IM_NEW(ImGuiViewportP)();
    viewport->ID = IMGUI_VIEWPORT_DEFAULT_ID;
    g.Viewports.push_back(viewport);
    g.TempBuffer.resize(1024 * 3 + 1, 0);

    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
        if ((key >= ImGuiKey_0 && key <= ImGuiKey_9) || (key >= ImGuiKey_A && key <= ImGuiKey_Z) || (key >= ImGuiKey_Keypad0 && key <= ImGuiKey_Keypad9)
            || key == ImGuiKey_Tab || key == ImGuiKey_Space || key == ImGuiKey_Apostrophe || key == ImGuiKey_Comma || key == ImGuiKey_Minus || key == ImGuiKey_Period
            || key == ImGuiKey_Slash || key == ImGuiKey_Semicolon || key == ImGuiKey_Equal || key == ImGuiKey_LeftBracket || key == ImGuiKey_RightBracket || key == ImGuiKey_GraveAccent
            || key == ImGuiKey_KeypadDecimal || key == ImGuiKey_KeypadDivide || key == ImGuiKey_KeypadMultiply || key == ImGuiKey_KeypadSubtract || key == ImGuiKey_KeypadAdd || key == ImGuiKey_KeypadEqual)
            g.KeysMayBeCharInput.SetBit(key);

#ifdef IMGUI_HAS_DOCK
#endif

#ifdef IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS
    DebugLog("IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS is enabled.\nMust disable after use! Otherwise Dear ImGui will run slower.\n");
#endif

    ImFontAtlas* atlas = g.IO.Fonts;
    g.DrawListSharedData.Context = &g;
    RegisterFontAtlas(atlas);

    g.Initialized = true;
}

void ImGui::Shutdown()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR(g.IO.BackendPlatformUserData == NULL, "Forgot to shutdown Platform backend?");
    IM_ASSERT_USER_ERROR(g.IO.BackendRendererUserData == NULL, "Forgot to shutdown Renderer backend?");

    for (ImFontAtlas* atlas : g.FontAtlases)
    {
        UnregisterFontAtlas(atlas);
        if (atlas->RefCount == 0)
        {
            atlas->Locked = false;
            IM_DELETE(atlas);
        }
    }
    g.DrawListSharedData.TempBuffer.clear();

    if (!g.Initialized)
        return;

    if (g.SettingsLoaded && g.IO.IniFilename != NULL)
        SaveIniSettingsToDisk(g.IO.IniFilename);

    CallContextHooks(&g, ImGuiContextHookType_Shutdown);

    g.Windows.clear_delete();
    g.WindowsFocusOrder.clear();
    g.WindowsTempSortBuffer.clear();
    g.CurrentWindow = NULL;
    g.CurrentWindowStack.clear();
    g.WindowsById.Clear();
    g.NavWindow = NULL;
    g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;
    g.ActiveIdWindow = NULL;
    g.MovingWindow = NULL;

    g.KeysRoutingTable.Clear();

    g.ColorStack.clear();
    g.StyleVarStack.clear();
    g.FontStack.clear();
    g.OpenPopupStack.clear();
    g.BeginPopupStack.clear();
    g.TreeNodeStack.clear();

    g.Viewports.clear_delete();

    g.TabBars.Clear();
    g.CurrentTabBarStack.clear();
    g.ShrinkWidthBuffer.clear();

    g.ClipperTempData.clear_destruct();

    g.Tables.Clear();
    g.TablesTempData.clear_destruct();
    g.DrawChannelsTempMergeBuffer.clear();

    g.MultiSelectStorage.Clear();
    g.MultiSelectTempData.clear_destruct();

    g.ClipboardHandlerData.clear();
    g.MenusIdSubmittedThisFrame.clear();
    g.InputTextState.ClearFreeMemory();
    g.InputTextLineIndex.clear();
    g.InputTextDeactivatedState.ClearFreeMemory();

    g.SettingsWindows.clear();
    g.SettingsHandlers.clear();

    if (g.LogFile)
    {
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
        if (g.LogFile != stdout)
#endif
            ImFileClose(g.LogFile);
        g.LogFile = NULL;
    }
    g.LogBuffer.clear();
    g.DebugLogBuf.clear();
    g.DebugLogIndex.clear();

    g.Initialized = false;
}

void ImGui::SetContextName(ImGuiContext* ctx, const char* name)
{
    ImStrncpy(ctx->ContextName, name, IM_COUNTOF(ctx->ContextName));
}

ImGuiID ImGui::AddContextHook(ImGuiContext* ctx, const ImGuiContextHook* hook)
{
    ImGuiContext& g = *ctx;
    IM_ASSERT(hook->Callback != NULL && hook->HookId == 0 && hook->Type != ImGuiContextHookType_PendingRemoval_);
    g.Hooks.push_back(*hook);
    g.Hooks.back().HookId = ++g.HookIdNext;
    return g.HookIdNext;
}

void ImGui::RemoveContextHook(ImGuiContext* ctx, ImGuiID hook_id)
{
    ImGuiContext& g = *ctx;
    IM_ASSERT(hook_id != 0);
    for (ImGuiContextHook& hook : g.Hooks)
        if (hook.HookId == hook_id)
            hook.Type = ImGuiContextHookType_PendingRemoval_;
}

void ImGui::CallContextHooks(ImGuiContext* ctx, ImGuiContextHookType hook_type)
{
    ImGuiContext& g = *ctx;
    for (ImGuiContextHook& hook : g.Hooks)
        if (hook.Type == hook_type)
            hook.Callback(&g, &hook);
}

ImGuiWindow::ImGuiWindow(ImGuiContext* ctx, const char* name) : DrawListInst(NULL)
{
    memset((void*)this, 0, sizeof(*this));
    Ctx = ctx;
    Name = ImStrdup(name);
    NameBufLen = (int)ImStrlen(name) + 1;
    ID = ImHashStr(name);
    IDStack.push_back(ID);
    MoveId = GetID("#MOVE");
    ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);
    ScrollTargetCenterRatio = ImVec2(0.5f, 0.5f);
    AutoPosLastDirection = ImGuiDir_None;
    AutoFitFramesX = AutoFitFramesY = -1;
    SetWindowPosAllowFlags = SetWindowSizeAllowFlags = SetWindowCollapsedAllowFlags = 0;
    SetWindowPosVal = SetWindowPosPivot = ImVec2(FLT_MAX, FLT_MAX);
    LastFrameActive = -1;
    LastTimeActive = -1.0f;
    FontRefSize = 0.0f;
    FontWindowScale = FontWindowScaleParents = 1.0f;
    SettingsOffset = -1;
    DrawList = &DrawListInst;
    DrawList->_OwnerName = Name;
    DrawList->_SetDrawListSharedData(&Ctx->DrawListSharedData);
    NavPreferredScoringPosRel[0] = NavPreferredScoringPosRel[1] = ImVec2(FLT_MAX, FLT_MAX);
}

ImGuiWindow::~ImGuiWindow()
{
    IM_ASSERT(DrawList == &DrawListInst);
    IM_DELETE(Name);
    ColumnsStorage.clear_destruct();
}

static void SetCurrentWindow(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    g.CurrentWindow = window;
    g.StackSizesInBeginForCurrentWindow = g.CurrentWindow ? &g.CurrentWindowStack.back().StackSizesInBegin : NULL;
    g.CurrentTable = window && window->DC.CurrentTableIdx != -1 ? g.Tables.GetByIndex(window->DC.CurrentTableIdx) : NULL;
    g.CurrentDpiScale = 1.0f;
    if (window)
    {
        if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures)
        {
            ImGuiViewport* viewport = window->Viewport;
            g.FontRasterizerDensity = (viewport->FramebufferScale.x != 0.0f) ? viewport->FramebufferScale.x : g.IO.DisplayFramebufferScale.x;
        }
        const bool backup_skip_items = window->SkipItems;
        window->SkipItems = false;
        ImGui::UpdateCurrentFontSize(0.0f);
        window->SkipItems = backup_skip_items;
        ImGui::NavUpdateCurrentWindowIsScrollPushableX();
    }
}

void ImGui::GcCompactTransientMiscBuffers()
{
    ImGuiContext& g = *GImGui;
    g.ItemFlagsStack.clear();
    g.GroupStack.clear();
    g.InputTextLineIndex.clear();
    g.MultiSelectTempDataStacked = 0;
    g.MultiSelectTempData.clear_destruct();
    TableGcCompactSettings();
    for (ImFontAtlas* atlas : g.FontAtlases)
        atlas->CompactCache();
}

void ImGui::GcCompactTransientWindowBuffers(ImGuiWindow* window)
{
    window->MemoryCompacted = true;
    window->MemoryDrawListIdxCapacity = ImMin((int)(window->DrawList->IdxBuffer.Size * 1.05f), window->DrawList->IdxBuffer.Capacity);
    window->MemoryDrawListVtxCapacity = ImMin((int)(window->DrawList->VtxBuffer.Size * 1.05f), window->DrawList->VtxBuffer.Capacity);
    window->IDStack.clear();
    window->DrawList->_ClearFreeMemory();
    window->DC.ChildWindows.clear();
    window->DC.ItemWidthStack.clear();
    window->DC.TextWrapPosStack.clear();
}

void ImGui::GcAwakeTransientWindowBuffers(ImGuiWindow* window)
{

    window->MemoryCompacted = false;
    window->DrawList->IdxBuffer.reserve(window->MemoryDrawListIdxCapacity);
    window->DrawList->VtxBuffer.reserve(window->MemoryDrawListVtxCapacity);
    window->MemoryDrawListIdxCapacity = window->MemoryDrawListVtxCapacity = 0;
}

void ImGui::SetActiveID(ImGuiID id, ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;

    if (g.ActiveId != 0)
    {

        ImGuiDeactivatedItemData* deactivated_data = &g.DeactivatedItemData;
        deactivated_data->ID = g.ActiveId;
        deactivated_data->ElapseFrame = (g.LastItemData.ID == g.ActiveId) ? g.FrameCount : g.FrameCount + 1;
        deactivated_data->HasBeenEditedBefore = g.ActiveIdHasBeenEditedBefore;
        deactivated_data->IsAlive = (g.ActiveIdIsAlive == g.ActiveId);

        if (g.InputTextState.ID == g.ActiveId)
            InputTextDeactivateHook(g.ActiveId);

        if (g.MovingWindow != NULL && g.ActiveId == g.MovingWindow->MoveId)
        {
            IMGUI_DEBUG_LOG_ACTIVEID("SetActiveID() cancel MovingWindow\n");
            StopMouseMovingWindow();
        }
    }

    g.ActiveIdIsJustActivated = (g.ActiveId != id);
    if (g.ActiveIdIsJustActivated)
    {
        IMGUI_DEBUG_LOG_ACTIVEID("SetActiveID() old:0x%08X (window \"%s\") -> new:0x%08X (window \"%s\")\n", g.ActiveId, g.ActiveIdWindow ? g.ActiveIdWindow->Name : "", id, window ? window->Name : "");
        g.ActiveIdTimer = 0.0f;
        g.ActiveIdHasBeenPressedBefore = false;
        g.ActiveIdHasBeenEditedBefore = false;
        g.ActiveIdMouseButton = -1;
        if (id != 0)
        {
            g.LastActiveId = id;
            g.LastActiveIdTimer = 0.0f;
        }
    }
    g.ActiveId = id;
    g.ActiveIdAllowOverlap = false;
    g.ActiveIdNoClearOnFocusLoss = false;
    g.ActiveIdWindow = window;
    g.ActiveIdHasBeenEditedThisFrame = false;
    g.ActiveIdFromShortcut = false;
    g.ActiveIdDisabledId = 0;
    if (id)
    {
        g.ActiveIdIsAlive = id;
        g.ActiveIdSource = (g.NavActivateId == id || g.NavJustMovedToId == id) ? g.NavInputSource : ImGuiInputSource_Mouse;
        IM_ASSERT(g.ActiveIdSource != ImGuiInputSource_None);
    }

    g.ActiveIdUsingNavDirMask = 0x00;
    g.ActiveIdUsingAllKeyboardKeys = false;
}

void ImGui::ClearActiveID()
{
    SetActiveID(0, NULL);
}

void ImGui::SetHoveredID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.HoveredId = id;
    g.HoveredIdAllowOverlap = false;
    if (id != 0 && g.HoveredIdPreviousFrame != id)
        g.HoveredIdTimer = g.HoveredIdNotActiveTimer = 0.0f;
}

ImGuiID ImGui::GetHoveredID()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredId ? g.HoveredId : g.HoveredIdPreviousFrame;
}

void ImGui::MarkItemEdited(ImGuiID id)
{

    ImGuiContext& g = *GImGui;
    if (g.LastItemData.ItemFlags & ImGuiItemFlags_NoMarkEdited)
        return;
    if (g.ActiveId == id || g.ActiveId == 0)
    {

        g.ActiveIdHasBeenEditedThisFrame = true;
        g.ActiveIdHasBeenEditedBefore = true;
        if (g.DeactivatedItemData.ID == id)
            g.DeactivatedItemData.HasBeenEditedBefore = true;
    }

    IM_ASSERT(g.DragDropActive || g.ActiveId == id || g.ActiveId == 0 || g.ActiveIdPreviousFrame == id || g.NavJustMovedToId || (g.CurrentMultiSelect != NULL && g.BoxSelectState.IsActive));

    g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Edited;
}

bool ImGui::IsWindowContentHoverable(ImGuiWindow* window, ImGuiHoveredFlags flags)
{

    ImGuiContext& g = *GImGui;
    if (g.NavWindow)
        if (ImGuiWindow* focused_root_window = g.NavWindow->RootWindow)
            if (focused_root_window->WasActive && focused_root_window != window->RootWindow)
            {

                bool want_inhibit = false;
                if (focused_root_window->Flags & ImGuiWindowFlags_Modal)
                    want_inhibit = true;
                else if ((focused_root_window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiHoveredFlags_AllowWhenBlockedByPopup))
                    want_inhibit = true;

                if (want_inhibit)
                    if (!IsWindowWithinBeginStackOf(window->RootWindow, focused_root_window))
                        return false;
            }
    return true;
}

static inline float CalcDelayFromHoveredFlags(ImGuiHoveredFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (flags & ImGuiHoveredFlags_DelayNormal)
        return g.Style.HoverDelayNormal;
    if (flags & ImGuiHoveredFlags_DelayShort)
        return g.Style.HoverDelayShort;
    return 0.0f;
}

static ImGuiHoveredFlags ApplyHoverFlagsForTooltip(ImGuiHoveredFlags user_flags, ImGuiHoveredFlags shared_flags)
{

    if (user_flags & (ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_DelayNormal))
        shared_flags &= ~(ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_DelayNormal);
    return user_flags | shared_flags;
}

bool ImGui::IsItemHovered(ImGuiHoveredFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT_USER_ERROR((flags & ~ImGuiHoveredFlags_AllowedMaskForIsItemHovered) == 0, "Invalid flags for IsItemHovered()!");

    if (g.NavHighlightItemUnderNav && g.NavCursorVisible && !(flags & ImGuiHoveredFlags_NoNavOverride))
    {
        if (!IsItemFocused())
            return false;
        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_Disabled) && !(flags & ImGuiHoveredFlags_AllowWhenDisabled))
            return false;

        if (flags & ImGuiHoveredFlags_ForTooltip)
            flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipNav);
    }
    else
    {

        ImGuiItemStatusFlags status_flags = g.LastItemData.StatusFlags;
        if (!(status_flags & ImGuiItemStatusFlags_HoveredRect))
            return false;

        if (flags & ImGuiHoveredFlags_ForTooltip)
            flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipMouse);

        if (g.HoveredWindow != window && (status_flags & ImGuiItemStatusFlags_HoveredWindow) == 0)
            if ((flags & ImGuiHoveredFlags_AllowWhenOverlappedByWindow) == 0)
                return false;

        const ImGuiID id = g.LastItemData.ID;
        if ((flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) == 0)
            if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap && !g.ActiveIdFromShortcut)
            {

                bool cancel_is_hovered = true;
                if (g.ActiveId == window->MoveId && (id == 0 || g.ActiveIdDisabledId == id))
                    cancel_is_hovered = false;
                if (cancel_is_hovered)
                    return false;
            }

        if (!IsWindowContentHoverable(window, flags) && !(g.LastItemData.ItemFlags & ImGuiItemFlags_NoWindowHoverableCheck))
            return false;

        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_Disabled) && !(flags & ImGuiHoveredFlags_AllowWhenDisabled))
            return false;

        if (id == window->MoveId && window->WriteAccessed)
            return false;

        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_AllowOverlap) && id != 0)
            if ((flags & ImGuiHoveredFlags_AllowWhenOverlappedByItem) == 0)
                if (g.HoveredIdPreviousFrame != g.LastItemData.ID)
                    return false;
    }

    const float delay = CalcDelayFromHoveredFlags(flags);
    if (delay > 0.0f || (flags & ImGuiHoveredFlags_Stationary))
    {
        ImGuiID hover_delay_id = (g.LastItemData.ID != 0) ? g.LastItemData.ID : window->GetIDFromPos(g.LastItemData.Rect.Min);
        if ((flags & ImGuiHoveredFlags_NoSharedDelay) && (g.HoverItemDelayIdPreviousFrame != hover_delay_id))
            g.HoverItemDelayTimer = 0.0f;
        g.HoverItemDelayId = hover_delay_id;

        if ((flags & ImGuiHoveredFlags_Stationary) != 0 && g.HoverItemUnlockedStationaryId != hover_delay_id)
            return false;

        if (g.HoverItemDelayTimer < delay)
            return false;
    }

    return true;
}

bool ImGui::ItemHoverable(const ImRect& bb, ImGuiID id, ImGuiItemFlags item_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (id != 0 && g.HoveredIdPreviousFrame == id && (item_flags & ImGuiItemFlags_AllowDuplicateId) == 0)
    {
        g.HoveredIdPreviousFrameItemCount++;
        if (g.DebugDrawIdConflictsId == id)
            window->DrawList->AddRect(bb.Min - ImVec2(1,1), bb.Max + ImVec2(1,1), IM_COL32(255, 0, 0, 255), 0.0f, ImDrawFlags_None, 2.0f);
    }
#endif

    if (g.HoveredWindow != window)
        return false;
    if (!IsMouseHoveringRect(bb.Min, bb.Max))
        return false;

    if (g.HoveredId != 0 && g.HoveredId != id && !g.HoveredIdAllowOverlap)
        return false;
    if (g.ActiveId != 0 && g.ActiveId != id && !g.ActiveIdAllowOverlap)
        if (!g.ActiveIdFromShortcut)
            return false;

    if (!(item_flags & ImGuiItemFlags_NoWindowHoverableCheck) && !IsWindowContentHoverable(window, ImGuiHoveredFlags_None))
    {
        g.HoveredIdIsDisabled = true;
        return false;
    }

    if (id != 0)
    {

        if (g.DragDropActive && g.DragDropPayload.SourceId == id && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoDisableHover))
            return false;

        SetHoveredID(id);

        if (item_flags & ImGuiItemFlags_AllowOverlap)
        {
            g.HoveredIdAllowOverlap = true;
            if (g.HoveredIdPreviousFrame != id)
                return false;
        }

        if (id == g.LastItemData.ID && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasShortcut) && g.ActiveId != id)
            if (IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal))
                SetTooltip("%s", GetKeyChordName(g.LastItemData.Shortcut));
    }

    if (item_flags & ImGuiItemFlags_Disabled)
    {

        if (g.ActiveId == id && id != 0)
            ClearActiveID();
        g.HoveredIdIsDisabled = true;
        return false;
    }

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (id != 0)
    {

        if (g.DebugItemPickerActive && g.HoveredIdPreviousFrame == id)
            GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(255, 255, 0, 255));
        if (g.DebugItemPickerBreakId == id)
            IM_DEBUG_BREAK();
    }
#endif

    if (g.NavHighlightItemUnderNav && (item_flags & ImGuiItemFlags_NoNavDisableMouseHover) == 0)
        return false;

    return true;
}

bool ImGui::IsClippedEx(const ImRect& bb, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!bb.Overlaps(window->ClipRect))
        if (id == 0 || (id != g.ActiveId && id != g.ActiveIdPreviousFrame && id != g.NavId && id != g.NavActivateId))
            if (!g.ItemUnclipByLog)
                return true;
    return false;
}

void ImGui::SetLastItemData(ImGuiID item_id, ImGuiItemFlags item_flags, ImGuiItemStatusFlags status_flags, const ImRect& item_rect)
{
    ImGuiContext& g = *GImGui;
    g.LastItemData.ID = item_id;
    g.LastItemData.ItemFlags = item_flags;
    g.LastItemData.StatusFlags = status_flags;
    g.LastItemData.Rect = g.LastItemData.NavRect = item_rect;
}

static void ImGui::SetLastItemDataForWindow(ImGuiWindow* window, const ImRect& rect)
{
    ImGuiContext& g = *GImGui;
    SetLastItemData(window->MoveId, g.CurrentItemFlags, window->DC.WindowItemStatusFlags, rect);
}

static void ImGui::SetLastItemDataForChildWindowItem(ImGuiWindow* window, const ImRect& rect)
{
    ImGuiContext& g = *GImGui;
    SetLastItemData(window->ChildId, g.CurrentItemFlags, window->DC.ChildItemStatusFlags, rect);
}

float ImGui::CalcWrapWidthForPos(const ImVec2& pos, float wrap_pos_x)
{
    if (wrap_pos_x < 0.0f)
        return 0.0f;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (wrap_pos_x == 0.0f)
    {

        wrap_pos_x = window->WorkRect.Max.x;
    }
    else if (wrap_pos_x > 0.0f)
    {
        wrap_pos_x += window->Pos.x - window->Scroll.x;
    }

    return ImMax(wrap_pos_x - pos.x, 1.0f);
}

void* ImGui::MemAlloc(size_t size)
{
    void* ptr = (*GImAllocatorAllocFunc)(size, GImAllocatorUserData);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (ImGuiContext* ctx = GImGui)
        DebugAllocHook(&ctx->DebugAllocInfo, ctx->FrameCount, ptr, size);
#endif
    return ptr;
}

void ImGui::MemFree(void* ptr)
{
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (ptr != NULL)
        if (ImGuiContext* ctx = GImGui)
            DebugAllocHook(&ctx->DebugAllocInfo, ctx->FrameCount, ptr, (size_t)-1);
#endif
    return (*GImAllocatorFreeFunc)(ptr, GImAllocatorUserData);
}

void ImGui::DemoMarker(const char* file, int line, const char* section)
{
    ImGuiContext& g = *GImGui;
    if (g.DemoMarkerCallback != NULL)
        g.DemoMarkerCallback(file, line, section);
}

void ImGui::DebugAllocHook(ImGuiDebugAllocInfo* info, int frame_count, void* ptr, size_t size)
{
    ImGuiDebugAllocEntry* entry = &info->LastEntriesBuf[info->LastEntriesIdx];
    IM_UNUSED(ptr);
    if (entry->FrameCount != frame_count)
    {
        info->LastEntriesIdx = (info->LastEntriesIdx + 1) % IM_COUNTOF(info->LastEntriesBuf);
        entry = &info->LastEntriesBuf[info->LastEntriesIdx];
        entry->FrameCount = frame_count;
        entry->AllocCount = entry->FreeCount = 0;
    }
    if (size != (size_t)-1)
    {

        entry->AllocCount++;
        info->TotalAllocCount++;
    }
    else
    {

        entry->FreeCount++;
        info->TotalFreeCount++;
    }
}

const char* ImGui::GetClipboardText()
{
    ImGuiContext& g = *GImGui;
    return g.PlatformIO.Platform_GetClipboardTextFn ? g.PlatformIO.Platform_GetClipboardTextFn(&g) : NULL;
}

void ImGui::SetClipboardText(const char* text)
{
    ImGuiContext& g = *GImGui;
    if (g.PlatformIO.Platform_SetClipboardTextFn != NULL)
        g.PlatformIO.Platform_SetClipboardTextFn(&g, text);
}

const char* ImGui::GetVersion()
{
    return IMGUI_VERSION;
}

ImGuiIO& ImGui::GetIO()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    return GImGui->IO;
}

ImGuiIO& ImGui::GetIO(ImGuiContext* ctx)
{
    IM_ASSERT(ctx != NULL);
    return ctx->IO;
}

ImGuiPlatformIO& ImGui::GetPlatformIO()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext()?");
    return GImGui->PlatformIO;
}

ImGuiPlatformIO& ImGui::GetPlatformIO(ImGuiContext* ctx)
{
    IM_ASSERT(ctx != NULL);
    return ctx->PlatformIO;
}

ImDrawData* ImGui::GetDrawData()
{
    ImGuiContext& g = *GImGui;
    ImGuiViewportP* viewport = g.Viewports[0];
    return viewport->DrawDataP.Valid ? &viewport->DrawDataP : NULL;
}

double ImGui::GetTime()
{
    return GImGui->Time;
}

int ImGui::GetFrameCount()
{
    return GImGui->FrameCount;
}

static ImDrawList* GetViewportBgFgDrawList(ImGuiViewportP* viewport, size_t drawlist_no, const char* drawlist_name)
{

    ImGuiContext& g = *GImGui;
    IM_ASSERT(drawlist_no < IM_COUNTOF(viewport->BgFgDrawLists));
    ImDrawList* draw_list = viewport->BgFgDrawLists[drawlist_no];
    if (draw_list == NULL)
    {
        draw_list = IM_NEW(ImDrawList)(&g.DrawListSharedData);
        draw_list->_OwnerName = drawlist_name;
        viewport->BgFgDrawLists[drawlist_no] = draw_list;
    }

    if (viewport->BgFgDrawListsLastTimeActive[drawlist_no] != (float)g.Time)
    {
        draw_list->_ResetForNewFrame();
        draw_list->PushTexture(g.IO.Fonts->TexRef);
        draw_list->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size, false);
        viewport->BgFgDrawListsLastTimeActive[drawlist_no] = (float)g.Time;
    }
    return draw_list;
}

ImDrawList* ImGui::GetBackgroundDrawList(ImGuiViewport* viewport)
{
    return GetViewportBgFgDrawList((ImGuiViewportP*)viewport, 0, "##Background");
}

ImDrawList* ImGui::GetBackgroundDrawList()
{
    ImGuiContext& g = *GImGui;
    return GetBackgroundDrawList(g.Viewports[0]);
}

ImDrawList* ImGui::GetForegroundDrawList(ImGuiViewport* viewport)
{
    return GetViewportBgFgDrawList((ImGuiViewportP*)viewport, 1, "##Foreground");
}

ImDrawList* ImGui::GetForegroundDrawList()
{
    ImGuiContext& g = *GImGui;
    return GetForegroundDrawList(g.Viewports[0]);
}

ImDrawListSharedData* ImGui::GetDrawListSharedData()
{
    return &GImGui->DrawListSharedData;
}

void ImGui::StartMouseMovingWindow(ImGuiWindow* window)
{

    ImGuiContext& g = *GImGui;
    FocusWindow(window);
    SetActiveID(window->MoveId, window);
    if (g.IO.ConfigNavCursorVisibleAuto)
        g.NavCursorVisible = false;
    g.ActiveIdClickOffset = g.IO.MouseClickedPos[0] - window->RootWindow->Pos;
    g.ActiveIdNoClearOnFocusLoss = true;
    SetActiveIdUsingAllKeyboardKeys();

    bool can_move_window = true;
    if ((window->Flags & ImGuiWindowFlags_NoMove) || (window->RootWindow->Flags & ImGuiWindowFlags_NoMove))
        can_move_window = false;
    if (can_move_window)
        g.MovingWindow = window;
}

void ImGui::StopMouseMovingWindow()
{
    ImGuiContext& g = *GImGui;

    g.MovingWindow = NULL;
}

void ImGui::UpdateMouseMovingWindowNewFrame()
{
    ImGuiContext& g = *GImGui;
    if (g.MovingWindow != NULL)
    {

        KeepAliveID(g.ActiveId);
        IM_ASSERT(g.MovingWindow && g.MovingWindow->RootWindow);
        ImGuiWindow* moving_window = g.MovingWindow->RootWindow;
        if (g.IO.MouseDown[0] && IsMousePosValid(&g.IO.MousePos))
        {
            ImVec2 pos = g.IO.MousePos - g.ActiveIdClickOffset;
            SetWindowPos(moving_window, pos, ImGuiCond_Always);
            FocusWindow(g.MovingWindow);
        }
        else
        {
            StopMouseMovingWindow();
            ClearActiveID();
        }
    }
    else
    {

        if (g.ActiveIdWindow && g.ActiveIdWindow->MoveId == g.ActiveId)
        {
            KeepAliveID(g.ActiveId);
            if (!g.IO.MouseDown[0])
                ClearActiveID();
        }
    }
}

void ImGui::UpdateMouseMovingWindowEndFrame()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId != 0 || (g.HoveredId != 0 && !g.HoveredIdIsDisabled))
        return;

    if (g.NavWindow && g.NavWindow->Appearing)
        return;

    ImGuiWindow* hovered_window = g.HoveredWindow;

    if (g.IO.MouseClicked[0])
    {

        ImGuiWindow* hovered_root = hovered_window ? hovered_window->RootWindow : NULL;
        const bool is_closed_popup = hovered_root && (hovered_root->Flags & ImGuiWindowFlags_Popup) && !IsPopupOpen(hovered_root->PopupId, ImGuiPopupFlags_AnyPopupLevel);

        if (hovered_window != NULL && !is_closed_popup)
        {
            StartMouseMovingWindow(hovered_window);

            if ((hovered_window->BgClickFlags & ImGuiWindowBgClickFlags_Move) == 0)
                if (!(hovered_root->Flags & ImGuiWindowFlags_NoTitleBar))
                    if (!hovered_root->TitleBarRect().Contains(g.IO.MouseClickedPos[0]))
                        g.MovingWindow = NULL;

            if (g.HoveredIdIsDisabled)
            {
                g.MovingWindow = NULL;
                g.ActiveIdDisabledId = g.HoveredId;
            }
        }
        else if (hovered_window == NULL && g.NavWindow != NULL)
        {

            FocusWindow(NULL, ImGuiFocusRequestFlags_UnlessBelowModal);
        }
    }

    if (g.IO.MouseClicked[1] && g.HoveredId == 0)
    {

        ImGuiWindow* modal = GetTopMostPopupModal();
        bool hovered_window_above_modal = hovered_window && (modal == NULL || IsWindowAbove(hovered_window, modal));
        ClosePopupsOverWindow(hovered_window_above_modal ? hovered_window : modal, true);
    }
}

static bool IsWindowActiveAndVisible(ImGuiWindow* window)
{
    return window->Active && !window->Hidden;
}

void ImGui::UpdateHoveredWindowAndCaptureFlags(const ImVec2& mouse_pos)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    g.WindowsBorderHoverPadding = ImMax(ImMax(g.Style.TouchExtraPadding.x, g.Style.TouchExtraPadding.y), g.Style.WindowBorderHoverPadding);

    bool clear_hovered_windows = false;
    FindHoveredWindowEx(mouse_pos, false, &g.HoveredWindow, &g.HoveredWindowUnderMovingWindow);
    g.HoveredWindowBeforeClear = g.HoveredWindow;

    ImGuiWindow* modal_window = GetTopMostPopupModal();
    if (modal_window && g.HoveredWindow && !IsWindowWithinBeginStackOf(g.HoveredWindow->RootWindow, modal_window))
        clear_hovered_windows = true;

    if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)
        clear_hovered_windows = true;

    const bool has_open_popup = (g.OpenPopupStack.Size > 0);
    const bool has_open_modal = (modal_window != NULL);
    int mouse_earliest_down = -1;
    bool mouse_any_down = false;
    for (int i = 0; i < IM_COUNTOF(io.MouseDown); i++)
    {
        if (io.MouseClicked[i])
        {
            io.MouseDownOwned[i] = (g.HoveredWindow != NULL) || has_open_popup;
            io.MouseDownOwnedUnlessPopupClose[i] = (g.HoveredWindow != NULL) || has_open_modal;
        }
        mouse_any_down |= io.MouseDown[i];
        if (io.MouseDown[i] || io.MouseReleased[i])
            if (mouse_earliest_down == -1 || io.MouseClickedTime[i] < io.MouseClickedTime[mouse_earliest_down])
                mouse_earliest_down = i;
    }
    const bool mouse_avail = (mouse_earliest_down == -1) || io.MouseDownOwned[mouse_earliest_down];
    const bool mouse_avail_unless_popup_close = (mouse_earliest_down == -1) || io.MouseDownOwnedUnlessPopupClose[mouse_earliest_down];

    const bool mouse_dragging_extern_payload = g.DragDropActive && (g.DragDropSourceFlags & ImGuiDragDropFlags_SourceExtern) != 0;
    if (!mouse_avail && !mouse_dragging_extern_payload)
        clear_hovered_windows = true;

    if (clear_hovered_windows)
        g.HoveredWindow = g.HoveredWindowUnderMovingWindow = NULL;

    if (g.WantCaptureMouseNextFrame != -1)
    {
        io.WantCaptureMouse = io.WantCaptureMouseUnlessPopupClose = (g.WantCaptureMouseNextFrame != 0);
    }
    else
    {
        io.WantCaptureMouse = (mouse_avail && (g.HoveredWindow != NULL || mouse_any_down)) || has_open_popup;
        io.WantCaptureMouseUnlessPopupClose = (mouse_avail_unless_popup_close && (g.HoveredWindow != NULL || mouse_any_down)) || has_open_modal;
    }

    io.WantCaptureKeyboard = false;
    if ((io.ConfigFlags & ImGuiConfigFlags_NoKeyboard) == 0)
    {
        if ((g.ActiveId != 0) || (modal_window != NULL))
            io.WantCaptureKeyboard = true;
        else if (io.NavActive && (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) && io.ConfigNavCaptureKeyboard)
            io.WantCaptureKeyboard = true;
    }
    if (g.WantCaptureKeyboardNextFrame != -1)
        io.WantCaptureKeyboard = (g.WantCaptureKeyboardNextFrame != 0);

    io.WantTextInput = (g.WantTextInputNextFrame != -1) ? (g.WantTextInputNextFrame != 0) : false;
}

static void SetupDrawListSharedData()
{
    ImGuiContext& g = *GImGui;
    ImRect virtual_space(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (ImGuiViewportP* viewport : g.Viewports)
        virtual_space.Add(viewport->GetMainRect());
    g.DrawListSharedData.ClipRectFullscreen = virtual_space.ToVec4();
    g.DrawListSharedData.CurveTessellationTol = g.Style.CurveTessellationTol;
    g.DrawListSharedData.SetCircleTessellationMaxError(g.Style.CircleTessellationMaxError);
    g.DrawListSharedData.InitialFlags = ImDrawListFlags_None;
    if (g.Style.AntiAliasedLines)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLines;
    if (g.Style.AntiAliasedLinesUseTex && !(g.IO.Fonts->Flags & ImFontAtlasFlags_NoBakedLines))
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedLinesUseTex;
    if (g.Style.AntiAliasedFill)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AntiAliasedFill;
    if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)
        g.DrawListSharedData.InitialFlags |= ImDrawListFlags_AllowVtxOffset;
    g.DrawListSharedData.InitialFringeScale = 1.0f;
}

void ImGui::NewFrame()
{
    IM_ASSERT(GImGui != NULL && "No current context. Did you call ImGui::CreateContext() and ImGui::SetCurrentContext() ?");
    ImGuiContext& g = *GImGui;

    for (int n = g.Hooks.Size - 1; n >= 0; n--)
        if (g.Hooks[n].Type == ImGuiContextHookType_PendingRemoval_)
            g.Hooks.erase(&g.Hooks[n]);

    CallContextHooks(&g, ImGuiContextHookType_NewFramePre);

    ErrorCheckNewFrameSanityChecks();

    UpdateSettings();

    g.Time += g.IO.DeltaTime;
    g.FrameCount += 1;
    g.TooltipOverrideCount = 0;
    g.WindowsActiveCount = 0;
    g.MenusIdSubmittedThisFrame.resize(0);

    g.FramerateSecPerFrameAccum += g.IO.DeltaTime - g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx];
    g.FramerateSecPerFrame[g.FramerateSecPerFrameIdx] = g.IO.DeltaTime;
    g.FramerateSecPerFrameIdx = (g.FramerateSecPerFrameIdx + 1) % IM_COUNTOF(g.FramerateSecPerFrame);
    g.FramerateSecPerFrameCount = ImMin(g.FramerateSecPerFrameCount + 1, IM_COUNTOF(g.FramerateSecPerFrame));
    g.IO.Framerate = (g.FramerateSecPerFrameAccum > 0.0f) ? (1.0f / (g.FramerateSecPerFrameAccum / (float)g.FramerateSecPerFrameCount)) : FLT_MAX;

    g.InputEventsTrail.resize(0);
    UpdateInputEvents(g.IO.ConfigInputTrickleEventQueue);

    UpdateViewportsNewFrame();

    UpdateTexturesNewFrame();

    SetupDrawListSharedData();
    UpdateFontsNewFrame();

    g.WithinFrameScope = true;

    for (ImGuiViewportP* viewport : g.Viewports)
        viewport->DrawDataP.Valid = false;

    if (g.DragDropActive && g.DragDropPayload.SourceId == g.ActiveId)
        KeepAliveID(g.DragDropPayload.SourceId);

    if (!g.IO.ConfigDebugHighlightIdConflicts || !g.IO.KeyCtrl)
        g.DebugDrawIdConflictsId = 0;
    if (g.IO.ConfigDebugHighlightIdConflicts && g.HoveredIdPreviousFrameItemCount > 1)
        g.DebugDrawIdConflictsId = g.HoveredIdPreviousFrame;

    if (!g.HoveredIdPreviousFrame)
        g.HoveredIdTimer = 0.0f;
    if (!g.HoveredIdPreviousFrame || (g.HoveredId && g.ActiveId == g.HoveredId))
        g.HoveredIdNotActiveTimer = 0.0f;
    if (g.HoveredId)
        g.HoveredIdTimer += g.IO.DeltaTime;
    if (g.HoveredId && g.ActiveId != g.HoveredId)
        g.HoveredIdNotActiveTimer += g.IO.DeltaTime;
    g.HoveredIdPreviousFrame = g.HoveredId;
    g.HoveredIdPreviousFrameItemCount = 0;
    g.HoveredId = 0;
    g.HoveredIdAllowOverlap = false;
    g.HoveredIdIsDisabled = false;

    if (g.ActiveId != 0 && g.ActiveIdIsAlive != g.ActiveId && g.ActiveIdPreviousFrame == g.ActiveId)
    {
        IMGUI_DEBUG_LOG_ACTIVEID("NewFrame(): ClearActiveID() 0x%08X because it isn't marked alive anymore!\n", g.ActiveId);
        ClearActiveID();
    }

    if (g.ActiveId)
        g.ActiveIdTimer += g.IO.DeltaTime;
    g.LastActiveIdTimer += g.IO.DeltaTime;
    g.ActiveIdPreviousFrame = g.ActiveId;
    g.ActiveIdIsAlive = 0;
    g.ActiveIdHasBeenEditedThisFrame = false;
    g.ActiveIdIsJustActivated = false;
    if (g.TempInputId != 0 && g.ActiveId != g.TempInputId)
        g.TempInputId = 0;
    if (g.InputTextReactivateId != 0 && g.InputTextReactivateId != g.DeactivatedItemData.ID)
        g.InputTextReactivateId = 0;
    if (g.ActiveId == 0)
    {
        g.ActiveIdUsingNavDirMask = 0x00;
        g.ActiveIdUsingAllKeyboardKeys = false;
    }
    if (g.DeactivatedItemData.ElapseFrame < g.FrameCount)
        g.DeactivatedItemData.ID = 0;
    g.DeactivatedItemData.IsAlive = false;

    if (g.HoverItemDelayId != 0 && g.MouseStationaryTimer >= g.Style.HoverStationaryDelay)
        g.HoverItemUnlockedStationaryId = g.HoverItemDelayId;
    else if (g.HoverItemDelayId == 0)
        g.HoverItemUnlockedStationaryId = 0;
    if (g.HoveredWindow != NULL && g.MouseStationaryTimer >= g.Style.HoverStationaryDelay)
        g.HoverWindowUnlockedStationaryId = g.HoveredWindow->ID;
    else if (g.HoveredWindow == NULL)
        g.HoverWindowUnlockedStationaryId = 0;

    g.HoverItemDelayIdPreviousFrame = g.HoverItemDelayId;
    if (g.HoverItemDelayId != 0)
    {
        g.HoverItemDelayTimer += g.IO.DeltaTime;
        g.HoverItemDelayClearTimer = 0.0f;
        g.HoverItemDelayId = 0;
    }
    else if (g.HoverItemDelayTimer > 0.0f)
    {

        g.HoverItemDelayClearTimer += g.IO.DeltaTime;
        if (g.HoverItemDelayClearTimer >= ImMax(0.25f, g.IO.DeltaTime * 2.0f))
            g.HoverItemDelayTimer = g.HoverItemDelayClearTimer = 0.0f;
    }

    UpdateKeyboardInputs();

    g.DragDropAcceptIdPrev = g.DragDropAcceptIdCurr;
    g.DragDropAcceptIdCurr = 0;
    g.DragDropAcceptFlagsPrev = g.DragDropAcceptFlagsCurr;
    g.DragDropAcceptFlagsCurr = ImGuiDragDropFlags_None;
    g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
    g.DragDropWithinSource = false;
    g.DragDropWithinTarget = false;
    g.DragDropHoldJustPressedId = 0;
    if (g.DragDropActive)
    {

        ImGuiID owner_id = g.ActiveId ? g.ActiveId : ImHashStr("##DragDropCancelHandler");
        if (Shortcut(ImGuiKey_Escape, ImGuiInputFlags_RouteGlobal, owner_id))
        {
            ClearActiveID();
            ClearDragDrop();
        }
    }
    g.TooltipPreviousWindow = NULL;

    NavUpdate();

    UpdateMouseInputs();

    IM_ASSERT(g.WindowsFocusOrder.Size <= g.Windows.Size);
    const bool gc_all = (g.GcCompactAll || g.IO.ConfigMemoryCompactTimer < 0.0f);
    const float memory_compact_start_time = gc_all ? FLT_MAX : (float)g.Time - g.IO.ConfigMemoryCompactTimer;
    for (ImGuiWindow* window : g.Windows)
    {
        window->WasActive = window->Active;
        window->Active = false;
        window->WriteAccessed = false;
        window->BeginCountPreviousFrame = window->BeginCount;
        window->BeginCount = 0;

        if ((!window->WasActive || gc_all) && !window->MemoryCompacted && window->LastTimeActive < memory_compact_start_time)
            GcCompactTransientWindowBuffers(window);
    }

    UpdateHoveredWindowAndCaptureFlags(g.IO.MousePos);

    UpdateMouseMovingWindowNewFrame();

    if (GetTopMostPopupModal() != NULL || (g.NavWindowingTarget != NULL && g.NavWindowingHighlightAlpha > 0.0f))
        g.DimBgRatio = ImMin(g.DimBgRatio + g.IO.DeltaTime * 6.0f, 1.0f);
    else
        g.DimBgRatio = ImMax(g.DimBgRatio - g.IO.DeltaTime * 10.0f, 0.0f);

    g.MouseCursor = ImGuiMouseCursor_Arrow;
    g.WantCaptureMouseNextFrame = g.WantCaptureKeyboardNextFrame = g.WantTextInputNextFrame = -1;

    g.PlatformImeDataPrev = g.PlatformImeData;
    g.PlatformImeData.WantVisible = g.PlatformImeData.WantTextInput = false;

    UpdateMouseWheel();

    for (int i = 0; i < g.TablesLastTimeActive.Size; i++)
        if (g.TablesLastTimeActive[i] >= 0.0f && g.TablesLastTimeActive[i] < memory_compact_start_time)
            TableGcCompactTransientBuffers(g.Tables.GetByIndex(i));
    for (ImGuiTableTempData& table_temp_data : g.TablesTempData)
        if (table_temp_data.LastTimeActive >= 0.0f && table_temp_data.LastTimeActive < memory_compact_start_time)
            TableGcCompactTransientBuffers(&table_temp_data);
    if (g.GcCompactAll)
        GcCompactTransientMiscBuffers();
    g.GcCompactAll = false;

    if (g.NavWindow && !g.NavWindow->WasActive)
        FocusTopMostWindowUnderOne(NULL, NULL, NULL, ImGuiFocusRequestFlags_RestoreFocusedChild);

    g.CurrentWindowStack.resize(0);
    g.BeginPopupStack.resize(0);
    g.ItemFlagsStack.resize(0);
    g.ItemFlagsStack.push_back(ImGuiItemFlags_AutoClosePopups);
    g.CurrentItemFlags = g.ItemFlagsStack.back();
    g.GroupStack.resize(0);

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    UpdateDebugToolItemPicker();
    UpdateDebugToolItemPathQuery();
    UpdateDebugToolFlashStyleColor();
    if (g.DebugLocateFrames > 0 && --g.DebugLocateFrames == 0)
    {
        g.DebugLocateId = 0;
        g.DebugBreakInLocateId = false;
    }
    if (g.DebugLogAutoDisableFrames > 0 && --g.DebugLogAutoDisableFrames == 0)
    {
        DebugLog("(Debug Log: Auto-disabled some ImGuiDebugLogFlags after 2 frames)\n");
        g.DebugLogFlags &= ~g.DebugLogAutoDisableFlags;
        g.DebugLogAutoDisableFlags = ImGuiDebugLogFlags_None;
    }
#endif

    g.WithinFrameScopeWithImplicitWindow = true;
    SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    Begin("Debug##Default");
    IM_ASSERT(g.CurrentWindow->IsFallbackWindow == true);

    g.ErrorCountCurrentFrame = 0;
    ErrorRecoveryStoreState(&g.StackSizesInNewFrame);

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (g.IO.ConfigDebugBeginReturnValueLoop)
        g.DebugBeginReturnValueCullDepth = (g.DebugBeginReturnValueCullDepth == -1) ? 0 : ((g.DebugBeginReturnValueCullDepth + ((g.FrameCount % 4) == 0 ? 1 : 0)) % 10);
    else
        g.DebugBeginReturnValueCullDepth = -1;
#endif

    CallContextHooks(&g, ImGuiContextHookType_NewFramePost);
}

static int IMGUI_CDECL ChildWindowComparer(const void* lhs, const void* rhs)
{
    const ImGuiWindow* const a = *(const ImGuiWindow* const *)lhs;
    const ImGuiWindow* const b = *(const ImGuiWindow* const *)rhs;
    if (int d = (a->Flags & ImGuiWindowFlags_Popup) - (b->Flags & ImGuiWindowFlags_Popup))
        return d;
    if (int d = (a->Flags & ImGuiWindowFlags_Tooltip) - (b->Flags & ImGuiWindowFlags_Tooltip))
        return d;
    return a->BeginOrderWithinParent - b->BeginOrderWithinParent;
}

static void AddWindowToSortBuffer(ImVector<ImGuiWindow*>* out_sorted_windows, ImGuiWindow* window)
{
    out_sorted_windows->push_back(window);
    if (window->Active)
    {
        int count = window->DC.ChildWindows.Size;
        ImQsort(window->DC.ChildWindows.Data, (size_t)count, sizeof(ImGuiWindow*), ChildWindowComparer);
        for (int i = 0; i < count; i++)
        {
            ImGuiWindow* child = window->DC.ChildWindows[i];
            if (child->Active)
                AddWindowToSortBuffer(out_sorted_windows, child);
        }
    }
}

static void AddWindowToDrawData(ImGuiWindow* window, int layer)
{
    ImGuiContext& g = *GImGui;
    ImGuiViewportP* viewport = g.Viewports[0];
    g.IO.MetricsRenderWindows++;
    if (window->DrawList->_Splitter._Count > 1)
        window->DrawList->ChannelsMerge();
    ImGui::AddDrawListToDrawDataEx(&viewport->DrawDataP, viewport->DrawDataBuilder.Layers[layer], window->DrawList);
    for (ImGuiWindow* child : window->DC.ChildWindows)
        if (IsWindowActiveAndVisible(child))
            AddWindowToDrawData(child, layer);
}

static inline int GetWindowDisplayLayer(ImGuiWindow* window)
{
    return (window->Flags & ImGuiWindowFlags_Tooltip) ? 1 : 0;
}

static inline void AddRootWindowToDrawData(ImGuiWindow* window)
{
    AddWindowToDrawData(window, GetWindowDisplayLayer(window));
}

static void FlattenDrawDataIntoSingleLayer(ImDrawDataBuilder* builder)
{
    int n = builder->Layers[0]->Size;
    int full_size = n;
    for (int i = 1; i < IM_COUNTOF(builder->Layers); i++)
        full_size += builder->Layers[i]->Size;
    builder->Layers[0]->resize(full_size);
    for (int layer_n = 1; layer_n < IM_COUNTOF(builder->Layers); layer_n++)
    {
        ImVector<ImDrawList*>* layer = builder->Layers[layer_n];
        if (layer->empty())
            continue;
        memcpy(builder->Layers[0]->Data + n, layer->Data, layer->Size * sizeof(ImDrawList*));
        n += layer->Size;
        layer->resize(0);
    }
}

static void InitViewportDrawData(ImGuiViewportP* viewport)
{
    ImGuiIO& io = ImGui::GetIO();
    ImDrawData* draw_data = &viewport->DrawDataP;

    viewport->DrawDataBuilder.Layers[0] = &draw_data->CmdLists;
    viewport->DrawDataBuilder.Layers[1] = &viewport->DrawDataBuilder.LayerData1;
    viewport->DrawDataBuilder.Layers[0]->resize(0);
    viewport->DrawDataBuilder.Layers[1]->resize(0);

    draw_data->Valid = true;
    draw_data->CmdListsCount = 0;
    draw_data->TotalVtxCount = draw_data->TotalIdxCount = 0;
    draw_data->DisplayPos = viewport->Pos;
    draw_data->DisplaySize = viewport->Size;
    draw_data->FramebufferScale = io.DisplayFramebufferScale;
    draw_data->OwnerViewport = viewport;
    draw_data->Textures = &ImGui::GetPlatformIO().Textures;
}

void ImGui::PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DrawList->PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
    window->ClipRect = window->DrawList->_ClipRectStack.back();
}

void ImGui::PopClipRect()
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DrawList->PopClipRect();
    window->ClipRect = window->DrawList->_ClipRectStack.back();
}

static void ImGui::RenderDimmedBackgroundBehindWindow(ImGuiWindow* window, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    ImGuiViewportP* viewport = (ImGuiViewportP*)GetMainViewport();
    ImRect viewport_rect = viewport->GetMainRect();

    {

        ImDrawList* draw_list = window->RootWindow->DrawList;
        if (draw_list->CmdBuffer.Size == 0)
            draw_list->AddDrawCmd();
        draw_list->PushClipRect(viewport_rect.Min - ImVec2(1, 1), viewport_rect.Max + ImVec2(1, 1), false);
        draw_list->AddRectFilled(viewport_rect.Min, viewport_rect.Max, col);
        ImDrawCmd cmd = draw_list->CmdBuffer.back();
        IM_ASSERT(cmd.ElemCount == 6);
        draw_list->CmdBuffer.pop_back();
        draw_list->CmdBuffer.push_front(cmd);
        draw_list->AddDrawCmd();
        draw_list->PopClipRect();
    }
}

ImGuiWindow* ImGui::FindBottomMostVisibleWindowWithinBeginStack(ImGuiWindow* parent_window)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* bottom_most_visible_window = parent_window;
    for (int i = FindWindowDisplayIndex(parent_window); i >= 0; i--)
    {
        ImGuiWindow* window = g.Windows[i];
        if (window->Flags & ImGuiWindowFlags_ChildWindow)
            continue;
        if (!IsWindowWithinBeginStackOf(window, parent_window))
            break;
        if (IsWindowActiveAndVisible(window) && GetWindowDisplayLayer(window) <= GetWindowDisplayLayer(parent_window))
            bottom_most_visible_window = window;
    }
    return bottom_most_visible_window;
}

static void ImGui::RenderDimmedBackgrounds()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* modal_window = GetTopMostAndVisiblePopupModal();
    if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
        return;
    const bool dim_bg_for_modal = (modal_window != NULL);
    const bool dim_bg_for_window_list = (g.NavWindowingTargetAnim != NULL && g.NavWindowingTargetAnim->Active);
    if (!dim_bg_for_modal && !dim_bg_for_window_list)
        return;

    if (dim_bg_for_modal)
    {

        ImGuiWindow* dim_behind_window = FindBottomMostVisibleWindowWithinBeginStack(modal_window);
        RenderDimmedBackgroundBehindWindow(dim_behind_window, GetColorU32(modal_window->DC.ModalDimBgColor, g.DimBgRatio));
    }
    else if (dim_bg_for_window_list)
    {

        RenderDimmedBackgroundBehindWindow(g.NavWindowingTargetAnim, GetColorU32(ImGuiCol_NavWindowingDimBg, g.DimBgRatio));

        ImGuiWindow* window = g.NavWindowingTargetAnim;
        ImGuiViewport* viewport = GetMainViewport();
        float distance = g.FontSize;
        ImRect bb = window->Rect();
        bb.Expand(distance);
        if (bb.GetWidth() >= viewport->Size.x && bb.GetHeight() >= viewport->Size.y)
            bb.Expand(-distance - 1.0f);
        if (window->DrawList->CmdBuffer.Size == 0)
            window->DrawList->AddDrawCmd();
        window->DrawList->PushClipRect(viewport->Pos, viewport->Pos + viewport->Size);
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_NavWindowingHighlight, g.NavWindowingHighlightAlpha), window->WindowRounding, 0, 3.0f);
        window->DrawList->PopClipRect();
    }
}

void ImGui::EndFrame()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);

    if (g.FrameCountEnded == g.FrameCount)
        return;
    IM_ASSERT_USER_ERROR_RET(g.WithinFrameScope, "Forgot to call ImGui::NewFrame()?");

    CallContextHooks(&g, ImGuiContextHookType_EndFramePre);

    if (g.IO.ConfigErrorRecovery)
        ErrorRecoveryTryToRecoverState(&g.StackSizesInNewFrame);
    ErrorCheckEndFrameSanityChecks();
    ErrorCheckEndFrameFinalizeErrorTooltip();

    ImGuiPlatformImeData* ime_data = &g.PlatformImeData;
    if (g.PlatformIO.Platform_SetImeDataFn != NULL && memcmp(ime_data, &g.PlatformImeDataPrev, sizeof(ImGuiPlatformImeData)) != 0)
    {
        IM_ASSERT(ime_data->ViewportId == IMGUI_VIEWPORT_DEFAULT_ID);
        ImGuiViewport* viewport = GetMainViewport();
        IMGUI_DEBUG_LOG_IO("[io] Calling Platform_SetImeDataFn(): WantVisible: %d, InputPos (%.2f,%.2f) for Viewport 0x%08X\n", ime_data->WantVisible, ime_data->InputPos.x, ime_data->InputPos.y, viewport->ID);
        g.PlatformIO.Platform_SetImeDataFn(&g, viewport, ime_data);
    }
    g.WantTextInputNextFrame = ime_data->WantTextInput ? 1 : 0;

    g.WithinFrameScopeWithImplicitWindow = false;
    if (g.CurrentWindow && g.CurrentWindow->IsFallbackWindow && g.CurrentWindow->WriteAccessed == false)
    {
        g.CurrentWindow->Active = false;
        if (g.NavWindow && g.NavWindow->RootWindow == g.CurrentWindow)
            FocusWindow(NULL);
    }
    End();

    NavEndFrame();

    if (g.DragDropActive)
    {
        bool is_delivered = g.DragDropPayload.Delivery;
        bool is_elapsed = (g.DragDropSourceFrameCount + 1 < g.FrameCount) && ((g.DragDropSourceFlags & ImGuiDragDropFlags_PayloadAutoExpire) || g.DragDropMouseButton == -1 || !IsMouseDown(g.DragDropMouseButton));
        if (is_delivered || is_elapsed)
            ClearDragDrop();
    }

    if (g.DragDropActive && g.DragDropSourceFrameCount + 1 < g.FrameCount && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
    {
        g.DragDropWithinSource = true;
        SetTooltip("...");
        g.DragDropWithinSource = false;
    }

    g.WithinFrameScope = false;
    g.FrameCountEnded = g.FrameCount;
    UpdateFontsEndFrame();

    UpdateMouseMovingWindowEndFrame();

    g.WindowsTempSortBuffer.resize(0);
    g.WindowsTempSortBuffer.reserve(g.Windows.Size);
    for (ImGuiWindow* window : g.Windows)
    {
        if (window->Active && (window->Flags & ImGuiWindowFlags_ChildWindow))
            continue;
        AddWindowToSortBuffer(&g.WindowsTempSortBuffer, window);
    }

    IM_ASSERT(g.Windows.Size == g.WindowsTempSortBuffer.Size);
    g.Windows.swap(g.WindowsTempSortBuffer);
    g.IO.MetricsActiveWindows = g.WindowsActiveCount;

    UpdateTexturesEndFrame();

    for (ImFontAtlas* atlas : g.FontAtlases)
        atlas->Locked = false;

    g.IO.MousePosPrev = g.IO.MousePos;
    g.IO.AppFocusLost = false;
    g.IO.MouseWheel = g.IO.MouseWheelH = 0.0f;
    g.IO.InputQueueCharacters.resize(0);

    CallContextHooks(&g, ImGuiContextHookType_EndFramePost);
}

void ImGui::Render()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);

    if (g.FrameCountEnded != g.FrameCount)
        EndFrame();
    if (g.FrameCountRendered == g.FrameCount)
        return;
    g.FrameCountRendered = g.FrameCount;

    g.IO.MetricsRenderWindows = 0;
    CallContextHooks(&g, ImGuiContextHookType_RenderPre);

    for (ImGuiViewportP* viewport : g.Viewports)
    {
        InitViewportDrawData(viewport);
        if (viewport->BgFgDrawLists[0] != NULL && viewport->BgFgDrawListsLastTimeActive[0] == (float)g.Time)
            AddDrawListToDrawDataEx(&viewport->DrawDataP, viewport->DrawDataBuilder.Layers[0], GetBackgroundDrawList(viewport));
    }

    RenderDimmedBackgrounds();

    ImGuiWindow* windows_to_render_top_most[2];
    windows_to_render_top_most[0] = (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) ? g.NavWindowingTarget->RootWindow : NULL;
    windows_to_render_top_most[1] = (g.NavWindowingTarget ? g.NavWindowingListWindow : NULL);
    for (ImGuiWindow* window : g.Windows)
    {
        IM_MSVC_WARNING_SUPPRESS(6011);
        if (IsWindowActiveAndVisible(window) && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0 && window != windows_to_render_top_most[0] && window != windows_to_render_top_most[1])
            AddRootWindowToDrawData(window);
    }
    for (int n = 0; n < IM_COUNTOF(windows_to_render_top_most); n++)
        if (windows_to_render_top_most[n] && IsWindowActiveAndVisible(windows_to_render_top_most[n]))
            AddRootWindowToDrawData(windows_to_render_top_most[n]);

    if (g.IO.MouseDrawCursor && g.MouseCursor != ImGuiMouseCursor_None)
        RenderMouseCursor(g.IO.MousePos, g.Style.MouseCursorScale, g.MouseCursor, IM_COL32_WHITE, IM_COL32_BLACK, IM_COL32(0, 0, 0, 48));

    g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = 0;
    for (ImGuiViewportP* viewport : g.Viewports)
    {
        FlattenDrawDataIntoSingleLayer(&viewport->DrawDataBuilder);

        if (viewport->BgFgDrawLists[1] != NULL && viewport->BgFgDrawListsLastTimeActive[1] == (float)g.Time)
            AddDrawListToDrawDataEx(&viewport->DrawDataP, viewport->DrawDataBuilder.Layers[0], GetForegroundDrawList(viewport));

        ImDrawData* draw_data = &viewport->DrawDataP;
        IM_ASSERT(draw_data->CmdLists.Size == draw_data->CmdListsCount);
        for (ImDrawList* draw_list : draw_data->CmdLists)
            draw_list->_PopUnusedDrawCmd();

        g.IO.MetricsRenderVertices += draw_data->TotalVtxCount;
        g.IO.MetricsRenderIndices += draw_data->TotalIdxCount;
    }

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures)
        for (ImFontAtlas* atlas : g.FontAtlases)
            ImFontAtlasDebugLogTextureRequests(atlas);
#endif

    CallContextHooks(&g, ImGuiContextHookType_RenderPost);
}

ImVec2 ImGui::CalcTextSize(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width)
{
    ImGuiContext& g = *GImGui;

    const char* text_display_end;
    if (hide_text_after_double_hash)
        text_display_end = FindRenderedTextEnd(text, text_end);
    else
        text_display_end = text_end;

    ImFont* font = g.Font;
    const float font_size = g.FontSize;
    if (text == text_display_end)
        return ImVec2(0.0f, font_size);
    ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, wrap_width, text, text_display_end, NULL);

    text_size.x = IM_TRUNC(text_size.x + 0.99999f);

    return text_size;
}

void ImGui::FindHoveredWindowEx(const ImVec2& pos, bool find_first_and_in_any_viewport, ImGuiWindow** out_hovered_window, ImGuiWindow** out_hovered_window_under_moving_window)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* hovered_window = NULL;
    ImGuiWindow* hovered_window_under_moving_window = NULL;

    if (find_first_and_in_any_viewport == false && g.MovingWindow && !(g.MovingWindow->Flags & ImGuiWindowFlags_NoMouseInputs))
        hovered_window = g.MovingWindow;

    ImVec2 padding_regular = g.Style.TouchExtraPadding;
    ImVec2 padding_for_resize = ImMax(g.Style.TouchExtraPadding, ImVec2(g.Style.WindowBorderHoverPadding, g.Style.WindowBorderHoverPadding));
    for (int i = g.Windows.Size - 1; i >= 0; i--)
    {
        ImGuiWindow* window = g.Windows[i];
        IM_MSVC_WARNING_SUPPRESS(28182);
        if (!window->WasActive || window->Hidden)
            continue;
        if (window->Flags & ImGuiWindowFlags_NoMouseInputs)
            continue;

        ImVec2 hit_padding = (window->Flags & (ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) ? padding_regular : padding_for_resize;
        if (!window->OuterRectClipped.ContainsWithPad(pos, hit_padding))
            continue;

        if (window->HitTestHoleSize.x != 0)
        {
            ImVec2 hole_pos(window->Pos.x + (float)window->HitTestHoleOffset.x, window->Pos.y + (float)window->HitTestHoleOffset.y);
            ImVec2 hole_size((float)window->HitTestHoleSize.x, (float)window->HitTestHoleSize.y);
            if (ImRect(hole_pos, hole_pos + hole_size).Contains(pos))
                continue;
        }

        if (find_first_and_in_any_viewport)
        {
            hovered_window = window;
            break;
        }
        else
        {
            if (hovered_window == NULL)
                hovered_window = window;
            IM_MSVC_WARNING_SUPPRESS(28182);
            if (hovered_window_under_moving_window == NULL && (!g.MovingWindow || window->RootWindow != g.MovingWindow->RootWindow))
                hovered_window_under_moving_window = window;
            if (hovered_window && hovered_window_under_moving_window)
                break;
        }
    }

    *out_hovered_window = hovered_window;
    if (out_hovered_window_under_moving_window != NULL)
        *out_hovered_window_under_moving_window = hovered_window_under_moving_window;
}

bool ImGui::IsItemActive()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId)
        return g.ActiveId == g.LastItemData.ID;
    return false;
}

bool ImGui::IsItemActivated()
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId)
        if (g.ActiveId == g.LastItemData.ID && g.ActiveIdPreviousFrame != g.LastItemData.ID)
            return true;
    return false;
}

bool ImGui::IsItemDeactivated()
{
    ImGuiContext& g = *GImGui;
    if (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDeactivated)
        return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Deactivated) != 0;
    return g.DeactivatedItemData.ID == g.LastItemData.ID && g.LastItemData.ID != 0 && g.DeactivatedItemData.ElapseFrame >= g.FrameCount;
}

bool ImGui::IsItemDeactivatedAfterEdit()
{
    ImGuiContext& g = *GImGui;
    return IsItemDeactivated() && g.DeactivatedItemData.HasBeenEditedBefore;
}

bool ImGui::IsItemFocused()
{
    ImGuiContext& g = *GImGui;
    return g.NavId == g.LastItemData.ID && g.NavId != 0;
}

bool ImGui::IsItemClicked(ImGuiMouseButton mouse_button)
{
    return IsMouseClicked(mouse_button) && IsItemHovered(ImGuiHoveredFlags_None);
}

bool ImGui::IsItemToggledOpen()
{
    ImGuiContext& g = *GImGui;
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_ToggledOpen) ? true : false;
}

bool ImGui::IsItemToggledSelection()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.CurrentMultiSelect != NULL);
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_ToggledSelection) ? true : false;
}

bool ImGui::IsAnyItemHovered()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredId != 0 || g.HoveredIdPreviousFrame != 0;
}

bool ImGui::IsAnyItemActive()
{
    ImGuiContext& g = *GImGui;
    return g.ActiveId != 0;
}

bool ImGui::IsAnyItemFocused()
{
    ImGuiContext& g = *GImGui;
    return g.NavId != 0 && g.NavCursorVisible;
}

bool ImGui::IsItemVisible()
{
    ImGuiContext& g = *GImGui;
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Visible) != 0;
}

bool ImGui::IsItemEdited()
{
    ImGuiContext& g = *GImGui;
    return (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Edited) != 0;
}

void ImGui::SetNextItemAllowOverlap()
{
    ImGuiContext& g = *GImGui;
    g.NextItemData.ItemFlags |= ImGuiItemFlags_AllowOverlap;
}

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#endif

void ImGui::SetActiveIdUsingAllKeyboardKeys()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.ActiveId != 0);
    g.ActiveIdUsingNavDirMask = (1 << ImGuiDir_COUNT) - 1;
    g.ActiveIdUsingAllKeyboardKeys = true;
    NavMoveRequestCancel();
}

ImGuiID ImGui::GetItemID()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.ID;
}

ImVec2 ImGui::GetItemRectMin()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.Rect.Min;
}

ImVec2 ImGui::GetItemRectMax()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.Rect.Max;
}

ImVec2 ImGui::GetItemRectSize()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.Rect.GetSize();
}

ImGuiItemFlags ImGui::GetItemFlags()
{
    ImGuiContext& g = *GImGui;
    return g.LastItemData.ItemFlags;
}

bool ImGui::BeginChild(const char* str_id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    ImGuiID id = GetCurrentWindow()->GetID(str_id);
    return BeginChildEx(str_id, id, size_arg, child_flags, window_flags);
}

bool ImGui::BeginChild(ImGuiID id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    return BeginChildEx(NULL, id, size_arg, child_flags, window_flags);
}

bool ImGui::BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;
    IM_ASSERT(id != 0);

    const ImGuiChildFlags ImGuiChildFlags_SupportedMask_ = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_FrameStyle | ImGuiChildFlags_NavFlattened;
    IM_UNUSED(ImGuiChildFlags_SupportedMask_);
    IM_ASSERT((child_flags & ~ImGuiChildFlags_SupportedMask_) == 0 && "Illegal ImGuiChildFlags value. Did you pass ImGuiWindowFlags values instead of ImGuiChildFlags?");
    IM_ASSERT((window_flags & ImGuiWindowFlags_AlwaysAutoResize) == 0 && "Cannot specify ImGuiWindowFlags_AlwaysAutoResize for BeginChild(). Use ImGuiChildFlags_AlwaysAutoResize!");
    if (child_flags & ImGuiChildFlags_AlwaysAutoResize)
    {
        IM_ASSERT((child_flags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0 && "Cannot use ImGuiChildFlags_ResizeX or ImGuiChildFlags_ResizeY with ImGuiChildFlags_AlwaysAutoResize!");
        IM_ASSERT((child_flags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY)) != 0 && "Must use ImGuiChildFlags_AutoResizeX or ImGuiChildFlags_AutoResizeY with ImGuiChildFlags_AlwaysAutoResize!");
    }
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#endif
    if (child_flags & ImGuiChildFlags_AutoResizeX)
        child_flags &= ~ImGuiChildFlags_ResizeX;
    if (child_flags & ImGuiChildFlags_AutoResizeY)
        child_flags &= ~ImGuiChildFlags_ResizeY;

    window_flags |= ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_NoTitleBar;
    window_flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);
    if (child_flags & (ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize))
        window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
    if ((child_flags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0)
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    if (child_flags & ImGuiChildFlags_FrameStyle)
    {
        PushStyleColor(ImGuiCol_ChildBg, g.Style.Colors[ImGuiCol_FrameBg]);
        PushStyleVar(ImGuiStyleVar_ChildRounding, g.Style.FrameRounding);
        PushStyleVar(ImGuiStyleVar_ChildBorderSize, g.Style.FrameBorderSize);
        PushStyleVar(ImGuiStyleVar_WindowPadding, g.Style.FramePadding);
        child_flags |= ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
        window_flags |= ImGuiWindowFlags_NoMove;
    }

    const ImVec2 size_avail = GetContentRegionAvail();
    const ImVec2 size_default((child_flags & ImGuiChildFlags_AutoResizeX) ? 0.0f : size_avail.x, (child_flags & ImGuiChildFlags_AutoResizeY) ? 0.0f : size_avail.y);
    ImVec2 size = CalcItemSize(size_arg, size_default.x, size_default.y);

    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) != 0 && (g.NextWindowData.SizeCond & ImGuiCond_Always) != 0)
    {
        if (g.NextWindowData.SizeVal.x > 0.0f)
        {
            size.x = g.NextWindowData.SizeVal.x;
            child_flags &= ~ImGuiChildFlags_ResizeX;
        }
        if (g.NextWindowData.SizeVal.y > 0.0f)
        {
            size.y = g.NextWindowData.SizeVal.y;
            child_flags &= ~ImGuiChildFlags_ResizeY;
        }
    }
    SetNextWindowSize(size);

    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasChildFlags)
        g.NextWindowData.ChildFlags |= child_flags;
    else
        g.NextWindowData.ChildFlags = child_flags;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasChildFlags;

    const char* temp_window_name;

    if (name)
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
    else
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

    const float backup_border_size = g.Style.ChildBorderSize;
    if ((child_flags & ImGuiChildFlags_Borders) == 0)
        g.Style.ChildBorderSize = 0.0f;

    const bool ret = Begin(temp_window_name, NULL, window_flags);

    g.Style.ChildBorderSize = backup_border_size;
    if (child_flags & ImGuiChildFlags_FrameStyle)
    {
        PopStyleVar(3);
        PopStyleColor();
    }

    ImGuiWindow* child_window = g.CurrentWindow;
    child_window->ChildId = id;

    if (child_window->BeginCount == 1)
        parent_window->DC.CursorPos = child_window->Pos;

    const ImGuiID temp_id_for_activation = ImHashStr("##Child", 0, id);
    if (g.ActiveId == temp_id_for_activation)
        ClearActiveID();
    if (g.NavActivateId == id && !(child_flags & ImGuiChildFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY))
    {
        FocusWindow(child_window);
        NavInitWindow(child_window, false);
        SetActiveID(temp_id_for_activation, child_window);
        g.ActiveIdSource = g.NavInputSource;
    }
    return ret;
}

void ImGui::EndChild()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* child_window = g.CurrentWindow;

    const ImGuiID backup_within_end_child_id = g.WithinEndChildID;
    IM_ASSERT(child_window->Flags & ImGuiWindowFlags_ChildWindow);

    g.WithinEndChildID = child_window->ID;
    ImVec2 child_size = child_window->Size;
    End();
    if (child_window->BeginCount == 1)
    {
        ImGuiWindow* parent_window = g.CurrentWindow;
        ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + child_size);
        ItemSize(child_size);
        const bool nav_flattened = (child_window->ChildFlags & ImGuiChildFlags_NavFlattened) != 0;
        if ((child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY) && !nav_flattened)
        {
            ItemAdd(bb, child_window->ChildId);
            RenderNavCursor(bb, child_window->ChildId);

            if (child_window->DC.NavLayersActiveMask == 0 && child_window == g.NavWindow)
                RenderNavCursor(ImRect(bb.Min - ImVec2(2, 2), bb.Max + ImVec2(2, 2)), g.NavId, ImGuiNavRenderCursorFlags_Compact);
        }
        else
        {

            ItemAdd(bb, child_window->ChildId, NULL, ImGuiItemFlags_NoNav);

            if (nav_flattened)
                parent_window->DC.NavLayersActiveMaskNext |= child_window->DC.NavLayersActiveMaskNext;
        }
        if (g.HoveredWindow == child_window)
            g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
        child_window->DC.ChildItemStatusFlags = g.LastItemData.StatusFlags;

    }
    else
    {
        SetLastItemDataForChildWindowItem(child_window, child_window->Rect());
    }

    g.WithinEndChildID = backup_within_end_child_id;
    g.LogLinePosY = -FLT_MAX;
}

static void SetWindowConditionAllowFlags(ImGuiWindow* window, ImGuiCond flags, bool enabled)
{
    window->SetWindowPosAllowFlags       = enabled ? (window->SetWindowPosAllowFlags       | flags) : (window->SetWindowPosAllowFlags       & ~flags);
    window->SetWindowSizeAllowFlags      = enabled ? (window->SetWindowSizeAllowFlags      | flags) : (window->SetWindowSizeAllowFlags      & ~flags);
    window->SetWindowCollapsedAllowFlags = enabled ? (window->SetWindowCollapsedAllowFlags | flags) : (window->SetWindowCollapsedAllowFlags & ~flags);
}

ImGuiWindow* ImGui::FindWindowByID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    return (ImGuiWindow*)g.WindowsById.GetVoidPtr(id);
}

ImGuiWindow* ImGui::FindWindowByName(const char* name)
{
    ImGuiID id = ImHashStr(name);
    return FindWindowByID(id);
}

static void ApplyWindowSettings(ImGuiWindow* window, ImGuiWindowSettings* settings)
{
    window->Pos = ImTrunc(ImVec2(settings->Pos.x, settings->Pos.y));
    if (settings->Size.x > 0 && settings->Size.y > 0)
        window->Size = window->SizeFull = ImTrunc(ImVec2(settings->Size.x, settings->Size.y));
    window->Collapsed = settings->Collapsed;
}

static void InitOrLoadWindowSettings(ImGuiWindow* window, ImGuiWindowSettings* settings)
{

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    window->Pos = main_viewport->Pos + ImVec2(60, 60);
    window->Size = window->SizeFull = ImVec2(0, 0);
    window->SetWindowPosAllowFlags = window->SetWindowSizeAllowFlags = window->SetWindowCollapsedAllowFlags = ImGuiCond_Always | ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing;

    if (settings != NULL)
    {
        SetWindowConditionAllowFlags(window, ImGuiCond_FirstUseEver, false);
        ApplyWindowSettings(window, settings);
    }
    window->DC.CursorStartPos = window->DC.CursorMaxPos = window->DC.IdealMaxPos = window->Pos;

    if ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) != 0)
    {
        window->AutoFitFramesX = window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = false;
    }
    else
    {
        if (window->Size.x <= 0.0f)
            window->AutoFitFramesX = 2;
        if (window->Size.y <= 0.0f)
            window->AutoFitFramesY = 2;
        window->AutoFitOnlyGrows = (window->AutoFitFramesX > 0) || (window->AutoFitFramesY > 0);
    }
}

static ImGuiWindow* CreateNewWindow(const char* name, ImGuiWindowFlags flags)
{

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = IM_NEW(ImGuiWindow)(&g, name);
    window->Flags = flags;
    g.WindowsById.SetVoidPtr(window->ID, window);

    ImGuiWindowSettings* settings = NULL;
    if (!(flags & ImGuiWindowFlags_NoSavedSettings))
        if ((settings = ImGui::FindWindowSettingsByWindow(window)) != 0)
            window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);

    InitOrLoadWindowSettings(window, settings);

    if (flags & ImGuiWindowFlags_NoBringToFrontOnFocus)
        g.Windows.push_front(window);
    else
        g.Windows.push_back(window);

    return window;
}

static inline ImVec2 CalcWindowMinSize(ImGuiWindow* window)
{

    ImGuiContext& g = *GImGui;
    ImVec2 size_min;
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) && !(window->Flags & ImGuiWindowFlags_Popup))
    {
        size_min.x = (window->ChildFlags & ImGuiChildFlags_ResizeX) ? g.Style.WindowMinSize.x : IMGUI_WINDOW_HARD_MIN_SIZE;
        size_min.y = (window->ChildFlags & ImGuiChildFlags_ResizeY) ? g.Style.WindowMinSize.y : IMGUI_WINDOW_HARD_MIN_SIZE;
    }
    else
    {
        size_min.x = ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) == 0) ? g.Style.WindowMinSize.x : IMGUI_WINDOW_HARD_MIN_SIZE;
        size_min.y = ((window->Flags & ImGuiWindowFlags_AlwaysAutoResize) == 0) ? g.Style.WindowMinSize.y : IMGUI_WINDOW_HARD_MIN_SIZE;
    }

    ImGuiWindow* window_for_height = window;
    size_min.y = ImMax(size_min.y, window_for_height->TitleBarHeight + window_for_height->MenuBarHeight + ImMax(0.0f, g.Style.WindowRounding - 1.0f));
    return size_min;
}

static ImVec2 CalcWindowSizeAfterConstraint(ImGuiWindow* window, const ImVec2& size_desired)
{
    ImGuiContext& g = *GImGui;
    ImVec2 new_size = size_desired;
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSizeConstraint)
    {

        ImRect cr = g.NextWindowData.SizeConstraintRect;
        new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? ImClamp(new_size.x, cr.Min.x, cr.Max.x) : window->SizeFull.x;
        new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? ImClamp(new_size.y, cr.Min.y, cr.Max.y) : window->SizeFull.y;
        if (g.NextWindowData.SizeCallback)
        {
            ImGuiSizeCallbackData data;
            data.UserData = g.NextWindowData.SizeCallbackUserData;
            data.Pos = window->Pos;
            data.CurrentSize = window->SizeFull;
            data.DesiredSize = new_size;
            g.NextWindowData.SizeCallback(&data);
            new_size = data.DesiredSize;
        }
        new_size.x = IM_TRUNC(new_size.x);
        new_size.y = IM_TRUNC(new_size.y);
    }

    ImVec2 size_min = CalcWindowMinSize(window);
    return ImMax(new_size, size_min);
}

static void CalcWindowContentSizes(ImGuiWindow* window, ImVec2* content_size_current, ImVec2* content_size_ideal)
{
    bool preserve_old_content_sizes = false;
    if (window->Collapsed && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
        preserve_old_content_sizes = true;
    else if (window->Hidden && window->HiddenFramesCannotSkipItems == 0 && window->HiddenFramesCanSkipItems > 0)
        preserve_old_content_sizes = true;
    if (preserve_old_content_sizes)
    {
        *content_size_current = window->ContentSize;
        *content_size_ideal = window->ContentSizeIdeal;
        return;
    }

    content_size_current->x = (window->ContentSizeExplicit.x != 0.0f) ? window->ContentSizeExplicit.x : ImTrunc64(window->DC.CursorMaxPos.x - window->DC.CursorStartPos.x);
    content_size_current->y = (window->ContentSizeExplicit.y != 0.0f) ? window->ContentSizeExplicit.y : ImTrunc64(window->DC.CursorMaxPos.y - window->DC.CursorStartPos.y);
    content_size_ideal->x = (window->ContentSizeExplicit.x != 0.0f) ? window->ContentSizeExplicit.x : ImTrunc64(ImMax(window->DC.CursorMaxPos.x, window->DC.IdealMaxPos.x) - window->DC.CursorStartPos.x);
    content_size_ideal->y = (window->ContentSizeExplicit.y != 0.0f) ? window->ContentSizeExplicit.y : ImTrunc64(ImMax(window->DC.CursorMaxPos.y, window->DC.IdealMaxPos.y) - window->DC.CursorStartPos.y);
}

static ImVec2 CalcWindowAutoFitSize(ImGuiWindow* window, const ImVec2& size_contents, int axis_mask)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    const float decoration_w_without_scrollbars = window->DecoOuterSizeX1 + window->DecoOuterSizeX2 - window->ScrollbarSizes.x;
    const float decoration_h_without_scrollbars = window->DecoOuterSizeY1 + window->DecoOuterSizeY2 - window->ScrollbarSizes.y;
    ImVec2 size_pad = window->WindowPadding * 2.0f;
    ImVec2 size_desired;
    size_desired[ImGuiAxis_X] = (axis_mask & 1) ? size_contents.x + size_pad.x + decoration_w_without_scrollbars : window->Size.x;
    size_desired[ImGuiAxis_Y] = (axis_mask & 2) ? size_contents.y + size_pad.y + decoration_h_without_scrollbars : window->Size.y;

    ImVec2 size_max = ((window->Flags & ImGuiWindowFlags_ChildWindow) && !(window->Flags & ImGuiWindowFlags_Popup)) ? ImVec2(FLT_MAX, FLT_MAX) : ImGui::GetMainViewport()->WorkSize - style.DisplaySafeAreaPadding * 2.0f;

    if (window->Flags & ImGuiWindowFlags_Tooltip)
    {

        return ImMin(size_desired, size_max);
    }
    else
    {
        ImVec2 size_min = CalcWindowMinSize(window);
        ImVec2 size_auto_fit = ImClamp(size_desired, ImMin(size_min, size_max), size_max);

        ImVec2 size_auto_fit_after_constraint = CalcWindowSizeAfterConstraint(window, size_auto_fit);
        bool will_have_scrollbar_x = (size_auto_fit_after_constraint.x - size_pad.x - decoration_w_without_scrollbars < size_contents.x && !(window->Flags & ImGuiWindowFlags_NoScrollbar) && (window->Flags & ImGuiWindowFlags_HorizontalScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        bool will_have_scrollbar_y = (size_auto_fit_after_constraint.y - size_pad.y - decoration_h_without_scrollbars < size_contents.y && !(window->Flags & ImGuiWindowFlags_NoScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysVerticalScrollbar);
        if (will_have_scrollbar_x)
            size_auto_fit.y += style.ScrollbarSize;
        if (will_have_scrollbar_y)
            size_auto_fit.x += style.ScrollbarSize;
        return size_auto_fit;
    }
}

ImVec2 ImGui::CalcWindowNextAutoFitSize(ImGuiWindow* window)
{
    ImVec2 size_contents_current;
    ImVec2 size_contents_ideal;
    CalcWindowContentSizes(window, &size_contents_current, &size_contents_ideal);
    ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, size_contents_ideal, ~0);
    ImVec2 size_final = CalcWindowSizeAfterConstraint(window, size_auto_fit);
    return size_final;
}

static ImGuiCol GetWindowBgColorIdx(ImGuiWindow* window)
{
    if (window->Flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup))
        return ImGuiCol_PopupBg;
    if (window->Flags & ImGuiWindowFlags_ChildWindow)
        return ImGuiCol_ChildBg;
    return ImGuiCol_WindowBg;
}

static void CalcResizePosSizeFromAnyCorner(ImGuiWindow* window, const ImVec2& corner_target_arg, const ImVec2& corner_norm, ImVec2* out_pos, ImVec2* out_size)
{
    ImVec2 corner_target = corner_target_arg;
    if (window->Flags & ImGuiWindowFlags_ChildWindow)
    {
        ImGuiWindow* parent_window = window->ParentWindow;
        ImGuiWindowFlags parent_flags = parent_window->Flags;
        ImRect limit_rect = parent_window->InnerRect;
        limit_rect.Expand(ImVec2(-ImMax(parent_window->WindowPadding.x, parent_window->WindowBorderSize), -ImMax(parent_window->WindowPadding.y, parent_window->WindowBorderSize)));
        if ((parent_flags & (ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar)) == 0 || (parent_flags & ImGuiWindowFlags_NoScrollbar))
            corner_target.x = ImClamp(corner_target.x, limit_rect.Min.x, limit_rect.Max.x);
        if (parent_flags & ImGuiWindowFlags_NoScrollbar)
            corner_target.y = ImClamp(corner_target.y, limit_rect.Min.y, limit_rect.Max.y);
    }
    ImVec2 pos_min = ImLerp(corner_target, window->Pos, corner_norm);
    ImVec2 pos_max = ImLerp(window->Pos + window->Size, corner_target, corner_norm);
    ImVec2 size_expected = pos_max - pos_min;
    ImVec2 size_constrained = CalcWindowSizeAfterConstraint(window, size_expected);
    *out_pos = pos_min;
    if (corner_norm.x == 0.0f)
        out_pos->x -= (size_constrained.x - size_expected.x);
    if (corner_norm.y == 0.0f)
        out_pos->y -= (size_constrained.y - size_expected.y);
    *out_size = size_constrained;
}

struct ImGuiResizeGripDef
{
    ImVec2  CornerPosN;
    ImVec2  InnerDir;
    int     AngleMin12, AngleMax12;
};
static const ImGuiResizeGripDef resize_grip_def[4] =
{
    { ImVec2(1, 1), ImVec2(-1, -1), 0, 3 },
    { ImVec2(0, 1), ImVec2(+1, -1), 3, 6 },
    { ImVec2(0, 0), ImVec2(+1, +1), 6, 9 },
    { ImVec2(1, 0), ImVec2(-1, +1), 9, 12 }
};

struct ImGuiResizeBorderDef
{
    ImVec2  InnerDir;
    ImVec2  SegmentN1, SegmentN2;
    float   OuterAngle;
};
static const ImGuiResizeBorderDef resize_border_def[4] =
{
    { ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f },
    { ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f },
    { ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f },
    { ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }
};

static ImRect GetResizeBorderRect(ImGuiWindow* window, int border_n, float perp_padding, float thickness)
{
    ImRect rect = window->Rect();
    if (thickness == 0.0f)
        rect.Max -= ImVec2(1, 1);
    if (border_n == ImGuiDir_Left)  { return ImRect(rect.Min.x - thickness,    rect.Min.y + perp_padding, rect.Min.x + thickness,    rect.Max.y - perp_padding); }
    if (border_n == ImGuiDir_Right) { return ImRect(rect.Max.x - thickness,    rect.Min.y + perp_padding, rect.Max.x + thickness,    rect.Max.y - perp_padding); }
    if (border_n == ImGuiDir_Up)    { return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness,    rect.Max.x - perp_padding, rect.Min.y + thickness);    }
    if (border_n == ImGuiDir_Down)  { return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness,    rect.Max.x - perp_padding, rect.Max.y + thickness);    }
    IM_ASSERT(0);
    return ImRect();
}

ImGuiID ImGui::GetWindowResizeCornerID(ImGuiWindow* window, int n)
{
    IM_ASSERT(n >= 0 && n < 4);
    ImGuiID id = window->ID;
    id = ImHashStr("#RESIZE", 0, id);
    id = ImHashData(&n, sizeof(int), id);
    return id;
}

ImGuiID ImGui::GetWindowResizeBorderID(ImGuiWindow* window, ImGuiDir dir)
{
    IM_ASSERT(dir >= 0 && dir < 4);
    int n = (int)dir + 4;
    ImGuiID id = window->ID;
    id = ImHashStr("#RESIZE", 0, id);
    id = ImHashData(&n, sizeof(int), id);
    return id;
}

static int ImGui::UpdateWindowManualResize(ImGuiWindow* window, int* border_hovered, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4], const ImRect& visibility_rect)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindowFlags flags = window->Flags;

    if ((flags & ImGuiWindowFlags_NoResize) || window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
        return false;
    if ((flags & ImGuiWindowFlags_AlwaysAutoResize) && (window->ChildFlags & (ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) == 0)
        return false;
    if (window->WasActive == false)
        return false;

    int ret_auto_fit_mask = 0x00;
    const float grip_draw_size = IM_TRUNC(ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
    const float grip_hover_inner_size = (resize_grip_count > 0) ? IM_TRUNC(grip_draw_size * 0.75f) : 0.0f;
    const float grip_hover_outer_size = g.WindowsBorderHoverPadding;

    ImRect clamp_rect = visibility_rect;
    const bool window_move_from_title_bar = !(window->BgClickFlags & ImGuiWindowBgClickFlags_Move) && !(window->Flags & ImGuiWindowFlags_NoTitleBar);
    if (window_move_from_title_bar)
        clamp_rect.Min.y -= window->TitleBarHeight;

    ImVec2 pos_target(FLT_MAX, FLT_MAX);
    ImVec2 size_target(FLT_MAX, FLT_MAX);

    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    PushID("#RESIZE");
    for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
    {
        const ImGuiResizeGripDef& def = resize_grip_def[resize_grip_n];
        const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, def.CornerPosN);

        bool hovered, held;
        ImRect resize_rect(corner - def.InnerDir * grip_hover_outer_size, corner + def.InnerDir * grip_hover_inner_size);
        if (resize_rect.Min.x > resize_rect.Max.x) ImSwap(resize_rect.Min.x, resize_rect.Max.x);
        if (resize_rect.Min.y > resize_rect.Max.y) ImSwap(resize_rect.Min.y, resize_rect.Max.y);
        ImGuiID resize_grip_id = window->GetID(resize_grip_n);
        ItemAdd(resize_rect, resize_grip_id, NULL, ImGuiItemFlags_NoNav);
        ButtonBehavior(resize_rect, resize_grip_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);

        if (hovered || held)
            SetMouseCursor((resize_grip_n & 1) ? ImGuiMouseCursor_ResizeNESW : ImGuiMouseCursor_ResizeNWSE);

        if (held && g.IO.MouseDoubleClicked[0])
        {

            ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal, ~0);
            size_target = CalcWindowSizeAfterConstraint(window, size_auto_fit);
            ret_auto_fit_mask = 0x03;
            ClearActiveID();
        }
        else if (held)
        {

            ImVec2 clamp_min = ImVec2(def.CornerPosN.x == 1.0f ? clamp_rect.Min.x : -FLT_MAX, (def.CornerPosN.y == 1.0f || (def.CornerPosN.y == 0.0f && window_move_from_title_bar)) ? clamp_rect.Min.y : -FLT_MAX);
            ImVec2 clamp_max = ImVec2(def.CornerPosN.x == 0.0f ? clamp_rect.Max.x : +FLT_MAX, def.CornerPosN.y == 0.0f ? clamp_rect.Max.y : +FLT_MAX);
            ImVec2 corner_target = g.IO.MousePos - g.ActiveIdClickOffset + ImLerp(def.InnerDir * grip_hover_outer_size, def.InnerDir * -grip_hover_inner_size, def.CornerPosN);
            corner_target = ImClamp(corner_target, clamp_min, clamp_max);
            CalcResizePosSizeFromAnyCorner(window, corner_target, def.CornerPosN, &pos_target, &size_target);
        }

        const bool resize_grip_visible = held || hovered || (resize_grip_n == 0 && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0);
        if (resize_grip_visible)
            resize_grip_col[resize_grip_n] = GetColorU32(held ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip);
    }

    int resize_border_mask = 0x00;
    if (window->Flags & ImGuiWindowFlags_ChildWindow)
        resize_border_mask |= ((window->ChildFlags & ImGuiChildFlags_ResizeX) ? 0x02 : 0) | ((window->ChildFlags & ImGuiChildFlags_ResizeY) ? 0x08 : 0);
    else
        resize_border_mask = g.IO.ConfigWindowsResizeFromEdges ? 0x0F : 0x00;
    for (int border_n = 0; border_n < 4; border_n++)
    {
        if ((resize_border_mask & (1 << border_n)) == 0)
            continue;
        const ImGuiResizeBorderDef& def = resize_border_def[border_n];
        const ImGuiAxis axis = (border_n == ImGuiDir_Left || border_n == ImGuiDir_Right) ? ImGuiAxis_X : ImGuiAxis_Y;

        bool hovered, held;
        ImRect border_rect = GetResizeBorderRect(window, border_n, grip_hover_inner_size, g.WindowsBorderHoverPadding);
        ImGuiID border_id = window->GetID(border_n + 4);
        ItemAdd(border_rect, border_id, NULL, ImGuiItemFlags_NoNav);
        ButtonBehavior(border_rect, border_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);

        if (hovered && g.HoveredIdTimer <= WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER)
            hovered = false;
        if (hovered || held)
            SetMouseCursor((axis == ImGuiAxis_X) ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
        if (held && g.IO.MouseDoubleClicked[0])
        {

            if (border_n == 1 || border_n == 3)
            {
                ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal, 1 << axis);
                size_target[axis] = CalcWindowSizeAfterConstraint(window, size_auto_fit)[axis];
                ret_auto_fit_mask |= (1 << axis);
                hovered = held = false;
            }
            ClearActiveID();
        }
        else if (held)
        {

            const bool just_scrolled_manually_while_resizing = (g.WheelingWindow != NULL && g.WheelingWindowScrolledFrame == g.FrameCount && IsWindowChildOf(window, g.WheelingWindow, false));
            if (g.ActiveIdIsJustActivated || just_scrolled_manually_while_resizing)
            {
                g.WindowResizeBorderExpectedRect = border_rect;
                g.WindowResizeRelativeMode = false;
            }
            if ((window->Flags & ImGuiWindowFlags_ChildWindow) && memcmp(&g.WindowResizeBorderExpectedRect, &border_rect, sizeof(ImRect)) != 0)
                g.WindowResizeRelativeMode = true;

            const ImVec2 border_curr = (window->Pos + ImMin(def.SegmentN1, def.SegmentN2) * window->Size);
            const float border_target_rel_mode_for_axis = border_curr[axis] + g.IO.MouseDelta[axis];
            const float border_target_abs_mode_for_axis = g.IO.MousePos[axis] - g.ActiveIdClickOffset[axis] + g.WindowsBorderHoverPadding;

            ImVec2 border_target = window->Pos;
            border_target[axis] = border_target_abs_mode_for_axis;

            bool ignore_resize = false;
            if (g.WindowResizeRelativeMode)
            {

                border_target[axis] = border_target_rel_mode_for_axis;
                if (g.IO.MouseDelta[axis] == 0.0f || (g.IO.MouseDelta[axis] > 0.0f) == (border_target_rel_mode_for_axis > border_target_abs_mode_for_axis))
                    ignore_resize = true;
            }

            ImVec2 clamp_min(border_n == ImGuiDir_Right ? clamp_rect.Min.x : -FLT_MAX, border_n == ImGuiDir_Down || (border_n == ImGuiDir_Up && window_move_from_title_bar) ? clamp_rect.Min.y : -FLT_MAX);
            ImVec2 clamp_max(border_n == ImGuiDir_Left ? clamp_rect.Max.x : +FLT_MAX, border_n == ImGuiDir_Up ? clamp_rect.Max.y : +FLT_MAX);
            border_target = ImClamp(border_target, clamp_min, clamp_max);
            if (!ignore_resize)
                CalcResizePosSizeFromAnyCorner(window, border_target, ImMin(def.SegmentN1, def.SegmentN2), &pos_target, &size_target);
        }
        if (hovered)
            *border_hovered = border_n;
        if (held)
            *border_held = border_n;
    }
    PopID();

    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;

    if (g.NavWindowingTarget && g.NavWindowingTarget->RootWindow == window)
    {
        ImVec2 nav_resize_dir;
        if (g.NavInputSource == ImGuiInputSource_Keyboard && g.IO.KeyShift)
            nav_resize_dir = GetKeyMagnitude2d(ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow);
        if (g.NavInputSource == ImGuiInputSource_Gamepad)
            nav_resize_dir = GetKeyMagnitude2d(ImGuiKey_GamepadDpadLeft, ImGuiKey_GamepadDpadRight, ImGuiKey_GamepadDpadUp, ImGuiKey_GamepadDpadDown);
        if (nav_resize_dir.x != 0.0f || nav_resize_dir.y != 0.0f)
        {
            const float NAV_RESIZE_SPEED = 600.0f;
            const float resize_step = NAV_RESIZE_SPEED * g.IO.DeltaTime * GetScale();
            g.NavWindowingAccumDeltaSize += nav_resize_dir * resize_step;
            g.NavWindowingAccumDeltaSize = ImMax(g.NavWindowingAccumDeltaSize, clamp_rect.Min - window->Pos - window->Size);
            g.NavWindowingToggleLayer = false;
            g.NavHighlightItemUnderNav = true;
            resize_grip_col[0] = GetColorU32(ImGuiCol_ResizeGripActive);
            ImVec2 accum_floored = ImTrunc(g.NavWindowingAccumDeltaSize);
            if (accum_floored.x != 0.0f || accum_floored.y != 0.0f)
            {

                size_target = CalcWindowSizeAfterConstraint(window, window->SizeFull + accum_floored);
                g.NavWindowingAccumDeltaSize -= accum_floored;
            }
        }
    }

    const ImVec2 old_pos = window->Pos;
    const ImVec2 old_size = window->SizeFull;
    if (size_target.x != FLT_MAX && (window->Size.x != size_target.x || window->SizeFull.x != size_target.x))
        window->Size.x = window->SizeFull.x = size_target.x;
    if (size_target.y != FLT_MAX && (window->Size.y != size_target.y || window->SizeFull.y != size_target.y))
        window->Size.y = window->SizeFull.y = size_target.y;
    if (pos_target.x != FLT_MAX && window->Pos.x != ImTrunc(pos_target.x))
        window->Pos.x = ImTrunc(pos_target.x);
    if (pos_target.y != FLT_MAX && window->Pos.y != ImTrunc(pos_target.y))
        window->Pos.y = ImTrunc(pos_target.y);
    if (old_pos.x != window->Pos.x || old_pos.y != window->Pos.y || old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
        MarkIniSettingsDirty(window);

    if (*border_held != -1)
        g.WindowResizeBorderExpectedRect = GetResizeBorderRect(window, *border_held, grip_hover_inner_size, g.WindowsBorderHoverPadding);

    return ret_auto_fit_mask;
}

static inline void ClampWindowPos(ImGuiWindow* window, const ImRect& visibility_rect)
{
    ImVec2 size_for_clamping = window->Size;
    if (!(window->BgClickFlags & ImGuiWindowBgClickFlags_Move) && !(window->Flags & ImGuiWindowFlags_NoTitleBar))
        size_for_clamping.y = window->TitleBarHeight;
    window->Pos = ImClamp(window->Pos, visibility_rect.Min - size_for_clamping, visibility_rect.Max);
}

static void RenderWindowOuterSingleBorder(ImGuiWindow* window, int border_n, ImU32 border_col, float border_size)
{
    const ImGuiResizeBorderDef& def = resize_border_def[border_n];
    const float rounding = window->WindowRounding;
    const ImRect border_r = GetResizeBorderRect(window, border_n, rounding, 0.0f);
    window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN1) + ImVec2(0.5f, 0.5f) + def.InnerDir * rounding, rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
    window->DrawList->PathArcTo(ImLerp(border_r.Min, border_r.Max, def.SegmentN2) + ImVec2(0.5f, 0.5f) + def.InnerDir * rounding, rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
    window->DrawList->PathStroke(border_col, ImDrawFlags_None, border_size);
}

static void ImGui::RenderWindowOuterBorders(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    const float border_size = window->WindowBorderSize;
    const ImU32 border_col = GetColorU32(ImGuiCol_Border);
    if (border_size > 0.0f && (window->Flags & ImGuiWindowFlags_NoBackground) == 0)
        window->DrawList->AddRect(window->Pos, window->Pos + window->Size, border_col, window->WindowRounding, 0, window->WindowBorderSize);
    else if (border_size > 0.0f)
    {
        if (window->ChildFlags & ImGuiChildFlags_ResizeX)
            RenderWindowOuterSingleBorder(window, 1, border_col, border_size);
        if (window->ChildFlags & ImGuiChildFlags_ResizeY)
            RenderWindowOuterSingleBorder(window, 3, border_col, border_size);
    }
    if (window->ResizeBorderHovered != -1 || window->ResizeBorderHeld != -1)
    {
        const int border_n = (window->ResizeBorderHeld != -1) ? window->ResizeBorderHeld : window->ResizeBorderHovered;
        const ImU32 border_col_resizing = GetColorU32((window->ResizeBorderHeld != -1) ? ImGuiCol_SeparatorActive : ImGuiCol_SeparatorHovered);
        RenderWindowOuterSingleBorder(window, border_n, border_col_resizing, ImMax(2.0f, window->WindowBorderSize));
    }
    if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar))
    {
        float y = window->Pos.y + window->TitleBarHeight - 1;
        window->DrawList->AddLine(ImVec2(window->Pos.x + border_size * 0.5f, y), ImVec2(window->Pos.x + window->Size.x - border_size * 0.5f, y), border_col, g.Style.FrameBorderSize);
    }
}

void ImGui::RenderWindowDecorations(ImGuiWindow* window, const ImRect& title_bar_rect, bool title_bar_is_highlight, bool handle_borders_and_resize_grips, int resize_grip_count, const ImU32 resize_grip_col[4], float resize_grip_draw_size)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    ImGuiWindowFlags flags = window->Flags;

    IM_ASSERT(window->BeginCount == 0);
    window->SkipItems = false;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    const float window_rounding = window->WindowRounding;
    const float window_border_size = window->WindowBorderSize;
    if (window->Collapsed)
    {

        const float backup_border_size = style.FrameBorderSize;
        g.Style.FrameBorderSize = window->WindowBorderSize;
        ImU32 title_bar_col = GetColorU32((title_bar_is_highlight && g.NavCursorVisible) ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBgCollapsed);
        RenderFrame(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, true, window_rounding);
        g.Style.FrameBorderSize = backup_border_size;
    }
    else
    {

        if (!(flags & ImGuiWindowFlags_NoBackground))
        {
            ImU32 bg_col = GetColorU32(GetWindowBgColorIdx(window));
            bool override_alpha = false;
            float alpha = 1.0f;
            if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasBgAlpha)
            {
                alpha = g.NextWindowData.BgAlphaVal;
                override_alpha = true;
            }
            if (override_alpha)
                bg_col = (bg_col & ~IM_COL32_A_MASK) | (IM_F32_TO_INT8_SAT(alpha) << IM_COL32_A_SHIFT);
            if (bg_col & IM_COL32_A_MASK)
            {
                ImRect bg_rect(window->Pos + ImVec2(0, window->TitleBarHeight), window->Pos + window->Size);
                ImDrawFlags bg_rounding_flags = (flags & ImGuiWindowFlags_NoTitleBar) ? 0 : ImDrawFlags_RoundCornersBottom;
                ImDrawList* bg_draw_list = window->DrawList;
                bg_draw_list->AddRectFilled(bg_rect.Min, bg_rect.Max, bg_col, window_rounding, bg_rounding_flags);
            }
        }

        if (!(flags & ImGuiWindowFlags_NoTitleBar))
        {
            ImU32 title_bar_col = GetColorU32(title_bar_is_highlight ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg);
            window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, window_rounding, ImDrawFlags_RoundCornersTop);
        }

        if (flags & ImGuiWindowFlags_MenuBar)
        {
            ImRect menu_bar_rect = window->MenuBarRect();
            menu_bar_rect.ClipWith(window->Rect());
            window->DrawList->AddRectFilled(menu_bar_rect.Min, menu_bar_rect.Max, GetColorU32(ImGuiCol_MenuBarBg), (flags & ImGuiWindowFlags_NoTitleBar) ? window_rounding : 0.0f, ImDrawFlags_RoundCornersTop);
            if (style.FrameBorderSize > 0.0f && menu_bar_rect.Max.y < window->Pos.y + window->Size.y)
                window->DrawList->AddLine(menu_bar_rect.GetBL() + ImVec2(window_border_size * 0.5f, 0.0f), menu_bar_rect.GetBR() - ImVec2(window_border_size * 0.5f, 0.0f), GetColorU32(ImGuiCol_Border), style.FrameBorderSize);
        }

        if (window->ScrollbarX)
            Scrollbar(ImGuiAxis_X);
        if (window->ScrollbarY)
            Scrollbar(ImGuiAxis_Y);

        if (handle_borders_and_resize_grips && !(flags & ImGuiWindowFlags_NoResize))
        {
            for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
            {
                const ImU32 col = resize_grip_col[resize_grip_n];
                if ((col & IM_COL32_A_MASK) == 0)
                    continue;
                const ImGuiResizeGripDef& grip = resize_grip_def[resize_grip_n];
                const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, grip.CornerPosN);
                const float border_inner = IM_ROUND(window_border_size * 0.5f);
                window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(border_inner, resize_grip_draw_size) : ImVec2(resize_grip_draw_size, border_inner)));
                window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(resize_grip_draw_size, border_inner) : ImVec2(border_inner, resize_grip_draw_size)));
                window->DrawList->PathArcToFast(ImVec2(corner.x + grip.InnerDir.x * (window_rounding + border_inner), corner.y + grip.InnerDir.y * (window_rounding + border_inner)), window_rounding, grip.AngleMin12, grip.AngleMax12);
                window->DrawList->PathFillConvex(col);
            }
        }

        if (handle_borders_and_resize_grips)
            RenderWindowOuterBorders(window);
    }
    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
}

void ImGui::RenderWindowTitleBarContents(ImGuiWindow* window, const ImRect& title_bar_rect, const char* name, bool* p_open)
{
    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    ImGuiWindowFlags flags = window->Flags;

    const bool has_close_button = (p_open != NULL);
    const bool has_collapse_button = !(flags & ImGuiWindowFlags_NoCollapse) && (style.WindowMenuButtonPosition != ImGuiDir_None);

    const ImGuiItemFlags item_flags_backup = g.CurrentItemFlags;
    g.CurrentItemFlags |= ImGuiItemFlags_NoNavDefaultFocus;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;

    float pad_l = style.FramePadding.x;
    float pad_r = style.FramePadding.x;
    float button_sz = g.FontSize;
    ImVec2 close_button_pos;
    ImVec2 collapse_button_pos;
    if (has_close_button)
    {
        close_button_pos = ImVec2(title_bar_rect.Max.x - pad_r - button_sz, title_bar_rect.Min.y + style.FramePadding.y);
        pad_r += button_sz + style.ItemInnerSpacing.x;
    }
    if (has_collapse_button && style.WindowMenuButtonPosition == ImGuiDir_Right)
    {
        collapse_button_pos = ImVec2(title_bar_rect.Max.x - pad_r - button_sz, title_bar_rect.Min.y + style.FramePadding.y);
        pad_r += button_sz + style.ItemInnerSpacing.x;
    }
    if (has_collapse_button && style.WindowMenuButtonPosition == ImGuiDir_Left)
    {
        collapse_button_pos = ImVec2(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y + style.FramePadding.y);
        pad_l += button_sz + style.ItemInnerSpacing.x;
    }

    if (has_collapse_button)
        if (CollapseButton(window->GetID("#COLLAPSE"), collapse_button_pos))
            window->WantCollapseToggle = true;

    if (has_close_button)
    {
        ImGuiItemFlags backup_item_flags = g.CurrentItemFlags;
        g.CurrentItemFlags |= ImGuiItemFlags_NoFocus;
        if (CloseButton(window->GetID("#CLOSE"), close_button_pos))
            *p_open = false;
        g.CurrentItemFlags = backup_item_flags;
    }

    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
    g.CurrentItemFlags = item_flags_backup;

    const float marker_size_x = (flags & ImGuiWindowFlags_UnsavedDocument) ? button_sz * 0.80f : 0.0f;
    const ImVec2 text_size = CalcTextSize(name, NULL, true) + ImVec2(marker_size_x, 0.0f);

    if (pad_l > style.FramePadding.x)
        pad_l += g.Style.ItemInnerSpacing.x;
    if (pad_r > style.FramePadding.x)
        pad_r += g.Style.ItemInnerSpacing.x;
    if (style.WindowTitleAlign.x > 0.0f && style.WindowTitleAlign.x < 1.0f)
    {
        float centerness = ImSaturate(1.0f - ImFabs(style.WindowTitleAlign.x - 0.5f) * 2.0f);
        float pad_extend = ImMin(ImMax(pad_l, pad_r), title_bar_rect.GetWidth() - pad_l - pad_r - text_size.x);
        pad_l = ImMax(pad_l, pad_extend * centerness);
        pad_r = ImMax(pad_r, pad_extend * centerness);
    }

    ImRect layout_r(title_bar_rect.Min.x + pad_l, title_bar_rect.Min.y, title_bar_rect.Max.x - pad_r, title_bar_rect.Max.y);
    ImRect clip_r(layout_r.Min.x, layout_r.Min.y, ImMin(layout_r.Max.x + g.Style.ItemInnerSpacing.x, title_bar_rect.Max.x), layout_r.Max.y);
    if (flags & ImGuiWindowFlags_UnsavedDocument)
    {
        ImVec2 marker_pos;
        marker_pos.x = ImClamp(layout_r.Min.x + (layout_r.GetWidth() - text_size.x) * style.WindowTitleAlign.x + text_size.x, layout_r.Min.x, layout_r.Max.x);
        marker_pos.y = (layout_r.Min.y + layout_r.Max.y) * 0.5f;
        if (marker_pos.x > layout_r.Min.x)
        {
            RenderBullet(window->DrawList, marker_pos, GetColorU32(ImGuiCol_UnsavedMarker));
            clip_r.Max.x = ImMin(clip_r.Max.x, marker_pos.x - (int)(marker_size_x * 0.5f));
        }
    }

    RenderTextClipped(layout_r.Min, layout_r.Max, name, NULL, &text_size, style.WindowTitleAlign, &clip_r);
}

void ImGui::UpdateWindowParentAndRootLinks(ImGuiWindow* window, ImGuiWindowFlags flags, ImGuiWindow* parent_window)
{
    window->ParentWindow = parent_window;
    window->RootWindow = window->RootWindowPopupTree = window->RootWindowForTitleBarHighlight = window->RootWindowForNav = window;
    if (parent_window && (flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Tooltip))
        window->RootWindow = parent_window->RootWindow;
    if (parent_window && (flags & ImGuiWindowFlags_Popup))
        window->RootWindowPopupTree = parent_window->RootWindowPopupTree;
    if (parent_window && !(flags & ImGuiWindowFlags_Modal) && (flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)))
        window->RootWindowForTitleBarHighlight = parent_window->RootWindowForTitleBarHighlight;
    while (window->RootWindowForNav->ChildFlags & ImGuiChildFlags_NavFlattened)
    {
        IM_ASSERT(window->RootWindowForNav->ParentWindow != NULL);
        window->RootWindowForNav = window->RootWindowForNav->ParentWindow;
    }
}

void ImGui::UpdateWindowSkipRefresh(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    window->SkipRefresh = false;
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasRefreshPolicy) == 0)
        return;
    if (g.NextWindowData.RefreshFlagsVal & ImGuiWindowRefreshFlags_TryToAvoidRefresh)
    {

        if (window->Appearing)
            return;
        if (window->Hidden)
            return;
        if ((g.NextWindowData.RefreshFlagsVal & ImGuiWindowRefreshFlags_RefreshOnHover) && g.HoveredWindow)
            if (window->RootWindow == g.HoveredWindow->RootWindow || IsWindowWithinBeginStackOf(g.HoveredWindow->RootWindow, window))
                return;
        if ((g.NextWindowData.RefreshFlagsVal & ImGuiWindowRefreshFlags_RefreshOnFocus) && g.NavWindow)
            if (window->RootWindow == g.NavWindow->RootWindow || IsWindowWithinBeginStackOf(g.NavWindow->RootWindow, window))
                return;
        window->DrawList = NULL;
        window->SkipRefresh = true;
    }
}

static void SetWindowActiveForSkipRefresh(ImGuiWindow* window)
{
    window->Active = true;
    for (ImGuiWindow* child : window->DC.ChildWindows)
        if (!child->Hidden)
        {
            child->Active = child->SkipRefresh = true;
            SetWindowActiveForSkipRefresh(child);
        }
}

bool ImGui::Begin(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    IM_ASSERT(name != NULL && name[0] != '\0');
    IM_ASSERT(g.WithinFrameScope);
    IM_ASSERT(g.FrameCountEnded != g.FrameCount);

    ImGuiWindow* window = FindWindowByName(name);
    const bool window_just_created = (window == NULL);
    if (window_just_created)
        window = CreateNewWindow(name, flags);

    if (g.DebugBreakInWindow == window->ID)
        IM_DEBUG_BREAK();

    if ((flags & ImGuiWindowFlags_NoInputs) == ImGuiWindowFlags_NoInputs)
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    const int current_frame = g.FrameCount;
    const bool first_begin_of_the_frame = (window->LastFrameActive != current_frame);
    window->IsFallbackWindow = (g.CurrentWindowStack.Size == 0 && g.WithinFrameScopeWithImplicitWindow);

    bool window_just_activated_by_user = (window->LastFrameActive < current_frame - 1);
    if (flags & ImGuiWindowFlags_Popup)
    {
        ImGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
        window_just_activated_by_user |= (window->PopupId != popup_ref.PopupId);
        window_just_activated_by_user |= (window != popup_ref.Window);
    }
    window->Appearing = window_just_activated_by_user;
    if (window->Appearing)
        SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, true);

    if (first_begin_of_the_frame)
    {
        UpdateWindowInFocusOrderList(window, window_just_created, flags);
        window->Flags = (ImGuiWindowFlags)flags;
        window->ChildFlags = (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasChildFlags) ? g.NextWindowData.ChildFlags : 0;
        window->LastFrameActive = current_frame;
        window->LastTimeActive = (float)g.Time;
        window->BeginOrderWithinParent = 0;
        window->BeginOrderWithinContext = (short)(g.WindowsActiveCount++);
    }
    else
    {
        flags = window->Flags;
    }

    ImGuiWindow* parent_window_in_stack = g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back().Window;
    ImGuiWindow* parent_window = first_begin_of_the_frame ? ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) ? parent_window_in_stack : NULL) : window->ParentWindow;
    IM_ASSERT(parent_window != NULL || !(flags & ImGuiWindowFlags_ChildWindow));

    if (window->IDStack.Size == 0)
        window->IDStack.push_back(window->ID);

    g.CurrentWindow = window;
    g.CurrentWindowStack.resize(g.CurrentWindowStack.Size + 1);
    ImGuiWindowStackData& window_stack_data = g.CurrentWindowStack.back();
    window_stack_data.Window = window;
    window_stack_data.ParentLastItemDataBackup = g.LastItemData;
    window_stack_data.DisabledOverrideReenable = (flags & ImGuiWindowFlags_Tooltip) && (g.CurrentItemFlags & ImGuiItemFlags_Disabled);
    window_stack_data.DisabledOverrideReenableAlphaBackup = 0.0f;
    ErrorRecoveryStoreState(&window_stack_data.StackSizesInBegin);
    g.StackSizesInBeginForCurrentWindow = &window_stack_data.StackSizesInBegin;
    if (flags & ImGuiWindowFlags_ChildMenu)
        g.BeginMenuDepth++;

    if (first_begin_of_the_frame)
    {
        UpdateWindowParentAndRootLinks(window, flags, parent_window);
        window->ParentWindowInBeginStack = parent_window_in_stack;

        window->ParentWindowForFocusRoute = (flags & ImGuiWindowFlags_ChildWindow) ? parent_window_in_stack : NULL;

        window->FontWindowScaleParents = parent_window ? parent_window->FontWindowScaleParents * parent_window->FontWindowScale : 1.0f;
    }

    PushFocusScope((window->ChildFlags & ImGuiChildFlags_NavFlattened) ? g.CurrentFocusScopeId : window->ID);
    window->NavRootFocusScopeId = g.CurrentFocusScopeId;

    if (flags & ImGuiWindowFlags_Popup)
    {
        ImGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
        popup_ref.Window = window;
        popup_ref.ParentNavLayer = parent_window_in_stack->DC.NavLayerCurrent;
        g.BeginPopupStack.push_back(popup_ref);
        window->PopupId = popup_ref.PopupId;
    }

    bool window_pos_set_by_api = false;
    bool window_size_x_set_by_api = false, window_size_y_set_by_api = false;
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasPos)
    {
        window_pos_set_by_api = (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) != 0;
        if (window_pos_set_by_api && ImLengthSqr(g.NextWindowData.PosPivotVal) > 0.00001f)
        {

            window->SetWindowPosVal = g.NextWindowData.PosVal;
            window->SetWindowPosPivot = g.NextWindowData.PosPivotVal;
            window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
        }
        else
        {
            SetWindowPos(window, g.NextWindowData.PosVal, g.NextWindowData.PosCond);
        }
    }
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize)
    {
        window_size_x_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.x > 0.0f);
        window_size_y_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.y > 0.0f);
        if ((window->ChildFlags & ImGuiChildFlags_ResizeX) && (window->SetWindowSizeAllowFlags & ImGuiCond_FirstUseEver) == 0)
            g.NextWindowData.SizeVal.x = window->SizeFull.x;
        if ((window->ChildFlags & ImGuiChildFlags_ResizeY) && (window->SetWindowSizeAllowFlags & ImGuiCond_FirstUseEver) == 0)
            g.NextWindowData.SizeVal.y = window->SizeFull.y;
        SetWindowSize(window, g.NextWindowData.SizeVal, g.NextWindowData.SizeCond);
    }
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasScroll)
    {
        if (g.NextWindowData.ScrollVal.x >= 0.0f)
        {
            window->ScrollTarget.x = g.NextWindowData.ScrollVal.x;
            window->ScrollTargetCenterRatio.x = 0.0f;
        }
        if (g.NextWindowData.ScrollVal.y >= 0.0f)
        {
            window->ScrollTarget.y = g.NextWindowData.ScrollVal.y;
            window->ScrollTargetCenterRatio.y = 0.0f;
        }
    }
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasContentSize)
        window->ContentSizeExplicit = g.NextWindowData.ContentSizeVal;
    else if (first_begin_of_the_frame)
        window->ContentSizeExplicit = ImVec2(0.0f, 0.0f);
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasCollapsed)
        SetWindowCollapsed(window, g.NextWindowData.CollapsedVal, g.NextWindowData.CollapsedCond);
    if (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasFocus)
        FocusWindow(window);
    if (window->Appearing)
        SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, false);

    UpdateWindowSkipRefresh(window);

    if (window_stack_data.DisabledOverrideReenable && window->RootWindow == window)
        BeginDisabledOverrideReenable();

    g.CurrentWindow = NULL;

    if (first_begin_of_the_frame && !window->SkipRefresh)
    {

        const bool window_is_child_tooltip = (flags & ImGuiWindowFlags_ChildWindow) && (flags & ImGuiWindowFlags_Tooltip);
        const bool window_just_appearing_after_hidden_for_resize = (window->HiddenFramesCannotSkipItems > 0);
        window->Active = true;
        window->HasCloseButton = (p_open != NULL);
        window->ClipRect = ImVec4(-FLT_MAX, -FLT_MAX, +FLT_MAX, +FLT_MAX);
        window->IDStack.resize(1);
        window->DrawList->_ResetForNewFrame();
        window->DC.CurrentTableIdx = -1;

        if (window->MemoryCompacted)
            GcAwakeTransientWindowBuffers(window);

        bool window_title_visible_elsewhere = false;
        if (g.NavWindowingListWindow != NULL && g.NavWindowingListWindow->WasActive && (flags & ImGuiWindowFlags_NoNavFocus) == 0)
            window_title_visible_elsewhere = true;
        if (flags & ImGuiWindowFlags_ChildMenu)
            window_title_visible_elsewhere = true;
        if ((window_title_visible_elsewhere || window_just_activated_by_user) && !window_just_created && strcmp(name, window->Name) != 0)
        {
            size_t buf_len = (size_t)window->NameBufLen;
            window->Name = ImStrdupcpy(window->Name, &buf_len, name);
            window->NameBufLen = (int)buf_len;
        }

        CalcWindowContentSizes(window, &window->ContentSize, &window->ContentSizeIdeal);
        if (window->HiddenFramesCanSkipItems > 0)
            window->HiddenFramesCanSkipItems--;
        if (window->HiddenFramesCannotSkipItems > 0)
            window->HiddenFramesCannotSkipItems--;
        if (window->HiddenFramesForRenderOnly > 0)
            window->HiddenFramesForRenderOnly--;

        if (window_just_created && (!window_size_x_set_by_api || !window_size_y_set_by_api))
            window->HiddenFramesCannotSkipItems = 1;

        if (window_just_activated_by_user && (flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) != 0)
        {
            window->HiddenFramesCannotSkipItems = 1;
            if (flags & ImGuiWindowFlags_AlwaysAutoResize)
            {
                if (!window_size_x_set_by_api)
                    window->Size.x = window->SizeFull.x = 0.f;
                if (!window_size_y_set_by_api)
                    window->Size.y = window->SizeFull.y = 0.f;
                window->ContentSize = window->ContentSizeIdeal = ImVec2(0.f, 0.f);
            }
        }

        ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)GetMainViewport();
        SetWindowViewport(window, viewport);
        SetCurrentWindow(window);

        if (flags & ImGuiWindowFlags_ChildWindow)
            window->WindowBorderSize = style.ChildBorderSize;
        else
            window->WindowBorderSize = ((flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupBorderSize : style.WindowBorderSize;
        window->WindowPadding = style.WindowPadding;
        if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !(window->ChildFlags & ImGuiChildFlags_AlwaysUseWindowPadding) && window->WindowBorderSize == 0.0f)
            window->WindowPadding = ImVec2(0.0f, (flags & ImGuiWindowFlags_MenuBar) ? style.WindowPadding.y : 0.0f);

        window->DC.MenuBarOffset.x = ImMax(ImMax(window->WindowPadding.x, style.ItemSpacing.x), g.NextWindowData.MenuBarOffsetMinVal.x);
        window->DC.MenuBarOffset.y = g.NextWindowData.MenuBarOffsetMinVal.y;
        window->TitleBarHeight = (flags & ImGuiWindowFlags_NoTitleBar) ? 0.0f : g.FontSize + g.Style.FramePadding.y * 2.0f;
        window->MenuBarHeight = (flags & ImGuiWindowFlags_MenuBar) ? window->DC.MenuBarOffset.y + g.FontSize + g.Style.FramePadding.y * 2.0f : 0.0f;
        window->FontRefSize = g.FontSize;

        bool use_current_size_for_scrollbar_x = window_just_created;
        bool use_current_size_for_scrollbar_y = window_just_created;
        if (window_size_x_set_by_api && window->ContentSizeExplicit.x != 0.0f)
            use_current_size_for_scrollbar_x = true;
        if (window_size_y_set_by_api && window->ContentSizeExplicit.y != 0.0f)
            use_current_size_for_scrollbar_y = true;

        if (!(flags & ImGuiWindowFlags_NoTitleBar) && !(flags & ImGuiWindowFlags_NoCollapse))
        {

            ImRect title_bar_rect = window->TitleBarRect();
            if (g.HoveredWindow == window && g.HoveredId == 0 && g.HoveredIdPreviousFrame == 0 && g.ActiveId == 0 && IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max))
                if (g.IO.MouseClickedCount[0] == 2 && GetKeyOwner(ImGuiKey_MouseLeft) == ImGuiKeyOwner_NoOwner)
                    window->WantCollapseToggle = true;
            if (window->WantCollapseToggle)
            {
                window->Collapsed = !window->Collapsed;
                if (!window->Collapsed)
                    use_current_size_for_scrollbar_y = true;
                MarkIniSettingsDirty(window);
            }
        }
        else
        {
            window->Collapsed = false;
        }
        window->WantCollapseToggle = false;

        const ImVec2 scrollbar_sizes_from_last_frame = window->ScrollbarSizes;
        window->DecoOuterSizeX1 = 0.0f;
        window->DecoOuterSizeX2 = 0.0f;
        window->DecoOuterSizeY1 = window->TitleBarHeight + window->MenuBarHeight;
        window->DecoOuterSizeY2 = 0.0f;
        window->ScrollbarSizes = ImVec2(0.0f, 0.0f);

        {
            const bool size_auto_fit_x_always = !window_size_x_set_by_api && (flags & ImGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed;
            const bool size_auto_fit_y_always = !window_size_y_set_by_api && (flags & ImGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed;
            const bool size_auto_fit_x_current = !window_size_x_set_by_api && (window->AutoFitFramesX > 0);
            const bool size_auto_fit_y_current = !window_size_y_set_by_api && (window->AutoFitFramesY > 0);
            int size_auto_fit_mask = 0;
            if (size_auto_fit_x_always || size_auto_fit_x_current)
                size_auto_fit_mask |= (1 << ImGuiAxis_X);
            if (size_auto_fit_y_always || size_auto_fit_y_current)
                size_auto_fit_mask |= (1 << ImGuiAxis_Y);
            const ImVec2 size_auto_fit = CalcWindowAutoFitSize(window, window->ContentSizeIdeal, size_auto_fit_mask);

            const ImVec2 old_size = window->SizeFull;
            if (size_auto_fit_x_always || size_auto_fit_x_current)
            {
                if (size_auto_fit_x_always)
                    window->SizeFull.x = size_auto_fit.x;
                else
                    window->SizeFull.x = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
                use_current_size_for_scrollbar_x = true;
            }
            if (size_auto_fit_y_always || size_auto_fit_y_current)
            {
                if (size_auto_fit_y_always)
                    window->SizeFull.y = size_auto_fit.y;
                else
                    window->SizeFull.y = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
                use_current_size_for_scrollbar_y = true;
            }
            if (old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
                MarkIniSettingsDirty(window);
        }

        window->SizeFull = CalcWindowSizeAfterConstraint(window, window->SizeFull);
        window->Size = window->Collapsed && !(flags & ImGuiWindowFlags_ChildWindow) ? window->TitleBarRect().GetSize() : window->SizeFull;

        if (window_just_activated_by_user)
        {
            window->AutoPosLastDirection = ImGuiDir_None;
            if ((flags & ImGuiWindowFlags_Popup) != 0 && !(flags & ImGuiWindowFlags_Modal) && !window_pos_set_by_api)
                window->Pos = g.BeginPopupStack.back().OpenPopupPos;
        }

        if (flags & ImGuiWindowFlags_ChildWindow)
        {
            IM_ASSERT(parent_window && parent_window->Active);
            window->BeginOrderWithinParent = (short)parent_window->DC.ChildWindows.Size;
            parent_window->DC.ChildWindows.push_back(window);
            if (!(flags & ImGuiWindowFlags_Popup) && !window_pos_set_by_api && !window_is_child_tooltip)
                window->Pos = parent_window->DC.CursorPos;
        }

        const bool window_pos_with_pivot = (window->SetWindowPosVal.x != FLT_MAX && window->HiddenFramesCannotSkipItems == 0);
        if (window_pos_with_pivot)
            SetWindowPos(window, window->SetWindowPosVal - window->Size * window->SetWindowPosPivot, 0);
        else if ((flags & ImGuiWindowFlags_ChildMenu) != 0)
            window->Pos = FindBestWindowPosForPopup(window);
        else if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api && window_just_appearing_after_hidden_for_resize)
            window->Pos = FindBestWindowPosForPopup(window);
        else if ((flags & ImGuiWindowFlags_Tooltip) != 0 && !window_pos_set_by_api && !window_is_child_tooltip)
            window->Pos = FindBestWindowPosForPopup(window);

        ImRect viewport_rect(viewport->GetMainRect());
        ImRect viewport_work_rect(viewport->GetWorkRect());
        ImVec2 visibility_padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
        ImRect visibility_rect(viewport_work_rect.Min + visibility_padding, viewport_work_rect.Max - visibility_padding);

        if (!window_pos_set_by_api && !(flags & ImGuiWindowFlags_ChildWindow))
            if (viewport_rect.GetWidth() > 0.0f && viewport_rect.GetHeight() > 0.0f)
                ClampWindowPos(window, visibility_rect);
        window->Pos = ImTrunc(window->Pos);

        window->WindowRounding = (flags & ImGuiWindowFlags_ChildWindow) ? style.ChildRounding : ((flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupRounding : style.WindowRounding;

        bool want_focus = false;
        if (window_just_activated_by_user && !(flags & ImGuiWindowFlags_NoFocusOnAppearing))
        {
            if (flags & ImGuiWindowFlags_Popup)
                want_focus = true;
            else if ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Tooltip)) == 0)
                want_focus = true;
        }

#ifdef IMGUI_ENABLE_TEST_ENGINE
        if (g.TestEngineHookItems)
        {
            IM_ASSERT(window->IDStack.Size == 1);
            window->IDStack.Size = 0;
            window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
            IMGUI_TEST_ENGINE_ITEM_ADD(window->ID, window->Rect(), NULL);
            IMGUI_TEST_ENGINE_ITEM_INFO(window->ID, window->Name, (g.HoveredWindow == window) ? ImGuiItemStatusFlags_HoveredRect : 0);
            window->IDStack.Size = 1;
            window->DC.NavLayerCurrent = ImGuiNavLayer_Main;

        }
#endif

        bool handle_borders_and_resize_grips = true;
        if ((flags & ImGuiWindowFlags_ChildWindow) && window->ParentWindow->SkipItems)
            handle_borders_and_resize_grips = false;

        int border_hovered = -1, border_held = -1;
        ImU32 resize_grip_col[4] = {};
        int resize_grip_count;
        if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup))
            resize_grip_count = ((window->ChildFlags & ImGuiChildFlags_ResizeX) && (window->ChildFlags & ImGuiChildFlags_ResizeY)) ? 1 : 0;
        else
            resize_grip_count = g.IO.ConfigWindowsResizeFromEdges ? 2 : 1;

        const float resize_grip_draw_size = IM_TRUNC(ImMax(g.FontSize * 1.10f, window->WindowRounding + 1.0f + g.FontSize * 0.2f));
        if (handle_borders_and_resize_grips && !window->Collapsed)
            if (int auto_fit_mask = UpdateWindowManualResize(window, &border_hovered, &border_held, resize_grip_count, &resize_grip_col[0], visibility_rect))
            {
                if (auto_fit_mask & (1 << ImGuiAxis_X))
                    use_current_size_for_scrollbar_x = true;
                if (auto_fit_mask & (1 << ImGuiAxis_Y))
                    use_current_size_for_scrollbar_y = true;
            }
        window->ResizeBorderHovered = (signed char)border_hovered;
        window->ResizeBorderHeld = (signed char)border_held;

        if (!window->Collapsed)
        {

            ImVec2 avail_size_from_current_frame = ImVec2(window->SizeFull.x, window->SizeFull.y - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2));
            ImVec2 avail_size_from_last_frame = window->InnerRect.GetSize() + scrollbar_sizes_from_last_frame;
            ImVec2 needed_size_from_last_frame = window_just_created ? ImVec2(0, 0) : window->ContentSize + window->WindowPadding * 2.0f;
            float size_x_for_scrollbars = use_current_size_for_scrollbar_x ? avail_size_from_current_frame.x : avail_size_from_last_frame.x;
            float size_y_for_scrollbars = use_current_size_for_scrollbar_y ? avail_size_from_current_frame.y : avail_size_from_last_frame.y;
            bool scrollbar_x_prev = window->ScrollbarX;

            window->ScrollbarY = (flags & ImGuiWindowFlags_AlwaysVerticalScrollbar) || ((needed_size_from_last_frame.y > size_y_for_scrollbars) && !(flags & ImGuiWindowFlags_NoScrollbar));
            window->ScrollbarX = (flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar) || ((needed_size_from_last_frame.x > size_x_for_scrollbars - (window->ScrollbarY ? style.ScrollbarSize : 0.0f)) && !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar));

            window->ScrollbarXStabilizeToggledHistory <<= 1;
            window->ScrollbarXStabilizeToggledHistory |= (scrollbar_x_prev != window->ScrollbarX) ? 0x01 : 0x00;
            const bool scrollbar_x_stabilize = (window->ScrollbarXStabilizeToggledHistory != 0) && ImCountSetBits(window->ScrollbarXStabilizeToggledHistory) >= 4;
            if (scrollbar_x_stabilize)
                window->ScrollbarX = true;

            window->ScrollbarXStabilizeEnabled = scrollbar_x_stabilize;

            if (window->ScrollbarX && !window->ScrollbarY)
                window->ScrollbarY = (needed_size_from_last_frame.y > size_y_for_scrollbars - style.ScrollbarSize) && !(flags & ImGuiWindowFlags_NoScrollbar);
            window->ScrollbarSizes = ImVec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f, window->ScrollbarX ? style.ScrollbarSize : 0.0f);

            window->DecoOuterSizeX2 += window->ScrollbarSizes.x;
            window->DecoOuterSizeY2 += window->ScrollbarSizes.y;
        }

        const ImRect host_rect = ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip) ? parent_window->ClipRect : viewport_rect;
        const ImRect outer_rect = window->Rect();
        const ImRect title_bar_rect = window->TitleBarRect();
        window->OuterRectClipped = outer_rect;
        window->OuterRectClipped.ClipWith(host_rect);

        window->InnerRect.Min.x = window->Pos.x + window->DecoOuterSizeX1;
        window->InnerRect.Min.y = window->Pos.y + window->DecoOuterSizeY1;
        window->InnerRect.Max.x = window->Pos.x + window->Size.x - window->DecoOuterSizeX2;
        window->InnerRect.Max.y = window->Pos.y + window->Size.y - window->DecoOuterSizeY2;

        float top_border_size = (((flags & ImGuiWindowFlags_MenuBar) || !(flags & ImGuiWindowFlags_NoTitleBar)) ? style.FrameBorderSize : window->WindowBorderSize);

        window->InnerClipRect.Min.x = ImFloor(0.5f + window->InnerRect.Min.x + window->WindowBorderSize * 0.5f);
        window->InnerClipRect.Min.y = ImFloor(0.5f + window->InnerRect.Min.y + top_border_size * 0.5f);
        window->InnerClipRect.Max.x = ImFloor(window->InnerRect.Max.x - window->WindowBorderSize * 0.5f);
        window->InnerClipRect.Max.y = ImFloor(window->InnerRect.Max.y - window->WindowBorderSize * 0.5f);
        window->InnerClipRect.ClipWithFull(host_rect);

        window->ScrollMax.x = ImMax(0.0f, window->ContentSize.x + window->WindowPadding.x * 2.0f - window->InnerRect.GetWidth());
        window->ScrollMax.y = ImMax(0.0f, window->ContentSize.y + window->WindowPadding.y * 2.0f - window->InnerRect.GetHeight());

        window->Scroll = CalcNextScrollFromScrollTargetAndClamp(window);
        window->ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);
        window->DecoInnerSizeX1 = window->DecoInnerSizeY1 = 0.0f;

        IM_ASSERT(window->DrawList->CmdBuffer.Size == 1 && window->DrawList->CmdBuffer[0].ElemCount == 0);
        window->DrawList->PushTexture(g.Font->OwnerAtlas->TexRef);
        PushClipRect(host_rect.Min, host_rect.Max, false);

        {
            bool render_decorations_in_parent = false;
            if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip)
            {

                ImGuiWindow* previous_child = parent_window->DC.ChildWindows.Size >= 2 ? parent_window->DC.ChildWindows[parent_window->DC.ChildWindows.Size - 2] : NULL;
                bool previous_child_overlapping = previous_child ? previous_child->Rect().Overlaps(window->Rect()) : false;
                bool parent_is_empty = (parent_window->DrawList->VtxBuffer.Size == 0);
                if (window->DrawList->CmdBuffer.back().ElemCount == 0 && !parent_is_empty && !previous_child_overlapping)
                    render_decorations_in_parent = true;
            }
            if (render_decorations_in_parent)
                window->DrawList = parent_window->DrawList;

            const ImGuiWindow* window_to_highlight = g.NavWindowingTarget ? g.NavWindowingTarget : g.NavWindow;
            const bool title_bar_is_highlight = want_focus || (window_to_highlight && window->RootWindowForTitleBarHighlight == window_to_highlight->RootWindowForTitleBarHighlight);
            RenderWindowDecorations(window, title_bar_rect, title_bar_is_highlight, handle_borders_and_resize_grips, resize_grip_count, resize_grip_col, resize_grip_draw_size);

            if (render_decorations_in_parent)
                window->DrawList = &window->DrawListInst;
        }

        const bool allow_scrollbar_x = !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar);
        const bool allow_scrollbar_y = !(flags & ImGuiWindowFlags_NoScrollbar);
        const float work_rect_size_x = (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : ImMax(allow_scrollbar_x ? window->ContentSize.x : 0.0f, window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
        const float work_rect_size_y = (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : ImMax(allow_scrollbar_y ? window->ContentSize.y : 0.0f, window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));
        window->WorkRect.Min.x = ImTrunc(window->InnerRect.Min.x - window->Scroll.x + ImMax(window->WindowPadding.x, window->WindowBorderSize));
        window->WorkRect.Min.y = ImTrunc(window->InnerRect.Min.y - window->Scroll.y + ImMax(window->WindowPadding.y, window->WindowBorderSize));
        window->WorkRect.Max.x = window->WorkRect.Min.x + work_rect_size_x;
        window->WorkRect.Max.y = window->WorkRect.Min.y + work_rect_size_y;
        window->ParentWorkRect = window->WorkRect;

        window->ContentRegionRect.Min.x = window->Pos.x - window->Scroll.x + window->WindowPadding.x + window->DecoOuterSizeX1;
        window->ContentRegionRect.Min.y = window->Pos.y - window->Scroll.y + window->WindowPadding.y + window->DecoOuterSizeY1;
        window->ContentRegionRect.Max.x = window->ContentRegionRect.Min.x + (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : (window->Size.x - window->WindowPadding.x * 2.0f - (window->DecoOuterSizeX1 + window->DecoOuterSizeX2)));
        window->ContentRegionRect.Max.y = window->ContentRegionRect.Min.y + (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : (window->Size.y - window->WindowPadding.y * 2.0f - (window->DecoOuterSizeY1 + window->DecoOuterSizeY2)));

        window->DC.Indent.x = window->DecoOuterSizeX1 + window->WindowPadding.x - window->Scroll.x;
        window->DC.GroupOffset.x = 0.0f;
        window->DC.ColumnsOffset.x = 0.0f;

        double start_pos_highp_x = (double)window->Pos.x + window->WindowPadding.x - (double)window->Scroll.x + window->DecoOuterSizeX1 + window->DC.ColumnsOffset.x;
        double start_pos_highp_y = (double)window->Pos.y + window->WindowPadding.y - (double)window->Scroll.y + window->DecoOuterSizeY1;
        window->DC.CursorStartPos  = ImVec2((float)start_pos_highp_x, (float)start_pos_highp_y);
        window->DC.CursorStartPosLossyness = ImVec2((float)(start_pos_highp_x - window->DC.CursorStartPos.x), (float)(start_pos_highp_y - window->DC.CursorStartPos.y));
        window->DC.CursorPos = window->DC.CursorStartPos;
        window->DC.CursorPosPrevLine = window->DC.CursorPos;
        window->DC.CursorMaxPos = window->DC.CursorStartPos;
        window->DC.IdealMaxPos = window->DC.CursorStartPos;
        window->DC.CurrLineSize = window->DC.PrevLineSize = ImVec2(0.0f, 0.0f);
        window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;
        window->DC.IsSameLine = window->DC.IsSetPos = false;

        window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        window->DC.NavLayersActiveMask = window->DC.NavLayersActiveMaskNext;
        window->DC.NavLayersActiveMaskNext = 0x00;
        window->DC.NavIsScrollPushableX = true;
        window->DC.NavHideHighlightOneFrame = false;
        window->DC.NavWindowHasScrollY = (window->ScrollMax.y > 0.0f);

        window->DC.MenuBarAppending = false;
        window->DC.MenuColumns.Update(style.ItemSpacing.x, window_just_activated_by_user);
        window->DC.TreeDepth = 0;
        window->DC.TreeHasStackDataDepthMask = window->DC.TreeRecordsClippedNodesY2Mask = 0x00;
        window->DC.ChildWindows.resize(0);
        window->DC.StateStorage = &window->StateStorage;
        window->DC.CurrentColumns = NULL;
        window->DC.LayoutType = ImGuiLayoutType_Vertical;
        window->DC.ParentLayoutType = parent_window ? parent_window->DC.LayoutType : ImGuiLayoutType_Vertical;

        const bool is_resizable_window = (window->Size.x > 0.0f && !(flags & ImGuiWindowFlags_Tooltip) && !(flags & ImGuiWindowFlags_AlwaysAutoResize));
        if (is_resizable_window)
            window->DC.ItemWidthDefault = ImTrunc(window->Size.x * 0.65f);
        else
            window->DC.ItemWidthDefault = ImTrunc(g.FontSize * 16.0f);
        window->DC.ItemWidth = window->DC.ItemWidthDefault;
        window->DC.ItemWidthStack.resize(0);
        window->DC.TextWrapPos = -1.0f;
        window->DC.TextWrapPosStack.resize(0);
        if (flags & ImGuiWindowFlags_Modal)
            window->DC.ModalDimBgColor = ColorConvertFloat4ToU32(GetStyleColorVec4(ImGuiCol_ModalWindowDimBg));

        if (window->AutoFitFramesX > 0)
            window->AutoFitFramesX--;
        if (window->AutoFitFramesY > 0)
            window->AutoFitFramesY--;

        if (want_focus)
            FocusWindow(window, ImGuiFocusRequestFlags_UnlessBelowModal);
        if (want_focus && window == g.NavWindow)
            NavInitWindow(window, false);

        if (g.IO.ConfigWindowsCopyContentsWithCtrlC)
            if (g.NavWindow && g.NavWindow->RootWindow == window && g.ActiveId == 0 && Shortcut(ImGuiMod_Ctrl | ImGuiKey_C))
                LogToClipboard(0);

        if (!(flags & ImGuiWindowFlags_NoTitleBar))
            RenderWindowTitleBarContents(window, ImRect(title_bar_rect.Min.x + window->WindowBorderSize, title_bar_rect.Min.y, title_bar_rect.Max.x - window->WindowBorderSize, title_bar_rect.Max.y), name, p_open);

        window->HitTestHoleSize.x = window->HitTestHoleSize.y = 0;

        if (flags & ImGuiWindowFlags_Tooltip)
            g.TooltipPreviousWindow = window;

        window->BgClickFlags = (flags & ImGuiWindowFlags_ChildWindow) ? parent_window->BgClickFlags : (g.IO.ConfigWindowsMoveFromTitleBarOnly ? ImGuiWindowBgClickFlags_None : ImGuiWindowBgClickFlags_Move);

        window->DC.WindowItemStatusFlags = ImGuiItemStatusFlags_None;
        window->DC.WindowItemStatusFlags |= IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max, false) ? ImGuiItemStatusFlags_HoveredRect : 0;
        SetLastItemDataForWindow(window, title_bar_rect);

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
        if (g.DebugLocateId != 0 && (window->ID == g.DebugLocateId || window->MoveId == g.DebugLocateId))
            DebugLocateItemResolveWithLastItem();
#endif

#ifdef IMGUI_ENABLE_TEST_ENGINE
        if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
        {
            window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
            IMGUI_TEST_ENGINE_ITEM_ADD(g.LastItemData.ID, g.LastItemData.Rect, &g.LastItemData);
            window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        }
#endif
    }
    else
    {

        if (window->SkipRefresh)
            SetWindowActiveForSkipRefresh(window);

        SetCurrentWindow(window);
        SetLastItemDataForWindow(window, window->TitleBarRect());
    }

    if (!window->SkipRefresh)
        PushClipRect(window->InnerClipRect.Min, window->InnerClipRect.Max, true);

    window->WriteAccessed = false;
    window->BeginCount++;
    g.NextWindowData.ClearFlags();

    if (first_begin_of_the_frame && !window->SkipRefresh)
    {
        if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_ChildMenu))
        {

            IM_ASSERT((flags & ImGuiWindowFlags_NoTitleBar) != 0);
            const bool nav_request = (window->ChildFlags & ImGuiChildFlags_NavFlattened) && (g.NavAnyRequest && g.NavWindow && g.NavWindow->RootWindowForNav == window->RootWindowForNav);
            if (!g.LogEnabled && !nav_request)
                if (window->OuterRectClipped.Min.x >= window->OuterRectClipped.Max.x || window->OuterRectClipped.Min.y >= window->OuterRectClipped.Max.y)
                {
                    if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
                        window->HiddenFramesCannotSkipItems = 1;
                    else
                        window->HiddenFramesCanSkipItems = 1;
                }

            if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCanSkipItems > 0))
                window->HiddenFramesCanSkipItems = 1;
            if (parent_window && parent_window->HiddenFramesCannotSkipItems > 0)
                window->HiddenFramesCannotSkipItems = 1;
        }

        if (style.Alpha <= 0.0f)
            window->HiddenFramesCanSkipItems = 1;

        bool hidden_regular = (window->HiddenFramesCanSkipItems > 0) || (window->HiddenFramesCannotSkipItems > 0);
        window->Hidden = hidden_regular || (window->HiddenFramesForRenderOnly > 0);

        if (window->DisableInputsFrames > 0)
        {
            window->DisableInputsFrames--;
            window->Flags |= ImGuiWindowFlags_NoInputs;
        }

        bool skip_items = false;
        if (window->Collapsed || !window->Active || hidden_regular)
            if (window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && window->HiddenFramesCannotSkipItems <= 0)
                skip_items = true;
        window->SkipItems = skip_items;
    }
    else if (first_begin_of_the_frame)
    {

        window->SkipItems = true;
    }

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (!window->IsFallbackWindow)
        if ((g.IO.ConfigDebugBeginReturnValueOnce && window_just_created) || (g.IO.ConfigDebugBeginReturnValueLoop && g.DebugBeginReturnValueCullDepth == g.CurrentWindowStack.Size))
        {
            if (window->AutoFitFramesX > 0) { window->AutoFitFramesX++; }
            if (window->AutoFitFramesY > 0) { window->AutoFitFramesY++; }
            return false;
        }
#endif

    return !window->SkipItems;
}

void ImGui::End()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    if (g.CurrentWindowStack.Size <= 1 && g.WithinFrameScopeWithImplicitWindow)
    {
        IM_ASSERT_USER_ERROR(g.CurrentWindowStack.Size > 1, "Calling End() too many times!");
        return;
    }
    ImGuiWindowStackData& window_stack_data = g.CurrentWindowStack.back();

    if (window->Flags & ImGuiWindowFlags_ChildWindow)
        IM_ASSERT_USER_ERROR(g.WithinEndChildID == window->ID, "Must call EndChild() and not End()!");

    if (window->DC.CurrentColumns)
        EndColumns();
    if (!window->SkipRefresh)
        PopClipRect();
    PopFocusScope();
    if (window_stack_data.DisabledOverrideReenable && window->RootWindow == window)
        EndDisabledOverrideReenable();

    if (window->SkipRefresh)
    {
        IM_ASSERT(window->DrawList == NULL);
        window->DrawList = &window->DrawListInst;
    }

    if (g.LogWindow == window)
        LogFinish();

    if (window->DC.IsSetPos)
        ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

    g.LastItemData = window_stack_data.ParentLastItemDataBackup;
    if (window->Flags & ImGuiWindowFlags_ChildMenu)
        g.BeginMenuDepth--;
    if (window->Flags & ImGuiWindowFlags_Popup)
        g.BeginPopupStack.pop_back();

    if (g.IO.ConfigErrorRecovery)
        ErrorRecoveryTryToRecoverWindowState(&window_stack_data.StackSizesInBegin);

    g.CurrentWindowStack.pop_back();
    SetCurrentWindow(g.CurrentWindowStack.Size == 0 ? NULL : g.CurrentWindowStack.back().Window);
}

void ImGui::PushItemFlag(ImGuiItemFlags option, bool enabled)
{
    ImGuiContext& g = *GImGui;
    ImGuiItemFlags item_flags = g.CurrentItemFlags;
    IM_ASSERT(item_flags == g.ItemFlagsStack.back());
    if (enabled)
        item_flags |= option;
    else
        item_flags &= ~option;
    g.CurrentItemFlags = item_flags;
    g.ItemFlagsStack.push_back(item_flags);
}

void ImGui::PopItemFlag()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.ItemFlagsStack.Size > 1, "Calling PopItemFlag() too many times!");
    g.ItemFlagsStack.pop_back();
    g.CurrentItemFlags = g.ItemFlagsStack.back();
}

void ImGui::BeginDisabled(bool disabled)
{
    ImGuiContext& g = *GImGui;
    bool was_disabled = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (!was_disabled && disabled)
    {
        g.DisabledAlphaBackup = g.Style.Alpha;
        g.Style.Alpha *= g.Style.DisabledAlpha;
    }
    if (was_disabled || disabled)
        g.CurrentItemFlags |= ImGuiItemFlags_Disabled;
    g.ItemFlagsStack.push_back(g.CurrentItemFlags);
    g.DisabledStackSize++;
}

void ImGui::EndDisabled()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.DisabledStackSize > 0, "Calling EndDisabled() too many times!");
    g.DisabledStackSize--;
    bool was_disabled = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;

    g.ItemFlagsStack.pop_back();
    g.CurrentItemFlags = g.ItemFlagsStack.back();
    if (was_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
        g.Style.Alpha = g.DisabledAlphaBackup;
}

void ImGui::BeginDisabledOverrideReenable()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.CurrentItemFlags & ImGuiItemFlags_Disabled);
    g.CurrentWindowStack.back().DisabledOverrideReenableAlphaBackup = g.Style.Alpha;
    g.Style.Alpha = g.DisabledAlphaBackup;
    g.CurrentItemFlags &= ~ImGuiItemFlags_Disabled;
    g.ItemFlagsStack.push_back(g.CurrentItemFlags);
    g.DisabledStackSize++;
}

void ImGui::EndDisabledOverrideReenable()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.DisabledStackSize > 0);
    g.DisabledStackSize--;
    g.ItemFlagsStack.pop_back();
    g.CurrentItemFlags = g.ItemFlagsStack.back();
    g.Style.Alpha = g.CurrentWindowStack.back().DisabledOverrideReenableAlphaBackup;
}

void ImGui::PushTextWrapPos(float wrap_local_pos_x)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DC.TextWrapPosStack.push_back(window->DC.TextWrapPos);
    window->DC.TextWrapPos = wrap_local_pos_x;
}

void ImGui::PopTextWrapPos()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT_USER_ERROR_RET(window->DC.TextWrapPosStack.Size > 0, "Calling PopTextWrapPos() too many times!");
    window->DC.TextWrapPos = window->DC.TextWrapPosStack.back();
    window->DC.TextWrapPosStack.pop_back();
}

static ImGuiWindow* GetCombinedRootWindow(ImGuiWindow* window, bool popup_hierarchy)
{
    ImGuiWindow* last_window = NULL;
    while (last_window != window)
    {
        last_window = window;
        window = window->RootWindow;
        if (popup_hierarchy)
            window = window->RootWindowPopupTree;
    }
    return window;
}

bool ImGui::IsWindowChildOf(ImGuiWindow* window, ImGuiWindow* potential_parent, bool popup_hierarchy)
{
    ImGuiWindow* window_root = GetCombinedRootWindow(window, popup_hierarchy);
    if (window_root == potential_parent)
        return true;
    while (window != NULL)
    {
        if (window == potential_parent)
            return true;
        if (window == window_root)
            return false;
        window = window->ParentWindow;
    }
    return false;
}

bool ImGui::IsWindowInBeginStack(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    for (int n = g.CurrentWindowStack.Size - 1; n >= 0; n--)
        if (g.CurrentWindowStack[n].Window == window)
            return true;
    return false;
}

bool ImGui::IsWindowWithinBeginStackOf(ImGuiWindow* window, ImGuiWindow* potential_parent)
{
    if (window->RootWindow == potential_parent)
        return true;
    while (window != NULL)
    {
        if (window == potential_parent)
            return true;
        window = window->ParentWindowInBeginStack;
    }
    return false;
}

bool ImGui::IsWindowAbove(ImGuiWindow* potential_above, ImGuiWindow* potential_below)
{
    ImGuiContext& g = *GImGui;

    const int display_layer_delta = GetWindowDisplayLayer(potential_above) - GetWindowDisplayLayer(potential_below);
    if (display_layer_delta != 0)
        return display_layer_delta > 0;

    for (int i = g.Windows.Size - 1; i >= 0; i--)
    {
        ImGuiWindow* candidate_window = g.Windows[i];
        if (candidate_window == potential_above)
            return true;
        if (candidate_window == potential_below)
            return false;
    }
    return false;
}

bool ImGui::IsWindowHovered(ImGuiHoveredFlags flags)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR((flags & ~ImGuiHoveredFlags_AllowedMaskForIsWindowHovered) == 0, "Invalid flags for IsWindowHovered()!");

    ImGuiWindow* ref_window = g.HoveredWindow;
    ImGuiWindow* cur_window = g.CurrentWindow;
    if (ref_window == NULL)
        return false;

    if ((flags & ImGuiHoveredFlags_AnyWindow) == 0)
    {
        IM_ASSERT(cur_window);
        const bool popup_hierarchy = (flags & ImGuiHoveredFlags_NoPopupHierarchy) == 0;
        if (flags & ImGuiHoveredFlags_RootWindow)
            cur_window = GetCombinedRootWindow(cur_window, popup_hierarchy);

        bool result;
        if (flags & ImGuiHoveredFlags_ChildWindows)
            result = IsWindowChildOf(ref_window, cur_window, popup_hierarchy);
        else
            result = (ref_window == cur_window);
        if (!result)
            return false;
    }

    if (!IsWindowContentHoverable(ref_window, flags))
        return false;
    if (!(flags & ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        if (g.ActiveId != 0 && !g.ActiveIdAllowOverlap && g.ActiveId != ref_window->MoveId)
            return false;

    if (flags & ImGuiHoveredFlags_ForTooltip)
        flags = ApplyHoverFlagsForTooltip(flags, g.Style.HoverFlagsForTooltipMouse);
    if ((flags & ImGuiHoveredFlags_Stationary) != 0 && g.HoverWindowUnlockedStationaryId != ref_window->ID)
        return false;

    return true;
}

float ImGui::GetWindowWidth()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Size.x;
}

float ImGui::GetWindowHeight()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Size.y;
}

ImVec2 ImGui::GetWindowPos()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    return window->Pos;
}

void ImGui::SetWindowPos(ImGuiWindow* window, const ImVec2& pos, ImGuiCond cond)
{

    if (cond && (window->SetWindowPosAllowFlags & cond) == 0)
        return;

    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond));
    window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
    window->SetWindowPosVal = ImVec2(FLT_MAX, FLT_MAX);

    const ImVec2 old_pos = window->Pos;
    window->Pos = ImTrunc(pos);
    ImVec2 offset = window->Pos - old_pos;
    if (offset.x == 0.0f && offset.y == 0.0f)
        return;
    MarkIniSettingsDirty(window);
    window->DC.CursorPos += offset;
    window->DC.CursorMaxPos += offset;
    window->DC.IdealMaxPos += offset;
    window->DC.CursorStartPos += offset;
}

void ImGui::SetWindowPos(const ImVec2& pos, ImGuiCond cond)
{
    ImGuiWindow* window = GetCurrentWindowRead();
    SetWindowPos(window, pos, cond);
}

void ImGui::SetWindowPos(const char* name, const ImVec2& pos, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowPos(window, pos, cond);
}

ImVec2 ImGui::GetWindowSize()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Size;
}

void ImGui::SetWindowSize(ImGuiWindow* window, const ImVec2& size, ImGuiCond cond)
{

    if (cond && (window->SetWindowSizeAllowFlags & cond) == 0)
        return;

    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond));
    window->SetWindowSizeAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    if ((window->Flags & ImGuiWindowFlags_ChildWindow) == 0 || window->Appearing || (window->ChildFlags & ImGuiChildFlags_AlwaysAutoResize) != 0)
        window->AutoFitFramesX = (size.x <= 0.0f) ? 2 : 0;
    if ((window->Flags & ImGuiWindowFlags_ChildWindow) == 0 || window->Appearing || (window->ChildFlags & ImGuiChildFlags_AlwaysAutoResize) != 0)
        window->AutoFitFramesY = (size.y <= 0.0f) ? 2 : 0;

    ImVec2 old_size = window->SizeFull;
    if (size.x <= 0.0f)
        window->AutoFitOnlyGrows = false;
    else
        window->SizeFull.x = IM_TRUNC(size.x);
    if (size.y <= 0.0f)
        window->AutoFitOnlyGrows = false;
    else
        window->SizeFull.y = IM_TRUNC(size.y);
    if (old_size.x != window->SizeFull.x || old_size.y != window->SizeFull.y)
        MarkIniSettingsDirty(window);
}

void ImGui::SetWindowSize(const ImVec2& size, ImGuiCond cond)
{
    SetWindowSize(GImGui->CurrentWindow, size, cond);
}

void ImGui::SetWindowSize(const char* name, const ImVec2& size, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowSize(window, size, cond);
}

void ImGui::SetWindowCollapsed(ImGuiWindow* window, bool collapsed, ImGuiCond cond)
{

    if (cond && (window->SetWindowCollapsedAllowFlags & cond) == 0)
        return;
    window->SetWindowCollapsedAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);

    if (window->WantCollapseToggle)
        window->Collapsed ^= 1;
    window->WantCollapseToggle = (window->Collapsed != collapsed);
}

void ImGui::SetWindowHitTestHole(ImGuiWindow* window, const ImVec2& pos, const ImVec2& size)
{
    IM_ASSERT(window->HitTestHoleSize.x == 0);
    window->HitTestHoleSize = ImVec2ih(size);
    window->HitTestHoleOffset = ImVec2ih(pos - window->Pos);
}

void ImGui::SetWindowHiddenAndSkipItemsForCurrentFrame(ImGuiWindow* window)
{
    window->Hidden = window->SkipItems = true;
    window->HiddenFramesCanSkipItems = 1;
}

void ImGui::SetWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    SetWindowCollapsed(GImGui->CurrentWindow, collapsed, cond);
}

bool ImGui::IsWindowCollapsed()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Collapsed;
}

bool ImGui::IsWindowAppearing()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->Appearing;
}

void ImGui::SetWindowCollapsed(const char* name, bool collapsed, ImGuiCond cond)
{
    if (ImGuiWindow* window = FindWindowByName(name))
        SetWindowCollapsed(window, collapsed, cond);
}

void ImGui::SetNextWindowPos(const ImVec2& pos, ImGuiCond cond, const ImVec2& pivot)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond));
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasPos;
    g.NextWindowData.PosVal = pos;
    g.NextWindowData.PosPivotVal = pivot;
    g.NextWindowData.PosCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowSize(const ImVec2& size, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond));
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasSize;
    g.NextWindowData.SizeVal = size;
    g.NextWindowData.SizeCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback, void* custom_callback_user_data)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasSizeConstraint;
    g.NextWindowData.SizeConstraintRect = ImRect(size_min, size_max);
    g.NextWindowData.SizeCallback = custom_callback;
    g.NextWindowData.SizeCallbackUserData = custom_callback_user_data;
}

void ImGui::SetNextWindowContentSize(const ImVec2& size)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasContentSize;
    g.NextWindowData.ContentSizeVal = ImTrunc(size);
}

void ImGui::SetNextWindowScroll(const ImVec2& scroll)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasScroll;
    g.NextWindowData.ScrollVal = scroll;
}

void ImGui::SetNextWindowCollapsed(bool collapsed, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond));
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasCollapsed;
    g.NextWindowData.CollapsedVal = collapsed;
    g.NextWindowData.CollapsedCond = cond ? cond : ImGuiCond_Always;
}

void ImGui::SetNextWindowBgAlpha(float alpha)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasBgAlpha;
    g.NextWindowData.BgAlphaVal = alpha;
}

void ImGui::SetNextWindowRefreshPolicy(ImGuiWindowRefreshFlags flags)
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasRefreshPolicy;
    g.NextWindowData.RefreshFlagsVal = flags;
}

ImDrawList* ImGui::GetWindowDrawList()
{
    ImGuiWindow* window = GetCurrentWindow();
    return window->DrawList;
}

ImFont* ImGui::GetFont()
{
    return GImGui->Font;
}

ImFontBaked* ImGui::GetFontBaked()
{
    return GImGui->FontBaked;
}

float ImGui::GetFontSize()
{
    return GImGui->FontSize;
}

ImVec2 ImGui::GetFontTexUvWhitePixel()
{
    return GImGui->DrawListSharedData.TexUvWhitePixel;
}

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
void ImGui::SetWindowFontScale(float scale)
{
    IM_ASSERT(scale > 0.0f);
    ImGuiWindow* window = GetCurrentWindow();
    window->FontWindowScale = scale;
    UpdateCurrentFontSize(0.0f);
}
#endif

void ImGui::PushFocusScope(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiFocusScopeData data;
    data.ID = id;
    data.WindowID = g.CurrentWindow->ID;
    g.FocusScopeStack.push_back(data);
    g.CurrentFocusScopeId = id;
}

void ImGui::PopFocusScope()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.FocusScopeStack.Size > g.StackSizesInBeginForCurrentWindow->SizeOfFocusScopeStack, "Calling PopFocusScope() too many times!");
    g.FocusScopeStack.pop_back();
    g.CurrentFocusScopeId = g.FocusScopeStack.Size ? g.FocusScopeStack.back().ID : 0;
}

void ImGui::SetNavFocusScope(ImGuiID focus_scope_id)
{
    ImGuiContext& g = *GImGui;
    g.NavFocusScopeId = focus_scope_id;
    g.NavFocusRoute.resize(0);
    if (focus_scope_id == 0)
        return;
    IM_ASSERT(g.NavWindow != NULL);

    if (focus_scope_id == g.CurrentFocusScopeId)
    {

        for (int n = g.FocusScopeStack.Size - 1; n >= 0 && g.FocusScopeStack.Data[n].WindowID == g.CurrentWindow->ID; n--)
            g.NavFocusRoute.push_back(g.FocusScopeStack.Data[n]);
    }
    else if (focus_scope_id == g.NavWindow->NavRootFocusScopeId)
        g.NavFocusRoute.push_back({ focus_scope_id, g.NavWindow->ID });
    else
        return;

    for (ImGuiWindow* window = g.NavWindow->ParentWindowForFocusRoute; window != NULL; window = window->ParentWindowForFocusRoute)
        g.NavFocusRoute.push_back({ window->NavRootFocusScopeId, window->ID });
    IM_ASSERT(g.NavFocusRoute.Size < 100);
}

void ImGui::FocusItem()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IMGUI_DEBUG_LOG_FOCUS("FocusItem(0x%08x) in window \"%s\"\n", g.LastItemData.ID, window->Name);
    if (g.DragDropActive || g.MovingWindow != NULL)
    {
        IMGUI_DEBUG_LOG_FOCUS("FocusItem() ignored while DragDropActive!\n");
        return;
    }

    ImGuiNavMoveFlags move_flags = ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_FocusApi | ImGuiNavMoveFlags_NoSetNavCursorVisible | ImGuiNavMoveFlags_NoSelect;
    ImGuiScrollFlags scroll_flags = window->Appearing ? ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_AlwaysCenterY : ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY;
    SetNavWindow(window);
    NavMoveRequestSubmit(ImGuiDir_None, ImGuiDir_Up, move_flags, scroll_flags);
    NavMoveRequestResolveWithLastItem(&g.NavMoveResultLocal);
}

void ImGui::ActivateItemByID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.NavNextActivateId = id;
    g.NavNextActivateFlags = ImGuiActivateFlags_None;
}

void ImGui::SetKeyboardFocusHere(int offset)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(offset >= -1);
    IMGUI_DEBUG_LOG_FOCUS("SetKeyboardFocusHere(%d) in window \"%s\"\n", offset, window->Name);

    if (g.DragDropActive || g.MovingWindow != NULL)
    {
        IMGUI_DEBUG_LOG_FOCUS("SetKeyboardFocusHere() ignored while DragDropActive!\n");
        return;
    }

    SetNavWindow(window);

    ImGuiNavMoveFlags move_flags = ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_Activate | ImGuiNavMoveFlags_FocusApi | ImGuiNavMoveFlags_NoSetNavCursorVisible;
    ImGuiScrollFlags scroll_flags = window->Appearing ? ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_AlwaysCenterY : ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY;
    NavMoveRequestSubmit(ImGuiDir_None, offset < 0 ? ImGuiDir_Up : ImGuiDir_Down, move_flags, scroll_flags);
    if (offset == -1)
    {
        NavMoveRequestResolveWithLastItem(&g.NavMoveResultLocal);
    }
    else
    {
        g.NavTabbingDir = 1;
        g.NavTabbingCounter = offset + 1;
    }
}

void ImGui::SetItemDefaultFocus()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!window->Appearing)
        return;
    if (g.NavWindow != window->RootWindowForNav || (!g.NavInitRequest && g.NavInitResult.ID == 0) || g.NavLayer != window->DC.NavLayerCurrent)
        return;

    g.NavInitRequest = false;
    NavApplyItemToResult(&g.NavInitResult);
    NavUpdateAnyRequestFlag();

    if (!window->ClipRect.Contains(g.LastItemData.Rect))
        ScrollToRectEx(window, g.LastItemData.Rect, ImGuiScrollFlags_None);
}

void ImGui::SetStateStorage(ImGuiStorage* tree)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    window->DC.StateStorage = tree ? tree : &window->StateStorage;
}

ImGuiStorage* ImGui::GetStateStorage()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->DC.StateStorage;
}

bool ImGui::IsRectVisible(const ImVec2& size)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ClipRect.Overlaps(ImRect(window->DC.CursorPos, window->DC.CursorPos + size));
}

bool ImGui::IsRectVisible(const ImVec2& rect_min, const ImVec2& rect_max)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ClipRect.Overlaps(ImRect(rect_min, rect_max));
}

static void ImGui::UpdateTexturesNewFrame()
{

    ImGuiContext& g = *GImGui;
    const bool has_textures = (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures) != 0;
    for (ImFontAtlas* atlas : g.FontAtlases)
    {
        if (atlas->OwnerContext == &g)
        {
            ImFontAtlasUpdateNewFrame(atlas, g.FrameCount, has_textures);
        }
        else
        {

            IM_ASSERT(atlas->Builder != NULL && atlas->Builder->FrameCount != -1);
            IM_ASSERT(atlas->RendererHasTextures == has_textures);
        }
    }
}

static void ImGui::UpdateTexturesEndFrame()
{
    ImGuiContext& g = *GImGui;
    g.PlatformIO.Textures.resize(0);
    for (ImFontAtlas* atlas : g.FontAtlases)
        for (ImTextureData* tex : atlas->TexList)
        {

            tex->RefCount = (unsigned short)atlas->RefCount;
            g.PlatformIO.Textures.push_back(tex);
        }
    for (ImTextureData* tex : g.UserTextures)
        g.PlatformIO.Textures.push_back(tex);
}

void ImGui::UpdateFontsNewFrame()
{
    ImGuiContext& g = *GImGui;
    if ((g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures) == 0)
        for (ImFontAtlas* atlas : g.FontAtlases)
            atlas->Locked = true;

    if (g.Style._NextFrameFontSizeBase != 0.0f)
    {
        g.Style.FontSizeBase = g.Style._NextFrameFontSizeBase;
        g.Style._NextFrameFontSizeBase = 0.0f;
    }

    ImFont* font = ImGui::GetDefaultFont();
    if (g.Style.FontSizeBase <= 0.0f)
        g.Style.FontSizeBase = (font->LegacySize > 0.0f ? font->LegacySize : FONT_DEFAULT_SIZE_BASE);

    g.Font = font;
    g.FontSizeBase = g.Style.FontSizeBase;
    g.FontSize = 0.0f;
    ImFontStackData font_stack_data = { font, g.Style.FontSizeBase, g.Style.FontSizeBase };
    SetCurrentFont(font_stack_data.Font, font_stack_data.FontSizeBeforeScaling, 0.0f);
    g.FontStack.push_back(font_stack_data);
    IM_ASSERT(g.Font->IsLoaded());
}

void ImGui::UpdateFontsEndFrame()
{
    PopFont();
}

ImFont* ImGui::GetDefaultFont()
{
    ImGuiContext& g = *GImGui;
    ImFontAtlas* atlas = g.IO.Fonts;
    if (atlas->Builder == NULL || atlas->Fonts.Size == 0)
        ImFontAtlasBuildMain(atlas);
    return g.IO.FontDefault ? g.IO.FontDefault : atlas->Fonts[0];
}

void ImGui::RegisterUserTexture(ImTextureData* tex)
{
    ImGuiContext& g = *GImGui;
    tex->RefCount++;
    g.UserTextures.push_back(tex);
}

void ImGui::UnregisterUserTexture(ImTextureData* tex)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(tex->RefCount > 0);
    tex->RefCount--;
    g.UserTextures.find_erase(tex);
}

void ImGui::RegisterFontAtlas(ImFontAtlas* atlas)
{
    ImGuiContext& g = *GImGui;
    if (g.FontAtlases.Size == 0)
        IM_ASSERT(atlas == g.IO.Fonts);
    atlas->RefCount++;
    g.FontAtlases.push_back(atlas);
    ImFontAtlasAddDrawListSharedData(atlas, &g.DrawListSharedData);
    for (ImTextureData* tex : atlas->TexList)
        tex->RefCount = (unsigned short)atlas->RefCount;
}

void ImGui::UnregisterFontAtlas(ImFontAtlas* atlas)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(atlas->RefCount > 0);
    ImFontAtlasRemoveDrawListSharedData(atlas, &g.DrawListSharedData);
    g.FontAtlases.find_erase(atlas);
    atlas->RefCount--;
    for (ImTextureData* tex : atlas->TexList)
        tex->RefCount = (unsigned short)atlas->RefCount;
}

void ImGui::SetCurrentFont(ImFont* font, float font_size_before_scaling, float font_size_after_scaling)
{
    ImGuiContext& g = *GImGui;
    g.Font = font;
    g.FontSizeBase = font_size_before_scaling;
    UpdateCurrentFontSize(font_size_after_scaling);

    if (font != NULL)
    {
        IM_ASSERT(font && font->IsLoaded());
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        IM_ASSERT(font->Scale > 0.0f);
#endif
        ImFontAtlas* atlas = font->OwnerAtlas;
        g.DrawListSharedData.FontAtlas = atlas;
        g.DrawListSharedData.Font = font;
        ImFontAtlasUpdateDrawListsSharedData(atlas);
        if (g.CurrentWindow != NULL)
            g.CurrentWindow->DrawList->_SetTexture(atlas->TexRef);
    }
}

void ImGui::UpdateCurrentFontSize(float restore_font_size_after_scaling)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    g.Style.FontSizeBase = g.FontSizeBase;

    float final_size = (restore_font_size_after_scaling > 0.0f) ? restore_font_size_after_scaling : 0.0f;
    if (final_size == 0.0f)
    {
        final_size = g.FontSizeBase;

        final_size *= g.Style.FontScaleMain;
        final_size *= g.Style.FontScaleDpi;

        if (window != NULL)
            final_size *= window->FontWindowScale;

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        final_size *= g.IO.FontGlobalScale;
        if (g.Font != NULL)
            final_size *= g.Font->Scale;
#endif
    }

    final_size = GetRoundedFontSize(final_size);
    final_size = ImClamp(final_size, 1.0f, IMGUI_FONT_SIZE_MAX);
    if (g.Font != NULL && (g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures))
        g.Font->CurrentRasterizerDensity = g.FontRasterizerDensity;

    g.FontSize = final_size;
    g.DrawListSharedData.FontSize = g.FontSize;

    if (window != NULL && window->SkipItems)
    {
        ImGuiTable* table = g.CurrentTable;
        const bool allow_early_out = table == NULL || (table->CurrentColumn != -1 && table->Columns[table->CurrentColumn].IsSkipItems == false);
        if (allow_early_out)
            return;
    }

    g.FontBaked = (g.Font != NULL && window != NULL) ? g.Font->GetFontBaked(final_size) : NULL;
    g.FontBakedScale = (g.Font != NULL && window != NULL) ? (g.FontSize / g.FontBaked->Size) : 0.0f;
    g.DrawListSharedData.FontScale = g.FontBakedScale;
}

void ImGui::SetFontRasterizerDensity(float rasterizer_density)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.IO.BackendFlags & ImGuiBackendFlags_RendererHasTextures);
    if (g.FontRasterizerDensity == rasterizer_density)
        return;
    g.FontRasterizerDensity = rasterizer_density;
    UpdateCurrentFontSize(0.0f);
}

void ImGui::PushFont(ImFont* font, float font_size_base)
{
    ImGuiContext& g = *GImGui;
    if (font == NULL)
        font = g.Font;
    IM_ASSERT(font != NULL);
    IM_ASSERT(font_size_base >= 0.0f);

    g.FontStack.push_back({ g.Font, g.FontSizeBase, g.FontSize });
    if (font_size_base == 0.0f)
        font_size_base = g.FontSizeBase;
    SetCurrentFont(font, font_size_base, 0.0f);
}

void  ImGui::PopFont()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT_USER_ERROR_RET(g.FontStack.Size > 0, "Calling PopFont() too many times!");
    ImFontStackData* font_stack_data = &g.FontStack.back();
    SetCurrentFont(font_stack_data->Font, font_stack_data->FontSizeBeforeScaling, font_stack_data->FontSizeAfterScaling);
    g.FontStack.pop_back();
}

IM_MSVC_RUNTIME_CHECKS_OFF
ImGuiID ImGuiWindow::GetID(const char* str, const char* str_end)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHashStr(str, str_end ? (str_end - str) : 0, seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *Ctx;
    if (g.DebugHookIdInfoId == id)
        ImGui::DebugHookIdInfo(id, ImGuiDataType_String, str, str_end);
#endif
    return id;
}

ImGuiID ImGuiWindow::GetID(const void* ptr)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHashData(&ptr, sizeof(void*), seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *Ctx;
    if (g.DebugHookIdInfoId == id)
        ImGui::DebugHookIdInfo(id, ImGuiDataType_Pointer, ptr, NULL);
#endif
    return id;
}

ImGuiID ImGuiWindow::GetID(int n)
{
    ImGuiID seed = IDStack.back();
    ImGuiID id = ImHashData(&n, sizeof(n), seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *Ctx;
    if (g.DebugHookIdInfoId == id)
        ImGui::DebugHookIdInfo(id, ImGuiDataType_S32, (void*)(intptr_t)n, NULL);
#endif
    return id;
}

ImGuiID ImGuiWindow::GetIDFromPos(const ImVec2& p_abs)
{
    ImGuiID seed = IDStack.back();
    ImVec2 p_rel = ImGui::WindowPosAbsToRel(this, p_abs);
    ImGuiID id = ImHashData(&p_rel, sizeof(p_rel), seed);
    return id;
}

ImGuiID ImGuiWindow::GetIDFromRectangle(const ImRect& r_abs)
{
    ImGuiID seed = IDStack.back();
    ImRect r_rel = ImGui::WindowRectAbsToRel(this, r_abs);
    ImGuiID id = ImHashData(&r_rel, sizeof(r_rel), seed);
    return id;
}

void ImGui::PushID(const char* str_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(str_id);
    window->IDStack.push_back(id);
}

void ImGui::PushID(const char* str_id_begin, const char* str_id_end)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(str_id_begin, str_id_end);
    window->IDStack.push_back(id);
}

void ImGui::PushID(const void* ptr_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(ptr_id);
    window->IDStack.push_back(id);
}

void ImGui::PushID(int int_id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID(int_id);
    window->IDStack.push_back(id);
}

void ImGui::PushOverrideID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (g.DebugHookIdInfoId == id)
        DebugHookIdInfo(id, ImGuiDataType_ID, NULL, NULL);
#endif
    window->IDStack.push_back(id);
}

ImGuiID ImGui::GetIDWithSeed(const char* str, const char* str_end, ImGuiID seed)
{
    ImGuiID id = ImHashStr(str, str_end ? (str_end - str) : 0, seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *GImGui;
    if (g.DebugHookIdInfoId == id)
        DebugHookIdInfo(id, ImGuiDataType_String, str, str_end);
#endif
    return id;
}

ImGuiID ImGui::GetIDWithSeed(int n, ImGuiID seed)
{
    ImGuiID id = ImHashData(&n, sizeof(n), seed);
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *GImGui;
    if (g.DebugHookIdInfoId == id)
        DebugHookIdInfo(id, ImGuiDataType_S32, (void*)(intptr_t)n, NULL);
#endif
    return id;
}

void ImGui::PopID()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    IM_ASSERT_USER_ERROR_RET(window->IDStack.Size > 1, "Calling PopID() too many times!");
    window->IDStack.pop_back();
}

ImGuiID ImGui::GetID(const char* str_id)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(str_id);
}

ImGuiID ImGui::GetID(const char* str_id_begin, const char* str_id_end)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(str_id_begin, str_id_end);
}

ImGuiID ImGui::GetID(const void* ptr_id)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(ptr_id);
}

ImGuiID ImGui::GetID(int int_id)
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->GetID(int_id);
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

static ImGuiKeyChord GetModForLRModKey(ImGuiKey key)
{
    if (key == ImGuiKey_LeftCtrl || key == ImGuiKey_RightCtrl)
        return ImGuiMod_Ctrl;
    if (key == ImGuiKey_LeftShift || key == ImGuiKey_RightShift)
        return ImGuiMod_Shift;
    if (key == ImGuiKey_LeftAlt || key == ImGuiKey_RightAlt)
        return ImGuiMod_Alt;
    if (key == ImGuiKey_LeftSuper || key == ImGuiKey_RightSuper)
        return ImGuiMod_Super;
    return ImGuiMod_None;
}

ImGuiKeyChord ImGui::FixupKeyChord(ImGuiKeyChord key_chord)
{

    ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    if (IsLRModKey(key))
        key_chord |= GetModForLRModKey(key);
    return key_chord;
}

ImGuiKeyData* ImGui::GetKeyData(ImGuiContext* ctx, ImGuiKey key)
{
    ImGuiContext& g = *ctx;

    if (key & ImGuiMod_Mask_)
        key = ConvertSingleModFlagToKey(key);

    IM_ASSERT(IsNamedKey(key) && "Support for user key indices was dropped in favor of ImGuiKey. Please update backend & user code.");
    return &g.IO.KeysData[key - ImGuiKey_NamedKey_BEGIN];
}

static const char* const GKeyNames[] =
{
    "Tab", "LeftArrow", "RightArrow", "UpArrow", "DownArrow", "PageUp", "PageDown",
    "Home", "End", "Insert", "Delete", "Backspace", "Space", "Enter", "Escape",
    "LeftCtrl", "LeftShift", "LeftAlt", "LeftSuper", "RightCtrl", "RightShift", "RightAlt", "RightSuper", "Menu",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H",
    "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
    "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24",
    "Apostrophe", "Comma", "Minus", "Period", "Slash", "Semicolon", "Equal", "LeftBracket",
    "Backslash", "RightBracket", "GraveAccent", "CapsLock", "ScrollLock", "NumLock", "PrintScreen",
    "Pause", "Keypad0", "Keypad1", "Keypad2", "Keypad3", "Keypad4", "Keypad5", "Keypad6",
    "Keypad7", "Keypad8", "Keypad9", "KeypadDecimal", "KeypadDivide", "KeypadMultiply",
    "KeypadSubtract", "KeypadAdd", "KeypadEnter", "KeypadEqual",
    "AppBack", "AppForward", "Oem102",
    "GamepadStart", "GamepadBack",
    "GamepadFaceLeft", "GamepadFaceRight", "GamepadFaceUp", "GamepadFaceDown",
    "GamepadDpadLeft", "GamepadDpadRight", "GamepadDpadUp", "GamepadDpadDown",
    "GamepadL1", "GamepadR1", "GamepadL2", "GamepadR2", "GamepadL3", "GamepadR3",
    "GamepadLStickLeft", "GamepadLStickRight", "GamepadLStickUp", "GamepadLStickDown",
    "GamepadRStickLeft", "GamepadRStickRight", "GamepadRStickUp", "GamepadRStickDown",
    "MouseLeft", "MouseRight", "MouseMiddle", "MouseX1", "MouseX2", "MouseWheelX", "MouseWheelY",
    "ModCtrl", "ModShift", "ModAlt", "ModSuper",
};
IM_STATIC_ASSERT(ImGuiKey_NamedKey_COUNT == IM_COUNTOF(GKeyNames));

const char* ImGui::GetKeyName(ImGuiKey key)
{
    if (key == ImGuiKey_None)
        return "None";
    IM_ASSERT(IsNamedKeyOrMod(key) && "Support for user key indices was dropped in favor of ImGuiKey. Please update backend and user code.");
    if (key & ImGuiMod_Mask_)
        key = ConvertSingleModFlagToKey(key);
    if (!IsNamedKey(key))
        return "Unknown";

    return GKeyNames[key - ImGuiKey_NamedKey_BEGIN];
}

const char* ImGui::GetKeyChordName(ImGuiKeyChord key_chord)
{
    ImGuiContext& g = *GImGui;

    const ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    if (IsLRModKey(key))
        key_chord &= ~GetModForLRModKey(key);
    ImFormatString(g.TempKeychordName, IM_COUNTOF(g.TempKeychordName), "%s%s%s%s%s",
        (key_chord & ImGuiMod_Ctrl) ? "Ctrl+" : "",
        (key_chord & ImGuiMod_Shift) ? "Shift+" : "",
        (key_chord & ImGuiMod_Alt) ? "Alt+" : "",
        (key_chord & ImGuiMod_Super) ? "Super+" : "",
        (key != ImGuiKey_None || key_chord == ImGuiKey_None) ? GetKeyName(key) : "");
    size_t len;
    if (key == ImGuiKey_None && key_chord != 0)
        if ((len = ImStrlen(g.TempKeychordName)) != 0)
            g.TempKeychordName[len - 1] = 0;
    return g.TempKeychordName;
}

int ImGui::CalcTypematicRepeatAmount(float t0, float t1, float repeat_delay, float repeat_rate)
{
    if (t1 == 0.0f)
        return 1;
    if (t0 >= t1)
        return 0;
    if (repeat_rate <= 0.0f)
        return t0 < repeat_delay && t1 >= repeat_delay;
    const int count_t0 = (t0 < repeat_delay) ? -1 : (int)((t0 - repeat_delay) / repeat_rate);
    const int count_t1 = (t1 < repeat_delay) ? -1 : (int)((t1 - repeat_delay) / repeat_rate);
    const int count = count_t1 - count_t0;
    return count;
}

void ImGui::GetTypematicRepeatRate(ImGuiInputFlags flags, float* repeat_delay, float* repeat_rate)
{
    ImGuiContext& g = *GImGui;
    switch (flags & ImGuiInputFlags_RepeatRateMask_)
    {
    case ImGuiInputFlags_RepeatRateNavMove:             *repeat_delay = g.IO.KeyRepeatDelay * 0.72f; *repeat_rate = g.IO.KeyRepeatRate * 0.80f; return;
    case ImGuiInputFlags_RepeatRateNavTweak:            *repeat_delay = g.IO.KeyRepeatDelay * 0.72f; *repeat_rate = g.IO.KeyRepeatRate * 0.30f; return;
    case ImGuiInputFlags_RepeatRateDefault: default:    *repeat_delay = g.IO.KeyRepeatDelay * 1.00f; *repeat_rate = g.IO.KeyRepeatRate * 1.00f; return;
    }
}

int ImGui::GetKeyPressedAmount(ImGuiKey key, float repeat_delay, float repeat_rate)
{
    ImGuiContext& g = *GImGui;
    const ImGuiKeyData* key_data = GetKeyData(key);
    if (!key_data->Down)
        return 0;
    const float t = key_data->DownDuration;
    return CalcTypematicRepeatAmount(t - g.IO.DeltaTime, t, repeat_delay, repeat_rate);
}

ImVec2 ImGui::GetKeyMagnitude2d(ImGuiKey key_left, ImGuiKey key_right, ImGuiKey key_up, ImGuiKey key_down)
{
    return ImVec2(
        GetKeyData(key_right)->AnalogValue - GetKeyData(key_left)->AnalogValue,
        GetKeyData(key_down)->AnalogValue - GetKeyData(key_up)->AnalogValue);
}

static void ImGui::UpdateKeyRoutingTable(ImGuiKeyRoutingTable* rt)
{
    ImGuiContext& g = *GImGui;
    rt->EntriesNext.resize(0);
    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
    {
        const int new_routing_start_idx = rt->EntriesNext.Size;
        ImGuiKeyRoutingData* routing_entry;
        for (int old_routing_idx = rt->Index[key - ImGuiKey_NamedKey_BEGIN]; old_routing_idx != -1; old_routing_idx = routing_entry->NextEntryIndex)
        {
            routing_entry = &rt->Entries[old_routing_idx];
            routing_entry->RoutingCurrScore = routing_entry->RoutingNextScore;
            routing_entry->RoutingCurr = routing_entry->RoutingNext;
            routing_entry->RoutingNext = ImGuiKeyOwner_NoOwner;
            routing_entry->RoutingNextScore = 0;
            if (routing_entry->RoutingCurr == ImGuiKeyOwner_NoOwner)
                continue;
            rt->EntriesNext.push_back(*routing_entry);

            if (routing_entry->Mods == g.IO.KeyMods)
            {
                ImGuiKeyOwnerData* owner_data = GetKeyOwnerData(&g, key);
                if (owner_data->OwnerCurr == ImGuiKeyOwner_NoOwner)
                {
                    owner_data->OwnerCurr = routing_entry->RoutingCurr;

                }
            }
        }

        rt->Index[key - ImGuiKey_NamedKey_BEGIN] = (ImGuiKeyRoutingIndex)(new_routing_start_idx < rt->EntriesNext.Size ? new_routing_start_idx : -1);
        for (int n = new_routing_start_idx; n < rt->EntriesNext.Size; n++)
            rt->EntriesNext[n].NextEntryIndex = (ImGuiKeyRoutingIndex)((n + 1 < rt->EntriesNext.Size) ? n + 1 : -1);
    }
    rt->Entries.swap(rt->EntriesNext);
}

static inline ImGuiID GetRoutingIdFromOwnerId(ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    return (owner_id != ImGuiKeyOwner_NoOwner && owner_id != ImGuiKeyOwner_Any) ? owner_id : g.CurrentFocusScopeId;
}

ImGuiKeyRoutingData* ImGui::GetShortcutRoutingData(ImGuiKeyChord key_chord)
{

    ImGuiContext& g = *GImGui;
    ImGuiKeyRoutingTable* rt = &g.KeysRoutingTable;
    ImGuiKeyRoutingData* routing_data;
    ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    ImGuiKey mods = (ImGuiKey)(key_chord & ImGuiMod_Mask_);
    if (key == ImGuiKey_None)
        key = ConvertSingleModFlagToKey(mods);
    IM_ASSERT(IsNamedKey(key));

    for (ImGuiKeyRoutingIndex idx = rt->Index[key - ImGuiKey_NamedKey_BEGIN]; idx != -1; idx = routing_data->NextEntryIndex)
    {
        routing_data = &rt->Entries[idx];
        if (routing_data->Mods == mods)
            return routing_data;
    }

    ImGuiKeyRoutingIndex routing_data_idx = (ImGuiKeyRoutingIndex)rt->Entries.Size;
    rt->Entries.push_back(ImGuiKeyRoutingData());
    routing_data = &rt->Entries[routing_data_idx];
    routing_data->Mods = (ImU16)mods;
    routing_data->NextEntryIndex = rt->Index[key - ImGuiKey_NamedKey_BEGIN];
    rt->Index[key - ImGuiKey_NamedKey_BEGIN] = routing_data_idx;
    return routing_data;
}

static int CalcRoutingScore(ImGuiID focus_scope_id, ImGuiID owner_id, ImGuiInputFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (flags & ImGuiInputFlags_RouteFocused)
    {

        if (owner_id != 0 && g.ActiveId == owner_id)
            return 300;

        if (focus_scope_id == 0)
            return 0;
        for (int index_in_focus_path = 0; index_in_focus_path < g.NavFocusRoute.Size; index_in_focus_path++)
            if (g.NavFocusRoute.Data[index_in_focus_path].ID == focus_scope_id)
            {
                if (flags & ImGuiInputFlags_RouteOverActive)
                    return 599 - index_in_focus_path;
                else
                    return 199 - index_in_focus_path;
            }
        return 0;
    }
    else if (flags & ImGuiInputFlags_RouteActive)
    {
        if (owner_id != 0 && g.ActiveId == owner_id)
            return 300;
        return 0;
    }
    else if (flags & ImGuiInputFlags_RouteGlobal)
    {
        if (flags & ImGuiInputFlags_RouteOverActive)
            return 400;
        if (owner_id != 0 && g.ActiveId == owner_id)
            return 300;
        if (flags & ImGuiInputFlags_RouteOverFocused)
            return 200;
        return 1;
    }
    IM_ASSERT(0);
    return 0;
}

static bool IsKeyChordPotentiallyCharInput(ImGuiKeyChord key_chord)
{

    ImGuiContext& g = *GImGui;

    ImGuiKey mods = (ImGuiKey)(key_chord & ImGuiMod_Mask_);
    const bool ignore_char_inputs = ((mods & ImGuiMod_Ctrl) && !(mods & ImGuiMod_Alt)) || (g.IO.ConfigMacOSXBehaviors && (mods & ImGuiMod_Ctrl));
    if (ignore_char_inputs)
        return false;

    ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    if (key == ImGuiKey_None)
        return false;
    return g.KeysMayBeCharInput.TestBit(key);
}

bool ImGui::SetShortcutRouting(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    if ((flags & ImGuiInputFlags_RouteTypeMask_) == 0)
        flags |= ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteOverFocused | ImGuiInputFlags_RouteOverActive;
    else
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiInputFlags_RouteTypeMask_));
    IM_ASSERT(owner_id != ImGuiKeyOwner_Any && owner_id != ImGuiKeyOwner_NoOwner);
    if (flags & (ImGuiInputFlags_RouteOverFocused | ImGuiInputFlags_RouteUnlessBgFocused))
        IM_ASSERT(flags & ImGuiInputFlags_RouteGlobal);
    if (flags & ImGuiInputFlags_RouteOverActive)
        IM_ASSERT(flags & (ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteFocused));

    key_chord = FixupKeyChord(key_chord);

    if (g.DebugBreakInShortcutRouting == key_chord)
        IM_DEBUG_BREAK();

    if (flags & ImGuiInputFlags_RouteUnlessBgFocused)
        if (g.NavWindow == NULL)
            return false;

    if (flags & ImGuiInputFlags_RouteAlways)
    {
        IMGUI_DEBUG_LOG_INPUTROUTING("SetShortcutRouting(%s, flags=%04X, owner_id=0x%08X) -> always, no register\n", GetKeyChordName(key_chord), flags, owner_id);
        return true;
    }

    if (g.ActiveId != 0 && g.ActiveId != owner_id)
    {
        if (flags & ImGuiInputFlags_RouteActive)
            return false;

        if (g.IO.WantTextInput && IsKeyChordPotentiallyCharInput(key_chord))
        {
            IMGUI_DEBUG_LOG_INPUTROUTING("SetShortcutRouting(%s, flags=%04X, owner_id=0x%08X) -> filtered as potential char input\n", GetKeyChordName(key_chord), flags, owner_id);
            return false;
        }

        if ((flags & ImGuiInputFlags_RouteOverActive) == 0 && g.ActiveIdUsingAllKeyboardKeys)
        {
            ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
            if (key == ImGuiKey_None)
                key = ConvertSingleModFlagToKey((ImGuiKey)(key_chord & ImGuiMod_Mask_));
            if (key >= ImGuiKey_Keyboard_BEGIN && key < ImGuiKey_Keyboard_END)
                return false;
        }
    }

    ImGuiID focus_scope_id = g.CurrentFocusScopeId;
    if (flags & ImGuiInputFlags_RouteFromRootWindow)
        focus_scope_id = g.CurrentWindow->RootWindow->ID;

    const int score = CalcRoutingScore(focus_scope_id, owner_id, flags);
    IMGUI_DEBUG_LOG_INPUTROUTING("SetShortcutRouting(%s, flags=%04X, owner_id=0x%08X) -> score %d\n", GetKeyChordName(key_chord), flags, owner_id, score);
    if (score == 0)
        return false;

    ImGuiKeyRoutingData* routing_data = GetShortcutRoutingData(key_chord);

    if (score > routing_data->RoutingNextScore)
    {
        routing_data->RoutingNext = owner_id;
        routing_data->RoutingNextScore = (ImU16)score;
    }

    if (routing_data->RoutingCurr == owner_id)
        IMGUI_DEBUG_LOG_INPUTROUTING("--> granting current route\n");
    return routing_data->RoutingCurr == owner_id;
}

bool ImGui::TestShortcutRouting(ImGuiKeyChord key_chord, ImGuiID owner_id)
{
    const ImGuiID routing_id = GetRoutingIdFromOwnerId(owner_id);
    key_chord = FixupKeyChord(key_chord);
    ImGuiKeyRoutingData* routing_data = GetShortcutRoutingData(key_chord);
    return routing_data->RoutingCurr == routing_id;
}

bool ImGui::IsKeyDown(ImGuiKey key)
{
    return IsKeyDown(key, ImGuiKeyOwner_Any);
}

bool ImGui::IsKeyDown(ImGuiKey key, ImGuiID owner_id)
{
    const ImGuiKeyData* key_data = GetKeyData(key);
    if (!key_data->Down)
        return false;
    if (!TestKeyOwner(key, owner_id))
        return false;
    return true;
}

bool ImGui::IsKeyPressed(ImGuiKey key, bool repeat)
{
    return IsKeyPressed(key, repeat ? ImGuiInputFlags_Repeat : ImGuiInputFlags_None, ImGuiKeyOwner_Any);
}

bool ImGui::IsKeyPressed(ImGuiKey key, ImGuiInputFlags flags, ImGuiID owner_id)
{
    const ImGuiKeyData* key_data = GetKeyData(key);
    if (!key_data->Down)
        return false;
    const float t = key_data->DownDuration;
    if (t < 0.0f)
        return false;
    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedByIsKeyPressed) == 0);
    if (flags & (ImGuiInputFlags_RepeatRateMask_ | ImGuiInputFlags_RepeatUntilMask_))
        flags |= ImGuiInputFlags_Repeat;

    bool pressed = (t == 0.0f);
    if (!pressed && (flags & ImGuiInputFlags_Repeat) != 0)
    {
        float repeat_delay, repeat_rate;
        GetTypematicRepeatRate(flags, &repeat_delay, &repeat_rate);
        pressed = (t > repeat_delay) && GetKeyPressedAmount(key, repeat_delay, repeat_rate) > 0;
        if (pressed && (flags & ImGuiInputFlags_RepeatUntilMask_))
        {

            ImGuiContext& g = *GImGui;
            double key_pressed_time = g.Time - t + 0.00001f;
            if ((flags & ImGuiInputFlags_RepeatUntilKeyModsChange) && (g.LastKeyModsChangeTime > key_pressed_time))
                pressed = false;
            if ((flags & ImGuiInputFlags_RepeatUntilKeyModsChangeFromNone) && (g.LastKeyModsChangeFromNoneTime > key_pressed_time))
                pressed = false;
            if ((flags & ImGuiInputFlags_RepeatUntilOtherKeyPress) && (g.LastKeyboardKeyPressTime > key_pressed_time))
                pressed = false;
        }
    }
    if (!pressed)
        return false;
    if (!TestKeyOwner(key, owner_id))
        return false;
    return true;
}

bool ImGui::IsKeyReleased(ImGuiKey key)
{
    return IsKeyReleased(key, ImGuiKeyOwner_Any);
}

bool ImGui::IsKeyReleased(ImGuiKey key, ImGuiID owner_id)
{
    const ImGuiKeyData* key_data = GetKeyData(key);
    if (key_data->DownDurationPrev < 0.0f || key_data->Down)
        return false;
    if (!TestKeyOwner(key, owner_id))
        return false;
    return true;
}

bool ImGui::IsMouseDown(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseDown[button] && TestKeyOwner(MouseButtonToKey(button), ImGuiKeyOwner_Any);
}

bool ImGui::IsMouseDown(ImGuiMouseButton button, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseDown[button] && TestKeyOwner(MouseButtonToKey(button), owner_id);
}

bool ImGui::IsMouseClicked(ImGuiMouseButton button, bool repeat)
{
    return IsMouseClicked(button, repeat ? ImGuiInputFlags_Repeat : ImGuiInputFlags_None, ImGuiKeyOwner_Any);
}

bool ImGui::IsMouseClicked(ImGuiMouseButton button, ImGuiInputFlags flags, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    if (!g.IO.MouseDown[button])
        return false;
    const float t = g.IO.MouseDownDuration[button];
    if (t < 0.0f)
        return false;
    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedByIsMouseClicked) == 0);

    const bool repeat = (flags & ImGuiInputFlags_Repeat) != 0;
    const bool pressed = (t == 0.0f) || (repeat && t > g.IO.KeyRepeatDelay && CalcTypematicRepeatAmount(t - g.IO.DeltaTime, t, g.IO.KeyRepeatDelay, g.IO.KeyRepeatRate) > 0);
    if (!pressed)
        return false;

    if (!TestKeyOwner(MouseButtonToKey(button), owner_id))
        return false;

    return true;
}

bool ImGui::IsMouseReleased(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseReleased[button] && TestKeyOwner(MouseButtonToKey(button), ImGuiKeyOwner_Any);
}

bool ImGui::IsMouseReleased(ImGuiMouseButton button, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseReleased[button] && TestKeyOwner(MouseButtonToKey(button), owner_id);
}

bool ImGui::IsMouseReleasedWithDelay(ImGuiMouseButton button, float delay)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    const float time_since_release = (float)(g.Time - g.IO.MouseReleasedTime[button]);
    return !IsMouseDown(button) && (time_since_release - g.IO.DeltaTime < delay) && (time_since_release >= delay);
}

bool ImGui::IsMouseDoubleClicked(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseClickedCount[button] == 2 && TestKeyOwner(MouseButtonToKey(button), ImGuiKeyOwner_Any);
}

bool ImGui::IsMouseDoubleClicked(ImGuiMouseButton button, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseClickedCount[button] == 2 && TestKeyOwner(MouseButtonToKey(button), owner_id);
}

int ImGui::GetMouseClickedCount(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseClickedCount[button];
}

bool ImGui::IsMouseHoveringRect(const ImVec2& r_min, const ImVec2& r_max, bool clip)
{
    ImGuiContext& g = *GImGui;

    ImRect rect_clipped(r_min, r_max);
    if (clip)
        rect_clipped.ClipWith(g.CurrentWindow->ClipRect);

    if (!rect_clipped.ContainsWithPad(g.IO.MousePos, g.Style.TouchExtraPadding))
        return false;
    return true;
}

bool ImGui::IsMouseDragPastThreshold(ImGuiMouseButton button, float lock_threshold)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    if (lock_threshold < 0.0f)
        lock_threshold = g.IO.MouseDragThreshold;
    return g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold;
}

bool ImGui::IsMouseDragging(ImGuiMouseButton button, float lock_threshold)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    if (!g.IO.MouseDown[button])
        return false;
    return IsMouseDragPastThreshold(button, lock_threshold);
}

ImVec2 ImGui::GetMousePos()
{
    ImGuiContext& g = *GImGui;
    return g.IO.MousePos;
}

void ImGui::TeleportMousePos(const ImVec2& pos)
{
    ImGuiContext& g = *GImGui;
    g.IO.MousePos = g.IO.MousePosPrev = pos;
    g.IO.MouseDelta = ImVec2(0.0f, 0.0f);
    g.IO.WantSetMousePos = true;

}

ImVec2 ImGui::GetMousePosOnOpeningCurrentPopup()
{
    ImGuiContext& g = *GImGui;
    if (g.BeginPopupStack.Size > 0)
        return g.OpenPopupStack[g.BeginPopupStack.Size - 1].OpenMousePos;
    return g.IO.MousePos;
}

bool ImGui::IsMousePosValid(const ImVec2* mouse_pos)
{

    IM_ASSERT(GImGui != NULL);
    const float MOUSE_INVALID = -256000.0f;
    ImVec2 p = mouse_pos ? *mouse_pos : GImGui->IO.MousePos;
    return p.x >= MOUSE_INVALID && p.y >= MOUSE_INVALID;
}

bool ImGui::IsAnyMouseDown()
{
    ImGuiContext& g = *GImGui;
    for (int n = 0; n < IM_COUNTOF(g.IO.MouseDown); n++)
        if (g.IO.MouseDown[n])
            return true;
    return false;
}

ImVec2 ImGui::GetMouseDragDelta(ImGuiMouseButton button, float lock_threshold)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    if (lock_threshold < 0.0f)
        lock_threshold = g.IO.MouseDragThreshold;
    if (g.IO.MouseDown[button] || g.IO.MouseReleased[button])
        if (g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
            if (IsMousePosValid(&g.IO.MousePos) && IsMousePosValid(&g.IO.MouseClickedPos[button]))
                return g.IO.MousePos - g.IO.MouseClickedPos[button];
    return ImVec2(0.0f, 0.0f);
}

void ImGui::ResetMouseDragDelta(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));

    g.IO.MouseClickedPos[button] = g.IO.MousePos;
}

ImGuiMouseCursor ImGui::GetMouseCursor()
{
    ImGuiContext& g = *GImGui;
    return g.MouseCursor;
}

void ImGui::SetMouseCursor(ImGuiMouseCursor cursor_type)
{
    ImGuiContext& g = *GImGui;
    g.MouseCursor = cursor_type;
}

static void UpdateAliasKey(ImGuiKey key, bool v, float analog_value)
{
    IM_ASSERT(ImGui::IsAliasKey(key));
    ImGuiKeyData* key_data = ImGui::GetKeyData(key);
    key_data->Down = v;
    key_data->AnalogValue = analog_value;
}

static ImGuiKeyChord GetMergedModsFromKeys()
{

    ImGuiKeyChord mods = 0;
    if (ImGui::GetKeyData(ImGuiMod_Ctrl)->Down)     { mods |= ImGuiMod_Ctrl; }
    if (ImGui::GetKeyData(ImGuiMod_Shift)->Down)    { mods |= ImGuiMod_Shift; }
    if (ImGui::GetKeyData(ImGuiMod_Alt)->Down)      { mods |= ImGuiMod_Alt; }
    if (ImGui::GetKeyData(ImGuiMod_Super)->Down)    { mods |= ImGuiMod_Super; }
    return mods;
}

static void ImGui::UpdateKeyboardInputs()
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    if (io.ConfigFlags & ImGuiConfigFlags_NoKeyboard)
        io.ClearInputKeys();

    for (int n = 0; n < ImGuiMouseButton_COUNT; n++)
        UpdateAliasKey(MouseButtonToKey(n), io.MouseDown[n], io.MouseDown[n] ? 1.0f : 0.0f);
    UpdateAliasKey(ImGuiKey_MouseWheelX, io.MouseWheelH != 0.0f, io.MouseWheelH);
    UpdateAliasKey(ImGuiKey_MouseWheelY, io.MouseWheel != 0.0f, io.MouseWheel);

    const ImGuiKeyChord prev_key_mods = io.KeyMods;
    io.KeyMods = GetMergedModsFromKeys();
    io.KeyCtrl = (io.KeyMods & ImGuiMod_Ctrl) != 0;
    io.KeyShift = (io.KeyMods & ImGuiMod_Shift) != 0;
    io.KeyAlt = (io.KeyMods & ImGuiMod_Alt) != 0;
    io.KeySuper = (io.KeyMods & ImGuiMod_Super) != 0;
    if (prev_key_mods != io.KeyMods)
        g.LastKeyModsChangeTime = g.Time;
    if (prev_key_mods != io.KeyMods && prev_key_mods == 0)
        g.LastKeyModsChangeFromNoneTime = g.Time;

    if ((io.BackendFlags & ImGuiBackendFlags_HasGamepad) == 0)
        for (int key = ImGuiKey_Gamepad_BEGIN; key < ImGuiKey_Gamepad_END; key++)
        {
            io.KeysData[key - ImGuiKey_NamedKey_BEGIN].Down = false;
            io.KeysData[key - ImGuiKey_NamedKey_BEGIN].AnalogValue = 0.0f;
        }

    for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++)
    {
        ImGuiKeyData* key_data = &io.KeysData[key - ImGuiKey_NamedKey_BEGIN];
        key_data->DownDurationPrev = key_data->DownDuration;
        key_data->DownDuration = key_data->Down ? (key_data->DownDuration < 0.0f ? 0.0f : key_data->DownDuration + io.DeltaTime) : -1.0f;
        if (key_data->DownDuration == 0.0f)
        {
            if (IsKeyboardKey((ImGuiKey)key))
                g.LastKeyboardKeyPressTime = g.Time;
            else if (key == ImGuiKey_ReservedForModCtrl || key == ImGuiKey_ReservedForModShift || key == ImGuiKey_ReservedForModAlt || key == ImGuiKey_ReservedForModSuper)
                g.LastKeyboardKeyPressTime = g.Time;
        }
    }

    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
    {
        ImGuiKeyData* key_data = &io.KeysData[key - ImGuiKey_NamedKey_BEGIN];
        ImGuiKeyOwnerData* owner_data = &g.KeysOwnerData[key - ImGuiKey_NamedKey_BEGIN];
        owner_data->OwnerCurr = owner_data->OwnerNext;
        if (!key_data->Down)
            owner_data->OwnerNext = ImGuiKeyOwner_NoOwner;
        owner_data->LockThisFrame = owner_data->LockUntilRelease = owner_data->LockUntilRelease && key_data->Down;
    }

    UpdateKeyRoutingTable(&g.KeysRoutingTable);
}

static void ImGui::UpdateMouseInputs()
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    io.MouseWheelRequestAxisSwap = io.KeyShift && !io.ConfigMacOSXBehaviors;

    if (IsMousePosValid(&io.MousePos))
        io.MousePos = g.MouseLastValidPos = ImFloor(io.MousePos);

    if (IsMousePosValid(&io.MousePos) && IsMousePosValid(&io.MousePosPrev))
        io.MouseDelta = io.MousePos - io.MousePosPrev;
    else
        io.MouseDelta = ImVec2(0.0f, 0.0f);

    const float mouse_stationary_threshold = (io.MouseSource == ImGuiMouseSource_Mouse) ? 2.0f : 3.0f;
    const bool mouse_stationary = (ImLengthSqr(io.MouseDelta) <= mouse_stationary_threshold * mouse_stationary_threshold);
    g.MouseStationaryTimer = mouse_stationary ? (g.MouseStationaryTimer + io.DeltaTime) : 0.0f;

    if (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f)
        g.NavHighlightItemUnderNav = false;

    for (int i = 0; i < IM_COUNTOF(io.MouseDown); i++)
    {
        io.MouseClicked[i] = io.MouseDown[i] && io.MouseDownDuration[i] < 0.0f;
        io.MouseClickedCount[i] = 0;
        io.MouseReleased[i] = !io.MouseDown[i] && io.MouseDownDuration[i] >= 0.0f;
        if (io.MouseReleased[i])
            io.MouseReleasedTime[i] = g.Time;
        io.MouseDownDurationPrev[i] = io.MouseDownDuration[i];
        io.MouseDownDuration[i] = io.MouseDown[i] ? (io.MouseDownDuration[i] < 0.0f ? 0.0f : io.MouseDownDuration[i] + io.DeltaTime) : -1.0f;
        if (io.MouseClicked[i])
        {
            bool is_repeated_click = false;
            if ((float)(g.Time - io.MouseClickedTime[i]) < io.MouseDoubleClickTime)
            {
                ImVec2 delta_from_click_pos = IsMousePosValid(&io.MousePos) ? (io.MousePos - io.MouseClickedPos[i]) : ImVec2(0.0f, 0.0f);
                if (ImLengthSqr(delta_from_click_pos) < io.MouseDoubleClickMaxDist * io.MouseDoubleClickMaxDist)
                    is_repeated_click = true;
            }
            if (is_repeated_click)
                io.MouseClickedLastCount[i]++;
            else
                io.MouseClickedLastCount[i] = 1;
            io.MouseClickedTime[i] = g.Time;
            io.MouseClickedPos[i] = io.MousePos;
            io.MouseClickedCount[i] = io.MouseClickedLastCount[i];
            io.MouseDragMaxDistanceSqr[i] = 0.0f;
        }
        else if (io.MouseDown[i])
        {

            float delta_sqr_click_pos = IsMousePosValid(&io.MousePos) ? ImLengthSqr(io.MousePos - io.MouseClickedPos[i]) : 0.0f;
            io.MouseDragMaxDistanceSqr[i] = ImMax(io.MouseDragMaxDistanceSqr[i], delta_sqr_click_pos);
        }

        io.MouseDoubleClicked[i] = (io.MouseClickedCount[i] == 2);

        if (io.MouseClicked[i])
            g.NavHighlightItemUnderNav = false;
    }
}

static void LockWheelingWindow(ImGuiWindow* window, float wheel_amount)
{
    ImGuiContext& g = *GImGui;
    if (window)
        g.WheelingWindowReleaseTimer = ImMin(g.WheelingWindowReleaseTimer + ImAbs(wheel_amount) * WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER, WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER);
    else
        g.WheelingWindowReleaseTimer = 0.0f;
    if (g.WheelingWindow == window)
        return;
    IMGUI_DEBUG_LOG_IO("[io] LockWheelingWindow() \"%s\"\n", window ? window->Name : "NULL");
    g.WheelingWindow = window;
    g.WheelingWindowRefMousePos = g.IO.MousePos;
    if (window == NULL)
    {
        g.WheelingWindowStartFrame = -1;
        g.WheelingAxisAvg = ImVec2(0.0f, 0.0f);
    }
}

static ImGuiWindow* FindBestWheelingWindow(const ImVec2& wheel)
{

    ImGuiContext& g = *GImGui;
    ImGuiWindow* windows[2] = { NULL, NULL };
    for (int axis = 0; axis < 2; axis++)
        if (wheel[axis] != 0.0f)
            for (ImGuiWindow* window = windows[axis] = g.HoveredWindow; window->Flags & ImGuiWindowFlags_ChildWindow; window = windows[axis] = window->ParentWindow)
            {

                const bool has_scrolling = (window->ScrollMax[axis] != 0.0f);
                const bool inputs_disabled = (window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & ImGuiWindowFlags_NoMouseInputs);

                if (has_scrolling && !inputs_disabled)
                    break;
            }
    if (windows[0] == NULL && windows[1] == NULL)
        return NULL;

    if (windows[0] == windows[1] || windows[0] == NULL || windows[1] == NULL)
        return windows[1] ? windows[1] : windows[0];

    if (g.WheelingWindowStartFrame == -1)
        g.WheelingWindowStartFrame = g.FrameCount;
    if ((g.WheelingWindowStartFrame == g.FrameCount && wheel.x != 0.0f && wheel.y != 0.0f) || (g.WheelingAxisAvg.x == g.WheelingAxisAvg.y))
    {
        g.WheelingWindowWheelRemainder = wheel;
        return NULL;
    }
    return (g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? windows[0] : windows[1];
}

void ImGui::UpdateMouseWheel()
{

    ImGuiContext& g = *GImGui;
    if (g.WheelingWindow != NULL)
    {
        g.WheelingWindowReleaseTimer -= g.IO.DeltaTime;
        if (IsMousePosValid() && ImLengthSqr(g.IO.MousePos - g.WheelingWindowRefMousePos) > g.IO.MouseDragThreshold * g.IO.MouseDragThreshold)
            g.WheelingWindowReleaseTimer = 0.0f;
        if (g.WheelingWindowReleaseTimer <= 0.0f)
            LockWheelingWindow(NULL, 0.0f);
    }

    ImVec2 wheel;
    wheel.x = TestKeyOwner(ImGuiKey_MouseWheelX, ImGuiKeyOwner_NoOwner) ? g.IO.MouseWheelH : 0.0f;
    wheel.y = TestKeyOwner(ImGuiKey_MouseWheelY, ImGuiKeyOwner_NoOwner) ? g.IO.MouseWheel : 0.0f;

    ImGuiWindow* mouse_window = g.WheelingWindow ? g.WheelingWindow : g.HoveredWindow;
    if (!mouse_window || mouse_window->Collapsed)
        return;

    if (wheel.y != 0.0f && g.IO.KeyCtrl && g.IO.FontAllowUserScaling)
    {
        LockWheelingWindow(mouse_window, wheel.y);
        ImGuiWindow* window = mouse_window;
        const float new_font_scale = ImClamp(window->FontWindowScale + g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
        const float scale = new_font_scale / window->FontWindowScale;
        window->FontWindowScale = new_font_scale;
        if (window == window->RootWindow)
        {
            const ImVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
            SetWindowPos(window, window->Pos + offset, 0);
            window->Size = ImTrunc(window->Size * scale);
            window->SizeFull = ImTrunc(window->SizeFull * scale);
            MarkIniSettingsDirty(window);
        }
        return;
    }
    if (g.IO.KeyCtrl)
        return;

    if (g.IO.MouseWheelRequestAxisSwap)
        wheel = ImVec2(wheel.y, 0.0f);

    g.WheelingAxisAvg.x = ImExponentialMovingAverage(g.WheelingAxisAvg.x, ImAbs(wheel.x), 30);
    g.WheelingAxisAvg.y = ImExponentialMovingAverage(g.WheelingAxisAvg.y, ImAbs(wheel.y), 30);

    wheel += g.WheelingWindowWheelRemainder;
    g.WheelingWindowWheelRemainder = ImVec2(0.0f, 0.0f);
    if (wheel.x == 0.0f && wheel.y == 0.0f)
        return;

    if (ImGuiWindow* window = (g.WheelingWindow ? g.WheelingWindow : FindBestWheelingWindow(wheel)))
        if (!(window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & ImGuiWindowFlags_NoMouseInputs))
        {
            bool do_scroll[2] = { wheel.x != 0.0f && window->ScrollMax.x != 0.0f, wheel.y != 0.0f && window->ScrollMax.y != 0.0f };
            if (do_scroll[ImGuiAxis_X] && do_scroll[ImGuiAxis_Y])
                do_scroll[(g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? ImGuiAxis_Y : ImGuiAxis_X] = false;
            if (do_scroll[ImGuiAxis_X])
            {
                LockWheelingWindow(window, wheel.x);
                float max_step = window->InnerRect.GetWidth() * 0.67f;
                float scroll_step = ImTrunc(ImMin(2 * window->FontRefSize, max_step));
                SetScrollX(window, window->Scroll.x - wheel.x * scroll_step);
                g.WheelingWindowScrolledFrame = g.FrameCount;
            }
            if (do_scroll[ImGuiAxis_Y])
            {
                LockWheelingWindow(window, wheel.y);
                float max_step = window->InnerRect.GetHeight() * 0.67f;
                float scroll_step = ImTrunc(ImMin(5 * window->FontRefSize, max_step));
                SetScrollY(window, window->Scroll.y - wheel.y * scroll_step);
                g.WheelingWindowScrolledFrame = g.FrameCount;
            }
        }
}

void ImGui::SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard)
{
    ImGuiContext& g = *GImGui;
    g.WantCaptureKeyboardNextFrame = want_capture_keyboard ? 1 : 0;
}

void ImGui::SetNextFrameWantCaptureMouse(bool want_capture_mouse)
{
    ImGuiContext& g = *GImGui;
    g.WantCaptureMouseNextFrame = want_capture_mouse ? 1 : 0;
}

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
static const char* GetInputSourceName(ImGuiInputSource source)
{
    const char* input_source_names[] = { "None", "Mouse", "Keyboard", "Gamepad" };
    IM_ASSERT(IM_COUNTOF(input_source_names) == ImGuiInputSource_COUNT);
    if (source < 0 || source >= ImGuiInputSource_COUNT)
        return "Unknown";
    return input_source_names[source];
}
static const char* GetMouseSourceName(ImGuiMouseSource source)
{
    const char* mouse_source_names[] = { "Mouse", "TouchScreen", "Pen" };
    IM_ASSERT(IM_COUNTOF(mouse_source_names) == ImGuiMouseSource_COUNT);
    if (source < 0 || source >= ImGuiMouseSource_COUNT)
        return "Unknown";
    return mouse_source_names[source];
}
static void DebugPrintInputEvent(const char* prefix, const ImGuiInputEvent* e)
{
    ImGuiContext& g = *GImGui;
    char buf[5];
    if (e->Type == ImGuiInputEventType_MousePos)    { if (e->MousePos.PosX == -FLT_MAX && e->MousePos.PosY == -FLT_MAX) IMGUI_DEBUG_LOG_IO("[io] %s: MousePos (-FLT_MAX, -FLT_MAX)\n", prefix); else IMGUI_DEBUG_LOG_IO("[io] %s: MousePos (%.1f, %.1f) (%s)\n", prefix, e->MousePos.PosX, e->MousePos.PosY, GetMouseSourceName(e->MousePos.MouseSource)); return; }
    if (e->Type == ImGuiInputEventType_MouseButton) { IMGUI_DEBUG_LOG_IO("[io] %s: MouseButton %d %s (%s)\n", prefix, e->MouseButton.Button, e->MouseButton.Down ? "Down" : "Up", GetMouseSourceName(e->MouseButton.MouseSource)); return; }
    if (e->Type == ImGuiInputEventType_MouseWheel)  { IMGUI_DEBUG_LOG_IO("[io] %s: MouseWheel (%.3f, %.3f) (%s)\n", prefix, e->MouseWheel.WheelX, e->MouseWheel.WheelY, GetMouseSourceName(e->MouseWheel.MouseSource)); return; }
    if (e->Type == ImGuiInputEventType_Key)         { IMGUI_DEBUG_LOG_IO("[io] %s: Key \"%s\" %s\n", prefix, ImGui::GetKeyName(e->Key.Key), e->Key.Down ? "Down" : "Up"); return; }
    if (e->Type == ImGuiInputEventType_Text)        { ImTextCharToUtf8(buf, e->Text.Char); IMGUI_DEBUG_LOG_IO("[io] %s: Text: '%s' (U+%08X)\n", prefix, buf, e->Text.Char); return; }
    if (e->Type == ImGuiInputEventType_Focus)       { IMGUI_DEBUG_LOG_IO("[io] %s: AppFocused %d\n", prefix, e->AppFocused.Focused); return; }
}
#endif

void ImGui::UpdateInputEvents(bool trickle_fast_inputs)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    const bool trickle_interleaved_nonchar_keys_and_text = (trickle_fast_inputs && g.WantTextInputNextFrame == 1);

    bool mouse_moved = false, mouse_wheeled = false, key_changed = false, key_changed_nonchar = false, text_inputted = false;
    int  mouse_button_changed = 0x00;
    ImBitArray<ImGuiKey_NamedKey_COUNT> key_changed_mask;

    int event_n = 0;
    for (; event_n < g.InputEventsQueue.Size; event_n++)
    {
        ImGuiInputEvent* e = &g.InputEventsQueue[event_n];
        if (e->Type == ImGuiInputEventType_MousePos)
        {
            if (g.IO.WantSetMousePos)
                continue;

            ImVec2 event_pos(e->MousePos.PosX, e->MousePos.PosY);
            if (trickle_fast_inputs && (mouse_button_changed != 0 || mouse_wheeled || key_changed || text_inputted))
                break;
            io.MousePos = event_pos;
            io.MouseSource = e->MousePos.MouseSource;
            mouse_moved = true;
        }
        else if (e->Type == ImGuiInputEventType_MouseButton)
        {

            const ImGuiMouseButton button = e->MouseButton.Button;
            IM_ASSERT(button >= 0 && button < ImGuiMouseButton_COUNT);
            if (trickle_fast_inputs && ((mouse_button_changed & (1 << button)) || mouse_wheeled))
                break;
            if (trickle_fast_inputs && e->MouseButton.MouseSource == ImGuiMouseSource_TouchScreen && mouse_moved)
                break;
            io.MouseDown[button] = e->MouseButton.Down;
            io.MouseSource = e->MouseButton.MouseSource;
            mouse_button_changed |= (1 << button);
        }
        else if (e->Type == ImGuiInputEventType_MouseWheel)
        {

            if (trickle_fast_inputs && (mouse_moved || mouse_button_changed != 0))
                break;
            io.MouseWheelH += e->MouseWheel.WheelX;
            io.MouseWheel += e->MouseWheel.WheelY;
            io.MouseSource = e->MouseWheel.MouseSource;
            mouse_wheeled = true;
        }
        else if (e->Type == ImGuiInputEventType_Key)
        {

            if (io.ConfigFlags & ImGuiConfigFlags_NoKeyboard)
                continue;
            ImGuiKey key = e->Key.Key;
            IM_ASSERT(key != ImGuiKey_None);
            ImGuiKeyData* key_data = GetKeyData(key);
            const int key_data_index = (int)(key_data - g.IO.KeysData);
            if (trickle_fast_inputs && key_data->Down != e->Key.Down && (key_changed_mask.TestBit(key_data_index) || mouse_button_changed != 0))
                break;

            const bool key_is_potentially_for_char_input = IsKeyChordPotentiallyCharInput(GetMergedModsFromKeys() | key);
            if (trickle_interleaved_nonchar_keys_and_text && (text_inputted && !key_is_potentially_for_char_input))
                break;

            if (key_data->Down != e->Key.Down)
            {
                key_changed = true;
                key_changed_mask.SetBit(key_data_index);
                if (trickle_interleaved_nonchar_keys_and_text && !key_is_potentially_for_char_input)
                    key_changed_nonchar = true;
            }

            key_data->Down = e->Key.Down;
            key_data->AnalogValue = e->Key.AnalogValue;
        }
        else if (e->Type == ImGuiInputEventType_Text)
        {
            if (io.ConfigFlags & ImGuiConfigFlags_NoKeyboard)
                continue;

            if (trickle_fast_inputs && (mouse_button_changed != 0 || mouse_moved || mouse_wheeled))
                break;
            if (trickle_interleaved_nonchar_keys_and_text && key_changed_nonchar)
                break;
            unsigned int c = e->Text.Char;
            io.InputQueueCharacters.push_back(c <= IM_UNICODE_CODEPOINT_MAX ? (ImWchar)c : IM_UNICODE_CODEPOINT_INVALID);
            if (trickle_interleaved_nonchar_keys_and_text)
                text_inputted = true;
        }
        else if (e->Type == ImGuiInputEventType_Focus)
        {

            const bool focus_lost = !e->AppFocused.Focused;
            io.AppFocusLost = focus_lost;
        }
        else
        {
            IM_ASSERT(0 && "Unknown event!");
        }
    }

    for (int n = 0; n < event_n; n++)
        g.InputEventsTrail.push_back(g.InputEventsQueue[n]);

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (event_n != 0 && (g.DebugLogFlags & ImGuiDebugLogFlags_EventIO))
        for (int n = 0; n < g.InputEventsQueue.Size; n++)
            DebugPrintInputEvent(n < event_n ? "Processed" : "Remaining", &g.InputEventsQueue[n]);
#endif

    if (event_n == g.InputEventsQueue.Size)
        g.InputEventsQueue.resize(0);
    else
        g.InputEventsQueue.erase(g.InputEventsQueue.Data, g.InputEventsQueue.Data + event_n);

    if (g.IO.AppFocusLost)
    {
        g.IO.ClearInputKeys();
        g.IO.ClearInputMouse();
    }
}

ImGuiID ImGui::GetKeyOwner(ImGuiKey key)
{
    if (!IsNamedKeyOrMod(key))
        return ImGuiKeyOwner_NoOwner;

    ImGuiContext& g = *GImGui;
    ImGuiKeyOwnerData* owner_data = GetKeyOwnerData(&g, key);
    ImGuiID owner_id = owner_data->OwnerCurr;

    if (g.ActiveIdUsingAllKeyboardKeys && owner_id != g.ActiveId && owner_id != ImGuiKeyOwner_Any)
        if (key >= ImGuiKey_Keyboard_BEGIN && key < ImGuiKey_Keyboard_END)
            return ImGuiKeyOwner_NoOwner;

    return owner_id;
}

bool ImGui::TestKeyOwner(ImGuiKey key, ImGuiID owner_id)
{
    if (!IsNamedKeyOrMod(key))
        return true;

    ImGuiContext& g = *GImGui;
    if (g.ActiveIdUsingAllKeyboardKeys && owner_id != g.ActiveId && owner_id != ImGuiKeyOwner_Any)
        if (key >= ImGuiKey_Keyboard_BEGIN && key < ImGuiKey_Keyboard_END)
            return false;

    ImGuiKeyOwnerData* owner_data = GetKeyOwnerData(&g, key);
    if (owner_id == ImGuiKeyOwner_Any)
        return owner_data->LockThisFrame == false;

    if (owner_data->OwnerCurr != owner_id)
    {
        if (owner_data->LockThisFrame)
            return false;
        if (owner_data->OwnerCurr != ImGuiKeyOwner_NoOwner)
            return false;
    }

    return true;
}

void ImGui::SetKeyOwner(ImGuiKey key, ImGuiID owner_id, ImGuiInputFlags flags)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(IsNamedKeyOrMod(key) && (owner_id != ImGuiKeyOwner_Any || (flags & (ImGuiInputFlags_LockThisFrame | ImGuiInputFlags_LockUntilRelease))));
    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedBySetKeyOwner) == 0);

    ImGuiKeyOwnerData* owner_data = GetKeyOwnerData(&g, key);
    owner_data->OwnerCurr = owner_data->OwnerNext = owner_id;

    owner_data->LockUntilRelease = (flags & ImGuiInputFlags_LockUntilRelease) != 0;
    owner_data->LockThisFrame = (flags & ImGuiInputFlags_LockThisFrame) != 0 || (owner_data->LockUntilRelease);
}

void ImGui::SetKeyOwnersForKeyChord(ImGuiKeyChord key_chord, ImGuiID owner_id, ImGuiInputFlags flags)
{
    if (key_chord & ImGuiMod_Ctrl)      { SetKeyOwner(ImGuiMod_Ctrl, owner_id, flags); }
    if (key_chord & ImGuiMod_Shift)     { SetKeyOwner(ImGuiMod_Shift, owner_id, flags); }
    if (key_chord & ImGuiMod_Alt)       { SetKeyOwner(ImGuiMod_Alt, owner_id, flags); }
    if (key_chord & ImGuiMod_Super)     { SetKeyOwner(ImGuiMod_Super, owner_id, flags); }
    if (key_chord & ~ImGuiMod_Mask_)    { SetKeyOwner((ImGuiKey)(key_chord & ~ImGuiMod_Mask_), owner_id, flags); }
}

void ImGui::SetItemKeyOwner(ImGuiKey key, ImGuiInputFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = g.LastItemData.ID;
    if (id == 0 || (g.HoveredId != id && g.ActiveId != id))
        return;
    if ((flags & ImGuiInputFlags_CondMask_) == 0)
        flags |= ImGuiInputFlags_CondDefault_;
    if ((g.HoveredId == id && (flags & ImGuiInputFlags_CondHovered)) || (g.ActiveId == id && (flags & ImGuiInputFlags_CondActive)))
    {
        IM_ASSERT((flags & ~ImGuiInputFlags_SupportedBySetItemKeyOwner) == 0);
        SetKeyOwner(key, id, flags & ~ImGuiInputFlags_CondMask_);
    }
}

void ImGui::SetItemKeyOwner(ImGuiKey key)
{
    SetItemKeyOwner(key, ImGuiInputFlags_None);
}

bool ImGui::IsKeyChordPressed(ImGuiKeyChord key_chord)
{
    return IsKeyChordPressed(key_chord, ImGuiInputFlags_None, ImGuiKeyOwner_Any);
}

bool ImGui::IsKeyChordPressed(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    key_chord = FixupKeyChord(key_chord);
    ImGuiKey mods = (ImGuiKey)(key_chord & ImGuiMod_Mask_);
    if (g.IO.KeyMods != mods)
        return false;

    ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    if (key == ImGuiKey_None)
        key = ConvertSingleModFlagToKey(mods);
    if (!IsKeyPressed(key, (flags & ImGuiInputFlags_RepeatMask_), owner_id))
        return false;
    return true;
}

void ImGui::SetNextItemShortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags)
{
    ImGuiContext& g = *GImGui;
    g.NextItemData.HasFlags |= ImGuiNextItemDataFlags_HasShortcut;
    g.NextItemData.Shortcut = key_chord;
    g.NextItemData.ShortcutFlags = flags;
}

void ImGui::ItemHandleShortcut(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiInputFlags flags = g.NextItemData.ShortcutFlags;
    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedBySetNextItemShortcut) == 0);

    if (g.LastItemData.ItemFlags & ImGuiItemFlags_Disabled)
        return;
    if (flags & ImGuiInputFlags_Tooltip)
    {
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasShortcut;
        g.LastItemData.Shortcut = g.NextItemData.Shortcut;
    }
    if (!Shortcut(g.NextItemData.Shortcut, flags & ImGuiInputFlags_SupportedByShortcut, id) || g.NavActivateId != 0)
        return;

    g.NavActivateId = id;
    g.NavActivateFlags = ImGuiActivateFlags_PreferInput | ImGuiActivateFlags_FromShortcut;

    g.NavActivateDownId = g.NavActivatePressedId = id;
    NavHighlightActivated(id);
}

bool ImGui::Shortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags)
{
    return Shortcut(key_chord, flags, ImGuiKeyOwner_Any);
}

bool ImGui::Shortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;

    if ((flags & ImGuiInputFlags_RouteTypeMask_) == 0)
        flags |= ImGuiInputFlags_RouteFocused;

    if (owner_id == ImGuiKeyOwner_Any || owner_id == ImGuiKeyOwner_NoOwner)
        owner_id = GetRoutingIdFromOwnerId(owner_id);

    if (g.CurrentItemFlags & ImGuiItemFlags_Disabled)
        return false;

    if (!SetShortcutRouting(key_chord, flags, owner_id))
        return false;

    if ((flags & ImGuiInputFlags_Repeat) != 0 && (flags & ImGuiInputFlags_RepeatUntilMask_) == 0)
        flags |= ImGuiInputFlags_RepeatUntilKeyModsChange;

    if (!IsKeyChordPressed(key_chord, flags, owner_id))
        return false;

    SetKeyOwnersForKeyChord(key_chord & ImGuiMod_Mask_, owner_id);

    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedByShortcut) == 0);
    return true;
}

bool ImGui::DebugCheckVersionAndDataLayout(const char* version, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx)
{
    bool error = false;
    if (strcmp(version, IMGUI_VERSION) != 0) { error = true; IM_ASSERT(strcmp(version, IMGUI_VERSION) == 0 && "Mismatched version string!"); }
    if (sz_io    != sizeof(ImGuiIO))    { error = true; IM_ASSERT(sz_io == sizeof(ImGuiIO) && "Mismatched struct layout!"); }
    if (sz_style != sizeof(ImGuiStyle)) { error = true; IM_ASSERT(sz_style == sizeof(ImGuiStyle) && "Mismatched struct layout!"); }
    if (sz_vec2  != sizeof(ImVec2))     { error = true; IM_ASSERT(sz_vec2 == sizeof(ImVec2) && "Mismatched struct layout!"); }
    if (sz_vec4  != sizeof(ImVec4))     { error = true; IM_ASSERT(sz_vec4 == sizeof(ImVec4) && "Mismatched struct layout!"); }
    if (sz_vert  != sizeof(ImDrawVert)) { error = true; IM_ASSERT(sz_vert == sizeof(ImDrawVert) && "Mismatched struct layout!"); }
    if (sz_idx   != sizeof(ImDrawIdx))  { error = true; IM_ASSERT(sz_idx == sizeof(ImDrawIdx) && "Mismatched struct layout!"); }
    return !error;
}

void ImGui::ErrorCheckUsingSetCursorPosToExtendParentBoundaries()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(window->DC.IsSetPos);
    window->DC.IsSetPos = false;
    if (window->DC.CursorPos.x <= window->DC.CursorMaxPos.x && window->DC.CursorPos.y <= window->DC.CursorMaxPos.y)
        return;
    if (window->SkipItems)
        return;
    IM_ASSERT_USER_ERROR(0, "Code uses SetCursorPos()/SetCursorScreenPos() to extend window/parent boundaries.\nPlease submit an item e.g. Dummy() afterwards in order to grow window/parent boundaries.");

}

static void ImGui::ErrorCheckNewFrameSanityChecks()
{
    ImGuiContext& g = *GImGui;

    if (true) IM_ASSERT(1); else IM_ASSERT(0);

#ifdef __EMSCRIPTEN__
    if (g.IO.DeltaTime <= 0.0f && g.FrameCount > 0)
        g.IO.DeltaTime = 0.00001f;
#endif

    IM_ASSERT(g.Initialized);
    IM_ASSERT((g.IO.DeltaTime > 0.0f || g.FrameCount == 0)              && "Need a positive DeltaTime!");
    IM_ASSERT((g.FrameCount == 0 || g.FrameCountEnded == g.FrameCount)  && "Forgot to call Render() or EndFrame() at the end of the previous frame?");
    IM_ASSERT(g.IO.DisplaySize.x >= 0.0f && g.IO.DisplaySize.y >= 0.0f  && "Invalid DisplaySize value!");
    IM_ASSERT(g.Style.CurveTessellationTol > 0.0f                       && "Invalid style setting!");
    IM_ASSERT(g.Style.CircleTessellationMaxError > 0.0f                 && "Invalid style setting!");
    IM_ASSERT(g.Style.Alpha >= 0.0f && g.Style.Alpha <= 1.0f            && "Invalid style setting!");
    IM_ASSERT(g.Style.WindowMinSize.x >= 1.0f && g.Style.WindowMinSize.y >= 1.0f && "Invalid style setting!");
    IM_ASSERT(g.Style.WindowBorderHoverPadding > 0.0f                   && "Invalid style setting!");
    IM_ASSERT(g.Style.WindowMenuButtonPosition == ImGuiDir_None || g.Style.WindowMenuButtonPosition == ImGuiDir_Left || g.Style.WindowMenuButtonPosition == ImGuiDir_Right);
    IM_ASSERT(g.Style.ColorButtonPosition == ImGuiDir_Left || g.Style.ColorButtonPosition == ImGuiDir_Right);
    IM_ASSERT(g.Style.TreeLinesFlags == ImGuiTreeNodeFlags_DrawLinesNone || g.Style.TreeLinesFlags == ImGuiTreeNodeFlags_DrawLinesFull || g.Style.TreeLinesFlags == ImGuiTreeNodeFlags_DrawLinesToNodes);

    if (g.IO.ConfigErrorRecovery)
        IM_ASSERT(g.IO.ConfigErrorRecoveryEnableAssert || g.IO.ConfigErrorRecoveryEnableDebugLog || g.IO.ConfigErrorRecoveryEnableTooltip || g.ErrorCallback != NULL);

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    if (g.IO.FontGlobalScale > 1.0f)
        IM_ASSERT(g.Style.FontScaleMain == 1.0f && "Since 1.92: use style.FontScaleMain instead of g.IO.FontGlobalScale!");

    if (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos)
    {
        g.IO.ConfigNavMoveSetMousePos = true;
        g.IO.ConfigFlags &= ~ImGuiConfigFlags_NavEnableSetMousePos;
    }
    if (g.IO.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard)
    {
        g.IO.ConfigNavCaptureKeyboard = false;
        g.IO.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;
    }

    if (g.IO.GetClipboardTextFn != NULL && (g.PlatformIO.Platform_GetClipboardTextFn == NULL || g.PlatformIO.Platform_GetClipboardTextFn == Platform_GetClipboardTextFn_DefaultImpl))
        g.PlatformIO.Platform_GetClipboardTextFn = [](ImGuiContext* ctx) { return ctx->IO.GetClipboardTextFn(ctx->IO.ClipboardUserData); };
    if (g.IO.SetClipboardTextFn != NULL && (g.PlatformIO.Platform_SetClipboardTextFn == NULL || g.PlatformIO.Platform_SetClipboardTextFn == Platform_SetClipboardTextFn_DefaultImpl))
        g.PlatformIO.Platform_SetClipboardTextFn = [](ImGuiContext* ctx, const char* text) { return ctx->IO.SetClipboardTextFn(ctx->IO.ClipboardUserData, text); };
#endif
}

static void ImGui::ErrorCheckEndFrameSanityChecks()
{

    ImGuiContext& g = *GImGui;
    const ImGuiKeyChord key_mods = GetMergedModsFromKeys();
    IM_UNUSED(g);
    IM_UNUSED(key_mods);
    IM_ASSERT((key_mods == 0 || g.IO.KeyMods == key_mods) && "Mismatching io.KeyCtrl/io.KeyShift/io.KeyAlt/io.KeySuper vs io.KeyMods");
    IM_UNUSED(key_mods);

    IM_ASSERT(g.CurrentWindowStack.Size == 1);
    IM_ASSERT(g.CurrentWindowStack[0].Window->IsFallbackWindow);
}

void ImGui::ErrorRecoveryStoreState(ImGuiErrorRecoveryState* state_out)
{
    ImGuiContext& g = *GImGui;
    state_out->SizeOfWindowStack = (short)g.CurrentWindowStack.Size;
    state_out->SizeOfIDStack = (short)g.CurrentWindow->IDStack.Size;
    state_out->SizeOfTreeStack = (short)g.CurrentWindow->DC.TreeDepth;
    state_out->SizeOfColorStack = (short)g.ColorStack.Size;
    state_out->SizeOfStyleVarStack = (short)g.StyleVarStack.Size;
    state_out->SizeOfFontStack = (short)g.FontStack.Size;
    state_out->SizeOfFocusScopeStack = (short)g.FocusScopeStack.Size;
    state_out->SizeOfGroupStack = (short)g.GroupStack.Size;
    state_out->SizeOfItemFlagsStack = (short)g.ItemFlagsStack.Size;
    state_out->SizeOfBeginPopupStack = (short)g.BeginPopupStack.Size;
    state_out->SizeOfDisabledStack = (short)g.DisabledStackSize;
}

void ImGui::ErrorRecoveryTryToRecoverState(const ImGuiErrorRecoveryState* state_in)
{

    ImGuiContext& g = *GImGui;
    while (g.CurrentWindowStack.Size > state_in->SizeOfWindowStack)
    {

        ImGuiWindow* window = g.CurrentWindow;
        if (window->Flags & ImGuiWindowFlags_ChildWindow)
        {
            if (g.CurrentTable != NULL && g.CurrentTable->InnerWindow == g.CurrentWindow)
            {
                IM_ASSERT_USER_ERROR(0, "Missing EndTable()");
                EndTable();
            }
            else
            {
                IM_ASSERT_USER_ERROR(0, "Missing EndChild()");
                EndChild();
            }
        }
        else
        {
            IM_ASSERT_USER_ERROR(0, "Missing End()");
            End();
        }
    }
    if (g.CurrentWindowStack.Size == state_in->SizeOfWindowStack)
        ErrorRecoveryTryToRecoverWindowState(state_in);
}

void    ImGui::ErrorRecoveryTryToRecoverWindowState(const ImGuiErrorRecoveryState* state_in)
{
    ImGuiContext& g = *GImGui;

    while (g.CurrentTable != NULL && g.CurrentTable->InnerWindow == g.CurrentWindow)
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndTable()");
        EndTable();
    }

    ImGuiWindow* window = g.CurrentWindow;

    while (g.CurrentTabBar != NULL && g.CurrentTabBar->Window == window)
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndTabBar()");
        EndTabBar();
    }
    while (g.CurrentMultiSelect != NULL && g.CurrentMultiSelect->Storage->Window == window)
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndMultiSelect()");
        EndMultiSelect();
    }
    if (window->DC.MenuBarAppending)
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndMenuBar()");
        EndMenuBar();
    }
    while (window->DC.TreeDepth > state_in->SizeOfTreeStack)
    {
        IM_ASSERT_USER_ERROR(0, "Missing TreePop()");
        TreePop();
    }
    while (g.GroupStack.Size > state_in->SizeOfGroupStack)
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndGroup()");
        EndGroup();
    }
    IM_ASSERT(g.GroupStack.Size == state_in->SizeOfGroupStack);
    while (window->IDStack.Size > state_in->SizeOfIDStack)
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopID()");
        PopID();
    }
    while (g.DisabledStackSize > state_in->SizeOfDisabledStack)
    {
        IM_ASSERT_USER_ERROR(0, "Missing EndDisabled()");
        if (g.CurrentItemFlags & ImGuiItemFlags_Disabled)
            EndDisabled();
        else
        {
            EndDisabledOverrideReenable();
            g.CurrentWindowStack.back().DisabledOverrideReenable = false;
        }
    }
    IM_ASSERT(g.DisabledStackSize == state_in->SizeOfDisabledStack);
    while (g.ColorStack.Size > state_in->SizeOfColorStack)
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopStyleColor()");
        PopStyleColor();
    }
    while (g.ItemFlagsStack.Size > state_in->SizeOfItemFlagsStack)
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopItemFlag()");
        PopItemFlag();
    }
    while (g.StyleVarStack.Size > state_in->SizeOfStyleVarStack)
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopStyleVar()");
        PopStyleVar();
    }
    while (g.FontStack.Size > state_in->SizeOfFontStack)
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopFont()");
        PopFont();
    }
    while (g.FocusScopeStack.Size > state_in->SizeOfFocusScopeStack)
    {
        IM_ASSERT_USER_ERROR(0, "Missing PopFocusScope()");
        PopFocusScope();
    }

}

bool    ImGui::ErrorLog(const char* msg)
{
    ImGuiContext& g = *GImGui;

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiWindow* window = g.CurrentWindow;

    if (g.IO.ConfigErrorRecoveryEnableDebugLog)
    {
        if (g.ErrorFirst)
            IMGUI_DEBUG_LOG_ERROR("[imgui-error] (current settings: Assert=%d, Log=%d, Tooltip=%d)\n",
                g.IO.ConfigErrorRecoveryEnableAssert, g.IO.ConfigErrorRecoveryEnableDebugLog, g.IO.ConfigErrorRecoveryEnableTooltip);
        IMGUI_DEBUG_LOG_ERROR("[imgui-error] In window '%s': %s\n", window ? window->Name : "NULL", msg);
    }
    g.ErrorFirst = false;

    if (g.IO.ConfigErrorRecoveryEnableTooltip)
    {
        if (g.WithinFrameScope && BeginErrorTooltip())
        {
            if (g.ErrorCountCurrentFrame < 20)
            {
                Text("In window '%s': %s", window ? window->Name : "NULL", msg);
                if (window && (!window->IsFallbackWindow || window->WasActive))
                    GetForegroundDrawList(window)->AddRect(window->Pos, window->Pos + window->Size, IM_COL32(255, 0, 0, 255));
            }
            if (g.ErrorCountCurrentFrame == 20)
                Text("(and more errors)");

            EndErrorTooltip();
        }
        g.ErrorCountCurrentFrame++;
    }
#endif

    if (g.ErrorCallback != NULL)
        g.ErrorCallback(&g, g.ErrorCallbackUserData, msg);

    return g.IO.ConfigErrorRecoveryEnableAssert;
}

void ImGui::ErrorCheckEndFrameFinalizeErrorTooltip()
{
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    ImGuiContext& g = *GImGui;
    if (g.DebugDrawIdConflictsId != 0 && g.IO.KeyCtrl == false)
        g.DebugDrawIdConflictsCount = g.HoveredIdPreviousFrameItemCount;
    if (g.DebugDrawIdConflictsId != 0 && g.DebugItemPickerActive == false && BeginErrorTooltip())
    {
        Text("Programmer error: %d visible items with conflicting ID!", g.DebugDrawIdConflictsCount);
        BulletText("Code should use PushID()/PopID() in loops, or append \"##xx\" to same-label identifiers!");
        BulletText("Empty label e.g. Button(\"\") == same ID as parent widget/node. Use Button(\"##xx\") instead!");

        BulletText("Set io.ConfigDebugHighlightIdConflicts=false to disable this warning in non-programmers builds.");
        Separator();
        if (g.IO.ConfigDebugHighlightIdConflictsShowItemPicker)
        {
            Text("(Hold Ctrl to: use ");
            SameLine(0.0f, 0.0f);
            if (SmallButton("Item Picker"))
                DebugStartItemPicker();
            SameLine(0.0f, 0.0f);
            Text(" to break in item call-stack, or ");
        }
        else
        {
            Text("(Hold Ctrl to: ");
        }
        SameLine(0.0f, 0.0f);
        TextLinkOpenURL("read FAQ \"About ID Stack System\"", "https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#qa-usage");
        SameLine(0.0f, 0.0f);
        Text(")");
        EndErrorTooltip();
    }

    if (g.ErrorCountCurrentFrame > 0 && BeginErrorTooltip())
    {
        Separator();
        Text("(Hold Ctrl to: ");
        SameLine(0.0f, 0.0f);
        if (SmallButton("Enable Asserts"))
            g.IO.ConfigErrorRecoveryEnableAssert = true;

        SameLine(0, 0);
        Text(")");
        EndErrorTooltip();
    }
#endif
}

bool ImGui::BeginErrorTooltip()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = FindWindowByName("##Tooltip_Error");
    const bool use_locked_pos = (g.IO.KeyCtrl && window && window->WasActive);
    PushStyleColor(ImGuiCol_PopupBg, ImLerp(g.Style.Colors[ImGuiCol_PopupBg], ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 0.15f));
    if (use_locked_pos)
        SetNextWindowPos(g.ErrorTooltipLockedPos);
    bool is_visible = Begin("##Tooltip_Error", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
    PopStyleColor();
    if (is_visible && g.CurrentWindow->BeginCount == 1)
    {
        SeparatorText("MESSAGE FROM DEAR IMGUI");
        BringWindowToDisplayFront(g.CurrentWindow);
        BringWindowToFocusFront(g.CurrentWindow);
        g.ErrorTooltipLockedPos = GetWindowPos();
    }
    else if (!is_visible)
    {
        End();
    }
    return is_visible;
}

void ImGui::EndErrorTooltip()
{
    End();
}

void ImGui::KeepAliveID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId == id)
        g.ActiveIdIsAlive = id;
    if (g.DeactivatedItemData.ID == id)
        g.DeactivatedItemData.IsAlive = true;
}

IM_MSVC_RUNTIME_CHECKS_OFF
bool ImGui::ItemAdd(const ImRect& bb, ImGuiID id, const ImRect* nav_bb_arg, ImGuiItemFlags extra_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    g.LastItemData.ID = id;
    g.LastItemData.Rect = bb;
    g.LastItemData.NavRect = nav_bb_arg ? *nav_bb_arg : bb;
    g.LastItemData.ItemFlags = g.CurrentItemFlags | g.NextItemData.ItemFlags | extra_flags;
    g.LastItemData.StatusFlags = ImGuiItemStatusFlags_None;

    if (id != 0)
    {
        KeepAliveID(id);

        if (!(g.LastItemData.ItemFlags & ImGuiItemFlags_NoNav))
        {

            window->DC.NavLayersActiveMaskNext |= (1 << window->DC.NavLayerCurrent);
            if (g.NavId == id || g.NavAnyRequest)
                if (g.NavWindow->RootWindowForNav == window->RootWindowForNav)
                    if (window == g.NavWindow || ((window->ChildFlags | g.NavWindow->ChildFlags) & ImGuiChildFlags_NavFlattened))
                        NavProcessItem();
        }

        if (g.NextItemData.HasFlags & ImGuiNextItemDataFlags_HasShortcut)
            ItemHandleShortcut(id);
    }

    g.NextItemData.HasFlags = ImGuiNextItemDataFlags_None;
    g.NextItemData.ItemFlags = ImGuiItemFlags_None;

#ifdef IMGUI_ENABLE_TEST_ENGINE
    if (id != 0)
        IMGUI_TEST_ENGINE_ITEM_ADD(id, g.LastItemData.NavRect, &g.LastItemData);
#endif

    const bool is_rect_visible = bb.Overlaps(window->ClipRect);
    if (!is_rect_visible)
        if (id == 0 || (id != g.ActiveId && id != g.ActiveIdPreviousFrame && id != g.NavId && id != g.NavActivateId))
            if (!g.ItemUnclipByLog)
                return false;

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (id != 0)
    {
        if (id == g.DebugLocateId)
            DebugLocateItemResolveWithLastItem();

        IM_ASSERT(id != window->ID && "Cannot have an empty ID at the root of a window. If you need an empty label, use ## and read the FAQ about how the ID Stack works!");

#ifdef IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS
        if ((g.LastItemData.ItemFlags & ImGuiItemFlags_AllowDuplicateId) == 0)
        {
            int* p_alive = g.DebugDrawIdConflictsAliveCount.GetIntRef(id, -1);
            int* p_highlight = g.DebugDrawIdConflictsHighlightSet.GetIntRef(id, -1);
            if (*p_alive == g.FrameCount)
                *p_highlight = g.FrameCount;
            *p_alive = g.FrameCount;
            if (*p_highlight >= g.FrameCount - 1)
                window->DrawList->AddRect(bb.Min - ImVec2(1, 1), bb.Max + ImVec2(1, 1), IM_COL32(255, 0, 0, 255), 0.0f, ImDrawFlags_None, 2.0f);
        }
#endif
    }

#endif

    if (id != 0 && g.DeactivatedItemData.ID == id)
        g.DeactivatedItemData.ElapseFrame = g.FrameCount;

    if (is_rect_visible)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Visible;
    if (IsMouseHoveringRect(bb.Min, bb.Max))
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredRect;
    return true;
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

IM_MSVC_RUNTIME_CHECKS_OFF
void ImGui::ItemSize(const ImVec2& size, float text_baseline_y)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    const float offset_to_match_baseline_y = (text_baseline_y >= 0) ? ImMax(0.0f, window->DC.CurrLineTextBaseOffset - text_baseline_y) : 0.0f;

    const float line_y1 = window->DC.IsSameLine ? window->DC.CursorPosPrevLine.y : window->DC.CursorPos.y;
    const float line_height = ImMax(window->DC.CurrLineSize.y, window->DC.CursorPos.y - line_y1 + size.y + offset_to_match_baseline_y);

    window->DC.CursorPosPrevLine.x = window->DC.CursorPos.x + size.x;
    window->DC.CursorPosPrevLine.y = line_y1;
    window->DC.CursorPos.x = IM_TRUNC(window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x);
    window->DC.CursorPos.y = IM_TRUNC(line_y1 + line_height + g.Style.ItemSpacing.y);
    window->DC.CursorMaxPos.x = ImMax(window->DC.CursorMaxPos.x, window->DC.CursorPosPrevLine.x);
    window->DC.CursorMaxPos.y = ImMax(window->DC.CursorMaxPos.y, window->DC.CursorPos.y - g.Style.ItemSpacing.y);

    window->DC.PrevLineSize.y = line_height;
    window->DC.CurrLineSize.y = 0.0f;
    window->DC.PrevLineTextBaseOffset = ImMax(window->DC.CurrLineTextBaseOffset, text_baseline_y);
    window->DC.CurrLineTextBaseOffset = 0.0f;
    window->DC.IsSameLine = window->DC.IsSetPos = false;

    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
        SameLine();
}
IM_MSVC_RUNTIME_CHECKS_RESTORE

void ImGui::SameLine(float offset_from_start_x, float spacing_w)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    if (offset_from_start_x != 0.0f)
    {
        if (spacing_w < 0.0f)
            spacing_w = 0.0f;
        window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + offset_from_start_x + spacing_w + window->DC.GroupOffset.x + window->DC.ColumnsOffset.x;
        window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
    }
    else
    {
        if (spacing_w < 0.0f)
            spacing_w = g.Style.ItemSpacing.x;
        window->DC.CursorPos.x = window->DC.CursorPosPrevLine.x + spacing_w;
        window->DC.CursorPos.y = window->DC.CursorPosPrevLine.y;
    }
    window->DC.CurrLineSize = window->DC.PrevLineSize;
    window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
    window->DC.IsSameLine = true;
}

ImVec2 ImGui::GetCursorScreenPos()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos;
}

void ImGui::SetCursorScreenPos(const ImVec2& pos)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos = pos;

    window->DC.IsSetPos = true;
}

ImVec2 ImGui::GetCursorPos()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos - window->Pos + window->Scroll;
}

float ImGui::GetCursorPosX()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos.x - window->Pos.x + window->Scroll.x;
}

float ImGui::GetCursorPosY()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorPos.y - window->Pos.y + window->Scroll.y;
}

void ImGui::SetCursorPos(const ImVec2& local_pos)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos = window->Pos - window->Scroll + local_pos;

    window->DC.IsSetPos = true;
}

void ImGui::SetCursorPosX(float x)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos.x = window->Pos.x - window->Scroll.x + x;

    window->DC.IsSetPos = true;
}

void ImGui::SetCursorPosY(float y)
{
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.CursorPos.y = window->Pos.y - window->Scroll.y + y;

    window->DC.IsSetPos = true;
}

ImVec2 ImGui::GetCursorStartPos()
{
    ImGuiWindow* window = GetCurrentWindowRead();
    return window->DC.CursorStartPos - window->Pos;
}

void ImGui::Indent(float indent_w)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.Indent.x += (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
    window->DC.CursorPos.x = window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x;
}

void ImGui::Unindent(float indent_w)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.Indent.x -= (indent_w != 0.0f) ? indent_w : g.Style.IndentSpacing;
    window->DC.CursorPos.x = window->Pos.x + window->DC.Indent.x + window->DC.ColumnsOffset.x;
}

void ImGui::SetNextItemWidth(float item_width)
{
    ImGuiContext& g = *GImGui;
    g.NextItemData.HasFlags |= ImGuiNextItemDataFlags_HasWidth;
    g.NextItemData.Width = item_width;
}

void ImGui::PushItemWidth(float item_width)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DC.ItemWidthStack.push_back(window->DC.ItemWidth);
    window->DC.ItemWidth = (item_width == 0.0f ? window->DC.ItemWidthDefault : item_width);
    g.NextItemData.HasFlags &= ~ImGuiNextItemDataFlags_HasWidth;
}

void ImGui::PushMultiItemsWidths(int components, float w_full)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(components > 0);
    const ImGuiStyle& style = g.Style;
    window->DC.ItemWidthStack.push_back(window->DC.ItemWidth);
    float w_items = w_full - style.ItemInnerSpacing.x * (components - 1);
    float prev_split = w_items;
    for (int i = components - 1; i > 0; i--)
    {
        float next_split = IM_TRUNC(w_items * i / components);
        window->DC.ItemWidthStack.push_back(ImMax(prev_split - next_split, 1.0f));
        prev_split = next_split;
    }
    window->DC.ItemWidth = ImMax(prev_split, 1.0f);
    g.NextItemData.HasFlags &= ~ImGuiNextItemDataFlags_HasWidth;
}

void ImGui::PopItemWidth()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->DC.ItemWidthStack.Size <= 0)
    {
        IM_ASSERT_USER_ERROR(0, "Calling PopItemWidth() too many times!");
        return;
    }
    window->DC.ItemWidth = window->DC.ItemWidthStack.back();
    window->DC.ItemWidthStack.pop_back();
}

float ImGui::CalcItemWidth()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float w;
    if (g.NextItemData.HasFlags & ImGuiNextItemDataFlags_HasWidth)
        w = g.NextItemData.Width;
    else
        w = window->DC.ItemWidth;
    if (w < 0.0f)
    {
        float region_avail_x = GetContentRegionAvail().x;
        w = ImMax(1.0f, region_avail_x + w);
    }
    w = IM_TRUNC(w);
    return w;
}

ImVec2 ImGui::CalcItemSize(ImVec2 size, float default_w, float default_h)
{
    ImVec2 avail;
    if (size.x < 0.0f || size.y < 0.0f)
        avail = GetContentRegionAvail();

    if (size.x == 0.0f)
        size.x = default_w;
    else if (size.x < 0.0f)
        size.x = ImMax(4.0f, avail.x + size.x);

    if (size.y == 0.0f)
        size.y = default_h;
    else if (size.y < 0.0f)
        size.y = ImMax(4.0f, avail.y + size.y);

    return size;
}

float ImGui::GetTextLineHeight()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize;
}

float ImGui::GetTextLineHeightWithSpacing()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + g.Style.ItemSpacing.y;
}

float ImGui::GetFrameHeight()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + g.Style.FramePadding.y * 2.0f;
}

float ImGui::GetFrameHeightWithSpacing()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + g.Style.FramePadding.y * 2.0f + g.Style.ItemSpacing.y;
}

ImVec2 ImGui::GetContentRegionAvail()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImVec2 mx = (window->DC.CurrentColumns || g.CurrentTable) ? window->WorkRect.Max : window->ContentRegionRect.Max;
    return mx - window->DC.CursorPos;
}

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS

ImVec2 ImGui::GetContentRegionMax()
{
    return GetContentRegionAvail() + GetCursorScreenPos() - GetWindowPos();
}

ImVec2 ImGui::GetWindowContentRegionMin()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ContentRegionRect.Min - window->Pos;
}

ImVec2 ImGui::GetWindowContentRegionMax()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ContentRegionRect.Max - window->Pos;
}
#endif

void ImGui::BeginGroup()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    g.GroupStack.resize(g.GroupStack.Size + 1);
    ImGuiGroupData& group_data = g.GroupStack.back();
    group_data.WindowID = window->ID;
    group_data.BackupCursorPos = window->DC.CursorPos;
    group_data.BackupCursorPosPrevLine = window->DC.CursorPosPrevLine;
    group_data.BackupCursorMaxPos = window->DC.CursorMaxPos;
    group_data.BackupIndent = window->DC.Indent;
    group_data.BackupGroupOffset = window->DC.GroupOffset;
    group_data.BackupCurrLineSize = window->DC.CurrLineSize;
    group_data.BackupCurrLineTextBaseOffset = window->DC.CurrLineTextBaseOffset;
    group_data.BackupActiveIdIsAlive = g.ActiveIdIsAlive;
    group_data.BackupHoveredIdIsAlive = g.HoveredId != 0;
    group_data.BackupIsSameLine = window->DC.IsSameLine;
    group_data.BackupActiveIdHasBeenEditedThisFrame = g.ActiveIdHasBeenEditedThisFrame;
    group_data.BackupDeactivatedIdIsAlive = g.DeactivatedItemData.IsAlive;
    group_data.EmitItem = true;

    window->DC.GroupOffset.x = window->DC.CursorPos.x - window->Pos.x - window->DC.ColumnsOffset.x;
    window->DC.Indent = window->DC.GroupOffset;
    window->DC.CursorMaxPos = window->DC.CursorPos;
    window->DC.CurrLineSize = ImVec2(0.0f, 0.0f);
    if (g.LogEnabled)
        g.LogLinePosY = -FLT_MAX;
}

void ImGui::EndGroup()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(g.GroupStack.Size > 0);

    ImGuiGroupData& group_data = g.GroupStack.back();
    IM_ASSERT(group_data.WindowID == window->ID);

    if (window->DC.IsSetPos)
        ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

    ImRect group_bb(group_data.BackupCursorPos, ImMax(ImMax(window->DC.CursorMaxPos, g.LastItemData.Rect.Max), group_data.BackupCursorPos));
    window->DC.CursorPos = group_data.BackupCursorPos;
    window->DC.CursorPosPrevLine = group_data.BackupCursorPosPrevLine;
    window->DC.CursorMaxPos = ImMax(group_data.BackupCursorMaxPos, group_bb.Max);
    window->DC.Indent = group_data.BackupIndent;
    window->DC.GroupOffset = group_data.BackupGroupOffset;
    window->DC.CurrLineSize = group_data.BackupCurrLineSize;
    window->DC.CurrLineTextBaseOffset = group_data.BackupCurrLineTextBaseOffset;
    window->DC.IsSameLine = group_data.BackupIsSameLine;
    if (g.LogEnabled)
        g.LogLinePosY = -FLT_MAX;

    if (!group_data.EmitItem)
    {
        g.GroupStack.pop_back();
        return;
    }

    window->DC.CurrLineTextBaseOffset = ImMax(window->DC.PrevLineTextBaseOffset, group_data.BackupCurrLineTextBaseOffset);
    ItemSize(group_bb.GetSize());
    ItemAdd(group_bb, 0, NULL, ImGuiItemFlags_NoTabStop);

    const bool group_contains_curr_active_id = (group_data.BackupActiveIdIsAlive != g.ActiveId) && (g.ActiveIdIsAlive == g.ActiveId) && g.ActiveId;
    const bool group_contains_deactivated_id = (group_data.BackupDeactivatedIdIsAlive == false) && (g.DeactivatedItemData.IsAlive == true);
    if (group_contains_curr_active_id)
        g.LastItemData.ID = g.ActiveId;
    else if (group_contains_deactivated_id)
        g.LastItemData.ID = g.DeactivatedItemData.ID;
    g.LastItemData.Rect = group_bb;

    const bool group_contains_curr_hovered_id = (group_data.BackupHoveredIdIsAlive == false) && g.HoveredId != 0;
    if (group_contains_curr_hovered_id)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;

    if (g.ActiveIdHasBeenEditedThisFrame && !group_data.BackupActiveIdHasBeenEditedThisFrame)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Edited;

    g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasDeactivated;
    if (group_contains_deactivated_id)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Deactivated;

    g.GroupStack.pop_back();
    if (g.DebugShowGroupRects)
        window->DrawList->AddRect(group_bb.Min, group_bb.Max, IM_COL32(255,0,255,255));
}

static float CalcScrollEdgeSnap(float target, float snap_min, float snap_max, float snap_threshold, float center_ratio)
{
    if (target <= snap_min + snap_threshold)
        return ImLerp(snap_min, target, center_ratio);
    if (target >= snap_max - snap_threshold)
        return ImLerp(target, snap_max, center_ratio);
    return target;
}

static ImVec2 CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window)
{
    ImVec2 scroll = window->Scroll;
    ImVec2 decoration_size(window->DecoOuterSizeX1 + window->DecoInnerSizeX1 + window->DecoOuterSizeX2, window->DecoOuterSizeY1 + window->DecoInnerSizeY1 + window->DecoOuterSizeY2);
    for (int axis = 0; axis < 2; axis++)
    {
        if (window->ScrollTarget[axis] < FLT_MAX)
        {
            float center_ratio = window->ScrollTargetCenterRatio[axis];
            float scroll_target = window->ScrollTarget[axis];
            if (window->ScrollTargetEdgeSnapDist[axis] > 0.0f)
            {
                float snap_min = 0.0f;
                float snap_max = window->ScrollMax[axis] + window->SizeFull[axis] - decoration_size[axis];
                scroll_target = CalcScrollEdgeSnap(scroll_target, snap_min, snap_max, window->ScrollTargetEdgeSnapDist[axis], center_ratio);
            }
            scroll[axis] = scroll_target - center_ratio * (window->SizeFull[axis] - decoration_size[axis]);
        }
        scroll[axis] = ImRound64(ImMax(scroll[axis], 0.0f));
        if (!window->Collapsed && !window->SkipItems)
            scroll[axis] = ImMin(scroll[axis], window->ScrollMax[axis]);
    }
    return scroll;
}

void ImGui::ScrollToItem(ImGuiScrollFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ScrollToRectEx(window, g.LastItemData.NavRect, flags);
}

void ImGui::ScrollToRect(ImGuiWindow* window, const ImRect& item_rect, ImGuiScrollFlags flags)
{
    ScrollToRectEx(window, item_rect, flags);
}

ImVec2 ImGui::ScrollToRectEx(ImGuiWindow* window, const ImRect& item_rect, ImGuiScrollFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImRect scroll_rect(window->InnerRect.Min - ImVec2(1, 1), window->InnerRect.Max + ImVec2(1, 1));
    scroll_rect.Min.x = ImMin(scroll_rect.Min.x + window->DecoInnerSizeX1, scroll_rect.Max.x);
    scroll_rect.Min.y = ImMin(scroll_rect.Min.y + window->DecoInnerSizeY1, scroll_rect.Max.y);

    IM_ASSERT((flags & ImGuiScrollFlags_MaskX_) == 0 || ImIsPowerOfTwo(flags & ImGuiScrollFlags_MaskX_));
    IM_ASSERT((flags & ImGuiScrollFlags_MaskY_) == 0 || ImIsPowerOfTwo(flags & ImGuiScrollFlags_MaskY_));

    ImGuiScrollFlags in_flags = flags;
    if ((flags & ImGuiScrollFlags_MaskX_) == 0 && window->ScrollbarX)
        flags |= ImGuiScrollFlags_KeepVisibleEdgeX;
    if ((flags & ImGuiScrollFlags_MaskY_) == 0)
        flags |= window->Appearing ? ImGuiScrollFlags_AlwaysCenterY : ImGuiScrollFlags_KeepVisibleEdgeY;

    const bool fully_visible_x = item_rect.Min.x >= scroll_rect.Min.x && item_rect.Max.x <= scroll_rect.Max.x;
    const bool fully_visible_y = item_rect.Min.y >= scroll_rect.Min.y && item_rect.Max.y <= scroll_rect.Max.y;
    const bool can_be_fully_visible_x = (item_rect.GetWidth() + g.Style.ItemSpacing.x * 2.0f) <= scroll_rect.GetWidth() || (window->AutoFitFramesX > 0) || (window->Flags & ImGuiWindowFlags_AlwaysAutoResize) != 0;
    const bool can_be_fully_visible_y = (item_rect.GetHeight() + g.Style.ItemSpacing.y * 2.0f) <= scroll_rect.GetHeight() || (window->AutoFitFramesY > 0) || (window->Flags & ImGuiWindowFlags_AlwaysAutoResize) != 0;

    if ((flags & ImGuiScrollFlags_KeepVisibleEdgeX) && !fully_visible_x)
    {
        if (item_rect.Min.x < scroll_rect.Min.x || !can_be_fully_visible_x)
            SetScrollFromPosX(window, item_rect.Min.x - g.Style.ItemSpacing.x - window->Pos.x, 0.0f);
        else if (item_rect.Max.x >= scroll_rect.Max.x)
            SetScrollFromPosX(window, item_rect.Max.x + g.Style.ItemSpacing.x - window->Pos.x, 1.0f);
    }
    else if (((flags & ImGuiScrollFlags_KeepVisibleCenterX) && !fully_visible_x) || (flags & ImGuiScrollFlags_AlwaysCenterX))
    {
        if (can_be_fully_visible_x)
            SetScrollFromPosX(window, ImTrunc((item_rect.Min.x + item_rect.Max.x) * 0.5f) - window->Pos.x, 0.5f);
        else
            SetScrollFromPosX(window, item_rect.Min.x - window->Pos.x, 0.0f);
    }

    if ((flags & ImGuiScrollFlags_KeepVisibleEdgeY) && !fully_visible_y)
    {
        if (item_rect.Min.y < scroll_rect.Min.y || !can_be_fully_visible_y)
            SetScrollFromPosY(window, item_rect.Min.y - g.Style.ItemSpacing.y - window->Pos.y, 0.0f);
        else if (item_rect.Max.y >= scroll_rect.Max.y)
            SetScrollFromPosY(window, item_rect.Max.y + g.Style.ItemSpacing.y - window->Pos.y, 1.0f);
    }
    else if (((flags & ImGuiScrollFlags_KeepVisibleCenterY) && !fully_visible_y) || (flags & ImGuiScrollFlags_AlwaysCenterY))
    {
        if (can_be_fully_visible_y)
            SetScrollFromPosY(window, ImTrunc((item_rect.Min.y + item_rect.Max.y) * 0.5f) - window->Pos.y, 0.5f);
        else
            SetScrollFromPosY(window, item_rect.Min.y - window->Pos.y, 0.0f);
    }

    ImVec2 next_scroll = CalcNextScrollFromScrollTargetAndClamp(window);
    ImVec2 delta_scroll = next_scroll - window->Scroll;

    if (!(flags & ImGuiScrollFlags_NoScrollParent) && (window->Flags & ImGuiWindowFlags_ChildWindow))
    {

        if ((in_flags & (ImGuiScrollFlags_AlwaysCenterX | ImGuiScrollFlags_KeepVisibleCenterX)) != 0)
            in_flags = (in_flags & ~ImGuiScrollFlags_MaskX_) | ImGuiScrollFlags_KeepVisibleEdgeX;
        if ((in_flags & (ImGuiScrollFlags_AlwaysCenterY | ImGuiScrollFlags_KeepVisibleCenterY)) != 0)
            in_flags = (in_flags & ~ImGuiScrollFlags_MaskY_) | ImGuiScrollFlags_KeepVisibleEdgeY;
        delta_scroll += ScrollToRectEx(window->ParentWindow, ImRect(item_rect.Min - delta_scroll, item_rect.Max - delta_scroll), in_flags);
    }

    return delta_scroll;
}

float ImGui::GetScrollX()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Scroll.x;
}

float ImGui::GetScrollY()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->Scroll.y;
}

float ImGui::GetScrollMaxX()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ScrollMax.x;
}

float ImGui::GetScrollMaxY()
{
    ImGuiWindow* window = GImGui->CurrentWindow;
    return window->ScrollMax.y;
}

void ImGui::SetScrollX(ImGuiWindow* window, float scroll_x)
{
    window->ScrollTarget.x = scroll_x;
    window->ScrollTargetCenterRatio.x = 0.0f;
    window->ScrollTargetEdgeSnapDist.x = 0.0f;
}

void ImGui::SetScrollY(ImGuiWindow* window, float scroll_y)
{
    window->ScrollTarget.y = scroll_y;
    window->ScrollTargetCenterRatio.y = 0.0f;
    window->ScrollTargetEdgeSnapDist.y = 0.0f;
}

void ImGui::SetScrollX(float scroll_x)
{
    ImGuiContext& g = *GImGui;
    SetScrollX(g.CurrentWindow, scroll_x);
}

void ImGui::SetScrollY(float scroll_y)
{
    ImGuiContext& g = *GImGui;
    SetScrollY(g.CurrentWindow, scroll_y);
}

void ImGui::SetScrollFromPosX(ImGuiWindow* window, float local_x, float center_x_ratio)
{
    IM_ASSERT(center_x_ratio >= 0.0f && center_x_ratio <= 1.0f);
    window->ScrollTarget.x = IM_TRUNC(local_x - window->DecoOuterSizeX1 - window->DecoInnerSizeX1 + window->Scroll.x);
    window->ScrollTargetCenterRatio.x = center_x_ratio;
    window->ScrollTargetEdgeSnapDist.x = 0.0f;
}

void ImGui::SetScrollFromPosY(ImGuiWindow* window, float local_y, float center_y_ratio)
{
    IM_ASSERT(center_y_ratio >= 0.0f && center_y_ratio <= 1.0f);
    window->ScrollTarget.y = IM_TRUNC(local_y - window->DecoOuterSizeY1 - window->DecoInnerSizeY1 + window->Scroll.y);
    window->ScrollTargetCenterRatio.y = center_y_ratio;
    window->ScrollTargetEdgeSnapDist.y = 0.0f;
}

void ImGui::SetScrollFromPosX(float local_x, float center_x_ratio)
{
    ImGuiContext& g = *GImGui;
    SetScrollFromPosX(g.CurrentWindow, local_x, center_x_ratio);
}

void ImGui::SetScrollFromPosY(float local_y, float center_y_ratio)
{
    ImGuiContext& g = *GImGui;
    SetScrollFromPosY(g.CurrentWindow, local_y, center_y_ratio);
}

void ImGui::SetScrollHereX(float center_x_ratio)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float spacing_x = ImMax(window->WindowPadding.x, g.Style.ItemSpacing.x);
    float target_pos_x = ImLerp(g.LastItemData.Rect.Min.x - spacing_x, g.LastItemData.Rect.Max.x + spacing_x, center_x_ratio);
    SetScrollFromPosX(window, target_pos_x - window->Pos.x, center_x_ratio);

    window->ScrollTargetEdgeSnapDist.x = ImMax(0.0f, window->WindowPadding.x - spacing_x);
}

void ImGui::SetScrollHereY(float center_y_ratio)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float spacing_y = ImMax(window->WindowPadding.y, g.Style.ItemSpacing.y);
    float target_pos_y = ImLerp(window->DC.CursorPosPrevLine.y - spacing_y, window->DC.CursorPosPrevLine.y + window->DC.PrevLineSize.y + spacing_y, center_y_ratio);
    SetScrollFromPosY(window, target_pos_y - window->Pos.y, center_y_ratio);

    window->ScrollTargetEdgeSnapDist.y = ImMax(0.0f, window->WindowPadding.y - spacing_y);
}

bool ImGui::BeginTooltip()
{
    return BeginTooltipEx(ImGuiTooltipFlags_None, ImGuiWindowFlags_None);
}

bool ImGui::BeginItemTooltip()
{
    if (!IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        return false;
    return BeginTooltipEx(ImGuiTooltipFlags_None, ImGuiWindowFlags_None);
}

bool ImGui::BeginTooltipEx(ImGuiTooltipFlags tooltip_flags, ImGuiWindowFlags extra_window_flags)
{
    ImGuiContext& g = *GImGui;

    const bool is_dragdrop_tooltip = g.DragDropWithinSource || g.DragDropWithinTarget;
    if (is_dragdrop_tooltip)
    {

        const bool is_touchscreen = (g.IO.MouseSource == ImGuiMouseSource_TouchScreen);
        if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasPos) == 0)
        {
            ImVec2 tooltip_pos = is_touchscreen ? (g.IO.MousePos + TOOLTIP_DEFAULT_OFFSET_TOUCH * g.Style.MouseCursorScale) : (g.IO.MousePos + TOOLTIP_DEFAULT_OFFSET_MOUSE * g.Style.MouseCursorScale);
            ImVec2 tooltip_pivot = is_touchscreen ? TOOLTIP_DEFAULT_PIVOT_TOUCH : ImVec2(0.0f, 0.0f);
            SetNextWindowPos(tooltip_pos, ImGuiCond_None, tooltip_pivot);
        }

        SetNextWindowBgAlpha(g.Style.Colors[ImGuiCol_PopupBg].w * 0.60f);

        tooltip_flags |= ImGuiTooltipFlags_OverridePrevious;
    }

    if ((tooltip_flags & ImGuiTooltipFlags_OverridePrevious) && g.TooltipPreviousWindow != NULL && g.TooltipPreviousWindow->Active && !IsWindowInBeginStack(g.TooltipPreviousWindow))
    {

        SetWindowHiddenAndSkipItemsForCurrentFrame(g.TooltipPreviousWindow);
        g.TooltipOverrideCount++;
    }

    const char* window_name_template = is_dragdrop_tooltip ? "##Tooltip_DragDrop_%02d" : "##Tooltip_%02d";
    char window_name[32];
    ImFormatString(window_name, IM_COUNTOF(window_name), window_name_template, g.TooltipOverrideCount);
    ImGuiWindowFlags flags = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    Begin(window_name, NULL, flags | extra_window_flags);

    return true;
}

void ImGui::EndTooltip()
{
    IM_ASSERT(GetCurrentWindowRead()->Flags & ImGuiWindowFlags_Tooltip);
    End();
}

void ImGui::SetTooltip(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    SetTooltipV(fmt, args);
    va_end(args);
}

void ImGui::SetTooltipV(const char* fmt, va_list args)
{
    if (!BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
        return;
    TextV(fmt, args);
    EndTooltip();
}

void ImGui::SetItemTooltip(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        SetTooltipV(fmt, args);
    va_end(args);
}

void ImGui::SetItemTooltipV(const char* fmt, va_list args)
{
    if (IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        SetTooltipV(fmt, args);
}

bool ImGui::IsPopupOpen(ImGuiID id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    if (popup_flags & ImGuiPopupFlags_AnyPopupId)
    {

        IM_ASSERT(id == 0);
        if (popup_flags & ImGuiPopupFlags_AnyPopupLevel)
            return g.OpenPopupStack.Size > 0;
        else
            return g.OpenPopupStack.Size > g.BeginPopupStack.Size;
    }
    else
    {
        if (popup_flags & ImGuiPopupFlags_AnyPopupLevel)
        {

            for (int n = 0; n < g.OpenPopupStack.Size; n++)
                if (g.OpenPopupStack[n].PopupId == id)
                    return true;
            return false;
        }
        else
        {

            return g.OpenPopupStack.Size > g.BeginPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].PopupId == id;
        }
    }
}

bool ImGui::IsPopupOpen(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = (popup_flags & ImGuiPopupFlags_AnyPopupId) ? 0 : g.CurrentWindow->GetID(str_id);
    if ((popup_flags & ImGuiPopupFlags_AnyPopupLevel) && id != 0)
        IM_ASSERT(0 && "Cannot use IsPopupOpen() with a string id and ImGuiPopupFlags_AnyPopupLevel.");
    return IsPopupOpen(id, popup_flags);
}

ImGuiWindow* ImGui::GetTopMostPopupModal()
{
    ImGuiContext& g = *GImGui;
    for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
        if (ImGuiWindow* popup = g.OpenPopupStack.Data[n].Window)
            if (popup->Flags & ImGuiWindowFlags_Modal)
                return popup;
    return NULL;
}

ImGuiWindow* ImGui::GetTopMostAndVisiblePopupModal()
{
    ImGuiContext& g = *GImGui;
    for (int n = g.OpenPopupStack.Size - 1; n >= 0; n--)
        if (ImGuiWindow* popup = g.OpenPopupStack.Data[n].Window)
            if ((popup->Flags & ImGuiWindowFlags_Modal) && IsWindowActiveAndVisible(popup))
                return popup;
    return NULL;
}

ImGuiWindow* ImGui::FindBlockingModal(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (g.OpenPopupStack.Size <= 0)
        return NULL;

    for (ImGuiPopupData& popup_data : g.OpenPopupStack)
    {
        ImGuiWindow* popup_window = popup_data.Window;
        if (popup_window == NULL || !(popup_window->Flags & ImGuiWindowFlags_Modal))
            continue;
        if (!popup_window->Active && !popup_window->WasActive)
            continue;
        if (window == NULL)
            return popup_window;
        if (IsWindowWithinBeginStackOf(window, popup_window))
            continue;
        return popup_window;
    }
    return NULL;
}

void ImGui::OpenPopup(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = g.CurrentWindow->GetID(str_id);
    IMGUI_DEBUG_LOG_POPUP("[popup] OpenPopup(\"%s\" -> 0x%08X)\n", str_id, id);
    OpenPopupEx(id, popup_flags);
}

void ImGui::OpenPopup(ImGuiID id, ImGuiPopupFlags popup_flags)
{
    OpenPopupEx(id, popup_flags);
}

void ImGui::OpenPopupEx(ImGuiID id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;
    const int current_stack_size = g.BeginPopupStack.Size;

    if (popup_flags & ImGuiPopupFlags_NoOpenOverExistingPopup)
        if (IsPopupOpen((ImGuiID)0, ImGuiPopupFlags_AnyPopupId))
            return;

    ImGuiPopupData popup_ref;
    popup_ref.PopupId = id;
    popup_ref.Window = NULL;
    popup_ref.RestoreNavWindow = g.NavWindow;
    popup_ref.OpenFrameCount = g.FrameCount;
    popup_ref.OpenParentId = parent_window->IDStack.back();
    popup_ref.OpenPopupPos = NavCalcPreferredRefPos(ImGuiWindowFlags_Popup);
    popup_ref.OpenMousePos = IsMousePosValid(&g.IO.MousePos) ? g.IO.MousePos : popup_ref.OpenPopupPos;

    IMGUI_DEBUG_LOG_POPUP("[popup] OpenPopupEx(0x%08X)\n", id);
    if (g.OpenPopupStack.Size < current_stack_size + 1)
    {
        g.OpenPopupStack.push_back(popup_ref);
    }
    else
    {

        bool keep_existing = false;
        if (g.OpenPopupStack[current_stack_size].PopupId == id)
            if ((g.OpenPopupStack[current_stack_size].OpenFrameCount == g.FrameCount - 1) || (popup_flags & ImGuiPopupFlags_NoReopen))
                keep_existing = true;
        if (keep_existing)
        {

            g.OpenPopupStack[current_stack_size].OpenFrameCount = popup_ref.OpenFrameCount;
        }
        else
        {

            ClosePopupToLevel(current_stack_size, true);
            g.OpenPopupStack.push_back(popup_ref);
        }

    }
}

void ImGui::ClosePopupsOverWindow(ImGuiWindow* ref_window, bool restore_focus_to_window_under_popup)
{
    ImGuiContext& g = *GImGui;
    if (g.OpenPopupStack.Size == 0)
        return;

    int popup_count_to_keep = 0;
    if (ref_window)
    {

        for (; popup_count_to_keep < g.OpenPopupStack.Size; popup_count_to_keep++)
        {
            ImGuiPopupData& popup = g.OpenPopupStack[popup_count_to_keep];
            if (!popup.Window)
                continue;
            IM_ASSERT((popup.Window->Flags & ImGuiWindowFlags_Popup) != 0);

            bool ref_window_is_descendant_of_popup = false;
            for (int n = popup_count_to_keep; n < g.OpenPopupStack.Size; n++)
                if (ImGuiWindow* popup_window = g.OpenPopupStack[n].Window)
                    if (IsWindowWithinBeginStackOf(ref_window, popup_window))
                    {
                        ref_window_is_descendant_of_popup = true;
                        break;
                    }
            if (!ref_window_is_descendant_of_popup)
                break;
        }
    }
    if (popup_count_to_keep < g.OpenPopupStack.Size)
    {
        IMGUI_DEBUG_LOG_POPUP("[popup] ClosePopupsOverWindow(\"%s\")\n", ref_window ? ref_window->Name : "<NULL>");
        ClosePopupToLevel(popup_count_to_keep, restore_focus_to_window_under_popup);
    }
}

void ImGui::ClosePopupsExceptModals()
{
    ImGuiContext& g = *GImGui;

    int popup_count_to_keep;
    for (popup_count_to_keep = g.OpenPopupStack.Size; popup_count_to_keep > 0; popup_count_to_keep--)
    {
        ImGuiWindow* window = g.OpenPopupStack[popup_count_to_keep - 1].Window;
        if (!window || (window->Flags & ImGuiWindowFlags_Modal))
            break;
    }
    if (popup_count_to_keep < g.OpenPopupStack.Size)
        ClosePopupToLevel(popup_count_to_keep, true);
}

void ImGui::ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup)
{
    ImGuiContext& g = *GImGui;
    IMGUI_DEBUG_LOG_POPUP("[popup] ClosePopupToLevel(%d), restore_under=%d\n", remaining, restore_focus_to_window_under_popup);
    IM_ASSERT(remaining >= 0 && remaining < g.OpenPopupStack.Size);
    if (g.DebugLogFlags & ImGuiDebugLogFlags_EventPopup)
        for (int n = remaining; n < g.OpenPopupStack.Size; n++)
            IMGUI_DEBUG_LOG_POPUP("[popup] - Closing PopupID 0x%08X Window \"%s\"\n", g.OpenPopupStack[n].PopupId, g.OpenPopupStack[n].Window ? g.OpenPopupStack[n].Window->Name : NULL);

    ImGuiPopupData prev_popup = g.OpenPopupStack[remaining];
    g.OpenPopupStack.resize(remaining);

    if (restore_focus_to_window_under_popup && prev_popup.Window)
    {
        ImGuiWindow* popup_window = prev_popup.Window;
        ImGuiWindow* focus_window = (popup_window->Flags & ImGuiWindowFlags_ChildMenu) ? popup_window->ParentWindow : prev_popup.RestoreNavWindow;
        if (focus_window && !focus_window->WasActive)
            FocusTopMostWindowUnderOne(popup_window, NULL, NULL, ImGuiFocusRequestFlags_RestoreFocusedChild);
        else
            FocusWindow(focus_window, (g.NavLayer == ImGuiNavLayer_Main) ? ImGuiFocusRequestFlags_RestoreFocusedChild : ImGuiFocusRequestFlags_None);
    }
}

void ImGui::CloseCurrentPopup()
{
    ImGuiContext& g = *GImGui;
    int popup_idx = g.BeginPopupStack.Size - 1;
    if (popup_idx < 0 || popup_idx >= g.OpenPopupStack.Size || g.BeginPopupStack[popup_idx].PopupId != g.OpenPopupStack[popup_idx].PopupId)
        return;

    while (popup_idx > 0)
    {
        ImGuiWindow* popup_window = g.OpenPopupStack[popup_idx].Window;
        ImGuiWindow* parent_popup_window = g.OpenPopupStack[popup_idx - 1].Window;
        bool close_parent = false;
        if (popup_window && (popup_window->Flags & ImGuiWindowFlags_ChildMenu))
            if (parent_popup_window && !(parent_popup_window->Flags & ImGuiWindowFlags_MenuBar))
                close_parent = true;
        if (!close_parent)
            break;
        popup_idx--;
    }
    IMGUI_DEBUG_LOG_POPUP("[popup] CloseCurrentPopup %d -> %d\n", g.BeginPopupStack.Size - 1, popup_idx);
    ClosePopupToLevel(popup_idx, true);

    if (ImGuiWindow* window = g.NavWindow)
        window->DC.NavHideHighlightOneFrame = true;
}

bool ImGui::BeginPopupEx(ImGuiID id, ImGuiWindowFlags extra_window_flags)
{
    ImGuiContext& g = *GImGui;
    if (!IsPopupOpen(id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags();
        return false;
    }

    char name[20];
    IM_ASSERT((extra_window_flags & ImGuiWindowFlags_ChildMenu) == 0);
    ImFormatString(name, IM_COUNTOF(name), "##Popup_%08x", id);

    bool is_open = Begin(name, NULL, extra_window_flags | ImGuiWindowFlags_Popup);
    if (!is_open)
        EndPopup();

    return is_open;
}

bool ImGui::BeginPopupMenuEx(ImGuiID id, const char* label, ImGuiWindowFlags extra_window_flags)
{
    ImGuiContext& g = *GImGui;
    if (!IsPopupOpen(id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags();
        return false;
    }

    char name[128];
    IM_ASSERT(extra_window_flags & ImGuiWindowFlags_ChildMenu);
    ImFormatString(name, IM_COUNTOF(name), "%s###Menu_%02d", label, g.BeginMenuDepth);
    bool is_open = Begin(name, NULL, extra_window_flags | ImGuiWindowFlags_Popup);
    if (!is_open)
        EndPopup();

    return is_open;
}

bool ImGui::BeginPopup(const char* str_id, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (g.OpenPopupStack.Size <= g.BeginPopupStack.Size)
    {
        g.NextWindowData.ClearFlags();
        return false;
    }
    flags |= ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;
    ImGuiID id = g.CurrentWindow->GetID(str_id);
    return BeginPopupEx(id, flags);
}

bool ImGui::BeginPopupModal(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const ImGuiID id = window->GetID(name);
    if (!IsPopupOpen(id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags();
        if (p_open && *p_open)
            *p_open = false;
        return false;
    }

    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasPos) == 0)
    {
        const ImGuiViewport* viewport = GetMainViewport();
        SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    }

    flags |= ImGuiWindowFlags_Popup | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse;
    const bool is_open = Begin(name, p_open, flags);
    if (!is_open || (p_open && !*p_open))
    {
        EndPopup();
        if (is_open)
            ClosePopupToLevel(g.BeginPopupStack.Size, true);
        return false;
    }
    return is_open;
}

void ImGui::EndPopup()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT_USER_ERROR_RET((window->Flags & ImGuiWindowFlags_Popup) != 0 && g.BeginPopupStack.Size > 0, "Calling EndPopup() in wrong window!");

    if (g.NavWindow == window)
        NavMoveRequestTryWrapping(window, ImGuiNavMoveFlags_LoopY);

    const ImGuiID backup_within_end_child_id = g.WithinEndChildID;
    if (window->Flags & ImGuiWindowFlags_ChildWindow)
        g.WithinEndChildID = window->ID;
    End();
    g.WithinEndChildID = backup_within_end_child_id;
}

ImGuiMouseButton ImGui::GetMouseButtonFromPopupFlags(ImGuiPopupFlags flags)
{
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    if ((flags & ImGuiPopupFlags_InvalidMask_) != 0)
        return (flags & ImGuiPopupFlags_InvalidMask_);
#else
    IM_ASSERT((flags & ImGuiPopupFlags_InvalidMask_) == 0);
#endif
    if (flags & ImGuiPopupFlags_MouseButtonMask_)
        return ((flags & ImGuiPopupFlags_MouseButtonMask_) >> ImGuiPopupFlags_MouseButtonShift_) - 1;
    return ImGuiMouseButton_Right;
}

bool ImGui::IsPopupOpenRequestForItem(ImGuiPopupFlags popup_flags, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiMouseButton mouse_button = GetMouseButtonFromPopupFlags(popup_flags);
    if (IsMouseReleased(mouse_button) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        return true;
    if (g.NavOpenContextMenuItemId == id && (IsItemFocused() || id == g.CurrentWindow->MoveId))
        return true;
    return false;
}

bool ImGui::IsPopupOpenRequestForWindow(ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiMouseButton mouse_button = GetMouseButtonFromPopupFlags(popup_flags);
    if (IsMouseReleased(mouse_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        if (!(popup_flags & ImGuiPopupFlags_NoOpenOverItems) || !IsAnyItemHovered())
            return true;
    if (g.NavOpenContextMenuWindowId && g.CurrentWindow->ID)
        if (IsWindowChildOf(g.NavWindow, g.CurrentWindow, false))
            return true;
    return false;
}

void ImGui::OpenPopupOnItemClick(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (IsPopupOpenRequestForItem(popup_flags, g.LastItemData.ID))
    {
        ImGuiID id = str_id ? window->GetID(str_id) : g.LastItemData.ID;
        IM_ASSERT(id != 0);
        OpenPopupEx(id, popup_flags);
    }
}

bool ImGui::BeginPopupContextItem(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return false;
    ImGuiID id = str_id ? window->GetID(str_id) : g.LastItemData.ID;
    IM_ASSERT(id != 0);
    if (IsPopupOpenRequestForItem(popup_flags, g.LastItemData.ID))
        OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::BeginPopupContextWindow(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!str_id)
        str_id = "window_context";
    ImGuiID id = window->GetID(str_id);
    if (IsPopupOpenRequestForWindow(popup_flags))
        OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

bool ImGui::BeginPopupContextVoid(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (!str_id)
        str_id = "void_context";
    ImGuiID id = window->GetID(str_id);
    ImGuiMouseButton mouse_button = GetMouseButtonFromPopupFlags(popup_flags);
    if (IsMouseReleased(mouse_button) && !IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
        if (GetTopMostPopupModal() == NULL)
            OpenPopupEx(id, popup_flags);
    return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

ImVec2 ImGui::FindBestWindowPosForPopupEx(const ImVec2& ref_pos, const ImVec2& size, ImGuiDir* last_dir, const ImRect& r_outer, const ImRect& r_avoid, ImGuiPopupPositionPolicy policy)
{
    ImVec2 base_pos_clamped = ImClamp(ref_pos, r_outer.Min, r_outer.Max - size);

    if (policy == ImGuiPopupPositionPolicy_ComboBox)
    {
        const ImGuiDir dir_preferred_order[ImGuiDir_COUNT] = { ImGuiDir_Down, ImGuiDir_Right, ImGuiDir_Left, ImGuiDir_Up };
        for (int n = (*last_dir != ImGuiDir_None) ? -1 : 0; n < ImGuiDir_COUNT; n++)
        {
            const ImGuiDir dir = (n == -1) ? *last_dir : dir_preferred_order[n];
            if (n != -1 && dir == *last_dir)
                continue;
            ImVec2 pos;
            if (dir == ImGuiDir_Down)  pos = ImVec2(r_avoid.Min.x, r_avoid.Max.y);
            if (dir == ImGuiDir_Right) pos = ImVec2(r_avoid.Min.x, r_avoid.Min.y - size.y);
            if (dir == ImGuiDir_Left)  pos = ImVec2(r_avoid.Max.x - size.x, r_avoid.Max.y);
            if (dir == ImGuiDir_Up)    pos = ImVec2(r_avoid.Max.x - size.x, r_avoid.Min.y - size.y);
            if (!r_outer.Contains(ImRect(pos, pos + size)))
                continue;
            *last_dir = dir;
            return pos;
        }
    }

    if (policy == ImGuiPopupPositionPolicy_Tooltip || policy == ImGuiPopupPositionPolicy_Default)
    {
        const ImGuiDir dir_preferred_order[ImGuiDir_COUNT] = { ImGuiDir_Right, ImGuiDir_Down, ImGuiDir_Up, ImGuiDir_Left };
        for (int n = (*last_dir != ImGuiDir_None) ? -1 : 0; n < ImGuiDir_COUNT; n++)
        {
            const ImGuiDir dir = (n == -1) ? *last_dir : dir_preferred_order[n];
            if (n != -1 && dir == *last_dir)
                continue;

            const float avail_w = (dir == ImGuiDir_Left ? r_avoid.Min.x : r_outer.Max.x) - (dir == ImGuiDir_Right ? r_avoid.Max.x : r_outer.Min.x);
            const float avail_h = (dir == ImGuiDir_Up ? r_avoid.Min.y : r_outer.Max.y) - (dir == ImGuiDir_Down ? r_avoid.Max.y : r_outer.Min.y);

            if (avail_w < size.x && (dir == ImGuiDir_Left || dir == ImGuiDir_Right))
                continue;
            if (avail_h < size.y && (dir == ImGuiDir_Up || dir == ImGuiDir_Down))
                continue;

            ImVec2 pos;
            pos.x = (dir == ImGuiDir_Left) ? r_avoid.Min.x - size.x : (dir == ImGuiDir_Right) ? r_avoid.Max.x : base_pos_clamped.x;
            pos.y = (dir == ImGuiDir_Up) ? r_avoid.Min.y - size.y : (dir == ImGuiDir_Down) ? r_avoid.Max.y : base_pos_clamped.y;

            pos.x = ImMax(pos.x, r_outer.Min.x);
            pos.y = ImMax(pos.y, r_outer.Min.y);

            *last_dir = dir;
            return pos;
        }
    }

    *last_dir = ImGuiDir_None;

    if (policy == ImGuiPopupPositionPolicy_Tooltip)
        return ref_pos + ImVec2(2, 2);

    ImVec2 pos = ref_pos;
    pos.x = ImMax(ImMin(pos.x + size.x, r_outer.Max.x) - size.x, r_outer.Min.x);
    pos.y = ImMax(ImMin(pos.y + size.y, r_outer.Max.y) - size.y, r_outer.Min.y);
    return pos;
}

ImRect ImGui::GetPopupAllowedExtentRect(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    IM_UNUSED(window);
    ImRect r_screen = ((ImGuiViewportP*)(void*)GetMainViewport())->GetMainRect();
    ImVec2 padding = g.Style.DisplaySafeAreaPadding;
    r_screen.Expand(ImVec2((r_screen.GetWidth() > padding.x * 2) ? -padding.x : 0.0f, (r_screen.GetHeight() > padding.y * 2) ? -padding.y : 0.0f));
    return r_screen;
}

ImVec2 ImGui::FindBestWindowPosForPopup(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;

    ImRect r_outer = GetPopupAllowedExtentRect(window);
    if (window->Flags & ImGuiWindowFlags_ChildMenu)
    {

        IM_ASSERT(g.CurrentWindow == window);
        ImGuiWindow* parent_window = g.CurrentWindowStack[g.CurrentWindowStack.Size - 2].Window;
        float horizontal_overlap = g.Style.ItemInnerSpacing.x;
        ImRect r_avoid;
        if (parent_window->DC.MenuBarAppending)
            r_avoid = ImRect(-FLT_MAX, parent_window->ClipRect.Min.y, FLT_MAX, parent_window->ClipRect.Max.y);
        else
            r_avoid = ImRect(parent_window->Pos.x + horizontal_overlap, -FLT_MAX, parent_window->Pos.x + parent_window->Size.x - horizontal_overlap - parent_window->ScrollbarSizes.x, FLT_MAX);
        return FindBestWindowPosForPopupEx(window->Pos, window->Size, &window->AutoPosLastDirection, r_outer, r_avoid, ImGuiPopupPositionPolicy_Default);
    }
    if (window->Flags & ImGuiWindowFlags_Popup)
    {
        return FindBestWindowPosForPopupEx(window->Pos, window->Size, &window->AutoPosLastDirection, r_outer, ImRect(window->Pos, window->Pos), ImGuiPopupPositionPolicy_Default);
    }
    if (window->Flags & ImGuiWindowFlags_Tooltip)
    {

        IM_ASSERT(g.CurrentWindow == window);
        const float scale = g.Style.MouseCursorScale;
        const ImVec2 ref_pos = NavCalcPreferredRefPos(ImGuiWindowFlags_Tooltip);

        if (g.IO.MouseSource == ImGuiMouseSource_TouchScreen && NavCalcPreferredRefPosSource(ImGuiWindowFlags_Tooltip) == ImGuiInputSource_Mouse)
        {
            ImVec2 tooltip_pos = ref_pos + TOOLTIP_DEFAULT_OFFSET_TOUCH * scale - (TOOLTIP_DEFAULT_PIVOT_TOUCH * window->Size);
            if (r_outer.Contains(ImRect(tooltip_pos, tooltip_pos + window->Size)))
                return tooltip_pos;
        }

        ImVec2 tooltip_pos = ref_pos + TOOLTIP_DEFAULT_OFFSET_MOUSE * scale;
        ImRect r_avoid;
        if (g.NavCursorVisible && g.NavHighlightItemUnderNav && !g.IO.ConfigNavMoveSetMousePos)
            r_avoid = ImRect(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 16, ref_pos.y + 8);
        else
            r_avoid = ImRect(ref_pos.x - 16, ref_pos.y - 8, ref_pos.x + 24 * scale, ref_pos.y + 24 * scale);

        return FindBestWindowPosForPopupEx(tooltip_pos, window->Size, &window->AutoPosLastDirection, r_outer, r_avoid, ImGuiPopupPositionPolicy_Tooltip);
    }
    IM_ASSERT(0);
    return window->Pos;
}

void ImGui::SetWindowFocus()
{
    FocusWindow(GImGui->CurrentWindow);
}

void ImGui::SetWindowFocus(const char* name)
{
    if (name)
    {
        if (ImGuiWindow* window = FindWindowByName(name))
            FocusWindow(window);
    }
    else
    {
        FocusWindow(NULL);
    }
}

void ImGui::SetNextWindowFocus()
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasFocus;
}

bool ImGui::IsWindowFocused(ImGuiFocusedFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* ref_window = g.NavWindow;
    ImGuiWindow* cur_window = g.CurrentWindow;

    if (ref_window == NULL)
        return false;
    if (flags & ImGuiFocusedFlags_AnyWindow)
        return true;

    IM_ASSERT(cur_window);
    const bool popup_hierarchy = (flags & ImGuiFocusedFlags_NoPopupHierarchy) == 0;
    if (flags & ImGuiFocusedFlags_RootWindow)
        cur_window = GetCombinedRootWindow(cur_window, popup_hierarchy);

    if (flags & ImGuiFocusedFlags_ChildWindows)
        return IsWindowChildOf(ref_window, cur_window, popup_hierarchy);
    else
        return ref_window == cur_window;
}

static int ImGui::FindWindowFocusIndex(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    IM_UNUSED(g);
    int order = window->FocusOrder;
    IM_ASSERT(window->RootWindow == window);
    IM_ASSERT(g.WindowsFocusOrder[order] == window);
    return order;
}

static void ImGui::UpdateWindowInFocusOrderList(ImGuiWindow* window, bool just_created, ImGuiWindowFlags new_flags)
{
    ImGuiContext& g = *GImGui;

    const bool new_is_explicit_child = (new_flags & ImGuiWindowFlags_ChildWindow) != 0 && ((new_flags & ImGuiWindowFlags_Popup) == 0 || (new_flags & ImGuiWindowFlags_ChildMenu) != 0);
    const bool child_flag_changed = new_is_explicit_child != window->IsExplicitChild;
    if ((just_created || child_flag_changed) && !new_is_explicit_child)
    {
        IM_ASSERT(!g.WindowsFocusOrder.contains(window));
        g.WindowsFocusOrder.push_back(window);
        window->FocusOrder = (short)(g.WindowsFocusOrder.Size - 1);
    }
    else if (!just_created && child_flag_changed && new_is_explicit_child)
    {
        IM_ASSERT(g.WindowsFocusOrder[window->FocusOrder] == window);
        for (int n = window->FocusOrder + 1; n < g.WindowsFocusOrder.Size; n++)
            g.WindowsFocusOrder[n]->FocusOrder--;
        g.WindowsFocusOrder.erase(g.WindowsFocusOrder.Data + window->FocusOrder);
        window->FocusOrder = -1;
    }
    window->IsExplicitChild = new_is_explicit_child;
}

void ImGui::BringWindowToFocusFront(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(window == window->RootWindow);

    const int cur_order = window->FocusOrder;
    IM_ASSERT(g.WindowsFocusOrder[cur_order] == window);
    if (g.WindowsFocusOrder.back() == window)
        return;

    const int new_order = g.WindowsFocusOrder.Size - 1;
    for (int n = cur_order; n < new_order; n++)
    {
        g.WindowsFocusOrder[n] = g.WindowsFocusOrder[n + 1];
        g.WindowsFocusOrder[n]->FocusOrder--;
        IM_ASSERT(g.WindowsFocusOrder[n]->FocusOrder == n);
    }
    g.WindowsFocusOrder[new_order] = window;
    window->FocusOrder = (short)new_order;
}

void ImGui::BringWindowToDisplayFront(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* current_front_window = g.Windows.back();
    if (current_front_window == window || current_front_window->RootWindow == window)
        return;
    for (int i = g.Windows.Size - 2; i >= 0; i--)
        if (g.Windows[i] == window)
        {
            memmove(&g.Windows[i], &g.Windows[i + 1], (size_t)(g.Windows.Size - i - 1) * sizeof(ImGuiWindow*));
            g.Windows[g.Windows.Size - 1] = window;
            break;
        }
}

void ImGui::BringWindowToDisplayBack(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (g.Windows[0] == window)
        return;
    for (int i = 0; i < g.Windows.Size; i++)
        if (g.Windows[i] == window)
        {
            memmove(&g.Windows[1], &g.Windows[0], (size_t)i * sizeof(ImGuiWindow*));
            g.Windows[0] = window;
            break;
        }
}

void ImGui::BringWindowToDisplayBehind(ImGuiWindow* window, ImGuiWindow* behind_window)
{
    IM_ASSERT(window != NULL && behind_window != NULL);
    ImGuiContext& g = *GImGui;
    window = window->RootWindow;
    behind_window = behind_window->RootWindow;
    int pos_wnd = FindWindowDisplayIndex(window);
    int pos_beh = FindWindowDisplayIndex(behind_window);
    if (pos_wnd < pos_beh)
    {
        size_t copy_bytes = (pos_beh - pos_wnd - 1) * sizeof(ImGuiWindow*);
        memmove(&g.Windows.Data[pos_wnd], &g.Windows.Data[pos_wnd + 1], copy_bytes);
        g.Windows[pos_beh - 1] = window;
    }
    else
    {
        size_t copy_bytes = (pos_wnd - pos_beh) * sizeof(ImGuiWindow*);
        memmove(&g.Windows.Data[pos_beh + 1], &g.Windows.Data[pos_beh], copy_bytes);
        g.Windows[pos_beh] = window;
    }
}

int ImGui::FindWindowDisplayIndex(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    return g.Windows.index_from_ptr(g.Windows.find(window));
}

void ImGui::FocusWindow(ImGuiWindow* window, ImGuiFocusRequestFlags flags)
{
    ImGuiContext& g = *GImGui;

    if ((flags & ImGuiFocusRequestFlags_UnlessBelowModal) && (g.NavWindow != window))
        if (ImGuiWindow* blocking_modal = FindBlockingModal(window))
        {

            IMGUI_DEBUG_LOG_FOCUS("[focus] FocusWindow(\"%s\", UnlessBelowModal): prevented by \"%s\".\n", window ? window->Name : "<NULL>", blocking_modal->Name);
            if (window && window == window->RootWindow && (window->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus) == 0)
                BringWindowToDisplayBehind(window, blocking_modal);
            ClosePopupsOverWindow(GetTopMostPopupModal(), false);
            return;
        }

    if ((flags & ImGuiFocusRequestFlags_RestoreFocusedChild) && window != NULL)
        window = NavRestoreLastChildNavWindow(window);

    if (g.NavWindow != window)
    {
        SetNavWindow(window);
        if (window && g.NavHighlightItemUnderNav)
            g.NavMousePosDirty = true;
        g.NavId = window ? window->NavLastIds[0] : 0;
        g.NavLayer = ImGuiNavLayer_Main;
        SetNavFocusScope(window ? window->NavRootFocusScopeId : 0);
        g.NavIdIsAlive = false;
        g.NavLastValidSelectionUserData = ImGuiSelectionUserData_Invalid;

        ClosePopupsOverWindow(window, false);
    }

    IM_ASSERT(window == NULL || window->RootWindow != NULL);
    ImGuiWindow* focus_front_window = window ? window->RootWindow : NULL;
    ImGuiWindow* display_front_window = window ? window->RootWindow : NULL;

    if (g.ActiveId != 0 && g.ActiveIdWindow && g.ActiveIdWindow->RootWindow != focus_front_window)
        if (!g.ActiveIdNoClearOnFocusLoss)
            ClearActiveID();

    if (!window)
        return;

    BringWindowToFocusFront(focus_front_window);
    if (((window->Flags | display_front_window->Flags) & ImGuiWindowFlags_NoBringToFrontOnFocus) == 0)
        BringWindowToDisplayFront(display_front_window);
}

void ImGui::FocusTopMostWindowUnderOne(ImGuiWindow* under_this_window, ImGuiWindow* ignore_window, ImGuiViewport* filter_viewport, ImGuiFocusRequestFlags flags)
{
    ImGuiContext& g = *GImGui;
    IM_UNUSED(filter_viewport);
    int start_idx = g.WindowsFocusOrder.Size - 1;
    if (under_this_window != NULL)
    {

        int offset = -1;
        while (under_this_window->Flags & ImGuiWindowFlags_ChildWindow)
        {
            under_this_window = under_this_window->ParentWindow;
            offset = 0;
        }
        start_idx = FindWindowFocusIndex(under_this_window) + offset;
    }
    for (int i = start_idx; i >= 0; i--)
    {

        ImGuiWindow* window = g.WindowsFocusOrder[i];
        if (window == ignore_window || !window->WasActive)
            continue;
        if ((window->Flags & (ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs)) != (ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs))
        {
            FocusWindow(window, flags);
            return;
        }
    }
    FocusWindow(NULL, flags);
}

void ImGui::SetNavCursorVisible(bool visible)
{
    ImGuiContext& g = *GImGui;
    if (g.NavWindow && (g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
        visible = false;
    else if (g.IO.ConfigNavCursorVisibleAlways)
        visible = true;
    g.NavCursorVisible = visible;
}

void ImGui::SetNavCursorVisibleAfterMove()
{
    ImGuiContext& g = *GImGui;
    if (g.NavWindow && (g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
        g.NavCursorVisible = false;
    else if (g.NavInputSource == ImGuiInputSource_Keyboard && (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) == 0)
        g.NavCursorVisible = false;
    else if (g.NavInputSource == ImGuiInputSource_Gamepad && (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        g.NavCursorVisible = false;
    else if (g.IO.ConfigNavCursorVisibleAuto)
        g.NavCursorVisible = true;
    g.NavHighlightItemUnderNav = g.NavMousePosDirty = true;
}

void ImGui::SetNavWindow(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (g.NavWindow != window)
    {
        IMGUI_DEBUG_LOG_FOCUS("[focus] SetNavWindow(\"%s\")\n", window ? window->Name : "<NULL>");
        g.NavWindow = window;
        g.NavLastValidSelectionUserData = ImGuiSelectionUserData_Invalid;
    }
    g.NavInitRequest = g.NavMoveSubmitted = g.NavMoveScoringItems = false;
    NavUpdateAnyRequestFlag();
}

void ImGui::NavHighlightActivated(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    g.NavHighlightActivatedId = id;
    g.NavHighlightActivatedTimer = NAV_ACTIVATE_HIGHLIGHT_TIMER;
}

void ImGui::NavClearPreferredPosForAxis(ImGuiAxis axis)
{
    ImGuiContext& g = *GImGui;
    g.NavWindow->RootWindowForNav->NavPreferredScoringPosRel[g.NavLayer][axis] = FLT_MAX;
}

void ImGui::SetNavID(ImGuiID id, ImGuiNavLayer nav_layer, ImGuiID focus_scope_id, const ImRect& rect_rel)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.NavWindow != NULL);
    IM_ASSERT(nav_layer == ImGuiNavLayer_Main || nav_layer == ImGuiNavLayer_Menu);
    g.NavId = id;
    g.NavLayer = nav_layer;
    SetNavFocusScope(focus_scope_id);
    g.NavWindow->NavLastIds[nav_layer] = id;
    g.NavWindow->NavRectRel[nav_layer] = rect_rel;

    NavClearPreferredPosForAxis(ImGuiAxis_X);
    NavClearPreferredPosForAxis(ImGuiAxis_Y);
}

void ImGui::SetFocusID(ImGuiID id, ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(id != 0);

    if (g.NavWindow != window)
       SetNavWindow(window);

    const ImGuiNavLayer nav_layer = window->DC.NavLayerCurrent;
    g.NavId = id;
    g.NavLayer = nav_layer;
    SetNavFocusScope(g.CurrentFocusScopeId);
    window->NavLastIds[nav_layer] = id;
    if (g.LastItemData.ID == id)
        window->NavRectRel[nav_layer] = WindowRectAbsToRel(window, g.LastItemData.NavRect);
    g.NavIdItemFlags = (g.LastItemData.ID == id) ? g.LastItemData.ItemFlags : ImGuiItemFlags_None;
    if (id == g.ActiveIdIsAlive)
        g.NavIdIsAlive = true;

    if (g.ActiveIdSource == ImGuiInputSource_Keyboard || g.ActiveIdSource == ImGuiInputSource_Gamepad)
        g.NavHighlightItemUnderNav = true;
    else if (g.IO.ConfigNavCursorVisibleAuto)
        g.NavCursorVisible = false;

    NavClearPreferredPosForAxis(ImGuiAxis_X);
    NavClearPreferredPosForAxis(ImGuiAxis_Y);
}

static ImGuiDir ImGetDirQuadrantFromDelta(float dx, float dy)
{
    if (ImFabs(dx) > ImFabs(dy))
        return (dx > 0.0f) ? ImGuiDir_Right : ImGuiDir_Left;
    return (dy > 0.0f) ? ImGuiDir_Down : ImGuiDir_Up;
}

static float inline NavScoreItemDistInterval(float cand_min, float cand_max, float curr_min, float curr_max)
{
    if (cand_max < curr_min)
        return cand_max - curr_min;
    if (curr_max < cand_min)
        return cand_min - curr_max;
    return 0.0f;
}

static bool ImGui::NavScoreItem(ImGuiNavItemData* result, const ImRect& nav_bb)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (g.NavLayer != window->DC.NavLayerCurrent)
        return false;

    ImRect cand = nav_bb;
    const ImRect curr = g.NavScoringRect;
    g.NavScoringDebugCount++;

    if (window->ParentWindow == g.NavWindow)
    {
        IM_ASSERT((window->ChildFlags | g.NavWindow->ChildFlags) & ImGuiChildFlags_NavFlattened);
        if (!window->ClipRect.Overlaps(cand))
            return false;
        cand.ClipWithFull(window->ClipRect);
    }

    float dbx = NavScoreItemDistInterval(cand.Min.x, cand.Max.x, curr.Min.x, curr.Max.x);
    float dby = NavScoreItemDistInterval(ImLerp(cand.Min.y, cand.Max.y, 0.2f), ImLerp(cand.Min.y, cand.Max.y, 0.8f), ImLerp(curr.Min.y, curr.Max.y, 0.2f), ImLerp(curr.Min.y, curr.Max.y, 0.8f));
    if (dby != 0.0f && dbx != 0.0f)
        dbx = (dbx / 1000.0f) + ((dbx > 0.0f) ? +1.0f : -1.0f);
    float dist_box = ImFabs(dbx) + ImFabs(dby);

    float dcx = (cand.Min.x + cand.Max.x) - (curr.Min.x + curr.Max.x);
    float dcy = (cand.Min.y + cand.Max.y) - (curr.Min.y + curr.Max.y);
    float dist_center = ImFabs(dcx) + ImFabs(dcy);

    ImGuiDir quadrant;
    float dax = 0.0f, day = 0.0f, dist_axial = 0.0f;
    if (dbx != 0.0f || dby != 0.0f)
    {

        dax = dbx;
        day = dby;
        dist_axial = dist_box;
        quadrant = ImGetDirQuadrantFromDelta(dbx, dby);
    }
    else if (dcx != 0.0f || dcy != 0.0f)
    {

        dax = dcx;
        day = dcy;
        dist_axial = dist_center;
        quadrant = ImGetDirQuadrantFromDelta(dcx, dcy);
    }
    else
    {

        quadrant = (g.LastItemData.ID < g.NavId) ? ImGuiDir_Left : ImGuiDir_Right;
    }

    const ImGuiDir move_dir = g.NavMoveDir;
#if IMGUI_DEBUG_NAV_SCORING
    char buf[200];
    if (g.IO.KeyCtrl)
    {
        if (quadrant == move_dir)
        {
            ImFormatString(buf, IM_COUNTOF(buf), "%.0f/%.0f", dist_box, dist_center);
            ImDrawList* draw_list = GetForegroundDrawList(window);
            draw_list->AddRectFilled(cand.Min, cand.Max, IM_COL32(255, 0, 0, 80));
            draw_list->AddRectFilled(cand.Min, cand.Min + CalcTextSize(buf), IM_COL32(255, 0, 0, 200));
            draw_list->AddText(cand.Min, IM_COL32(255, 255, 255, 255), buf);
        }
    }
    const bool debug_hovering = IsMouseHoveringRect(cand.Min, cand.Max);
    const bool debug_tty = (g.IO.KeyCtrl && IsKeyPressed(ImGuiKey_Space));
    if (debug_hovering || debug_tty)
    {
        ImFormatString(buf, IM_COUNTOF(buf),
            "d-box    (%7.3f,%7.3f) -> %7.3f\nd-center (%7.3f,%7.3f) -> %7.3f\nd-axial  (%7.3f,%7.3f) -> %7.3f\nnav %c, quadrant %c",
            dbx, dby, dist_box, dcx, dcy, dist_center, dax, day, dist_axial, "-WENS"[move_dir+1], "-WENS"[quadrant+1]);
        if (debug_hovering)
        {
            ImDrawList* draw_list = GetForegroundDrawList(window);
            draw_list->AddRect(curr.Min, curr.Max, IM_COL32(255, 200, 0, 100));
            draw_list->AddRect(cand.Min, cand.Max, IM_COL32(255, 255, 0, 200));
            draw_list->AddRectFilled(cand.Max - ImVec2(4, 4), cand.Max + CalcTextSize(buf) + ImVec2(4, 4), IM_COL32(40, 0, 0, 200));
            draw_list->AddText(cand.Max, ~0U, buf);
        }
        if (debug_tty) { IMGUI_DEBUG_LOG_NAV("id 0x%08X\n%s\n", g.LastItemData.ID, buf); }
    }
#endif

    bool new_best = false;
    if (quadrant == move_dir)
    {

        if (dist_box < result->DistBox)
        {
            result->DistBox = dist_box;
            result->DistCenter = dist_center;
            return true;
        }
        if (dist_box == result->DistBox)
        {

            if (dist_center < result->DistCenter)
            {
                result->DistCenter = dist_center;
                new_best = true;
            }
            else if (dist_center == result->DistCenter)
            {

                if (((move_dir == ImGuiDir_Up || move_dir == ImGuiDir_Down) ? dby : dbx) < 0.0f)
                    new_best = true;
            }
        }
    }

    if (result->DistBox == FLT_MAX && dist_axial < result->DistAxial)
        if (g.NavLayer == ImGuiNavLayer_Menu && !(g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu))
            if ((move_dir == ImGuiDir_Left && dax < 0.0f) || (move_dir == ImGuiDir_Right && dax > 0.0f) || (move_dir == ImGuiDir_Up && day < 0.0f) || (move_dir == ImGuiDir_Down && day > 0.0f))
            {
                result->DistAxial = dist_axial;
                new_best = true;
            }

    return new_best;
}

static void ImGui::NavApplyItemToResult(ImGuiNavItemData* result)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    result->Window = window;
    result->ID = g.LastItemData.ID;
    result->FocusScopeId = g.CurrentFocusScopeId;
    result->ItemFlags = g.LastItemData.ItemFlags;
    result->RectRel = WindowRectAbsToRel(window, g.LastItemData.NavRect);
    if (result->ItemFlags & ImGuiItemFlags_HasSelectionUserData)
    {
        IM_ASSERT(g.NextItemData.SelectionUserData != ImGuiSelectionUserData_Invalid);
        result->SelectionUserData = g.NextItemData.SelectionUserData;
    }
}

void ImGui::NavUpdateCurrentWindowIsScrollPushableX()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    window->DC.NavIsScrollPushableX = (g.CurrentTable == NULL && window->DC.CurrentColumns == NULL);
}

static void ImGui::NavProcessItem()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    const ImGuiID id = g.LastItemData.ID;
    const ImGuiItemFlags item_flags = g.LastItemData.ItemFlags;

    ImRect nav_bb = g.LastItemData.NavRect;
    if (window->DC.NavIsScrollPushableX == false)
    {
        nav_bb.Min.x = ImClamp(nav_bb.Min.x, window->ClipRect.Min.x, window->ClipRect.Max.x);
        nav_bb.Max.x = ImClamp(nav_bb.Max.x, window->ClipRect.Min.x, window->ClipRect.Max.x);
    }

    if (g.NavInitRequest && g.NavLayer == window->DC.NavLayerCurrent && (item_flags & ImGuiItemFlags_Disabled) == 0)
    {

        const bool candidate_for_nav_default_focus = (item_flags & ImGuiItemFlags_NoNavDefaultFocus) == 0;
        if (candidate_for_nav_default_focus || g.NavInitResult.ID == 0)
        {
            NavApplyItemToResult(&g.NavInitResult);
        }
        if (candidate_for_nav_default_focus)
        {
            g.NavInitRequest = false;
            NavUpdateAnyRequestFlag();
        }
    }

    if (g.NavMoveScoringItems && (item_flags & ImGuiItemFlags_Disabled) == 0)
    {
        if ((g.NavMoveFlags & ImGuiNavMoveFlags_FocusApi) || (window->Flags & ImGuiWindowFlags_NoNavInputs) == 0)
        {
            const bool is_tabbing = (g.NavMoveFlags & ImGuiNavMoveFlags_IsTabbing) != 0;
            if (is_tabbing)
            {
                NavProcessItemForTabbingRequest(id, item_flags, g.NavMoveFlags);
            }
            else if (g.NavId != id || (g.NavMoveFlags & ImGuiNavMoveFlags_AllowCurrentNavId))
            {
                ImGuiNavItemData* result = (window == g.NavWindow) ? &g.NavMoveResultLocal : &g.NavMoveResultOther;
                if (NavScoreItem(result, nav_bb))
                    NavApplyItemToResult(result);

                const float VISIBLE_RATIO = 0.70f;
                if (g.NavMoveFlags & ImGuiNavMoveFlags_AlsoScoreVisibleSet)
                {
                    const ImRect& r = window->InnerRect;
                    if (r.Overlaps(nav_bb))
                        if (ImClamp(nav_bb.Max.y, r.Min.y, r.Max.y) - ImClamp(nav_bb.Min.y, r.Min.y, r.Max.y) >= (nav_bb.Max.y - nav_bb.Min.y) * VISIBLE_RATIO)
                            if (NavScoreItem(&g.NavMoveResultLocalVisible, nav_bb))
                                NavApplyItemToResult(&g.NavMoveResultLocalVisible);
                }
            }
        }
    }

    if (g.NavId == id)
    {
        if (g.NavWindow != window)
            SetNavWindow(window);
        g.NavLayer = window->DC.NavLayerCurrent;
        SetNavFocusScope(g.CurrentFocusScopeId);
        g.NavFocusScopeId = g.CurrentFocusScopeId;
        g.NavIdIsAlive = true;
        g.NavIdItemFlags = item_flags;
        if (g.LastItemData.ItemFlags & ImGuiItemFlags_HasSelectionUserData)
        {
            IM_ASSERT(g.NextItemData.SelectionUserData != ImGuiSelectionUserData_Invalid);
            g.NavLastValidSelectionUserData = g.NextItemData.SelectionUserData;
        }
        window->NavRectRel[window->DC.NavLayerCurrent] = WindowRectAbsToRel(window, nav_bb);
    }
}

void ImGui::NavProcessItemForTabbingRequest(ImGuiID id, ImGuiItemFlags item_flags, ImGuiNavMoveFlags move_flags)
{
    ImGuiContext& g = *GImGui;

    if ((move_flags & ImGuiNavMoveFlags_FocusApi) == 0)
    {
        if (g.NavLayer != g.CurrentWindow->DC.NavLayerCurrent)
            return;
        if (g.NavFocusScopeId != g.CurrentFocusScopeId)
            return;
    }

    bool can_stop;
    if (move_flags & ImGuiNavMoveFlags_FocusApi)
        can_stop = true;
    else
        can_stop = (item_flags & ImGuiItemFlags_NoTabStop) == 0 && ((g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) || (item_flags & ImGuiItemFlags_Inputable));

    ImGuiNavItemData* result = &g.NavMoveResultLocal;
    if (g.NavTabbingDir == +1)
    {

        if (can_stop && g.NavTabbingResultFirst.ID == 0)
            NavApplyItemToResult(&g.NavTabbingResultFirst);
        if (can_stop && g.NavTabbingCounter > 0 && --g.NavTabbingCounter == 0)
            NavMoveRequestResolveWithLastItem(result);
        else if (g.NavId == id)
            g.NavTabbingCounter = 1;
    }
    else if (g.NavTabbingDir == -1)
    {

        if (g.NavId == id)
        {
            if (result->ID)
            {
                g.NavMoveScoringItems = false;
                NavUpdateAnyRequestFlag();
            }
        }
        else if (can_stop)
        {

            NavApplyItemToResult(result);
        }
    }
    else if (g.NavTabbingDir == 0)
    {
        if (can_stop && g.NavId == id)
            NavMoveRequestResolveWithLastItem(result);
        if (can_stop && g.NavTabbingResultFirst.ID == 0)
            NavApplyItemToResult(&g.NavTabbingResultFirst);
    }
}

bool ImGui::NavMoveRequestButNoResultYet()
{
    ImGuiContext& g = *GImGui;
    return g.NavMoveScoringItems && g.NavMoveResultLocal.ID == 0 && g.NavMoveResultOther.ID == 0;
}

void ImGui::NavMoveRequestSubmit(ImGuiDir move_dir, ImGuiDir clip_dir, ImGuiNavMoveFlags move_flags, ImGuiScrollFlags scroll_flags)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.NavWindow != NULL);

    if (move_flags & ImGuiNavMoveFlags_IsTabbing)
        move_flags |= ImGuiNavMoveFlags_AllowCurrentNavId;

    g.NavMoveSubmitted = g.NavMoveScoringItems = true;
    g.NavMoveDir = move_dir;
    g.NavMoveDirForDebug = move_dir;
    g.NavMoveClipDir = clip_dir;
    g.NavMoveFlags = move_flags;
    g.NavMoveScrollFlags = scroll_flags;
    g.NavMoveForwardToNextFrame = false;
    g.NavMoveKeyMods = (move_flags & ImGuiNavMoveFlags_FocusApi) ? 0 : g.IO.KeyMods;
    g.NavMoveResultLocal.Clear();
    g.NavMoveResultLocalVisible.Clear();
    g.NavMoveResultOther.Clear();
    g.NavTabbingCounter = 0;
    g.NavTabbingResultFirst.Clear();
    NavUpdateAnyRequestFlag();
}

void ImGui::NavMoveRequestResolveWithLastItem(ImGuiNavItemData* result)
{
    ImGuiContext& g = *GImGui;
    g.NavMoveScoringItems = false;
    NavApplyItemToResult(result);
    NavUpdateAnyRequestFlag();
}

void ImGui::NavMoveRequestResolveWithPastTreeNode(ImGuiNavItemData* result, const ImGuiTreeNodeStackData* tree_node_data)
{
    ImGuiContext& g = *GImGui;
    g.NavMoveScoringItems = false;
    g.LastItemData.ID = tree_node_data->ID;
    g.LastItemData.ItemFlags = tree_node_data->ItemFlags & ~ImGuiItemFlags_HasSelectionUserData;
    g.LastItemData.NavRect = tree_node_data->NavRect;
    NavApplyItemToResult(result);
    NavClearPreferredPosForAxis(ImGuiAxis_Y);
    NavUpdateAnyRequestFlag();
}

void ImGui::NavMoveRequestCancel()
{
    ImGuiContext& g = *GImGui;
    g.NavMoveSubmitted = g.NavMoveScoringItems = false;
    NavUpdateAnyRequestFlag();
}

void ImGui::NavMoveRequestForward(ImGuiDir move_dir, ImGuiDir clip_dir, ImGuiNavMoveFlags move_flags, ImGuiScrollFlags scroll_flags)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.NavMoveForwardToNextFrame == false);
    NavMoveRequestCancel();
    g.NavMoveForwardToNextFrame = true;
    g.NavMoveDir = move_dir;
    g.NavMoveClipDir = clip_dir;
    g.NavMoveFlags = move_flags | ImGuiNavMoveFlags_Forwarded;
    g.NavMoveScrollFlags = scroll_flags;
}

void ImGui::NavMoveRequestTryWrapping(ImGuiWindow* window, ImGuiNavMoveFlags wrap_flags)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT((wrap_flags & ImGuiNavMoveFlags_WrapMask_ ) != 0 && (wrap_flags & ~ImGuiNavMoveFlags_WrapMask_) == 0);

    if (g.NavWindow == window && g.NavMoveScoringItems && g.NavLayer == window->DC.NavLayerCurrent)
        g.NavMoveFlags = (g.NavMoveFlags & ~ImGuiNavMoveFlags_WrapMask_) | wrap_flags;
}

static void ImGui::NavSaveLastChildNavWindowIntoParent(ImGuiWindow* nav_window)
{
    ImGuiWindow* parent = nav_window;
    while (parent && parent->RootWindow != parent && (parent->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu)) == 0)
        parent = parent->ParentWindow;
    if (parent && parent != nav_window)
        parent->NavLastChildNavWindow = nav_window;
}

static ImGuiWindow* ImGui::NavRestoreLastChildNavWindow(ImGuiWindow* window)
{
    if (window->NavLastChildNavWindow && window->NavLastChildNavWindow->WasActive)
        return window->NavLastChildNavWindow;
    return window;
}

void ImGui::NavRestoreLayer(ImGuiNavLayer layer)
{
    ImGuiContext& g = *GImGui;
    if (layer == ImGuiNavLayer_Main)
    {
        ImGuiWindow* prev_nav_window = g.NavWindow;
        g.NavWindow = NavRestoreLastChildNavWindow(g.NavWindow);
        g.NavLastValidSelectionUserData = ImGuiSelectionUserData_Invalid;
        if (prev_nav_window)
            IMGUI_DEBUG_LOG_FOCUS("[focus] NavRestoreLayer: from \"%s\" to SetNavWindow(\"%s\")\n", prev_nav_window->Name, g.NavWindow->Name);
    }
    ImGuiWindow* window = g.NavWindow;
    if (window->NavLastIds[layer] != 0)
    {
        SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
    }
    else
    {
        g.NavLayer = layer;
        NavInitWindow(window, true);
    }
}

static inline void ImGui::NavUpdateAnyRequestFlag()
{
    ImGuiContext& g = *GImGui;
    g.NavAnyRequest = g.NavMoveScoringItems || g.NavInitRequest || (IMGUI_DEBUG_NAV_SCORING && g.NavWindow != NULL);
    if (g.NavAnyRequest)
        IM_ASSERT(g.NavWindow != NULL);
}

void ImGui::NavInitWindow(ImGuiWindow* window, bool force_reinit)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(window == g.NavWindow);

    if (window->Flags & ImGuiWindowFlags_NoNavInputs)
    {
        g.NavId = 0;
        SetNavFocusScope(window->NavRootFocusScopeId);
        return;
    }

    bool init_for_nav = false;
    if (window == window->RootWindow || (window->Flags & ImGuiWindowFlags_Popup) || (window->NavLastIds[0] == 0) || force_reinit)
        init_for_nav = true;
    IMGUI_DEBUG_LOG_NAV("[nav] NavInitRequest: from NavInitWindow(), init_for_nav=%d, window=\"%s\", layer=%d\n", init_for_nav, window->Name, g.NavLayer);
    if (init_for_nav)
    {
        SetNavID(0, g.NavLayer, window->NavRootFocusScopeId, ImRect());
        g.NavInitRequest = true;
        g.NavInitRequestFromMove = false;
        g.NavInitResult.ID = 0;
        NavUpdateAnyRequestFlag();
    }
    else
    {
        g.NavId = window->NavLastIds[0];
        SetNavFocusScope(window->NavRootFocusScopeId);
    }
}

static ImGuiInputSource ImGui::NavCalcPreferredRefPosSource(ImGuiWindowFlags window_type)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.NavWindow;

    const bool activated_shortcut = g.ActiveId != 0 && g.ActiveIdFromShortcut && g.ActiveId == g.LastItemData.ID;
    if ((window_type & ImGuiWindowFlags_Popup) && activated_shortcut)
        return ImGuiInputSource_Keyboard;

    if (!g.NavCursorVisible || !g.NavHighlightItemUnderNav || !window)
        return ImGuiInputSource_Mouse;
    else
        return ImGuiInputSource_Keyboard;
}

static ImVec2 ImGui::NavCalcPreferredRefPos(ImGuiWindowFlags window_type)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.NavWindow;
    ImGuiInputSource source = NavCalcPreferredRefPosSource(window_type);

    if (source == ImGuiInputSource_Mouse)
    {

        ImVec2 p = IsMousePosValid(&g.IO.MousePos) ? g.IO.MousePos : g.MouseLastValidPos;
        return ImVec2(p.x + 1.0f, p.y);
    }
    else
    {

        const bool activated_shortcut = g.ActiveId != 0 && g.ActiveIdFromShortcut && g.ActiveId == g.LastItemData.ID;
        ImRect ref_rect;
        if (activated_shortcut && (window_type & ImGuiWindowFlags_Popup))
            ref_rect = g.LastItemData.NavRect;
        else if (window != NULL)
            ref_rect = WindowRectRelToAbs(window, window->NavRectRel[g.NavLayer]);

        if (window != NULL && window->LastFrameActive != g.FrameCount && (window->ScrollTarget.x != FLT_MAX || window->ScrollTarget.y != FLT_MAX))
        {
            ImVec2 next_scroll = CalcNextScrollFromScrollTargetAndClamp(window);
            ref_rect.Translate(window->Scroll - next_scroll);
        }
        ImVec2 pos = ImVec2(ref_rect.Min.x + ImMin(g.Style.FramePadding.x * 4, ref_rect.GetWidth()), ref_rect.Max.y - ImMin(g.Style.FramePadding.y, ref_rect.GetHeight()));
        ImGuiViewport* viewport = GetMainViewport();
        return ImTrunc(ImClamp(pos, viewport->Pos, viewport->Pos + viewport->Size));
    }
}

float ImGui::GetNavTweakPressedAmount(ImGuiAxis axis)
{
    ImGuiContext& g = *GImGui;
    float repeat_delay, repeat_rate;
    GetTypematicRepeatRate(ImGuiInputFlags_RepeatRateNavTweak, &repeat_delay, &repeat_rate);

    ImGuiKey key_less, key_more;
    if (g.NavInputSource == ImGuiInputSource_Gamepad)
    {
        key_less = (axis == ImGuiAxis_X) ? ImGuiKey_GamepadDpadLeft : ImGuiKey_GamepadDpadUp;
        key_more = (axis == ImGuiAxis_X) ? ImGuiKey_GamepadDpadRight : ImGuiKey_GamepadDpadDown;
    }
    else
    {
        key_less = (axis == ImGuiAxis_X) ? ImGuiKey_LeftArrow : ImGuiKey_UpArrow;
        key_more = (axis == ImGuiAxis_X) ? ImGuiKey_RightArrow : ImGuiKey_DownArrow;
    }
    float amount = (float)GetKeyPressedAmount(key_more, repeat_delay, repeat_rate) - (float)GetKeyPressedAmount(key_less, repeat_delay, repeat_rate);
    if (amount != 0.0f && IsKeyDown(key_less) && IsKeyDown(key_more))
        amount = 0.0f;
    return amount;
}

static void ImGui::NavUpdate()
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    io.WantSetMousePos = false;

    const bool nav_gamepad_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
    const ImGuiKey nav_gamepad_keys_to_change_source[] = { ImGuiKey_GamepadFaceRight, ImGuiKey_GamepadFaceLeft, ImGuiKey_GamepadFaceUp, ImGuiKey_GamepadFaceDown, ImGuiKey_GamepadDpadRight, ImGuiKey_GamepadDpadLeft, ImGuiKey_GamepadDpadUp, ImGuiKey_GamepadDpadDown };
    if (nav_gamepad_active)
        for (ImGuiKey key : nav_gamepad_keys_to_change_source)
            if (IsKeyDown(key))
                g.NavInputSource = ImGuiInputSource_Gamepad;
    const bool nav_keyboard_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0;
    const ImGuiKey nav_keyboard_keys_to_change_source[] = { ImGuiKey_Space, ImGuiKey_Enter, ImGuiKey_Escape, ImGuiKey_RightArrow, ImGuiKey_LeftArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow };
    if (nav_keyboard_active)
        for (ImGuiKey key : nav_keyboard_keys_to_change_source)
            if (IsKeyDown(key))
                g.NavInputSource = ImGuiInputSource_Keyboard;

    g.NavJustMovedToId = 0;
    g.NavJustMovedToFocusScopeId = g.NavJustMovedFromFocusScopeId = 0;
    if (g.NavInitResult.ID != 0)
        NavInitRequestApplyResult();
    g.NavInitRequest = false;
    g.NavInitRequestFromMove = false;
    g.NavInitResult.ID = 0;

    if (g.NavMoveSubmitted)
        NavMoveRequestApplyResult();
    g.NavTabbingCounter = 0;
    g.NavMoveSubmitted = g.NavMoveScoringItems = false;
    if (g.NavCursorHideFrames > 0)
        if (--g.NavCursorHideFrames == 0)
            g.NavCursorVisible = true;

    bool set_mouse_pos = false;
    if (g.NavMousePosDirty && g.NavIdIsAlive)
        if (g.NavCursorVisible && g.NavHighlightItemUnderNav && g.NavWindow)
            set_mouse_pos = true;
    g.NavMousePosDirty = false;
    IM_ASSERT(g.NavLayer == ImGuiNavLayer_Main || g.NavLayer == ImGuiNavLayer_Menu);

    if (g.NavWindow)
        NavSaveLastChildNavWindowIntoParent(g.NavWindow);
    if (g.NavWindow && g.NavWindow->NavLastChildNavWindow != NULL && g.NavLayer == ImGuiNavLayer_Main)
        g.NavWindow->NavLastChildNavWindow = NULL;

    NavUpdateWindowing();

    io.NavActive = (nav_keyboard_active || nav_gamepad_active) && g.NavWindow && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs);
    io.NavVisible = (io.NavActive && g.NavId != 0 && g.NavCursorVisible) || (g.NavWindowingTarget != NULL);

    NavUpdateCancelRequest();
    NavUpdateContextMenuRequest();

    g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId = 0;
    g.NavActivateFlags = ImGuiActivateFlags_None;
    if (g.NavId != 0 && g.NavCursorVisible && !g.NavWindowingTarget && g.NavWindow && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
    {
        const bool activate_down = (nav_keyboard_active && IsKeyDown(ImGuiKey_Space, ImGuiKeyOwner_NoOwner)) || (nav_gamepad_active && IsKeyDown(ImGuiKey_NavGamepadActivate, ImGuiKeyOwner_NoOwner));
        const bool activate_pressed = activate_down && ((nav_keyboard_active && IsKeyPressed(ImGuiKey_Space, 0, ImGuiKeyOwner_NoOwner)) || (nav_gamepad_active && IsKeyPressed(ImGuiKey_NavGamepadActivate, 0, ImGuiKeyOwner_NoOwner)));
        const bool input_pressed_keyboard = nav_keyboard_active && (IsKeyPressed(ImGuiKey_Enter, 0, ImGuiKeyOwner_NoOwner) || IsKeyPressed(ImGuiKey_KeypadEnter, 0, ImGuiKeyOwner_NoOwner));
        bool input_pressed_gamepad = false;
        if (activate_down && nav_gamepad_active && IsKeyDown(ImGuiKey_NavGamepadActivate, ImGuiKeyOwner_NoOwner) && (g.NavIdItemFlags & ImGuiItemFlags_Inputable))
            if (GetKeyData(ImGuiKey_NavGamepadActivate)->DownDurationPrev < NAV_ACTIVATE_INPUT_WITH_GAMEPAD_DELAY && GetKeyData(ImGuiKey_NavGamepadActivate)->DownDuration >= NAV_ACTIVATE_INPUT_WITH_GAMEPAD_DELAY)
                input_pressed_gamepad = true;

        if (g.ActiveId == 0 && activate_pressed)
        {
            g.NavActivateId = g.NavId;
            g.NavActivateFlags = ImGuiActivateFlags_PreferTweak;
        }
        if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && (input_pressed_keyboard || input_pressed_gamepad))
        {
            g.NavActivateId = g.NavId;
            g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
        }
        if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && (activate_down || input_pressed_keyboard || input_pressed_gamepad))
            g.NavActivateDownId = g.NavId;
        if ((g.ActiveId == 0 || g.ActiveId == g.NavId) && (activate_pressed || input_pressed_keyboard || input_pressed_gamepad))
        {
            g.NavActivatePressedId = g.NavId;
            NavHighlightActivated(g.NavId);
        }
    }
    if (g.NavWindow && (g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
        g.NavCursorVisible = false;
    else if (g.IO.ConfigNavCursorVisibleAlways && g.NavCursorHideFrames == 0)
        g.NavCursorVisible = true;
    if (g.NavActivateId != 0)
        IM_ASSERT(g.NavActivateDownId == g.NavActivateId);

    if (g.NavHighlightActivatedTimer > 0.0f)
        g.NavHighlightActivatedTimer = ImMax(0.0f, g.NavHighlightActivatedTimer - io.DeltaTime);
    if (g.NavHighlightActivatedTimer == 0.0f)
        g.NavHighlightActivatedId = 0;

    if (g.NavNextActivateId != 0)
    {
        g.NavActivateId = g.NavActivateDownId = g.NavActivatePressedId = g.NavNextActivateId;
        g.NavActivateFlags = g.NavNextActivateFlags;
    }
    g.NavNextActivateId = 0;

    NavUpdateCreateMoveRequest();
    if (g.NavMoveDir == ImGuiDir_None)
        NavUpdateCreateTabbingRequest();
    NavUpdateAnyRequestFlag();
    g.NavIdIsAlive = false;

    if (g.NavWindow && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs) && !g.NavWindowingTarget)
    {

        ImGuiWindow* window = g.NavWindow;
        const float scroll_speed = IM_ROUND(window->FontRefSize * 100 * io.DeltaTime);
        const ImGuiDir move_dir = g.NavMoveDir;
        if (window->DC.NavLayersActiveMask == 0x00 && window->DC.NavWindowHasScrollY && move_dir != ImGuiDir_None)
        {
            if (move_dir == ImGuiDir_Left || move_dir == ImGuiDir_Right)
                SetScrollX(window, ImTrunc(window->Scroll.x + ((move_dir == ImGuiDir_Left) ? -1.0f : +1.0f) * scroll_speed));
            if (move_dir == ImGuiDir_Up || move_dir == ImGuiDir_Down)
                SetScrollY(window, ImTrunc(window->Scroll.y + ((move_dir == ImGuiDir_Up) ? -1.0f : +1.0f) * scroll_speed));
        }

        if (nav_gamepad_active)
        {
            const ImVec2 scroll_dir = GetKeyMagnitude2d(ImGuiKey_GamepadLStickLeft, ImGuiKey_GamepadLStickRight, ImGuiKey_GamepadLStickUp, ImGuiKey_GamepadLStickDown);
            const float tweak_factor = IsKeyDown(ImGuiKey_NavGamepadTweakSlow) ? 1.0f / 10.0f : IsKeyDown(ImGuiKey_NavGamepadTweakFast) ? 10.0f : 1.0f;
            if (scroll_dir.x != 0.0f && window->ScrollbarX)
                SetScrollX(window, ImTrunc(window->Scroll.x + scroll_dir.x * scroll_speed * tweak_factor));
            if (scroll_dir.y != 0.0f)
                SetScrollY(window, ImTrunc(window->Scroll.y + scroll_dir.y * scroll_speed * tweak_factor));
        }
    }

    if (!nav_keyboard_active && !nav_gamepad_active)
    {
        g.NavCursorVisible = false;
        g.NavHighlightItemUnderNav = set_mouse_pos = false;
    }

    if (set_mouse_pos && io.ConfigNavMoveSetMousePos && (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos))
        TeleportMousePos(NavCalcPreferredRefPos(ImGuiWindowFlags_Popup));

    g.NavScoringDebugCount = 0;
#if IMGUI_DEBUG_NAV_RECTS
    if (ImGuiWindow* debug_window = g.NavWindow)
    {
        ImDrawList* draw_list = GetForegroundDrawList(debug_window);
        int layer = g.NavLayer;  { ImRect r = WindowRectRelToAbs(debug_window, debug_window->NavRectRel[layer]); draw_list->AddRect(r.Min, r.Max, IM_COL32(255, 200, 0, 255)); }

    }
#endif
}

void ImGui::NavInitRequestApplyResult()
{

    ImGuiContext& g = *GImGui;
    if (!g.NavWindow)
        return;

    ImGuiNavItemData* result = &g.NavInitResult;
    if (g.NavId != result->ID)
    {
        g.NavJustMovedFromFocusScopeId = g.NavFocusScopeId;
        g.NavJustMovedToId = result->ID;
        g.NavJustMovedToFocusScopeId = result->FocusScopeId;
        g.NavJustMovedToKeyMods = 0;
        g.NavJustMovedToIsTabbing = false;
        g.NavJustMovedToHasSelectionData = (result->ItemFlags & ImGuiItemFlags_HasSelectionUserData) != 0;
    }

    IMGUI_DEBUG_LOG_NAV("[nav] NavInitRequest: ApplyResult: NavID 0x%08X in Layer %d Window \"%s\"\n", result->ID, g.NavLayer, g.NavWindow->Name);
    SetNavID(result->ID, g.NavLayer, result->FocusScopeId, result->RectRel);
    g.NavIdIsAlive = true;
    if (result->SelectionUserData != ImGuiSelectionUserData_Invalid)
        g.NavLastValidSelectionUserData = result->SelectionUserData;
    if (g.NavInitRequestFromMove)
        SetNavCursorVisibleAfterMove();
}

static void NavBiasScoringRect(ImRect& r, ImVec2& preferred_pos_rel, ImGuiDir move_dir, ImGuiNavMoveFlags move_flags)
{

    ImGuiContext& g = *GImGui;
    const ImVec2 rel_to_abs_offset = g.NavWindow->DC.CursorStartPos;

    if ((move_flags & ImGuiNavMoveFlags_Forwarded) == 0)
    {
        if (preferred_pos_rel.x == FLT_MAX)
            preferred_pos_rel.x = ImMin(r.Min.x + 1.0f, r.Max.x) - rel_to_abs_offset.x;
        if (preferred_pos_rel.y == FLT_MAX)
            preferred_pos_rel.y = r.GetCenter().y - rel_to_abs_offset.y;
    }

    if ((move_dir == ImGuiDir_Up || move_dir == ImGuiDir_Down) && preferred_pos_rel.x != FLT_MAX)
        r.Min.x = r.Max.x = preferred_pos_rel.x + rel_to_abs_offset.x;
    else if ((move_dir == ImGuiDir_Left || move_dir == ImGuiDir_Right) && preferred_pos_rel.y != FLT_MAX)
        r.Min.y = r.Max.y = preferred_pos_rel.y + rel_to_abs_offset.y;
}

void ImGui::NavUpdateCreateMoveRequest()
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    ImGuiWindow* window = g.NavWindow;
    const bool nav_gamepad_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
    const bool nav_keyboard_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0;

    if (g.NavMoveForwardToNextFrame && window != NULL)
    {

        IM_ASSERT(g.NavMoveDir != ImGuiDir_None && g.NavMoveClipDir != ImGuiDir_None);
        IM_ASSERT(g.NavMoveFlags & ImGuiNavMoveFlags_Forwarded);
        IMGUI_DEBUG_LOG_NAV("[nav] NavMoveRequestForward %d\n", g.NavMoveDir);
    }
    else
    {

        g.NavMoveDir = ImGuiDir_None;
        g.NavMoveFlags = ImGuiNavMoveFlags_None;
        g.NavMoveScrollFlags = ImGuiScrollFlags_None;
        if (window && !g.NavWindowingTarget && !(window->Flags & ImGuiWindowFlags_NoNavInputs))
        {
            const ImGuiInputFlags repeat_mode = ImGuiInputFlags_Repeat | (ImGuiInputFlags)ImGuiInputFlags_RepeatRateNavMove;
            if (!IsActiveIdUsingNavDir(ImGuiDir_Left)  && ((nav_gamepad_active && IsKeyPressed(ImGuiKey_GamepadDpadLeft,  repeat_mode, ImGuiKeyOwner_NoOwner)) || (nav_keyboard_active && IsKeyPressed(ImGuiKey_LeftArrow,  repeat_mode, ImGuiKeyOwner_NoOwner)))) { g.NavMoveDir = ImGuiDir_Left; }
            if (!IsActiveIdUsingNavDir(ImGuiDir_Right) && ((nav_gamepad_active && IsKeyPressed(ImGuiKey_GamepadDpadRight, repeat_mode, ImGuiKeyOwner_NoOwner)) || (nav_keyboard_active && IsKeyPressed(ImGuiKey_RightArrow, repeat_mode, ImGuiKeyOwner_NoOwner)))) { g.NavMoveDir = ImGuiDir_Right; }
            if (!IsActiveIdUsingNavDir(ImGuiDir_Up)    && ((nav_gamepad_active && IsKeyPressed(ImGuiKey_GamepadDpadUp,    repeat_mode, ImGuiKeyOwner_NoOwner)) || (nav_keyboard_active && IsKeyPressed(ImGuiKey_UpArrow,    repeat_mode, ImGuiKeyOwner_NoOwner)))) { g.NavMoveDir = ImGuiDir_Up; }
            if (!IsActiveIdUsingNavDir(ImGuiDir_Down)  && ((nav_gamepad_active && IsKeyPressed(ImGuiKey_GamepadDpadDown,  repeat_mode, ImGuiKeyOwner_NoOwner)) || (nav_keyboard_active && IsKeyPressed(ImGuiKey_DownArrow,  repeat_mode, ImGuiKeyOwner_NoOwner)))) { g.NavMoveDir = ImGuiDir_Down; }
        }
        g.NavMoveClipDir = g.NavMoveDir;
        g.NavScoringNoClipRect = ImRect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX);
    }

    float scoring_page_offset_y = 0.0f;
    if (window && g.NavMoveDir == ImGuiDir_None && nav_keyboard_active)
        scoring_page_offset_y = NavUpdatePageUpPageDown();

#if IMGUI_DEBUG_NAV_SCORING

    if (io.KeyCtrl)
    {
        if (g.NavMoveDir == ImGuiDir_None)
            g.NavMoveDir = g.NavMoveDirForDebug;
        g.NavMoveClipDir = g.NavMoveDir;
        g.NavMoveFlags |= ImGuiNavMoveFlags_DebugNoResult;
    }
#endif

    g.NavMoveForwardToNextFrame = false;
    if (g.NavMoveDir != ImGuiDir_None)
        NavMoveRequestSubmit(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags, g.NavMoveScrollFlags);

    if (g.NavMoveSubmitted && g.NavId == 0)
    {
        IMGUI_DEBUG_LOG_NAV("[nav] NavInitRequest: from move, window \"%s\", layer=%d\n", window ? window->Name : "<NULL>", g.NavLayer);
        g.NavInitRequest = g.NavInitRequestFromMove = true;
        g.NavInitResult.ID = 0;
        if (g.IO.ConfigNavCursorVisibleAuto)
            g.NavCursorVisible = true;
    }

    if (g.NavMoveSubmitted && g.NavInputSource == ImGuiInputSource_Gamepad && g.NavLayer == ImGuiNavLayer_Main && window != NULL)
    {
        bool clamp_x = (g.NavMoveFlags & (ImGuiNavMoveFlags_LoopX | ImGuiNavMoveFlags_WrapX)) == 0;
        bool clamp_y = (g.NavMoveFlags & (ImGuiNavMoveFlags_LoopY | ImGuiNavMoveFlags_WrapY)) == 0;
        ImRect inner_rect_rel = WindowRectAbsToRel(window, ImRect(window->InnerRect.Min - ImVec2(1, 1), window->InnerRect.Max + ImVec2(1, 1)));

        inner_rect_rel.Translate(CalcNextScrollFromScrollTargetAndClamp(window) - window->Scroll);

        if ((clamp_x || clamp_y) && !inner_rect_rel.Contains(window->NavRectRel[g.NavLayer]))
        {
            IMGUI_DEBUG_LOG_NAV("[nav] NavMoveRequest: clamp NavRectRel for gamepad move\n");
            float pad_x = ImMin(inner_rect_rel.GetWidth(), window->FontRefSize * 0.5f);
            float pad_y = ImMin(inner_rect_rel.GetHeight(), window->FontRefSize * 0.5f);
            inner_rect_rel.Min.x = clamp_x ? (inner_rect_rel.Min.x + pad_x) : -FLT_MAX;
            inner_rect_rel.Max.x = clamp_x ? (inner_rect_rel.Max.x - pad_x) : +FLT_MAX;
            inner_rect_rel.Min.y = clamp_y ? (inner_rect_rel.Min.y + pad_y) : -FLT_MAX;
            inner_rect_rel.Max.y = clamp_y ? (inner_rect_rel.Max.y - pad_y) : +FLT_MAX;
            window->NavRectRel[g.NavLayer].ClipWithFull(inner_rect_rel);
            g.NavId = 0;
        }
    }

    ImRect scoring_rect;
    if (window != NULL)
    {
        ImRect nav_rect_rel = !window->NavRectRel[g.NavLayer].IsInverted() ? window->NavRectRel[g.NavLayer] : ImRect(0, 0, 0, 0);
        scoring_rect = WindowRectRelToAbs(window, nav_rect_rel);

        if (g.NavMoveFlags & ImGuiNavMoveFlags_IsPageMove)
        {

            if (window->InnerRect.Contains(scoring_rect))
                g.NavMoveFlags |= ImGuiNavMoveFlags_AlsoScoreVisibleSet;
            g.NavScoringNoClipRect = scoring_rect;
            scoring_rect.TranslateY(scoring_page_offset_y);
            g.NavScoringNoClipRect.Add(scoring_rect);
        }

        if (g.NavMoveSubmitted)
            NavBiasScoringRect(scoring_rect, window->RootWindowForNav->NavPreferredScoringPosRel[g.NavLayer], g.NavMoveDir, g.NavMoveFlags);
        IM_ASSERT(!scoring_rect.IsInverted());

    }
    g.NavScoringRect = scoring_rect;

}

void ImGui::NavUpdateCreateTabbingRequest()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.NavWindow;
    IM_ASSERT(g.NavMoveDir == ImGuiDir_None);
    if (window == NULL || g.NavWindowingTarget != NULL || (window->Flags & ImGuiWindowFlags_NoNavInputs) || !g.ConfigNavEnableTabbing)
        return;

    const bool tab_pressed = IsKeyPressed(ImGuiKey_Tab, ImGuiInputFlags_Repeat, ImGuiKeyOwner_NoOwner) && !g.IO.KeyCtrl && !g.IO.KeyAlt;
    if (!tab_pressed)
        return;

    const bool nav_keyboard_active = (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0;
    if (nav_keyboard_active)
        g.NavTabbingDir = g.IO.KeyShift ? -1 : (g.NavCursorVisible == false && g.ActiveId == 0) ? 0 : +1;
    else
        g.NavTabbingDir = g.IO.KeyShift ? -1 : (g.ActiveId == 0) ? 0 : +1;
    ImGuiNavMoveFlags move_flags = ImGuiNavMoveFlags_IsTabbing | ImGuiNavMoveFlags_Activate;
    ImGuiScrollFlags scroll_flags = window->Appearing ? ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_AlwaysCenterY : ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleEdgeY;
    ImGuiDir clip_dir = (g.NavTabbingDir < 0) ? ImGuiDir_Up : ImGuiDir_Down;
    NavMoveRequestSubmit(ImGuiDir_None, clip_dir, move_flags, scroll_flags);
    g.NavTabbingCounter = -1;
}

void ImGui::NavMoveRequestApplyResult()
{
    ImGuiContext& g = *GImGui;
#if IMGUI_DEBUG_NAV_SCORING
    if (g.NavMoveFlags & ImGuiNavMoveFlags_DebugNoResult)
        return;
#endif

    ImGuiNavItemData* result = (g.NavMoveResultLocal.ID != 0) ? &g.NavMoveResultLocal : (g.NavMoveResultOther.ID != 0) ? &g.NavMoveResultOther : NULL;

    if ((g.NavMoveFlags & ImGuiNavMoveFlags_IsTabbing) && result == NULL)
        if ((g.NavTabbingCounter == 1 || g.NavTabbingDir == 0) && g.NavTabbingResultFirst.ID)
            result = &g.NavTabbingResultFirst;

    const ImGuiAxis axis = (g.NavMoveDir == ImGuiDir_Up || g.NavMoveDir == ImGuiDir_Down) ? ImGuiAxis_Y : ImGuiAxis_X;
    if (result == NULL)
    {
        if (g.NavMoveFlags & ImGuiNavMoveFlags_IsTabbing)
            g.NavMoveFlags |= ImGuiNavMoveFlags_NoSetNavCursorVisible;
        if (g.NavId != 0 && (g.NavMoveFlags & ImGuiNavMoveFlags_NoSetNavCursorVisible) == 0)
            SetNavCursorVisibleAfterMove();
        NavClearPreferredPosForAxis(axis);
        IMGUI_DEBUG_LOG_NAV("[nav] NavMoveSubmitted but not led to a result!\n");
        return;
    }

    if (g.NavMoveFlags & ImGuiNavMoveFlags_AlsoScoreVisibleSet)
        if (g.NavMoveResultLocalVisible.ID != 0 && g.NavMoveResultLocalVisible.ID != g.NavId)
            result = &g.NavMoveResultLocalVisible;

    if (result != &g.NavMoveResultOther && g.NavMoveResultOther.ID != 0 && g.NavMoveResultOther.Window->ParentWindow == g.NavWindow)
        if ((g.NavMoveResultOther.DistBox < result->DistBox) || (g.NavMoveResultOther.DistBox == result->DistBox && g.NavMoveResultOther.DistCenter < result->DistCenter))
            result = &g.NavMoveResultOther;
    IM_ASSERT(g.NavWindow && result->Window);

    if (g.NavLayer == ImGuiNavLayer_Main)
    {
        ImRect rect_abs = WindowRectRelToAbs(result->Window, result->RectRel);
        ScrollToRectEx(result->Window, rect_abs, g.NavMoveScrollFlags);

        if (g.NavMoveFlags & ImGuiNavMoveFlags_ScrollToEdgeY)
        {

            float scroll_target = (g.NavMoveDir == ImGuiDir_Up) ? result->Window->ScrollMax.y : 0.0f;
            SetScrollY(result->Window, scroll_target);
        }
    }

    if (g.NavWindow != result->Window)
    {
        IMGUI_DEBUG_LOG_FOCUS("[focus] NavMoveRequest: SetNavWindow(\"%s\")\n", result->Window->Name);
        g.NavWindow = result->Window;
        g.NavLastValidSelectionUserData = ImGuiSelectionUserData_Invalid;
    }

    if (g.ActiveId != result->ID && (g.NavMoveFlags & ImGuiNavMoveFlags_NoClearActiveId) == 0)
        ClearActiveID();

    if ((g.NavId != result->ID || (g.NavMoveFlags & ImGuiNavMoveFlags_IsPageMove)) && (g.NavMoveFlags & ImGuiNavMoveFlags_NoSelect) == 0)
    {
        g.NavJustMovedFromFocusScopeId = g.NavFocusScopeId;
        g.NavJustMovedToId = result->ID;
        g.NavJustMovedToFocusScopeId = result->FocusScopeId;
        g.NavJustMovedToKeyMods = g.NavMoveKeyMods;
        g.NavJustMovedToIsTabbing = (g.NavMoveFlags & ImGuiNavMoveFlags_IsTabbing) != 0;
        g.NavJustMovedToHasSelectionData = (result->ItemFlags & ImGuiItemFlags_HasSelectionUserData) != 0;

    }

    IMGUI_DEBUG_LOG_NAV("[nav] NavMoveRequest: result NavID 0x%08X in Layer %d Window \"%s\"\n", result->ID, g.NavLayer, g.NavWindow->Name);
    ImVec2 preferred_scoring_pos_rel = g.NavWindow->RootWindowForNav->NavPreferredScoringPosRel[g.NavLayer];
    SetNavID(result->ID, g.NavLayer, result->FocusScopeId, result->RectRel);
    if (result->SelectionUserData != ImGuiSelectionUserData_Invalid)
        g.NavLastValidSelectionUserData = result->SelectionUserData;

    if ((g.NavMoveFlags & ImGuiNavMoveFlags_IsTabbing) == 0)
    {
        preferred_scoring_pos_rel[axis] = result->RectRel.GetCenter()[axis];
        g.NavWindow->RootWindowForNav->NavPreferredScoringPosRel[g.NavLayer] = preferred_scoring_pos_rel;
    }

    if ((g.NavMoveFlags & ImGuiNavMoveFlags_IsTabbing) && (result->ItemFlags & ImGuiItemFlags_Inputable) == 0)
        g.NavMoveFlags &= ~ImGuiNavMoveFlags_Activate;

    if (g.NavMoveFlags & ImGuiNavMoveFlags_Activate)
    {
        g.NavNextActivateId = result->ID;
        g.NavNextActivateFlags = ImGuiActivateFlags_None;
        if (g.NavMoveFlags & ImGuiNavMoveFlags_FocusApi)
            g.NavNextActivateFlags |= ImGuiActivateFlags_FromFocusApi;
        if (g.NavMoveFlags & ImGuiNavMoveFlags_IsTabbing)
            g.NavNextActivateFlags |= ImGuiActivateFlags_PreferInput | ImGuiActivateFlags_TryToPreserveState | ImGuiActivateFlags_FromTabbing;
    }

    if ((g.NavMoveFlags & ImGuiNavMoveFlags_NoSetNavCursorVisible) == 0)
        SetNavCursorVisibleAfterMove();
}

static void ImGui::NavUpdateCancelRequest()
{
    ImGuiContext& g = *GImGui;
    const bool nav_gamepad_active = (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 && (g.IO.BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
    const bool nav_keyboard_active = (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0;
    if (!(nav_keyboard_active && IsKeyPressed(ImGuiKey_Escape, 0, ImGuiKeyOwner_NoOwner)) && !(nav_gamepad_active && IsKeyPressed(ImGuiKey_NavGamepadCancel, 0, ImGuiKeyOwner_NoOwner)))
        return;

    IMGUI_DEBUG_LOG_NAV("[nav] NavUpdateCancelRequest()\n");
    if (g.ActiveId != 0)
    {
        ClearActiveID();
    }
    else if (g.NavLayer != ImGuiNavLayer_Main)
    {

        NavRestoreLayer(ImGuiNavLayer_Main);
        SetNavCursorVisibleAfterMove();
    }
    else if (g.NavWindow && g.NavWindow != g.NavWindow->RootWindow && !(g.NavWindow->RootWindowForNav->Flags & ImGuiWindowFlags_Popup) && g.NavWindow->RootWindowForNav->ParentWindow)
    {

        ImGuiWindow* child_window = g.NavWindow->RootWindowForNav;
        ImGuiWindow* parent_window = child_window->ParentWindow;
        IM_ASSERT(child_window->ChildId != 0);
        FocusWindow(parent_window);
        SetNavID(child_window->ChildId, ImGuiNavLayer_Main, 0, WindowRectAbsToRel(parent_window, child_window->Rect()));
        SetNavCursorVisibleAfterMove();
    }
    else if (g.OpenPopupStack.Size > 0 && g.OpenPopupStack.back().Window != NULL && !(g.OpenPopupStack.back().Window->Flags & ImGuiWindowFlags_Modal))
    {

        ClosePopupToLevel(g.OpenPopupStack.Size - 1, true);
    }
    else
    {

        if (g.IO.ConfigNavEscapeClearFocusItem || g.IO.ConfigNavEscapeClearFocusWindow)
            if (g.NavWindow && ((g.NavWindow->Flags & ImGuiWindowFlags_Popup)))
                g.NavWindow->NavLastIds[0] = 0;

        if (g.IO.ConfigNavEscapeClearFocusItem || g.IO.ConfigNavEscapeClearFocusWindow)
            g.NavId = 0;
        if (g.IO.ConfigNavEscapeClearFocusWindow)
            FocusWindow(NULL);
    }
}

static void ImGui::NavUpdateContextMenuRequest()
{
    ImGuiContext& g = *GImGui;
    g.NavOpenContextMenuItemId = g.NavOpenContextMenuWindowId = 0;
    const bool nav_keyboard_active = (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0;
    const bool nav_gamepad_active = (g.IO.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0;
    if ((!nav_keyboard_active && !nav_gamepad_active) || g.NavWindow == NULL)
        return;

    bool request = false;
    request |= nav_keyboard_active && (IsKeyReleased(ImGuiKey_Menu, ImGuiKeyOwner_NoOwner) || (IsKeyPressed(ImGuiKey_F10, ImGuiInputFlags_None, ImGuiKeyOwner_NoOwner) && g.IO.KeyMods == ImGuiMod_Shift));
    request |= nav_gamepad_active && IsKeyPressed(ImGuiKey_NavGamepadContextMenu, ImGuiInputFlags_None, ImGuiKeyOwner_NoOwner);
    if (!request)
        return;
    g.NavOpenContextMenuItemId = g.NavId;
    g.NavOpenContextMenuWindowId = g.NavWindow->ID;

    if (g.NavId == g.NavWindow->GetID("#CLOSE") || g.NavId == g.NavWindow->GetID("#COLLAPSE"))
        g.NavOpenContextMenuItemId = g.NavWindow->MoveId;

    g.NavInputSource = ImGuiInputSource_Keyboard;
    SetNavCursorVisibleAfterMove();
}

static float ImGui::NavUpdatePageUpPageDown()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.NavWindow;
    if ((window->Flags & ImGuiWindowFlags_NoNavInputs) || g.NavWindowingTarget != NULL)
        return 0.0f;

    const bool page_up_held = IsKeyDown(ImGuiKey_PageUp, ImGuiKeyOwner_NoOwner);
    const bool page_down_held = IsKeyDown(ImGuiKey_PageDown, ImGuiKeyOwner_NoOwner);
    const bool home_pressed = IsKeyPressed(ImGuiKey_Home, ImGuiInputFlags_Repeat, ImGuiKeyOwner_NoOwner);
    const bool end_pressed = IsKeyPressed(ImGuiKey_End, ImGuiInputFlags_Repeat, ImGuiKeyOwner_NoOwner);
    if (page_up_held == page_down_held && home_pressed == end_pressed)
        return 0.0f;

    if (g.NavLayer != ImGuiNavLayer_Main)
        NavRestoreLayer(ImGuiNavLayer_Main);

    if ((window->DC.NavLayersActiveMask & (1 << ImGuiNavLayer_Main)) == 0 && window->DC.NavWindowHasScrollY)
    {

        if (IsKeyPressed(ImGuiKey_PageUp, ImGuiInputFlags_Repeat, ImGuiKeyOwner_NoOwner))
            SetScrollY(window, window->Scroll.y - window->InnerRect.GetHeight());
        else if (IsKeyPressed(ImGuiKey_PageDown, ImGuiInputFlags_Repeat, ImGuiKeyOwner_NoOwner))
            SetScrollY(window, window->Scroll.y + window->InnerRect.GetHeight());
        else if (home_pressed)
            SetScrollY(window, 0.0f);
        else if (end_pressed)
            SetScrollY(window, window->ScrollMax.y);
    }
    else
    {
        ImRect& nav_rect_rel = window->NavRectRel[g.NavLayer];
        const float page_offset_y = ImMax(0.0f, window->InnerRect.GetHeight() - window->FontRefSize * 1.0f + nav_rect_rel.GetHeight());
        float nav_scoring_rect_offset_y = 0.0f;
        if (IsKeyPressed(ImGuiKey_PageUp, true))
        {
            nav_scoring_rect_offset_y = -page_offset_y;
            g.NavMoveDir = ImGuiDir_Down;
            g.NavMoveClipDir = ImGuiDir_Up;
            g.NavMoveFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_IsPageMove;
        }
        else if (IsKeyPressed(ImGuiKey_PageDown, true))
        {
            nav_scoring_rect_offset_y = +page_offset_y;
            g.NavMoveDir = ImGuiDir_Up;
            g.NavMoveClipDir = ImGuiDir_Down;
            g.NavMoveFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_IsPageMove;
        }
        else if (home_pressed)
        {

            nav_rect_rel.Min.y = nav_rect_rel.Max.y = 0.0f;
            if (nav_rect_rel.IsInverted())
                nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
            g.NavMoveDir = ImGuiDir_Down;
            g.NavMoveFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_ScrollToEdgeY;

        }
        else if (end_pressed)
        {
            nav_rect_rel.Min.y = nav_rect_rel.Max.y = window->ContentSize.y;
            if (nav_rect_rel.IsInverted())
                nav_rect_rel.Min.x = nav_rect_rel.Max.x = 0.0f;
            g.NavMoveDir = ImGuiDir_Up;
            g.NavMoveFlags = ImGuiNavMoveFlags_AllowCurrentNavId | ImGuiNavMoveFlags_ScrollToEdgeY;

        }
        return nav_scoring_rect_offset_y;
    }
    return 0.0f;
}

static void ImGui::NavEndFrame()
{
    ImGuiContext& g = *GImGui;

    if (g.NavWindowingTarget != NULL)
        NavUpdateWindowingOverlay();

    if (g.NavWindow && NavMoveRequestButNoResultYet() && (g.NavMoveFlags & ImGuiNavMoveFlags_WrapMask_) && (g.NavMoveFlags & ImGuiNavMoveFlags_Forwarded) == 0)
        NavUpdateCreateWrappingRequest();
}

static void ImGui::NavUpdateCreateWrappingRequest()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.NavWindow;

    bool do_forward = false;
    ImRect bb_rel = window->NavRectRel[g.NavLayer];
    ImGuiDir clip_dir = g.NavMoveDir;

    const ImGuiNavMoveFlags move_flags = g.NavMoveFlags;

    ImVec2 wrap_size = (g.NavLayer == ImGuiNavLayer_Menu) ? window->Size : window->ContentSize + window->WindowPadding;

    if (g.NavMoveDir == ImGuiDir_Left && (move_flags & (ImGuiNavMoveFlags_WrapX | ImGuiNavMoveFlags_LoopX)))
    {
        bb_rel.Min.x = bb_rel.Max.x = wrap_size.x;
        if (move_flags & ImGuiNavMoveFlags_WrapX)
        {
            bb_rel.TranslateY(-bb_rel.GetHeight());
            clip_dir = ImGuiDir_Up;
        }
        do_forward = true;
    }
    if (g.NavMoveDir == ImGuiDir_Right && (move_flags & (ImGuiNavMoveFlags_WrapX | ImGuiNavMoveFlags_LoopX)))
    {
        bb_rel.Min.x = bb_rel.Max.x = -window->WindowPadding.x;
        if (move_flags & ImGuiNavMoveFlags_WrapX)
        {
            bb_rel.TranslateY(+bb_rel.GetHeight());
            clip_dir = ImGuiDir_Down;
        }
        do_forward = true;
    }
    if (g.NavMoveDir == ImGuiDir_Up && (move_flags & (ImGuiNavMoveFlags_WrapY | ImGuiNavMoveFlags_LoopY)))
    {
        bb_rel.Min.y = bb_rel.Max.y = wrap_size.y;
        if (move_flags & ImGuiNavMoveFlags_WrapY)
        {
            bb_rel.TranslateX(-bb_rel.GetWidth());
            clip_dir = ImGuiDir_Left;
        }
        do_forward = true;
    }
    if (g.NavMoveDir == ImGuiDir_Down && (move_flags & (ImGuiNavMoveFlags_WrapY | ImGuiNavMoveFlags_LoopY)))
    {
        bb_rel.Min.y = bb_rel.Max.y = -window->WindowPadding.y;
        if (move_flags & ImGuiNavMoveFlags_WrapY)
        {
            bb_rel.TranslateX(+bb_rel.GetWidth());
            clip_dir = ImGuiDir_Right;
        }
        do_forward = true;
    }
    if (!do_forward)
        return;
    window->NavRectRel[g.NavLayer] = bb_rel;
    NavClearPreferredPosForAxis(ImGuiAxis_X);
    NavClearPreferredPosForAxis(ImGuiAxis_Y);
    NavMoveRequestForward(g.NavMoveDir, clip_dir, move_flags, g.NavMoveScrollFlags);
}

bool ImGui::IsWindowNavFocusable(ImGuiWindow* window)
{
    return window->WasActive && window == window->RootWindow && !(window->Flags & ImGuiWindowFlags_NoNavFocus);
}

static ImGuiWindow* FindWindowNavFocusable(int i_start, int i_stop, int dir)
{
    ImGuiContext& g = *GImGui;
    for (int i = i_start; i >= 0 && i < g.WindowsFocusOrder.Size && i != i_stop; i += dir)
        if (ImGui::IsWindowNavFocusable(g.WindowsFocusOrder[i]))
            return g.WindowsFocusOrder[i];
    return NULL;
}

static void NavUpdateWindowingTarget(int focus_change_dir)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.NavWindowingTarget);
    if (g.NavWindowingTarget->Flags & ImGuiWindowFlags_Modal)
        return;

    const int i_current = ImGui::FindWindowFocusIndex(g.NavWindowingTarget);
    ImGuiWindow* window_target = FindWindowNavFocusable(i_current + focus_change_dir, -INT_MAX, focus_change_dir);
    if (!window_target)
        window_target = FindWindowNavFocusable((focus_change_dir < 0) ? (g.WindowsFocusOrder.Size - 1) : 0, i_current, focus_change_dir);
    if (window_target)
    {
        g.NavWindowingTarget = g.NavWindowingTargetAnim = window_target;
        g.NavWindowingAccumDeltaPos = g.NavWindowingAccumDeltaSize = ImVec2(0.0f, 0.0f);
    }
    g.NavWindowingToggleLayer = false;
}

static void ImGui::NavUpdateWindowingApplyFocus(ImGuiWindow* apply_focus_window)
{
    ImGuiContext& g = *GImGui;
    if (g.NavWindow == NULL || apply_focus_window != g.NavWindow->RootWindow)
    {
        ClearActiveID();
        SetNavCursorVisibleAfterMove();
        ClosePopupsOverWindow(apply_focus_window, false);
        FocusWindow(apply_focus_window, ImGuiFocusRequestFlags_RestoreFocusedChild);
        IM_ASSERT(g.NavWindow != NULL);
        apply_focus_window = g.NavWindow;
        if (apply_focus_window->NavLastIds[0] == 0)
            NavInitWindow(apply_focus_window, false);

        if (apply_focus_window->DC.NavLayersActiveMaskNext == (1 << ImGuiNavLayer_Menu))
            g.NavLayer = ImGuiNavLayer_Menu;
    }
    g.NavWindowingTarget = NULL;
}

static void ImGui::NavUpdateWindowing()
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    ImGuiWindow* apply_focus_window = NULL;
    bool apply_toggle_layer = false;

    ImGuiWindow* modal_window = GetTopMostPopupModal();
    bool allow_windowing = (modal_window == NULL);
    if (!allow_windowing)
        g.NavWindowingTarget = NULL;

    if (g.NavWindowingTargetAnim && g.NavWindowingTarget == NULL)
    {
        g.NavWindowingHighlightAlpha = ImMax(g.NavWindowingHighlightAlpha - io.DeltaTime * 10.0f, 0.0f);
        if (g.DimBgRatio <= 0.0f && g.NavWindowingHighlightAlpha <= 0.0f)
            g.NavWindowingTargetAnim = NULL;
    }

    const ImGuiID owner_id = ImHashStr("##NavUpdateWindowing");
    const bool nav_gamepad_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
    const bool nav_keyboard_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard) != 0;
    const bool keyboard_next_window = allow_windowing && g.ConfigNavWindowingKeyNext && Shortcut(g.ConfigNavWindowingKeyNext, ImGuiInputFlags_Repeat | ImGuiInputFlags_RouteAlways, owner_id);
    const bool keyboard_prev_window = allow_windowing && g.ConfigNavWindowingKeyPrev && Shortcut(g.ConfigNavWindowingKeyPrev, ImGuiInputFlags_Repeat | ImGuiInputFlags_RouteAlways, owner_id);
    const bool start_toggling_with_gamepad = nav_gamepad_active && !g.NavWindowingTarget && Shortcut(ImGuiKey_NavGamepadMenu, ImGuiInputFlags_RouteAlways, owner_id);
    const bool start_windowing_with_gamepad = allow_windowing && start_toggling_with_gamepad;
    const bool start_windowing_with_keyboard = allow_windowing && !g.NavWindowingTarget && (keyboard_next_window || keyboard_prev_window);
    bool just_started_windowing_from_null_focus = false;
    if (start_toggling_with_gamepad)
    {
        g.NavWindowingToggleLayer = true;
        g.NavWindowingToggleKey = ImGuiKey_NavGamepadMenu;
        g.NavWindowingInputSource = g.NavInputSource = ImGuiInputSource_Gamepad;
    }
    if (start_windowing_with_gamepad || start_windowing_with_keyboard)
        if (ImGuiWindow* window = (g.NavWindow && IsWindowNavFocusable(g.NavWindow)) ? g.NavWindow : FindWindowNavFocusable(g.WindowsFocusOrder.Size - 1, -INT_MAX, -1))
        {
            if (start_windowing_with_keyboard || g.ConfigNavWindowingWithGamepad)
                g.NavWindowingTarget = g.NavWindowingTargetAnim = window->RootWindow;
            g.NavWindowingTimer = g.NavWindowingHighlightAlpha = 0.0f;
            g.NavWindowingAccumDeltaPos = g.NavWindowingAccumDeltaSize = ImVec2(0.0f, 0.0f);
            g.NavWindowingInputSource = g.NavInputSource = start_windowing_with_keyboard ? ImGuiInputSource_Keyboard : ImGuiInputSource_Gamepad;
            if (g.NavWindow == NULL)
                just_started_windowing_from_null_focus = true;

            if (keyboard_next_window || keyboard_prev_window)
                SetKeyOwnersForKeyChord((g.ConfigNavWindowingKeyNext | g.ConfigNavWindowingKeyPrev) & ImGuiMod_Mask_, owner_id);
        }

    if ((g.NavWindowingTarget || g.NavWindowingToggleLayer) && g.NavWindowingInputSource == ImGuiInputSource_Gamepad)
    {
        if (g.NavWindowingTarget != NULL)
        {

            g.NavWindowingTimer += io.DeltaTime;
            g.NavWindowingHighlightAlpha = ImMax(g.NavWindowingHighlightAlpha, ImSaturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) / 0.05f));

            const int focus_change_dir = (int)IsKeyPressed(ImGuiKey_GamepadL1) - (int)IsKeyPressed(ImGuiKey_GamepadR1);
            if (focus_change_dir != 0 && !just_started_windowing_from_null_focus)
            {
                NavUpdateWindowingTarget(focus_change_dir);
                g.NavWindowingHighlightAlpha = 1.0f;
            }
        }

        if (!IsKeyDown(ImGuiKey_NavGamepadMenu))
        {
            g.NavWindowingToggleLayer &= (g.NavWindowingHighlightAlpha < 1.0f);
            if (g.NavWindowingToggleLayer && g.NavWindow)
                apply_toggle_layer = true;
            else if (!g.NavWindowingToggleLayer)
                apply_focus_window = g.NavWindowingTarget;
            g.NavWindowingTarget = NULL;
            g.NavWindowingToggleLayer = false;
        }
    }

    if (g.NavWindowingTarget && g.NavWindowingInputSource == ImGuiInputSource_Keyboard)
    {

        ImGuiKeyChord shared_mods = ((g.ConfigNavWindowingKeyNext ? g.ConfigNavWindowingKeyNext : ImGuiMod_Mask_) & (g.ConfigNavWindowingKeyPrev ? g.ConfigNavWindowingKeyPrev : ImGuiMod_Mask_)) & ImGuiMod_Mask_;
        IM_ASSERT(shared_mods != 0);
        g.NavWindowingTimer += io.DeltaTime;
        g.NavWindowingHighlightAlpha = ImMax(g.NavWindowingHighlightAlpha, ImSaturate((g.NavWindowingTimer - NAV_WINDOWING_HIGHLIGHT_DELAY) / 0.05f));
        if ((keyboard_next_window || keyboard_prev_window) && !just_started_windowing_from_null_focus)
            NavUpdateWindowingTarget(keyboard_next_window ? -1 : +1);
        else if ((io.KeyMods & shared_mods) != shared_mods)
            apply_focus_window = g.NavWindowingTarget;
    }

    const ImGuiKey windowing_toggle_keys[] = { ImGuiKey_LeftAlt, ImGuiKey_RightAlt };
    bool windowing_toggle_layer_start = false;
    if (g.NavWindow != NULL && !(g.NavWindow->Flags & ImGuiWindowFlags_NoNavInputs))
        for (ImGuiKey windowing_toggle_key : windowing_toggle_keys)
            if (nav_keyboard_active && IsKeyPressed(windowing_toggle_key, 0, ImGuiKeyOwner_NoOwner))
            {
                windowing_toggle_layer_start = true;
                g.NavWindowingToggleLayer = true;
                g.NavWindowingToggleKey = windowing_toggle_key;
                g.NavWindowingInputSource = g.NavInputSource = ImGuiInputSource_Keyboard;
                break;
            }
    if (g.NavWindowingToggleLayer && g.NavWindowingInputSource == ImGuiInputSource_Keyboard)
    {

        if (io.InputQueueCharacters.Size > 0 || io.KeyCtrl || io.KeyShift || io.KeySuper)
            g.NavWindowingToggleLayer = false;
        else if (windowing_toggle_layer_start == false && g.LastKeyboardKeyPressTime == g.Time)
            g.NavWindowingToggleLayer = false;
        else if (TestKeyOwner(g.NavWindowingToggleKey, ImGuiKeyOwner_NoOwner) == false || TestKeyOwner(ImGuiMod_Alt, ImGuiKeyOwner_NoOwner) == false)
            g.NavWindowingToggleLayer = false;

        if (IsKeyReleased(g.NavWindowingToggleKey) && g.NavWindowingToggleLayer)
            if (g.ActiveId == 0 || g.ActiveIdAllowOverlap)
                if (IsMousePosValid(&io.MousePos) == IsMousePosValid(&io.MousePosPrev))
                    apply_toggle_layer = true;
        if (!IsKeyDown(g.NavWindowingToggleKey))
            g.NavWindowingToggleLayer = false;
    }

    if (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoMove))
    {
        ImVec2 nav_move_dir;
        if (g.NavInputSource == ImGuiInputSource_Keyboard && !io.KeyShift)
            nav_move_dir = GetKeyMagnitude2d(ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow);
        if (g.NavInputSource == ImGuiInputSource_Gamepad)
            nav_move_dir = GetKeyMagnitude2d(ImGuiKey_GamepadLStickLeft, ImGuiKey_GamepadLStickRight, ImGuiKey_GamepadLStickUp, ImGuiKey_GamepadLStickDown);
        if (nav_move_dir.x != 0.0f || nav_move_dir.y != 0.0f)
        {
            const float NAV_MOVE_SPEED = 800.0f;
            const float move_step = NAV_MOVE_SPEED * io.DeltaTime * GetScale();
            g.NavWindowingAccumDeltaPos += nav_move_dir * move_step;
            g.NavHighlightItemUnderNav = true;
            ImVec2 accum_floored = ImTrunc(g.NavWindowingAccumDeltaPos);
            if (accum_floored.x != 0.0f || accum_floored.y != 0.0f)
            {
                ImGuiWindow* moving_window = g.NavWindowingTarget->RootWindow;
                SetWindowPos(moving_window, moving_window->Pos + accum_floored, ImGuiCond_Always);
                g.NavWindowingAccumDeltaPos -= accum_floored;
            }
        }
    }

    if (apply_focus_window)
        NavUpdateWindowingApplyFocus(apply_focus_window);

    if (apply_toggle_layer && g.NavWindow)
    {
        ClearActiveID();

        ImGuiWindow* new_nav_window = g.NavWindow;
        while (new_nav_window->ParentWindow
            && (new_nav_window->DC.NavLayersActiveMask & (1 << ImGuiNavLayer_Menu)) == 0
            && (new_nav_window->Flags & ImGuiWindowFlags_ChildWindow) != 0
            && (new_nav_window->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu)) == 0)
            new_nav_window = new_nav_window->ParentWindow;
        if (new_nav_window != g.NavWindow)
        {
            ImGuiWindow* old_nav_window = g.NavWindow;
            FocusWindow(new_nav_window);
            new_nav_window->NavLastChildNavWindow = old_nav_window;
        }

        const ImGuiNavLayer new_nav_layer = (g.NavWindow->DC.NavLayersActiveMask & (1 << ImGuiNavLayer_Menu)) ? (ImGuiNavLayer)((int)g.NavLayer ^ 1) : ImGuiNavLayer_Main;
        if (new_nav_layer != g.NavLayer)
        {

            if (new_nav_layer == ImGuiNavLayer_Menu)
                g.NavWindow->NavLastIds[new_nav_layer] = 0;
            NavRestoreLayer(new_nav_layer);
            SetNavCursorVisibleAfterMove();
        }
    }
}

static const char* GetFallbackWindowNameForWindowingList(ImGuiWindow* window)
{
    if (window->Flags & ImGuiWindowFlags_Popup)
        return ImGui::LocalizeGetMsg(ImGuiLocKey_WindowingPopup);
    if ((window->Flags & ImGuiWindowFlags_MenuBar) && strcmp(window->Name, "##MainMenuBar") == 0)
        return ImGui::LocalizeGetMsg(ImGuiLocKey_WindowingMainMenuBar);
    return ImGui::LocalizeGetMsg(ImGuiLocKey_WindowingUntitled);
}

void ImGui::NavUpdateWindowingOverlay()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.NavWindowingTarget != NULL);

    if (g.NavWindowingTimer < NAV_WINDOWING_LIST_APPEAR_DELAY)
        return;

    const ImGuiViewport* viewport = GetMainViewport();
    SetNextWindowSizeConstraints(ImVec2(viewport->Size.x * 0.20f, viewport->Size.y * 0.20f), ImVec2(FLT_MAX, FLT_MAX));
    SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    PushStyleVar(ImGuiStyleVar_WindowPadding, g.Style.WindowPadding * 2.0f);
    Begin("##NavWindowingOverlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    g.NavWindowingListWindow = g.CurrentWindow;
    if (g.ContextName[0] != 0)
        SeparatorText(g.ContextName);
    for (int n = g.WindowsFocusOrder.Size - 1; n >= 0; n--)
    {
        ImGuiWindow* window = g.WindowsFocusOrder[n];
        IM_ASSERT(window != NULL);
        if (!IsWindowNavFocusable(window))
            continue;
        const char* label = window->Name;
        if (label == FindRenderedTextEnd(label))
            label = GetFallbackWindowNameForWindowingList(window);
        Selectable(label, g.NavWindowingTarget == window);
    }
    End();
    PopStyleVar();
}

bool ImGui::IsDragDropActive()
{
    ImGuiContext& g = *GImGui;
    return g.DragDropActive;
}

void ImGui::ClearDragDrop()
{
    ImGuiContext& g = *GImGui;
    if (g.DragDropActive)
        IMGUI_DEBUG_LOG_ACTIVEID("[dragdrop] ClearDragDrop()\n");
    g.DragDropActive = false;
    g.DragDropPayload.Clear();
    g.DragDropAcceptFlagsCurr = ImGuiDragDropFlags_None;
    g.DragDropAcceptIdCurr = g.DragDropAcceptIdPrev = 0;
    g.DragDropAcceptIdCurrRectSurface = FLT_MAX;
    g.DragDropAcceptFrameCount = -1;

    g.DragDropPayloadBufHeap.clear();
    memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
}

bool ImGui::BeginTooltipHidden()
{
    ImGuiContext& g = *GImGui;
    bool ret = Begin("##Tooltip_Hidden", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
    SetWindowHiddenAndSkipItemsForCurrentFrame(g.CurrentWindow);
    return ret;
}

bool ImGui::BeginDragDropSource(ImGuiDragDropFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    ImGuiMouseButton mouse_button = ImGuiMouseButton_Left;

    bool source_drag_active = false;
    ImGuiID source_id = 0;
    ImGuiID source_parent_id = 0;
    if ((flags & ImGuiDragDropFlags_SourceExtern) == 0)
    {
        source_id = g.LastItemData.ID;
        if (source_id != 0)
        {

            if (g.ActiveId != source_id)
                return false;
            if (g.ActiveIdMouseButton != -1)
                mouse_button = g.ActiveIdMouseButton;
            if (g.IO.MouseDown[mouse_button] == false || window->SkipItems)
                return false;
            g.ActiveIdAllowOverlap = false;
        }
        else
        {

            if (g.IO.MouseDown[mouse_button] == false || window->SkipItems)
                return false;
            if ((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) == 0 && (g.ActiveId == 0 || g.ActiveIdWindow != window))
                return false;

            if (!(flags & ImGuiDragDropFlags_SourceAllowNullID))
            {
                IM_ASSERT(0);
                return false;
            }

            source_id = g.LastItemData.ID = window->GetIDFromRectangle(g.LastItemData.Rect);
            KeepAliveID(source_id);
            bool is_hovered = ItemHoverable(g.LastItemData.Rect, source_id, g.LastItemData.ItemFlags);
            if (is_hovered && g.IO.MouseClicked[mouse_button])
            {
                SetActiveID(source_id, window);
                FocusWindow(window);
            }
            if (g.ActiveId == source_id)
                g.ActiveIdAllowOverlap = is_hovered;
        }
        if (g.ActiveId != source_id)
            return false;
        source_parent_id = window->IDStack.back();
        source_drag_active = IsMouseDragging(mouse_button);

        SetActiveIdUsingAllKeyboardKeys();
    }
    else
    {

        window = NULL;
        source_id = ImHashStr("#SourceExtern");
        source_drag_active = true;
        mouse_button = g.IO.MouseDown[0] ? 0 : -1;
        KeepAliveID(source_id);
        SetActiveID(source_id, NULL);
    }

    IM_ASSERT(g.DragDropWithinTarget == false);
    if (!source_drag_active)
        return false;

    if (!g.DragDropActive)
    {
        IM_ASSERT(source_id != 0);
        ClearDragDrop();
        IMGUI_DEBUG_LOG_ACTIVEID("[dragdrop] BeginDragDropSource() DragDropActive = true, source_id = 0x%08X%s\n",
            source_id, (flags & ImGuiDragDropFlags_SourceExtern) ? " (EXTERN)" : "");
        ImGuiPayload& payload = g.DragDropPayload;
        payload.SourceId = source_id;
        payload.SourceParentId = source_parent_id;
        g.DragDropActive = true;
        g.DragDropSourceFlags = flags;
        g.DragDropMouseButton = mouse_button;
        if (payload.SourceId == g.ActiveId)
            g.ActiveIdNoClearOnFocusLoss = true;
    }
    g.DragDropSourceFrameCount = g.FrameCount;
    g.DragDropWithinSource = true;

    if (!(flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
    {

        bool ret;
        if (g.DragDropAcceptIdPrev && (g.DragDropAcceptFlagsPrev & ImGuiDragDropFlags_AcceptNoPreviewTooltip))
            ret = BeginTooltipHidden();
        else
            ret = BeginTooltip();
        IM_ASSERT(ret);
        IM_UNUSED(ret);
    }

    if (!(flags & ImGuiDragDropFlags_SourceNoDisableHover) && !(flags & ImGuiDragDropFlags_SourceExtern))
        g.LastItemData.StatusFlags &= ~ImGuiItemStatusFlags_HoveredRect;

    return true;
}

void ImGui::EndDragDropSource()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.DragDropActive);
    IM_ASSERT(g.DragDropWithinSource && "Not after a BeginDragDropSource()?");

    if (!(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
        EndTooltip();

    if (g.DragDropPayload.DataFrameCount == -1)
        ClearDragDrop();
    g.DragDropWithinSource = false;
}

bool ImGui::SetDragDropPayload(const char* type, const void* data, size_t data_size, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    ImGuiPayload& payload = g.DragDropPayload;
    if (cond == 0)
        cond = ImGuiCond_Always;

    IM_ASSERT(type != NULL);
    IM_ASSERT(ImStrlen(type) < IM_COUNTOF(payload.DataType) && "Payload type can be at most 32 characters long");
    IM_ASSERT((data != NULL && data_size > 0) || (data == NULL && data_size == 0));
    IM_ASSERT(cond == ImGuiCond_Always || cond == ImGuiCond_Once);
    IM_ASSERT(payload.SourceId != 0);

    if (cond == ImGuiCond_Always || payload.DataFrameCount == -1)
    {

        ImStrncpy(payload.DataType, type, IM_COUNTOF(payload.DataType));
        g.DragDropPayloadBufHeap.resize(0);
        if (data_size > sizeof(g.DragDropPayloadBufLocal))
        {

            g.DragDropPayloadBufHeap.resize((int)data_size);
            payload.Data = g.DragDropPayloadBufHeap.Data;
            memcpy(payload.Data, data, (size_t)(int)data_size);
        }
        else if (data_size > 0)
        {

            memset(&g.DragDropPayloadBufLocal, 0, sizeof(g.DragDropPayloadBufLocal));
            payload.Data = g.DragDropPayloadBufLocal;
            memcpy(payload.Data, data, (size_t)(int)data_size);
        }
        else
        {
            payload.Data = NULL;
        }
        payload.DataSize = (int)data_size;
    }
    payload.DataFrameCount = g.FrameCount;

    return (g.DragDropAcceptFrameCount == g.FrameCount) || (g.DragDropAcceptFrameCount == g.FrameCount - 1);
}

bool ImGui::BeginDragDropTargetCustom(const ImRect& bb, ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    if (!g.DragDropActive)
        return false;

    ImGuiWindow* window = g.CurrentWindow;
    ImGuiWindow* hovered_window = g.HoveredWindowUnderMovingWindow;
    if (hovered_window == NULL || window->RootWindow != hovered_window->RootWindow)
        return false;
    IM_ASSERT(id != 0);
    if (!IsMouseHoveringRect(bb.Min, bb.Max) || (id == g.DragDropPayload.SourceId))
        return false;
    if (window->SkipItems)
        return false;

    IM_ASSERT(g.DragDropWithinTarget == false && g.DragDropWithinSource == false);
    g.DragDropTargetRect = bb;
    g.DragDropTargetClipRect = window->ClipRect;
    g.DragDropTargetId = id;
    g.DragDropTargetFullViewport = 0;
    g.DragDropWithinTarget = true;
    return true;
}

bool ImGui::BeginDragDropTargetViewport(ImGuiViewport* viewport, const ImRect* p_bb)
{
    ImGuiContext& g = *GImGui;
    if (!g.DragDropActive)
        return false;

    ImRect bb = p_bb ? *p_bb : ((ImGuiViewportP*)viewport)->GetWorkRect();
    ImGuiID id = viewport->ID;
    if (!IsMouseHoveringRect(bb.Min, bb.Max, false) || (id == g.DragDropPayload.SourceId))
        return false;

    IM_ASSERT(g.DragDropWithinTarget == false && g.DragDropWithinSource == false);
    g.DragDropTargetRect = bb;
    g.DragDropTargetClipRect = bb;
    g.DragDropTargetId = id;
    g.DragDropTargetFullViewport = id;
    g.DragDropWithinTarget = true;
    return true;
}

bool ImGui::BeginDragDropTarget()
{
    ImGuiContext& g = *GImGui;
    if (!g.DragDropActive)
        return false;

    ImGuiWindow* window = g.CurrentWindow;
    if (!(g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect))
        return false;
    ImGuiWindow* hovered_window = g.HoveredWindowUnderMovingWindow;
    if (hovered_window == NULL || window->RootWindow != hovered_window->RootWindow || window->SkipItems)
        return false;

    const ImRect& display_rect = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDisplayRect) ? g.LastItemData.DisplayRect : g.LastItemData.Rect;
    ImGuiID id = g.LastItemData.ID;
    if (id == 0)
    {
        id = window->GetIDFromRectangle(display_rect);
        KeepAliveID(id);
    }
    if (g.DragDropPayload.SourceId == id)
        return false;

    IM_ASSERT(g.DragDropWithinTarget == false && g.DragDropWithinSource == false);
    g.DragDropTargetRect = display_rect;
    g.DragDropTargetClipRect = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasClipRect) ? g.LastItemData.ClipRect : window->ClipRect;
    g.DragDropTargetId = id;
    g.DragDropWithinTarget = true;
    return true;
}

bool ImGui::IsDragDropPayloadBeingAccepted()
{
    ImGuiContext& g = *GImGui;
    return g.DragDropActive && g.DragDropAcceptIdPrev != 0;
}

const ImGuiPayload* ImGui::AcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiPayload& payload = g.DragDropPayload;
    IM_ASSERT(g.DragDropActive);
    IM_ASSERT(payload.DataFrameCount != -1);
    if (type != NULL && !payload.IsDataType(type))
        return NULL;

    const bool was_accepted_previously = (g.DragDropAcceptIdPrev == g.DragDropTargetId);
    ImRect r = g.DragDropTargetRect;
    float r_surface = r.GetWidth() * r.GetHeight();
    if (r_surface > g.DragDropAcceptIdCurrRectSurface)
        return NULL;

    g.DragDropAcceptFlagsCurr = flags;
    g.DragDropAcceptIdCurr = g.DragDropTargetId;
    g.DragDropAcceptIdCurrRectSurface = r_surface;

    payload.Preview = was_accepted_previously;
    flags |= (g.DragDropSourceFlags & ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
    const bool draw_target_rect = payload.Preview && !(flags & ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
    if (draw_target_rect && g.DragDropTargetFullViewport != 0)
    {
        ImRect bb = g.DragDropTargetRect;
        bb.Expand(-3.5f);
        RenderDragDropTargetRectEx(GetForegroundDrawList(), bb);
    }
    else if (draw_target_rect)
    {
        RenderDragDropTargetRectForItem(r);
    }

    g.DragDropAcceptFrameCount = g.FrameCount;
    if ((g.DragDropSourceFlags & ImGuiDragDropFlags_SourceExtern) && g.DragDropMouseButton == -1)
        payload.Delivery = was_accepted_previously && (g.DragDropSourceFrameCount < g.FrameCount);
    else
        payload.Delivery = was_accepted_previously && !IsMouseDown(g.DragDropMouseButton);
    if (!payload.Delivery && !(flags & ImGuiDragDropFlags_AcceptBeforeDelivery))
        return NULL;

    if (payload.Delivery)
        IMGUI_DEBUG_LOG_ACTIVEID("[dragdrop] AcceptDragDropPayload(): 0x%08X: payload delivery\n", g.DragDropTargetId);
    return &payload;
}

void ImGui::RenderDragDropTargetRectForItem(const ImRect& bb)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImRect bb_display = bb;
    bb_display.ClipWith(g.DragDropTargetClipRect);
    bb_display.Expand(g.Style.DragDropTargetPadding);
    bool push_clip_rect = !window->ClipRect.Contains(bb_display);
    if (push_clip_rect)
        window->DrawList->PushClipRectFullScreen();
    RenderDragDropTargetRectEx(window->DrawList, bb_display);
    if (push_clip_rect)
        window->DrawList->PopClipRect();
}

void ImGui::RenderDragDropTargetRectEx(ImDrawList* draw_list, const ImRect& bb)
{
    ImGuiContext& g = *GImGui;
    draw_list->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_DragDropTargetBg), g.Style.DragDropTargetRounding, 0);
    draw_list->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_DragDropTarget), g.Style.DragDropTargetRounding, 0, g.Style.DragDropTargetBorderSize);
}

const ImGuiPayload* ImGui::GetDragDropPayload()
{
    ImGuiContext& g = *GImGui;
    return (g.DragDropActive && g.DragDropPayload.DataFrameCount != -1) ? &g.DragDropPayload : NULL;
}

void ImGui::EndDragDropTarget()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.DragDropActive);
    IM_ASSERT(g.DragDropWithinTarget);
    g.DragDropWithinTarget = false;

    if (g.DragDropPayload.Delivery)
        ClearDragDrop();
}

static inline void LogTextV(ImGuiContext& g, const char* fmt, va_list args)
{
    if (g.LogFile)
    {
        g.LogBuffer.Buf.resize(0);
        g.LogBuffer.appendfv(fmt, args);
        ImFileWrite(g.LogBuffer.c_str(), sizeof(char), (ImU64)g.LogBuffer.size(), g.LogFile);
    }
    else
    {
        g.LogBuffer.appendfv(fmt, args);
    }
}

void ImGui::LogText(const char* fmt, ...)
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    va_list args;
    va_start(args, fmt);
    LogTextV(g, fmt, args);
    va_end(args);
}

void ImGui::LogTextV(const char* fmt, va_list args)
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    LogTextV(g, fmt, args);
}

void ImGui::LogRenderedText(const ImVec2* ref_pos, const char* text, const char* text_end)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const char* prefix = g.LogNextPrefix;
    const char* suffix = g.LogNextSuffix;
    g.LogNextPrefix = g.LogNextSuffix = NULL;

    if (!text_end)
        text_end = FindRenderedTextEnd(text, text_end);

    const bool log_new_line = ref_pos && (ref_pos->y > g.LogLinePosY + ImMax(g.Style.FramePadding.y, g.Style.ItemSpacing.y) + 1);
    if (ref_pos)
        g.LogLinePosY = ref_pos->y;
    if (log_new_line)
    {
        LogText(IM_NEWLINE);
        g.LogLineFirstItem = true;
    }

    if (prefix)
        LogRenderedText(ref_pos, prefix, prefix + ImStrlen(prefix));

    if (g.LogDepthRef > window->DC.TreeDepth)
        g.LogDepthRef = window->DC.TreeDepth;
    const int tree_depth = (window->DC.TreeDepth - g.LogDepthRef);

    const char* text_remaining = text;
    for (;;)
    {

        const char* line_start = text_remaining;
        const char* line_end = ImStreolRange(line_start, text_end);
        const bool is_last_line = (line_end == text_end);
        if (line_start != line_end || !is_last_line)
        {
            const int line_length = (int)(line_end - line_start);
            const int indentation = g.LogLineFirstItem ? tree_depth * 4 : 1;
            LogText("%*s%.*s", indentation, "", line_length, line_start);
            g.LogLineFirstItem = false;
            if (*line_end == '\n')
            {
                LogText(IM_NEWLINE);
                g.LogLineFirstItem = true;
            }
        }
        if (is_last_line)
            break;
        text_remaining = line_end + 1;
    }

    if (suffix)
        LogRenderedText(ref_pos, suffix, suffix + ImStrlen(suffix));
}

void ImGui::LogBegin(ImGuiLogFlags flags, int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    IM_ASSERT(g.LogEnabled == false);
    IM_ASSERT(g.LogFile == NULL && g.LogBuffer.empty());
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiLogFlags_OutputMask_));

    g.LogEnabled = g.ItemUnclipByLog = true;
    g.LogFlags = flags;
    g.LogWindow = window;
    g.LogNextPrefix = g.LogNextSuffix = NULL;
    g.LogDepthRef = window->DC.TreeDepth;
    g.LogDepthToExpand = ((auto_open_depth >= 0) ? auto_open_depth : g.LogDepthToExpandDefault);
    g.LogLinePosY = FLT_MAX;
    g.LogLineFirstItem = true;
}

void ImGui::LogSetNextTextDecoration(const char* prefix, const char* suffix)
{
    ImGuiContext& g = *GImGui;
    g.LogNextPrefix = prefix;
    g.LogNextSuffix = suffix;
}

void ImGui::LogToTTY(int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    IM_UNUSED(auto_open_depth);
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
    LogBegin(ImGuiLogFlags_OutputTTY, auto_open_depth);
    g.LogFile = stdout;
#endif
}

void ImGui::LogToFile(int auto_open_depth, const char* filename)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;

    if (!filename)
        filename = g.IO.LogFilename;
    if (!filename || !filename[0])
        return;
    ImFileHandle f = ImFileOpen(filename, "ab");
    if (!f)
    {
        IM_ASSERT(0);
        return;
    }

    LogBegin(ImGuiLogFlags_OutputFile, auto_open_depth);
    g.LogFile = f;
}

void ImGui::LogToClipboard(int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    LogBegin(ImGuiLogFlags_OutputClipboard, auto_open_depth);
}

void ImGui::LogToBuffer(int auto_open_depth)
{
    ImGuiContext& g = *GImGui;
    if (g.LogEnabled)
        return;
    LogBegin(ImGuiLogFlags_OutputBuffer, auto_open_depth);
}

void ImGui::LogFinish()
{
    ImGuiContext& g = *GImGui;
    if (!g.LogEnabled)
        return;

    LogText(IM_NEWLINE);
    switch (g.LogFlags & ImGuiLogFlags_OutputMask_)
    {
    case ImGuiLogFlags_OutputTTY:
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
        fflush(g.LogFile);
#endif
        break;
    case ImGuiLogFlags_OutputFile:
        ImFileClose(g.LogFile);
        break;
    case ImGuiLogFlags_OutputBuffer:
        break;
    case ImGuiLogFlags_OutputClipboard:
        if (!g.LogBuffer.empty())
            SetClipboardText(g.LogBuffer.begin());
        break;
    default:
        IM_ASSERT(0);
        break;
    }

    g.LogEnabled = g.ItemUnclipByLog = false;
    g.LogFlags = ImGuiLogFlags_None;
    g.LogFile = NULL;
    g.LogBuffer.clear();
}

void ImGui::LogButtons()
{
    ImGuiContext& g = *GImGui;

    PushID("LogButtons");
#ifndef IMGUI_DISABLE_TTY_FUNCTIONS
    const bool log_to_tty = Button("Log To TTY"); SameLine();
#else
    const bool log_to_tty = false;
#endif
    const bool log_to_file = Button("Log To File"); SameLine();
    const bool log_to_clipboard = Button("Log To Clipboard"); SameLine();
    PushItemFlag(ImGuiItemFlags_NoTabStop, true);
    SetNextItemWidth(CalcTextSize("999").x);
    SliderInt("Default Depth", &g.LogDepthToExpandDefault, 0, 9, NULL);
    PopItemFlag();
    PopID();

    if (log_to_tty)
        LogToTTY();
    if (log_to_file)
        LogToFile();
    if (log_to_clipboard)
        LogToClipboard();
}

void ImGui::UpdateSettings()
{

    ImGuiContext& g = *GImGui;
    if (!g.SettingsLoaded)
    {
        IM_ASSERT(g.SettingsWindows.empty());
        if (g.IO.IniFilename)
            LoadIniSettingsFromDisk(g.IO.IniFilename);
        g.SettingsLoaded = true;
    }

    if (g.SettingsDirtyTimer > 0.0f)
    {
        g.SettingsDirtyTimer -= g.IO.DeltaTime;
        if (g.SettingsDirtyTimer <= 0.0f)
        {
            if (g.IO.IniFilename != NULL)
                SaveIniSettingsToDisk(g.IO.IniFilename);
            else
                g.IO.WantSaveIniSettings = true;
            g.SettingsDirtyTimer = 0.0f;
        }
    }
}

void ImGui::MarkIniSettingsDirty()
{
    ImGuiContext& g = *GImGui;
    if (g.SettingsDirtyTimer <= 0.0f)
        g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

void ImGui::MarkIniSettingsDirty(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings))
        if (g.SettingsDirtyTimer <= 0.0f)
            g.SettingsDirtyTimer = g.IO.IniSavingRate;
}

void ImGui::AddSettingsHandler(const ImGuiSettingsHandler* handler)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(FindSettingsHandler(handler->TypeName) == NULL);
    g.SettingsHandlers.push_back(*handler);
}

void ImGui::RemoveSettingsHandler(const char* type_name)
{
    ImGuiContext& g = *GImGui;
    if (ImGuiSettingsHandler* handler = FindSettingsHandler(type_name))
        g.SettingsHandlers.erase(handler);
}

ImGuiSettingsHandler* ImGui::FindSettingsHandler(const char* type_name)
{
    ImGuiContext& g = *GImGui;
    const ImGuiID type_hash = ImHashStr(type_name);
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.TypeHash == type_hash)
            return &handler;
    return NULL;
}

void ImGui::ClearIniSettings()
{
    ImGuiContext& g = *GImGui;
    g.SettingsIniData.clear();
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.ClearAllFn != NULL)
            handler.ClearAllFn(&g, &handler);
}

void ImGui::LoadIniSettingsFromDisk(const char* ini_filename)
{
    size_t file_data_size = 0;
    char* file_data = (char*)ImFileLoadToMemory(ini_filename, "rb", &file_data_size);
    if (!file_data)
        return;
    if (file_data_size > 0)
        LoadIniSettingsFromMemory(file_data, (size_t)file_data_size);
    IM_FREE(file_data);
}

void ImGui::LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);

    if (ini_size == 0)
        ini_size = ImStrlen(ini_data);
    g.SettingsIniData.Buf.resize((int)ini_size + 1);
    char* const buf = g.SettingsIniData.Buf.Data;
    char* const buf_end = buf + ini_size;
    memcpy(buf, ini_data, ini_size);
    buf_end[0] = 0;

    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.ReadInitFn != NULL)
            handler.ReadInitFn(&g, &handler);

    void* entry_data = NULL;
    ImGuiSettingsHandler* entry_handler = NULL;

    char* line_end = NULL;
    for (char* line = buf; line < buf_end; line = line_end + 1)
    {

        while (*line == '\n' || *line == '\r')
            line++;
        line_end = line;
        while (line_end < buf_end && *line_end != '\n' && *line_end != '\r')
            line_end++;
        line_end[0] = 0;
        if (line[0] == ';')
            continue;
        if (line[0] == '[' && line_end > line && line_end[-1] == ']')
        {

            line_end[-1] = 0;
            const char* name_end = line_end - 1;
            const char* type_start = line + 1;
            char* type_end = (char*)(void*)ImStrchrRange(type_start, name_end, ']');
            const char* name_start = type_end ? ImStrchrRange(type_end + 1, name_end, '[') : NULL;
            if (!type_end || !name_start)
                continue;
            *type_end = 0;
            name_start++;
            entry_handler = FindSettingsHandler(type_start);
            entry_data = entry_handler ? entry_handler->ReadOpenFn(&g, entry_handler, name_start) : NULL;
        }
        else if (entry_handler != NULL && entry_data != NULL)
        {

            entry_handler->ReadLineFn(&g, entry_handler, entry_data, line);
        }
    }
    g.SettingsLoaded = true;

    memcpy(buf, ini_data, ini_size);

    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        if (handler.ApplyAllFn != NULL)
            handler.ApplyAllFn(&g, &handler);
}

void ImGui::SaveIniSettingsToDisk(const char* ini_filename)
{
    ImGuiContext& g = *GImGui;
    g.SettingsDirtyTimer = 0.0f;
    if (!ini_filename)
        return;

    size_t ini_data_size = 0;
    const char* ini_data = SaveIniSettingsToMemory(&ini_data_size);
    ImFileHandle f = ImFileOpen(ini_filename, "wt");
    if (!f)
        return;
    ImFileWrite(ini_data, sizeof(char), ini_data_size, f);
    ImFileClose(f);
}

const char* ImGui::SaveIniSettingsToMemory(size_t* out_size)
{
    ImGuiContext& g = *GImGui;
    g.SettingsDirtyTimer = 0.0f;
    g.SettingsIniData.Buf.resize(0);
    g.SettingsIniData.Buf.push_back(0);
    for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
        handler.WriteAllFn(&g, &handler, &g.SettingsIniData);
    if (out_size)
        *out_size = (size_t)g.SettingsIniData.size();
    return g.SettingsIniData.c_str();
}

ImGuiWindowSettings* ImGui::CreateNewWindowSettings(const char* name)
{
    ImGuiContext& g = *GImGui;

    if (g.IO.ConfigDebugIniSettings == false)
        name = ImHashSkipUncontributingPrefix(name);
    const size_t name_len = ImStrlen(name);

    const size_t chunk_size = sizeof(ImGuiWindowSettings) + name_len + 1;
    ImGuiWindowSettings* settings = g.SettingsWindows.alloc_chunk(chunk_size);
    IM_PLACEMENT_NEW(settings) ImGuiWindowSettings();
    settings->ID = ImHashStr(name, name_len);
    memcpy(settings->GetName(), name, name_len + 1);

    return settings;
}

ImGuiWindowSettings* ImGui::FindWindowSettingsByID(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
        if (settings->ID == id && !settings->WantDelete)
            return settings;
    return NULL;
}

ImGuiWindowSettings* ImGui::FindWindowSettingsByWindow(ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (window->SettingsOffset != -1)
        return g.SettingsWindows.ptr_from_offset(window->SettingsOffset);
    return FindWindowSettingsByID(window->ID);
}

void ImGui::ClearWindowSettings(const char* name)
{

    ImGuiWindow* window = FindWindowByName(name);
    if (window != NULL)
    {
        window->Flags |= ImGuiWindowFlags_NoSavedSettings;
        InitOrLoadWindowSettings(window, NULL);
    }
    if (ImGuiWindowSettings* settings = window ? FindWindowSettingsByWindow(window) : FindWindowSettingsByID(ImHashStr(name)))
        settings->WantDelete = true;
}

static void WindowSettingsHandler_ClearAll(ImGuiContext* ctx, ImGuiSettingsHandler*)
{
    ImGuiContext& g = *ctx;
    for (ImGuiWindow* window : g.Windows)
        window->SettingsOffset = -1;
    g.SettingsWindows.clear();
}

static void* WindowSettingsHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name)
{
    ImGuiID id = ImHashStr(name);
    ImGuiWindowSettings* settings = ImGui::FindWindowSettingsByID(id);
    if (settings)
        *settings = ImGuiWindowSettings();
    else
        settings = ImGui::CreateNewWindowSettings(name);
    settings->ID = id;
    settings->WantApply = true;
    return (void*)settings;
}

static void WindowSettingsHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
{
    ImGuiWindowSettings* settings = (ImGuiWindowSettings*)entry;
    int x, y;
    int i;
    if (sscanf(line, "Pos=%i,%i", &x, &y) == 2)         { settings->Pos = ImVec2ih((short)x, (short)y); }
    else if (sscanf(line, "Size=%i,%i", &x, &y) == 2)   { settings->Size = ImVec2ih((short)x, (short)y); }
    else if (sscanf(line, "Collapsed=%d", &i) == 1)     { settings->Collapsed = (i != 0); }
    else if (sscanf(line, "IsChild=%d", &i) == 1)       { settings->IsChild = (i != 0); }
}

static void WindowSettingsHandler_ApplyAll(ImGuiContext* ctx, ImGuiSettingsHandler*)
{
    ImGuiContext& g = *ctx;
    for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
        if (settings->WantApply)
        {
            if (ImGuiWindow* window = ImGui::FindWindowByID(settings->ID))
                ApplyWindowSettings(window, settings);
            settings->WantApply = false;
        }
}

static void WindowSettingsHandler_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{

    ImGuiContext& g = *ctx;
    for (ImGuiWindow* window : g.Windows)
    {
        if (window->Flags & ImGuiWindowFlags_NoSavedSettings)
            continue;

        ImGuiWindowSettings* settings = ImGui::FindWindowSettingsByWindow(window);
        if (!settings)
        {
            settings = ImGui::CreateNewWindowSettings(window->Name);
            window->SettingsOffset = g.SettingsWindows.offset_from_ptr(settings);
        }
        IM_ASSERT(settings->ID == window->ID);
        settings->Pos = ImVec2ih(window->Pos);
        settings->Size = ImVec2ih(window->SizeFull);
        settings->IsChild = (window->Flags & ImGuiWindowFlags_ChildWindow) != 0;
        settings->Collapsed = window->Collapsed;
        settings->WantDelete = false;
    }

    buf->reserve(buf->size() + g.SettingsWindows.size() * 6);
    for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
    {
        if (settings->WantDelete)
            continue;
        const char* settings_name = settings->GetName();
        buf->appendf("[%s][%s]\n", handler->TypeName, settings_name);
        if (settings->IsChild)
        {
            buf->appendf("IsChild=1\n");
            buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
        }
        else
        {
            buf->appendf("Pos=%d,%d\n", settings->Pos.x, settings->Pos.y);
            buf->appendf("Size=%d,%d\n", settings->Size.x, settings->Size.y);
            if (settings->Collapsed)
                buf->appendf("Collapsed=1\n");
        }
        buf->append("\n");
    }
}

void ImGui::LocalizeRegisterEntries(const ImGuiLocEntry* entries, int count)
{
    ImGuiContext& g = *GImGui;
    for (int n = 0; n < count; n++)
        g.LocalizationTable[entries[n].Key] = entries[n].Text;
}

void ImGuiPlatformIO::ClearPlatformHandlers()
{
    Platform_GetClipboardTextFn = NULL;
    Platform_SetClipboardTextFn = NULL;
    Platform_ClipboardUserData = NULL;
    Platform_OpenInShellFn = NULL;
    Platform_OpenInShellUserData = NULL;
    Platform_SetImeDataFn = NULL;
    Platform_ImeUserData = NULL;
}

void ImGuiPlatformIO::ClearRendererHandlers()
{
    Renderer_TextureMaxWidth = Renderer_TextureMaxHeight = 0;
    Renderer_RenderState = NULL;
}

ImGuiViewport* ImGui::GetMainViewport()
{
    ImGuiContext& g = *GImGui;
    return g.Viewports[0];
}

void ImGui::SetWindowViewport(ImGuiWindow* window, ImGuiViewportP* viewport)
{
    window->Viewport = viewport;
}

static void ScaleWindow(ImGuiWindow* window, float scale)
{
    ImVec2 origin = window->Viewport->Pos;
    window->Pos = ImFloor((window->Pos - origin) * scale + origin);
    window->Size = ImTrunc(window->Size * scale);
    window->SizeFull = ImTrunc(window->SizeFull * scale);
    window->ContentSize = ImTrunc(window->ContentSize * scale);
}

void ImGui::ScaleWindowsInViewport(ImGuiViewportP* viewport, float scale)
{
    ImGuiContext& g = *GImGui;
    for (ImGuiWindow* window : g.Windows)
        if (window->Viewport == viewport)
            ScaleWindow(window, scale);
}

static void ImGui::UpdateViewportsNewFrame()
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Viewports.Size == 1);

    ImGuiViewportP* main_viewport = g.Viewports[0];
    main_viewport->Flags = ImGuiViewportFlags_IsPlatformWindow | ImGuiViewportFlags_OwnedByApp;
    main_viewport->Pos = ImVec2(0.0f, 0.0f);
    main_viewport->Size = g.IO.DisplaySize;
    main_viewport->FramebufferScale = g.IO.DisplayFramebufferScale;
    IM_ASSERT(main_viewport->FramebufferScale.x > 0.0f && main_viewport->FramebufferScale.y > 0.0f);

    const float memory_compact_start_time = (g.GcCompactAll || g.IO.ConfigMemoryCompactTimer < 0.0f) ? FLT_MAX : (float)g.Time - g.IO.ConfigMemoryCompactTimer;
    for (ImGuiViewportP* viewport : g.Viewports)
    {

        viewport->WorkInsetMin = viewport->BuildWorkInsetMin;
        viewport->WorkInsetMax = viewport->BuildWorkInsetMax;
        viewport->BuildWorkInsetMin = viewport->BuildWorkInsetMax = ImVec2(0.0f, 0.0f);
        viewport->UpdateWorkRect();

        for (int n = 0; n < IM_COUNTOF(viewport->BgFgDrawLists); n++)
            if (viewport->BgFgDrawListsLastTimeActive[n] < memory_compact_start_time && viewport->BgFgDrawLists[n] != NULL)
            {
                IM_DELETE(viewport->BgFgDrawLists[n]);
                viewport->BgFgDrawLists[n] = NULL;
            }
    }
}

#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS)

#ifdef _MSC_VER
#pragma comment(lib, "user32")
#pragma comment(lib, "kernel32")
#endif

static const char* Platform_GetClipboardTextFn_DefaultImpl(ImGuiContext* ctx)
{
    ImGuiContext& g = *ctx;
    g.ClipboardHandlerData.clear();
    if (!::OpenClipboard(NULL))
        return NULL;
    HANDLE wbuf_handle = ::GetClipboardData(CF_UNICODETEXT);
    if (wbuf_handle == NULL)
    {
        ::CloseClipboard();
        return NULL;
    }
    if (const WCHAR* wbuf_global = (const WCHAR*)::GlobalLock(wbuf_handle))
    {
        int buf_len = ::WideCharToMultiByte(CP_UTF8, 0, wbuf_global, -1, NULL, 0, NULL, NULL);
        g.ClipboardHandlerData.resize(buf_len);
        ::WideCharToMultiByte(CP_UTF8, 0, wbuf_global, -1, g.ClipboardHandlerData.Data, buf_len, NULL, NULL);
    }
    ::GlobalUnlock(wbuf_handle);
    ::CloseClipboard();
    return g.ClipboardHandlerData.Data;
}

static void Platform_SetClipboardTextFn_DefaultImpl(ImGuiContext*, const char* text)
{
    if (!::OpenClipboard(NULL))
        return;
    const int wbuf_length = ::MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    HGLOBAL wbuf_handle = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(WCHAR));
    if (wbuf_handle == NULL)
    {
        ::CloseClipboard();
        return;
    }
    WCHAR* wbuf_global = (WCHAR*)::GlobalLock(wbuf_handle);
    ::MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf_global, wbuf_length);
    ::GlobalUnlock(wbuf_handle);
    ::EmptyClipboard();
    if (::SetClipboardData(CF_UNICODETEXT, wbuf_handle) == NULL)
        ::GlobalFree(wbuf_handle);
    ::CloseClipboard();
}

#elif defined(__APPLE__) && TARGET_OS_OSX && defined(IMGUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS)

#include <Carbon/Carbon.h>
static PasteboardRef main_clipboard = 0;

static void Platform_SetClipboardTextFn_DefaultImpl(ImGuiContext*, const char* text)
{
    if (!main_clipboard)
        PasteboardCreate(kPasteboardClipboard, &main_clipboard);
    PasteboardClear(main_clipboard);
    CFDataRef cf_data = CFDataCreate(kCFAllocatorDefault, (const UInt8*)text, ImStrlen(text));
    if (cf_data)
    {
        PasteboardPutItemFlavor(main_clipboard, (PasteboardItemID)1, CFSTR("public.utf8-plain-text"), cf_data, 0);
        CFRelease(cf_data);
    }
}

static const char* Platform_GetClipboardTextFn_DefaultImpl(ImGuiContext* ctx)
{
    ImGuiContext& g = *ctx;
    if (!main_clipboard)
        PasteboardCreate(kPasteboardClipboard, &main_clipboard);
    PasteboardSynchronize(main_clipboard);

    ItemCount item_count = 0;
    PasteboardGetItemCount(main_clipboard, &item_count);
    for (ItemCount i = 0; i < item_count; i++)
    {
        PasteboardItemID item_id = 0;
        PasteboardGetItemIdentifier(main_clipboard, i + 1, &item_id);
        CFArrayRef flavor_type_array = 0;
        PasteboardCopyItemFlavors(main_clipboard, item_id, &flavor_type_array);
        for (CFIndex j = 0, nj = CFArrayGetCount(flavor_type_array); j < nj; j++)
        {
            CFDataRef cf_data;
            if (PasteboardCopyItemFlavorData(main_clipboard, item_id, CFSTR("public.utf8-plain-text"), &cf_data) == noErr)
            {
                g.ClipboardHandlerData.clear();
                int length = (int)CFDataGetLength(cf_data);
                g.ClipboardHandlerData.resize(length + 1);
                CFDataGetBytes(cf_data, CFRangeMake(0, length), (UInt8*)g.ClipboardHandlerData.Data);
                g.ClipboardHandlerData[length] = 0;
                CFRelease(cf_data);
                return g.ClipboardHandlerData.Data;
            }
        }
    }
    return NULL;
}

#else

static const char* Platform_GetClipboardTextFn_DefaultImpl(ImGuiContext* ctx)
{
    ImGuiContext& g = *ctx;
    return g.ClipboardHandlerData.empty() ? NULL : g.ClipboardHandlerData.begin();
}

static void Platform_SetClipboardTextFn_DefaultImpl(ImGuiContext* ctx, const char* text)
{
    ImGuiContext& g = *ctx;
    g.ClipboardHandlerData.clear();
    const char* text_end = text + ImStrlen(text);
    g.ClipboardHandlerData.resize((int)(text_end - text) + 1);
    memcpy(&g.ClipboardHandlerData[0], text, (size_t)(text_end - text));
    g.ClipboardHandlerData[(int)(text_end - text)] = 0;
}

#endif

#ifndef IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#if defined(__APPLE__) && TARGET_OS_IPHONE
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#endif
#if defined(__3DS__)
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#endif
#if defined(_WIN32) && defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#endif
#endif

#ifndef IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
#ifdef _WIN32
#include <shellapi.h>
#ifdef _MSC_VER
#pragma comment(lib, "shell32")
#endif
static bool Platform_OpenInShellFn_DefaultImpl(ImGuiContext*, const char* path)
{
    const int path_wsize = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    ImVector<wchar_t> path_wbuf;
    path_wbuf.resize(path_wsize);
    ::MultiByteToWideChar(CP_UTF8, 0, path, -1, path_wbuf.Data, path_wsize);
    return (INT_PTR)::ShellExecuteW(NULL, L"open", path_wbuf.Data, NULL, NULL, SW_SHOWDEFAULT) > 32;
}
#else
#include <sys/wait.h>
#include <unistd.h>
static bool Platform_OpenInShellFn_DefaultImpl(ImGuiContext*, const char* path)
{
#if defined(__APPLE__)
    const char* args[] { "open", "--", path, NULL };
#else
    const char* args[] { "xdg-open", path, NULL };
#endif
    pid_t pid = fork();
    if (pid < 0)
        return false;
    if (!pid)
    {
        execvp(args[0], const_cast<char **>(args));
        exit(-1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status) == 0;
    }
}
#endif
#else
static bool Platform_OpenInShellFn_DefaultImpl(ImGuiContext*, const char*) { return false; }
#endif

#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)

#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif

static void Platform_SetImeDataFn_DefaultImpl(ImGuiContext*, ImGuiViewport* viewport, ImGuiPlatformImeData* data)
{

    HWND hwnd = (HWND)viewport->PlatformHandleRaw;
    if (hwnd == 0)
        return;

    if (HIMC himc = ::ImmGetContext(hwnd))
    {
        COMPOSITIONFORM composition_form = {};
        composition_form.ptCurrentPos.x = (LONG)data->InputPos.x;
        composition_form.ptCurrentPos.y = (LONG)data->InputPos.y;
        composition_form.dwStyle = CFS_FORCE_POSITION;
        ::ImmSetCompositionWindow(himc, &composition_form);
        CANDIDATEFORM candidate_form = {};
        candidate_form.dwStyle = CFS_CANDIDATEPOS;
        candidate_form.ptCurrentPos.x = (LONG)data->InputPos.x;
        candidate_form.ptCurrentPos.y = (LONG)data->InputPos.y;
        ::ImmSetCandidateWindow(himc, &candidate_form);
        ::ImmReleaseContext(hwnd, himc);
    }
}

#else

static void Platform_SetImeDataFn_DefaultImpl(ImGuiContext*, ImGuiViewport*, ImGuiPlatformImeData*) {}

#endif

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS) || !defined(IMGUI_DISABLE_DEBUG_TOOLS)

static void MetricsHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
#endif

#ifndef IMGUI_DISABLE_DEBUG_TOOLS

void ImGui::DebugRenderViewportThumbnail(ImDrawList* draw_list, ImGuiViewportP* viewport, const ImRect& bb)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    ImVec2 scale = bb.GetSize() / viewport->Size;
    ImVec2 off = bb.Min - viewport->Pos * scale;
    float alpha_mul = 1.0f;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border, alpha_mul * 0.40f));
    for (ImGuiWindow* thumb_window : g.Windows)
    {
        if (!thumb_window->WasActive || (thumb_window->Flags & ImGuiWindowFlags_ChildWindow))
            continue;

        ImRect thumb_r = thumb_window->Rect();
        ImRect title_r = thumb_window->TitleBarRect();
        thumb_r = ImRect(ImTrunc(off + thumb_r.Min * scale), ImTrunc(off +  thumb_r.Max * scale));
        title_r = ImRect(ImTrunc(off + title_r.Min * scale), ImTrunc(off +  ImVec2(title_r.Max.x, title_r.Min.y + title_r.GetHeight() * 3.0f) * scale));
        thumb_r.ClipWithFull(bb);
        title_r.ClipWithFull(bb);
        const bool window_is_focused = (g.NavWindow && thumb_window->RootWindowForTitleBarHighlight == g.NavWindow->RootWindowForTitleBarHighlight);
        window->DrawList->AddRectFilled(thumb_r.Min, thumb_r.Max, GetColorU32(ImGuiCol_WindowBg, alpha_mul));
        window->DrawList->AddRectFilled(title_r.Min, title_r.Max, GetColorU32(window_is_focused ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg, alpha_mul));
        window->DrawList->AddRect(thumb_r.Min, thumb_r.Max, GetColorU32(ImGuiCol_Border, alpha_mul));
        window->DrawList->AddText(g.Font, g.FontSize * 1.0f, title_r.Min, GetColorU32(ImGuiCol_Text, alpha_mul), thumb_window->Name, FindRenderedTextEnd(thumb_window->Name));
    }
    draw_list->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border, alpha_mul));
    if (viewport->ID == g.DebugMetricsConfig.HighlightViewportID)
        window->DrawList->AddRect(bb.Min, bb.Max, IM_COL32(255, 255, 0, 255));
}

static void RenderViewportsThumbnails()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    float SCALE = 1.0f / 8.0f;
    ImRect bb_full(g.Viewports[0]->Pos, g.Viewports[0]->Pos + g.Viewports[0]->Size);
    ImVec2 p = window->DC.CursorPos;
    ImVec2 off = p - bb_full.Min * SCALE;

    for (ImGuiViewportP* viewport : g.Viewports)
    {
        ImRect viewport_draw_bb(off + (viewport->Pos) * SCALE, off + (viewport->Pos + viewport->Size) * SCALE);
        ImGui::DebugRenderViewportThumbnail(window->DrawList, viewport, viewport_draw_bb);
    }
    ImGui::Dummy(bb_full.GetSize() * SCALE);
}

void ImGui::DebugRenderKeyboardPreview(ImDrawList* draw_list)
{
    const float scale = ImGui::GetFontSize() / 13.0f;
    const ImVec2 key_size = ImVec2(35.0f, 35.0f) * scale;
    const float  key_rounding = 3.0f * scale;
    const ImVec2 key_face_size = ImVec2(25.0f, 25.0f) * scale;
    const ImVec2 key_face_pos = ImVec2(5.0f, 3.0f) * scale;
    const float  key_face_rounding = 2.0f * scale;
    const ImVec2 key_label_pos = ImVec2(7.0f, 4.0f) * scale;
    const ImVec2 key_step = ImVec2(key_size.x - 1.0f, key_size.y - 1.0f);
    const float  key_row_offset = 9.0f * scale;

    ImVec2 board_min = GetCursorScreenPos();
    ImVec2 board_max = ImVec2(board_min.x + 3 * key_step.x + 2 * key_row_offset + 10.0f, board_min.y + 3 * key_step.y + 10.0f);
    ImVec2 start_pos = ImVec2(board_min.x + 5.0f - key_step.x, board_min.y);

    struct KeyLayoutData { int Row, Col; const char* Label; ImGuiKey Key; };
    const KeyLayoutData keys_to_display[] =
    {
        { 0, 0, "", ImGuiKey_Tab },      { 0, 1, "Q", ImGuiKey_Q }, { 0, 2, "W", ImGuiKey_W }, { 0, 3, "E", ImGuiKey_E }, { 0, 4, "R", ImGuiKey_R },
        { 1, 0, "", ImGuiKey_CapsLock }, { 1, 1, "A", ImGuiKey_A }, { 1, 2, "S", ImGuiKey_S }, { 1, 3, "D", ImGuiKey_D }, { 1, 4, "F", ImGuiKey_F },
        { 2, 0, "", ImGuiKey_LeftShift },{ 2, 1, "Z", ImGuiKey_Z }, { 2, 2, "X", ImGuiKey_X }, { 2, 3, "C", ImGuiKey_C }, { 2, 4, "V", ImGuiKey_V }
    };

    Dummy(board_max - board_min);
    if (!IsItemVisible())
        return;
    draw_list->PushClipRect(board_min, board_max, true);
    for (int n = 0; n < IM_COUNTOF(keys_to_display); n++)
    {
        const KeyLayoutData* key_data = &keys_to_display[n];
        ImVec2 key_min = ImVec2(start_pos.x + key_data->Col * key_step.x + key_data->Row * key_row_offset, start_pos.y + key_data->Row * key_step.y);
        ImVec2 key_max = key_min + key_size;
        draw_list->AddRectFilled(key_min, key_max, IM_COL32(204, 204, 204, 255), key_rounding);
        draw_list->AddRect(key_min, key_max, IM_COL32(24, 24, 24, 255), key_rounding);
        ImVec2 face_min = ImVec2(key_min.x + key_face_pos.x, key_min.y + key_face_pos.y);
        ImVec2 face_max = ImVec2(face_min.x + key_face_size.x, face_min.y + key_face_size.y);
        draw_list->AddRect(face_min, face_max, IM_COL32(193, 193, 193, 255), key_face_rounding, ImDrawFlags_None, 2.0f);
        draw_list->AddRectFilled(face_min, face_max, IM_COL32(252, 252, 252, 255), key_face_rounding);
        ImVec2 label_min = ImVec2(key_min.x + key_label_pos.x, key_min.y + key_label_pos.y);
        draw_list->AddText(label_min, IM_COL32(64, 64, 64, 255), key_data->Label);
        if (IsKeyDown(key_data->Key))
            draw_list->AddRectFilled(key_min, key_max, IM_COL32(255, 0, 0, 128), key_rounding);
    }
    draw_list->PopClipRect();
}

void ImGui::DebugTextEncoding(const char* str)
{
    Text("Text: \"%s\"", str);
    if (!BeginTable("##DebugTextEncoding", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable))
        return;
    TableSetupColumn("Offset");
    TableSetupColumn("UTF-8");
    TableSetupColumn("Glyph");
    TableSetupColumn("Codepoint");
    TableHeadersRow();
    const char* str_end = str + strlen(str);
    for (const char* p = str; *p != 0; )
    {
        unsigned int c;
        const int c_utf8_len = ImTextCharFromUtf8(&c, p, str_end);
        TableNextColumn();
        Text("%d", (int)(p - str));
        TableNextColumn();
        for (int byte_index = 0; byte_index < c_utf8_len; byte_index++)
        {
            if (byte_index > 0)
                SameLine();
            Text("0x%02X", (int)(unsigned char)p[byte_index]);
        }
        TableNextColumn();
        TextUnformatted(p, p + c_utf8_len);
        if (!GetFont()->IsGlyphInFont((ImWchar)c))
        {
            SameLine();
            TextUnformatted("[missing]");
        }
        TableNextColumn();
        Text("U+%04X", (int)c);
        p += c_utf8_len;
    }
    EndTable();
}

static void DebugFlashStyleColorStop()
{
    ImGuiContext& g = *GImGui;
    if (g.DebugFlashStyleColorIdx != ImGuiCol_COUNT)
        g.Style.Colors[g.DebugFlashStyleColorIdx] = g.DebugFlashStyleColorBackup;
    g.DebugFlashStyleColorIdx = ImGuiCol_COUNT;
}

void ImGui::DebugFlashStyleColor(ImGuiCol idx)
{
    ImGuiContext& g = *GImGui;
    DebugFlashStyleColorStop();
    g.DebugFlashStyleColorTime = 0.5f;
    g.DebugFlashStyleColorIdx = idx;
    g.DebugFlashStyleColorBackup = g.Style.Colors[idx];
}

void ImGui::UpdateDebugToolFlashStyleColor()
{
    ImGuiContext& g = *GImGui;
    if (g.DebugFlashStyleColorTime <= 0.0f)
        return;
    ColorConvertHSVtoRGB(ImCos(g.DebugFlashStyleColorTime * 6.0f) * 0.5f + 0.5f, 0.5f, 0.5f, g.Style.Colors[g.DebugFlashStyleColorIdx].x, g.Style.Colors[g.DebugFlashStyleColorIdx].y, g.Style.Colors[g.DebugFlashStyleColorIdx].z);
    g.Style.Colors[g.DebugFlashStyleColorIdx].w = 1.0f;
    if ((g.DebugFlashStyleColorTime -= g.IO.DeltaTime) <= 0.0f)
        DebugFlashStyleColorStop();
}

ImU64 ImGui::DebugTextureIDToU64(ImTextureID tex_id)
{
    ImU64 v = 0;
    memcpy(&v, &tex_id, ImMin(sizeof(ImU64), sizeof(ImTextureID)));
    return v;
}

static const char* FormatTextureRefForDebugDisplay(char* buf, int buf_size, ImTextureRef tex_ref)
{
    char* buf_p = buf;
    char* buf_end = buf + buf_size;
    if (tex_ref._TexData != NULL)
        buf_p += ImFormatString(buf_p, buf_end - buf_p, "#%03d: ", tex_ref._TexData->UniqueID);
    ImFormatString(buf_p, buf_end - buf_p, "0x%X", ImGui::DebugTextureIDToU64(tex_ref.GetTexID()));
    return buf;
}

#ifdef IMGUI_ENABLE_FREETYPE
namespace ImGuiFreeType { IMGUI_API const ImFontLoader* GetFontLoader(); IMGUI_API bool DebugEditFontLoaderFlags(unsigned int* p_font_builder_flags); }
#endif

void ImGui::ShowFontAtlas(ImFontAtlas* atlas)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    ImGuiStyle& style = g.Style;

    BeginDisabled();
    CheckboxFlags("io.BackendFlags: RendererHasTextures", &io.BackendFlags, ImGuiBackendFlags_RendererHasTextures);
    EndDisabled();
    ShowFontSelector("Font");

    if (DragFloat("FontSizeBase", &style.FontSizeBase, 0.20f, 5.0f, 100.0f, "%.0f"))
        style._NextFrameFontSizeBase = style.FontSizeBase;
    SameLine(0.0f, 0.0f); Text(" (out %.2f)", GetFontSize());
    SameLine(); MetricsHelpMarker("- This is scaling font only. General scaling will come later.");
    DragFloat("FontScaleMain", &style.FontScaleMain, 0.02f, 0.5f, 4.0f);

    DragFloat("FontScaleDpi", &style.FontScaleDpi, 0.02f, 0.5f, 4.0f);

    if ((io.BackendFlags & ImGuiBackendFlags_RendererHasTextures) == 0)
    {
        BulletText("Warning: Font scaling will NOT be smooth, because\nImGuiBackendFlags_RendererHasTextures is not set!");
        BulletText("For instructions, see:");
        SameLine();
        TextLinkOpenURL("docs/BACKENDS.md", "https://github.com/ocornut/imgui/blob/master/docs/BACKENDS.md");
    }
    BulletText("Load a nice font for better results!");
    BulletText("Please submit feedback:");
    SameLine(); TextLinkOpenURL("#8465", "https://github.com/ocornut/imgui/issues/8465");
    BulletText("Read FAQ for more details:");
    SameLine(); TextLinkOpenURL("dearimgui.com/faq", "https://www.dearimgui.com/faq/");

    SeparatorText("Font List");

    ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
    Checkbox("Show font preview", &cfg->ShowFontPreview);

    if (TreeNode("Loader", "Loader: \'%s\'", atlas->FontLoaderName ? atlas->FontLoaderName : "NULL"))
    {
        const ImFontLoader* loader_current = atlas->FontLoader;
        BeginDisabled(!atlas->RendererHasTextures);
#ifdef IMGUI_ENABLE_STB_TRUETYPE
        const ImFontLoader* loader_stbtruetype = ImFontAtlasGetFontLoaderForStbTruetype();
        if (RadioButton("stb_truetype", loader_current == loader_stbtruetype))
            atlas->SetFontLoader(loader_stbtruetype);
#else
        BeginDisabled();
        RadioButton("stb_truetype", false);
        SetItemTooltip("Requires #define IMGUI_ENABLE_STB_TRUETYPE");
        EndDisabled();
#endif
        SameLine();
#ifdef IMGUI_ENABLE_FREETYPE
        const ImFontLoader* loader_freetype = ImGuiFreeType::GetFontLoader();
        if (RadioButton("FreeType", loader_current == loader_freetype))
            atlas->SetFontLoader(loader_freetype);
        if (loader_current == loader_freetype)
        {
            unsigned int loader_flags = atlas->FontLoaderFlags;
            Text("Shared FreeType Loader Flags:  0x%08X", loader_flags);
            if (ImGuiFreeType::DebugEditFontLoaderFlags(&loader_flags))
            {
                for (ImFont* font : atlas->Fonts)
                    ImFontAtlasFontDestroyOutput(atlas, font);
                atlas->FontLoaderFlags = loader_flags;
                for (ImFont* font : atlas->Fonts)
                    ImFontAtlasFontInitOutput(atlas, font);
            }
        }
#else
        BeginDisabled();
        RadioButton("FreeType", false);
        SetItemTooltip("Requires #define IMGUI_ENABLE_FREETYPE + imgui_freetype.cpp.");
        EndDisabled();
#endif
        EndDisabled();
        TreePop();
    }

    for (ImFont* font : atlas->Fonts)
    {
        PushID(font);
        DebugNodeFont(font);
        PopID();
    }

    SeparatorText("Font Atlas");
    if (Button("Compact"))
        atlas->CompactCache();
    SameLine();
    if (Button("Grow"))
        ImFontAtlasTextureGrow(atlas);
    SameLine();
    if (Button("Clear All"))
        ImFontAtlasBuildClear(atlas);
    SetItemTooltip("Destroy cache and custom rectangles.");

    for (int tex_n = 0; tex_n < atlas->TexList.Size; tex_n++)
    {
        ImTextureData* tex = atlas->TexList[tex_n];
        if (tex_n > 0)
            SameLine();
        Text("Tex: %dx%d", tex->Width, tex->Height);
    }
    const int packed_surface_sqrt = (int)sqrtf((float)atlas->Builder->RectsPackedSurface);
    const int discarded_surface_sqrt = (int)sqrtf((float)atlas->Builder->RectsDiscardedSurface);
    Text("Packed rects: %d, area: about %d px ~%dx%d px", atlas->Builder->RectsPackedCount, atlas->Builder->RectsPackedSurface, packed_surface_sqrt, packed_surface_sqrt);
    Text("incl. Discarded rects: %d, area: about %d px ~%dx%d px", atlas->Builder->RectsDiscardedCount, atlas->Builder->RectsDiscardedSurface, discarded_surface_sqrt, discarded_surface_sqrt);

    ImFontAtlasRectId highlight_r_id = ImFontAtlasRectId_Invalid;
    if (TreeNode("Rects Index", "Rects Index (%d)", atlas->Builder->RectsPackedCount))
    {
        PushStyleVar(ImGuiStyleVar_ImageBorderSize, 1.0f);
        if (BeginTable("##table", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImVec2(0.0f, GetTextLineHeightWithSpacing() * 12)))
        {
            for (const ImFontAtlasRectEntry& entry : atlas->Builder->RectsIndex)
                if (entry.IsUsed)
                {
                    ImFontAtlasRectId id = ImFontAtlasRectId_Make(atlas->Builder->RectsIndex.index_from_ptr(&entry), entry.Generation);
                    ImFontAtlasRect r = {};
                    atlas->GetCustomRect(id, &r);
                    const char* buf;
                    ImFormatStringToTempBuffer(&buf, NULL, "ID:%08X, used:%d, { w:%3d, h:%3d } { x:%4d, y:%4d }", id, entry.IsUsed, r.w, r.h, r.x, r.y);
                    TableNextColumn();
                    Selectable(buf);
                    if (IsItemHovered())
                        highlight_r_id = id;
                    TableNextColumn();
                    Image(atlas->TexRef, ImVec2(r.w, r.h), r.uv0, r.uv1);
                }
            EndTable();
        }
        PopStyleVar();
        TreePop();
    }

    ImFontAtlasRect highlight_r;
    if (highlight_r_id != ImFontAtlasRectId_Invalid)
        atlas->GetCustomRect(highlight_r_id, &highlight_r);
    for (int tex_n = 0; tex_n < atlas->TexList.Size; tex_n++)
    {
        if (tex_n == atlas->TexList.Size - 1)
            SetNextItemOpen(true, ImGuiCond_Once);
        DebugNodeTexture(atlas->TexList[tex_n], atlas->TexList.Size - 1 - tex_n, (highlight_r_id != ImFontAtlasRectId_Invalid) ? &highlight_r : NULL);
    }
}

void ImGui::DebugNodeTexture(ImTextureData* tex, int int_id, const ImFontAtlasRect* highlight_rect)
{
    ImGuiContext& g = *GImGui;
    PushID(int_id);
    if (TreeNode("", "Texture #%03d (%dx%d pixels)", tex->UniqueID, tex->Width, tex->Height))
    {
        ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
        Checkbox("Show used rect", &cfg->ShowTextureUsedRect);
        PushStyleVar(ImGuiStyleVar_ImageBorderSize, ImMax(1.0f, g.Style.ImageBorderSize));
        ImVec2 p = GetCursorScreenPos();
        if (tex->WantDestroyNextFrame)
            Dummy(ImVec2((float)tex->Width, (float)tex->Height));
        else
            ImageWithBg(tex->GetTexRef(), ImVec2((float)tex->Width, (float)tex->Height), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        if (cfg->ShowTextureUsedRect)
            GetWindowDrawList()->AddRect(ImVec2(p.x + tex->UsedRect.x, p.y + tex->UsedRect.y), ImVec2(p.x + tex->UsedRect.x + tex->UsedRect.w, p.y + tex->UsedRect.y + tex->UsedRect.h), IM_COL32(255, 0, 255, 255));
        if (highlight_rect != NULL)
        {
            ImRect r_outer(p.x, p.y, p.x + tex->Width, p.y + tex->Height);
            ImRect r_inner(p.x + highlight_rect->x, p.y + highlight_rect->y, p.x + highlight_rect->x + highlight_rect->w, p.y + highlight_rect->y + highlight_rect->h);
            RenderRectFilledWithHole(GetWindowDrawList(), r_outer, r_inner, IM_COL32(0, 0, 0, 100), 0.0f);
            GetWindowDrawList()->AddRect(r_inner.Min - ImVec2(1, 1), r_inner.Max + ImVec2(1, 1), IM_COL32(255, 255, 0, 255));
        }
        PopStyleVar();

        char texref_desc[30];
        Text("Status = %s (%d), Format = %s (%d), UseColors = %d", ImTextureDataGetStatusName(tex->Status), tex->Status, ImTextureDataGetFormatName(tex->Format), tex->Format, tex->UseColors);
        Text("TexRef = %s, BackendUserData = %p", FormatTextureRefForDebugDisplay(texref_desc, IM_COUNTOF(texref_desc), tex->GetTexRef()), tex->BackendUserData);
        TreePop();
    }
    PopID();
}

void ImGui::ShowMetricsWindow(bool* p_open)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
    if (cfg->ShowDebugLog)
        ShowDebugLogWindow(&cfg->ShowDebugLog);
    if (cfg->ShowIDStackTool)
        ShowIDStackToolWindow(&cfg->ShowIDStackTool);

    if (!Begin("Dear ImGui Metrics/Debugger", p_open) || GetCurrentWindow()->BeginCount > 1)
    {
        End();
        return;
    }

    DebugBreakClearData();

    Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
    if (g.ContextName[0] != 0)
    {
        SameLine();
        Text("(Context Name: \"%s\")", g.ContextName);
    }
    Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices, io.MetricsRenderIndices, io.MetricsRenderIndices / 3);
    Text("%d visible windows, %d current allocations", io.MetricsRenderWindows, g.DebugAllocInfo.TotalAllocCount - g.DebugAllocInfo.TotalFreeCount);

    Separator();

    enum { WRT_OuterRect, WRT_OuterRectClipped, WRT_InnerRect, WRT_InnerClipRect, WRT_WorkRect, WRT_Content, WRT_ContentIdeal, WRT_ContentRegionRect, WRT_Count };
    const char* wrt_rects_names[WRT_Count] = { "OuterRect", "OuterRectClipped", "InnerRect", "InnerClipRect", "WorkRect", "Content", "ContentIdeal", "ContentRegionRect" };
    enum { TRT_OuterRect, TRT_InnerRect, TRT_WorkRect, TRT_HostClipRect, TRT_InnerClipRect, TRT_BackgroundClipRect, TRT_ColumnsRect, TRT_ColumnsWorkRect, TRT_ColumnsClipRect, TRT_ColumnsContentHeadersUsed, TRT_ColumnsContentHeadersIdeal, TRT_ColumnsContentFrozen, TRT_ColumnsContentUnfrozen, TRT_Count };
    const char* trt_rects_names[TRT_Count] = { "OuterRect", "InnerRect", "WorkRect", "HostClipRect", "InnerClipRect", "BackgroundClipRect", "ColumnsRect", "ColumnsWorkRect", "ColumnsClipRect", "ColumnsContentHeadersUsed", "ColumnsContentHeadersIdeal", "ColumnsContentFrozen", "ColumnsContentUnfrozen" };
    if (cfg->ShowWindowsRectsType < 0)
        cfg->ShowWindowsRectsType = WRT_WorkRect;
    if (cfg->ShowTablesRectsType < 0)
        cfg->ShowTablesRectsType = TRT_WorkRect;

    struct Funcs
    {
        static ImRect GetTableRect(ImGuiTable* table, int rect_type, int n)
        {
            ImGuiTableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
            if (rect_type == TRT_OuterRect)                     { return table->OuterRect; }
            else if (rect_type == TRT_InnerRect)                { return table->InnerRect; }
            else if (rect_type == TRT_WorkRect)                 { return table->WorkRect; }
            else if (rect_type == TRT_HostClipRect)             { return table->HostClipRect; }
            else if (rect_type == TRT_InnerClipRect)            { return table->InnerClipRect; }
            else if (rect_type == TRT_BackgroundClipRect)       { return table->BgClipRect; }
            else if (rect_type == TRT_ColumnsRect)              { ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->MinX, table->InnerClipRect.Min.y, c->MaxX, table->InnerClipRect.Min.y + table_instance->LastOuterHeight); }
            else if (rect_type == TRT_ColumnsWorkRect)          { ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->WorkRect.Min.y, c->WorkMaxX, table->WorkRect.Max.y); }
            else if (rect_type == TRT_ColumnsClipRect)          { ImGuiTableColumn* c = &table->Columns[n]; return c->ClipRect; }
            else if (rect_type == TRT_ColumnsContentHeadersUsed){ ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXHeadersUsed, table->InnerClipRect.Min.y + table_instance->LastTopHeadersRowHeight); }
            else if (rect_type == TRT_ColumnsContentHeadersIdeal){ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXHeadersIdeal, table->InnerClipRect.Min.y + table_instance->LastTopHeadersRowHeight); }
            else if (rect_type == TRT_ColumnsContentFrozen)     { ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->InnerClipRect.Min.y, c->ContentMaxXFrozen, table->InnerClipRect.Min.y + table_instance->LastFrozenHeight); }
            else if (rect_type == TRT_ColumnsContentUnfrozen)   { ImGuiTableColumn* c = &table->Columns[n]; return ImRect(c->WorkMinX, table->InnerClipRect.Min.y + table_instance->LastFrozenHeight, c->ContentMaxXUnfrozen, table->InnerClipRect.Max.y); }
            IM_ASSERT(0);
            return ImRect();
        }

        static ImRect GetWindowRect(ImGuiWindow* window, int rect_type)
        {
            if (rect_type == WRT_OuterRect)                 { return window->Rect(); }
            else if (rect_type == WRT_OuterRectClipped)     { return window->OuterRectClipped; }
            else if (rect_type == WRT_InnerRect)            { return window->InnerRect; }
            else if (rect_type == WRT_InnerClipRect)        { return window->InnerClipRect; }
            else if (rect_type == WRT_WorkRect)             { return window->WorkRect; }
            else if (rect_type == WRT_Content)              { ImVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding; return ImRect(min, min + window->ContentSize); }
            else if (rect_type == WRT_ContentIdeal)         { ImVec2 min = window->InnerRect.Min - window->Scroll + window->WindowPadding; return ImRect(min, min + window->ContentSizeIdeal); }
            else if (rect_type == WRT_ContentRegionRect)    { return window->ContentRegionRect; }
            IM_ASSERT(0);
            return ImRect();
        }
    };

#ifdef IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS
    TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS is enabled.\nMust disable after use! Otherwise Dear ImGui will run slower.\n");
#endif

    if (TreeNode("Tools"))
    {

        SeparatorTextEx(0, "Debug breaks", NULL, CalcTextSize("(?)").x + g.Style.SeparatorTextPadding.x);
        SameLine();
        MetricsHelpMarker("Will call the IM_DEBUG_BREAK() macro to break in debugger.\nWarning: If you don't have a debugger attached, this will probably crash.");
        if (Checkbox("Show Item Picker", &g.DebugItemPickerActive) && g.DebugItemPickerActive)
            DebugStartItemPicker();
        Checkbox("Show \"Debug Break\" buttons in other sections (io.ConfigDebugIsDebuggerPresent)", &g.IO.ConfigDebugIsDebuggerPresent);

        SeparatorText("Visualize");

        Checkbox("Show Debug Log", &cfg->ShowDebugLog);
        SameLine();
        MetricsHelpMarker("You can also call ImGui::ShowDebugLogWindow() from your code.");

        Checkbox("Show ID Stack Tool", &cfg->ShowIDStackTool);
        SameLine();
        MetricsHelpMarker("You can also call ImGui::ShowIDStackToolWindow() from your code.");

        Checkbox("Show windows begin order", &cfg->ShowWindowsBeginOrder);
        Checkbox("Show windows rectangles", &cfg->ShowWindowsRects);
        SameLine();
        SetNextItemWidth(GetFontSize() * 12);
        cfg->ShowWindowsRects |= Combo("##show_windows_rect_type", &cfg->ShowWindowsRectsType, wrt_rects_names, WRT_Count, WRT_Count);
        if (cfg->ShowWindowsRects && g.NavWindow != NULL)
        {
            BulletText("'%s':", g.NavWindow->Name);
            Indent();
            for (int rect_n = 0; rect_n < WRT_Count; rect_n++)
            {
                ImRect r = Funcs::GetWindowRect(g.NavWindow, rect_n);
                Text("(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) %s", r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(), wrt_rects_names[rect_n]);
            }
            Unindent();
        }

        Checkbox("Show tables rectangles", &cfg->ShowTablesRects);
        SameLine();
        SetNextItemWidth(GetFontSize() * 12);
        cfg->ShowTablesRects |= Combo("##show_table_rects_type", &cfg->ShowTablesRectsType, trt_rects_names, TRT_Count, TRT_Count);
        if (cfg->ShowTablesRects && g.NavWindow != NULL)
        {
            for (int table_n = 0; table_n < g.Tables.GetMapSize(); table_n++)
            {
                ImGuiTable* table = g.Tables.TryGetMapData(table_n);
                if (table == NULL || table->LastFrameActive < g.FrameCount - 1 || (table->OuterWindow != g.NavWindow && table->InnerWindow != g.NavWindow))
                    continue;

                BulletText("Table 0x%08X (%d columns, in '%s')", table->ID, table->ColumnsCount, table->OuterWindow->Name);
                if (IsItemHovered())
                    GetForegroundDrawList(table->OuterWindow)->AddRect(table->OuterRect.Min - ImVec2(1, 1), table->OuterRect.Max + ImVec2(1, 1), IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
                Indent();
                char buf[128];
                for (int rect_n = 0; rect_n < TRT_Count; rect_n++)
                {
                    if (rect_n >= TRT_ColumnsRect)
                    {
                        if (rect_n != TRT_ColumnsRect && rect_n != TRT_ColumnsClipRect)
                            continue;
                        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                        {
                            ImRect r = Funcs::GetTableRect(table, rect_n, column_n);
                            ImFormatString(buf, IM_COUNTOF(buf), "(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) Col %d %s", r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(), column_n, trt_rects_names[rect_n]);
                            Selectable(buf);
                            if (IsItemHovered())
                                GetForegroundDrawList(table->OuterWindow)->AddRect(r.Min - ImVec2(1, 1), r.Max + ImVec2(1, 1), IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
                        }
                    }
                    else
                    {
                        ImRect r = Funcs::GetTableRect(table, rect_n, -1);
                        ImFormatString(buf, IM_COUNTOF(buf), "(%6.1f,%6.1f) (%6.1f,%6.1f) Size (%6.1f,%6.1f) %s", r.Min.x, r.Min.y, r.Max.x, r.Max.y, r.GetWidth(), r.GetHeight(), trt_rects_names[rect_n]);
                        Selectable(buf);
                        if (IsItemHovered())
                            GetForegroundDrawList(table->OuterWindow)->AddRect(r.Min - ImVec2(1, 1), r.Max + ImVec2(1, 1), IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
                    }
                }
                Unindent();
            }
        }
        Checkbox("Show groups rectangles", &g.DebugShowGroupRects);

        SeparatorText("Validate");

        Checkbox("Debug Begin/BeginChild return value", &io.ConfigDebugBeginReturnValueLoop);
        SameLine();
        MetricsHelpMarker("Some calls to Begin()/BeginChild() will return false.\n\nWill cycle through window depths then repeat. Windows should be flickering while running.");

        Checkbox("UTF-8 Encoding viewer", &cfg->ShowTextEncodingViewer);
        SameLine();
        MetricsHelpMarker("You can also call ImGui::DebugTextEncoding() from your code with a given string to test that your UTF-8 encoding settings are correct.");
        if (cfg->ShowTextEncodingViewer)
        {
            static char buf[64] = "";
            SetNextItemWidth(-FLT_MIN);
            InputText("##DebugTextEncodingBuf", buf, IM_COUNTOF(buf));
            if (buf[0] != 0)
                DebugTextEncoding(buf);
        }

        TreePop();
    }

    if (TreeNode("Windows", "Windows (%d)", g.Windows.Size))
    {

        DebugNodeWindowsList(&g.Windows, "By display order");
        DebugNodeWindowsList(&g.WindowsFocusOrder, "By focus order (root windows)");
        if (TreeNode("By submission order (begin stack)"))
        {

            ImVector<ImGuiWindow*>& temp_buffer = g.WindowsTempSortBuffer;
            temp_buffer.resize(0);
            for (ImGuiWindow* window : g.Windows)
                if (window->LastFrameActive + 1 >= g.FrameCount)
                    temp_buffer.push_back(window);
            struct Func { static int IMGUI_CDECL WindowComparerByBeginOrder(const void* lhs, const void* rhs) { return ((int)(*(const ImGuiWindow* const *)lhs)->BeginOrderWithinContext - (*(const ImGuiWindow* const*)rhs)->BeginOrderWithinContext); } };
            ImQsort(temp_buffer.Data, (size_t)temp_buffer.Size, sizeof(ImGuiWindow*), Func::WindowComparerByBeginOrder);
            DebugNodeWindowsListByBeginStackParent(temp_buffer.Data, temp_buffer.Size, NULL);
            TreePop();
        }

        TreePop();
    }

    int drawlist_count = 0;
    for (ImGuiViewportP* viewport : g.Viewports)
        drawlist_count += viewport->DrawDataP.CmdLists.Size;
    if (TreeNode("DrawLists", "DrawLists (%d)", drawlist_count))
    {
        Checkbox("Show ImDrawCmd mesh when hovering", &cfg->ShowDrawCmdMesh);
        Checkbox("Show ImDrawCmd bounding boxes when hovering", &cfg->ShowDrawCmdBoundingBoxes);
        for (ImGuiViewportP* viewport : g.Viewports)
            for (ImDrawList* draw_list : viewport->DrawDataP.CmdLists)
                DebugNodeDrawList(NULL, viewport, draw_list, "DrawList");
        TreePop();
    }

    if (TreeNode("Viewports", "Viewports (%d)", g.Viewports.Size))
    {
        SetNextItemOpen(true, ImGuiCond_Once);
        if (TreeNode("Windows Minimap"))
        {
            RenderViewportsThumbnails();
            TreePop();
        }
        cfg->HighlightViewportID = 0;

        for (ImGuiViewportP* viewport : g.Viewports)
            DebugNodeViewport(viewport);
        TreePop();
    }

    for (ImFontAtlas* atlas : g.FontAtlases)
        if (TreeNode((void*)atlas, "Fonts (%d), Textures (%d)", atlas->Fonts.Size, atlas->TexList.Size))
        {
            ShowFontAtlas(atlas);
            TreePop();
        }

    if (TreeNode("Popups", "Popups (%d)", g.OpenPopupStack.Size))
    {
        for (const ImGuiPopupData& popup_data : g.OpenPopupStack)
        {

            ImGuiWindow* window = popup_data.Window;
            BulletText("PopupID: %08x, Window: '%s' (%s%s), RestoreNavWindow '%s', ParentWindow '%s'",
                popup_data.PopupId, window ? window->Name : "NULL", window && (window->Flags & ImGuiWindowFlags_ChildWindow) ? "Child;" : "", window && (window->Flags & ImGuiWindowFlags_ChildMenu) ? "Menu;" : "",
                popup_data.RestoreNavWindow ? popup_data.RestoreNavWindow->Name : "NULL", window && window->ParentWindow ? window->ParentWindow->Name : "NULL");
        }
        TreePop();
    }

    if (TreeNode("TabBars", "Tab Bars (%d)", g.TabBars.GetAliveCount()))
    {
        for (int n = 0; n < g.TabBars.GetMapSize(); n++)
            if (ImGuiTabBar* tab_bar = g.TabBars.TryGetMapData(n))
            {
                PushID(tab_bar);
                DebugNodeTabBar(tab_bar, "TabBar");
                PopID();
            }
        TreePop();
    }

    if (TreeNode("Tables", "Tables (%d)", g.Tables.GetAliveCount()))
    {
        for (int n = 0; n < g.Tables.GetMapSize(); n++)
            if (ImGuiTable* table = g.Tables.TryGetMapData(n))
                DebugNodeTable(table);
        TreePop();
    }

    if (TreeNode("InputText"))
    {
        DebugNodeInputTextState(&g.InputTextState);
        TreePop();
    }

    if (TreeNode("TypingSelect", "TypingSelect (%d)", g.TypingSelectState.SearchBuffer[0] != 0 ? 1 : 0))
    {
        DebugNodeTypingSelectState(&g.TypingSelectState);
        TreePop();
    }

    if (TreeNode("MultiSelect", "MultiSelect (%d)", g.MultiSelectStorage.GetAliveCount()))
    {
        ImGuiBoxSelectState* bs = &g.BoxSelectState;
        BulletText("BoxSelect ID=0x%08X, Starting = %d, Active %d", bs->ID, bs->IsStarting, bs->IsActive);
        for (int n = 0; n < g.MultiSelectStorage.GetMapSize(); n++)
            if (ImGuiMultiSelectState* state = g.MultiSelectStorage.TryGetMapData(n))
                DebugNodeMultiSelectState(state);
        TreePop();
    }

#ifdef IMGUI_HAS_DOCK
    if (TreeNode("Docking"))
    {
        TreePop();
    }
#endif

    if (TreeNode("Settings"))
    {
        if (SmallButton("Clear"))
            ClearIniSettings();
        SameLine();
        if (SmallButton("Save to memory"))
            SaveIniSettingsToMemory();
        SameLine();
        if (SmallButton("Save to disk"))
            SaveIniSettingsToDisk(g.IO.IniFilename);
        SameLine();
        if (g.IO.IniFilename)
            Text("\"%s\"", g.IO.IniFilename);
        else
            TextUnformatted("<NULL>");
        Checkbox("io.ConfigDebugIniSettings", &io.ConfigDebugIniSettings);
        Text("SettingsDirtyTimer %.2f", g.SettingsDirtyTimer);
        if (TreeNode("SettingsHandlers", "Settings handlers: (%d)", g.SettingsHandlers.Size))
        {
            for (ImGuiSettingsHandler& handler : g.SettingsHandlers)
                BulletText("\"%s\"", handler.TypeName);
            TreePop();
        }
        if (TreeNode("SettingsWindows", "Settings packed data: Windows: %d bytes", g.SettingsWindows.size()))
        {
            for (ImGuiWindowSettings* settings = g.SettingsWindows.begin(); settings != NULL; settings = g.SettingsWindows.next_chunk(settings))
                DebugNodeWindowSettings(settings);
            TreePop();
        }

        if (TreeNode("SettingsTables", "Settings packed data: Tables: %d bytes", g.SettingsTables.size()))
        {
            for (ImGuiTableSettings* settings = g.SettingsTables.begin(); settings != NULL; settings = g.SettingsTables.next_chunk(settings))
                DebugNodeTableSettings(settings);
            TreePop();
        }

#ifdef IMGUI_HAS_DOCK
#endif

        if (TreeNode("SettingsIniData", "Settings unpacked data (.ini): %d bytes", g.SettingsIniData.size()))
        {
            InputTextMultiline("##Ini", (char*)(void*)g.SettingsIniData.c_str(), g.SettingsIniData.Buf.Size, ImVec2(-FLT_MIN, GetTextLineHeight() * 20), ImGuiInputTextFlags_ReadOnly);
            TreePop();
        }
        TreePop();
    }

    if (TreeNode("Memory allocations"))
    {
        ImGuiDebugAllocInfo* info = &g.DebugAllocInfo;
        Text("%d current allocations", info->TotalAllocCount - info->TotalFreeCount);
        Text("Releasing selected unused buffers after: %.2f secs", g.IO.ConfigMemoryCompactTimer);
        if (SmallButton("GC now")) { g.GcCompactAll = true; }
        Text("Recent frames with allocations:");
        int buf_size = IM_COUNTOF(info->LastEntriesBuf);
        for (int n = buf_size - 1; n >= 0; n--)
        {
            ImGuiDebugAllocEntry* entry = &info->LastEntriesBuf[(info->LastEntriesIdx - n + buf_size) % buf_size];
            BulletText("Frame %06d: %+3d ( %2d alloc, %2d free )", entry->FrameCount, entry->AllocCount - entry->FreeCount, entry->AllocCount, entry->FreeCount);
            if (n == 0)
            {
                SameLine();
                Text("<- %d frames ago", g.FrameCount - entry->FrameCount);
            }
        }
        TreePop();
    }

    if (TreeNode("Inputs"))
    {
        Text("KEYBOARD/GAMEPAD/MOUSE KEYS");
        {

            Indent();
            Text("Keys down:");         for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (!IsKeyDown(key)) continue;     SameLine(); Text(IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key); SameLine(); Text("(%.02f)", GetKeyData(key)->DownDuration); }
            Text("Keys pressed:");      for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (!IsKeyPressed(key)) continue;  SameLine(); Text(IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key); }
            Text("Keys released:");     for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (!IsKeyReleased(key)) continue; SameLine(); Text(IsNamedKey(key) ? "\"%s\"" : "\"%s\" %d", GetKeyName(key), key); }
            Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "Ctrl " : "", io.KeyShift ? "Shift " : "", io.KeyAlt ? "Alt " : "", io.KeySuper ? "Super " : "");
            Text("Chars queue:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; SameLine(); Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); }
            DebugRenderKeyboardPreview(GetWindowDrawList());
            Unindent();
        }

        Text("MOUSE STATE");
        {
            Indent();
            if (IsMousePosValid())
                Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else
                Text("Mouse pos: <INVALID>");
            Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
            int count = IM_COUNTOF(io.MouseDown);
            Text("Mouse down:");     for (int i = 0; i < count; i++) if (IsMouseDown(i)) { SameLine(); Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
            Text("Mouse clicked:");  for (int i = 0; i < count; i++) if (IsMouseClicked(i)) { SameLine(); Text("b%d (%d)", i, io.MouseClickedCount[i]); }
            Text("Mouse released:"); for (int i = 0; i < count; i++) if (IsMouseReleased(i)) { SameLine(); Text("b%d", i); }
            Text("Mouse wheel: %.1f", io.MouseWheel);
            Text("MouseStationaryTimer: %.2f", g.MouseStationaryTimer);
            Text("Mouse source: %s", GetMouseSourceName(io.MouseSource));
            Text("Pen Pressure: %.1f", io.PenPressure);
            Unindent();
        }

        Text("MOUSE WHEELING");
        {
            Indent();
            Text("WheelingWindow: '%s'", g.WheelingWindow ? g.WheelingWindow->Name : "NULL");
            Text("WheelingWindowReleaseTimer: %.2f", g.WheelingWindowReleaseTimer);
            Text("WheelingAxisAvg[] = { %.3f, %.3f }, Main Axis: %s", g.WheelingAxisAvg.x, g.WheelingAxisAvg.y, (g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? "X" : (g.WheelingAxisAvg.x < g.WheelingAxisAvg.y) ? "Y" : "<none>");
            Unindent();
        }

        Text("KEY OWNERS");
        {
            Indent();
            if (BeginChild("##owners", ImVec2(-FLT_MIN, GetTextLineHeightWithSpacing() * 8), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoSavedSettings))
                for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
                {
                    ImGuiKeyOwnerData* owner_data = GetKeyOwnerData(&g, key);
                    if (owner_data->OwnerCurr == ImGuiKeyOwner_NoOwner)
                        continue;
                    Text("%s: 0x%08X%s", GetKeyName(key), owner_data->OwnerCurr,
                        owner_data->LockUntilRelease ? " LockUntilRelease" : owner_data->LockThisFrame ? " LockThisFrame" : "");
                    DebugLocateItemOnHover(owner_data->OwnerCurr);
                }
            EndChild();
            Unindent();
        }
        Text("SHORTCUT ROUTING");
        SameLine();
        MetricsHelpMarker("Declared shortcut routes automatically set key owner when mods matches.");
        {
            Indent();
            if (BeginChild("##routes", ImVec2(-FLT_MIN, GetTextLineHeightWithSpacing() * 8), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoSavedSettings))
                for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
                {
                    ImGuiKeyRoutingTable* rt = &g.KeysRoutingTable;
                    for (ImGuiKeyRoutingIndex idx = rt->Index[key - ImGuiKey_NamedKey_BEGIN]; idx != -1; )
                    {
                        ImGuiKeyRoutingData* routing_data = &rt->Entries[idx];
                        ImGuiKeyChord key_chord = key | routing_data->Mods;
                        Text("%s: 0x%08X (scored %d)", GetKeyChordName(key_chord), routing_data->RoutingCurr, routing_data->RoutingCurrScore);
                        DebugLocateItemOnHover(routing_data->RoutingCurr);
                        if (g.IO.ConfigDebugIsDebuggerPresent)
                        {
                            SameLine();
                            if (DebugBreakButton("**DebugBreak**", "in SetShortcutRouting() for this KeyChord"))
                                g.DebugBreakInShortcutRouting = key_chord;
                        }
                        idx = routing_data->NextEntryIndex;
                    }
                }
            EndChild();
            Text("(ActiveIdUsing: AllKeyboardKeys: %d, NavDirMask: 0x%X)", g.ActiveIdUsingAllKeyboardKeys, g.ActiveIdUsingNavDirMask);
            Unindent();
        }
        TreePop();
    }

    if (TreeNode("Internal state"))
    {
        Text("WINDOWING");
        Indent();
        Text("HoveredWindow: '%s'", g.HoveredWindow ? g.HoveredWindow->Name : "NULL");
        Text("HoveredWindow->Root: '%s'", g.HoveredWindow ? g.HoveredWindow->RootWindow->Name : "NULL");
        Text("HoveredWindowUnderMovingWindow: '%s'", g.HoveredWindowUnderMovingWindow ? g.HoveredWindowUnderMovingWindow->Name : "NULL");
        Text("MovingWindow: '%s'", g.MovingWindow ? g.MovingWindow->Name : "NULL");
        Unindent();

        Text("ITEMS");
        Indent();
        Text("ActiveId: 0x%08X/0x%08X (%.2f sec), AllowOverlap: %d, Source: %s", g.ActiveId, g.ActiveIdPreviousFrame, g.ActiveIdTimer, g.ActiveIdAllowOverlap, GetInputSourceName(g.ActiveIdSource));
        DebugLocateItemOnHover(g.ActiveId);
        Text("ActiveIdWindow: '%s'", g.ActiveIdWindow ? g.ActiveIdWindow->Name : "NULL");
        Text("ActiveIdUsing: AllKeyboardKeys: %d, NavDirMask: %X", g.ActiveIdUsingAllKeyboardKeys, g.ActiveIdUsingNavDirMask);
        Text("HoveredId: 0x%08X (%.2f sec), AllowOverlap: %d", g.HoveredIdPreviousFrame, g.HoveredIdTimer, g.HoveredIdAllowOverlap);
        Text("HoverItemDelayId: 0x%08X, Timer: %.2f, ClearTimer: %.2f", g.HoverItemDelayId, g.HoverItemDelayTimer, g.HoverItemDelayClearTimer);
        Text("DragDrop: %d, SourceId = 0x%08X, Payload \"%s\" (%d bytes)", g.DragDropActive, g.DragDropPayload.SourceId, g.DragDropPayload.DataType, g.DragDropPayload.DataSize);
        DebugLocateItemOnHover(g.DragDropPayload.SourceId);
        Unindent();

        Text("NAV,FOCUS");
        Indent();
        Text("NavWindow: '%s'", g.NavWindow ? g.NavWindow->Name : "NULL");
        Text("NavId: 0x%08X, NavLayer: %d", g.NavId, g.NavLayer);
        DebugLocateItemOnHover(g.NavId);
        Text("NavInputSource: %s", GetInputSourceName(g.NavInputSource));
        Text("NavLastValidSelectionUserData = %" IM_PRId64 " (0x%" IM_PRIX64 ")", g.NavLastValidSelectionUserData, g.NavLastValidSelectionUserData);
        Text("NavActive: %d, NavVisible: %d", g.IO.NavActive, g.IO.NavVisible);
        Text("NavActivateId/DownId/PressedId: %08X/%08X/%08X", g.NavActivateId, g.NavActivateDownId, g.NavActivatePressedId);
        Text("NavActivateFlags: %04X", g.NavActivateFlags);
        Text("NavCursorVisible: %d, NavHighlightItemUnderNav: %d", g.NavCursorVisible, g.NavHighlightItemUnderNav);
        Text("NavFocusScopeId = 0x%08X", g.NavFocusScopeId);
        Text("NavFocusRoute[] = ");
        for (int path_n = g.NavFocusRoute.Size - 1; path_n >= 0; path_n--)
        {
            const ImGuiFocusScopeData& focus_scope = g.NavFocusRoute[path_n];
            SameLine(0.0f, 0.0f);
            Text("0x%08X/", focus_scope.ID);
            SetItemTooltip("In window \"%s\"", FindWindowByID(focus_scope.WindowID)->Name);
        }
        Text("NavWindowingTarget: '%s'", g.NavWindowingTarget ? g.NavWindowingTarget->Name : "NULL");
        Unindent();

        TreePop();
    }

    if (cfg->ShowWindowsRects || cfg->ShowWindowsBeginOrder)
    {
        for (ImGuiWindow* window : g.Windows)
        {
            if (!window->WasActive)
                continue;
            ImDrawList* draw_list = GetForegroundDrawList(window);
            if (cfg->ShowWindowsRects)
            {
                ImRect r = Funcs::GetWindowRect(window, cfg->ShowWindowsRectsType);
                draw_list->AddRect(r.Min, r.Max, IM_COL32(255, 0, 128, 255));
            }
            if (cfg->ShowWindowsBeginOrder && !(window->Flags & ImGuiWindowFlags_ChildWindow))
            {
                char buf[32];
                ImFormatString(buf, IM_COUNTOF(buf), "%d", window->BeginOrderWithinContext);
                float font_size = GetFontSize();
                draw_list->AddRectFilled(window->Pos, window->Pos + ImVec2(font_size, font_size), IM_COL32(200, 100, 100, 255));
                draw_list->AddText(window->Pos, IM_COL32(255, 255, 255, 255), buf);
            }
        }
    }

    if (cfg->ShowTablesRects)
    {
        for (int table_n = 0; table_n < g.Tables.GetMapSize(); table_n++)
        {
            ImGuiTable* table = g.Tables.TryGetMapData(table_n);
            if (table == NULL || table->LastFrameActive < g.FrameCount - 1)
                continue;
            ImDrawList* draw_list = GetForegroundDrawList(table->OuterWindow);
            if (cfg->ShowTablesRectsType >= TRT_ColumnsRect)
            {
                for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                {
                    ImRect r = Funcs::GetTableRect(table, cfg->ShowTablesRectsType, column_n);
                    ImU32 col = (table->HoveredColumnBody == column_n) ? IM_COL32(255, 255, 128, 255) : IM_COL32(255, 0, 128, 255);
                    float thickness = (table->HoveredColumnBody == column_n) ? 3.0f : 1.0f;
                    draw_list->AddRect(r.Min, r.Max, col, 0.0f, 0, thickness);
                }
            }
            else
            {
                ImRect r = Funcs::GetTableRect(table, cfg->ShowTablesRectsType, -1);
                draw_list->AddRect(r.Min, r.Max, IM_COL32(255, 0, 128, 255));
            }
        }
    }

#ifdef IMGUI_HAS_DOCK

    if (show_docking_nodes && g.IO.KeyCtrl)
    {
    }
#endif

    End();
}

void ImGui::DebugBreakClearData()
{

    ImGuiContext& g = *GImGui;
    g.DebugBreakInWindow = 0;
    g.DebugBreakInTable = 0;
    g.DebugBreakInShortcutRouting = ImGuiKey_None;
}

void ImGui::DebugBreakButtonTooltip(bool keyboard_only, const char* description_of_location)
{
    if (!BeginItemTooltip())
        return;
    Text("To call IM_DEBUG_BREAK() %s:", description_of_location);
    Separator();
    TextUnformatted(keyboard_only ? "- Press 'Pause/Break' on keyboard." : "- Press 'Pause/Break' on keyboard.\n- or Click (may alter focus/active id).\n- or navigate using keyboard and press space.");
    Separator();
    TextUnformatted("Choose one way that doesn't interfere with what you are trying to debug!\nYou need a debugger attached or this will crash!");
    EndTooltip();
}

bool ImGui::DebugBreakButton(const char* label, const char* description_of_location)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 pos = window->DC.CursorPos + ImVec2(0.0f, window->DC.CurrLineTextBaseOffset);
    ImVec2 size = ImVec2(label_size.x + g.Style.FramePadding.x * 2.0f, label_size.y);

    const ImRect bb(pos, pos + size);
    ItemSize(size, 0.0f);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered = ItemHoverable(bb, id, g.CurrentItemFlags);
    bool pressed = hovered && (IsKeyChordPressed(g.DebugBreakKeyChord) || IsMouseClicked(0) || g.NavActivateId == id);
    DebugBreakButtonTooltip(false, description_of_location);

    ImVec4 col4f = GetStyleColorVec4(hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImVec4 hsv;
    ColorConvertRGBtoHSV(col4f.x, col4f.y, col4f.z, hsv.x, hsv.y, hsv.z);
    ColorConvertHSVtoRGB(hsv.x + 0.20f, hsv.y, hsv.z, col4f.x, col4f.y, col4f.z);

    RenderNavCursor(bb, id);
    RenderFrame(bb.Min, bb.Max, GetColorU32(col4f), true, g.Style.FrameRounding);
    RenderTextClipped(bb.Min, bb.Max, label, NULL, &label_size, g.Style.ButtonTextAlign, &bb);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

void ImGui::DebugNodeColumns(ImGuiOldColumns* columns)
{
    if (!TreeNode((void*)(uintptr_t)columns->ID, "Columns Id: 0x%08X, Count: %d, Flags: 0x%04X", columns->ID, columns->Count, columns->Flags))
        return;
    BulletText("Width: %.1f (MinX: %.1f, MaxX: %.1f)", columns->OffMaxX - columns->OffMinX, columns->OffMinX, columns->OffMaxX);
    for (ImGuiOldColumnData& column : columns->Columns)
        BulletText("Column %02d: OffsetNorm %.3f (= %.1f px)", (int)columns->Columns.index_from_ptr(&column), column.OffsetNorm, GetColumnOffsetFromNorm(columns, column.OffsetNorm));
    TreePop();
}

void ImGui::DebugNodeDrawList(ImGuiWindow* window, ImGuiViewportP* viewport, const ImDrawList* draw_list, const char* label)
{
    ImGuiContext& g = *GImGui;
    IM_UNUSED(viewport);
    ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
    int cmd_count = draw_list->CmdBuffer.Size;
    if (cmd_count > 0 && draw_list->CmdBuffer.back().ElemCount == 0 && draw_list->CmdBuffer.back().UserCallback == NULL)
        cmd_count--;
    bool node_open = TreeNode(draw_list, "%s: '%s' %d vtx, %d indices, %d cmds", label, draw_list->_OwnerName ? draw_list->_OwnerName : "", draw_list->VtxBuffer.Size, draw_list->IdxBuffer.Size, cmd_count);
    if (draw_list == GetWindowDrawList())
    {
        SameLine();
        TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "CURRENTLY APPENDING");
        if (node_open)
            TreePop();
        return;
    }

    ImDrawList* fg_draw_list = GetForegroundDrawList(window);
    if (window && IsItemHovered() && fg_draw_list)
        fg_draw_list->AddRect(window->Pos, window->Pos + window->Size, IM_COL32(255, 255, 0, 255));
    if (!node_open)
        return;

    if (window && !window->WasActive)
        TextDisabled("Warning: owning Window is inactive. This DrawList is not being rendered!");

    for (const ImDrawCmd* pcmd = draw_list->CmdBuffer.Data; pcmd < draw_list->CmdBuffer.Data + cmd_count; pcmd++)
    {
        if (pcmd->UserCallback)
        {
            BulletText("Callback %p, user_data %p", pcmd->UserCallback, pcmd->UserCallbackData);
            continue;
        }

        char texid_desc[30];
        FormatTextureRefForDebugDisplay(texid_desc, IM_COUNTOF(texid_desc), pcmd->TexRef);
        char buf[300];
        ImFormatString(buf, IM_COUNTOF(buf), "DrawCmd:%5d tris, Tex %s, ClipRect (%4.0f,%4.0f)-(%4.0f,%4.0f)",
            pcmd->ElemCount / 3, texid_desc, pcmd->ClipRect.x, pcmd->ClipRect.y, pcmd->ClipRect.z, pcmd->ClipRect.w);
        bool pcmd_node_open = TreeNode((void*)(pcmd - draw_list->CmdBuffer.begin()), "%s", buf);
        if (IsItemHovered() && (cfg->ShowDrawCmdMesh || cfg->ShowDrawCmdBoundingBoxes) && fg_draw_list)
            DebugNodeDrawCmdShowMeshAndBoundingBox(fg_draw_list, draw_list, pcmd, cfg->ShowDrawCmdMesh, cfg->ShowDrawCmdBoundingBoxes);
        if (!pcmd_node_open)
            continue;

        const ImDrawIdx* idx_buffer = (draw_list->IdxBuffer.Size > 0) ? draw_list->IdxBuffer.Data : NULL;
        const ImDrawVert* vtx_buffer = draw_list->VtxBuffer.Data + pcmd->VtxOffset;
        float total_area = 0.0f;
        for (unsigned int idx_n = pcmd->IdxOffset; idx_n < pcmd->IdxOffset + pcmd->ElemCount; )
        {
            ImVec2 triangle[3];
            for (int n = 0; n < 3; n++, idx_n++)
                triangle[n] = vtx_buffer[idx_buffer ? idx_buffer[idx_n] : idx_n].pos;
            total_area += ImTriangleArea(triangle[0], triangle[1], triangle[2]);
        }

        ImFormatString(buf, IM_COUNTOF(buf), "Mesh: ElemCount: %d, VtxOffset: +%d, IdxOffset: +%d, Area: ~%0.f px", pcmd->ElemCount, pcmd->VtxOffset, pcmd->IdxOffset, total_area);
        Selectable(buf);
        if (IsItemHovered() && fg_draw_list)
            DebugNodeDrawCmdShowMeshAndBoundingBox(fg_draw_list, draw_list, pcmd, true, false);

        ImGuiListClipper clipper;
        clipper.Begin(pcmd->ElemCount / 3);
        while (clipper.Step())
            for (int prim = clipper.DisplayStart, idx_i = pcmd->IdxOffset + clipper.DisplayStart * 3; prim < clipper.DisplayEnd; prim++)
            {
                char* buf_p = buf, * buf_end = buf + IM_COUNTOF(buf);
                ImVec2 triangle[3];
                for (int n = 0; n < 3; n++, idx_i++)
                {
                    const ImDrawVert& v = vtx_buffer[idx_buffer ? idx_buffer[idx_i] : idx_i];
                    triangle[n] = v.pos;
                    buf_p += ImFormatString(buf_p, buf_end - buf_p, "%s %04d: pos (%8.2f,%8.2f), uv (%.6f,%.6f), col %08X\n",
                        (n == 0) ? "Vert:" : "     ", idx_i, v.pos.x, v.pos.y, v.uv.x, v.uv.y, v.col);
                }

                Selectable(buf, false);
                if (fg_draw_list && IsItemHovered())
                {
                    ImDrawListFlags backup_flags = fg_draw_list->Flags;
                    fg_draw_list->Flags &= ~ImDrawListFlags_AntiAliasedLines;
                    fg_draw_list->AddPolyline(triangle, 3, IM_COL32(255, 255, 0, 255), ImDrawFlags_Closed, 1.0f);
                    fg_draw_list->Flags = backup_flags;
                }
            }
        TreePop();
    }
    TreePop();
}

void ImGui::DebugNodeDrawCmdShowMeshAndBoundingBox(ImDrawList* out_draw_list, const ImDrawList* draw_list, const ImDrawCmd* draw_cmd, bool show_mesh, bool show_aabb)
{
    IM_ASSERT(show_mesh || show_aabb);

    ImRect clip_rect = draw_cmd->ClipRect;
    ImRect vtxs_rect(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    ImDrawListFlags backup_flags = out_draw_list->Flags;
    out_draw_list->Flags &= ~ImDrawListFlags_AntiAliasedLines;
    for (unsigned int idx_n = draw_cmd->IdxOffset, idx_end = draw_cmd->IdxOffset + draw_cmd->ElemCount; idx_n < idx_end; )
    {
        ImDrawIdx* idx_buffer = (draw_list->IdxBuffer.Size > 0) ? draw_list->IdxBuffer.Data : NULL;
        ImDrawVert* vtx_buffer = draw_list->VtxBuffer.Data + draw_cmd->VtxOffset;

        ImVec2 triangle[3];
        for (int n = 0; n < 3; n++, idx_n++)
            vtxs_rect.Add((triangle[n] = vtx_buffer[idx_buffer ? idx_buffer[idx_n] : idx_n].pos));
        if (show_mesh)
            out_draw_list->AddPolyline(triangle, 3, IM_COL32(255, 255, 0, 255), ImDrawFlags_Closed, 1.0f);
    }

    if (show_aabb)
    {
        out_draw_list->AddRect(ImTrunc(clip_rect.Min), ImTrunc(clip_rect.Max), IM_COL32(255, 0, 255, 255));
        out_draw_list->AddRect(ImTrunc(vtxs_rect.Min), ImTrunc(vtxs_rect.Max), IM_COL32(0, 255, 255, 255));
    }
    out_draw_list->Flags = backup_flags;
}

static int CalcFontGlyphSrcOverlapMask(ImFontAtlas* atlas, ImFont* font, unsigned int codepoint)
{
    int mask = 0, count = 0;
    for (int src_n = 0; src_n < font->Sources.Size; src_n++)
    {
        ImFontConfig* src = font->Sources[src_n];
        if (!(src->FontLoader ? src->FontLoader : atlas->FontLoader)->FontSrcContainsGlyph(atlas, src, (ImWchar)codepoint))
            continue;
        mask |= (1 << src_n);
        count++;
    }
    return count > 1 ? mask : 0;
}

void ImGui::DebugNodeFont(ImFont* font)
{
    ImGuiContext& g = *GImGui;
    ImGuiMetricsConfig* cfg = &g.DebugMetricsConfig;
    ImFontAtlas* atlas = font->OwnerAtlas;
    bool opened = TreeNode(font, "Font: \"%s\": %d sources(s)", font->GetDebugName(), font->Sources.Size);

    if (!opened)
        Indent();
    Indent();
    if (cfg->ShowFontPreview)
    {
        PushFont(font, 0.0f);
        Text("The quick brown fox jumps over the lazy dog");
        PopFont();
    }
    if (!opened)
    {
        Unindent();
        Unindent();
        return;
    }
    if (SmallButton("Set as default"))
        GetIO().FontDefault = font;
    SameLine();
    BeginDisabled(atlas->Fonts.Size <= 1 || atlas->Locked);
    if (SmallButton("Remove"))
        atlas->RemoveFont(font);
    EndDisabled();
    SameLine();
    if (SmallButton("Clear bakes"))
        ImFontAtlasFontDiscardBakes(atlas, font, 0);
    SameLine();
    if (SmallButton("Clear unused"))
        ImFontAtlasFontDiscardBakes(atlas, font, 2);

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    SetNextItemWidth(GetFontSize() * 8);
    DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");

#endif

    char c_str[5];
    ImTextCharToUtf8(c_str, font->FallbackChar);
    Text("Fallback character: '%s' (U+%04X)", c_str, font->FallbackChar);
    ImTextCharToUtf8(c_str, font->EllipsisChar);
    Text("Ellipsis character: '%s' (U+%04X)", c_str, font->EllipsisChar);

    for (int src_n = 0; src_n < font->Sources.Size; src_n++)
    {
        ImFontConfig* src = font->Sources[src_n];
        if (TreeNode(src, "Input %d: \'%s\' [%d], Oversample: %d,%d, PixelSnapH: %d, Offset: (%.1f,%.1f)",
            src_n, src->Name, src->FontNo, src->OversampleH, src->OversampleV, src->PixelSnapH, src->GlyphOffset.x, src->GlyphOffset.y))
        {
            const ImFontLoader* loader = src->FontLoader ? src->FontLoader : atlas->FontLoader;
            Text("Loader: '%s'", loader->Name ? loader->Name : "N/A");

#ifdef IMGUI_ENABLE_FREETYPE
            if (loader->Name != NULL && strcmp(loader->Name, "FreeType") == 0)
            {
                unsigned int loader_flags = src->FontLoaderFlags;
                Text("FreeType Loader Flags: 0x%08X", loader_flags);
                if (ImGuiFreeType::DebugEditFontLoaderFlags(&loader_flags))
                {
                    ImFontAtlasFontDestroyOutput(atlas, font);
                    src->FontLoaderFlags = loader_flags;
                    ImFontAtlasFontInitOutput(atlas, font);
                }
            }
#endif
            TreePop();
        }
    }
    if (font->Sources.Size > 1 && TreeNode("Input Glyphs Overlap Detection Tool"))
    {
        TextWrapped("- First Input that contains the glyph is used.\n"
            "- Use ImFontConfig::GlyphExcludeRanges[] to specify ranges to ignore glyph in given Input.\n- Prefer using a small number of ranges as the list is scanned every time a new glyph is loaded,\n  - e.g. GlyphExcludeRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };\n- This tool doesn't cache results and is slow, don't keep it open!");
        if (BeginTable("table", 2))
        {
            for (unsigned int c = 0; c < 0x10000; c++)
                if (int overlap_mask = CalcFontGlyphSrcOverlapMask(atlas, font, c))
                {
                    unsigned int c_end = c + 1;
                    while (c_end < 0x10000 && CalcFontGlyphSrcOverlapMask(atlas, font, c_end) == overlap_mask)
                        c_end++;
                    if (TableNextColumn() && TreeNode((void*)(intptr_t)c, "U+%04X-U+%04X: %d codepoints in %d inputs", c, c_end - 1, c_end - c, ImCountSetBits(overlap_mask)))
                    {
                        char utf8_buf[5];
                        for (unsigned int n = c; n < c_end; n++)
                        {
                            ImTextCharToUtf8(utf8_buf, n);
                            BulletText("Codepoint U+%04X (%s)", n, utf8_buf);
                        }
                        TreePop();
                    }
                    TableNextColumn();
                    for (int src_n = 0; src_n < font->Sources.Size; src_n++)
                        if (overlap_mask & (1 << src_n))
                        {
                            Text("%d ", src_n);
                            SameLine();
                        }
                    c = c_end - 1;
                }
            EndTable();
        }
        TreePop();
    }

    for (int baked_n = 0; baked_n < atlas->Builder->BakedPool.Size; baked_n++)
    {
        ImFontBaked* baked = &atlas->Builder->BakedPool[baked_n];
        if (baked->OwnerFont != font)
            continue;
        PushID(baked->BakedId);
        if (TreeNode("Glyphs", "Baked at { %.2fpx, d.%.2f }: %d glyphs%s", baked->Size, baked->RasterizerDensity, baked->Glyphs.Size, (baked->LastUsedFrame < atlas->Builder->FrameCount - 1) ? " *Unused*" : ""))
        {
            if (SmallButton("Load all"))
                for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base++)
                    baked->FindGlyph((ImWchar)base);

            const int surface_sqrt = (int)ImSqrt((float)baked->MetricsTotalSurface);
            Text("Ascent: %f, Descent: %f, Ascent-Descent: %f", baked->Ascent, baked->Descent, baked->Ascent - baked->Descent);
            Text("Texture Area: about %d px ~%dx%d px", baked->MetricsTotalSurface, surface_sqrt, surface_sqrt);
            for (int src_n = 0; src_n < font->Sources.Size; src_n++)
            {
                ImFontConfig* src = font->Sources[src_n];
                int oversample_h, oversample_v;
                ImFontAtlasBuildGetOversampleFactors(src, baked, &oversample_h, &oversample_v);
                BulletText("Input %d: \'%s\', Oversample: (%d=>%d,%d=>%d), PixelSnapH: %d, Offset: (%.1f,%.1f)",
                    src_n, src->Name, src->OversampleH, oversample_h, src->OversampleV, oversample_v, src->PixelSnapH, src->GlyphOffset.x, src->GlyphOffset.y);
            }

            DebugNodeFontGlyphsForSrcMask(font, baked, ~0);
            TreePop();
        }
        PopID();
    }
    TreePop();
    Unindent();
}

void ImGui::DebugNodeFontGlyphsForSrcMask(ImFont* font, ImFontBaked* baked, int src_mask)
{
    ImDrawList* draw_list = GetWindowDrawList();
    const ImU32 glyph_col = GetColorU32(ImGuiCol_Text);
    const float cell_size = baked->Size * 1;
    const float cell_spacing = GetStyle().ItemSpacing.y;
    for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
    {

        if (!(base & 8191) && font->IsGlyphRangeUnused(base, base + 8191))
        {
            base += 8192 - 256;
            continue;
        }

        int count = 0;
        for (unsigned int n = 0; n < 256; n++)
            if (const ImFontGlyph* glyph = baked->IsGlyphLoaded((ImWchar)(base + n)) ? baked->FindGlyph((ImWchar)(base + n)) : NULL)
                if (src_mask & (1 << glyph->SourceIdx))
                    count++;
        if (count <= 0)
            continue;
        if (!TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base + 255, count, count > 1 ? "glyphs" : "glyph"))
            continue;

        ImVec2 base_pos = GetCursorScreenPos();
        for (unsigned int n = 0; n < 256; n++)
        {

            ImVec2 cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing), base_pos.y + (n / 16) * (cell_size + cell_spacing));
            ImVec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
            const ImFontGlyph* glyph = baked->IsGlyphLoaded((ImWchar)(base + n)) ? baked->FindGlyph((ImWchar)(base + n)) : NULL;
            draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 50));
            if (!glyph || (src_mask & (1 << glyph->SourceIdx)) == 0)
                continue;
            font->RenderChar(draw_list, cell_size, cell_p1, glyph_col, (ImWchar)(base + n));
            if (IsMouseHoveringRect(cell_p1, cell_p2) && BeginTooltip())
            {
                DebugNodeFontGlyph(font, glyph);
                EndTooltip();
            }
        }
        Dummy(ImVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
        TreePop();
    }
}

void ImGui::DebugNodeFontGlyph(ImFont* font, const ImFontGlyph* glyph)
{
    Text("Codepoint: U+%04X", glyph->Codepoint);
    Separator();
    Text("Visible: %d", glyph->Visible);
    Text("AdvanceX: %.1f", glyph->AdvanceX);
    Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
    Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
    if (glyph->PackId >= 0)
    {
        ImTextureRect* r = ImFontAtlasPackGetRect(font->OwnerAtlas, glyph->PackId);
        Text("PackId: 0x%X (%dx%d rect at %d,%d)", glyph->PackId, r->w, r->h, r->x, r->y);
    }
    Text("SourceIdx: %d", glyph->SourceIdx);
}

void ImGui::DebugNodeStorage(ImGuiStorage* storage, const char* label)
{
    if (!TreeNode(label, "%s: %d entries, %d bytes", label, storage->Data.Size, storage->Data.size_in_bytes()))
        return;
    for (const ImGuiStoragePair& p : storage->Data)
    {
        BulletText("Key 0x%08X Value { i: %d }", p.key, p.val_i);
        DebugLocateItemOnHover(p.key);
    }
    TreePop();
}

void ImGui::DebugNodeTabBar(ImGuiTabBar* tab_bar, const char* label)
{

    char buf[256];
    char* p = buf;
    const char* buf_end = buf + IM_COUNTOF(buf);
    const bool is_active = (tab_bar->PrevFrameVisible >= GetFrameCount() - 2);
    p += ImFormatString(p, buf_end - p, "%s 0x%08X (%d tabs)%s  {", label, tab_bar->ID, tab_bar->Tabs.Size, is_active ? "" : " *Inactive*");
    for (int tab_n = 0; tab_n < ImMin(tab_bar->Tabs.Size, 3); tab_n++)
    {
        ImGuiTabItem* tab = &tab_bar->Tabs[tab_n];
        p += ImFormatString(p, buf_end - p, "%s'%s'", tab_n > 0 ? ", " : "", TabBarGetTabName(tab_bar, tab));
    }
    p += ImFormatString(p, buf_end - p, (tab_bar->Tabs.Size > 3) ? " ... }" : " } ");
    if (!is_active) { PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled)); }
    bool open = TreeNode(label, "%s", buf);
    if (!is_active) { PopStyleColor(); }
    if (is_active && IsItemHovered())
    {
        ImDrawList* draw_list = GetForegroundDrawList(tab_bar->Window);
        draw_list->AddRect(tab_bar->BarRect.Min, tab_bar->BarRect.Max, IM_COL32(255, 255, 0, 255));
        draw_list->AddLine(ImVec2(tab_bar->ScrollingRectMinX, tab_bar->BarRect.Min.y), ImVec2(tab_bar->ScrollingRectMinX, tab_bar->BarRect.Max.y), IM_COL32(0, 255, 0, 255));
        draw_list->AddLine(ImVec2(tab_bar->ScrollingRectMaxX, tab_bar->BarRect.Min.y), ImVec2(tab_bar->ScrollingRectMaxX, tab_bar->BarRect.Max.y), IM_COL32(0, 255, 0, 255));
    }
    if (open)
    {
        for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
        {
            ImGuiTabItem* tab = &tab_bar->Tabs[tab_n];
            PushID(tab);
            if (SmallButton("<")) { TabBarQueueReorder(tab_bar, tab, -1); } SameLine(0, 2);
            if (SmallButton(">")) { TabBarQueueReorder(tab_bar, tab, +1); } SameLine();
            Text("%02d%c Tab 0x%08X '%s' Offset: %.2f, Width: %.2f/%.2f",
                tab_n, (tab->ID == tab_bar->SelectedTabId) ? '*' : ' ', tab->ID, TabBarGetTabName(tab_bar, tab), tab->Offset, tab->Width, tab->ContentWidth);
            PopID();
        }
        TreePop();
    }
}

void ImGui::DebugNodeViewport(ImGuiViewportP* viewport)
{
    ImGuiContext& g = *GImGui;
    SetNextItemOpen(true, ImGuiCond_Once);
    bool open = TreeNode("viewport0", "Viewport #%d", 0);
    if (IsItemHovered())
        g.DebugMetricsConfig.HighlightViewportID = viewport->ID;
    if (open)
    {
        ImGuiWindowFlags flags = viewport->Flags;
        BulletText("Main Pos: (%.0f,%.0f), Size: (%.0f,%.0f)\nWorkArea Inset Left: %.0f Top: %.0f, Right: %.0f, Bottom: %.0f",
            viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y,
            viewport->WorkInsetMin.x, viewport->WorkInsetMin.y, viewport->WorkInsetMax.x, viewport->WorkInsetMax.y);
        BulletText("Flags: 0x%04X =%s%s%s", viewport->Flags,
            (flags & ImGuiViewportFlags_IsPlatformWindow)  ? " IsPlatformWindow"  : "",
            (flags & ImGuiViewportFlags_IsPlatformMonitor) ? " IsPlatformMonitor" : "",
            (flags & ImGuiViewportFlags_OwnedByApp)        ? " OwnedByApp"        : "");
        for (ImDrawList* draw_list : viewport->DrawDataP.CmdLists)
            DebugNodeDrawList(NULL, viewport, draw_list, "DrawList");
        TreePop();
    }
}

void ImGui::DebugNodeWindow(ImGuiWindow* window, const char* label)
{
    if (window == NULL)
    {
        BulletText("%s: NULL", label);
        return;
    }

    ImGuiContext& g = *GImGui;
    const bool is_active = window->WasActive;
    ImGuiTreeNodeFlags tree_node_flags = (window == g.NavWindow) ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
    if (!is_active) { PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled)); }
    const bool open = TreeNodeEx(label, tree_node_flags, "%s '%s'%s", label, window->Name, is_active ? "" : " *Inactive*");
    if (!is_active) { PopStyleColor(); }
    if (IsItemHovered() && is_active)
        GetForegroundDrawList(window)->AddRect(window->Pos, window->Pos + window->Size, IM_COL32(255, 255, 0, 255));
    if (!open)
        return;

    if (window->MemoryCompacted)
        TextDisabled("Note: some memory buffers have been compacted/freed.");

    if (g.IO.ConfigDebugIsDebuggerPresent && DebugBreakButton("**DebugBreak**", "in Begin()"))
        g.DebugBreakInWindow = window->ID;

    ImGuiWindowFlags flags = window->Flags;
    DebugNodeDrawList(window, window->Viewport, window->DrawList, "DrawList");
    BulletText("Pos: (%.1f,%.1f), Size: (%.1f,%.1f), ContentSize (%.1f,%.1f) Ideal (%.1f,%.1f)", window->Pos.x, window->Pos.y, window->Size.x, window->Size.y, window->ContentSize.x, window->ContentSize.y, window->ContentSizeIdeal.x, window->ContentSizeIdeal.y);
    BulletText("Flags: 0x%08X (%s%s%s%s%s%s%s%s%s..)", flags,
        (flags & ImGuiWindowFlags_ChildWindow)  ? "Child " : "",      (flags & ImGuiWindowFlags_Tooltip)     ? "Tooltip "   : "",  (flags & ImGuiWindowFlags_Popup) ? "Popup " : "",
        (flags & ImGuiWindowFlags_Modal)        ? "Modal " : "",      (flags & ImGuiWindowFlags_ChildMenu)   ? "ChildMenu " : "",  (flags & ImGuiWindowFlags_NoSavedSettings) ? "NoSavedSettings " : "",
        (flags & ImGuiWindowFlags_NoMouseInputs)? "NoMouseInputs":"", (flags & ImGuiWindowFlags_NoNavInputs) ? "NoNavInputs" : "", (flags & ImGuiWindowFlags_AlwaysAutoResize) ? "AlwaysAutoResize" : "");
    if (flags & ImGuiWindowFlags_ChildWindow)
        BulletText("ChildFlags: 0x%08X (%s%s%s%s..)", window->ChildFlags,
            (window->ChildFlags & ImGuiChildFlags_Borders) ? "Borders " : "",
            (window->ChildFlags & ImGuiChildFlags_ResizeX) ? "ResizeX " : "",
            (window->ChildFlags & ImGuiChildFlags_ResizeY) ? "ResizeY " : "",
            (window->ChildFlags & ImGuiChildFlags_NavFlattened) ? "NavFlattened " : "");
    BulletText("Scroll: (%.2f/%.2f,%.2f/%.2f) Scrollbar:%s%s", window->Scroll.x, window->ScrollMax.x, window->Scroll.y, window->ScrollMax.y, window->ScrollbarX ? "X" : "", window->ScrollbarY ? "Y" : "");
    BulletText("Active: %d/%d, WriteAccessed: %d, BeginOrderWithinContext: %d", window->Active, window->WasActive, window->WriteAccessed, (window->Active || window->WasActive) ? window->BeginOrderWithinContext : -1);
    BulletText("Appearing: %d, Hidden: %d (CanSkip %d Cannot %d), SkipItems: %d", window->Appearing, window->Hidden, window->HiddenFramesCanSkipItems, window->HiddenFramesCannotSkipItems, window->SkipItems);
    for (int layer = 0; layer < ImGuiNavLayer_COUNT; layer++)
    {
        ImRect r = window->NavRectRel[layer];
        if (r.Min.x >= r.Max.x && r.Min.y >= r.Max.y)
            BulletText("NavLastIds[%d]: 0x%08X", layer, window->NavLastIds[layer]);
        else
            BulletText("NavLastIds[%d]: 0x%08X at +(%.1f,%.1f)(%.1f,%.1f)", layer, window->NavLastIds[layer], r.Min.x, r.Min.y, r.Max.x, r.Max.y);
        DebugLocateItemOnHover(window->NavLastIds[layer]);
    }
    const ImVec2* pr = window->NavPreferredScoringPosRel;
    for (int layer = 0; layer < ImGuiNavLayer_COUNT; layer++)
        BulletText("NavPreferredScoringPosRel[%d] = (%.1f,%.1f)", layer, (pr[layer].x == FLT_MAX ? -99999.0f : pr[layer].x), (pr[layer].y == FLT_MAX ? -99999.0f : pr[layer].y));
    BulletText("NavLayersActiveMask: %X, NavLastChildNavWindow: %s", window->DC.NavLayersActiveMask, window->NavLastChildNavWindow ? window->NavLastChildNavWindow->Name : "NULL");
    if (window->RootWindow != window)               { DebugNodeWindow(window->RootWindow, "RootWindow"); }
    if (window->ParentWindow != NULL)               { DebugNodeWindow(window->ParentWindow, "ParentWindow"); }
    if (window->ParentWindowForFocusRoute != NULL)  { DebugNodeWindow(window->ParentWindowForFocusRoute, "ParentWindowForFocusRoute"); }
    if (window->DC.ChildWindows.Size > 0)           { DebugNodeWindowsList(&window->DC.ChildWindows, "ChildWindows"); }
    if (window->ColumnsStorage.Size > 0 && TreeNode("Columns", "Columns sets (%d)", window->ColumnsStorage.Size))
    {
        for (ImGuiOldColumns& columns : window->ColumnsStorage)
            DebugNodeColumns(&columns);
        TreePop();
    }
    DebugNodeStorage(&window->StateStorage, "Storage");
    TreePop();
}

void ImGui::DebugNodeWindowSettings(ImGuiWindowSettings* settings)
{
    if (settings->WantDelete)
        BeginDisabled();
    Text("0x%08X \"%s\" Pos (%d,%d) Size (%d,%d) Collapsed=%d",
        settings->ID, settings->GetName(), settings->Pos.x, settings->Pos.y, settings->Size.x, settings->Size.y, settings->Collapsed);
    if (settings->WantDelete)
        EndDisabled();
}

void ImGui::DebugNodeWindowsList(ImVector<ImGuiWindow*>* windows, const char* label)
{
    if (!TreeNode(label, "%s (%d)", label, windows->Size))
        return;
    for (int i = windows->Size - 1; i >= 0; i--)
    {
        PushID((*windows)[i]);
        DebugNodeWindow((*windows)[i], "Window");
        PopID();
    }
    TreePop();
}

void ImGui::DebugNodeWindowsListByBeginStackParent(ImGuiWindow** windows, int windows_size, ImGuiWindow* parent_in_begin_stack)
{
    for (int i = 0; i < windows_size; i++)
    {
        ImGuiWindow* window = windows[i];
        if (window->ParentWindowInBeginStack != parent_in_begin_stack)
            continue;
        char buf[20];
        ImFormatString(buf, IM_COUNTOF(buf), "[%04d] Window", window->BeginOrderWithinContext);

        DebugNodeWindow(window, buf);
        TreePush(buf);
        DebugNodeWindowsListByBeginStackParent(windows + i + 1, windows_size - i - 1, window);
        TreePop();
    }
}

void ImGui::DebugLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    DebugLogV(fmt, args);
    va_end(args);
}

void ImGui::DebugLogV(const char* fmt, va_list args)
{
    ImGuiContext& g = *GImGui;
    const int old_size = g.DebugLogBuf.size();
    if (g.ContextName[0] != 0)
        g.DebugLogBuf.appendf("[%s] [%05d] ", g.ContextName, g.FrameCount);
    else
        g.DebugLogBuf.appendf("[%05d] ", g.FrameCount);
    g.DebugLogBuf.appendfv(fmt, args);
    g.DebugLogIndex.append(g.DebugLogBuf.c_str(), old_size, g.DebugLogBuf.size());

    const char* str = g.DebugLogBuf.begin() + old_size;
    if (g.DebugLogFlags & ImGuiDebugLogFlags_OutputToTTY)
        IMGUI_DEBUG_PRINTF("%s", str);
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
    if (g.DebugLogFlags & ImGuiDebugLogFlags_OutputToDebugger)
    {
        ::OutputDebugStringA("[imgui] ");
        ::OutputDebugStringA(str);
    }
#endif
#ifdef IMGUI_ENABLE_TEST_ENGINE

    const int new_size = g.DebugLogBuf.size();
    const bool trailing_carriage_return = (g.DebugLogBuf[new_size - 1] == '\n');
    if (g.DebugLogFlags & ImGuiDebugLogFlags_OutputToTestEngine)
        IMGUI_TEST_ENGINE_LOG("%.*s", new_size - old_size - (trailing_carriage_return ? 1 : 0), str);
#endif
}

static void SameLineOrWrap(const ImVec2& size)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImVec2 pos(window->DC.CursorPosPrevLine.x + g.Style.ItemSpacing.x, window->DC.CursorPosPrevLine.y);
    if (window->WorkRect.Contains(ImRect(pos, pos + size)))
        ImGui::SameLine();
}

static void ShowDebugLogFlag(const char* name, ImGuiDebugLogFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImVec2 size(ImGui::GetFrameHeight() + g.Style.ItemInnerSpacing.x + ImGui::CalcTextSize(name).x, ImGui::GetFrameHeight());
    SameLineOrWrap(size);

    bool highlight_errors = (flags == ImGuiDebugLogFlags_EventError && g.DebugLogSkippedErrors > 0);
    if (highlight_errors)
        ImGui::PushStyleColor(ImGuiCol_Text, ImLerp(g.Style.Colors[ImGuiCol_Text], ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 0.30f));
    if (ImGui::CheckboxFlags(name, &g.DebugLogFlags, flags) && g.IO.KeyShift && (g.DebugLogFlags & flags) != 0)
    {
        g.DebugLogAutoDisableFrames = 2;
        g.DebugLogAutoDisableFlags |= flags;
    }
    if (highlight_errors)
    {
        ImGui::PopStyleColor();
        ImGui::SetItemTooltip("%d past errors skipped.", g.DebugLogSkippedErrors);
    }
    else
    {
        ImGui::SetItemTooltip("Hold Shift when clicking to enable for 2 frames only (useful for spammy log entries)");
    }
}

void ImGui::ShowDebugLogWindow(bool* p_open)
{
    ImGuiContext& g = *GImGui;
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) == 0)
        SetNextWindowSize(ImVec2(0.0f, GetFontSize() * 12.0f), ImGuiCond_FirstUseEver);
    if (!Begin("Dear ImGui Debug Log", p_open) || GetCurrentWindow()->BeginCount > 1)
    {
        End();
        return;
    }

    ImGuiDebugLogFlags all_enable_flags = ImGuiDebugLogFlags_EventMask_ & ~ImGuiDebugLogFlags_EventInputRouting;
    CheckboxFlags("All", &g.DebugLogFlags, all_enable_flags);
    SetItemTooltip("(except InputRouting which is spammy)");

    ShowDebugLogFlag("Errors", ImGuiDebugLogFlags_EventError);
    ShowDebugLogFlag("ActiveId", ImGuiDebugLogFlags_EventActiveId);
    ShowDebugLogFlag("Clipper", ImGuiDebugLogFlags_EventClipper);
    ShowDebugLogFlag("Focus", ImGuiDebugLogFlags_EventFocus);
    ShowDebugLogFlag("IO", ImGuiDebugLogFlags_EventIO);
    ShowDebugLogFlag("Font", ImGuiDebugLogFlags_EventFont);
    ShowDebugLogFlag("Nav", ImGuiDebugLogFlags_EventNav);
    ShowDebugLogFlag("Popup", ImGuiDebugLogFlags_EventPopup);
    ShowDebugLogFlag("Selection", ImGuiDebugLogFlags_EventSelection);
    ShowDebugLogFlag("InputRouting", ImGuiDebugLogFlags_EventInputRouting);

    if (SmallButton("Clear"))
    {
        g.DebugLogBuf.clear();
        g.DebugLogIndex.clear();
        g.DebugLogSkippedErrors = 0;
    }
    SameLine();
    if (SmallButton("Copy"))
        SetClipboardText(g.DebugLogBuf.c_str());
    SameLine();
    if (SmallButton("Configure Outputs.."))
        OpenPopup("Outputs");
    if (BeginPopup("Outputs"))
    {
        CheckboxFlags("OutputToTTY", &g.DebugLogFlags, ImGuiDebugLogFlags_OutputToTTY);
        CheckboxFlags("OutputToDebugger", &g.DebugLogFlags, ImGuiDebugLogFlags_OutputToDebugger);
#ifndef IMGUI_ENABLE_TEST_ENGINE
        BeginDisabled();
#endif
        CheckboxFlags("OutputToTestEngine", &g.DebugLogFlags, ImGuiDebugLogFlags_OutputToTestEngine);
#ifndef IMGUI_ENABLE_TEST_ENGINE
        EndDisabled();
#endif
        EndPopup();
    }

    BeginChild("##log", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

    const ImGuiDebugLogFlags backup_log_flags = g.DebugLogFlags;
    g.DebugLogFlags &= ~ImGuiDebugLogFlags_EventClipper;

    ImGuiListClipper clipper;
    clipper.Begin(g.DebugLogIndex.size());
    while (clipper.Step())
        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
            DebugTextUnformattedWithLocateItem(g.DebugLogIndex.get_line_begin(g.DebugLogBuf.c_str(), line_no), g.DebugLogIndex.get_line_end(g.DebugLogBuf.c_str(), line_no));
    g.DebugLogFlags = backup_log_flags;
    if (GetScrollY() >= GetScrollMaxY())
        SetScrollHereY(1.0f);
    EndChild();

    End();
}

void ImGui::DebugTextUnformattedWithLocateItem(const char* line_begin, const char* line_end)
{
    TextUnformatted(line_begin, line_end);
    if (!IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        return;
    ImGuiContext& g = *GImGui;
    ImRect text_rect = g.LastItemData.Rect;
    for (const char* p = line_begin; p <= line_end - 10; p++)
    {
        ImGuiID id = 0;
        if (p[0] != '0' || (p[1] != 'x' && p[1] != 'X') || sscanf(p + 2, "%X", &id) != 1 || ImCharIsXdigitA(p[10]))
            continue;
        ImVec2 p0 = CalcTextSize(line_begin, p);
        ImVec2 p1 = CalcTextSize(p, p + 10);
        g.LastItemData.Rect = ImRect(text_rect.Min + ImVec2(p0.x, 0.0f), text_rect.Min + ImVec2(p0.x + p1.x, p1.y));
        if (IsMouseHoveringRect(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, true))
            DebugLocateItemOnHover(id);
        p += 10;
    }
}

void ImGui::DebugDrawCursorPos(ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImVec2 pos = window->DC.CursorPos;
    window->DrawList->AddLine(ImVec2(pos.x, pos.y - 3.0f), ImVec2(pos.x, pos.y + 4.0f), col, 1.0f);
    window->DrawList->AddLine(ImVec2(pos.x - 3.0f, pos.y), ImVec2(pos.x + 4.0f, pos.y), col, 1.0f);
}

void ImGui::DebugDrawLineExtents(ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    float curr_x = window->DC.CursorPos.x;
    float line_y1 = (window->DC.IsSameLine ? window->DC.CursorPosPrevLine.y : window->DC.CursorPos.y);
    float line_y2 = line_y1 + (window->DC.IsSameLine ? window->DC.PrevLineSize.y : window->DC.CurrLineSize.y);
    window->DrawList->AddLine(ImVec2(curr_x - 5.0f, line_y1), ImVec2(curr_x + 5.0f, line_y1), col, 1.0f);
    window->DrawList->AddLine(ImVec2(curr_x - 0.5f, line_y1), ImVec2(curr_x - 0.5f, line_y2), col, 1.0f);
    window->DrawList->AddLine(ImVec2(curr_x - 5.0f, line_y2), ImVec2(curr_x + 5.0f, line_y2), col, 1.0f);
}

void ImGui::DebugDrawItemRect(ImU32 col)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    GetForegroundDrawList(window)->AddRect(g.LastItemData.Rect.Min, g.LastItemData.Rect.Max, col);
}

static const ImU32 DEBUG_LOCATE_ITEM_COLOR = IM_COL32(0, 255, 0, 255);

void ImGui::DebugLocateItem(ImGuiID target_id)
{
    ImGuiContext& g = *GImGui;
    g.DebugLocateId = target_id;
    g.DebugLocateFrames = 2;
    g.DebugBreakInLocateId = false;
}

void ImGui::DebugLocateItemOnHover(ImGuiID target_id)
{
    if (target_id == 0 || !IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        return;
    ImGuiContext& g = *GImGui;
    DebugLocateItem(target_id);
    GetForegroundDrawList(g.CurrentWindow)->AddRect(g.LastItemData.Rect.Min - ImVec2(3.0f, 3.0f), g.LastItemData.Rect.Max + ImVec2(3.0f, 3.0f), DEBUG_LOCATE_ITEM_COLOR);

    if (g.IO.ConfigDebugIsDebuggerPresent && g.MouseStationaryTimer > 1.0f)
    {
        DebugBreakButtonTooltip(false, "in ItemAdd()");
        if (IsKeyChordPressed(g.DebugBreakKeyChord))
            g.DebugBreakInLocateId = true;
    }
}

void ImGui::DebugLocateItemResolveWithLastItem()
{
    ImGuiContext& g = *GImGui;

    if (g.DebugBreakInLocateId)
        IM_DEBUG_BREAK();

    ImGuiLastItemData item_data = g.LastItemData;
    g.DebugLocateId = 0;
    ImDrawList* draw_list = GetForegroundDrawList(g.CurrentWindow);
    ImRect r = item_data.Rect;
    r.Expand(3.0f);
    ImVec2 p1 = g.IO.MousePos;
    ImVec2 p2 = ImVec2((p1.x < r.Min.x) ? r.Min.x : (p1.x > r.Max.x) ? r.Max.x : p1.x, (p1.y < r.Min.y) ? r.Min.y : (p1.y > r.Max.y) ? r.Max.y : p1.y);
    draw_list->AddRect(r.Min, r.Max, DEBUG_LOCATE_ITEM_COLOR);
    draw_list->AddLine(p1, p2, DEBUG_LOCATE_ITEM_COLOR);
}

void ImGui::DebugStartItemPicker()
{
    ImGuiContext& g = *GImGui;
    g.DebugItemPickerActive = true;
}

void ImGui::UpdateDebugToolItemPicker()
{
    ImGuiContext& g = *GImGui;
    g.DebugItemPickerBreakId = 0;
    if (!g.DebugItemPickerActive)
        return;

    const ImGuiID hovered_id = g.HoveredIdPreviousFrame;
    SetMouseCursor(ImGuiMouseCursor_Hand);
    if (IsKeyPressed(ImGuiKey_Escape))
        g.DebugItemPickerActive = false;
    const bool change_mapping = g.IO.KeyMods == (ImGuiMod_Ctrl | ImGuiMod_Shift);
    if (!change_mapping && IsMouseClicked(g.DebugItemPickerMouseButton) && hovered_id)
    {
        g.DebugItemPickerBreakId = hovered_id;
        g.DebugItemPickerActive = false;
    }
    for (int mouse_button = 0; mouse_button < 3; mouse_button++)
        if (change_mapping && IsMouseClicked(mouse_button))
            g.DebugItemPickerMouseButton = (ImU8)mouse_button;
    SetNextWindowBgAlpha(0.70f);
    if (!BeginTooltip())
        return;
    Text("HoveredId: 0x%08X", hovered_id);
    Text("Press ESC to abort picking.");
    const char* mouse_button_names[] = { "Left", "Right", "Middle" };
    if (change_mapping)
        Text("Remap w/ Ctrl+Shift: click anywhere to select new mouse button.");
    else
        TextColored(GetStyleColorVec4(hovered_id ? ImGuiCol_Text : ImGuiCol_TextDisabled), "Click %s Button to break in debugger! (remap w/ Ctrl+Shift)", mouse_button_names[g.DebugItemPickerMouseButton]);
    EndTooltip();
}

static ImGuiID DebugItemPathQuery_UpdateAndGetHookId(ImGuiDebugItemPathQuery* query, ImGuiID id)
{

    if (query->MainID != id)
    {
        query->MainID = id;
        query->Step = -1;
        query->Complete = false;
        query->Results.resize(0);
        query->ResultsDescBuf.resize(0);
    }
    query->Active = false;
    if (id == 0)
        return 0;

    if (query->Step >= 0 && query->Step < query->Results.Size)
        if (query->Results[query->Step].QuerySuccess || query->Results[query->Step].QueryFrameCount > 2)
            query->Step++;

    query->Complete = (query->Step == query->Results.Size);
    if (query->Step == -1)
    {
        query->Active = true;
        return id;
    }
    else if (query->Step >= 0 && query->Step < query->Results.Size)
    {
        query->Results[query->Step].QueryFrameCount++;
        query->Active = true;
        return query->Results[query->Step].ID;
    }
    return 0;
}

void ImGui::UpdateDebugToolItemPathQuery()
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = 0;
    if (g.DebugIDStackTool.LastActiveFrame + 1 == g.FrameCount)
        id = g.HoveredIdPreviousFrame ? g.HoveredIdPreviousFrame : g.ActiveId;
    g.DebugHookIdInfoId = DebugItemPathQuery_UpdateAndGetHookId(&g.DebugItemPathQuery, id);
}

void ImGui::DebugHookIdInfo(ImGuiID id, ImGuiDataType data_type, const void* data_id, const void* data_id_end)
{
    ImGuiContext& g = *GImGui;
    ImGuiDebugItemPathQuery* query = &g.DebugItemPathQuery;
    if (query->Active == false)
    {
        IM_ASSERT(id == 0);
        return;
    }
    ImGuiWindow* window = g.CurrentWindow;

    if (query->Step == -1)
    {
        IM_ASSERT(query->Results.Size == 0);
        query->Step++;
        query->Results.resize(window->IDStack.Size + 1, ImGuiStackLevelInfo());
        for (int n = 0; n < window->IDStack.Size + 1; n++)
            query->Results[n].ID = (n < window->IDStack.Size) ? window->IDStack[n] : id;
        return;
    }

    IM_ASSERT(query->Step >= 0);
    if (query->Step != window->IDStack.Size)
        return;
    ImGuiStackLevelInfo* info = &query->Results[query->Step];
    IM_ASSERT(info->ID == id && info->QueryFrameCount > 0);

    if (info->DescOffset == -1)
    {
        const char* result = NULL;
        const char* result_end = NULL;
        switch (data_type)
        {
        case ImGuiDataType_S32:
            ImFormatStringToTempBuffer(&result, &result_end, "%d", (int)(intptr_t)data_id);
            break;
        case ImGuiDataType_String:
            ImFormatStringToTempBuffer(&result, &result_end, "%.*s", data_id_end ? (int)((const char*)data_id_end - (const char*)data_id) : (int)ImStrlen((const char*)data_id), (const char*)data_id);
            break;
        case ImGuiDataType_Pointer:
            ImFormatStringToTempBuffer(&result, &result_end, "(void*)0x%p", data_id);
            break;
        case ImGuiDataType_ID:

            ImFormatStringToTempBuffer(&result, &result_end, "0x%08X [override]", id);
            break;
        default:
            IM_ASSERT(0);
        }
        info->DescOffset = query->ResultsDescBuf.size();
        query->ResultsDescBuf.append(result, result_end + 1);
    }
    info->QuerySuccess = true;
    if (info->DataType == -1)
        info->DataType = (ImS8)data_type;
}

static int DebugItemPathQuery_FormatLevelInfo(ImGuiDebugItemPathQuery* query, int n, bool format_for_ui, char* buf, size_t buf_size)
{
    ImGuiStackLevelInfo* info = &query->Results[n];
    ImGuiWindow* window = (info->DescOffset == -1 && n == 0) ? ImGui::FindWindowByID(info->ID) : NULL;
    if (window)
        return ImFormatString(buf, buf_size, format_for_ui ? "\"%s\" [window]" : "%s", ImHashSkipUncontributingPrefix(window->Name));
    if (info->QuerySuccess)
        return ImFormatString(buf, buf_size, (format_for_ui && info->DataType == ImGuiDataType_String) ? "\"%s\"" : "%s", ImHashSkipUncontributingPrefix(&query->ResultsDescBuf.Buf[info->DescOffset]));
    if (query->Step < query->Results.Size)
        return (*buf = 0);
#ifdef IMGUI_ENABLE_TEST_ENGINE
    if (const char* label = ImGuiTestEngine_FindItemDebugLabel(GImGui, info->ID))
        return ImFormatString(buf, buf_size, format_for_ui ? "??? \"%s\"" : "%s", ImHashSkipUncontributingPrefix(label));
#endif
    return ImFormatString(buf, buf_size, "???");
}

static const char* DebugItemPathQuery_GetResultAsPath(ImGuiDebugItemPathQuery* query, bool hex_encode_non_ascii_chars)
{
    ImGuiTextBuffer* buf = &query->ResultPathBuf;
    buf->resize(0);
    for (int stack_n = 0; stack_n < query->Results.Size; stack_n++)
    {
        char level_desc[256];
        DebugItemPathQuery_FormatLevelInfo(query, stack_n, false, level_desc, IM_COUNTOF(level_desc));
        buf->append(stack_n == 0 ? "//" : "/");
        for (const char* p = level_desc; *p != 0; )
        {
            unsigned int c;
            const char* p_next = p + ImTextCharFromUtf8(&c, p, NULL);
            if (c == '/')
                buf->append("\\");
            if (c < 256 || !hex_encode_non_ascii_chars)
                buf->append(p, p_next);
            else for (; p < p_next; p++)
                buf->appendf("\\x%02x", (unsigned char)*p);
            p = p_next;
        }
    }
    return buf->c_str();
}

void ImGui::ShowIDStackToolWindow(bool* p_open)
{
    ImGuiContext& g = *GImGui;
    if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasSize) == 0)
        SetNextWindowSize(ImVec2(0.0f, GetFontSize() * 8.0f), ImGuiCond_FirstUseEver);
    if (!Begin("Dear ImGui ID Stack Tool", p_open) || GetCurrentWindow()->BeginCount > 1)
    {
        End();
        return;
    }

    ImGuiDebugItemPathQuery* query = &g.DebugItemPathQuery;
    ImGuiIDStackTool* tool = &g.DebugIDStackTool;
    tool->LastActiveFrame = g.FrameCount;
    const char* result_path = DebugItemPathQuery_GetResultAsPath(query, tool->OptHexEncodeNonAsciiChars);
    Text("0x%08X", query->MainID);
    SameLine();
    MetricsHelpMarker("Hover an item with the mouse to display elements of the ID Stack leading to the item's final ID.\nEach level of the stack correspond to a PushID() call.\nAll levels of the stack are hashed together to make the final ID of a widget (ID displayed at the bottom level of the stack).\nRead FAQ entry about the ID stack for details.");

    const float time_since_copy = (float)g.Time - tool->CopyToClipboardLastTime;
    PushStyleVarY(ImGuiStyleVar_FramePadding, 0.0f);
    Checkbox("Hex-encode non-ASCII", &tool->OptHexEncodeNonAsciiChars);
    SameLine();
    Checkbox("Ctrl+C: copy path", &tool->OptCopyToClipboardOnCtrlC);
    PopStyleVar();
    SameLine();
    TextColored((time_since_copy >= 0.0f && time_since_copy < 0.75f && ImFmod(time_since_copy, 0.25f) < 0.25f * 0.5f) ? ImVec4(1.f, 1.f, 0.3f, 1.f) : ImVec4(), "*COPIED*");
    if (tool->OptCopyToClipboardOnCtrlC && Shortcut(ImGuiMod_Ctrl | ImGuiKey_C, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteOverFocused))
    {
        tool->CopyToClipboardLastTime = (float)g.Time;
        SetClipboardText(result_path);
    }

    Text("- Path \"%s\"", query->Complete ? result_path : "");
#ifdef IMGUI_ENABLE_TEST_ENGINE
    Text("- Label \"%s\"", query->MainID ? ImGuiTestEngine_FindItemDebugLabel(&g, query->MainID) : "");
#endif
    Separator();

    if (query->Results.Size > 0 && BeginTable("##table", 3, ImGuiTableFlags_Borders))
    {
        const float id_width = CalcTextSize("0xDDDDDDDD").x;
        TableSetupColumn("Seed", ImGuiTableColumnFlags_WidthFixed, id_width);
        TableSetupColumn("PushID", ImGuiTableColumnFlags_WidthStretch);
        TableSetupColumn("Result", ImGuiTableColumnFlags_WidthFixed, id_width);
        TableHeadersRow();
        for (int n = 0; n < query->Results.Size; n++)
        {
            ImGuiStackLevelInfo* info = &query->Results[n];
            TableNextColumn();
            Text("0x%08X", (n > 0) ? query->Results[n - 1].ID : 0);
            TableNextColumn();
            DebugItemPathQuery_FormatLevelInfo(query, n, true, g.TempBuffer.Data, g.TempBuffer.Size);
            TextUnformatted(g.TempBuffer.Data);
            TableNextColumn();
            Text("0x%08X", info->ID);
            if (n == query->Results.Size - 1)
                TableSetBgColor(ImGuiTableBgTarget_CellBg, GetColorU32(ImGuiCol_Header));
        }
        EndTable();
    }
    End();
}

#else

void ImGui::ShowMetricsWindow(bool*) {}
void ImGui::ShowFontAtlas(ImFontAtlas*) {}
void ImGui::DebugNodeColumns(ImGuiOldColumns*) {}
void ImGui::DebugNodeDrawList(ImGuiWindow*, ImGuiViewportP*, const ImDrawList*, const char*) {}
void ImGui::DebugNodeDrawCmdShowMeshAndBoundingBox(ImDrawList*, const ImDrawList*, const ImDrawCmd*, bool, bool) {}
void ImGui::DebugNodeFont(ImFont*) {}
void ImGui::DebugNodeFontGlyphsForSrcMask(ImFont*, ImFontBaked*, int) {}
void ImGui::DebugNodeStorage(ImGuiStorage*, const char*) {}
void ImGui::DebugNodeTabBar(ImGuiTabBar*, const char*) {}
void ImGui::DebugNodeWindow(ImGuiWindow*, const char*) {}
void ImGui::DebugNodeWindowSettings(ImGuiWindowSettings*) {}
void ImGui::DebugNodeWindowsList(ImVector<ImGuiWindow*>*, const char*) {}
void ImGui::DebugNodeViewport(ImGuiViewportP*) {}

void ImGui::ShowDebugLogWindow(bool*) {}
void ImGui::ShowIDStackToolWindow(bool*) {}
void ImGui::DebugStartItemPicker() {}
void ImGui::DebugHookIdInfo(ImGuiID, ImGuiDataType, const void*, const void*) {}

#endif

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS) || !defined(IMGUI_DISABLE_DEBUG_TOOLS)

void ImGui::ShowFontSelector(const char* label)
{
    ImGuiIO& io = GetIO();
    ImFont* font_current = GetFont();
    if (BeginCombo(label, font_current->GetDebugName()))
    {
        for (ImFont* font : io.Fonts->Fonts)
        {
            PushID((void*)font);
            if (Selectable(font->GetDebugName(), font == font_current, ImGuiSelectableFlags_SelectOnNav))
                io.FontDefault = font;
            if (font == font_current)
                SetItemDefaultFocus();
            PopID();
        }
        EndCombo();
    }
    SameLine();
    if (io.BackendFlags & ImGuiBackendFlags_RendererHasTextures)
        MetricsHelpMarker(
            "- Load additional fonts with io.Fonts->AddFontXXX() functions.\n"
            "- Read FAQ and docs/FONTS.md for more details.");
    else
        MetricsHelpMarker(
            "- Load additional fonts with io.Fonts->AddFontXXX() functions.\n"
            "- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
            "- Read FAQ and docs/FONTS.md for more details.\n"
            "- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");
}
#endif

#ifdef IMGUI_INCLUDE_IMGUI_USER_INL
#include "imgui_user.inl"
#endif

#endif
