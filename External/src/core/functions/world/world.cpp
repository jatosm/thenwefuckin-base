// discord.gg/thenwefuckin
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "world.h"
#include "core/cache/worldcache.h"
#include "core/cache/cache.h"
#include "core/variables/variables.h"
#include "sdk/w2s.h"
#include "render/menu/library.h"
#include <mutex>
#include <vector>
#include <cmath>
#include <cstring>
#include <string>
namespace {
ImFont* WF_EspFont(){ auto &f=imGuiCustom::GetFonts(); return f.CascadiaMonoBL?f.CascadiaMonoBL:ImGui::GetFont(); }
float WF_EspSize(){ return variables::Misc::espFontSize; }
ImU32 WF_ToU32(const ImVec4& c){ return IM_COL32(int(c.x*255),int(c.y*255),int(c.z*255),int(c.w*255)); }
void WF_DrawOutlinedText(ImDrawList* dl,ImVec2 p,std::string t,ImU32 col){ ImFont* f=WF_EspFont(); float s=WF_EspSize(); dl->AddText(f,s,ImVec2(p.x+1.0f,p.y+1.0f),IM_COL32(0,0,0,255),t.c_str()); dl->AddText(f,s,p,col,t.c_str()); }
bool WF_ToScreen(const RBX::Vec3& w,const RBX::Mat4& v,ImVec2& o){ auto s=W2S::WorldToScreen(w,v); if(s.X==0&&s.Y==0) return false; o=ImVec2(s.X,s.Y); return true; }
void WF_DrawWorldBox(ImDrawList* dl,float x0,float y0,float x1,float y1,ImU32 col){ dl->AddRect(ImVec2(x0-1,y0-1),ImVec2(x1+1,y1+1),IM_COL32(0,0,0,255),0,0,1); dl->AddRect(ImVec2(x0,y0),ImVec2(x1,y1),col,0,0,1); dl->AddRect(ImVec2(x0+1,y0+1),ImVec2(x1-1,y1-1),IM_COL32(0,0,0,255),0,0,1); }
void WF_DrawHealthBar(ImDrawList* dl,float bx0,float bx1,float by0,float by1,float frac,ImU32 col){ if(by1<by0) std::swap(by0,by1); bx0=std::floor(bx0+0.5f); bx1=bx0+2; by0=std::floor(by0+0.5f); by1=std::floor(by1+0.5f); float h=by1-by0; if(h<=1) return; frac=std::clamp(frac,0.f,1.f); dl->AddRectFilled(ImVec2(bx0,by0),ImVec2(bx1,by1),IM_COL32(0,0,0,200)); float ft=by1-h*frac; if(ft<by0) ft=by0; if(frac>0.001f) dl->AddRectFilled(ImVec2(bx0,ft),ImVec2(bx1,by1),col); dl->AddRectFilled(ImVec2(bx0-1,by0-1),ImVec2(bx1+1,by0),IM_COL32(0,0,0,255)); dl->AddRectFilled(ImVec2(bx0-1,by1),ImVec2(bx1+1,by1+1),IM_COL32(0,0,0,255)); dl->AddRectFilled(ImVec2(bx0-1,by0),ImVec2(bx0,by1),IM_COL32(0,0,0,255)); dl->AddRectFilled(ImVec2(bx1,by0),ImVec2(bx1+1,by1),IM_COL32(0,0,0,255)); }
RBX::Vec3 WF_PartPos(std::uintptr_t partAddr){ if(!partAddr) return {}; auto prim=memory->read<std::uintptr_t>(partAddr + Offsets::BasePart::Primitive); if(!prim) return {}; return memory->read<RBX::Vec3>(prim + Offsets::Primitive::Position); }
}

