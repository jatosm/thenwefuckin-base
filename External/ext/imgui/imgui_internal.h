
#pragma once
#ifndef IMGUI_DISABLE

#ifndef IMGUI_VERSION
#include "imgui.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#if (defined __SSE__ || defined __x86_64__ || defined _M_X64 || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))) && !defined(IMGUI_DISABLE_SSE) && !defined(_M_ARM64) && !defined(_M_ARM64EC)
#define IMGUI_ENABLE_SSE
#include <immintrin.h>
#if (defined __AVX__ || defined __SSE4_2__)
#define IMGUI_ENABLE_SSE4_2
#include <nmmintrin.h>
#endif
#endif

#if defined(IMGUI_ENABLE_SSE4_2) && !defined(IMGUI_USE_LEGACY_CRC32_ADLER) && !defined(__EMSCRIPTEN__)
#define IMGUI_ENABLE_SSE4_2_CRC
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4251)
#pragma warning (disable: 26495)
#pragma warning (disable: 26812)
#if defined(_MSC_VER) && _MSC_VER >= 1922
#pragma warning (disable: 5054)
#endif
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wfloat-equal"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wnontrivial-memaccess"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#if defined(IMGUI_DEFINE_MATH_OPERATORS) && !defined(IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED)
#error Please '#define IMGUI_DEFINE_MATH_OPERATORS' _BEFORE_ including imgui.h!
#endif

#ifdef IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS
#error Use IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#endif
#ifdef IMGUI_DISABLE_MATH_FUNCTIONS
#error Use IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#endif

#ifndef IMGUI_ENABLE_FREETYPE
#define IMGUI_ENABLE_STB_TRUETYPE
#endif

struct ImBitVector;
struct ImRect;
struct ImGuiTextIndex;

struct ImDrawDataBuilder;
struct ImDrawListSharedData;
struct ImFontAtlasBuilder;
struct ImFontAtlasPostProcessData;
struct ImFontAtlasRectEntry;

struct ImGuiBoxSelectState;
struct ImGuiColorMod;
struct ImGuiContext;
struct ImGuiContextHook;
struct ImGuiDataTypeInfo;
struct ImGuiDeactivatedItemData;
struct ImGuiErrorRecoveryState;
struct ImGuiGroupData;
struct ImGuiInputTextState;
struct ImGuiInputTextDeactivateData;
struct ImGuiLastItemData;
struct ImGuiLocEntry;
struct ImGuiMenuColumns;
struct ImGuiMultiSelectState;
struct ImGuiMultiSelectTempData;
struct ImGuiNavItemData;
struct ImGuiMetricsConfig;
struct ImGuiNextWindowData;
struct ImGuiNextItemData;
struct ImGuiOldColumnData;
struct ImGuiOldColumns;
struct ImGuiPopupData;
struct ImGuiSettingsHandler;
struct ImGuiStyleMod;
struct ImGuiStyleVarInfo;
struct ImGuiTabBar;
struct ImGuiTabItem;
struct ImGuiTable;
struct ImGuiTableHeaderData;
struct ImGuiTableColumn;
struct ImGuiTableInstanceData;
struct ImGuiTableTempData;
struct ImGuiTableSettings;
struct ImGuiTableColumnsSettings;
struct ImGuiTreeNodeStackData;
struct ImGuiTypingSelectState;
struct ImGuiTypingSelectRequest;
struct ImGuiWindow;
struct ImGuiWindowTempData;
struct ImGuiWindowSettings;

enum ImGuiLocKey : int;
typedef int ImGuiLayoutType;

typedef int ImDrawTextFlags;
typedef int ImGuiActivateFlags;
typedef int ImGuiDebugLogFlags;
typedef int ImGuiFocusRequestFlags;
typedef int ImGuiItemStatusFlags;
typedef int ImGuiOldColumnFlags;
typedef int ImGuiLogFlags;
typedef int ImGuiNavRenderCursorFlags;
typedef int ImGuiNavMoveFlags;
typedef int ImGuiNextItemDataFlags;
typedef int ImGuiNextWindowDataFlags;
typedef int ImGuiScrollFlags;
typedef int ImGuiSeparatorFlags;
typedef int ImGuiTextFlags;
typedef int ImGuiTooltipFlags;
typedef int ImGuiTypingSelectFlags;
typedef int ImGuiWindowBgClickFlags;
typedef int ImGuiWindowRefreshFlags;

typedef ImS16 ImGuiTableColumnIdx;
typedef ImU16 ImGuiTableDrawChannelIdx;

#ifndef GImGui
extern IMGUI_API ImGuiContext* GImGui;
#endif

#ifndef IMGUI_DEBUG_PRINTF
#ifndef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
#define IMGUI_DEBUG_PRINTF(_FMT,...)    printf(_FMT, __VA_ARGS__)
#else
#define IMGUI_DEBUG_PRINTF(_FMT,...)    ((void)0)
#endif
#endif

#define IMGUI_DEBUG_LOG_ERROR(...)      do { if (g.DebugLogFlags & ImGuiDebugLogFlags_EventError)       IMGUI_DEBUG_LOG(__VA_ARGS__); else g.DebugLogSkippedErrors++; } while (0)
#define IMGUI_DEBUG_LOG_ACTIVEID(...)   do { if (g.DebugLogFlags & ImGuiDebugLogFlags_EventActiveId)    IMGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define IMGUI_DEBUG_LOG_FOCUS(...)      do { if (g.DebugLogFlags & ImGuiDebugLogFlags_EventFocus)       IMGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define IMGUI_DEBUG_LOG_POPUP(...)      do { if (g.DebugLogFlags & ImGuiDebugLogFlags_EventPopup)       IMGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define IMGUI_DEBUG_LOG_NAV(...)        do { if (g.DebugLogFlags & ImGuiDebugLogFlags_EventNav)         IMGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define IMGUI_DEBUG_LOG_SELECTION(...)  do { if (g.DebugLogFlags & ImGuiDebugLogFlags_EventSelection)   IMGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define IMGUI_DEBUG_LOG_CLIPPER(...)    do { if (g.DebugLogFlags & ImGuiDebugLogFlags_EventClipper)     IMGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define IMGUI_DEBUG_LOG_IO(...)         do { if (g.DebugLogFlags & ImGuiDebugLogFlags_EventIO)          IMGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define IMGUI_DEBUG_LOG_FONT(...)       do { ImGuiContext* g2 = GImGui; if (g2 && g2->DebugLogFlags & ImGuiDebugLogFlags_EventFont) IMGUI_DEBUG_LOG(__VA_ARGS__); } while (0)
#define IMGUI_DEBUG_LOG_INPUTROUTING(...) do{if (g.DebugLogFlags & ImGuiDebugLogFlags_EventInputRouting)IMGUI_DEBUG_LOG(__VA_ARGS__); } while (0)

#define IM_STATIC_ASSERT(_COND)         static_assert(_COND, "")

#ifdef IMGUI_DEBUG_PARANOID
#define IM_ASSERT_PARANOID(_EXPR)       IM_ASSERT(_EXPR)
#else
#define IM_ASSERT_PARANOID(_EXPR)
#endif

#define IM_PI                           3.14159265358979323846f
#ifdef _WIN32
#define IM_NEWLINE                      "\r\n"
#else
#define IM_NEWLINE                      "\n"
#endif
#ifndef IM_TABSIZE
#define IM_TABSIZE                      (4)
#endif
#define IM_MEMALIGN(_OFF,_ALIGN)        (((_OFF) + ((_ALIGN) - 1)) & ~((_ALIGN) - 1))
#define IM_F32_TO_INT8_UNBOUND(_VAL)    ((int)((_VAL) * 255.0f + ((_VAL)>=0 ? 0.5f : -0.5f)))
#define IM_F32_TO_INT8_SAT(_VAL)        ((int)(ImSaturate(_VAL) * 255.0f + 0.5f))
#define IM_TRUNC(_VAL)                  ((float)(int)(_VAL))
#define IM_ROUND(_VAL)                  ((float)(int)((_VAL) + 0.5f))

#if (defined(__cplusplus) && (__cplusplus >= 202002L)) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L))
#define IM_LIKELY   [[likely]]
#define IM_UNLIKELY [[unlikely]]
#else
#define IM_LIKELY
#define IM_UNLIKELY
#endif

#ifdef _MSC_VER
#define IMGUI_CDECL __cdecl
#else
#define IMGUI_CDECL
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define IM_MSVC_WARNING_SUPPRESS(XXXX)  __pragma(warning(suppress: XXXX))
#else
#define IM_MSVC_WARNING_SUPPRESS(XXXX)
#endif

#ifndef IM_DEBUG_BREAK
#if defined (_MSC_VER)
#define IM_DEBUG_BREAK()    __debugbreak()
#elif defined(__clang__)
#define IM_DEBUG_BREAK()    __builtin_debugtrap()
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define IM_DEBUG_BREAK()    __asm__ volatile("int3;nop")
#elif defined(__GNUC__) && defined(__thumb__)
#define IM_DEBUG_BREAK()    __asm__ volatile(".inst 0xde01")
#elif defined(__GNUC__) && defined(__arm__) && !defined(__thumb__)
#define IM_DEBUG_BREAK()    __asm__ volatile(".inst 0xe7f001f0")
#else
#define IM_DEBUG_BREAK()    IM_ASSERT(0)
#endif
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define IM_PRId64   "I64d"
#define IM_PRIu64   "I64u"
#define IM_PRIX64   "I64X"
#else
#define IM_PRId64   "lld"
#define IM_PRIu64   "llu"
#define IM_PRIX64   "llX"
#endif

IMGUI_API ImGuiID       ImHashData(const void* data, size_t data_size, ImGuiID seed = 0);
IMGUI_API ImGuiID       ImHashStr(const char* data, size_t data_size = 0, ImGuiID seed = 0);
IMGUI_API const char*   ImHashSkipUncontributingPrefix(const char* label);

#ifndef ImQsort
inline void             ImQsort(void* base, size_t count, size_t size_of_element, int(IMGUI_CDECL *compare_func)(void const*, void const*)) { if (count > 1) qsort(base, count, size_of_element, compare_func); }
#endif

IMGUI_API ImU32         ImAlphaBlendColors(ImU32 col_a, ImU32 col_b);

inline bool             ImIsPowerOfTwo(int v)               { return v != 0 && (v & (v - 1)) == 0; }
inline bool             ImIsPowerOfTwo(ImU64 v)             { return v != 0 && (v & (v - 1)) == 0; }
inline int              ImUpperPowerOfTwo(int v)            { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }
inline unsigned int     ImCountSetBits(unsigned int v)      { unsigned int count = 0; while (v > 0) { v = v & (v - 1); count++; } return count; }

#define ImStrlen strlen
#define ImMemchr memchr
IMGUI_API int           ImStricmp(const char* str1, const char* str2);
IMGUI_API int           ImStrnicmp(const char* str1, const char* str2, size_t count);
IMGUI_API void          ImStrncpy(char* dst, const char* src, size_t count);
IMGUI_API char*         ImStrdup(const char* str);
IMGUI_API void*         ImMemdup(const void* src, size_t size);
IMGUI_API char*         ImStrdupcpy(char* dst, size_t* p_dst_size, const char* str);
IMGUI_API const char*   ImStrchrRange(const char* str_begin, const char* str_end, char c);
IMGUI_API const char*   ImStreolRange(const char* str, const char* str_end);
IMGUI_API const char*   ImStristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end);
IMGUI_API void          ImStrTrimBlanks(char* str);
IMGUI_API const char*   ImStrSkipBlank(const char* str);
IMGUI_API int           ImStrlenW(const ImWchar* str);
IMGUI_API const char*   ImStrbol(const char* buf_mid_line, const char* buf_begin);
IM_MSVC_RUNTIME_CHECKS_OFF
inline char             ImToUpper(char c)               { return (c >= 'a' && c <= 'z') ? c &= ~32 : c; }
inline bool             ImCharIsBlankA(char c)          { return c == ' ' || c == '\t'; }
inline bool             ImCharIsBlankW(unsigned int c)  { return c == ' ' || c == '\t' || c == 0x3000; }
inline bool             ImCharIsXdigitA(char c)         { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
IM_MSVC_RUNTIME_CHECKS_RESTORE

IMGUI_API int           ImFormatString(char* buf, size_t buf_size, const char* fmt, ...) IM_FMTARGS(3);
IMGUI_API int           ImFormatStringV(char* buf, size_t buf_size, const char* fmt, va_list args) IM_FMTLIST(3);
IMGUI_API void          ImFormatStringToTempBuffer(const char** out_buf, const char** out_buf_end, const char* fmt, ...) IM_FMTARGS(3);
IMGUI_API void          ImFormatStringToTempBufferV(const char** out_buf, const char** out_buf_end, const char* fmt, va_list args) IM_FMTLIST(3);
IMGUI_API const char*   ImParseFormatFindStart(const char* format);
IMGUI_API const char*   ImParseFormatFindEnd(const char* format);
IMGUI_API const char*   ImParseFormatTrimDecorations(const char* format, char* buf, size_t buf_size);
IMGUI_API void          ImParseFormatSanitizeForPrinting(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
IMGUI_API const char*   ImParseFormatSanitizeForScanning(const char* fmt_in, char* fmt_out, size_t fmt_out_size);
IMGUI_API int           ImParseFormatPrecision(const char* format, int default_value);

IMGUI_API int           ImTextCharToUtf8(char out_buf[5], unsigned int c);
IMGUI_API int           ImTextStrToUtf8(char* out_buf, int out_buf_size, const ImWchar* in_text, const ImWchar* in_text_end);
IMGUI_API int           ImTextCharFromUtf8(unsigned int* out_char, const char* in_text, const char* in_text_end);
IMGUI_API int           ImTextStrFromUtf8(ImWchar* out_buf, int out_buf_size, const char* in_text, const char* in_text_end, const char** in_remaining = NULL);
IMGUI_API int           ImTextCountCharsFromUtf8(const char* in_text, const char* in_text_end);
IMGUI_API int           ImTextCountUtf8BytesFromChar(const char* in_text, const char* in_text_end);
IMGUI_API int           ImTextCountUtf8BytesFromStr(const ImWchar* in_text, const ImWchar* in_text_end);
IMGUI_API const char*   ImTextFindPreviousUtf8Codepoint(const char* in_text_start, const char* in_p);
IMGUI_API const char*   ImTextFindValidUtf8CodepointEnd(const char* in_text_start, const char* in_text_end, const char* in_p);
IMGUI_API int           ImTextCountLines(const char* in_text, const char* in_text_end);

enum ImDrawTextFlags_
{
    ImDrawTextFlags_None                = 0,
    ImDrawTextFlags_CpuFineClip         = 1 << 0,
    ImDrawTextFlags_WrapKeepBlanks      = 1 << 1,
    ImDrawTextFlags_StopOnNewLine       = 1 << 2,
};
IMGUI_API ImVec2        ImFontCalcTextSizeEx(ImFont* font, float size, float max_width, float wrap_width, const char* text_begin, const char* text_end_display, const char* text_end, const char** out_remaining, ImVec2* out_offset, ImDrawTextFlags flags);
IMGUI_API const char*   ImFontCalcWordWrapPositionEx(ImFont* font, float size, const char* text, const char* text_end, float wrap_width, ImDrawTextFlags flags = 0);
IMGUI_API const char*   ImTextCalcWordWrapNextLineStart(const char* text, const char* text_end, ImDrawTextFlags flags = 0);

enum ImWcharClass
{
    ImWcharClass_Blank, ImWcharClass_Punct, ImWcharClass_Other
};
IMGUI_API void          ImTextInitClassifiers();
IMGUI_API void          ImTextClassifierClear(ImU32* bits, unsigned int codepoint_min, unsigned int codepoint_end, ImWcharClass char_class);
IMGUI_API void          ImTextClassifierSetCharClass(ImU32* bits, unsigned int codepoint_min, unsigned int codepoint_end, ImWcharClass char_class, unsigned int c);
IMGUI_API void          ImTextClassifierSetCharClassFromStr(ImU32* bits, unsigned int codepoint_min, unsigned int codepoint_end, ImWcharClass char_class, const char* s);

#ifdef IMGUI_DISABLE_FILE_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef void* ImFileHandle;
inline ImFileHandle         ImFileOpen(const char*, const char*)                    { return NULL; }
inline bool                 ImFileClose(ImFileHandle)                               { return false; }
inline ImU64                ImFileGetSize(ImFileHandle)                             { return (ImU64)-1; }
inline ImU64                ImFileRead(void*, ImU64, ImU64, ImFileHandle)           { return 0; }
inline ImU64                ImFileWrite(const void*, ImU64, ImU64, ImFileHandle)    { return 0; }
#endif
#ifndef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
typedef FILE* ImFileHandle;
IMGUI_API ImFileHandle      ImFileOpen(const char* filename, const char* mode);
IMGUI_API bool              ImFileClose(ImFileHandle file);
IMGUI_API ImU64             ImFileGetSize(ImFileHandle file);
IMGUI_API ImU64             ImFileRead(void* data, ImU64 size, ImU64 count, ImFileHandle file);
IMGUI_API ImU64             ImFileWrite(const void* data, ImU64 size, ImU64 count, ImFileHandle file);
#else
#define IMGUI_DISABLE_TTY_FUNCTIONS
#endif
IMGUI_API void*             ImFileLoadToMemory(const char* filename, const char* mode, size_t* out_file_size = NULL, int padding_bytes = 0);

IM_MSVC_RUNTIME_CHECKS_OFF

#ifndef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
#define ImFabs(X)           fabsf(X)
#define ImSqrt(X)           sqrtf(X)
#define ImFmod(X, Y)        fmodf((X), (Y))
#define ImCos(X)            cosf(X)
#define ImSin(X)            sinf(X)
#define ImAcos(X)           acosf(X)
#define ImAtan2(Y, X)       atan2f((Y), (X))
#define ImAtof(STR)         atof(STR)
#define ImCeil(X)           ceilf(X)
inline float  ImPow(float x, float y)    { return powf(x, y); }
inline double ImPow(double x, double y)  { return pow(x, y); }
inline float  ImLog(float x)             { return logf(x); }
inline double ImLog(double x)            { return log(x); }
inline int    ImAbs(int x)               { return x < 0 ? -x : x; }
inline float  ImAbs(float x)             { return fabsf(x); }
inline double ImAbs(double x)            { return fabs(x); }
inline float  ImSign(float x)            { return (x < 0.0f) ? -1.0f : (x > 0.0f) ? 1.0f : 0.0f; }
inline double ImSign(double x)           { return (x < 0.0) ? -1.0 : (x > 0.0) ? 1.0 : 0.0; }
#ifdef IMGUI_ENABLE_SSE
inline float  ImRsqrt(float x)           { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }
#else
inline float  ImRsqrt(float x)           { return 1.0f / sqrtf(x); }
#endif
inline double ImRsqrt(double x)          { return 1.0 / sqrt(x); }
#endif

template<typename T> T ImMin(T lhs, T rhs)                              { return lhs < rhs ? lhs : rhs; }
template<typename T> T ImMax(T lhs, T rhs)                              { return lhs >= rhs ? lhs : rhs; }
template<typename T> T ImClamp(T v, T mn, T mx)                         { return (v < mn) ? mn : (v > mx) ? mx : v; }
template<typename T> T ImLerp(T a, T b, float t)                        { return (T)(a + (b - a) * t); }
template<typename T> void ImSwap(T& a, T& b)                            { T tmp = a; a = b; b = tmp; }
template<typename T> T ImAddClampOverflow(T a, T b, T mn, T mx)         { if (b < 0 && (a < mn - b)) return mn; if (b > 0 && (a > mx - b)) return mx; return a + b; }
template<typename T> T ImSubClampOverflow(T a, T b, T mn, T mx)         { if (b > 0 && (a < mn + b)) return mn; if (b < 0 && (a > mx + b)) return mx; return a - b; }

inline ImVec2 ImMin(const ImVec2& lhs, const ImVec2& rhs)               { return ImVec2(lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y); }
inline ImVec2 ImMax(const ImVec2& lhs, const ImVec2& rhs)               { return ImVec2(lhs.x >= rhs.x ? lhs.x : rhs.x, lhs.y >= rhs.y ? lhs.y : rhs.y); }
inline ImVec2 ImClamp(const ImVec2& v, const ImVec2&mn, const ImVec2&mx){ return ImVec2((v.x < mn.x) ? mn.x : (v.x > mx.x) ? mx.x : v.x, (v.y < mn.y) ? mn.y : (v.y > mx.y) ? mx.y : v.y); }
inline ImVec2 ImLerp(const ImVec2& a, const ImVec2& b, float t)         { return ImVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
inline ImVec2 ImLerp(const ImVec2& a, const ImVec2& b, const ImVec2& t) { return ImVec2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
inline ImVec4 ImLerp(const ImVec4& a, const ImVec4& b, float t)         { return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t); }
inline float  ImSaturate(float f)                                       { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
inline float  ImLengthSqr(const ImVec2& lhs)                            { return (lhs.x * lhs.x) + (lhs.y * lhs.y); }
inline float  ImLengthSqr(const ImVec4& lhs)                            { return (lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z) + (lhs.w * lhs.w); }
inline float  ImInvLength(const ImVec2& lhs, float fail_value)          { float d = (lhs.x * lhs.x) + (lhs.y * lhs.y); if (d > 0.0f) return ImRsqrt(d); return fail_value; }
inline float  ImTrunc(float f)                                          { return (float)(int)(f); }
inline ImVec2 ImTrunc(const ImVec2& v)                                  { return ImVec2((float)(int)(v.x), (float)(int)(v.y)); }
inline float  ImFloor(float f)                                          { return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1); }
inline ImVec2 ImFloor(const ImVec2& v)                                  { return ImVec2(ImFloor(v.x), ImFloor(v.y)); }
inline float  ImTrunc64(float f)                                        { return (float)(ImS64)(f); }
inline float  ImRound64(float f)                                        { return (float)(ImS64)(f + 0.5f); }
inline int    ImModPositive(int a, int b)                               { return (a + b) % b; }
inline float  ImDot(const ImVec2& a, const ImVec2& b)                   { return a.x * b.x + a.y * b.y; }
inline ImVec2 ImRotate(const ImVec2& v, float cos_a, float sin_a)       { return ImVec2(v.x * cos_a - v.y * sin_a, v.x * sin_a + v.y * cos_a); }
inline float  ImLinearSweep(float current, float target, float speed)   { if (current < target) return ImMin(current + speed, target); if (current > target) return ImMax(current - speed, target); return current; }
inline float  ImLinearRemapClamp(float s0, float s1, float d0, float d1, float x) { return ImSaturate((x - s0) / (s1 - s0)) * (d1 - d0) + d0; }
inline ImVec2 ImMul(const ImVec2& lhs, const ImVec2& rhs)               { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline bool   ImIsFloatAboveGuaranteedIntegerPrecision(float f)         { return f <= -16777216 || f >= 16777216; }
inline float  ImExponentialMovingAverage(float avg, float sample, int n){ avg -= avg / (float)n; avg += sample / (float)n; return avg; }
IM_MSVC_RUNTIME_CHECKS_RESTORE

IMGUI_API ImVec2     ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t);
IMGUI_API ImVec2     ImBezierCubicClosestPoint(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& p, int num_segments);
IMGUI_API ImVec2     ImBezierCubicClosestPointCasteljau(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, const ImVec2& p, float tess_tol);
IMGUI_API ImVec2     ImBezierQuadraticCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t);
IMGUI_API ImVec2     ImLineClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& p);
IMGUI_API bool       ImTriangleContainsPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p);
IMGUI_API ImVec2     ImTriangleClosestPoint(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p);
IMGUI_API void       ImTriangleBarycentricCoords(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& p, float& out_u, float& out_v, float& out_w);
inline float         ImTriangleArea(const ImVec2& a, const ImVec2& b, const ImVec2& c)          { return ImFabs((a.x * (b.y - c.y)) + (b.x * (c.y - a.y)) + (c.x * (a.y - b.y))) * 0.5f; }
inline bool          ImTriangleIsClockwise(const ImVec2& a, const ImVec2& b, const ImVec2& c)   { return ((b.x - a.x) * (c.y - b.y)) - ((c.x - b.x) * (b.y - a.y)) > 0.0f; }

IM_MSVC_RUNTIME_CHECKS_OFF
struct ImVec1
{
    float   x;
    constexpr ImVec1()         : x(0.0f) { }
    constexpr ImVec1(float _x) : x(_x) { }
};

struct ImVec2i
{
    int         x, y;
    constexpr ImVec2i()                             : x(0), y(0) {}
    constexpr ImVec2i(int _x, int _y)               : x(_x), y(_y) {}
};

