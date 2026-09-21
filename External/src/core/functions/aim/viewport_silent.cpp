#include "viewport_silent.h"
#include "../../globals/globals.h"
#include "../../../memory/memory.h"
#include "../../../sdk/offsets.h"
#include <Windows.h>
#include <atomic>
#include <cstdint>
#include <cmath>

namespace ViewportSilent {
    struct Vec2i16 {
        std::int16_t x = 0;
        std::int16_t y = 0;
    };
    static std::atomic<bool> g_active{ false };
    static std::atomic<bool> g_stop{ false };
    static std::atomic<bool> g_spoofed{ false };
    static std::atomic<float> g_tx{ 0.f }, g_ty{ 0.f }, g_tz{ 0.f };
    static HANDLE g_thread = nullptr;
    static uintptr_t g_cam = 0;
    static float g_dims_x = 0.f, g_dims_y = 0.f;
    static Vec2i16 g_last{};
    static int g_fails = 0;

    static bool get_scene(float view[16], float& dw, float& dh)
    {
        uintptr_t base = memory->get_module_address();
        if (!base) return false;
        uintptr_t ve = memory->read<uintptr_t>(base + Offsets::VisualEngine::Pointer);
        if (!ve) return false;
        for (int i = 0; i < 16; i++)
            view[i] = memory->read<float>(ve + Offsets::VisualEngine::ViewMatrix + i * sizeof(float));
        dw = memory->read<float>(ve + Offsets::VisualEngine::Dimensions);
        dh = memory->read<float>(ve + Offsets::VisualEngine::Dimensions + sizeof(float));
        return (dw >= 1.f && dh >= 1.f);
    }
    static bool world_to_screen(const RBX::Vec3& pos, float view[16], float dw, float dh, float& sx, float& sy)
    {
        float x = pos.X * view[0] + pos.Y * view[1] + pos.Z * view[2] + view[3];
        float y = pos.X * view[4] + pos.Y * view[5] + pos.Z * view[6] + view[7];
        float w = pos.X * view[12] + pos.Y * view[13] + pos.Z * view[14] + view[15];
        if (w < 0.001f) return false;
        float inv = 1.0f / w;
        sx = (dw * 0.5f) * (1.0f + x * inv);
        sy = (dh * 0.5f) * (1.0f - y * inv);
        return (sx > 0.f && sy > 0.f && sx < dw && sy < dh);
    }
    static Vec2i16 calc_viewport(float tx, float ty, float dw, float dh, float mx, float my)
    {
        constexpr double kMaxShift = 0.65;
        double tyd = (double)ty;
        if (tyd > (double)dh - 1.0) tyd = (double)dh - 1.0;
        if (tyd < 1.0) tyd = 1.0;
        double ratio = (double)my / tyd;
        double vy = (double)dh * ratio;
        double maxDy = (double)dh * kMaxShift;
        double rawDy = vy - (double)dh;
        if (rawDy > maxDy) vy = (double)dh + maxDy;
        else if (rawDy < -maxDy) vy = (double)dh - maxDy;
        if (vy > 32767.0) vy = 32767.0;
        if (vy < 1.0) vy = 1.0;
        ratio = vy / (double)dh;
        double vx = 2.0 * (double)mx - ratio * (2.0 * (double)tx - (double)dw);
        double maxDx = (double)dw * kMaxShift;
        double rawDx = vx - (double)dw;
        if (rawDx > maxDx) vx = (double)dw + maxDx;
        else if (rawDx < -maxDx) vx = (double)dw - maxDx;
        if (vx > 32767.0) vx = 32767.0;
        if (vx < 1.0) vx = 1.0;
        return { (std::int16_t)std::lround(vx), (std::int16_t)std::lround(vy) };
    }
    static bool get_mouse_in_viewport(float dw, float dh, float& mx, float& my)
    {
        HWND hwnd = FindWindowW(nullptr, L"Roblox");
        if (!hwnd || !IsWindow(hwnd)) return false;
        POINT pt{};
        if (!GetCursorPos(&pt) || !ScreenToClient(hwnd, &pt)) return false;
        RECT cr{};
        if (!GetClientRect(hwnd, &cr)) return false;
        float cw = (float)(cr.right - cr.left);
        float ch = (float)(cr.bottom - cr.top);
        if (cw < 1.f || ch < 1.f) return false;
        if (pt.x < 0 || pt.y < 0 || pt.x >= cr.right || pt.y >= cr.bottom) return false;
        mx = (float)pt.x * (dw / cw);
        my = (float)pt.y * (dh / ch);
        if (mx > dw - 1.f) mx = dw - 1.f;
        if (mx < 1.f) mx = 1.f;
        if (my > dh - 1.f) my = dh - 1.f;
        if (my < 1.f) my = 1.f;
        return true;
    }
    static void write_viewport(uintptr_t cam, const Vec2i16& v)
    {
        if (!cam) return;
        memory->write<Vec2i16>(cam + Offsets::Camera::Viewport, v);
        g_spoofed.store(true, std::memory_order_release);
    }
    static void restore_viewport()
    {
        if (!g_cam || g_dims_x < 1.f || g_dims_y < 1.f) return;
        Vec2i16 v{
            (std::int16_t)std::lround(g_dims_x),
            (std::int16_t)std::lround(g_dims_y)
        };
        memory->write<Vec2i16>(g_cam + Offsets::Camera::Viewport, v);
    }
    static bool compute(Vec2i16& out)
    {
        if (Globals::camera.Addr == 0) return false;
        RBX::Vec3 world{
            g_tx.load(std::memory_order_relaxed),
            g_ty.load(std::memory_order_relaxed),
            g_tz.load(std::memory_order_relaxed)
        };
        float view[16]{};
        float dw = 0.f, dh = 0.f;
        if (!get_scene(view, dw, dh)) return false;
        float sx = 0.f, sy = 0.f;
        if (!world_to_screen(world, view, dw, dh, sx, sy)) return false;
        float mx = 0.f, my = 0.f;
        if (!get_mouse_in_viewport(dw, dh, mx, my)) return false;
        out = calc_viewport(sx, sy, dw, dh, mx, my);
        g_cam = Globals::camera.Addr;
        g_dims_x = dw;
        g_dims_y = dh;
        return true;
    }
    static DWORD WINAPI writer_thread(LPVOID)
    {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        bool on = false;
        while (!g_stop.load(std::memory_order_acquire))
        {
            if (!g_active.load(std::memory_order_acquire))
            {
                if (on)
                {
                    restore_viewport();
                    g_spoofed.store(false, std::memory_order_release);
                    g_fails = 0;
                    on = false;
                }
                Sleep(16);
                continue;
            }
            Vec2i16 v{};
            if (compute(v))
            {
                g_last = v;
                g_fails = 0;
                on = true;
                write_viewport(g_cam, g_last);
            }
            else if (on && g_fails < 40)
            {
                ++g_fails;
                write_viewport(g_cam, g_last);
            }
            else if (on)
            {
                restore_viewport();
                g_spoofed.store(false, std::memory_order_release);
                on = false;
                g_fails = 0;
            }
            Sleep(1);
        }
        restore_viewport();
        g_spoofed.store(false, std::memory_order_release);
        return 0;
    }
    static void ensure_thread()
    {
        if (g_thread) return;
        g_stop.store(false, std::memory_order_release);
        g_thread = CreateThread(nullptr, 0, &writer_thread, nullptr, 0, nullptr);
        if (g_thread)
            SetThreadPriority(g_thread, THREAD_PRIORITY_HIGHEST);
    }
    void SetTarget(const RBX::Vec3& world)
    {
        ensure_thread();
        g_tx.store(world.X, std::memory_order_relaxed);
        g_ty.store(world.Y, std::memory_order_relaxed);
        g_tz.store(world.Z, std::memory_order_relaxed);
        g_active.store(true, std::memory_order_release);
    }
    void Clear()
    {
        g_active.store(false, std::memory_order_release);

        if (g_spoofed.load(std::memory_order_acquire)) {
            restore_viewport();
            g_spoofed.store(false, std::memory_order_release);
        }
    }
    void Shutdown()
    {
        g_active.store(false, std::memory_order_release);
        if (!g_thread) return;
        g_stop.store(true, std::memory_order_release);
        WaitForSingleObject(g_thread, 1000);
        CloseHandle(g_thread);
        g_thread = nullptr;
        g_cam = 0;
        g_dims_x = g_dims_y = 0.f;
    }
    bool IsAiming()
    {
        return g_active.load(std::memory_order_acquire) &&
               g_spoofed.load(std::memory_order_acquire);
    }
}