namespace WorldVisuals {
void Render(ImDrawList* dl, const RBX::Mat4& v) {
    if (!variables::World::enabled) return;

    if (!variables::World::ores && !variables::World::plants && !variables::World::animals &&
        !variables::World::soldiers && !variables::World::tools && !variables::World::crates &&
        !variables::World::drops && !variables::World::keycards)
        return;
    static std::vector<WorldCache::Entry> local;
    {
        std::lock_guard<std::mutex> lk(WorldCache::mtx);
        if (WorldCache::entries.empty()) return;
        local = WorldCache::entries;
    }
    ImFont* font = WF_EspFont();
    float sz = WF_EspSize();
    const ImU32 white = IM_COL32(255,255,255,255);
    for (auto &e : local) {
        if (e.humanoidAddr) {
            float liveHp = memory->read<float>(e.humanoidAddr + Offsets::Humanoid::Health);
            if (!std::isfinite(liveHp) || liveHp <= 0.0f) continue;
            e.health = liveHp;
        }
        ImVec2 scr;
        if (!WF_ToScreen(e.pos, v, scr)) continue;
        bool showName = variables::World::name;
        bool showDist = variables::World::distance;
        std::string distTxt;
        if (showDist)
            distTxt = std::to_string((int)e.dist) + "m";
        ImU32 distCol = IM_COL32(120,255,120,255);
        if (e.category=="plant" && e.plantIdx>=0 && e.plantIdx<7) distCol = WF_ToU32(variables::World::plantsColor[e.plantIdx]);
        else if (e.category=="ore" && e.oreIdx>=0 && e.oreIdx<3) distCol = WF_ToU32(variables::World::oresColor[e.oreIdx]);
        else if (e.category=="animal" && e.animalIdx>=0 && e.animalIdx<3) distCol = WF_ToU32(variables::World::animalsColor[e.animalIdx]);
        else if (e.category=="soldier" && e.soldierIdx>=0) distCol = WF_ToU32(variables::World::soldiersColor[e.soldierIdx % 4]);
        else if (e.category=="tool" && e.toolIdx>=0 && e.toolIdx<7) distCol = WF_ToU32(variables::World::toolsColor[e.toolIdx]);
        else if (e.category=="keycard" && e.keycardIdx>=0 && e.keycardIdx<5) distCol = WF_ToU32(variables::World::keycardsColor[e.keycardIdx]);
        else if (e.category=="crate" && e.crateIdx>=0 && e.crateIdx<34) distCol = WF_ToU32(variables::World::cratesColor[e.crateIdx]);
        else if (e.category=="drop") distCol = WF_ToU32(variables::World::dropsColor);
        else if (e.category=="npc") distCol = WF_ToU32(variables::World::soldiersColor[0]);
        else if (e.category=="plant") distCol = IM_COL32(80,255,80,255);
        else if (e.category=="ore") distCol = IM_COL32(200,200,60,255);
        else if (e.category=="tool") distCol = IM_COL32(120,180,255,255);
        else if (e.category=="soldier") distCol = IM_COL32(255,80,80,255);
        else if (e.category=="animal") distCol = IM_COL32(255,160,80,255);

        bool needBox=false, needHealth=false;
        if (e.category=="animal") { needBox=variables::World::animalsBox; needHealth=variables::World::animalsHealth; }
        if (e.category=="soldier") { needBox=variables::World::soldiersBox; needHealth=variables::World::soldiersHealth; }
        if (e.category=="npc") {
            needBox = variables::World::soldiersBox;
            needHealth = variables::World::soldiersHealth;
        }
        float bx0=0,bx1=0,by0=0,by1=0; bool hasBox=false;
        if (e.category=="tool" && needBox) {
            if (e.name=="Base Cabinet") {
                RBX::Vec3 hs{ e.size.X*0.5f, e.size.Y*0.5f, e.size.Z*0.5f };
                if (hs.X<0.5f) hs.X=0.7f; if (hs.Y<0.5f) hs.Y=0.7f; if (hs.Z<0.5f) hs.Z=0.7f;
                RBX::Vec3 c0{ e.pos.X - hs.X, e.pos.Y - hs.Y, e.pos.Z - hs.Z };
                RBX::Vec3 c1{ e.pos.X + hs.X, e.pos.Y - hs.Y, e.pos.Z - hs.Z };
                RBX::Vec3 c2{ e.pos.X - hs.X, e.pos.Y + hs.Y, e.pos.Z - hs.Z };
                RBX::Vec3 c3{ e.pos.X + hs.X, e.pos.Y + hs.Y, e.pos.Z - hs.Z };
                RBX::Vec3 c4{ e.pos.X - hs.X, e.pos.Y - hs.Y, e.pos.Z + hs.Z };
                RBX::Vec3 c5{ e.pos.X + hs.X, e.pos.Y - hs.Y, e.pos.Z + hs.Z };
                RBX::Vec3 c6{ e.pos.X - hs.X, e.pos.Y + hs.Y, e.pos.Z + hs.Z };
                RBX::Vec3 c7{ e.pos.X + hs.X, e.pos.Y + hs.Y, e.pos.Z + hs.Z };
                float mnX=1e9f,mxX=-1e9f,mnY=1e9f,mxY=-1e9f; bool any=false;
                RBX::Vec3 cor[8]={c0,c1,c2,c3,c4,c5,c6,c7};
                for(int i=0;i<8;++i){ ImVec2 s; if(!WF_ToScreen(cor[i],v,s)) continue; any=true; if(s.x<mnX) mnX=s.x; if(s.x>mxX) mxX=s.x; if(s.y<mnY) mnY=s.y; if(s.y>mxY) mxY=s.y; }
                if(any){ bx0=mnX; bx1=mxX; by0=mnY; by1=mxY; hasBox=true; WF_DrawWorldBox(dl,bx0,by0,bx1,by1,distCol); }
            }
        } else if ((needBox || needHealth) && e.headPos.X!=0 && e.rootPos.X!=0) {
            RBX::Vec3 top3{e.headPos.X, e.headPos.Y+0.5f, e.headPos.Z};
            RBX::Vec3 bot3{e.rootPos.X, e.rootPos.Y-2.5f, e.rootPos.Z};
            auto top = W2S::WorldToScreen(top3, v);
            auto bot = W2S::WorldToScreen(bot3, v);
            if (!(top.X==0&&top.Y==0) && !(bot.X==0&&bot.Y==0)) {
                float h = bot.Y - top.Y; float w = h*0.55f;
                bx0 = top.X - w*0.5f; bx1 = top.X + w*0.5f; by0 = top.Y; by1 = bot.Y;
                hasBox=true;
                if (needBox) WF_DrawWorldBox(dl, bx0, by0, bx1, by1, distCol);
                if (needHealth && e.maxHealth>0.01f) {
                    float f = e.health / e.maxHealth; if(!std::isfinite(f)) f=1.f; f=std::clamp(f,0.f,1.f);
                    float hbx0 = std::floor(bx0 - 4.0f), hbx1 = hbx0 + 2.0f;
                    ImU32 hcol = IM_COL32((int)(255*(1.f-f)), (int)(255*f), 0, 255);
                    WF_DrawHealthBar(dl, hbx0, hbx1, by0, by1, f, hcol);
                }
            }
        }
        bool isNPC = (e.category=="animal" || e.category=="soldier" || e.category=="npc");
        if (showName && showDist) {
            const ImVec2 tsN = font->CalcTextSizeA(sz, FLT_MAX, 0.0f, e.name.c_str());
            const ImVec2 tsD = font->CalcTextSizeA(sz, FLT_MAX, 0.0f, distTxt.c_str());
            float cx = hasBox ? (bx0+bx1)*0.5f : scr.x;
            if (isNPC) {

                float nameY = hasBox ? (by0 - tsN.y - 4.0f) : (scr.y - tsD.y - tsN.y - 2.0f);
                float distY = hasBox ? (by1 + 2.0f) : (scr.y + 1.0f);
                WF_DrawOutlinedText(dl, ImVec2(cx - tsN.x*0.5f, nameY), e.name, white);
                WF_DrawOutlinedText(dl, ImVec2(cx - tsD.x*0.5f, distY), distTxt, distCol);
            } else {
                float baseY = hasBox ? (by0 - tsD.y - 4.0f) : (scr.y - tsD.y - 1.0f);
                float nameY = hasBox ? (by1 + 2.0f) : (scr.y + 1.0f);
                WF_DrawOutlinedText(dl, ImVec2(cx - tsD.x*0.5f, baseY), distTxt, distCol);
                WF_DrawOutlinedText(dl, ImVec2(cx - tsN.x*0.5f, nameY), e.name, white);
            }
        } else if (showDist) {
            const ImVec2 tsD = font->CalcTextSizeA(sz, FLT_MAX, 0.0f, distTxt.c_str());
            float y = hasBox ? (by0 - tsD.y - 2.0f) : (scr.y - tsD.y*0.5f);
            float cx = hasBox ? (bx0+bx1)*0.5f : scr.x;
            WF_DrawOutlinedText(dl, ImVec2(cx - tsD.x*0.5f, y), distTxt, distCol);
        } else if (showName) {
            const ImVec2 tsN = font->CalcTextSizeA(sz, FLT_MAX, 0.0f, e.name.c_str());
            float y = hasBox ? (by1 + 2.0f) : (scr.y - tsN.y*0.5f);
            float cx = hasBox ? (bx0+bx1)*0.5f : scr.x;
            WF_DrawOutlinedText(dl, ImVec2(cx - tsN.x*0.5f, y), e.name, white);
        }
    }
}
}