struct ImVec2ih
{
    short   x, y;
    constexpr ImVec2ih()                           : x(0), y(0) {}
    constexpr ImVec2ih(short _x, short _y)         : x(_x), y(_y) {}
    constexpr explicit ImVec2ih(const ImVec2& rhs) : x((short)rhs.x), y((short)rhs.y) {}
};

struct IMGUI_API ImRect
{
    ImVec2      Min;
    ImVec2      Max;

    constexpr ImRect()                                        : Min(0.0f, 0.0f), Max(0.0f, 0.0f)  {}
    constexpr ImRect(const ImVec2& min, const ImVec2& max)    : Min(min), Max(max)                {}
    constexpr ImRect(const ImVec4& v)                         : Min(v.x, v.y), Max(v.z, v.w)      {}
    constexpr ImRect(float x1, float y1, float x2, float y2)  : Min(x1, y1), Max(x2, y2)          {}

    ImVec2      GetCenter() const                   { return ImVec2((Min.x + Max.x) * 0.5f, (Min.y + Max.y) * 0.5f); }
    ImVec2      GetSize() const                     { return ImVec2(Max.x - Min.x, Max.y - Min.y); }
    float       GetWidth() const                    { return Max.x - Min.x; }
    float       GetHeight() const                   { return Max.y - Min.y; }
    float       GetArea() const                     { return (Max.x - Min.x) * (Max.y - Min.y); }
    ImVec2      GetTL() const                       { return Min; }
    ImVec2      GetTR() const                       { return ImVec2(Max.x, Min.y); }
    ImVec2      GetBL() const                       { return ImVec2(Min.x, Max.y); }
    ImVec2      GetBR() const                       { return Max; }
    bool        Contains(const ImVec2& p) const     { return p.x     >= Min.x && p.y     >= Min.y && p.x     <  Max.x && p.y     <  Max.y; }
    bool        Contains(const ImRect& r) const     { return r.Min.x >= Min.x && r.Min.y >= Min.y && r.Max.x <= Max.x && r.Max.y <= Max.y; }
    bool        ContainsWithPad(const ImVec2& p, const ImVec2& pad) const { return p.x >= Min.x - pad.x && p.y >= Min.y - pad.y && p.x < Max.x + pad.x && p.y < Max.y + pad.y; }
    bool        Overlaps(const ImRect& r) const     { return r.Min.y <  Max.y && r.Max.y >  Min.y && r.Min.x <  Max.x && r.Max.x >  Min.x; }
    void        Add(const ImVec2& p)                { if (Min.x > p.x)     Min.x = p.x;     if (Min.y > p.y)     Min.y = p.y;     if (Max.x < p.x)     Max.x = p.x;     if (Max.y < p.y)     Max.y = p.y; }
    void        Add(const ImRect& r)                { if (Min.x > r.Min.x) Min.x = r.Min.x; if (Min.y > r.Min.y) Min.y = r.Min.y; if (Max.x < r.Max.x) Max.x = r.Max.x; if (Max.y < r.Max.y) Max.y = r.Max.y; }
    void        Expand(const float amount)          { Min.x -= amount;   Min.y -= amount;   Max.x += amount;   Max.y += amount; }
    void        Expand(const ImVec2& amount)        { Min.x -= amount.x; Min.y -= amount.y; Max.x += amount.x; Max.y += amount.y; }
    void        Translate(const ImVec2& d)          { Min.x += d.x; Min.y += d.y; Max.x += d.x; Max.y += d.y; }
    void        TranslateX(float dx)                { Min.x += dx; Max.x += dx; }
    void        TranslateY(float dy)                { Min.y += dy; Max.y += dy; }
    void        ClipWith(const ImRect& r)           { Min = ImMax(Min, r.Min); Max = ImMin(Max, r.Max); }
    void        ClipWithFull(const ImRect& r)       { Min = ImClamp(Min, r.Min, r.Max); Max = ImClamp(Max, r.Min, r.Max); }
    bool        IsInverted() const                  { return Min.x > Max.x || Min.y > Max.y; }
    ImVec4      ToVec4() const                      { return ImVec4(Min.x, Min.y, Max.x, Max.y); }
    const ImVec4& AsVec4() const                    { return *(const ImVec4*)&Min.x; }
};

#define         IM_BITARRAY_TESTBIT(_ARRAY, _N)                 ((_ARRAY[(_N) >> 5] & ((ImU32)1 << ((_N) & 31))) != 0)
#define         IM_BITARRAY_CLEARBIT(_ARRAY, _N)                ((_ARRAY[(_N) >> 5] &= ~((ImU32)1 << ((_N) & 31))))
inline size_t   ImBitArrayGetStorageSizeInBytes(int bitcount)   { return (size_t)((bitcount + 31) >> 5) << 2; }
inline void     ImBitArrayClearAllBits(ImU32* arr, int bitcount){ memset(arr, 0, ImBitArrayGetStorageSizeInBytes(bitcount)); }
inline bool     ImBitArrayTestBit(const ImU32* arr, int n)      { ImU32 mask = (ImU32)1 << (n & 31); return (arr[n >> 5] & mask) != 0; }
inline void     ImBitArrayClearBit(ImU32* arr, int n)           { ImU32 mask = (ImU32)1 << (n & 31); arr[n >> 5] &= ~mask; }
inline void     ImBitArraySetBit(ImU32* arr, int n)             { ImU32 mask = (ImU32)1 << (n & 31); arr[n >> 5] |= mask; }
inline void     ImBitArraySetBitRange(ImU32* arr, int n, int n2)
{
    n2--;
    while (n <= n2)
    {
        int a_mod = (n & 31);
        int b_mod = (n2 > (n | 31) ? 31 : (n2 & 31)) + 1;
        ImU32 mask = (ImU32)(((ImU64)1 << b_mod) - 1) & ~(ImU32)(((ImU64)1 << a_mod) - 1);
        arr[n >> 5] |= mask;
        n = (n + 32) & ~31;
    }
}

typedef ImU32* ImBitArrayPtr;

template<int BITCOUNT, int OFFSET = 0>
struct ImBitArray
{
    ImU32           Data[(BITCOUNT + 31) >> 5];
    ImBitArray()                                { ClearAllBits(); }
    void            ClearAllBits()              { memset(Data, 0, sizeof(Data)); }
    void            SetAllBits()                { memset(Data, 255, sizeof(Data)); }
    bool            TestBit(int n) const        { n += OFFSET; IM_ASSERT(n >= 0 && n < BITCOUNT); return IM_BITARRAY_TESTBIT(Data, n); }
    void            SetBit(int n)               { n += OFFSET; IM_ASSERT(n >= 0 && n < BITCOUNT); ImBitArraySetBit(Data, n); }
    void            ClearBit(int n)             { n += OFFSET; IM_ASSERT(n >= 0 && n < BITCOUNT); ImBitArrayClearBit(Data, n); }
    void            SetBitRange(int n, int n2)  { n += OFFSET; n2 += OFFSET; IM_ASSERT(n >= 0 && n < BITCOUNT && n2 > n && n2 <= BITCOUNT); ImBitArraySetBitRange(Data, n, n2); }
    bool            operator[](int n) const     { n += OFFSET; IM_ASSERT(n >= 0 && n < BITCOUNT); return IM_BITARRAY_TESTBIT(Data, n); }
};

struct IMGUI_API ImBitVector
{
    ImVector<ImU32> Storage;
    void            Create(int sz)              { Storage.resize((sz + 31) >> 5); memset(Storage.Data, 0, (size_t)Storage.Size * sizeof(Storage.Data[0])); }
    void            Clear()                     { Storage.clear(); }
    bool            TestBit(int n) const        { IM_ASSERT(n < (Storage.Size << 5)); return IM_BITARRAY_TESTBIT(Storage.Data, n); }
    void            SetBit(int n)               { IM_ASSERT(n < (Storage.Size << 5)); ImBitArraySetBit(Storage.Data, n); }
    void            ClearBit(int n)             { IM_ASSERT(n < (Storage.Size << 5)); ImBitArrayClearBit(Storage.Data, n); }
};
IM_MSVC_RUNTIME_CHECKS_RESTORE

template<typename T>
struct ImSpan
{
    T*                  Data;
    T*                  DataEnd;

    inline ImSpan()                                 { Data = DataEnd = NULL; }
    inline ImSpan(T* data, int size)                { Data = data; DataEnd = data + size; }
    inline ImSpan(T* data, T* data_end)             { Data = data; DataEnd = data_end; }

    inline void         set(T* data, int size)      { Data = data; DataEnd = data + size; }
    inline void         set(T* data, T* data_end)   { Data = data; DataEnd = data_end; }
    inline int          size() const                { return (int)(ptrdiff_t)(DataEnd - Data); }
    inline int          size_in_bytes() const       { return (int)(ptrdiff_t)(DataEnd - Data) * (int)sizeof(T); }
    inline T&           operator[](int i)           { T* p = Data + i; IM_ASSERT(p >= Data && p < DataEnd); return *p; }
    inline const T&     operator[](int i) const     { const T* p = Data + i; IM_ASSERT(p >= Data && p < DataEnd); return *p; }

    inline T*           begin()                     { return Data; }
    inline const T*     begin() const               { return Data; }
    inline T*           end()                       { return DataEnd; }
    inline const T*     end() const                 { return DataEnd; }

    inline int  index_from_ptr(const T* it) const   { IM_ASSERT(it >= Data && it < DataEnd); const ptrdiff_t off = it - Data; return (int)off; }
};

template<int CHUNKS>
struct ImSpanAllocator
{
    char*   BasePtr;
    int     CurrOff;
    int     CurrIdx;
    int     Offsets[CHUNKS];
    int     Sizes[CHUNKS];

    ImSpanAllocator()                               { memset((void*)this, 0, sizeof(*this)); }
    inline void  Reserve(int n, size_t sz, int a=4) { IM_ASSERT(n == CurrIdx && n < CHUNKS); CurrOff = IM_MEMALIGN(CurrOff, a); Offsets[n] = CurrOff; Sizes[n] = (int)sz; CurrIdx++; CurrOff += (int)sz; }
    inline int   GetArenaSizeInBytes()              { return CurrOff; }
    inline void  SetArenaBasePtr(void* base_ptr)    { BasePtr = (char*)base_ptr; }
    inline void* GetSpanPtrBegin(int n)             { IM_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS); return (void*)(BasePtr + Offsets[n]); }
    inline void* GetSpanPtrEnd(int n)               { IM_ASSERT(n >= 0 && n < CHUNKS && CurrIdx == CHUNKS); return (void*)(BasePtr + Offsets[n] + Sizes[n]); }
    template<typename T>
    inline void  GetSpan(int n, ImSpan<T>* span)    { span->set((T*)GetSpanPtrBegin(n), (T*)GetSpanPtrEnd(n)); }
};

template<typename T, int BLOCKSIZE>
struct ImStableVector
{
    int                 Size = 0;
    int                 Capacity = 0;
    ImVector<T*>        Blocks;

    inline ~ImStableVector()                        { for (T* block : Blocks) IM_FREE(block); }

    inline void         clear()                     { Size = Capacity = 0; Blocks.clear_delete(); }
    inline void         resize(int new_size)        { if (new_size > Capacity) reserve(new_size); Size = new_size; }
    inline void         reserve(int new_cap)
    {
        new_cap = IM_MEMALIGN(new_cap, BLOCKSIZE);
        int old_count = Capacity / BLOCKSIZE;
        int new_count = new_cap / BLOCKSIZE;
        if (new_count <= old_count)
            return;
        Blocks.resize(new_count);
        for (int n = old_count; n < new_count; n++)
            Blocks[n] = (T*)IM_ALLOC(sizeof(T) * BLOCKSIZE);
        Capacity = new_cap;
    }
    inline T&           operator[](int i)           { IM_ASSERT(i >= 0 && i < Size); return Blocks[i / BLOCKSIZE][i % BLOCKSIZE]; }
    inline const T&     operator[](int i) const     { IM_ASSERT(i >= 0 && i < Size); return Blocks[i / BLOCKSIZE][i % BLOCKSIZE]; }
    inline T*           push_back(const T& v)       { int i = Size; IM_ASSERT(i >= 0); if (Size == Capacity) reserve(Capacity + BLOCKSIZE); void* ptr = &Blocks[i / BLOCKSIZE][i % BLOCKSIZE]; memcpy(ptr, &v, sizeof(v)); Size++; return (T*)ptr; }
};

typedef int ImPoolIdx;
template<typename T>
struct ImPool
{
    ImVector<T>     Buf;
    ImGuiStorage    Map;
    ImPoolIdx       FreeIdx;
    ImPoolIdx       AliveCount;

    ImPool()    { FreeIdx = AliveCount = 0; }
    ~ImPool()   { Clear(); }
    T*          GetByKey(ImGuiID key)               { int idx = Map.GetInt(key, -1); return (idx != -1) ? &Buf[idx] : NULL; }
    T*          GetByIndex(ImPoolIdx n)             { return &Buf[n]; }
    ImPoolIdx   GetIndex(const T* p) const          { IM_ASSERT(p >= Buf.Data && p < Buf.Data + Buf.Size); return (ImPoolIdx)(p - Buf.Data); }
    T*          GetOrAddByKey(ImGuiID key)          { int* p_idx = Map.GetIntRef(key, -1); if (*p_idx != -1) return &Buf[*p_idx]; *p_idx = FreeIdx; return Add(); }
    bool        Contains(const T* p) const          { return (p >= Buf.Data && p < Buf.Data + Buf.Size); }
    void        Clear()                             { for (int n = 0; n < Map.Data.Size; n++) { int idx = Map.Data[n].val_i; if (idx != -1) Buf[idx].~T(); } Map.Clear(); Buf.clear(); FreeIdx = AliveCount = 0; }
    T*          Add()                               { int idx = FreeIdx; if (idx == Buf.Size) { Buf.resize(Buf.Size + 1); FreeIdx++; } else { FreeIdx = *(int*)&Buf[idx]; } IM_PLACEMENT_NEW(&Buf[idx]) T(); AliveCount++; return &Buf[idx]; }
    void        Remove(ImGuiID key, const T* p)     { Remove(key, GetIndex(p)); }
    void        Remove(ImGuiID key, ImPoolIdx idx)  { Buf[idx].~T(); *(int*)&Buf[idx] = FreeIdx; FreeIdx = idx; Map.SetInt(key, -1); AliveCount--; }
    void        Reserve(int capacity)               { Buf.reserve(capacity); Map.Data.reserve(capacity); }

    int         GetAliveCount() const               { return AliveCount; }
    int         GetBufSize() const                  { return Buf.Size; }
    int         GetMapSize() const                  { return Map.Data.Size; }
    T*          TryGetMapData(ImPoolIdx n)          { int idx = Map.Data[n].val_i; if (idx == -1) return NULL; return GetByIndex(idx); }
};

template<typename T>
struct ImChunkStream
{
    ImVector<char>  Buf;

    void    clear()                     { Buf.clear(); }
    bool    empty() const               { return Buf.Size == 0; }
    int     size() const                { return Buf.Size; }
    T*      alloc_chunk(size_t sz)      { size_t HDR_SZ = 4; sz = IM_MEMALIGN(HDR_SZ + sz, 4u); int off = Buf.Size; Buf.resize(off + (int)sz); ((int*)(void*)(Buf.Data + off))[0] = (int)sz; return (T*)(void*)(Buf.Data + off + (int)HDR_SZ); }
    T*      begin()                     { size_t HDR_SZ = 4; if (!Buf.Data) return NULL; return (T*)(void*)(Buf.Data + HDR_SZ); }
    T*      next_chunk(T* p)            { size_t HDR_SZ = 4; IM_ASSERT(p >= begin() && p < end()); p = (T*)(void*)((char*)(void*)p + chunk_size(p)); if (p == (T*)(void*)((char*)end() + HDR_SZ)) return (T*)0; IM_ASSERT(p < end()); return p; }
    int     chunk_size(const T* p)      { return ((const int*)p)[-1]; }
    T*      end()                       { return (T*)(void*)(Buf.Data + Buf.Size); }
    int     offset_from_ptr(const T* p) { IM_ASSERT(p >= begin() && p < end()); const ptrdiff_t off = (const char*)p - Buf.Data; return (int)off; }
    T*      ptr_from_offset(int off)    { IM_ASSERT(off >= 4 && off < Buf.Size); return (T*)(void*)(Buf.Data + off); }
    void    swap(ImChunkStream<T>& rhs) { rhs.Buf.swap(Buf); }
};

struct ImGuiTextIndex
{
    ImVector<int>   Offsets;
    int             EndOffset = 0;

    void            clear()                                 { Offsets.clear(); EndOffset = 0; }
    int             size()                                  { return Offsets.Size; }
    const char*     get_line_begin(const char* base, int n) { return base + (Offsets.Size != 0 ? Offsets[n] : 0); }
    const char*     get_line_end(const char* base, int n)   { return base + (n + 1 < Offsets.Size ? (Offsets[n + 1] - 1) : EndOffset); }
    void            append(const char* base, int old_size, int new_size);
};

IMGUI_API ImGuiStoragePair* ImLowerBound(ImGuiStoragePair* in_begin, ImGuiStoragePair* in_end, ImGuiID key);

#define IM_ROUNDUP_TO_EVEN(_V)                                  ((((_V) + 1) / 2) * 2)
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN                     4
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX                     512
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD,_MAXERROR)    ImClamp(IM_ROUNDUP_TO_EVEN((int)ImCeil(IM_PI / ImAcos(1 - ImMin((_MAXERROR), (_RAD)) / (_RAD)))), IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)

#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(_N,_MAXERROR)    ((_MAXERROR) / (1 - ImCos(IM_PI / ImMax((float)(_N), IM_PI))))
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_ERROR(_N,_RAD)     ((1 - ImCos(IM_PI / ImMax((float)(_N), IM_PI))) / (_RAD))

#ifndef IM_DRAWLIST_ARCFAST_TABLE_SIZE
#define IM_DRAWLIST_ARCFAST_TABLE_SIZE                          48
#endif
#define IM_DRAWLIST_ARCFAST_SAMPLE_MAX                          IM_DRAWLIST_ARCFAST_TABLE_SIZE

struct IMGUI_API ImDrawListSharedData
{
    ImVec2          TexUvWhitePixel;
    const ImVec4*   TexUvLines;
    ImFontAtlas*    FontAtlas;
    ImFont*         Font;
    float           FontSize;
    float           FontScale;
    float           CurveTessellationTol;
    float           CircleSegmentMaxError;
    float           InitialFringeScale;
    ImDrawListFlags InitialFlags;
    ImVec4          ClipRectFullscreen;
    ImVector<ImVec2> TempBuffer;
    ImVector<ImDrawList*> DrawLists;
    ImGuiContext*   Context;

    ImVec2          ArcFastVtx[IM_DRAWLIST_ARCFAST_TABLE_SIZE];
    float           ArcFastRadiusCutoff;
    ImU8            CircleSegmentCounts[64];

    ImDrawListSharedData();
    ~ImDrawListSharedData();
    void SetCircleTessellationMaxError(float max_error);
};

struct ImDrawDataBuilder
{
    ImVector<ImDrawList*>*  Layers[2];
    ImVector<ImDrawList*>   LayerData1;

    ImDrawDataBuilder()                     { memset((void*)this, 0, sizeof(*this)); }
};

struct ImFontStackData
{
    ImFont*     Font;
    float       FontSizeBeforeScaling;
    float       FontSizeAfterScaling;
};

struct ImGuiStyleVarInfo
{
    ImU32           Count : 8;
    ImGuiDataType   DataType : 8;
    ImU32           Offset : 16;
    void* GetVarPtr(void* parent) const { return (void*)((unsigned char*)parent + Offset); }
};

struct ImGuiColorMod
{
    ImGuiCol        Col;
    ImVec4          BackupValue;
};

struct ImGuiStyleMod
{
    ImGuiStyleVar   VarIdx;
    union           { int BackupInt[2]; float BackupFloat[2]; };
    ImGuiStyleMod(ImGuiStyleVar idx, int v)     { VarIdx = idx; BackupInt[0] = v; }
    ImGuiStyleMod(ImGuiStyleVar idx, float v)   { VarIdx = idx; BackupFloat[0] = v; }
    ImGuiStyleMod(ImGuiStyleVar idx, ImVec2 v)  { VarIdx = idx; BackupFloat[0] = v.x; BackupFloat[1] = v.y; }
};

struct ImGuiDataTypeStorage
{
    ImU8        Data[8];
};

struct ImGuiDataTypeInfo
{
    size_t      Size;
    const char* Name;
    const char* PrintFmt;
    const char* ScanFmt;
};

enum ImGuiDataTypePrivate_
{
    ImGuiDataType_Pointer = ImGuiDataType_COUNT,
    ImGuiDataType_ID,
};

enum ImGuiItemFlagsPrivate_
{

    ImGuiItemFlags_ReadOnly                 = 1 << 11,
    ImGuiItemFlags_MixedValue               = 1 << 12,
    ImGuiItemFlags_NoWindowHoverableCheck   = 1 << 13,
    ImGuiItemFlags_AllowOverlap             = 1 << 14,
    ImGuiItemFlags_NoNavDisableMouseHover   = 1 << 15,
    ImGuiItemFlags_NoMarkEdited             = 1 << 16,
    ImGuiItemFlags_NoFocus                  = 1 << 17,

    ImGuiItemFlags_Inputable                = 1 << 20,
    ImGuiItemFlags_HasSelectionUserData     = 1 << 21,
    ImGuiItemFlags_IsMultiSelect            = 1 << 22,

    ImGuiItemFlags_Default_                 = ImGuiItemFlags_AutoClosePopups,

};

enum ImGuiItemStatusFlags_
{
    ImGuiItemStatusFlags_None               = 0,
    ImGuiItemStatusFlags_HoveredRect        = 1 << 0,
    ImGuiItemStatusFlags_HasDisplayRect     = 1 << 1,
    ImGuiItemStatusFlags_Edited             = 1 << 2,
    ImGuiItemStatusFlags_ToggledSelection   = 1 << 3,
    ImGuiItemStatusFlags_ToggledOpen        = 1 << 4,
    ImGuiItemStatusFlags_HasDeactivated     = 1 << 5,
    ImGuiItemStatusFlags_Deactivated        = 1 << 6,
    ImGuiItemStatusFlags_HoveredWindow      = 1 << 7,
    ImGuiItemStatusFlags_Visible            = 1 << 8,
    ImGuiItemStatusFlags_HasClipRect        = 1 << 9,
    ImGuiItemStatusFlags_HasShortcut        = 1 << 10,

#ifdef IMGUI_ENABLE_TEST_ENGINE
    ImGuiItemStatusFlags_Openable           = 1 << 20,
    ImGuiItemStatusFlags_Opened             = 1 << 21,
    ImGuiItemStatusFlags_Checkable          = 1 << 22,
    ImGuiItemStatusFlags_Checked            = 1 << 23,
    ImGuiItemStatusFlags_Inputable          = 1 << 24,
#endif
};

enum ImGuiHoveredFlagsPrivate_
{
    ImGuiHoveredFlags_DelayMask_                    = ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay,
    ImGuiHoveredFlags_AllowedMaskForIsWindowHovered = ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_NoPopupHierarchy | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_Stationary,
    ImGuiHoveredFlags_AllowedMaskForIsItemHovered   = ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped | ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_NoNavOverride | ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayMask_,
};

enum ImGuiInputTextFlagsPrivate_
{

    ImGuiInputTextFlags_Multiline           = 1 << 26,
    ImGuiInputTextFlags_TempInput           = 1 << 27,
    ImGuiInputTextFlags_LocalizeDecimalPoint= 1 << 28,
};

enum ImGuiButtonFlagsPrivate_
{
    ImGuiButtonFlags_PressedOnClick         = 1 << 4,
    ImGuiButtonFlags_PressedOnClickRelease  = 1 << 5,
    ImGuiButtonFlags_PressedOnClickReleaseAnywhere = 1 << 6,
    ImGuiButtonFlags_PressedOnRelease       = 1 << 7,
    ImGuiButtonFlags_PressedOnDoubleClick   = 1 << 8,
    ImGuiButtonFlags_PressedOnDragDropHold  = 1 << 9,

    ImGuiButtonFlags_FlattenChildren        = 1 << 11,

