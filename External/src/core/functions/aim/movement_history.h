#pragma once
#include <unordered_map>
#include <deque>
#include <chrono>
#include <cmath>

namespace MovementHistory {
struct Sample {
    float x, y, z;
    std::chrono::steady_clock::time_point t;
};
struct Result {
    float vx = 0.f, vy = 0.f, vz = 0.f;
    bool airborne = false;
    float damp = 1.f;
};
inline std::unordered_map<std::uintptr_t, std::deque<Sample>>& Store() {
    static std::unordered_map<std::uintptr_t, std::deque<Sample>> s;
    return s;
}
inline void Push(std::uintptr_t key, float x, float y, float z) {
    if (!key) return;
    auto now = std::chrono::steady_clock::now();
    auto& store = Store();
    auto& dq = store[key];
    dq.push_back(Sample{x, y, z, now});
    while (dq.size() > 14) dq.pop_front();
    if (store.size() > 48) {
        for (auto it = store.begin(); it != store.end();) {
            if (it->first != key && !it->second.empty() &&
                std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.back().t).count() > 2000)
                it = store.erase(it);
            else
                ++it;
        }
    }
}
inline void Clear(std::uintptr_t key) {
    Store().erase(key);
}
inline bool Sample(std::uintptr_t key, Result& out) {
    auto it = Store().find(key);
    if (it == Store().end()) return false;
    auto& dq = it->second;
    auto now = std::chrono::steady_clock::now();
    while (!dq.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(now - dq.front().t).count() > 450)
        dq.pop_front();
    size_t n = dq.size();
    if (n < 3) return false;
    double t0 = 0.0;
    double st = 0, stt = 0, sx = 0, sy = 0, sz = 0, stx = 0, sty = 0, stz = 0;
    bool first = true;
    for (auto& s : dq) {
        double t = std::chrono::duration<double>(s.t - dq.front().t).count();
        if (first) { t0 = t; first = false; }
        double rt = t - t0;
        st += rt; stt += rt * rt;
        sx += s.x; sy += s.y; sz += s.z;
        stx += rt * s.x; sty += rt * s.y; stz += rt * s.z;
    }
    double denom = (double)n * stt - st * st;
    if (fabs(denom) < 1e-6) return false;
    out.vx = (float)(((double)n * stx - st * sx) / denom);
    out.vy = (float)(((double)n * sty - st * sy) / denom);
    out.vz = (float)(((double)n * stz - st * sz) / denom);
    float yMin = dq.front().y, yMax = dq.front().y;
    for (auto& s : dq) {
        if (s.y < yMin) yMin = s.y;
        if (s.y > yMax) yMax = s.y;
    }
    out.airborne = (fabsf(out.vy) > 6.f) || ((yMax - yMin) > 3.5f);
    out.damp = 1.f;
    if (n >= 5) {
        size_t half = n / 2;
        double ax = 0, az = 0, bx = 0, bz = 0;
        size_t ca = 0, cb = 0;
        for (size_t i = 1; i < n; ++i) {
            double dt = std::chrono::duration<double>(dq[i].t - dq[i - 1].t).count();
            if (dt < 1e-4) continue;
            float vx = (dq[i].x - dq[i - 1].x) / (float)dt;
            float vz = (dq[i].z - dq[i - 1].z) / (float)dt;
            if (i <= half) { ax += vx; az += vz; ++ca; }
            else { bx += vx; bz += vz; ++cb; }
        }
        if (ca && cb) {
            double al = sqrt(ax * ax + az * az), bl = sqrt(bx * bx + bz * bz);
            if (al > 1.0 && bl > 1.0) {
                double cosA = (ax * bx + az * bz) / (al * bl);
                if (cosA < 1.0) {
                    double d = 0.45 + 0.55 * (cosA * 0.5 + 0.5);
                    if (d < 0.45) d = 0.45;
                    if (d > 1.0) d = 1.0;
                    out.damp = (float)d;
                }
            }
        }
    }
    return true;
}
}
