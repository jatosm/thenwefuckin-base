// discord.gg/thenwefuckin
#include "worldgeo.h"

#include "core/globals/globals.h"
#include "core/variables/variables.h"
#include "sdk/offsets.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <thread>

namespace WorldGeo {
namespace {

constexpr float kCell = 16.0f;
constexpr int kMaxDrawBoxes = 2500;
constexpr float kMinPx = 4.0f;

std::mutex s_mtx;
std::shared_ptr<const Snapshot> s_snap;

std::shared_ptr<const Snapshot> GetSnap() {
    std::lock_guard<std::mutex> lk(s_mtx);
    return s_snap;
}

void SetSnap(std::shared_ptr<const Snapshot> s) {
    std::lock_guard<std::mutex> lk(s_mtx);
    s_snap = std::move(s);
}

inline std::int64_t CellKey(int cx, int cy, int cz) {
    return (static_cast<std::int64_t>(cx) * 73856093LL) ^
           (static_cast<std::int64_t>(cy) * 19349663LL) ^
           (static_cast<std::int64_t>(cz) * 83492791LL);
}

bool IsPartClass(const std::string& cls) {
    return cls == "Part" || cls == "MeshPart" || cls == "UnionOperation" ||
           cls == "WedgePart" || cls == "CornerWedgePart" || cls == "TrussPart";
}

void Walk(std::uintptr_t addr, int depth, std::vector<Box>& out, int& counter, bool ignoreCollide, int maxDepth, float minDim) {
    if (!addr || depth > maxDepth || !running.load(std::memory_order_relaxed))
        return;
    RBX::RbxInstance inst{addr};
    const std::string cls = inst.GetClass();
    if (cls.empty())
        return;
    if (cls == "Camera")
        return;
    if (cls == "Model") {
        if (inst.FindChildByClass("Humanoid").Addr)
            return;
    }
    if (depth <= 1 && cls == "Folder") {
        const std::string nm = inst.GetName();
        if (nm == "Players" || nm == "Ignore")
            return;
    }
    if (IsPartClass(cls)) {
        const std::uintptr_t prim = memory->read<std::uintptr_t>(addr + Offsets::BasePart::Primitive);
        if (prim) {
            bool solid = ignoreCollide;
            if (!solid) {
                const std::uint8_t flags = memory->read<std::uint8_t>(prim + Offsets::Primitive::Flags);
                solid = (flags & (std::uint8_t)Offsets::PrimitiveFlags::CanCollide) != 0;
            }
            if (solid) {
                RBX::CFrame cf = memory->read<RBX::CFrame>(prim + Offsets::Primitive::Rotation);
                RBX::Vec3 sz = memory->read<RBX::Vec3>(prim + Offsets::Primitive::Size);
                const float maxDim = (std::max)(sz.X, (std::max)(sz.Y, sz.Z));
                if (!(cf.data[9] == 0 && cf.data[10] == 0 && cf.data[11] == 0) &&
                    maxDim >= minDim &&
                    sz.X > 0.02f && sz.Y > 0.02f && sz.Z > 0.02f &&
                    sz.X < 2048.0f && sz.Y < 2048.0f && sz.Z < 2048.0f) {
                    Box b{};
                    b.px = cf.data[9];
                    b.py = cf.data[10];
                    b.pz = cf.data[11];
                    for (int i = 0; i < 9; ++i)
                        b.r[i] = cf.data[i];
                    b.sx = sz.X;
                    b.sy = sz.Y;
                    b.sz = sz.Z;
                    out.push_back(b);
                }
            }
        }
        if ((++counter & 511) == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        for (auto& c : inst.GetChildList())
            Walk(c.Addr, depth + 1, out, counter, ignoreCollide, maxDepth, minDim);
        return;
    }
    if (cls == "Workspace" || cls == "Folder" || cls == "Model") {
        for (auto& c : inst.GetChildList())
            Walk(c.Addr, depth + 1, out, counter, ignoreCollide, maxDepth, minDim);
    }
}

bool WantsData() {
    return variables::World::wireframe || variables::Aimbot::visibleCheck;
}

}

void PublishBoxes(std::vector<Box>&& boxes) {
    if (boxes.empty() || !running.load(std::memory_order_relaxed))
        return;
    auto snap = std::make_shared<Snapshot>();
    snap->boxes = std::move(boxes);
    snap->grid.reserve(snap->boxes.size() * 2);
    for (int i = 0; i < (int)snap->boxes.size(); ++i) {
        const Box& b = snap->boxes[i];
        const float hx = b.sx * 0.5f, hy = b.sy * 0.5f, hz = b.sz * 0.5f;
        const float ex = std::fabs(b.r[0]) * hx + std::fabs(b.r[1]) * hy + std::fabs(b.r[2]) * hz;
        const float ey = std::fabs(b.r[3]) * hx + std::fabs(b.r[4]) * hy + std::fabs(b.r[5]) * hz;
        const float ez = std::fabs(b.r[6]) * hx + std::fabs(b.r[7]) * hy + std::fabs(b.r[8]) * hz;
        const int x0 = (int)std::floor((b.px - ex) / kCell);
        const int x1 = (int)std::floor((b.px + ex) / kCell);
        const int y0 = (int)std::floor((b.py - ey) / kCell);
        const int y1 = (int)std::floor((b.py + ey) / kCell);
        const int z0 = (int)std::floor((b.pz - ez) / kCell);
        const int z1 = (int)std::floor((b.pz + ez) / kCell);
        for (int cx = x0; cx <= x1; ++cx)
            for (int cy = y0; cy <= y1; ++cy)
                for (int cz = z0; cz <= z1; ++cz)
                    snap->grid[CellKey(cx, cy, cz)].push_back(i);
    }
    SetSnap(std::move(snap));
}

void Loop() {
    auto NeedsParse = [] {
        if (!WantsData())
            return false;
        auto s = GetSnap();
        return !s || s->boxes.empty();
    };
    while (running.load(std::memory_order_relaxed)) {
        if (Globals::workspace.Addr && WantsData()) {

            {
                auto cur = GetSnap();
                if (!cur || cur->boxes.empty()) {
                    std::vector<Box> fast;
                    fast.reserve(1024);
                    int counter = 0;
                    Walk(Globals::workspace.Addr, 0, fast, counter, false, 4, 2.0f);
                    if (!fast.empty())
                        PublishBoxes(std::move(fast));
                }
            }
            if (!running.load(std::memory_order_relaxed))
                break;
            std::vector<Box> boxes;
            boxes.reserve(4096);
            int counter = 0;
            Walk(Globals::workspace.Addr, 0, boxes, counter, false, 9, 0.02f);
            if (boxes.size() < 20 && running.load(std::memory_order_relaxed)) {
                boxes.clear();
                counter = 0;
                Walk(Globals::workspace.Addr, 0, boxes, counter, true, 9, 0.02f);
            }
            PublishBoxes(std::move(boxes));
        }
        for (int i = 0; i < 200 && running.load(std::memory_order_relaxed); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (NeedsParse())
                break;
        }
    }
}

void Render(ImDrawList* dl, const RBX::Mat4& view) {
    static bool firstSet = false;
    static std::chrono::steady_clock::time_point firstOn{};
    static std::chrono::steady_clock::time_point lastNote{};
    if (!variables::World::wireframe || !dl) {
        firstSet = false;
        return;
    }
    auto snap = GetSnap();
    if (!snap || snap->boxes.empty()) {
        const auto now = std::chrono::steady_clock::now();
        if (!firstSet) {
            firstSet = true;
            firstOn = now;
            lastNote = now - std::chrono::seconds(60);
        }
        (void)firstOn; (void)lastNote;
        return;
    }
    const ImVec2 disp = ImGui::GetIO().DisplaySize;
    const ImU32 col = IM_COL32(160, 160, 160, 220);
    {
        char buf[48];
        std::snprintf(buf, sizeof(buf), "wire %d", (int)snap->boxes.size());
        dl->AddText(ImGui::GetFont(), 13.0f, ImVec2(12.0f, 40.0f), IM_COL32(140, 140, 140, 255), buf);
    }
    int drawn = 0;
    for (const Box& b : snap->boxes) {
        if (drawn >= kMaxDrawBoxes)
            break;

        const float cw = b.px * view.data[12] + b.py * view.data[13] + b.pz * view.data[14] + view.data[15];
        if (cw < 0.5f)
            continue;
        const float cx = b.px * view.data[0] + b.py * view.data[1] + b.pz * view.data[2] + view.data[3];
        const float cy = b.px * view.data[4] + b.py * view.data[5] + b.pz * view.data[6] + view.data[7];
        const float nx = cx / cw, ny = cy / cw;
        if (nx < -1.6f || nx > 1.6f || ny < -1.6f || ny > 1.6f)
            continue;

        const float maxDim = (std::max)(b.sx, (std::max)(b.sy, b.sz));
        const float estPx = maxDim * disp.y / ((std::max)(cw, 1.0f) * 1.4f);
        if (estPx < kMinPx)
            continue;
        const float hx = b.sx * 0.5f, hy = b.sy * 0.5f, hz = b.sz * 0.5f;
        ImVec2 sp[8];
        bool ok[8]{};
        bool any = false;
        for (int i = 0; i < 8; ++i) {
            const float lx = (i & 1) ? hx : -hx;
            const float ly = (i & 2) ? hy : -hy;
            const float lz = (i & 4) ? hz : -hz;
            const float wx = b.px + b.r[0] * lx + b.r[1] * ly + b.r[2] * lz;
            const float wy = b.py + b.r[3] * lx + b.r[4] * ly + b.r[5] * lz;
            const float wz = b.pz + b.r[6] * lx + b.r[7] * ly + b.r[8] * lz;
            const float w = wx * view.data[12] + wy * view.data[13] + wz * view.data[14] + view.data[15];
            if (w < 0.1f)
                continue;
            const float sx = wx * view.data[0] + wy * view.data[1] + wz * view.data[2] + view.data[3];
            const float sy = wx * view.data[4] + wy * view.data[5] + wz * view.data[6] + view.data[7];
            sp[i] = ImVec2((disp.x * 0.5f * (sx / w)) + (disp.x * 0.5f),
                           -(disp.y * 0.5f * (sy / w)) + (disp.y * 0.5f));
            ok[i] = true;
            any = true;
        }
        if (!any)
            continue;
        static const int kEdges[12][2] = {
            {0, 1}, {1, 3}, {3, 2}, {2, 0},
            {4, 5}, {5, 7}, {7, 6}, {6, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7},
        };
        for (auto& e : kEdges) {
            if (!ok[e[0]] || !ok[e[1]])
                continue;
            dl->AddLine(sp[e[0]], sp[e[1]], col, 1.0f);
        }
        ++drawn;
    }
}

float RayBoxT(const Box& b, float ox, float oy, float oz,
              float dx, float dy, float dz, float maxT) {
    float px = ox - b.px, py = oy - b.py, pz = oz - b.pz;

    const float lx = b.r[0] * px + b.r[3] * py + b.r[6] * pz;
    const float ly = b.r[1] * px + b.r[4] * py + b.r[7] * pz;
    const float lz = b.r[2] * px + b.r[5] * py + b.r[8] * pz;
    const float ldx = b.r[0] * dx + b.r[3] * dy + b.r[6] * dz;
    const float ldy = b.r[1] * dx + b.r[4] * dy + b.r[7] * dz;
    const float ldz = b.r[2] * dx + b.r[5] * dy + b.r[8] * dz;
    const float hx = b.sx * 0.5f, hy = b.sy * 0.5f, hz = b.sz * 0.5f;
    float tmin = 0.0f, tmax = maxT;
    const float o[3] = {lx, ly, lz};
    const float d[3] = {ldx, ldy, ldz};
    const float h[3] = {hx, hy, hz};
    for (int i = 0; i < 3; ++i) {
        if (std::fabs(d[i]) < 1e-9f) {
            if (o[i] < -h[i] || o[i] > h[i])
                return -1.0f;
        } else {
            float t1 = (-h[i] - o[i]) / d[i];
            float t2 = (h[i] - o[i]) / d[i];
            if (t1 > t2) {
                const float t = t1;
                t1 = t2;
                t2 = t;
            }
            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;
            if (tmin > tmax)
                return -1.0f;
        }
    }
    return tmin;
}

bool IsVisible(const RBX::Vec3& cam, const RBX::Vec3& target) {
    auto snap = GetSnap();
    if (!snap || snap->boxes.empty() || !snap->grid.size())
        return true;
    float dx = target.X - cam.X;
    float dy = target.Y - cam.Y;
    float dz = target.Z - cam.Z;
    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist < 1.0f)
        return true;
    dx /= dist;
    dy /= dist;
    dz /= dist;