    ImGuiButtonFlags_AlignTextBaseLine      = 1 << 15,
    ImGuiButtonFlags_NoKeyModsAllowed       = 1 << 16,
    ImGuiButtonFlags_NoHoldingActiveId      = 1 << 17,
    ImGuiButtonFlags_NoNavFocus             = 1 << 18,
    ImGuiButtonFlags_NoHoveredOnFocus       = 1 << 19,
    ImGuiButtonFlags_NoSetKeyOwner          = 1 << 20,
    ImGuiButtonFlags_NoTestKeyOwner         = 1 << 21,
    ImGuiButtonFlags_NoFocus                = 1 << 22,
    ImGuiButtonFlags_PressedOnMask_         = ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnClickReleaseAnywhere | ImGuiButtonFlags_PressedOnRelease | ImGuiButtonFlags_PressedOnDoubleClick | ImGuiButtonFlags_PressedOnDragDropHold,
    ImGuiButtonFlags_PressedOnDefault_      = ImGuiButtonFlags_PressedOnClickRelease,

};

enum ImGuiComboFlagsPrivate_
{
    ImGuiComboFlags_CustomPreview           = 1 << 20,
};

enum ImGuiSliderFlagsPrivate_
{
    ImGuiSliderFlags_Vertical               = 1 << 20,
    ImGuiSliderFlags_ReadOnly               = 1 << 21,
};

enum ImGuiSelectableFlagsPrivate_
{

    ImGuiSelectableFlags_NoHoldingActiveID      = 1 << 20,
    ImGuiSelectableFlags_SelectOnClick          = 1 << 22,
    ImGuiSelectableFlags_SelectOnRelease        = 1 << 23,
    ImGuiSelectableFlags_SpanAvailWidth         = 1 << 24,
    ImGuiSelectableFlags_SetNavIdOnHover        = 1 << 25,
    ImGuiSelectableFlags_NoPadWithHalfSpacing   = 1 << 26,
    ImGuiSelectableFlags_NoSetKeyOwner          = 1 << 27,
};

enum ImGuiTreeNodeFlagsPrivate_
{
    ImGuiTreeNodeFlags_NoNavFocus                 = 1 << 27,
    ImGuiTreeNodeFlags_ClipLabelForTrailingButton = 1 << 28,
    ImGuiTreeNodeFlags_UpsideDownArrow            = 1 << 29,
    ImGuiTreeNodeFlags_OpenOnMask_                = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow,
    ImGuiTreeNodeFlags_DrawLinesMask_             = ImGuiTreeNodeFlags_DrawLinesNone | ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_DrawLinesToNodes,
};

enum ImGuiSeparatorFlags_
{
    ImGuiSeparatorFlags_None                    = 0,
    ImGuiSeparatorFlags_Horizontal              = 1 << 0,
    ImGuiSeparatorFlags_Vertical                = 1 << 1,
    ImGuiSeparatorFlags_SpanAllColumns          = 1 << 2,
};

enum ImGuiFocusRequestFlags_
{
    ImGuiFocusRequestFlags_None                 = 0,
    ImGuiFocusRequestFlags_RestoreFocusedChild  = 1 << 0,
    ImGuiFocusRequestFlags_UnlessBelowModal     = 1 << 1,
};

enum ImGuiTextFlags_
{
    ImGuiTextFlags_None                         = 0,
    ImGuiTextFlags_NoWidthForLargeClippedText   = 1 << 0,
};

enum ImGuiTooltipFlags_
{
    ImGuiTooltipFlags_None                      = 0,
    ImGuiTooltipFlags_OverridePrevious          = 1 << 1,
};

enum ImGuiLayoutType_
{
    ImGuiLayoutType_Horizontal = 0,
    ImGuiLayoutType_Vertical = 1
};

enum ImGuiLogFlags_
{
    ImGuiLogFlags_None = 0,

    ImGuiLogFlags_OutputTTY         = 1 << 0,
    ImGuiLogFlags_OutputFile        = 1 << 1,
    ImGuiLogFlags_OutputBuffer      = 1 << 2,
    ImGuiLogFlags_OutputClipboard   = 1 << 3,
    ImGuiLogFlags_OutputMask_       = ImGuiLogFlags_OutputTTY | ImGuiLogFlags_OutputFile | ImGuiLogFlags_OutputBuffer | ImGuiLogFlags_OutputClipboard,
};

enum ImGuiAxis
{
    ImGuiAxis_None = -1,
    ImGuiAxis_X = 0,
    ImGuiAxis_Y = 1
};

enum ImGuiPlotType
{
    ImGuiPlotType_Lines,
    ImGuiPlotType_Histogram,
};

struct IMGUI_API ImGuiComboPreviewData
{
    ImRect          PreviewRect;
    ImVec2          BackupCursorPos;
    ImVec2          BackupCursorMaxPos;
    ImVec2          BackupCursorPosPrevLine;
    float           BackupPrevLineTextBaseOffset;
    ImGuiLayoutType BackupLayout;

    ImGuiComboPreviewData() { memset((void*)this, 0, sizeof(*this)); }
};

struct IMGUI_API ImGuiGroupData
{
    ImGuiID     WindowID;
    ImVec2      BackupCursorPos;
    ImVec2      BackupCursorMaxPos;
    ImVec2      BackupCursorPosPrevLine;
    ImVec1      BackupIndent;
    ImVec1      BackupGroupOffset;
    ImVec2      BackupCurrLineSize;
    float       BackupCurrLineTextBaseOffset;
    ImGuiID     BackupActiveIdIsAlive;
    bool        BackupActiveIdHasBeenEditedThisFrame;
    bool        BackupDeactivatedIdIsAlive;
    bool        BackupHoveredIdIsAlive;
    bool        BackupIsSameLine;
    bool        EmitItem;
};

struct IMGUI_API ImGuiMenuColumns
{
    ImU32       TotalWidth;
    ImU32       NextTotalWidth;
    ImU16       Spacing;
    ImU16       OffsetIcon;
    ImU16       OffsetLabel;
    ImU16       OffsetShortcut;
    ImU16       OffsetMark;
    ImU16       Widths[4];

    ImGuiMenuColumns() { memset((void*)this, 0, sizeof(*this)); }
    void        Update(float spacing, bool window_reappearing);
    float       DeclColumns(float w_icon, float w_label, float w_shortcut, float w_mark);
    void        CalcNextTotalWidth(bool update_offsets);
};

struct IMGUI_API ImGuiInputTextDeactivatedState
{
    ImGuiID            ID;
    ImVector<char>     TextA;

    ImGuiInputTextDeactivatedState()    { memset((void*)this, 0, sizeof(*this)); }
    void    ClearFreeMemory()           { ID = 0; TextA.clear(); }
};

#undef IMSTB_TEXTEDIT_STRING
#undef IMSTB_TEXTEDIT_CHARTYPE
#define IMSTB_TEXTEDIT_STRING             ImGuiInputTextState
#define IMSTB_TEXTEDIT_CHARTYPE           char
#define IMSTB_TEXTEDIT_GETWIDTH_NEWLINE   (-1.0f)
#define IMSTB_TEXTEDIT_UNDOSTATECOUNT     99
#define IMSTB_TEXTEDIT_UNDOCHARCOUNT      999
namespace ImStb { struct STB_TexteditState; }
typedef ImStb::STB_TexteditState ImStbTexteditState;

struct IMGUI_API ImGuiInputTextState
{
    ImGuiContext*           Ctx;
    ImStbTexteditState*     Stb;
    ImGuiInputTextFlags     Flags;
    ImGuiID                 ID;
    int                     TextLen;
    const char*             TextSrc;
    ImVector<char>          TextA;
    ImVector<char>          TextToRevertTo;
    ImVector<char>          CallbackTextBackup;
    int                     BufCapacity;
    ImVec2                  Scroll;
    int                     LineCount;
    float                   WrapWidth;
    float                   CursorAnim;
    bool                    CursorFollow;
    bool                    CursorCenterY;
    bool                    SelectedAllMouseLock;
    bool                    EditedBefore;
    bool                    EditedThisFrame;
    bool                    WantReloadUserBuf;
    ImS8                    LastMoveDirectionLR;
    int                     ReloadSelectionStart;
    int                     ReloadSelectionEnd;

    ImGuiInputTextState();
    ~ImGuiInputTextState();
    void        ClearText()                 { TextLen = 0; TextA[0] = 0; CursorClamp(); }
    void        ClearFreeMemory()           { TextA.clear(); TextToRevertTo.clear(); }
    void        OnKeyPressed(int key);
    void        OnCharPressed(unsigned int c);
    float       GetPreferredOffsetX() const;
    const char* GetText()                   { return TextA.Data ? TextA.Data : ""; }

    void        CursorAnimReset();
    void        CursorClamp();
    bool        HasSelection() const;
    void        ClearSelection();
    int         GetCursorPos() const;
    int         GetSelectionStart() const;
    int         GetSelectionEnd() const;
    void        SetSelection(int start, int end);
    void        SelectAll();

    void        ReloadUserBufAndSelectAll();
    void        ReloadUserBufAndKeepSelection();
    void        ReloadUserBufAndMoveToEnd();
};

enum ImGuiWindowRefreshFlags_
{
    ImGuiWindowRefreshFlags_None                = 0,
    ImGuiWindowRefreshFlags_TryToAvoidRefresh   = 1 << 0,
    ImGuiWindowRefreshFlags_RefreshOnHover      = 1 << 1,
    ImGuiWindowRefreshFlags_RefreshOnFocus      = 1 << 2,

};

enum ImGuiWindowBgClickFlags_
{
    ImGuiWindowBgClickFlags_None                = 0,
    ImGuiWindowBgClickFlags_Move                = 1 << 0,
};

enum ImGuiNextWindowDataFlags_
{
    ImGuiNextWindowDataFlags_None               = 0,
    ImGuiNextWindowDataFlags_HasPos             = 1 << 0,
    ImGuiNextWindowDataFlags_HasSize            = 1 << 1,
    ImGuiNextWindowDataFlags_HasContentSize     = 1 << 2,
    ImGuiNextWindowDataFlags_HasCollapsed       = 1 << 3,
    ImGuiNextWindowDataFlags_HasSizeConstraint  = 1 << 4,
    ImGuiNextWindowDataFlags_HasFocus           = 1 << 5,
    ImGuiNextWindowDataFlags_HasBgAlpha         = 1 << 6,
    ImGuiNextWindowDataFlags_HasScroll          = 1 << 7,
    ImGuiNextWindowDataFlags_HasWindowFlags     = 1 << 8,
    ImGuiNextWindowDataFlags_HasChildFlags      = 1 << 9,
    ImGuiNextWindowDataFlags_HasRefreshPolicy   = 1 << 10,
};

struct ImGuiNextWindowData
{
    ImGuiNextWindowDataFlags    HasFlags;

    ImGuiCond                   PosCond;
    ImGuiCond                   SizeCond;
    ImGuiCond                   CollapsedCond;
    ImVec2                      PosVal;
    ImVec2                      PosPivotVal;
    ImVec2                      SizeVal;
    ImVec2                      ContentSizeVal;
    ImVec2                      ScrollVal;
    ImGuiWindowFlags            WindowFlags;
    ImGuiChildFlags             ChildFlags;
    bool                        CollapsedVal;
    ImRect                      SizeConstraintRect;
    ImGuiSizeCallback           SizeCallback;
    void*                       SizeCallbackUserData;
    float                       BgAlphaVal;
    ImVec2                      MenuBarOffsetMinVal;
    ImGuiWindowRefreshFlags     RefreshFlagsVal;

    ImGuiNextWindowData()       { memset((void*)this, 0, sizeof(*this)); }
    inline void ClearFlags()    { HasFlags = ImGuiNextWindowDataFlags_None; }
};

enum ImGuiNextItemDataFlags_
{
    ImGuiNextItemDataFlags_None             = 0,
    ImGuiNextItemDataFlags_HasWidth         = 1 << 0,
    ImGuiNextItemDataFlags_HasOpen          = 1 << 1,
    ImGuiNextItemDataFlags_HasShortcut      = 1 << 2,
    ImGuiNextItemDataFlags_HasRefVal        = 1 << 3,
    ImGuiNextItemDataFlags_HasStorageID     = 1 << 4,
    ImGuiNextItemDataFlags_HasColorMarker   = 1 << 5,
};

struct ImGuiNextItemData
{
    ImGuiNextItemDataFlags      HasFlags;
    ImGuiItemFlags              ItemFlags;

    ImGuiID                     FocusScopeId;
    ImGuiSelectionUserData      SelectionUserData;
    float                       Width;
    ImGuiKeyChord               Shortcut;
    ImGuiInputFlags             ShortcutFlags;
    bool                        OpenVal;
    ImU8                        OpenCond;
    ImGuiDataTypeStorage        RefVal;
    ImGuiID                     StorageId;
    ImU32                       ColorMarker;

    ImGuiNextItemData()         { memset((void*)this, 0, sizeof(*this)); SelectionUserData = -1; }
    inline void ClearFlags()    { HasFlags = ImGuiNextItemDataFlags_None; ItemFlags = ImGuiItemFlags_None; }
};

struct ImGuiLastItemData
{
    ImGuiID                 ID;
    ImGuiItemFlags          ItemFlags;
    ImGuiItemStatusFlags    StatusFlags;
    ImRect                  Rect;
    ImRect                  NavRect;

    ImRect                  DisplayRect;
    ImRect                  ClipRect;
    ImGuiKeyChord           Shortcut;

    ImGuiLastItemData()     { memset((void*)this, 0, sizeof(*this)); }
};

struct ImGuiTreeNodeStackData
{
    ImGuiID                 ID;
    ImGuiTreeNodeFlags      TreeFlags;
    ImGuiItemFlags          ItemFlags;
    ImRect                  NavRect;
    float                   DrawLinesX1;
    float                   DrawLinesToNodesY2;
    ImGuiTableColumnIdx     DrawLinesTableColumn;
};

struct IMGUI_API ImGuiErrorRecoveryState
{
    short   SizeOfWindowStack;
    short   SizeOfIDStack;
    short   SizeOfTreeStack;
    short   SizeOfColorStack;
    short   SizeOfStyleVarStack;
    short   SizeOfFontStack;
    short   SizeOfFocusScopeStack;
    short   SizeOfGroupStack;
    short   SizeOfItemFlagsStack;
    short   SizeOfBeginPopupStack;
    short   SizeOfDisabledStack;

    ImGuiErrorRecoveryState() { memset((void*)this, 0, sizeof(*this)); }
};

struct ImGuiWindowStackData
{
    ImGuiWindow*            Window;
    ImGuiLastItemData       ParentLastItemDataBackup;
    ImGuiErrorRecoveryState StackSizesInBegin;
    bool                    DisabledOverrideReenable;
    float                   DisabledOverrideReenableAlphaBackup;
};

struct ImGuiShrinkWidthItem
{
    int         Index;
    float       Width;
    float       InitialWidth;
};

struct ImGuiPtrOrIndex
{
    void*       Ptr;
    int         Index;

    ImGuiPtrOrIndex(void* ptr)  { Ptr = ptr; Index = -1; }
    ImGuiPtrOrIndex(int index)  { Ptr = NULL; Index = index; }
};

struct ImGuiDeactivatedItemData
{
    ImGuiID     ID;
    int         ElapseFrame;
    bool        HasBeenEditedBefore;
    bool        IsAlive;
};

enum ImGuiPopupPositionPolicy
{
    ImGuiPopupPositionPolicy_Default,
    ImGuiPopupPositionPolicy_ComboBox,
    ImGuiPopupPositionPolicy_Tooltip,
};

struct ImGuiPopupData
{
    ImGuiID             PopupId;
    ImGuiWindow*        Window;
    ImGuiWindow*        RestoreNavWindow;
    int                 ParentNavLayer;
    int                 OpenFrameCount;
    ImGuiID             OpenParentId;
    ImVec2              OpenPopupPos;
    ImVec2              OpenMousePos;

    ImGuiPopupData()    { memset((void*)this, 0, sizeof(*this)); ParentNavLayer = OpenFrameCount = -1; }
};

typedef ImBitArray<ImGuiKey_NamedKey_COUNT, -ImGuiKey_NamedKey_BEGIN>    ImBitArrayForNamedKeys;

#define ImGuiKey_LegacyNativeKey_BEGIN  0
#define ImGuiKey_LegacyNativeKey_END    512
#define ImGuiKey_Keyboard_BEGIN         (ImGuiKey_NamedKey_BEGIN)
#define ImGuiKey_Keyboard_END           (ImGuiKey_GamepadStart)
#define ImGuiKey_Gamepad_BEGIN          (ImGuiKey_GamepadStart)
#define ImGuiKey_Gamepad_END            (ImGuiKey_GamepadRStickDown + 1)
#define ImGuiKey_Mouse_BEGIN            (ImGuiKey_MouseLeft)
#define ImGuiKey_Mouse_END              (ImGuiKey_MouseWheelY + 1)
#define ImGuiKey_Aliases_BEGIN          (ImGuiKey_Mouse_BEGIN)
#define ImGuiKey_Aliases_END            (ImGuiKey_Mouse_END)

#define ImGuiKey_NavKeyboardTweakSlow   ImGuiMod_Ctrl
#define ImGuiKey_NavKeyboardTweakFast   ImGuiMod_Shift
#define ImGuiKey_NavGamepadTweakSlow    ImGuiKey_GamepadL1
#define ImGuiKey_NavGamepadTweakFast    ImGuiKey_GamepadR1
#define ImGuiKey_NavGamepadActivate     (g.IO.ConfigNavSwapGamepadButtons ? ImGuiKey_GamepadFaceRight : ImGuiKey_GamepadFaceDown)
#define ImGuiKey_NavGamepadCancel       (g.IO.ConfigNavSwapGamepadButtons ? ImGuiKey_GamepadFaceDown : ImGuiKey_GamepadFaceRight)
#define ImGuiKey_NavGamepadMenu         ImGuiKey_GamepadFaceLeft
#define ImGuiKey_NavGamepadContextMenu  ImGuiKey_GamepadFaceUp

enum ImGuiInputEventType
{
    ImGuiInputEventType_None = 0,
    ImGuiInputEventType_MousePos,
    ImGuiInputEventType_MouseWheel,
    ImGuiInputEventType_MouseButton,
    ImGuiInputEventType_Key,
    ImGuiInputEventType_Text,
    ImGuiInputEventType_Focus,
    ImGuiInputEventType_COUNT
};

enum ImGuiInputSource : int
{
    ImGuiInputSource_None = 0,
    ImGuiInputSource_Mouse,
    ImGuiInputSource_Keyboard,
    ImGuiInputSource_Gamepad,
    ImGuiInputSource_COUNT
};

struct ImGuiInputEventMousePos      { float PosX, PosY; ImGuiMouseSource MouseSource; };
struct ImGuiInputEventMouseWheel    { float WheelX, WheelY; ImGuiMouseSource MouseSource; };
struct ImGuiInputEventMouseButton   { int Button; bool Down; ImGuiMouseSource MouseSource; };
struct ImGuiInputEventKey           { ImGuiKey Key; bool Down; float AnalogValue; };
struct ImGuiInputEventText          { unsigned int Char; };
struct ImGuiInputEventAppFocused    { bool Focused; };

struct ImGuiInputEvent
{
    ImGuiInputEventType             Type;
    ImGuiInputSource                Source;
    ImU32                           EventId;
    union
    {
        ImGuiInputEventMousePos     MousePos;
        ImGuiInputEventMouseWheel   MouseWheel;
        ImGuiInputEventMouseButton  MouseButton;
        ImGuiInputEventKey          Key;
        ImGuiInputEventText         Text;
        ImGuiInputEventAppFocused   AppFocused;
    };
    bool                            AddedByTestEngine;

    ImGuiInputEvent() { memset((void*)this, 0, sizeof(*this)); }
};

#define ImGuiKeyOwner_Any           ((ImGuiID)0)
#define ImGuiKeyOwner_NoOwner       ((ImGuiID)-1)

typedef ImS16 ImGuiKeyRoutingIndex;

struct ImGuiKeyRoutingData
{
    ImGuiKeyRoutingIndex            NextEntryIndex;
    ImU16                           Mods;
    ImU16                           RoutingCurrScore;
    ImU16                           RoutingNextScore;
    ImGuiID                         RoutingCurr;
    ImGuiID                         RoutingNext;

    ImGuiKeyRoutingData()           { NextEntryIndex = -1; Mods = 0; RoutingCurrScore = RoutingNextScore = 0; RoutingCurr = RoutingNext = ImGuiKeyOwner_NoOwner; }
};

struct ImGuiKeyRoutingTable
{
    ImGuiKeyRoutingIndex            Index[ImGuiKey_NamedKey_COUNT];
    ImVector<ImGuiKeyRoutingData>   Entries;
    ImVector<ImGuiKeyRoutingData>   EntriesNext;

    ImGuiKeyRoutingTable()          { Clear(); }
    void Clear()                    { for (int n = 0; n < IM_COUNTOF(Index); n++) Index[n] = -1; Entries.clear(); EntriesNext.clear(); }
};

struct ImGuiKeyOwnerData
{
    ImGuiID     OwnerCurr;
    ImGuiID     OwnerNext;
    bool        LockThisFrame;
    bool        LockUntilRelease;

    ImGuiKeyOwnerData()             { OwnerCurr = OwnerNext = ImGuiKeyOwner_NoOwner; LockThisFrame = LockUntilRelease = false; }
};

enum ImGuiInputFlagsPrivate_
{

    ImGuiInputFlags_RepeatRateDefault           = 1 << 1,
    ImGuiInputFlags_RepeatRateNavMove           = 1 << 2,
    ImGuiInputFlags_RepeatRateNavTweak          = 1 << 3,

    ImGuiInputFlags_RepeatUntilRelease          = 1 << 4,
    ImGuiInputFlags_RepeatUntilKeyModsChange    = 1 << 5,
    ImGuiInputFlags_RepeatUntilKeyModsChangeFromNone = 1 << 6,
    ImGuiInputFlags_RepeatUntilOtherKeyPress    = 1 << 7,

    ImGuiInputFlags_LockThisFrame               = 1 << 20,
    ImGuiInputFlags_LockUntilRelease            = 1 << 21,

    ImGuiInputFlags_CondHovered                 = 1 << 22,
    ImGuiInputFlags_CondActive                  = 1 << 23,
    ImGuiInputFlags_CondDefault_                = ImGuiInputFlags_CondHovered | ImGuiInputFlags_CondActive,

    ImGuiInputFlags_RepeatRateMask_             = ImGuiInputFlags_RepeatRateDefault | ImGuiInputFlags_RepeatRateNavMove | ImGuiInputFlags_RepeatRateNavTweak,
    ImGuiInputFlags_RepeatUntilMask_            = ImGuiInputFlags_RepeatUntilRelease | ImGuiInputFlags_RepeatUntilKeyModsChange | ImGuiInputFlags_RepeatUntilKeyModsChangeFromNone | ImGuiInputFlags_RepeatUntilOtherKeyPress,
    ImGuiInputFlags_RepeatMask_                 = ImGuiInputFlags_Repeat | ImGuiInputFlags_RepeatRateMask_ | ImGuiInputFlags_RepeatUntilMask_,
    ImGuiInputFlags_CondMask_                   = ImGuiInputFlags_CondHovered | ImGuiInputFlags_CondActive,
    ImGuiInputFlags_RouteTypeMask_              = ImGuiInputFlags_RouteActive | ImGuiInputFlags_RouteFocused | ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteAlways,
    ImGuiInputFlags_RouteOptionsMask_           = ImGuiInputFlags_RouteOverFocused | ImGuiInputFlags_RouteOverActive | ImGuiInputFlags_RouteUnlessBgFocused | ImGuiInputFlags_RouteFromRootWindow,
    ImGuiInputFlags_SupportedByIsKeyPressed     = ImGuiInputFlags_RepeatMask_,
    ImGuiInputFlags_SupportedByIsMouseClicked   = ImGuiInputFlags_Repeat,
    ImGuiInputFlags_SupportedByShortcut         = ImGuiInputFlags_RepeatMask_ | ImGuiInputFlags_RouteTypeMask_ | ImGuiInputFlags_RouteOptionsMask_,
    ImGuiInputFlags_SupportedBySetNextItemShortcut = ImGuiInputFlags_RepeatMask_ | ImGuiInputFlags_RouteTypeMask_ | ImGuiInputFlags_RouteOptionsMask_ | ImGuiInputFlags_Tooltip,
    ImGuiInputFlags_SupportedBySetKeyOwner      = ImGuiInputFlags_LockThisFrame | ImGuiInputFlags_LockUntilRelease,
    ImGuiInputFlags_SupportedBySetItemKeyOwner  = ImGuiInputFlags_SupportedBySetKeyOwner | ImGuiInputFlags_CondMask_,
};