namespace WorldVisuals {
namespace WorldFeature {
using RBX::Vec3;
struct color3 { float r = 0.f, g = 0.f, b = 0.f; };
struct lighting_backup {
    bool captured = false;
    uintptr_t address = 0;
    float clock_time = 0.f;
    uint32_t source = 0;
    Vec3 light_direction{}, light_color{}, sun_position{}, moon_position{}, gradient_top{}, gradient_bottom{};
    color3 ambient{}, outdoor_ambient{}, color_shift_top{}, color_shift_bottom{};
    float fog_start = 0.f, fog_end = 0.f;
    color3 fog_color{};
    float exposure = 0.f;
    float brightness = 2.f;
};
inline uintptr_t lighting_address = 0;
inline lighting_backup lighting{};
inline bool clock_time_active = false;
inline bool brightness_active = false;
inline bool ambient_active = false;
inline bool fog_active = false;
inline bool exposure_active = false;
inline bool is_valid_ptr(uintptr_t p) {
    return p >= 0x10000 && p <= 0x00007FFFFFFFFFFF;
}
inline uintptr_t cached_renderview = 0;
inline Vec3 vadd(const Vec3& a, const Vec3& b) { return { a.X + b.X, a.Y + b.Y, a.Z + b.Z }; }
inline Vec3 vsub(const Vec3& a, const Vec3& b) { return { a.X - b.X, a.Y - b.Y, a.Z - b.Z }; }
inline Vec3 vscale(const Vec3& a, float s) { return { a.X * s, a.Y * s, a.Z * s }; }
inline float vdot(const Vec3& a, const Vec3& b) { return a.X * b.X + a.Y * b.Y + a.Z * b.Z; }
inline Vec3 vcross(const Vec3& a, const Vec3& b) {
    return { a.Y * b.Z - a.Z * b.Y, a.Z * b.X - a.X * b.Z, a.X * b.Y - a.Y * b.X };
}
inline Vec3 vunit(const Vec3& a) {
    float len = std::sqrt(vdot(a, a));
    return len > 1e-6f ? vscale(a, 1.0f / len) : Vec3{ 0.f, 0.f, 0.f };
}
inline color3 to_color3(const ImVec4& c) {
    color3 o;
    o.r = std::clamp(c.x, 0.0f, 1.0f);
    o.g = std::clamp(c.y, 0.0f, 1.0f);
    o.b = std::clamp(c.z, 0.0f, 1.0f);
    return o;
}
inline void write_color3(uintptr_t addr, const color3& c) {
    memory->write<Vec3>(addr, Vec3{ c.r, c.g, c.b });
}
inline Vec3 read_color3(uintptr_t addr) {
    return memory->read<Vec3>(addr);
}
inline constexpr float k_pi = 3.1415927f;
inline constexpr float k_hour = 3600.0f;
inline constexpr float k_day = 86400.0f;
inline constexpr float k_sunrise = 6.0f * k_hour;
inline constexpr float k_sunset = 18.0f * k_hour;
inline constexpr float k_rise_set = k_hour;
inline constexpr float k_solar_year = 365.2564f * k_day;
inline constexpr float k_half_solar = 182.6282f;
inline float deg_to_rad(float degrees) {
    return degrees * (k_pi / 180.0f);
}
inline Vec3 spline(float t, const float* times, const Vec3* colors, int count) {
    if (count <= 0)
        return { 1.0f, 1.0f, 1.0f };
    if (t <= times[0])
        return colors[0];
    if (t >= times[count - 1])
        return colors[count - 1];
    for (int i = 0; i < count - 1; i++) {
        if (t < times[i] || t > times[i + 1])
            continue;
        float span = times[i + 1] - times[i];
        float a = span > 0.0f ? (t - times[i]) / span : 0.0f;
        return vadd(vscale(colors[i], 1.0f - a), vscale(colors[i + 1], a));
    }
    return colors[0];
}
inline Vec3 sky_ambient(float t) {
    static const float times[] = {
        0.0f, k_sunrise - 2.0f * k_hour, k_sunrise - k_hour, k_sunrise - k_hour * 0.5f,
        k_sunrise, k_sunrise + k_rise_set, k_sunset - k_rise_set, k_sunset,
        k_sunset + k_hour / 3.0f, k_day
    };
    static const Vec3 colors[] = {
        { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.07f, 0.07f, 0.1f }, { 0.2f, 0.15f, 0.01f },
        { 0.2f, 0.15f, 0.01f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 0.4f, 0.2f, 0.05f },
        { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }
    };
    return spline(t, times, colors, 10);
}
inline Vec3 sky_ambient2(float t) {
    static const float times[] = {
        0.0f, k_sunrise - 3.0f * k_hour, k_sunrise - 2.0f * k_hour, k_sunrise - k_hour * 0.5f,
        k_sunrise, k_sunrise + k_rise_set, k_sunset - k_rise_set, k_sunset,
        k_sunset + k_hour / 3.0f, k_sunset + 2.0f * k_hour, k_sunset + 3.0f * k_hour, k_day
    };
    static const Vec3 colors[] = {
        { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.21f, 0.21f, 0.28f }, { 0.4f, 0.3f, 0.3f },
        { 0.3f, 0.2f, 0.3f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 0.4f, 0.3f, 0.2f },
        { 0.3f, 0.2f, 0.3f }, { 0.3f, 0.2f, 0.3f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }
    };
    return spline(t, times, colors, 12);
}
inline Vec3 light_color_for_time(float t) {
    constexpr Vec3 day{ 0.75f, 0.75f, 0.75f };
    static const float times[] = {
        0.0f, k_sunrise - k_hour, k_sunrise, k_sunrise + k_rise_set * 0.25f,
        k_sunrise + k_rise_set, k_sunset - k_rise_set, k_sunset - k_rise_set * 0.5f,
        k_sunset, k_sunset + k_hour * 0.5f, k_day
    };
    static const Vec3 colors[] = {
        { 0.2f, 0.2f, 0.2f }, { 0.1f, 0.1f, 0.1f }, { 0.0f, 0.0f, 0.0f }, { 0.6f, 0.6f, 0.0f },
        day, day, { 0.1f, 0.1f, 0.075f }, { 0.1f, 0.05f, 0.05f }, { 0.1f, 0.1f, 0.1f }, { 0.2f, 0.2f, 0.2f }
    };
    return spline(t, times, colors, 10);
}
inline Vec3 rotate_axis_angle(const Vec3& v, const Vec3& axis, float angle) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    return vadd(vadd(vscale(v, cos_a), vscale(vcross(axis, v), sin_a)), vscale(axis, vdot(axis, v) * (1.0f - cos_a)));
}
inline Vec3 true_sun_position(float source_angle, float abs_seconds, float latitude_deg) {
    Vec3 sun{ std::sin(source_angle), -std::cos(source_angle), 0.0f };
    float day_of_year = (abs_seconds - std::floor(abs_seconds / k_solar_year) * k_solar_year) / k_day;
    float sun_offset = -deg_to_rad(23.5f) * std::cos(k_pi * (day_of_year - k_half_solar) / k_half_solar) - deg_to_rad(latitude_deg);
    Vec3 axis = vunit(vcross(Vec3{ 0.0f, 0.0f, 1.0f }, sun));
    return rotate_axis_angle(sun, axis, sun_offset);
}
inline void apply_clock_time(uintptr_t addr, float hours) {
    float seconds = hours * k_hour;
    float time_of_day = seconds - std::floor(seconds / k_day) * k_day;
    float source_angle = (time_of_day * 2.0f * k_pi) / k_day;
    float latitude = memory->read<float>(addr + Offsets::Lighting::GeographicLatitude);
    Vec3 sun = true_sun_position(source_angle, seconds, latitude);
    Vec3 moon = vscale(sun, -1.0f);
    bool use_sun = sun.Y > -0.3f;
    Vec3 light_direction = use_sun ? sun : moon;
    uint32_t source = use_sun ? 0u : 1u;
    memory->write<float>(addr + Offsets::Lighting::ClockTime, hours);
    memory->write<Vec3>(addr + Offsets::Lighting::GradientTop, sky_ambient(time_of_day));
    memory->write<Vec3>(addr + Offsets::Lighting::GradientBottom, sky_ambient2(time_of_day));
    memory->write<Vec3>(addr + Offsets::Lighting::LightColor, light_color_for_time(time_of_day));
    memory->write<Vec3>(addr + Offsets::Lighting::SunPosition, sun);
    memory->write<Vec3>(addr + Offsets::Lighting::MoonPosition, moon);
    memory->write<Vec3>(addr + Offsets::Lighting::LightDirection, light_direction);
    memory->write<uint32_t>(addr + Offsets::Lighting::Source, source);
}
inline lighting_backup capture_lighting(uintptr_t addr) {
    lighting_backup backup{};
    backup.address = addr;
    backup.clock_time = memory->read<float>(addr + Offsets::Lighting::ClockTime);
    backup.source = memory->read<uint32_t>(addr + Offsets::Lighting::Source);
    backup.light_direction = memory->read<Vec3>(addr + Offsets::Lighting::LightDirection);
    backup.light_color = memory->read<Vec3>(addr + Offsets::Lighting::LightColor);
    backup.sun_position = memory->read<Vec3>(addr + Offsets::Lighting::SunPosition);
    backup.moon_position = memory->read<Vec3>(addr + Offsets::Lighting::MoonPosition);
    backup.gradient_top = memory->read<Vec3>(addr + Offsets::Lighting::GradientTop);
    backup.gradient_bottom = memory->read<Vec3>(addr + Offsets::Lighting::GradientBottom);
    {
        Vec3 a = read_color3(addr + Offsets::Lighting::Ambient);
        backup.ambient = { a.X, a.Y, a.Z };
        Vec3 oa = read_color3(addr + Offsets::Lighting::OutdoorAmbient);
        backup.outdoor_ambient = { oa.X, oa.Y, oa.Z };
        Vec3 cst = read_color3(addr + Offsets::Lighting::ColorShift_Top);
        backup.color_shift_top = { cst.X, cst.Y, cst.Z };
        Vec3 csb = read_color3(addr + Offsets::Lighting::ColorShift_Bottom);
        backup.color_shift_bottom = { csb.X, csb.Y, csb.Z };
    }
    backup.fog_start = memory->read<float>(addr + Offsets::Lighting::FogStart);
    backup.fog_end = memory->read<float>(addr + Offsets::Lighting::FogEnd);
    {
        Vec3 fc = read_color3(addr + Offsets::Lighting::FogColor);
        backup.fog_color = { fc.X, fc.Y, fc.Z };
    }
    backup.exposure = memory->read<float>(addr + Offsets::Lighting::ExposureCompensation);
    backup.brightness = memory->read<float>(addr + Offsets::Lighting::Brightness);
    backup.captured = true;
    return backup;
}
inline void restore_clock_params(const lighting_backup& backup) {
    if (!backup.captured)
        return;
    memory->write<float>(backup.address + Offsets::Lighting::ClockTime, backup.clock_time);
    memory->write<uint32_t>(backup.address + Offsets::Lighting::Source, backup.source);
    memory->write<Vec3>(backup.address + Offsets::Lighting::LightDirection, backup.light_direction);
    memory->write<Vec3>(backup.address + Offsets::Lighting::LightColor, backup.light_color);
    memory->write<Vec3>(backup.address + Offsets::Lighting::SunPosition, backup.sun_position);
    memory->write<Vec3>(backup.address + Offsets::Lighting::MoonPosition, backup.moon_position);
    memory->write<Vec3>(backup.address + Offsets::Lighting::GradientTop, backup.gradient_top);
    memory->write<Vec3>(backup.address + Offsets::Lighting::GradientBottom, backup.gradient_bottom);
}
inline void force_lighting_dirty() {
    uintptr_t rv = cached_renderview;
    if (!is_valid_ptr(rv)) {
        uintptr_t ve = 0;
        if (Globals::renderEngine.Addr && is_valid_ptr(Globals::renderEngine.Addr))
            ve = Globals::renderEngine.Addr;
        if (!is_valid_ptr(ve)) {
            auto base = memory->get_module_address();
            if (base)
                ve = memory->read<uintptr_t>(base + Offsets::VisualEngine::Pointer);
        }
        if (is_valid_ptr(ve)) {
            rv = memory->read<uintptr_t>(ve + Offsets::VisualEngine::RenderView);
            if (is_valid_ptr(rv))
                cached_renderview = rv;
        }
    }
    if (!is_valid_ptr(rv))
        return;
    memory->write<bool>(rv + Offsets::RenderView::LightingValid, false);
    memory->write<bool>(rv + Offsets::RenderView::SkyValid, false);
}
inline void capture_ambient_if_needed(uintptr_t addr) {
    if (lighting.captured && lighting.address == addr)
        return;
    lighting = capture_lighting(addr);
}
inline void reset_state() {
    lighting_address = 0;
    lighting = {};
    clock_time_active = false;
    brightness_active = false;
    ambient_active = false;
    fog_active = false;
    exposure_active = false;
    cached_renderview = 0;
}
inline void tick(uintptr_t addr) {
    if (lighting_address != addr) {
        reset_state();
        lighting_address = addr;
    }
    const bool any_lighting = variables::World::clockTimeEnabled || variables::World::brightnessEnabled
        || variables::World::ambientEnabled
        || variables::World::fogEnabled || variables::World::exposureEnabled;
    if (any_lighting)
        capture_ambient_if_needed(addr);
    if (variables::World::clockTimeEnabled) {
        apply_clock_time(addr, variables::World::clockTimeValue);
        clock_time_active = true;
    } else if (clock_time_active) {
        restore_clock_params(lighting);
        clock_time_active = false;
    }
    if (variables::World::brightnessEnabled) {
        memory->write<float>(addr + Offsets::Lighting::Brightness, variables::World::brightnessValue);
        brightness_active = true;
    } else if (brightness_active) {
        memory->write<float>(addr + Offsets::Lighting::Brightness, lighting.brightness);
        brightness_active = false;
    }
    if (variables::World::ambientEnabled) {
        auto ambient = to_color3(variables::World::ambientColor);
        auto outdoor = to_color3(variables::World::outdoorAmbientColor);
        write_color3(addr + Offsets::Lighting::Ambient, ambient);
        write_color3(addr + Offsets::Lighting::OutdoorAmbient, outdoor);
        write_color3(addr + Offsets::Lighting::ColorShift_Top, outdoor);
        write_color3(addr + Offsets::Lighting::ColorShift_Bottom, ambient);
        if (!variables::World::clockTimeEnabled) {
            memory->write<Vec3>(addr + Offsets::Lighting::GradientTop, Vec3{ outdoor.r, outdoor.g, outdoor.b });
            memory->write<Vec3>(addr + Offsets::Lighting::GradientBottom, Vec3{ ambient.r, ambient.g, ambient.b });
        }
        ambient_active = true;
    } else if (ambient_active) {
        write_color3(addr + Offsets::Lighting::Ambient, lighting.ambient);
        write_color3(addr + Offsets::Lighting::OutdoorAmbient, lighting.outdoor_ambient);
        write_color3(addr + Offsets::Lighting::ColorShift_Top, lighting.color_shift_top);
        write_color3(addr + Offsets::Lighting::ColorShift_Bottom, lighting.color_shift_bottom);
        if (!clock_time_active) {
            memory->write<Vec3>(addr + Offsets::Lighting::GradientTop, lighting.gradient_top);
            memory->write<Vec3>(addr + Offsets::Lighting::GradientBottom, lighting.gradient_bottom);
        }
        ambient_active = false;
    }
    if (variables::World::fogEnabled) {
        memory->write<float>(addr + Offsets::Lighting::FogStart, variables::World::fogStart);
        memory->write<float>(addr + Offsets::Lighting::FogEnd, variables::World::fogEnd);
        write_color3(addr + Offsets::Lighting::FogColor, to_color3(variables::World::fogColor));
        fog_active = true;
    } else if (fog_active) {
        memory->write<float>(addr + Offsets::Lighting::FogStart, lighting.fog_start);
        memory->write<float>(addr + Offsets::Lighting::FogEnd, lighting.fog_end);
        write_color3(addr + Offsets::Lighting::FogColor, lighting.fog_color);
        fog_active = false;
    }
    if (variables::World::exposureEnabled) {
        memory->write<float>(addr + Offsets::Lighting::ExposureCompensation, variables::World::exposureValue);
        exposure_active = true;
    } else if (exposure_active) {
        memory->write<float>(addr + Offsets::Lighting::ExposureCompensation, lighting.exposure);
        exposure_active = false;
    }
    if (!any_lighting && lighting.captured && lighting.address == addr)
        lighting = {};
    if (any_lighting || clock_time_active || brightness_active || ambient_active || fog_active || exposure_active)
        force_lighting_dirty();
    (void)addr; (void)any_lighting;
}

inline std::uintptr_t g_skyBuf = 0;
inline std::uintptr_t g_skyAddr = 0;
inline bool g_skyHaveBackup = false;
inline std::string g_skyBackup[6];
inline std::string g_skyLast[6];
inline uintptr_t kSkyOffs[6] = {
    Offsets::Sky::SkyboxBk, Offsets::Sky::SkyboxDn, Offsets::Sky::SkyboxFt,
    Offsets::Sky::SkyboxLf, Offsets::Sky::SkyboxRt, Offsets::Sky::SkyboxUp
};

inline std::uintptr_t sky_buf()
{
    if (g_skyBuf)
        return g_skyBuf;
    HANDLE h = memory->get_process_handle();
    if (!h || h == INVALID_HANDLE_VALUE)
        return 0;
    auto p = (std::uintptr_t)VirtualAllocEx(h, nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (p)
        g_skyBuf = p;
    return g_skyBuf;
}

inline void write_rbx_string(std::uint64_t field, int slot, const std::string& s)
{
    std::string v = s;
    if (v.size() > 511)
        v.resize(511);
    std::int32_t len = (std::int32_t)v.size();
    memory->write<std::int32_t>(field + 0x10, len);
    if (len < 16) {
        char tmp[16] = {};
        if (len > 0)
            std::memcpy(tmp, v.data(), (std::size_t)len);
        memory->write_raw(field, tmp, 16);
    } else {
        std::uintptr_t buf = sky_buf();
        if (!buf)
            return;
        std::uintptr_t dst = buf + (std::uintptr_t)slot * 512ull;
        memory->write_raw(dst, v.data(), (std::size_t)len);
        memory->write<std::uint64_t>(field, dst);
    }
}

inline void skybox_tick(std::uintptr_t lightAddr)
{
    std::uintptr_t sky = 0;
    if (lightAddr)
        sky = RBX::RbxInstance(lightAddr).FindChildByClass("Sky").Addr;
    if (sky && (!g_skyAddr || memory->read<std::uintptr_t>(sky + Offsets::Instance::Parent) != lightAddr)) {
        g_skyAddr = sky;
        g_skyHaveBackup = false;
    }
    if (!variables::World::skyboxEnabled || !sky) {
        if (g_skyHaveBackup && sky) {
            for (int i = 0; i < 6; ++i) {
                write_rbx_string(sky + kSkyOffs[i], i, g_skyBackup[i]);
                g_skyLast[i] = g_skyBackup[i];
            }
        }
        if (!sky)
            g_skyAddr = 0;
        g_skyHaveBackup = false;
        return;
    }
    if (!g_skyHaveBackup) {
        for (int i = 0; i < 6; ++i) {
            g_skyBackup[i] = memory->read_string(sky + kSkyOffs[i]);
            g_skyLast[i] = g_skyBackup[i];
        }
        g_skyHaveBackup = true;
    }
    std::string want[6];
    if (variables::World::skyboxPreset == 0) {
        for (int i = 0; i < 6; ++i)
            want[i] = "";
    } else {
        want[0] = variables::World::skyIdBk;
        want[1] = variables::World::skyIdDn;
        want[2] = variables::World::skyIdFt;
        want[3] = variables::World::skyIdLf;
        want[4] = variables::World::skyIdRt;
        want[5] = variables::World::skyIdUp;
    }
    for (int i = 0; i < 6; ++i) {
        if (want[i] != g_skyLast[i]) {
            write_rbx_string(sky + kSkyOffs[i], i, want[i]);
            g_skyLast[i] = want[i];
        }
    }
}
}

void TickLighting()
{
    static std::uintptr_t lightAddr = 0;
    const std::uintptr_t dm = Globals::dataModel.Addr;
    if (!dm) {
        lightAddr = 0;
        return;
    }
    if (!lightAddr || memory->read<std::uintptr_t>(lightAddr + Offsets::Instance::Parent) != dm)
        lightAddr = Globals::dataModel.FindChildByClass("Lighting").Addr;
    if (!lightAddr)
        return;
    WorldFeature::tick(lightAddr);
    WorldFeature::skybox_tick(lightAddr);
}
}