    int cx = (int)std::floor(cam.X / kCell);
    int cy = (int)std::floor(cam.Y / kCell);
    int cz = (int)std::floor(cam.Z / kCell);
    const int stepX = (dx > 0) ? 1 : -1;
    const int stepY = (dy > 0) ? 1 : -1;
    const int stepZ = (dz > 0) ? 1 : -1;
    const float tDeltaX = (dx != 0) ? std::fabs(kCell / dx) : 1e9f;
    const float tDeltaY = (dy != 0) ? std::fabs(kCell / dy) : 1e9f;
    const float tDeltaZ = (dz != 0) ? std::fabs(kCell / dz) : 1e9f;
    auto axisT = [](float origin, float d, int c) -> float {
        if (d == 0)
            return 1e9f;
        const float edge = (d > 0) ? ((c + 1) * kCell) : (c * kCell);
        return (edge - origin) / d;
    };
    float tMaxX = axisT(cam.X, dx, cx);
    float tMaxY = axisT(cam.Y, dy, cy);
    float tMaxZ = axisT(cam.Z, dz, cz);
    float t = 0.0f;
    int cells = 0;
    int tests = 0;
    while (t <= dist && cells < 512 && tests < 2048) {
        auto it = snap->grid.find(CellKey(cx, cy, cz));
        if (it != snap->grid.end()) {
            for (int idx : it->second) {
                if (++tests > 2048)
                    break;
                const Box& b = snap->boxes[(std::size_t)idx];
                const float hit = RayBoxT(b, cam.X, cam.Y, cam.Z, dx, dy, dz, dist);

                if (hit > 1.0f && hit < dist - 1.0f)
                    return false;
            }
        }
        if (tMaxX < tMaxY && tMaxX < tMaxZ) {
            t = tMaxX;
            tMaxX += tDeltaX;
            cx += stepX;
        } else if (tMaxY < tMaxZ) {
            t = tMaxY;
            tMaxY += tDeltaY;
            cy += stepY;
        } else {
            t = tMaxZ;
            tMaxZ += tDeltaZ;
            cz += stepZ;
        }
        ++cells;
    }
    return true;
}

}