struct ImGuiListClipperRange
{
    int     Min;
    int     Max;
    bool    PosToIndexConvert;
    ImS8    PosToIndexOffsetMin;
    ImS8    PosToIndexOffsetMax;

    static ImGuiListClipperRange    FromIndices(int min, int max)                               { ImGuiListClipperRange r = { min, max, false, 0, 0 }; return r; }
    static ImGuiListClipperRange    FromPositions(float y1, float y2, int off_min, int off_max) { ImGuiListClipperRange r = { (int)y1, (int)y2, true, (ImS8)off_min, (ImS8)off_max }; return r; }
};

struct ImGuiListClipperData
{
    ImGuiListClipper*               ListClipper;
    float                           LossynessOffset;
    int                             StepNo;
    int                             ItemsFrozen;
    ImVector<ImGuiListClipperRange> Ranges;

    ImGuiListClipperData()          { memset((void*)this, 0, sizeof(*this)); }
    void                            Reset(ImGuiListClipper* clipper) { ListClipper = clipper; StepNo = ItemsFrozen = 0; Ranges.resize(0); }
};

enum ImGuiActivateFlags_
{
    ImGuiActivateFlags_None                 = 0,
    ImGuiActivateFlags_PreferInput          = 1 << 0,
    ImGuiActivateFlags_PreferTweak          = 1 << 1,
    ImGuiActivateFlags_TryToPreserveState   = 1 << 2,
    ImGuiActivateFlags_FromTabbing          = 1 << 3,
    ImGuiActivateFlags_FromShortcut         = 1 << 4,
    ImGuiActivateFlags_FromFocusApi         = 1 << 5,
};

enum ImGuiScrollFlags_
{
    ImGuiScrollFlags_None                   = 0,
    ImGuiScrollFlags_KeepVisibleEdgeX       = 1 << 0,
    ImGuiScrollFlags_KeepVisibleEdgeY       = 1 << 1,
    ImGuiScrollFlags_KeepVisibleCenterX     = 1 << 2,
    ImGuiScrollFlags_KeepVisibleCenterY     = 1 << 3,
    ImGuiScrollFlags_AlwaysCenterX          = 1 << 4,
    ImGuiScrollFlags_AlwaysCenterY          = 1 << 5,
    ImGuiScrollFlags_NoScrollParent         = 1 << 6,
    ImGuiScrollFlags_MaskX_                 = ImGuiScrollFlags_KeepVisibleEdgeX | ImGuiScrollFlags_KeepVisibleCenterX | ImGuiScrollFlags_AlwaysCenterX,
    ImGuiScrollFlags_MaskY_                 = ImGuiScrollFlags_KeepVisibleEdgeY | ImGuiScrollFlags_KeepVisibleCenterY | ImGuiScrollFlags_AlwaysCenterY,
};

enum ImGuiNavRenderCursorFlags_
{
    ImGuiNavRenderCursorFlags_None          = 0,
    ImGuiNavRenderCursorFlags_Compact       = 1 << 1,
    ImGuiNavRenderCursorFlags_AlwaysDraw    = 1 << 2,
    ImGuiNavRenderCursorFlags_NoRounding    = 1 << 3,
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    ImGuiNavHighlightFlags_None             = ImGuiNavRenderCursorFlags_None,
    ImGuiNavHighlightFlags_Compact          = ImGuiNavRenderCursorFlags_Compact,
    ImGuiNavHighlightFlags_AlwaysDraw       = ImGuiNavRenderCursorFlags_AlwaysDraw,
    ImGuiNavHighlightFlags_NoRounding       = ImGuiNavRenderCursorFlags_NoRounding,

#endif
};

enum ImGuiNavMoveFlags_
{
    ImGuiNavMoveFlags_None                  = 0,
    ImGuiNavMoveFlags_LoopX                 = 1 << 0,
    ImGuiNavMoveFlags_LoopY                 = 1 << 1,
    ImGuiNavMoveFlags_WrapX                 = 1 << 2,
    ImGuiNavMoveFlags_WrapY                 = 1 << 3,
    ImGuiNavMoveFlags_WrapMask_             = ImGuiNavMoveFlags_LoopX | ImGuiNavMoveFlags_LoopY | ImGuiNavMoveFlags_WrapX | ImGuiNavMoveFlags_WrapY,
    ImGuiNavMoveFlags_AllowCurrentNavId     = 1 << 4,
    ImGuiNavMoveFlags_AlsoScoreVisibleSet   = 1 << 5,
    ImGuiNavMoveFlags_ScrollToEdgeY         = 1 << 6,
    ImGuiNavMoveFlags_Forwarded             = 1 << 7,
    ImGuiNavMoveFlags_DebugNoResult         = 1 << 8,
    ImGuiNavMoveFlags_FocusApi              = 1 << 9,
    ImGuiNavMoveFlags_IsTabbing             = 1 << 10,
    ImGuiNavMoveFlags_IsPageMove            = 1 << 11,
    ImGuiNavMoveFlags_Activate              = 1 << 12,
    ImGuiNavMoveFlags_NoSelect              = 1 << 13,
    ImGuiNavMoveFlags_NoSetNavCursorVisible = 1 << 14,
    ImGuiNavMoveFlags_NoClearActiveId       = 1 << 15,
};

enum ImGuiNavLayer
{
    ImGuiNavLayer_Main  = 0,
    ImGuiNavLayer_Menu  = 1,
    ImGuiNavLayer_COUNT
};

struct ImGuiNavItemData
{
    ImGuiWindow*        Window;
    ImGuiID             ID;
    ImGuiID             FocusScopeId;
    ImRect              RectRel;
    ImGuiItemFlags      ItemFlags;
    float               DistBox;
    float               DistCenter;
    float               DistAxial;
    ImGuiSelectionUserData SelectionUserData;

    ImGuiNavItemData()  { Clear(); }
    void Clear()        { Window = NULL; ID = FocusScopeId = 0; ItemFlags = 0; SelectionUserData = -1; DistBox = DistCenter = DistAxial = FLT_MAX; }
};

struct ImGuiFocusScopeData
{
    ImGuiID             ID;
    ImGuiID             WindowID;
};

enum ImGuiTypingSelectFlags_
{
    ImGuiTypingSelectFlags_None                 = 0,
    ImGuiTypingSelectFlags_AllowBackspace       = 1 << 0,
    ImGuiTypingSelectFlags_AllowSingleCharMode  = 1 << 1,
};

struct IMGUI_API ImGuiTypingSelectRequest
{
    ImGuiTypingSelectFlags  Flags;
    int                     SearchBufferLen;
    const char*             SearchBuffer;
    bool                    SelectRequest;
    bool                    SingleCharMode;
    ImS8                    SingleCharSize;
};

struct IMGUI_API ImGuiTypingSelectState
{
    ImGuiTypingSelectRequest Request;
    char            SearchBuffer[64];
    ImGuiID         FocusScope;
    int             LastRequestFrame = 0;
    float           LastRequestTime = 0.0f;
    bool            SingleCharModeLock = false;

    ImGuiTypingSelectState() { memset((void*)this, 0, sizeof(*this)); }
    void            Clear()  { SearchBuffer[0] = 0; SingleCharModeLock = false; }
};

enum ImGuiOldColumnFlags_
{
    ImGuiOldColumnFlags_None                    = 0,
    ImGuiOldColumnFlags_NoBorder                = 1 << 0,
    ImGuiOldColumnFlags_NoResize                = 1 << 1,
    ImGuiOldColumnFlags_NoPreserveWidths        = 1 << 2,
    ImGuiOldColumnFlags_NoForceWithinWindow     = 1 << 3,
    ImGuiOldColumnFlags_GrowParentContentsSize  = 1 << 4,

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#endif
};

struct ImGuiOldColumnData
{
    float               OffsetNorm;
    float               OffsetNormBeforeResize;
    ImGuiOldColumnFlags Flags;
    ImRect              ClipRect;

    ImGuiOldColumnData() { memset((void*)this, 0, sizeof(*this)); }
};

struct ImGuiOldColumns
{
    ImGuiID             ID;
    ImGuiOldColumnFlags Flags;
    bool                IsFirstFrame;
    bool                IsBeingResized;
    int                 Current;
    int                 Count;
    float               OffMinX, OffMaxX;
    float               LineMinY, LineMaxY;
    float               HostCursorPosY;
    float               HostCursorMaxPosX;
    ImRect              HostInitialClipRect;
    ImRect              HostBackupClipRect;
    ImRect              HostBackupParentWorkRect;
    ImVector<ImGuiOldColumnData> Columns;
    ImDrawListSplitter  Splitter;

    ImGuiOldColumns()   { memset((void*)this, 0, sizeof(*this)); }
};

struct ImGuiBoxSelectState
{

    ImGuiID                 ID;
    bool                    IsActive;
    bool                    IsStarting;
    bool                    IsStartedFromVoid;
    bool                    IsStartedSetNavIdOnce;
    bool                    RequestClear;
    ImGuiKeyChord           KeyMods : 16;
    ImVec2                  StartPosRel;
    ImVec2                  EndPosRel;
    ImVec2                  ScrollAccum;
    ImGuiWindow*            Window;

    bool                    UnclipMode;
    ImRect                  UnclipRect;
    ImRect                  BoxSelectRectPrev;
    ImRect                  BoxSelectRectCurr;

    ImGuiBoxSelectState()   { memset((void*)this, 0, sizeof(*this)); }
};

#define ImGuiSelectionUserData_Invalid        ((ImGuiSelectionUserData)-1)

struct IMGUI_API ImGuiMultiSelectTempData
{
    ImGuiMultiSelectIO      IO;
    ImGuiMultiSelectState*  Storage;
    ImGuiID                 FocusScopeId;
    ImGuiMultiSelectFlags   Flags;
    ImVec2                  ScopeRectMin;
    ImVec2                  BackupCursorMaxPos;
    ImGuiSelectionUserData  LastSubmittedItem;
    ImGuiID                 BoxSelectId;
    ImGuiKeyChord           KeyMods;
    ImS8                    LoopRequestSetAll;
    bool                    IsEndIO;
    bool                    IsFocused;
    bool                    IsKeyboardSetRange;
    bool                    NavIdPassedBy;
    bool                    RangeSrcPassedBy;
    bool                    RangeDstPassedBy;

    ImGuiMultiSelectTempData()  { Clear(); }
    void Clear()            { size_t io_sz = sizeof(IO); ClearIO(); memset((void*)(&IO + 1), 0, sizeof(*this) - io_sz); }
    void ClearIO()          { IO.Requests.resize(0); IO.RangeSrcItem = IO.NavIdItem = ImGuiSelectionUserData_Invalid; IO.NavIdSelected = IO.RangeSrcReset = false; }
};

struct IMGUI_API ImGuiMultiSelectState
{
    ImGuiWindow*            Window;
    ImGuiID                 ID;
    int                     LastFrameActive;
    int                     LastSelectionSize;
    ImS8                    RangeSelected;
    ImS8                    NavIdSelected;
    ImGuiSelectionUserData  RangeSrcItem;
    ImGuiSelectionUserData  NavIdItem;

    ImGuiMultiSelectState() { Window = NULL; ID = 0; LastFrameActive = LastSelectionSize = 0; RangeSelected = NavIdSelected = -1; RangeSrcItem = NavIdItem = ImGuiSelectionUserData_Invalid; }
};

#ifdef IMGUI_HAS_DOCK

#endif

struct ImGuiViewportP : public ImGuiViewport
{
    float               BgFgDrawListsLastTimeActive[2];
    ImDrawList*         BgFgDrawLists[2];
    ImDrawData          DrawDataP;
    ImDrawDataBuilder   DrawDataBuilder;

    ImVec2              WorkInsetMin;
    ImVec2              WorkInsetMax;
    ImVec2              BuildWorkInsetMin;
    ImVec2              BuildWorkInsetMax;

    ImGuiViewportP()    { BgFgDrawListsLastTimeActive[0] = BgFgDrawListsLastTimeActive[1] = -1.0f; BgFgDrawLists[0] = BgFgDrawLists[1] = NULL; }
    ~ImGuiViewportP()   { if (BgFgDrawLists[0]) IM_DELETE(BgFgDrawLists[0]); if (BgFgDrawLists[1]) IM_DELETE(BgFgDrawLists[1]); }

    ImVec2  CalcWorkRectPos(const ImVec2& inset_min) const                           { return ImVec2(Pos.x + inset_min.x, Pos.y + inset_min.y); }
    ImVec2  CalcWorkRectSize(const ImVec2& inset_min, const ImVec2& inset_max) const { return ImVec2(ImMax(0.0f, Size.x - inset_min.x - inset_max.x), ImMax(0.0f, Size.y - inset_min.y - inset_max.y)); }
    void    UpdateWorkRect()            { WorkPos = CalcWorkRectPos(WorkInsetMin); WorkSize = CalcWorkRectSize(WorkInsetMin, WorkInsetMax); }

    ImRect  GetMainRect() const         { return ImRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    ImRect  GetWorkRect() const         { return ImRect(WorkPos.x, WorkPos.y, WorkPos.x + WorkSize.x, WorkPos.y + WorkSize.y); }
    ImRect  GetBuildWorkRect() const    { ImVec2 pos = CalcWorkRectPos(BuildWorkInsetMin); ImVec2 size = CalcWorkRectSize(BuildWorkInsetMin, BuildWorkInsetMax); return ImRect(pos.x, pos.y, pos.x + size.x, pos.y + size.y); }
};

struct ImGuiWindowSettings
{
    ImGuiID     ID;
    ImVec2ih    Pos;
    ImVec2ih    Size;
    bool        Collapsed;
    bool        IsChild;
    bool        WantApply;
    bool        WantDelete;

    ImGuiWindowSettings()       { memset((void*)this, 0, sizeof(*this)); }
    char* GetName()             { return (char*)(this + 1); }
};

struct ImGuiSettingsHandler
{
    const char* TypeName;
    ImGuiID     TypeHash;
    void        (*ClearAllFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
    void        (*ReadInitFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
    void*       (*ReadOpenFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name);
    void        (*ReadLineFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line);
    void        (*ApplyAllFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
    void        (*WriteAllFn)(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf);
    void*       UserData;

    ImGuiSettingsHandler() { memset((void*)this, 0, sizeof(*this)); }
};

enum ImGuiLocKey : int
{
    ImGuiLocKey_VersionStr,
    ImGuiLocKey_TableSizeOne,
    ImGuiLocKey_TableSizeAllFit,
    ImGuiLocKey_TableSizeAllDefault,
    ImGuiLocKey_TableResetOrder,
    ImGuiLocKey_WindowingMainMenuBar,
    ImGuiLocKey_WindowingPopup,
    ImGuiLocKey_WindowingUntitled,
    ImGuiLocKey_OpenLink_s,
    ImGuiLocKey_CopyLink,
    ImGuiLocKey_COUNT
};

struct ImGuiLocEntry
{
    ImGuiLocKey     Key;
    const char*     Text;
};

#ifndef IM_ASSERT_USER_ERROR
#define IM_ASSERT_USER_ERROR(_EXPR,_MSG)            do { if (!(_EXPR)) { if (ImGui::ErrorLog(_MSG)) { IM_ASSERT((_EXPR) && _MSG); } } } while (0)
#define IM_ASSERT_USER_ERROR_RET(_EXPR,_MSG)        do { if (!(_EXPR)) { if (ImGui::ErrorLog(_MSG)) { IM_ASSERT((_EXPR) && _MSG); } return; } } while (0)
#define IM_ASSERT_USER_ERROR_RETV(_EXPR,_RETV,_MSG) do { if (!(_EXPR)) { if (ImGui::ErrorLog(_MSG)) { IM_ASSERT((_EXPR) && _MSG); } return _RETV; } } while (0)
#endif

typedef void (*ImGuiErrorCallback)(ImGuiContext* ctx, void* user_data, const char* msg);

enum ImGuiDebugLogFlags_
{

    ImGuiDebugLogFlags_None                 = 0,
    ImGuiDebugLogFlags_EventError           = 1 << 0,
    ImGuiDebugLogFlags_EventActiveId        = 1 << 1,
    ImGuiDebugLogFlags_EventFocus           = 1 << 2,
    ImGuiDebugLogFlags_EventPopup           = 1 << 3,
    ImGuiDebugLogFlags_EventNav             = 1 << 4,
    ImGuiDebugLogFlags_EventClipper         = 1 << 5,
    ImGuiDebugLogFlags_EventSelection       = 1 << 6,
    ImGuiDebugLogFlags_EventIO              = 1 << 7,
    ImGuiDebugLogFlags_EventFont            = 1 << 8,
    ImGuiDebugLogFlags_EventInputRouting    = 1 << 9,
    ImGuiDebugLogFlags_EventDocking         = 1 << 10,
    ImGuiDebugLogFlags_EventViewport        = 1 << 11,

    ImGuiDebugLogFlags_EventMask_           = ImGuiDebugLogFlags_EventError | ImGuiDebugLogFlags_EventActiveId | ImGuiDebugLogFlags_EventFocus | ImGuiDebugLogFlags_EventPopup | ImGuiDebugLogFlags_EventNav | ImGuiDebugLogFlags_EventClipper | ImGuiDebugLogFlags_EventSelection | ImGuiDebugLogFlags_EventIO | ImGuiDebugLogFlags_EventFont | ImGuiDebugLogFlags_EventInputRouting | ImGuiDebugLogFlags_EventDocking | ImGuiDebugLogFlags_EventViewport,
    ImGuiDebugLogFlags_OutputToTTY          = 1 << 20,
    ImGuiDebugLogFlags_OutputToDebugger     = 1 << 21,
    ImGuiDebugLogFlags_OutputToTestEngine   = 1 << 22,
};

struct ImGuiDebugAllocEntry
{
    int         FrameCount;
    ImS16       AllocCount;
    ImS16       FreeCount;
};

struct ImGuiDebugAllocInfo
{
    int         TotalAllocCount;
    int         TotalFreeCount;
    ImS16       LastEntriesIdx;
    ImGuiDebugAllocEntry LastEntriesBuf[6];

    ImGuiDebugAllocInfo() { memset((void*)this, 0, sizeof(*this)); }
};

struct ImGuiMetricsConfig
{
    bool        ShowDebugLog = false;
    bool        ShowIDStackTool = false;
    bool        ShowWindowsRects = false;
    bool        ShowWindowsBeginOrder = false;
    bool        ShowTablesRects = false;
    bool        ShowDrawCmdMesh = true;
    bool        ShowDrawCmdBoundingBoxes = true;
    bool        ShowTextEncodingViewer = false;
    bool        ShowTextureUsedRect = false;
    int         ShowWindowsRectsType = -1;
    int         ShowTablesRectsType = -1;
    int         HighlightMonitorIdx = -1;
    ImGuiID     HighlightViewportID = 0;
    bool        ShowFontPreview = true;
};

struct ImGuiStackLevelInfo
{
    ImGuiID                 ID;
    ImS8                    QueryFrameCount;
    bool                    QuerySuccess;
    ImS8                    DataType;
    int                     DescOffset;

    ImGuiStackLevelInfo()   { memset((void*)this, 0, sizeof(*this)); DataType = -1; DescOffset = -1; }
};

struct ImGuiDebugItemPathQuery
{
    ImGuiID                 MainID;
    bool                    Active;
    bool                    Complete;
    ImS8                    Step;
    ImVector<ImGuiStackLevelInfo> Results;
    ImGuiTextBuffer         ResultsDescBuf;
    ImGuiTextBuffer         ResultPathBuf;

    ImGuiDebugItemPathQuery() { memset((void*)this, 0, sizeof(*this)); }
};

struct ImGuiIDStackTool
{
    bool                    OptHexEncodeNonAsciiChars;
    bool                    OptCopyToClipboardOnCtrlC;
    int                     LastActiveFrame;
    float                   CopyToClipboardLastTime;

    ImGuiIDStackTool()      { memset((void*)this, 0, sizeof(*this)); LastActiveFrame = -1; OptHexEncodeNonAsciiChars = true; CopyToClipboardLastTime = -FLT_MAX; }
};

typedef void (*ImGuiContextHookCallback)(ImGuiContext* ctx, ImGuiContextHook* hook);
enum ImGuiContextHookType { ImGuiContextHookType_NewFramePre, ImGuiContextHookType_NewFramePost, ImGuiContextHookType_EndFramePre, ImGuiContextHookType_EndFramePost, ImGuiContextHookType_RenderPre, ImGuiContextHookType_RenderPost, ImGuiContextHookType_Shutdown, ImGuiContextHookType_PendingRemoval_ };

struct ImGuiContextHook
{
    ImGuiID                     HookId;
    ImGuiContextHookType        Type;
    ImGuiID                     Owner;
    ImGuiContextHookCallback    Callback;
    void*                       UserData;

    ImGuiContextHook()          { memset((void*)this, 0, sizeof(*this)); }
};

typedef void (*ImGuiDemoMarkerCallback)(const char* file, int line, const char* section);

struct ImGuiContext
{
    bool                    Initialized;
    bool                    WithinFrameScope;
    bool                    WithinFrameScopeWithImplicitWindow;
    bool                    TestEngineHookItems;
    int                     FrameCount;
    int                     FrameCountEnded;
    int                     FrameCountRendered;
    double                  Time;
    char                    ContextName[16];
    ImGuiIO                 IO;
    ImGuiPlatformIO         PlatformIO;
    ImGuiStyle              Style;
    ImVector<ImFontAtlas*>  FontAtlases;
    ImFont*                 Font;
    ImFontBaked*            FontBaked;
    float                   FontSize;
    float                   FontSizeBase;
    float                   FontBakedScale;
    float                   FontRasterizerDensity;
    float                   CurrentDpiScale;
    ImDrawListSharedData    DrawListSharedData;
    ImGuiID                 WithinEndChildID;
    void*                   TestEngine;

    ImVector<ImGuiInputEvent> InputEventsQueue;
    ImVector<ImGuiInputEvent> InputEventsTrail;
    ImGuiMouseSource        InputEventsNextMouseSource;
    ImU32                   InputEventsNextEventId;

    ImVector<ImGuiWindow*>  Windows;
    ImVector<ImGuiWindow*>  WindowsFocusOrder;
    ImVector<ImGuiWindow*>  WindowsTempSortBuffer;
    ImVector<ImGuiWindowStackData> CurrentWindowStack;
    ImGuiStorage            WindowsById;
    int                     WindowsActiveCount;
    float                   WindowsBorderHoverPadding;
    ImGuiID                 DebugBreakInWindow;
    ImGuiWindow*            CurrentWindow;
    ImGuiWindow*            HoveredWindow;
    ImGuiWindow*            HoveredWindowUnderMovingWindow;
    ImGuiWindow*            HoveredWindowBeforeClear;
    ImGuiWindow*            MovingWindow;
    ImGuiWindow*            WheelingWindow;
    ImVec2                  WheelingWindowRefMousePos;
    int                     WheelingWindowStartFrame;
    int                     WheelingWindowScrolledFrame;
    float                   WheelingWindowReleaseTimer;
    ImVec2                  WheelingWindowWheelRemainder;
    ImVec2                  WheelingAxisAvg;

    ImGuiID                 DebugDrawIdConflictsId;
    ImGuiID                 DebugHookIdInfoId;
    ImGuiID                 HoveredId;
    ImGuiID                 HoveredIdPreviousFrame;
    int                     HoveredIdPreviousFrameItemCount;
    float                   HoveredIdTimer;
    float                   HoveredIdNotActiveTimer;
    bool                    HoveredIdAllowOverlap;
    bool                    HoveredIdIsDisabled;
    bool                    ItemUnclipByLog;
    ImGuiID                 ActiveId;
    ImGuiID                 ActiveIdIsAlive;
    float                   ActiveIdTimer;
    bool                    ActiveIdIsJustActivated;
    bool                    ActiveIdAllowOverlap;
    bool                    ActiveIdNoClearOnFocusLoss;
    bool                    ActiveIdHasBeenPressedBefore;
    bool                    ActiveIdHasBeenEditedBefore;
    bool                    ActiveIdHasBeenEditedThisFrame;
    bool                    ActiveIdFromShortcut;
    ImS8                    ActiveIdMouseButton;
    ImGuiID                 ActiveIdDisabledId;
    ImVec2                  ActiveIdClickOffset;
    ImGuiInputSource        ActiveIdSource;
    ImGuiWindow*            ActiveIdWindow;
    ImGuiID                 ActiveIdPreviousFrame;
    ImGuiDeactivatedItemData DeactivatedItemData;
    ImGuiDataTypeStorage    ActiveIdValueOnActivation;
    ImGuiID                 LastActiveId;
    float                   LastActiveIdTimer;

    double                  LastKeyModsChangeTime;
    double                  LastKeyModsChangeFromNoneTime;
    double                  LastKeyboardKeyPressTime;
    ImBitArrayForNamedKeys  KeysMayBeCharInput;
    ImGuiKeyOwnerData       KeysOwnerData[ImGuiKey_NamedKey_COUNT];
    ImGuiKeyRoutingTable    KeysRoutingTable;
    ImU32                   ActiveIdUsingNavDirMask;
    bool                    ActiveIdUsingAllKeyboardKeys;
    ImGuiKeyChord           DebugBreakInShortcutRouting;

    ImGuiID                 CurrentFocusScopeId;
    ImGuiItemFlags          CurrentItemFlags;
    ImGuiID                 DebugLocateId;
    ImGuiNextItemData       NextItemData;
    ImGuiLastItemData       LastItemData;
    ImGuiNextWindowData     NextWindowData;
    bool                    DebugShowGroupRects;
    bool                    GcCompactAll;

    ImGuiCol                        DebugFlashStyleColorIdx;
    ImVector<ImGuiColorMod>         ColorStack;
    ImVector<ImGuiStyleMod>         StyleVarStack;
    ImVector<ImFontStackData>       FontStack;
    ImVector<ImGuiFocusScopeData>   FocusScopeStack;
    ImVector<ImGuiItemFlags>        ItemFlagsStack;
    ImVector<ImGuiGroupData>        GroupStack;
    ImVector<ImGuiPopupData>        OpenPopupStack;
    ImVector<ImGuiPopupData>        BeginPopupStack;
    ImVector<ImGuiTreeNodeStackData>TreeNodeStack;

    ImVector<ImGuiViewportP*> Viewports;

    bool                    NavCursorVisible;
    bool                    NavHighlightItemUnderNav;

    bool                    NavMousePosDirty;
    bool                    NavIdIsAlive;
    ImGuiID                 NavId;
    ImGuiWindow*            NavWindow;
    ImGuiID                 NavFocusScopeId;
    ImGuiNavLayer           NavLayer;
    ImGuiItemFlags          NavIdItemFlags;
    ImGuiID                 NavActivateId;
    ImGuiID                 NavActivateDownId;
    ImGuiID                 NavActivatePressedId;
    ImGuiActivateFlags      NavActivateFlags;
    ImVector<ImGuiFocusScopeData> NavFocusRoute;
    ImGuiID                 NavHighlightActivatedId;
    float                   NavHighlightActivatedTimer;
    ImGuiID                 NavOpenContextMenuItemId;
    ImGuiID                 NavOpenContextMenuWindowId;
    ImGuiID                 NavNextActivateId;
    ImGuiActivateFlags      NavNextActivateFlags;
    ImGuiInputSource        NavInputSource;
    ImGuiSelectionUserData  NavLastValidSelectionUserData;
    ImS8                    NavCursorHideFrames;

    bool                    NavAnyRequest;
    bool                    NavInitRequest;
    bool                    NavInitRequestFromMove;
    ImGuiNavItemData        NavInitResult;
    bool                    NavMoveSubmitted;
    bool                    NavMoveScoringItems;
    bool                    NavMoveForwardToNextFrame;
    ImGuiNavMoveFlags       NavMoveFlags;
    ImGuiScrollFlags        NavMoveScrollFlags;
    ImGuiKeyChord           NavMoveKeyMods;
    ImGuiDir                NavMoveDir;
    ImGuiDir                NavMoveDirForDebug;
    ImGuiDir                NavMoveClipDir;
    ImRect                  NavScoringRect;
    ImRect                  NavScoringNoClipRect;
    int                     NavScoringDebugCount;
    int                     NavTabbingDir;
    int                     NavTabbingCounter;
    ImGuiNavItemData        NavMoveResultLocal;
    ImGuiNavItemData        NavMoveResultLocalVisible;
    ImGuiNavItemData        NavMoveResultOther;
    ImGuiNavItemData        NavTabbingResultFirst;

    ImGuiID                 NavJustMovedFromFocusScopeId;
    ImGuiID                 NavJustMovedToId;
    ImGuiID                 NavJustMovedToFocusScopeId;
    ImGuiKeyChord           NavJustMovedToKeyMods;
    bool                    NavJustMovedToIsTabbing;
    bool                    NavJustMovedToHasSelectionData;

    bool                    ConfigNavEnableTabbing;
    bool                    ConfigNavWindowingWithGamepad;
    ImGuiKeyChord           ConfigNavWindowingKeyNext;
    ImGuiKeyChord           ConfigNavWindowingKeyPrev;

    ImGuiWindow*            NavWindowingTarget;
    ImGuiWindow*            NavWindowingTargetAnim;
    ImGuiWindow*            NavWindowingListWindow;
    float                   NavWindowingTimer;
    float                   NavWindowingHighlightAlpha;
    ImGuiInputSource        NavWindowingInputSource;
    bool                    NavWindowingToggleLayer;
    ImGuiKey                NavWindowingToggleKey;
    ImVec2                  NavWindowingAccumDeltaPos;
    ImVec2                  NavWindowingAccumDeltaSize;

    float                   DimBgRatio;

    bool                    DragDropActive;
    bool                    DragDropWithinSource;
    bool                    DragDropWithinTarget;
    ImGuiDragDropFlags      DragDropSourceFlags;
    int                     DragDropSourceFrameCount;
    int                     DragDropMouseButton;
    ImGuiPayload            DragDropPayload;
    ImRect                  DragDropTargetRect;
    ImRect                  DragDropTargetClipRect;
    ImGuiID                 DragDropTargetId;
    ImGuiID                 DragDropTargetFullViewport;
    ImGuiDragDropFlags      DragDropAcceptFlagsCurr;
    ImGuiDragDropFlags      DragDropAcceptFlagsPrev;
    float                   DragDropAcceptIdCurrRectSurface;
    ImGuiID                 DragDropAcceptIdCurr;
    ImGuiID                 DragDropAcceptIdPrev;
    int                     DragDropAcceptFrameCount;
    ImGuiID                 DragDropHoldJustPressedId;
    ImVector<unsigned char> DragDropPayloadBufHeap;
    unsigned char           DragDropPayloadBufLocal[16];

    int                             ClipperTempDataStacked;
    ImVector<ImGuiListClipperData>  ClipperTempData;

    ImGuiTable*                     CurrentTable;
    ImGuiID                         DebugBreakInTable;
    int                             TablesTempDataStacked;
    ImVector<ImGuiTableTempData>    TablesTempData;
    ImPool<ImGuiTable>              Tables;
    ImVector<float>                 TablesLastTimeActive;
    ImVector<ImDrawChannel>         DrawChannelsTempMergeBuffer;

    ImGuiTabBar*                    CurrentTabBar;
    ImPool<ImGuiTabBar>             TabBars;
    ImVector<ImGuiPtrOrIndex>       CurrentTabBarStack;
    ImVector<ImGuiShrinkWidthItem>  ShrinkWidthBuffer;

    ImGuiBoxSelectState             BoxSelectState;
    ImGuiMultiSelectTempData*       CurrentMultiSelect;
    int                             MultiSelectTempDataStacked;
    ImVector<ImGuiMultiSelectTempData> MultiSelectTempData;
    ImPool<ImGuiMultiSelectState>   MultiSelectStorage;

    ImGuiID                 HoverItemDelayId;
    ImGuiID                 HoverItemDelayIdPreviousFrame;
    float                   HoverItemDelayTimer;
    float                   HoverItemDelayClearTimer;
    ImGuiID                 HoverItemUnlockedStationaryId;
    ImGuiID                 HoverWindowUnlockedStationaryId;

    ImGuiMouseCursor        MouseCursor;
    float                   MouseStationaryTimer;
    ImVec2                  MouseLastValidPos;

    ImGuiInputTextState     InputTextState;
    ImGuiTextIndex          InputTextLineIndex;
    ImGuiInputTextDeactivatedState InputTextDeactivatedState;
    ImFontBaked             InputTextPasswordFontBackupBaked;
    ImFontFlags             InputTextPasswordFontBackupFlags;
    ImGuiID                 InputTextReactivateId;
    ImGuiID                 TempInputId;
    ImGuiDataTypeStorage    DataTypeZeroValue;
    int                     BeginMenuDepth;
    int                     BeginComboDepth;
    ImGuiColorEditFlags     ColorEditOptions;
    ImGuiID                 ColorEditCurrentID;
    ImGuiID                 ColorEditSavedID;
    float                   ColorEditSavedHue;
    float                   ColorEditSavedSat;
    ImU32                   ColorEditSavedColor;
    ImVec4                  ColorPickerRef;
    ImGuiComboPreviewData   ComboPreviewData;
    ImRect                  WindowResizeBorderExpectedRect;
    bool                    WindowResizeRelativeMode;
    short                   ScrollbarSeekMode;
    float                   ScrollbarClickDeltaToGrabCenter;
    float                   SliderGrabClickOffset;
    float                   SliderCurrentAccum;
    bool                    SliderCurrentAccumDirty;
    bool                    DragCurrentAccumDirty;
    float                   DragCurrentAccum;
    float                   DragSpeedDefaultRatio;
    float                   DisabledAlphaBackup;
    short                   DisabledStackSize;
    short                   TooltipOverrideCount;
    ImGuiWindow*            TooltipPreviousWindow;
    ImVector<char>          ClipboardHandlerData;
    ImVector<ImGuiID>       MenusIdSubmittedThisFrame;
    ImGuiTypingSelectState  TypingSelectState;

    ImGuiPlatformImeData    PlatformImeData;
    ImGuiPlatformImeData    PlatformImeDataPrev;

    ImVector<ImTextureData*> UserTextures;

    bool                    SettingsLoaded;
    float                   SettingsDirtyTimer;
    ImGuiTextBuffer         SettingsIniData;
    ImVector<ImGuiSettingsHandler>      SettingsHandlers;
    ImChunkStream<ImGuiWindowSettings>  SettingsWindows;
    ImChunkStream<ImGuiTableSettings>   SettingsTables;

    ImVector<ImGuiContextHook>          Hooks;
    ImGuiID                             HookIdNext;
    ImGuiDemoMarkerCallback             DemoMarkerCallback;

    const char*             LocalizationTable[ImGuiLocKey_COUNT];

    bool                    LogEnabled;
    bool                    LogLineFirstItem;
    ImGuiLogFlags           LogFlags;
    ImGuiWindow*            LogWindow;
    ImFileHandle            LogFile;
    ImGuiTextBuffer         LogBuffer;
    const char*             LogNextPrefix;
    const char*             LogNextSuffix;
    float                   LogLinePosY;
    int                     LogDepthRef;
    int                     LogDepthToExpand;
    int                     LogDepthToExpandDefault;

    ImGuiErrorCallback      ErrorCallback;
    void*                   ErrorCallbackUserData;
    ImVec2                  ErrorTooltipLockedPos;
    bool                    ErrorFirst;
    int                     ErrorCountCurrentFrame;
    ImGuiErrorRecoveryState StackSizesInNewFrame;
    ImGuiErrorRecoveryState*StackSizesInBeginForCurrentWindow;

    int                     DebugDrawIdConflictsCount;
    ImGuiDebugLogFlags      DebugLogFlags;
    ImGuiTextBuffer         DebugLogBuf;
    ImGuiTextIndex          DebugLogIndex;
    int                     DebugLogSkippedErrors;
    ImGuiDebugLogFlags      DebugLogAutoDisableFlags;
    ImU8                    DebugLogAutoDisableFrames;
    ImU8                    DebugLocateFrames;
    bool                    DebugBreakInLocateId;
    ImGuiKeyChord           DebugBreakKeyChord;
    ImS8                    DebugBeginReturnValueCullDepth;
    bool                    DebugItemPickerActive;
    ImU8                    DebugItemPickerMouseButton;
    ImGuiID                 DebugItemPickerBreakId;
    float                   DebugFlashStyleColorTime;
    ImVec4                  DebugFlashStyleColorBackup;
    ImGuiMetricsConfig      DebugMetricsConfig;
    ImGuiDebugItemPathQuery DebugItemPathQuery;
    ImGuiIDStackTool        DebugIDStackTool;
    ImGuiDebugAllocInfo     DebugAllocInfo;
#if defined(IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS) && !defined(IMGUI_DISABLE_DEBUG_TOOLS)
    ImGuiStorage            DebugDrawIdConflictsAliveCount;
    ImGuiStorage            DebugDrawIdConflictsHighlightSet;
#endif

    float                   FramerateSecPerFrame[60];
    int                     FramerateSecPerFrameIdx;
    int                     FramerateSecPerFrameCount;
    float                   FramerateSecPerFrameAccum;
    int                     WantCaptureMouseNextFrame;
    int                     WantCaptureKeyboardNextFrame;
    int                     WantTextInputNextFrame;
    ImVector<char>          TempBuffer;
    char                    TempKeychordName[64];

    ImGuiContext(ImFontAtlas* shared_font_atlas);
    ~ImGuiContext();
};

#define IMGUI_WINDOW_HARD_MIN_SIZE 4.0f

struct IMGUI_API ImGuiWindowTempData
{

    ImVec2                  CursorPos;
    ImVec2                  CursorPosPrevLine;
    ImVec2                  CursorStartPos;
    ImVec2                  CursorMaxPos;
    ImVec2                  IdealMaxPos;
    ImVec2                  CurrLineSize;
    ImVec2                  PrevLineSize;
    float                   CurrLineTextBaseOffset;
    float                   PrevLineTextBaseOffset;
    bool                    IsSameLine;
    bool                    IsSetPos;
    ImVec1                  Indent;
    ImVec1                  ColumnsOffset;
    ImVec1                  GroupOffset;
    ImVec2                  CursorStartPosLossyness;

    ImGuiNavLayer           NavLayerCurrent;
    short                   NavLayersActiveMask;
    short                   NavLayersActiveMaskNext;
    bool                    NavIsScrollPushableX;
    bool                    NavHideHighlightOneFrame;
    bool                    NavWindowHasScrollY;

    bool                    MenuBarAppending;
    ImVec2                  MenuBarOffset;
    ImGuiMenuColumns        MenuColumns;
    int                     TreeDepth;
    ImU32                   TreeHasStackDataDepthMask;
    ImU32                   TreeRecordsClippedNodesY2Mask;
    ImVector<ImGuiWindow*>  ChildWindows;
    ImGuiStorage*           StateStorage;
    ImGuiOldColumns*        CurrentColumns;
    int                     CurrentTableIdx;
    ImGuiLayoutType         LayoutType;
    ImGuiLayoutType         ParentLayoutType;
    ImU32                   ModalDimBgColor;
    ImGuiItemStatusFlags    WindowItemStatusFlags;
    ImGuiItemStatusFlags    ChildItemStatusFlags;

    float                   ItemWidth;
    float                   ItemWidthDefault;
    float                   TextWrapPos;
    ImVector<float>         ItemWidthStack;
    ImVector<float>         TextWrapPosStack;
};

struct IMGUI_API ImGuiWindow
{
    ImGuiContext*           Ctx;
    char*                   Name;
    ImGuiID                 ID;
    ImGuiWindowFlags        Flags;
    ImGuiChildFlags         ChildFlags;
    ImGuiViewportP*         Viewport;
    ImVec2                  Pos;
    ImVec2                  Size;
    ImVec2                  SizeFull;
    ImVec2                  ContentSize;
    ImVec2                  ContentSizeIdeal;
    ImVec2                  ContentSizeExplicit;
    ImVec2                  WindowPadding;
    float                   WindowRounding;
    float                   WindowBorderSize;
    float                   TitleBarHeight, MenuBarHeight;
    float                   DecoOuterSizeX1, DecoOuterSizeY1;
    float                   DecoOuterSizeX2, DecoOuterSizeY2;
    float                   DecoInnerSizeX1, DecoInnerSizeY1;
    int                     NameBufLen;
    ImGuiID                 MoveId;
    ImGuiID                 ChildId;
    ImGuiID                 PopupId;
    ImVec2                  Scroll;
    ImVec2                  ScrollMax;
    ImVec2                  ScrollTarget;
    ImVec2                  ScrollTargetCenterRatio;
    ImVec2                  ScrollTargetEdgeSnapDist;
    ImVec2                  ScrollbarSizes;
    bool                    ScrollbarX, ScrollbarY;
    bool                    ScrollbarXStabilizeEnabled;
    ImU8                    ScrollbarXStabilizeToggledHistory;
    bool                    Active;
    bool                    WasActive;
    bool                    WriteAccessed;
    bool                    Collapsed;
    bool                    WantCollapseToggle;
    bool                    SkipItems;
    bool                    SkipRefresh;
    bool                    Appearing;
    bool                    Hidden;
    bool                    IsFallbackWindow;
    bool                    IsExplicitChild;
    bool                    HasCloseButton;
    signed char             ResizeBorderHovered;
    signed char             ResizeBorderHeld;
    short                   BeginCount;
    short                   BeginCountPreviousFrame;
    short                   BeginOrderWithinParent;
    short                   BeginOrderWithinContext;
    short                   FocusOrder;
    ImGuiDir                AutoPosLastDirection;
    ImS8                    AutoFitFramesX, AutoFitFramesY;
    bool                    AutoFitOnlyGrows;
    ImS8                    HiddenFramesCanSkipItems;
    ImS8                    HiddenFramesCannotSkipItems;
    ImS8                    HiddenFramesForRenderOnly;
    ImS8                    DisableInputsFrames;
    ImGuiWindowBgClickFlags BgClickFlags : 8;
    ImGuiCond               SetWindowPosAllowFlags : 8;
    ImGuiCond               SetWindowSizeAllowFlags : 8;
    ImGuiCond               SetWindowCollapsedAllowFlags : 8;
    ImVec2                  SetWindowPosVal;
    ImVec2                  SetWindowPosPivot;

    ImVector<ImGuiID>       IDStack;
    ImGuiWindowTempData     DC;

    ImRect                  OuterRectClipped;
    ImRect                  InnerRect;
    ImRect                  InnerClipRect;
    ImRect                  WorkRect;
    ImRect                  ParentWorkRect;
    ImRect                  ClipRect;
    ImRect                  ContentRegionRect;
    ImVec2ih                HitTestHoleSize;
    ImVec2ih                HitTestHoleOffset;

    int                     LastFrameActive;
    float                   LastTimeActive;
    ImGuiStorage            StateStorage;
    ImVector<ImGuiOldColumns> ColumnsStorage;
    float                   FontWindowScale;
    float                   FontWindowScaleParents;
    float                   FontRefSize;
    int                     SettingsOffset;

    ImDrawList*             DrawList;
    ImDrawList              DrawListInst;
    ImGuiWindow*            ParentWindow;
    ImGuiWindow*            ParentWindowInBeginStack;
    ImGuiWindow*            RootWindow;
    ImGuiWindow*            RootWindowPopupTree;
    ImGuiWindow*            RootWindowForTitleBarHighlight;
    ImGuiWindow*            RootWindowForNav;
    ImGuiWindow*            ParentWindowForFocusRoute;

    ImGuiWindow*            NavLastChildNavWindow;
    ImGuiID                 NavLastIds[ImGuiNavLayer_COUNT];
    ImRect                  NavRectRel[ImGuiNavLayer_COUNT];
    ImVec2                  NavPreferredScoringPosRel[ImGuiNavLayer_COUNT];
    ImGuiID                 NavRootFocusScopeId;

    int                     MemoryDrawListIdxCapacity;
    int                     MemoryDrawListVtxCapacity;
    bool                    MemoryCompacted;

public:
    ImGuiWindow(ImGuiContext* context, const char* name);
    ~ImGuiWindow();

    ImGuiID     GetID(const char* str, const char* str_end = NULL);
    ImGuiID     GetID(const void* ptr);
    ImGuiID     GetID(int n);
    ImGuiID     GetIDFromPos(const ImVec2& p_abs);
    ImGuiID     GetIDFromRectangle(const ImRect& r_abs);

    ImRect      Rect() const            { return ImRect(Pos.x, Pos.y, Pos.x + Size.x, Pos.y + Size.y); }
    ImRect      TitleBarRect() const    { return ImRect(Pos, ImVec2(Pos.x + SizeFull.x, Pos.y + TitleBarHeight)); }
    ImRect      MenuBarRect() const     { float y1 = Pos.y + TitleBarHeight; return ImRect(Pos.x, y1, Pos.x + SizeFull.x, y1 + MenuBarHeight); }

};

enum ImGuiTabBarFlagsPrivate_
{
    ImGuiTabBarFlags_DockNode                   = 1 << 20,
    ImGuiTabBarFlags_IsFocused                  = 1 << 21,
    ImGuiTabBarFlags_SaveSettings               = 1 << 22,
};

enum ImGuiTabItemFlagsPrivate_
{
    ImGuiTabItemFlags_SectionMask_              = ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_Trailing,
    ImGuiTabItemFlags_NoCloseButton             = 1 << 20,
    ImGuiTabItemFlags_Button                    = 1 << 21,
    ImGuiTabItemFlags_Invisible                 = 1 << 22,

};

struct ImGuiTabItem
{
    ImGuiID             ID;
    ImGuiTabItemFlags   Flags;
    int                 LastFrameVisible;
    int                 LastFrameSelected;
    float               Offset;
    float               Width;
    float               ContentWidth;
    float               RequestedWidth;
    ImS32               NameOffset;
    ImS16               BeginOrder;
    ImS16               IndexDuringLayout;
    bool                WantClose;

    ImGuiTabItem()      { memset((void*)this, 0, sizeof(*this)); LastFrameVisible = LastFrameSelected = -1; RequestedWidth = -1.0f; NameOffset = -1; BeginOrder = IndexDuringLayout = -1; }
};

struct IMGUI_API ImGuiTabBar
{
    ImGuiWindow*        Window;
    ImVector<ImGuiTabItem> Tabs;
    ImGuiTabBarFlags    Flags;
    ImGuiID             ID;
    ImGuiID             SelectedTabId;
    ImGuiID             NextSelectedTabId;
    ImGuiID             NextScrollToTabId;
    ImGuiID             VisibleTabId;
    int                 CurrFrameVisible;
    int                 PrevFrameVisible;
    ImRect              BarRect;
    float               BarRectPrevWidth;
    float               CurrTabsContentsHeight;
    float               PrevTabsContentsHeight;
    float               WidthAllTabs;
    float               WidthAllTabsIdeal;
    float               ScrollingAnim;
    float               ScrollingTarget;
    float               ScrollingTargetDistToVisibility;
    float               ScrollingSpeed;
    float               ScrollingRectMinX;
    float               ScrollingRectMaxX;
    float               SeparatorMinX;
    float               SeparatorMaxX;
    ImGuiID             ReorderRequestTabId;
    ImS16               ReorderRequestOffset;
    ImS8                BeginCount;
    bool                WantLayout;
    bool                VisibleTabWasSubmitted;
    bool                TabsAddedNew;
    bool                ScrollButtonEnabled;
    ImS16               TabsActiveCount;
    ImS16               LastTabItemIdx;
    float               ItemSpacingY;
    ImVec2              FramePadding;
    ImVec2              BackupCursorPos;
    ImGuiTextBuffer     TabsNames;

    ImGuiTabBar();
};

#define IM_COL32_DISABLE                IM_COL32(0,0,0,1)
#define IMGUI_TABLE_MAX_COLUMNS         512

struct ImGuiTableColumn
{
    ImGuiTableColumnFlags   Flags;
    float                   WidthGiven;
    float                   MinX;
    float                   MaxX;
    float                   WidthRequest;
    float                   WidthAuto;
    float                   WidthMax;
    float                   StretchWeight;
    float                   InitStretchWeightOrWidth;
    ImRect                  ClipRect;
    ImGuiID                 UserID;
    float                   WorkMinX;
    float                   WorkMaxX;
    float                   ItemWidth;
    float                   ContentMaxXFrozen;
    float                   ContentMaxXUnfrozen;
    float                   ContentMaxXHeadersUsed;
    float                   ContentMaxXHeadersIdeal;
    ImS16                   NameOffset;
    ImGuiTableColumnIdx     DisplayOrder;
    ImGuiTableColumnIdx     IndexWithinEnabledSet;
    ImGuiTableColumnIdx     PrevEnabledColumn;
    ImGuiTableColumnIdx     NextEnabledColumn;
    ImGuiTableColumnIdx     SortOrder;
    ImGuiTableDrawChannelIdx DrawChannelCurrent;
    ImGuiTableDrawChannelIdx DrawChannelFrozen;
    ImGuiTableDrawChannelIdx DrawChannelUnfrozen;
    bool                    IsEnabled;
    bool                    IsUserEnabled;
    bool                    IsUserEnabledNextFrame;
    bool                    IsVisibleX;
    bool                    IsVisibleY;
    bool                    IsRequestOutput;
    bool                    IsSkipItems;
    bool                    IsPreserveWidthAuto;
    ImS8                    NavLayerCurrent;
    ImU8                    AutoFitQueue;
    ImU8                    CannotSkipItemsQueue;
    ImU8                    SortDirection : 2;
    ImU8                    SortDirectionsAvailCount : 2;
    ImU8                    SortDirectionsAvailMask : 4;
    ImU8                    SortDirectionsAvailList;

    ImGuiTableColumn()
    {
        memset((void*)this, 0, sizeof(*this));
        StretchWeight = WidthRequest = -1.0f;
        NameOffset = -1;
        DisplayOrder = IndexWithinEnabledSet = -1;
        PrevEnabledColumn = NextEnabledColumn = -1;
        SortOrder = -1;
        SortDirection = ImGuiSortDirection_None;
        DrawChannelCurrent = DrawChannelFrozen = DrawChannelUnfrozen = (ImU8)-1;
    }
};

struct ImGuiTableCellData
{
    ImU32                       BgColor;
    ImGuiTableColumnIdx         Column;
};

struct ImGuiTableHeaderData
{
    ImGuiTableColumnIdx         Index;
    ImU32                       TextColor;
    ImU32                       BgColor0;
    ImU32                       BgColor1;
};

struct ImGuiTableInstanceData
{
    ImGuiID                     TableInstanceID;
    float                       LastOuterHeight;
    float                       LastTopHeadersRowHeight;
    float                       LastFrozenHeight;
    int                         HoveredRowLast;
    int                         HoveredRowNext;

    ImGuiTableInstanceData()    { TableInstanceID = 0; LastOuterHeight = LastTopHeadersRowHeight = LastFrozenHeight = 0.0f; HoveredRowLast = HoveredRowNext = -1; }
};

struct IMGUI_API ImGuiTable
{
    ImGuiID                     ID;
    ImGuiTableFlags             Flags;
    void*                       RawData;
    ImGuiTableTempData*         TempData;
    ImSpan<ImGuiTableColumn>    Columns;
    ImSpan<ImGuiTableColumnIdx> DisplayOrderToIndex;
    ImSpan<ImGuiTableCellData>  RowCellData;
    ImBitArrayPtr               EnabledMaskByDisplayOrder;
    ImBitArrayPtr               EnabledMaskByIndex;
    ImBitArrayPtr               VisibleMaskByIndex;
    ImGuiTableFlags             SettingsLoadedFlags;
    int                         SettingsOffset;
    int                         LastFrameActive;
    int                         ColumnsCount;
    int                         CurrentRow;
    int                         CurrentColumn;
    ImS16                       InstanceCurrent;
    ImS16                       InstanceInteracted;
    float                       RowPosY1;
    float                       RowPosY2;
    float                       RowMinHeight;
    float                       RowCellPaddingY;
    float                       RowTextBaseline;
    float                       RowIndentOffsetX;
    ImGuiTableRowFlags          RowFlags : 16;
    ImGuiTableRowFlags          LastRowFlags : 16;
    int                         RowBgColorCounter;
    ImU32                       RowBgColor[2];
    ImU32                       BorderColorStrong;
    ImU32                       BorderColorLight;
    float                       BorderX1;
    float                       BorderX2;
    float                       HostIndentX;
    float                       MinColumnWidth;
    float                       OuterPaddingX;
    float                       CellPaddingX;
    float                       CellSpacingX1;
    float                       CellSpacingX2;
    float                       InnerWidth;
    float                       ColumnsGivenWidth;
    float                       ColumnsAutoFitWidth;
    float                       ColumnsStretchSumWeights;
    float                       ResizedColumnNextWidth;
    float                       ResizeLockMinContentsX2;
    float                       RefScale;
    float                       AngledHeadersHeight;
    float                       AngledHeadersSlope;
    ImRect                      OuterRect;
    ImRect                      InnerRect;
    ImRect                      WorkRect;
    ImRect                      InnerClipRect;
    ImRect                      BgClipRect;
    ImRect                      Bg0ClipRectForDrawCmd;
    ImRect                      Bg2ClipRectForDrawCmd;
    ImRect                      HostClipRect;
    ImRect                      HostBackupInnerClipRect;
    ImGuiWindow*                OuterWindow;
    ImGuiWindow*                InnerWindow;
    ImGuiTextBuffer             ColumnsNames;
    ImDrawListSplitter*         DrawSplitter;
    ImGuiTableInstanceData      InstanceDataFirst;
    ImVector<ImGuiTableInstanceData>    InstanceDataExtra;
    ImGuiTableColumnSortSpecs   SortSpecsSingle;
    ImVector<ImGuiTableColumnSortSpecs> SortSpecsMulti;
    ImGuiTableSortSpecs         SortSpecs;
    ImGuiTableColumnIdx         SortSpecsCount;
    ImGuiTableColumnIdx         ColumnsEnabledCount;
    ImGuiTableColumnIdx         ColumnsEnabledFixedCount;
    ImGuiTableColumnIdx         DeclColumnsCount;
    ImGuiTableColumnIdx         AngledHeadersCount;
    ImGuiTableColumnIdx         HoveredColumnBody;
    ImGuiTableColumnIdx         HoveredColumnBorder;
    ImGuiTableColumnIdx         HighlightColumnHeader;
    ImGuiTableColumnIdx         AutoFitSingleColumn;
    ImGuiTableColumnIdx         ResizedColumn;
    ImGuiTableColumnIdx         LastResizedColumn;
    ImGuiTableColumnIdx         HeldHeaderColumn;
    ImGuiTableColumnIdx         LastHeldHeaderColumn;
    ImGuiTableColumnIdx         ReorderColumn;
    ImGuiTableColumnIdx         ReorderColumnDstOrder;
    ImGuiTableColumnIdx         LeftMostEnabledColumn;
    ImGuiTableColumnIdx         RightMostEnabledColumn;
    ImGuiTableColumnIdx         LeftMostStretchedColumn;
    ImGuiTableColumnIdx         RightMostStretchedColumn;
    ImGuiTableColumnIdx         ContextPopupColumn;
    ImGuiTableColumnIdx         FreezeRowsRequest;
    ImGuiTableColumnIdx         FreezeRowsCount;
    ImGuiTableColumnIdx         FreezeColumnsRequest;
    ImGuiTableColumnIdx         FreezeColumnsCount;
    ImGuiTableColumnIdx         RowCellDataCurrent;
    ImGuiTableDrawChannelIdx    DummyDrawChannel;
    ImGuiTableDrawChannelIdx    Bg2DrawChannelCurrent;
    ImGuiTableDrawChannelIdx    Bg2DrawChannelUnfrozen;
    ImS8                        NavLayer;
    bool                        IsLayoutLocked;
    bool                        IsInsideRow;
    bool                        IsInitializing;
    bool                        IsSortSpecsDirty;
    bool                        IsUsingHeaders;
    bool                        IsContextPopupOpen;
    bool                        DisableDefaultContextMenu;
    bool                        IsSettingsRequestLoad;
    bool                        IsSettingsDirty;
    bool                        IsDefaultDisplayOrder;
    bool                        IsResetAllRequest;
    bool                        IsResetDisplayOrderRequest;
    bool                        IsUnfrozenRows;
    bool                        IsDefaultSizingPolicy;
    bool                        IsActiveIdAliveBeforeTable;
    bool                        IsActiveIdInTable;
    bool                        HasScrollbarYCurr;
    bool                        HasScrollbarYPrev;
    bool                        MemoryCompacted;
    bool                        HostSkipItems;

    ImGuiTable()                { memset((void*)this, 0, sizeof(*this)); LastFrameActive = -1; }
    ~ImGuiTable()               { IM_FREE(RawData); }
};

struct IMGUI_API ImGuiTableTempData
{
    ImGuiID                     WindowID;
    int                         TableIndex;
    float                       LastTimeActive;
    float                       AngledHeadersExtraWidth;
    ImVector<ImGuiTableHeaderData> AngledHeadersRequests;

    ImVec2                      UserOuterSize;
    ImDrawListSplitter          DrawSplitter;

    ImRect                      HostBackupWorkRect;
    ImRect                      HostBackupParentWorkRect;
    ImVec2                      HostBackupPrevLineSize;
    ImVec2                      HostBackupCurrLineSize;
    ImVec2                      HostBackupCursorMaxPos;
    ImVec1                      HostBackupColumnsOffset;
    float                       HostBackupItemWidth;
    int                         HostBackupItemWidthStackSize;

    ImGuiTableTempData()        { memset((void*)this, 0, sizeof(*this)); LastTimeActive = -1.0f; }
};

struct ImGuiTableColumnSettings
{
    float                   WidthOrWeight;
    ImGuiID                 UserID;
    ImGuiTableColumnIdx     Index;
    ImGuiTableColumnIdx     DisplayOrder;
    ImGuiTableColumnIdx     SortOrder;
    ImU8                    SortDirection : 2;
    ImS8                    IsEnabled : 2;
    ImU8                    IsStretch : 1;

    ImGuiTableColumnSettings()
    {
        WidthOrWeight = 0.0f;
        UserID = 0;
        Index = -1;
        DisplayOrder = SortOrder = -1;
        SortDirection = ImGuiSortDirection_None;
        IsEnabled = -1;
        IsStretch = 0;
    }
};

struct ImGuiTableSettings
{
    ImGuiID                     ID;
    ImGuiTableFlags             SaveFlags;
    float                       RefScale;
    ImGuiTableColumnIdx         ColumnsCount;
    ImGuiTableColumnIdx         ColumnsCountMax;
    bool                        WantApply;

    ImGuiTableSettings()        { memset((void*)this, 0, sizeof(*this)); }
    ImGuiTableColumnSettings*   GetColumnSettings()     { return (ImGuiTableColumnSettings*)(this + 1); }
};

namespace ImGui
{

    IMGUI_API ImGuiIO&         GetIO(ImGuiContext* ctx);
    IMGUI_API ImGuiPlatformIO& GetPlatformIO(ImGuiContext* ctx);
    inline    float         GetScale()                  { ImGuiContext& g = *GImGui; return g.Style._MainScale; }
    inline    ImGuiWindow*  GetCurrentWindowRead()      { ImGuiContext& g = *GImGui; return g.CurrentWindow; }
    inline    ImGuiWindow*  GetCurrentWindow()          { ImGuiContext& g = *GImGui; g.CurrentWindow->WriteAccessed = true; return g.CurrentWindow; }
    IMGUI_API ImGuiWindow*  FindWindowByID(ImGuiID id);
    IMGUI_API ImGuiWindow*  FindWindowByName(const char* name);
    IMGUI_API void          UpdateWindowParentAndRootLinks(ImGuiWindow* window, ImGuiWindowFlags flags, ImGuiWindow* parent_window);
    IMGUI_API void          UpdateWindowSkipRefresh(ImGuiWindow* window);
    IMGUI_API ImVec2        CalcWindowNextAutoFitSize(ImGuiWindow* window);
    IMGUI_API bool          IsWindowChildOf(ImGuiWindow* window, ImGuiWindow* potential_parent, bool popup_hierarchy);
    IMGUI_API bool          IsWindowInBeginStack(ImGuiWindow* window);
    IMGUI_API bool          IsWindowWithinBeginStackOf(ImGuiWindow* window, ImGuiWindow* potential_parent);
    IMGUI_API bool          IsWindowAbove(ImGuiWindow* potential_above, ImGuiWindow* potential_below);
    IMGUI_API bool          IsWindowNavFocusable(ImGuiWindow* window);
    IMGUI_API void          SetWindowPos(ImGuiWindow* window, const ImVec2& pos, ImGuiCond cond = 0);
    IMGUI_API void          SetWindowSize(ImGuiWindow* window, const ImVec2& size, ImGuiCond cond = 0);
    IMGUI_API void          SetWindowCollapsed(ImGuiWindow* window, bool collapsed, ImGuiCond cond = 0);
    IMGUI_API void          SetWindowHitTestHole(ImGuiWindow* window, const ImVec2& pos, const ImVec2& size);
    IMGUI_API void          SetWindowHiddenAndSkipItemsForCurrentFrame(ImGuiWindow* window);
    inline void             SetWindowParentWindowForFocusRoute(ImGuiWindow* window, ImGuiWindow* parent_window) { window->ParentWindowForFocusRoute = parent_window; }
    inline ImRect           WindowRectAbsToRel(ImGuiWindow* window, const ImRect& r) { ImVec2 off = window->DC.CursorStartPos; return ImRect(r.Min.x - off.x, r.Min.y - off.y, r.Max.x - off.x, r.Max.y - off.y); }
    inline ImRect           WindowRectRelToAbs(ImGuiWindow* window, const ImRect& r) { ImVec2 off = window->DC.CursorStartPos; return ImRect(r.Min.x + off.x, r.Min.y + off.y, r.Max.x + off.x, r.Max.y + off.y); }
    inline ImVec2           WindowPosAbsToRel(ImGuiWindow* window, const ImVec2& p)  { ImVec2 off = window->DC.CursorStartPos; return ImVec2(p.x - off.x, p.y - off.y); }
    inline ImVec2           WindowPosRelToAbs(ImGuiWindow* window, const ImVec2& p)  { ImVec2 off = window->DC.CursorStartPos; return ImVec2(p.x + off.x, p.y + off.y); }

    IMGUI_API void          FocusWindow(ImGuiWindow* window, ImGuiFocusRequestFlags flags = 0);
    IMGUI_API void          FocusTopMostWindowUnderOne(ImGuiWindow* under_this_window, ImGuiWindow* ignore_window, ImGuiViewport* filter_viewport, ImGuiFocusRequestFlags flags);
    IMGUI_API void          BringWindowToFocusFront(ImGuiWindow* window);
    IMGUI_API void          BringWindowToDisplayFront(ImGuiWindow* window);
    IMGUI_API void          BringWindowToDisplayBack(ImGuiWindow* window);
    IMGUI_API void          BringWindowToDisplayBehind(ImGuiWindow* window, ImGuiWindow* above_window);
    IMGUI_API int           FindWindowDisplayIndex(ImGuiWindow* window);
    IMGUI_API ImGuiWindow*  FindBottomMostVisibleWindowWithinBeginStack(ImGuiWindow* window);

    IMGUI_API void          SetNextWindowRefreshPolicy(ImGuiWindowRefreshFlags flags);

    IMGUI_API void          RegisterUserTexture(ImTextureData* tex);
    IMGUI_API void          UnregisterUserTexture(ImTextureData* tex);
    IMGUI_API void          RegisterFontAtlas(ImFontAtlas* atlas);
    IMGUI_API void          UnregisterFontAtlas(ImFontAtlas* atlas);
    IMGUI_API void          SetCurrentFont(ImFont* font, float font_size_before_scaling, float font_size_after_scaling);
    IMGUI_API void          UpdateCurrentFontSize(float restore_font_size_after_scaling);
    IMGUI_API void          SetFontRasterizerDensity(float rasterizer_density);
    inline float            GetFontRasterizerDensity() { return GImGui->FontRasterizerDensity; }
    inline float            GetRoundedFontSize(float size) { return IM_ROUND(size); }
    IMGUI_API ImFont*       GetDefaultFont();
    IMGUI_API void          PushPasswordFont();
    IMGUI_API void          PopPasswordFont();
    inline ImDrawList*      GetForegroundDrawList(ImGuiWindow* window) { IM_UNUSED(window); return GetForegroundDrawList(); }
    IMGUI_API ImDrawList*   GetBackgroundDrawList(ImGuiViewport* viewport);
    IMGUI_API ImDrawList*   GetForegroundDrawList(ImGuiViewport* viewport);
    IMGUI_API void          AddDrawListToDrawDataEx(ImDrawData* draw_data, ImVector<ImDrawList*>* out_list, ImDrawList* draw_list);

    IMGUI_API void          Initialize();
    IMGUI_API void          Shutdown();

    IMGUI_API void          SetContextName(ImGuiContext* ctx, const char* name);
    IMGUI_API ImGuiID       AddContextHook(ImGuiContext* ctx, const ImGuiContextHook* hook);
    IMGUI_API void          RemoveContextHook(ImGuiContext* ctx, ImGuiID hook_to_remove);
    IMGUI_API void          CallContextHooks(ImGuiContext* ctx, ImGuiContextHookType type);

    IMGUI_API void          UpdateInputEvents(bool trickle_fast_inputs);
    IMGUI_API void          UpdateHoveredWindowAndCaptureFlags(const ImVec2& mouse_pos);
    IMGUI_API void          FindHoveredWindowEx(const ImVec2& pos, bool find_first_and_in_any_viewport, ImGuiWindow** out_hovered_window, ImGuiWindow** out_hovered_window_under_moving_window);
    IMGUI_API void          StartMouseMovingWindow(ImGuiWindow* window);
    IMGUI_API void          StopMouseMovingWindow();
    IMGUI_API void          UpdateMouseMovingWindowNewFrame();
    IMGUI_API void          UpdateMouseMovingWindowEndFrame();

    inline ImGuiViewport*   GetWindowViewport() { return GetMainViewport(); }
    IMGUI_API void          ScaleWindowsInViewport(ImGuiViewportP* viewport, float scale);
    IMGUI_API void          SetWindowViewport(ImGuiWindow* window, ImGuiViewportP* viewport);

    IMGUI_API void                  MarkIniSettingsDirty();
    IMGUI_API void                  MarkIniSettingsDirty(ImGuiWindow* window);
    IMGUI_API void                  ClearIniSettings();
    IMGUI_API void                  AddSettingsHandler(const ImGuiSettingsHandler* handler);
    IMGUI_API void                  RemoveSettingsHandler(const char* type_name);
    IMGUI_API ImGuiSettingsHandler* FindSettingsHandler(const char* type_name);

    IMGUI_API ImGuiWindowSettings*  CreateNewWindowSettings(const char* name);
    IMGUI_API ImGuiWindowSettings*  FindWindowSettingsByID(ImGuiID id);
    IMGUI_API ImGuiWindowSettings*  FindWindowSettingsByWindow(ImGuiWindow* window);
    IMGUI_API void                  ClearWindowSettings(const char* name);

    IMGUI_API void          LocalizeRegisterEntries(const ImGuiLocEntry* entries, int count);
    inline const char*      LocalizeGetMsg(ImGuiLocKey key) { ImGuiContext& g = *GImGui; const char* msg = g.LocalizationTable[key]; return msg ? msg : "*Missing Text*"; }

    IMGUI_API void          SetScrollX(ImGuiWindow* window, float scroll_x);
    IMGUI_API void          SetScrollY(ImGuiWindow* window, float scroll_y);
    IMGUI_API void          SetScrollFromPosX(ImGuiWindow* window, float local_x, float center_x_ratio);
    IMGUI_API void          SetScrollFromPosY(ImGuiWindow* window, float local_y, float center_y_ratio);

    IMGUI_API void          ScrollToItem(ImGuiScrollFlags flags = 0);
    IMGUI_API void          ScrollToRect(ImGuiWindow* window, const ImRect& rect, ImGuiScrollFlags flags = 0);
    IMGUI_API ImVec2        ScrollToRectEx(ImGuiWindow* window, const ImRect& rect, ImGuiScrollFlags flags = 0);

    inline void             ScrollToBringRectIntoView(ImGuiWindow* window, const ImRect& rect) { ScrollToRect(window, rect, ImGuiScrollFlags_KeepVisibleEdgeY); }

    inline ImGuiItemStatusFlags GetItemStatusFlags() { ImGuiContext& g = *GImGui; return g.LastItemData.StatusFlags; }
    inline ImGuiID          GetActiveID()   { ImGuiContext& g = *GImGui; return g.ActiveId; }
    inline ImGuiID          GetFocusID()    { ImGuiContext& g = *GImGui; return g.NavId; }
    IMGUI_API void          SetActiveID(ImGuiID id, ImGuiWindow* window);
    IMGUI_API void          SetFocusID(ImGuiID id, ImGuiWindow* window);
    IMGUI_API void          ClearActiveID();
    IMGUI_API ImGuiID       GetHoveredID();
    IMGUI_API void          SetHoveredID(ImGuiID id);
    IMGUI_API void          KeepAliveID(ImGuiID id);
    IMGUI_API void          MarkItemEdited(ImGuiID id);
    IMGUI_API void          PushOverrideID(ImGuiID id);
    IMGUI_API ImGuiID       GetIDWithSeed(const char* str_id_begin, const char* str_id_end, ImGuiID seed);
    IMGUI_API ImGuiID       GetIDWithSeed(int n, ImGuiID seed);

    IMGUI_API void          ItemSize(const ImVec2& size, float text_baseline_y = -1.0f);
    inline void             ItemSize(const ImRect& bb, float text_baseline_y = -1.0f) { ItemSize(bb.GetSize(), text_baseline_y); }
    IMGUI_API bool          ItemAdd(const ImRect& bb, ImGuiID id, const ImRect* nav_bb = NULL, ImGuiItemFlags extra_flags = 0);
    IMGUI_API bool          ItemHoverable(const ImRect& bb, ImGuiID id, ImGuiItemFlags item_flags);
    IMGUI_API bool          IsWindowContentHoverable(ImGuiWindow* window, ImGuiHoveredFlags flags = 0);
    IMGUI_API bool          IsClippedEx(const ImRect& bb, ImGuiID id);
    IMGUI_API void          SetLastItemData(ImGuiID item_id, ImGuiItemFlags item_flags, ImGuiItemStatusFlags status_flags, const ImRect& item_rect);
    IMGUI_API ImVec2        CalcItemSize(ImVec2 size, float default_w, float default_h);
    IMGUI_API float         CalcWrapWidthForPos(const ImVec2& pos, float wrap_pos_x);
    IMGUI_API void          PushMultiItemsWidths(int components, float width_full);
    IMGUI_API void          ShrinkWidths(ImGuiShrinkWidthItem* items, int count, float width_excess, float width_min);
    IMGUI_API void          CalcClipRectVisibleItemsY(const ImRect& clip_rect, const ImVec2& pos, float items_height, int* out_visible_start, int* out_visible_end);

    IMGUI_API const ImGuiStyleVarInfo* GetStyleVarInfo(ImGuiStyleVar idx);
    IMGUI_API void          BeginDisabledOverrideReenable();
    IMGUI_API void          EndDisabledOverrideReenable();

    IMGUI_API void          LogBegin(ImGuiLogFlags flags, int auto_open_depth);
    IMGUI_API void          LogToBuffer(int auto_open_depth = -1);
    IMGUI_API void          LogRenderedText(const ImVec2* ref_pos, const char* text, const char* text_end = NULL);
    IMGUI_API void          LogSetNextTextDecoration(const char* prefix, const char* suffix);

    IMGUI_API bool          BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);

    IMGUI_API bool          BeginPopupEx(ImGuiID id, ImGuiWindowFlags extra_window_flags);
    IMGUI_API bool          BeginPopupMenuEx(ImGuiID id, const char* label, ImGuiWindowFlags extra_window_flags);
    IMGUI_API void          OpenPopupEx(ImGuiID id, ImGuiPopupFlags popup_flags = ImGuiPopupFlags_None);
    IMGUI_API void          ClosePopupToLevel(int remaining, bool restore_focus_to_window_under_popup);
    IMGUI_API void          ClosePopupsOverWindow(ImGuiWindow* ref_window, bool restore_focus_to_window_under_popup);
    IMGUI_API void          ClosePopupsExceptModals();
    IMGUI_API bool          IsPopupOpen(ImGuiID id, ImGuiPopupFlags popup_flags);
    IMGUI_API ImRect        GetPopupAllowedExtentRect(ImGuiWindow* window);
    IMGUI_API ImGuiWindow*  GetTopMostPopupModal();
    IMGUI_API ImGuiWindow*  GetTopMostAndVisiblePopupModal();
    IMGUI_API ImGuiWindow*  FindBlockingModal(ImGuiWindow* window);
    IMGUI_API ImVec2        FindBestWindowPosForPopup(ImGuiWindow* window);
    IMGUI_API ImVec2        FindBestWindowPosForPopupEx(const ImVec2& ref_pos, const ImVec2& size, ImGuiDir* last_dir, const ImRect& r_outer, const ImRect& r_avoid, ImGuiPopupPositionPolicy policy);
    IMGUI_API ImGuiMouseButton GetMouseButtonFromPopupFlags(ImGuiPopupFlags flags);
    IMGUI_API bool          IsPopupOpenRequestForItem(ImGuiPopupFlags flags, ImGuiID id);
    IMGUI_API bool          IsPopupOpenRequestForWindow(ImGuiPopupFlags flags);

    IMGUI_API bool          BeginTooltipEx(ImGuiTooltipFlags tooltip_flags, ImGuiWindowFlags extra_window_flags);
    IMGUI_API bool          BeginTooltipHidden();

    IMGUI_API bool          BeginViewportSideBar(const char* name, ImGuiViewport* viewport, ImGuiDir dir, float size, ImGuiWindowFlags window_flags);
    IMGUI_API bool          BeginMenuEx(const char* label, const char* icon, bool enabled = true);
    IMGUI_API bool          MenuItemEx(const char* label, const char* icon, const char* shortcut = NULL, bool selected = false, bool enabled = true);

    IMGUI_API bool          BeginComboPopup(ImGuiID popup_id, const ImRect& bb, ImGuiComboFlags flags);
    IMGUI_API bool          BeginComboPreview();
    IMGUI_API void          EndComboPreview();

    IMGUI_API void          NavInitWindow(ImGuiWindow* window, bool force_reinit);
    IMGUI_API void          NavInitRequestApplyResult();
    IMGUI_API bool          NavMoveRequestButNoResultYet();
    IMGUI_API void          NavMoveRequestSubmit(ImGuiDir move_dir, ImGuiDir clip_dir, ImGuiNavMoveFlags move_flags, ImGuiScrollFlags scroll_flags);
    IMGUI_API void          NavMoveRequestForward(ImGuiDir move_dir, ImGuiDir clip_dir, ImGuiNavMoveFlags move_flags, ImGuiScrollFlags scroll_flags);
    IMGUI_API void          NavMoveRequestResolveWithLastItem(ImGuiNavItemData* result);
    IMGUI_API void          NavMoveRequestResolveWithPastTreeNode(ImGuiNavItemData* result, const ImGuiTreeNodeStackData* tree_node_data);
    IMGUI_API void          NavMoveRequestCancel();
    IMGUI_API void          NavMoveRequestApplyResult();
    IMGUI_API void          NavMoveRequestTryWrapping(ImGuiWindow* window, ImGuiNavMoveFlags move_flags);
    IMGUI_API void          NavHighlightActivated(ImGuiID id);
    IMGUI_API void          NavClearPreferredPosForAxis(ImGuiAxis axis);
    IMGUI_API void          SetNavCursorVisibleAfterMove();
    IMGUI_API void          NavUpdateCurrentWindowIsScrollPushableX();
    IMGUI_API void          SetNavWindow(ImGuiWindow* window);
    IMGUI_API void          SetNavID(ImGuiID id, ImGuiNavLayer nav_layer, ImGuiID focus_scope_id, const ImRect& rect_rel);
    IMGUI_API void          SetNavFocusScope(ImGuiID focus_scope_id);

    IMGUI_API void          FocusItem();
    IMGUI_API void          ActivateItemByID(ImGuiID id);

    inline bool             IsNamedKey(ImGuiKey key)                    { return key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END; }
    inline bool             IsNamedKeyOrMod(ImGuiKey key)               { return (key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END) || key == ImGuiMod_Ctrl || key == ImGuiMod_Shift || key == ImGuiMod_Alt || key == ImGuiMod_Super; }
    inline bool             IsLegacyKey(ImGuiKey key)                   { return key >= ImGuiKey_LegacyNativeKey_BEGIN && key < ImGuiKey_LegacyNativeKey_END; }
    inline bool             IsKeyboardKey(ImGuiKey key)                 { return key >= ImGuiKey_Keyboard_BEGIN && key < ImGuiKey_Keyboard_END; }
    inline bool             IsGamepadKey(ImGuiKey key)                  { return key >= ImGuiKey_Gamepad_BEGIN && key < ImGuiKey_Gamepad_END; }
    inline bool             IsMouseKey(ImGuiKey key)                    { return key >= ImGuiKey_Mouse_BEGIN && key < ImGuiKey_Mouse_END; }
    inline bool             IsAliasKey(ImGuiKey key)                    { return key >= ImGuiKey_Aliases_BEGIN && key < ImGuiKey_Aliases_END; }
    inline bool             IsLRModKey(ImGuiKey key)                    { return key >= ImGuiKey_LeftCtrl && key <= ImGuiKey_RightSuper; }
    ImGuiKeyChord           FixupKeyChord(ImGuiKeyChord key_chord);
    inline ImGuiKey         ConvertSingleModFlagToKey(ImGuiKey key)
    {
        if (key == ImGuiMod_Ctrl) return ImGuiKey_ReservedForModCtrl;
        if (key == ImGuiMod_Shift) return ImGuiKey_ReservedForModShift;
        if (key == ImGuiMod_Alt) return ImGuiKey_ReservedForModAlt;
        if (key == ImGuiMod_Super) return ImGuiKey_ReservedForModSuper;
        return key;
    }

    IMGUI_API ImGuiKeyData* GetKeyData(ImGuiContext* ctx, ImGuiKey key);
    inline ImGuiKeyData*    GetKeyData(ImGuiKey key)                                    { ImGuiContext& g = *GImGui; return GetKeyData(&g, key); }
    IMGUI_API const char*   GetKeyChordName(ImGuiKeyChord key_chord);
    inline ImGuiKey         MouseButtonToKey(ImGuiMouseButton button)                   { IM_ASSERT(button >= 0 && button < ImGuiMouseButton_COUNT); return (ImGuiKey)(ImGuiKey_MouseLeft + button); }
    IMGUI_API bool          IsMouseDragPastThreshold(ImGuiMouseButton button, float lock_threshold = -1.0f);
    IMGUI_API ImVec2        GetKeyMagnitude2d(ImGuiKey key_left, ImGuiKey key_right, ImGuiKey key_up, ImGuiKey key_down);
    IMGUI_API float         GetNavTweakPressedAmount(ImGuiAxis axis);
    IMGUI_API int           CalcTypematicRepeatAmount(float t0, float t1, float repeat_delay, float repeat_rate);
    IMGUI_API void          GetTypematicRepeatRate(ImGuiInputFlags flags, float* repeat_delay, float* repeat_rate);
    IMGUI_API void          TeleportMousePos(const ImVec2& pos);
    IMGUI_API void          SetActiveIdUsingAllKeyboardKeys();
    inline bool             IsActiveIdUsingNavDir(ImGuiDir dir)                         { ImGuiContext& g = *GImGui; return (g.ActiveIdUsingNavDirMask & (1 << dir)) != 0; }

    IMGUI_API ImGuiID       GetKeyOwner(ImGuiKey key);
    IMGUI_API void          SetKeyOwner(ImGuiKey key, ImGuiID owner_id, ImGuiInputFlags flags = 0);
    IMGUI_API void          SetKeyOwnersForKeyChord(ImGuiKeyChord key, ImGuiID owner_id, ImGuiInputFlags flags = 0);
    IMGUI_API void          SetItemKeyOwner(ImGuiKey key, ImGuiInputFlags flags);
    IMGUI_API bool          TestKeyOwner(ImGuiKey key, ImGuiID owner_id);
    inline ImGuiKeyOwnerData* GetKeyOwnerData(ImGuiContext* ctx, ImGuiKey key)          { if (key & ImGuiMod_Mask_) key = ConvertSingleModFlagToKey(key); IM_ASSERT(IsNamedKey(key)); return &ctx->KeysOwnerData[key - ImGuiKey_NamedKey_BEGIN]; }

    IMGUI_API bool          IsKeyDown(ImGuiKey key, ImGuiID owner_id);
    IMGUI_API bool          IsKeyPressed(ImGuiKey key, ImGuiInputFlags flags, ImGuiID owner_id = 0);
    IMGUI_API bool          IsKeyReleased(ImGuiKey key, ImGuiID owner_id);
    IMGUI_API bool          IsKeyChordPressed(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id = 0);
    IMGUI_API bool          IsMouseDown(ImGuiMouseButton button, ImGuiID owner_id);
    IMGUI_API bool          IsMouseClicked(ImGuiMouseButton button, ImGuiInputFlags flags, ImGuiID owner_id = 0);
    IMGUI_API bool          IsMouseReleased(ImGuiMouseButton button, ImGuiID owner_id);
    IMGUI_API bool          IsMouseDoubleClicked(ImGuiMouseButton button, ImGuiID owner_id);

    IMGUI_API bool          Shortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id);
    IMGUI_API bool          SetShortcutRouting(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id);
    IMGUI_API bool          TestShortcutRouting(ImGuiKeyChord key_chord, ImGuiID owner_id);
    IMGUI_API ImGuiKeyRoutingData* GetShortcutRoutingData(ImGuiKeyChord key_chord);

    IMGUI_API void          PushFocusScope(ImGuiID id);
    IMGUI_API void          PopFocusScope();
    inline ImGuiID          GetCurrentFocusScope() { ImGuiContext& g = *GImGui; return g.CurrentFocusScopeId; }

    IMGUI_API bool          IsDragDropActive();
    IMGUI_API bool          BeginDragDropTargetCustom(const ImRect& bb, ImGuiID id);
    IMGUI_API bool          BeginDragDropTargetViewport(ImGuiViewport* viewport, const ImRect* p_bb = NULL);
    IMGUI_API void          ClearDragDrop();
    IMGUI_API bool          IsDragDropPayloadBeingAccepted();
    IMGUI_API void          RenderDragDropTargetRectForItem(const ImRect& bb);
    IMGUI_API void          RenderDragDropTargetRectEx(ImDrawList* draw_list, const ImRect& bb);

    IMGUI_API ImGuiTypingSelectRequest* GetTypingSelectRequest(ImGuiTypingSelectFlags flags = ImGuiTypingSelectFlags_None);
    IMGUI_API int           TypingSelectFindMatch(ImGuiTypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data, int nav_item_idx);
    IMGUI_API int           TypingSelectFindNextSingleCharMatch(ImGuiTypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data, int nav_item_idx);
    IMGUI_API int           TypingSelectFindBestLeadingMatch(ImGuiTypingSelectRequest* req, int items_count, const char* (*get_item_name_func)(void*, int), void* user_data);

    IMGUI_API bool          BeginBoxSelect(const ImRect& scope_rect, ImGuiWindow* window, ImGuiID box_select_id, ImGuiMultiSelectFlags ms_flags);
    IMGUI_API void          EndBoxSelect(const ImRect& scope_rect, ImGuiMultiSelectFlags ms_flags);

    IMGUI_API void          MultiSelectItemHeader(ImGuiID id, bool* p_selected, ImGuiButtonFlags* p_button_flags);
    IMGUI_API void          MultiSelectItemFooter(ImGuiID id, bool* p_selected, bool* p_pressed);
    IMGUI_API void          MultiSelectAddSetAll(ImGuiMultiSelectTempData* ms, bool selected);
    IMGUI_API void          MultiSelectAddSetRange(ImGuiMultiSelectTempData* ms, bool selected, int range_dir, ImGuiSelectionUserData first_item, ImGuiSelectionUserData last_item);
    inline ImGuiBoxSelectState*     GetBoxSelectState(ImGuiID id)   { ImGuiContext& g = *GImGui; return (id != 0 && g.BoxSelectState.ID == id && g.BoxSelectState.IsActive) ? &g.BoxSelectState : NULL; }
    inline ImGuiMultiSelectState*   GetMultiSelectState(ImGuiID id) { ImGuiContext& g = *GImGui; return g.MultiSelectStorage.GetByKey(id); }

    IMGUI_API void          SetWindowClipRectBeforeSetChannel(ImGuiWindow* window, const ImRect& clip_rect);
    IMGUI_API void          BeginColumns(const char* str_id, int count, ImGuiOldColumnFlags flags = 0);
    IMGUI_API void          EndColumns();
    IMGUI_API void          PushColumnClipRect(int column_index);
    IMGUI_API void          PushColumnsBackground();
    IMGUI_API void          PopColumnsBackground();
    IMGUI_API ImGuiID       GetColumnsID(const char* str_id, int count);
    IMGUI_API ImGuiOldColumns* FindOrCreateColumns(ImGuiWindow* window, ImGuiID id);
    IMGUI_API float         GetColumnOffsetFromNorm(const ImGuiOldColumns* columns, float offset_norm);
    IMGUI_API float         GetColumnNormFromOffset(const ImGuiOldColumns* columns, float offset);

    IMGUI_API void          TableOpenContextMenu(int column_n = -1);
    IMGUI_API void          TableSetColumnWidth(int column_n, float width);
    IMGUI_API void          TableSetColumnSortDirection(int column_n, ImGuiSortDirection sort_direction, bool append_to_sort_specs);
    IMGUI_API int           TableGetHoveredRow();
    IMGUI_API float         TableGetHeaderRowHeight();
    IMGUI_API float         TableGetHeaderAngledMaxLabelWidth();
    IMGUI_API void          TablePushBackgroundChannel();
    IMGUI_API void          TablePopBackgroundChannel();
    IMGUI_API void          TablePushColumnChannel(int column_n);
    IMGUI_API void          TablePopColumnChannel();
    IMGUI_API void          TableAngledHeadersRowEx(ImGuiID row_id, float angle, float max_label_width, const ImGuiTableHeaderData* data, int data_count);

    inline    ImGuiTable*   GetCurrentTable() { ImGuiContext& g = *GImGui; return g.CurrentTable; }
    IMGUI_API ImGuiTable*   TableFindByID(ImGuiID id);
    IMGUI_API bool          BeginTableEx(const char* name, ImGuiID id, int columns_count, ImGuiTableFlags flags = 0, const ImVec2& outer_size = ImVec2(0, 0), float inner_width = 0.0f);
    IMGUI_API void          TableBeginInitMemory(ImGuiTable* table, int columns_count);
    IMGUI_API void          TableBeginApplyRequests(ImGuiTable* table);
    IMGUI_API void          TableSetupDrawChannels(ImGuiTable* table);
    IMGUI_API void          TableUpdateLayout(ImGuiTable* table);
    IMGUI_API void          TableUpdateBorders(ImGuiTable* table);
    IMGUI_API void          TableUpdateColumnsWeightFromWidth(ImGuiTable* table);
    IMGUI_API void          TableDrawBorders(ImGuiTable* table);
    IMGUI_API void          TableDrawDefaultContextMenu(ImGuiTable* table, ImGuiTableFlags flags_for_section_to_display);
    IMGUI_API bool          TableBeginContextMenuPopup(ImGuiTable* table);
    IMGUI_API void          TableMergeDrawChannels(ImGuiTable* table);
    inline ImGuiTableInstanceData*  TableGetInstanceData(ImGuiTable* table, int instance_no) { if (instance_no == 0) return &table->InstanceDataFirst; return &table->InstanceDataExtra[instance_no - 1]; }
    inline ImGuiID                  TableGetInstanceID(ImGuiTable* table, int instance_no)   { return TableGetInstanceData(table, instance_no)->TableInstanceID; }
    IMGUI_API void          TableFixDisplayOrder(ImGuiTable* table);
    IMGUI_API void          TableSortSpecsSanitize(ImGuiTable* table);
    IMGUI_API void          TableSortSpecsBuild(ImGuiTable* table);
    IMGUI_API ImGuiSortDirection TableGetColumnNextSortDirection(ImGuiTableColumn* column);
    IMGUI_API void          TableFixColumnSortDirection(ImGuiTable* table, ImGuiTableColumn* column);
    IMGUI_API float         TableGetColumnWidthAuto(ImGuiTable* table, ImGuiTableColumn* column);
    IMGUI_API void          TableBeginRow(ImGuiTable* table);
    IMGUI_API void          TableEndRow(ImGuiTable* table);
    IMGUI_API void          TableBeginCell(ImGuiTable* table, int column_n);
    IMGUI_API void          TableEndCell(ImGuiTable* table);
    IMGUI_API ImRect        TableGetCellBgRect(const ImGuiTable* table, int column_n);
    IMGUI_API const char*   TableGetColumnName(const ImGuiTable* table, int column_n);
    IMGUI_API ImGuiID       TableGetColumnResizeID(ImGuiTable* table, int column_n, int instance_no = 0);
    IMGUI_API float         TableCalcMaxColumnWidth(const ImGuiTable* table, int column_n);
    IMGUI_API void          TableSetColumnWidthAutoSingle(ImGuiTable* table, int column_n);
    IMGUI_API void          TableSetColumnWidthAutoAll(ImGuiTable* table);
    IMGUI_API void          TableSetColumnDisplayOrder(ImGuiTable* table, int column_n, int dst_order);
    IMGUI_API void          TableQueueSetColumnDisplayOrder(ImGuiTable* table, int column_n, int dst_order);
    IMGUI_API void          TableRemove(ImGuiTable* table);
    IMGUI_API void          TableGcCompactTransientBuffers(ImGuiTable* table);
    IMGUI_API void          TableGcCompactTransientBuffers(ImGuiTableTempData* table);
    IMGUI_API void          TableGcCompactSettings();

    IMGUI_API void                  TableLoadSettings(ImGuiTable* table);
    IMGUI_API void                  TableSaveSettings(ImGuiTable* table);
    IMGUI_API void                  TableResetSettings(ImGuiTable* table);
    IMGUI_API ImGuiTableSettings*   TableGetBoundSettings(ImGuiTable* table);
    IMGUI_API void                  TableSettingsAddSettingsHandler();
    IMGUI_API ImGuiTableSettings*   TableSettingsCreate(ImGuiID id, int columns_count);
    IMGUI_API ImGuiTableSettings*   TableSettingsFindByID(ImGuiID id);

    inline    ImGuiTabBar*  GetCurrentTabBar() { ImGuiContext& g = *GImGui; return g.CurrentTabBar; }
    IMGUI_API ImGuiTabBar*  TabBarFindByID(ImGuiID id);
    IMGUI_API void          TabBarRemove(ImGuiTabBar* tab_bar);
    IMGUI_API bool          BeginTabBarEx(ImGuiTabBar* tab_bar, const ImRect& bb, ImGuiTabBarFlags flags);
    IMGUI_API ImGuiTabItem* TabBarFindTabByID(ImGuiTabBar* tab_bar, ImGuiID tab_id);
    IMGUI_API ImGuiTabItem* TabBarFindTabByOrder(ImGuiTabBar* tab_bar, int order);
    IMGUI_API ImGuiTabItem* TabBarGetCurrentTab(ImGuiTabBar* tab_bar);
    inline int              TabBarGetTabOrder(ImGuiTabBar* tab_bar, ImGuiTabItem* tab) { return tab_bar->Tabs.index_from_ptr(tab); }
    IMGUI_API const char*   TabBarGetTabName(ImGuiTabBar* tab_bar, ImGuiTabItem* tab);
    IMGUI_API void          TabBarRemoveTab(ImGuiTabBar* tab_bar, ImGuiID tab_id);
    IMGUI_API void          TabBarCloseTab(ImGuiTabBar* tab_bar, ImGuiTabItem* tab);
    IMGUI_API void          TabBarQueueFocus(ImGuiTabBar* tab_bar, ImGuiTabItem* tab);
    IMGUI_API void          TabBarQueueFocus(ImGuiTabBar* tab_bar, const char* tab_name);
    IMGUI_API void          TabBarQueueReorder(ImGuiTabBar* tab_bar, ImGuiTabItem* tab, int offset);
    IMGUI_API void          TabBarQueueReorderFromMousePos(ImGuiTabBar* tab_bar, ImGuiTabItem* tab, ImVec2 mouse_pos);
    IMGUI_API bool          TabBarProcessReorder(ImGuiTabBar* tab_bar);
    IMGUI_API bool          TabItemEx(ImGuiTabBar* tab_bar, const char* label, bool* p_open, ImGuiTabItemFlags flags, ImGuiWindow* docked_window);
    IMGUI_API void          TabItemSpacing(const char* str_id, ImGuiTabItemFlags flags, float width);
    IMGUI_API ImVec2        TabItemCalcSize(const char* label, bool has_close_button_or_unsaved_marker);
    IMGUI_API ImVec2        TabItemCalcSize(ImGuiWindow* window);
    IMGUI_API void          TabItemBackground(ImDrawList* draw_list, const ImRect& bb, ImGuiTabItemFlags flags, ImU32 col);
    IMGUI_API void          TabItemLabelAndCloseButton(ImDrawList* draw_list, const ImRect& bb, ImGuiTabItemFlags flags, ImVec2 frame_padding, const char* label, ImGuiID tab_id, ImGuiID close_button_id, bool is_contents_visible, bool* out_just_closed, bool* out_text_clipped);

    IMGUI_API void          RenderText(ImVec2 pos, const char* text, const char* text_end = NULL, bool hide_text_after_hash = true);
    IMGUI_API void          RenderTextWrapped(ImVec2 pos, const char* text, const char* text_end, float wrap_width);
    IMGUI_API void          RenderTextClipped(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align = ImVec2(0, 0), const ImRect* clip_rect = NULL);
    IMGUI_API void          RenderTextClippedEx(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align = ImVec2(0, 0), const ImRect* clip_rect = NULL);
    IMGUI_API void          RenderTextEllipsis(ImDrawList* draw_list, const ImVec2& pos_min, const ImVec2& pos_max, float ellipsis_max_x, const char* text, const char* text_end, const ImVec2* text_size_if_known);
    IMGUI_API void          RenderFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool borders = true, float rounding = 0.0f);
    IMGUI_API void          RenderFrameBorder(ImVec2 p_min, ImVec2 p_max, float rounding = 0.0f);
    IMGUI_API void          RenderColorComponentMarker(const ImRect& bb, ImU32 col, float rounding);
    IMGUI_API void          RenderColorRectWithAlphaCheckerboard(ImDrawList* draw_list, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, float grid_step, ImVec2 grid_off, float rounding = 0.0f, ImDrawFlags flags = 0);
    IMGUI_API void          RenderNavCursor(const ImRect& bb, ImGuiID id, ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None);
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    inline    void          RenderNavHighlight(const ImRect& bb, ImGuiID id, ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None) { RenderNavCursor(bb, id, flags); }
#endif
    IMGUI_API const char*   FindRenderedTextEnd(const char* text, const char* text_end = NULL);
    IMGUI_API void          RenderMouseCursor(ImVec2 pos, float scale, ImGuiMouseCursor mouse_cursor, ImU32 col_fill, ImU32 col_border, ImU32 col_shadow);

    IMGUI_API void          RenderArrow(ImDrawList* draw_list, ImVec2 pos, ImU32 col, ImGuiDir dir, float scale = 1.0f);
    IMGUI_API void          RenderBullet(ImDrawList* draw_list, ImVec2 pos, ImU32 col);
    IMGUI_API void          RenderCheckMark(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz);
    IMGUI_API void          RenderArrowPointingAt(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col);
    IMGUI_API void          RenderRectFilledInRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float fill_x0, float fill_x1, float rounding);
    IMGUI_API void          RenderRectFilledWithHole(ImDrawList* draw_list, const ImRect& outer, const ImRect& inner, ImU32 col, float rounding);
    IMGUI_API ImDrawFlags   CalcRoundingFlagsForRectInRect(const ImRect& r_in, const ImRect& r_outer, float threshold);

    IMGUI_API void          TextEx(const char* text, const char* text_end = NULL, ImGuiTextFlags flags = 0);
    IMGUI_API void          TextAligned(float align_x, float size_x, const char* fmt, ...);
    IMGUI_API void          TextAlignedV(float align_x, float size_x, const char* fmt, va_list args);

    IMGUI_API bool          ButtonEx(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = 0);
    IMGUI_API bool          ArrowButtonEx(const char* str_id, ImGuiDir dir, ImVec2 size_arg, ImGuiButtonFlags flags = 0);
    IMGUI_API bool          ImageButtonEx(ImGuiID id, ImTextureRef tex_ref, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col, ImGuiButtonFlags flags = 0);
    IMGUI_API void          SeparatorEx(ImGuiSeparatorFlags flags, float thickness = 1.0f);
    IMGUI_API void          SeparatorTextEx(ImGuiID id, const char* label, const char* label_end, float extra_width);
    IMGUI_API bool          CheckboxFlags(const char* label, ImS64* flags, ImS64 flags_value);
    IMGUI_API bool          CheckboxFlags(const char* label, ImU64* flags, ImU64 flags_value);

    IMGUI_API bool          CloseButton(ImGuiID id, const ImVec2& pos);
    IMGUI_API bool          CollapseButton(ImGuiID id, const ImVec2& pos);
    IMGUI_API void          Scrollbar(ImGuiAxis axis);
    IMGUI_API bool          ScrollbarEx(const ImRect& bb, ImGuiID id, ImGuiAxis axis, ImS64* p_scroll_v, ImS64 avail_v, ImS64 contents_v, ImDrawFlags draw_rounding_flags = 0);
    IMGUI_API ImRect        GetWindowScrollbarRect(ImGuiWindow* window, ImGuiAxis axis);
    IMGUI_API ImGuiID       GetWindowScrollbarID(ImGuiWindow* window, ImGuiAxis axis);
    IMGUI_API ImGuiID       GetWindowResizeCornerID(ImGuiWindow* window, int n);
    IMGUI_API ImGuiID       GetWindowResizeBorderID(ImGuiWindow* window, ImGuiDir dir);
    IMGUI_API void          ExtendHitBoxWhenNearViewportEdge(ImGuiWindow* window, ImRect* bb, float threshold, ImGuiAxis axis);

    IMGUI_API bool          ButtonBehavior(const ImRect& bb, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags = 0);
    IMGUI_API bool          DragBehavior(ImGuiID id, ImGuiDataType data_type, void* p_v, float v_speed, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
    IMGUI_API bool          SliderBehavior(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, void* p_v, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags, ImRect* out_grab_bb);
    IMGUI_API bool          SplitterBehavior(const ImRect& bb, ImGuiID id, ImGuiAxis axis, float* size1, float* size2, float min_size1, float min_size2, float hover_extend = 0.0f, float hover_visibility_delay = 0.0f, ImU32 bg_col = 0);

    IMGUI_API bool          TreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end = NULL);
    IMGUI_API void          TreeNodeDrawLineToChildNode(const ImVec2& target_pos);
    IMGUI_API void          TreeNodeDrawLineToTreePop(const ImGuiTreeNodeStackData* data);
    IMGUI_API void          TreePushOverrideID(ImGuiID id);
    IMGUI_API void          TreeNodeSetOpen(ImGuiID storage_id, bool open);
    IMGUI_API bool          TreeNodeUpdateNextOpen(ImGuiID storage_id, ImGuiTreeNodeFlags flags);

    template<typename T, typename SIGNED_T, typename FLOAT_T>   IMGUI_API float ScaleRatioFromValueT(ImGuiDataType data_type, T v, T v_min, T v_max, float logarithmic_zero_epsilon, float zero_deadzone_size);
    template<typename T, typename SIGNED_T, typename FLOAT_T>   IMGUI_API T     ScaleValueFromRatioT(ImGuiDataType data_type, float t, T v_min, T v_max, float logarithmic_zero_epsilon, float zero_deadzone_size);
    template<typename T, typename SIGNED_T, typename FLOAT_T>   IMGUI_API bool  DragBehaviorT(ImGuiDataType data_type, T* v, float v_speed, T v_min, T v_max, const char* format, ImGuiSliderFlags flags);
    template<typename T, typename SIGNED_T, typename FLOAT_T>   IMGUI_API bool  SliderBehaviorT(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, T* v, T v_min, T v_max, const char* format, ImGuiSliderFlags flags, ImRect* out_grab_bb);
    template<typename T>                                        IMGUI_API T     RoundScalarWithFormatT(const char* format, ImGuiDataType data_type, T v);
    template<typename T>                                        IMGUI_API bool  CheckboxFlagsT(const char* label, T* flags, T flags_value);

    IMGUI_API const ImGuiDataTypeInfo*  DataTypeGetInfo(ImGuiDataType data_type);
    IMGUI_API int           DataTypeFormatString(char* buf, int buf_size, ImGuiDataType data_type, const void* p_data, const char* format);
    IMGUI_API void          DataTypeApplyOp(ImGuiDataType data_type, int op, void* output, const void* arg_1, const void* arg_2);
    IMGUI_API bool          DataTypeApplyFromText(const char* buf, ImGuiDataType data_type, void* p_data, const char* format, void* p_data_when_empty = NULL);
    IMGUI_API int           DataTypeCompare(ImGuiDataType data_type, const void* arg_1, const void* arg_2);
    IMGUI_API bool          DataTypeClamp(ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max);
    IMGUI_API bool          DataTypeIsZero(ImGuiDataType data_type, const void* p_data);

    IMGUI_API bool          InputTextEx(const char* label, const char* hint, char* buf, int buf_size, const ImVec2& size_arg, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    IMGUI_API void          InputTextDeactivateHook(ImGuiID id);
    IMGUI_API bool          TempInputText(const ImRect& bb, ImGuiID id, const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    IMGUI_API bool          TempInputScalar(const ImRect& bb, ImGuiID id, const char* label, ImGuiDataType data_type, void* p_data, const char* format, const void* p_clamp_min = NULL, const void* p_clamp_max = NULL);
    inline bool             TempInputIsActive(ImGuiID id)       { ImGuiContext& g = *GImGui; return g.ActiveId == id && g.TempInputId == id; }
    inline ImGuiInputTextState* GetInputTextState(ImGuiID id)   { ImGuiContext& g = *GImGui; return (id != 0 && g.InputTextState.ID == id) ? &g.InputTextState : NULL; }
    IMGUI_API void          SetNextItemRefVal(ImGuiDataType data_type, void* p_data);
    inline bool             IsItemActiveAsInputText() { ImGuiContext& g = *GImGui; return g.ActiveId != 0 && g.ActiveId == g.LastItemData.ID && g.InputTextState.ID == g.LastItemData.ID; }

    IMGUI_API void          ColorTooltip(const char* text, const float* col, ImGuiColorEditFlags flags);
    IMGUI_API void          ColorEditOptionsPopup(const float* col, ImGuiColorEditFlags flags);
    IMGUI_API void          ColorPickerOptionsPopup(const float* ref_col, ImGuiColorEditFlags flags);
    inline void             SetNextItemColorMarker(ImU32 col) { ImGuiContext& g = *GImGui; g.NextItemData.HasFlags |= ImGuiNextItemDataFlags_HasColorMarker; g.NextItemData.ColorMarker = col; }

    IMGUI_API int           PlotEx(ImGuiPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, const ImVec2& size_arg);

    IMGUI_API void          ShadeVertsLinearColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1);
    IMGUI_API void          ShadeVertsLinearUV(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp);
    IMGUI_API void          ShadeVertsTransformPos(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& pivot_in, float cos_a, float sin_a, const ImVec2& pivot_out);

    IMGUI_API void          GcCompactTransientMiscBuffers();
    IMGUI_API void          GcCompactTransientWindowBuffers(ImGuiWindow* window);
    IMGUI_API void          GcAwakeTransientWindowBuffers(ImGuiWindow* window);

    IMGUI_API bool          ErrorLog(const char* msg);
    IMGUI_API void          ErrorRecoveryStoreState(ImGuiErrorRecoveryState* state_out);
    IMGUI_API void          ErrorRecoveryTryToRecoverState(const ImGuiErrorRecoveryState* state_in);
    IMGUI_API void          ErrorRecoveryTryToRecoverWindowState(const ImGuiErrorRecoveryState* state_in);
    IMGUI_API void          ErrorCheckUsingSetCursorPosToExtendParentBoundaries();
    IMGUI_API void          ErrorCheckEndFrameFinalizeErrorTooltip();
    IMGUI_API bool          BeginErrorTooltip();
    IMGUI_API void          EndErrorTooltip();

    IMGUI_API void          DemoMarker(const char* file, int line, const char* section);

    IMGUI_API void          DebugAllocHook(ImGuiDebugAllocInfo* info, int frame_count, void* ptr, size_t size);
    IMGUI_API void          DebugDrawCursorPos(ImU32 col = IM_COL32(255, 0, 0, 255));
    IMGUI_API void          DebugDrawLineExtents(ImU32 col = IM_COL32(255, 0, 0, 255));
    IMGUI_API void          DebugDrawItemRect(ImU32 col = IM_COL32(255, 0, 0, 255));
    IMGUI_API void          DebugTextUnformattedWithLocateItem(const char* line_begin, const char* line_end);
    IMGUI_API void          DebugLocateItem(ImGuiID target_id);
    IMGUI_API void          DebugLocateItemOnHover(ImGuiID target_id);
    IMGUI_API void          DebugLocateItemResolveWithLastItem();
    IMGUI_API void          DebugBreakClearData();
    IMGUI_API bool          DebugBreakButton(const char* label, const char* description_of_location);
    IMGUI_API void          DebugBreakButtonTooltip(bool keyboard_only, const char* description_of_location);
    IMGUI_API void          ShowFontAtlas(ImFontAtlas* atlas);
    IMGUI_API ImU64         DebugTextureIDToU64(ImTextureID tex_id);
    IMGUI_API void          DebugHookIdInfo(ImGuiID id, ImGuiDataType data_type, const void* data_id, const void* data_id_end);
    IMGUI_API void          DebugNodeColumns(ImGuiOldColumns* columns);
    IMGUI_API void          DebugNodeDrawList(ImGuiWindow* window, ImGuiViewportP* viewport, const ImDrawList* draw_list, const char* label);
    IMGUI_API void          DebugNodeDrawCmdShowMeshAndBoundingBox(ImDrawList* out_draw_list, const ImDrawList* draw_list, const ImDrawCmd* draw_cmd, bool show_mesh, bool show_aabb);
    IMGUI_API void          DebugNodeFont(ImFont* font);
    IMGUI_API void          DebugNodeFontGlyphsForSrcMask(ImFont* font, ImFontBaked* baked, int src_mask);
    IMGUI_API void          DebugNodeFontGlyph(ImFont* font, const ImFontGlyph* glyph);
    IMGUI_API void          DebugNodeTexture(ImTextureData* tex, int int_id, const ImFontAtlasRect* highlight_rect = NULL);
    IMGUI_API void          DebugNodeStorage(ImGuiStorage* storage, const char* label);
    IMGUI_API void          DebugNodeTabBar(ImGuiTabBar* tab_bar, const char* label);
    IMGUI_API void          DebugNodeTable(ImGuiTable* table);
    IMGUI_API void          DebugNodeTableSettings(ImGuiTableSettings* settings);
    IMGUI_API void          DebugNodeInputTextState(ImGuiInputTextState* state);
    IMGUI_API void          DebugNodeTypingSelectState(ImGuiTypingSelectState* state);
    IMGUI_API void          DebugNodeMultiSelectState(ImGuiMultiSelectState* state);
    IMGUI_API void          DebugNodeWindow(ImGuiWindow* window, const char* label);
    IMGUI_API void          DebugNodeWindowSettings(ImGuiWindowSettings* settings);
    IMGUI_API void          DebugNodeWindowsList(ImVector<ImGuiWindow*>* windows, const char* label);
    IMGUI_API void          DebugNodeWindowsListByBeginStackParent(ImGuiWindow** windows, int windows_size, ImGuiWindow* parent_in_begin_stack);
    IMGUI_API void          DebugNodeViewport(ImGuiViewportP* viewport);
    IMGUI_API void          DebugRenderKeyboardPreview(ImDrawList* draw_list);
    IMGUI_API void          DebugRenderViewportThumbnail(ImDrawList* draw_list, ImGuiViewportP* viewport, const ImRect& bb);

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#endif

}

struct ImFontLoader
{
    const char*     Name;
    bool            (*LoaderInit)(ImFontAtlas* atlas);
    void            (*LoaderShutdown)(ImFontAtlas* atlas);
    bool            (*FontSrcInit)(ImFontAtlas* atlas, ImFontConfig* src);
    void            (*FontSrcDestroy)(ImFontAtlas* atlas, ImFontConfig* src);
    bool            (*FontSrcContainsGlyph)(ImFontAtlas* atlas, ImFontConfig* src, ImWchar codepoint);
    bool            (*FontBakedInit)(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* loader_data_for_baked_src);
    void            (*FontBakedDestroy)(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* loader_data_for_baked_src);
    bool            (*FontBakedLoadGlyph)(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* loader_data_for_baked_src, ImWchar codepoint, ImFontGlyph* out_glyph, float* out_advance_x);

    size_t          FontBakedSrcLoaderDataSize;

    ImFontLoader()  { memset((void*)this, 0, sizeof(*this)); }
};

#ifdef IMGUI_ENABLE_STB_TRUETYPE
IMGUI_API const ImFontLoader* ImFontAtlasGetFontLoaderForStbTruetype();
#endif
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
typedef ImFontLoader ImFontBuilderIO;
#endif

#define IMGUI_FONT_SIZE_MAX                                     (512.0f)
#define IMGUI_FONT_SIZE_THRESHOLD_FOR_LOADADVANCEXONLYMODE      (128.0f)

inline bool operator==(const ImTextureRef& lhs, const ImTextureRef& rhs)    { return lhs._TexID == rhs._TexID && lhs._TexData == rhs._TexData; }
inline bool operator!=(const ImTextureRef& lhs, const ImTextureRef& rhs)    { return lhs._TexID != rhs._TexID || lhs._TexData != rhs._TexData; }

#define ImFontAtlasRectId_IndexMask_        (0x0007FFFF)
#define ImFontAtlasRectId_GenerationMask_   (0x3FF00000)
#define ImFontAtlasRectId_GenerationShift_  (20)
inline int               ImFontAtlasRectId_GetIndex(ImFontAtlasRectId id)       { return (id & ImFontAtlasRectId_IndexMask_); }
inline unsigned int      ImFontAtlasRectId_GetGeneration(ImFontAtlasRectId id)  { return (unsigned int)(id & ImFontAtlasRectId_GenerationMask_) >> ImFontAtlasRectId_GenerationShift_; }
inline ImFontAtlasRectId ImFontAtlasRectId_Make(int index_idx, int gen_idx)     { IM_ASSERT(index_idx >= 0 && index_idx <= ImFontAtlasRectId_IndexMask_ && gen_idx <= (ImFontAtlasRectId_GenerationMask_ >> ImFontAtlasRectId_GenerationShift_)); return (ImFontAtlasRectId)(index_idx | (gen_idx << ImFontAtlasRectId_GenerationShift_)); }

struct ImFontAtlasRectEntry
{
    int                 TargetIndex : 20;
    unsigned int        Generation : 10;
    unsigned int        IsUsed : 1;
};

struct ImFontAtlasPostProcessData
{
    ImFontAtlas*        FontAtlas;
    ImFont*             Font;
    ImFontConfig*       FontSrc;
    ImFontBaked*        FontBaked;
    ImFontGlyph*        Glyph;

    void*               Pixels;
    ImTextureFormat     Format;
    int                 Pitch;
    int                 Width;
    int                 Height;
};

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE { struct stbrp_node; }
typedef IMGUI_STB_NAMESPACE::stbrp_node stbrp_node_im;
#else
struct stbrp_node;
typedef stbrp_node stbrp_node_im;
#endif
struct stbrp_context_opaque { char data[80]; };

struct ImFontAtlasBuilder
{
    stbrp_context_opaque        PackContext;
    ImVector<stbrp_node_im>     PackNodes;
    ImVector<ImTextureRect>     Rects;
    ImVector<ImFontAtlasRectEntry> RectsIndex;
    ImVector<unsigned char>     TempBuffer;
    int                         RectsIndexFreeListStart;
    int                         RectsPackedCount;
    int                         RectsPackedSurface;
    int                         RectsDiscardedCount;
    int                         RectsDiscardedSurface;
    int                         FrameCount;
    ImVec2i                     MaxRectSize;
    ImVec2i                     MaxRectBounds;
    bool                        LockDisableResize;
    bool                        PreloadedAllGlyphsRanges;

    ImStableVector<ImFontBaked,32> BakedPool;
    ImGuiStorage                BakedMap;
    int                         BakedDiscardedCount;

    ImFontAtlasRectId           PackIdMouseCursors;
    ImFontAtlasRectId           PackIdLinesTexData;

    ImFontAtlasBuilder()        { memset((void*)this, 0, sizeof(*this)); FrameCount = -1; RectsIndexFreeListStart = -1; PackIdMouseCursors = PackIdLinesTexData = -1; }
};

IMGUI_API void              ImFontAtlasBuildInit(ImFontAtlas* atlas);
IMGUI_API void              ImFontAtlasBuildDestroy(ImFontAtlas* atlas);
IMGUI_API void              ImFontAtlasBuildMain(ImFontAtlas* atlas);
IMGUI_API void              ImFontAtlasBuildSetupFontLoader(ImFontAtlas* atlas, const ImFontLoader* font_loader);
IMGUI_API void              ImFontAtlasBuildNotifySetFont(ImFontAtlas* atlas, ImFont* old_font, ImFont* new_font);
IMGUI_API void              ImFontAtlasBuildUpdatePointers(ImFontAtlas* atlas);
IMGUI_API void              ImFontAtlasBuildRenderBitmapFromString(ImFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char);
IMGUI_API void              ImFontAtlasBuildClear(ImFontAtlas* atlas);

IMGUI_API ImTextureData*    ImFontAtlasTextureAdd(ImFontAtlas* atlas, int w, int h);
IMGUI_API void              ImFontAtlasTextureMakeSpace(ImFontAtlas* atlas);
IMGUI_API void              ImFontAtlasTextureRepack(ImFontAtlas* atlas, int w, int h);
IMGUI_API void              ImFontAtlasTextureGrow(ImFontAtlas* atlas, int old_w = -1, int old_h = -1);
IMGUI_API void              ImFontAtlasTextureCompact(ImFontAtlas* atlas);
IMGUI_API ImVec2i           ImFontAtlasTextureGetSizeEstimate(ImFontAtlas* atlas);

IMGUI_API void              ImFontAtlasBuildSetupFontSpecialGlyphs(ImFontAtlas* atlas, ImFont* font, ImFontConfig* src);
IMGUI_API void              ImFontAtlasBuildLegacyPreloadAllGlyphRanges(ImFontAtlas* atlas);
IMGUI_API void              ImFontAtlasBuildGetOversampleFactors(ImFontConfig* src, ImFontBaked* baked, int* out_oversample_h, int* out_oversample_v);
IMGUI_API void              ImFontAtlasBuildDiscardBakes(ImFontAtlas* atlas, int unused_frames);

IMGUI_API bool              ImFontAtlasFontSourceInit(ImFontAtlas* atlas, ImFontConfig* src);
IMGUI_API void              ImFontAtlasFontSourceAddToFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* src);
IMGUI_API void              ImFontAtlasFontDestroySourceData(ImFontAtlas* atlas, ImFontConfig* src);
IMGUI_API bool              ImFontAtlasFontInitOutput(ImFontAtlas* atlas, ImFont* font);
IMGUI_API void              ImFontAtlasFontDestroyOutput(ImFontAtlas* atlas, ImFont* font);
IMGUI_API void              ImFontAtlasFontRebuildOutput(ImFontAtlas* atlas, ImFont* font);
IMGUI_API void              ImFontAtlasFontDiscardBakes(ImFontAtlas* atlas, ImFont* font, int unused_frames);

IMGUI_API ImGuiID           ImFontAtlasBakedGetId(ImGuiID font_id, float baked_size, float rasterizer_density);
IMGUI_API ImFontBaked*      ImFontAtlasBakedGetOrAdd(ImFontAtlas* atlas, ImFont* font, float font_size, float font_rasterizer_density);
IMGUI_API ImFontBaked*      ImFontAtlasBakedGetClosestMatch(ImFontAtlas* atlas, ImFont* font, float font_size, float font_rasterizer_density);
IMGUI_API ImFontBaked*      ImFontAtlasBakedAdd(ImFontAtlas* atlas, ImFont* font, float font_size, float font_rasterizer_density, ImGuiID baked_id);
IMGUI_API void              ImFontAtlasBakedDiscard(ImFontAtlas* atlas, ImFont* font, ImFontBaked* baked);
IMGUI_API ImFontGlyph*      ImFontAtlasBakedAddFontGlyph(ImFontAtlas* atlas, ImFontBaked* baked, ImFontConfig* src, const ImFontGlyph* in_glyph);
IMGUI_API void              ImFontAtlasBakedAddFontGlyphAdvancedX(ImFontAtlas* atlas, ImFontBaked* baked, ImFontConfig* src, ImWchar codepoint, float advance_x);
IMGUI_API void              ImFontAtlasBakedDiscardFontGlyph(ImFontAtlas* atlas, ImFont* font, ImFontBaked* baked, ImFontGlyph* glyph);
IMGUI_API void              ImFontAtlasBakedSetFontGlyphBitmap(ImFontAtlas* atlas, ImFontBaked* baked, ImFontConfig* src, ImFontGlyph* glyph, ImTextureRect* r, const unsigned char* src_pixels, ImTextureFormat src_fmt, int src_pitch);

IMGUI_API void              ImFontAtlasPackInit(ImFontAtlas* atlas);
IMGUI_API ImFontAtlasRectId ImFontAtlasPackAddRect(ImFontAtlas* atlas, int w, int h, ImFontAtlasRectEntry* overwrite_entry = NULL);
IMGUI_API ImTextureRect*    ImFontAtlasPackGetRect(ImFontAtlas* atlas, ImFontAtlasRectId id);
IMGUI_API ImTextureRect*    ImFontAtlasPackGetRectSafe(ImFontAtlas* atlas, ImFontAtlasRectId id);
IMGUI_API void              ImFontAtlasPackDiscardRect(ImFontAtlas* atlas, ImFontAtlasRectId id);

IMGUI_API void              ImFontAtlasUpdateNewFrame(ImFontAtlas* atlas, int frame_count, bool renderer_has_textures);
IMGUI_API void              ImFontAtlasAddDrawListSharedData(ImFontAtlas* atlas, ImDrawListSharedData* data);
IMGUI_API void              ImFontAtlasRemoveDrawListSharedData(ImFontAtlas* atlas, ImDrawListSharedData* data);
IMGUI_API void              ImFontAtlasUpdateDrawListsTextures(ImFontAtlas* atlas, ImTextureRef old_tex, ImTextureRef new_tex);
IMGUI_API void              ImFontAtlasUpdateDrawListsSharedData(ImFontAtlas* atlas);

IMGUI_API void              ImFontAtlasTextureBlockConvert(const unsigned char* src_pixels, ImTextureFormat src_fmt, int src_pitch, unsigned char* dst_pixels, ImTextureFormat dst_fmt, int dst_pitch, int w, int h);
IMGUI_API void              ImFontAtlasTextureBlockPostProcess(ImFontAtlasPostProcessData* data);
IMGUI_API void              ImFontAtlasTextureBlockPostProcessMultiply(ImFontAtlasPostProcessData* data, float multiply_factor);
IMGUI_API void              ImFontAtlasTextureBlockFill(ImTextureData* dst_tex, int dst_x, int dst_y, int w, int h, ImU32 col);
IMGUI_API void              ImFontAtlasTextureBlockCopy(ImTextureData* src_tex, int src_x, int src_y, ImTextureData* dst_tex, int dst_x, int dst_y, int w, int h);
IMGUI_API void              ImFontAtlasTextureBlockQueueUpload(ImFontAtlas* atlas, ImTextureData* tex, int x, int y, int w, int h);

IMGUI_API int               ImTextureDataGetFormatBytesPerPixel(ImTextureFormat format);
IMGUI_API const char*       ImTextureDataGetStatusName(ImTextureStatus status);
IMGUI_API const char*       ImTextureDataGetFormatName(ImTextureFormat format);

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
IMGUI_API void              ImFontAtlasDebugLogTextureRequests(ImFontAtlas* atlas);
#endif

IMGUI_API bool      ImFontAtlasGetMouseCursorTexData(ImFontAtlas* atlas, ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2]);

#ifdef IMGUI_ENABLE_TEST_ENGINE
extern void         ImGuiTestEngineHook_ItemAdd(ImGuiContext* ctx, ImGuiID id, const ImRect& bb, const ImGuiLastItemData* item_data);
extern void         ImGuiTestEngineHook_ItemInfo(ImGuiContext* ctx, ImGuiID id, const char* label, ImGuiItemStatusFlags flags);
extern void         ImGuiTestEngineHook_Log(ImGuiContext* ctx, const char* fmt, ...);
extern const char*  ImGuiTestEngine_FindItemDebugLabel(ImGuiContext* ctx, ImGuiID id);

#define IMGUI_TEST_ENGINE_ITEM_ADD(_ID,_BB,_ITEM_DATA)      if (g.TestEngineHookItems) ImGuiTestEngineHook_ItemAdd(&g, _ID, _BB, _ITEM_DATA)
#define IMGUI_TEST_ENGINE_ITEM_INFO(_ID,_LABEL,_FLAGS)      if (g.TestEngineHookItems) ImGuiTestEngineHook_ItemInfo(&g, _ID, _LABEL, _FLAGS)
#define IMGUI_TEST_ENGINE_LOG(_FMT,...)                     ImGuiTestEngineHook_Log(&g, _FMT, __VA_ARGS__)
#else
#define IMGUI_TEST_ENGINE_ITEM_ADD(_ID,_BB,_ITEM_DATA)      ((void)0)
#define IMGUI_TEST_ENGINE_ITEM_INFO(_ID,_LABEL,_FLAGS)      ((void)g)
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#endif
