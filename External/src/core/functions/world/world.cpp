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
namespace {
ImFont* WF_EspFont(){ auto &f=imGuiCustom::GetFonts(); return f.CascadiaMonoBL?f.CascadiaMonoBL:ImGui::GetFont(); }
float WF_EspSize(){ return variables::Misc::espFontSize; }
ImU32 WF_ToU32(const ImVec4& c){ return IM_COL32(int(c.x*255),int(c.y*255),int(c.z*255),int(c.w*255)); }
void WF_DrawOutlinedText(ImDrawList* dl,ImVec2 p,std::string t,ImU32 col){ ImFont* f=WF_EspFont(); float s=WF_EspSize(); dl->AddText(f,s,ImVec2(p.x-1,p.y),IM_COL32(0,0,0,255),t.c_str()); dl->AddText(f,s,ImVec2(p.x+1,p.y),IM_COL32(0,0,0,255),t.c_str()); dl->AddText(f,s,ImVec2(p.x,p.y-1),IM_COL32(0,0,0,255),t.c_str()); dl->AddText(f,s,ImVec2(p.x,p.y+1),IM_COL32(0,0,0,255),t.c_str()); dl->AddText(f,s,p,col,t.c_str()); }
bool WF_ToScreen(const RBX::Vec3& w,const RBX::Mat4& v,ImVec2& o){ auto s=W2S::WorldToScreen(w,v); if(s.X==0&&s.Y==0) return false; o=ImVec2(s.X,s.Y); return true; }
void WF_DrawWorldBox(ImDrawList* dl,float x0,float y0,float x1,float y1,ImU32 col){ dl->AddRect(ImVec2(x0-1,y0-1),ImVec2(x1+1,y1+1),IM_COL32(0,0,0,255),0,0,1); dl->AddRect(ImVec2(x0,y0),ImVec2(x1,y1),col,0,0,1); dl->AddRect(ImVec2(x0+1,y0+1),ImVec2(x1-1,y1-1),IM_COL32(0,0,0,255),0,0,1); }
void WF_DrawHealthBar(ImDrawList* dl,float bx0,float bx1,float by0,float by1,float frac,ImU32 col){ if(by1<by0) std::swap(by0,by1); bx0=std::floor(bx0+0.5f); bx1=bx0+2; by0=std::floor(by0+0.5f); by1=std::floor(by1+0.5f); float h=by1-by0; if(h<=1) return; frac=std::clamp(frac,0.f,1.f); dl->AddRectFilled(ImVec2(bx0,by0),ImVec2(bx1,by1),IM_COL32(0,0,0,200)); float ft=by1-h*frac; if(ft<by0) ft=by0; if(frac>0.001f) dl->AddRectFilled(ImVec2(bx0,ft),ImVec2(bx1,by1),col); dl->AddRectFilled(ImVec2(bx0-1,by0-1),ImVec2(bx1+1,by0),IM_COL32(0,0,0,255)); dl->AddRectFilled(ImVec2(bx0-1,by1),ImVec2(bx1+1,by1+1),IM_COL32(0,0,0,255)); dl->AddRectFilled(ImVec2(bx0-1,by0),ImVec2(bx0,by1),IM_COL32(0,0,0,255)); dl->AddRectFilled(ImVec2(bx1,by0),ImVec2(bx1+1,by1),IM_COL32(0,0,0,255)); }
RBX::Vec3 WF_PartPos(std::uintptr_t partAddr){ if(!partAddr) return {}; auto prim=memory->read<std::uintptr_t>(partAddr + Offsets::BasePart::Primitive); if(!prim) return {}; return memory->read<RBX::Vec3>(prim + Offsets::Primitive::Position); }
}

namespace WorldVisuals {
void Render(ImDrawList* dl, const RBX::Mat4& v) {
    if (!variables::World::enabled) return;
    std::vector<WorldCache::Entry> local;
    {
        std::lock_guard<std::mutex> lk(WorldCache::mtx);
        if (WorldCache::entries.empty()) return;
        local = WorldCache::entries;
    }
    ImFont* font = WF_EspFont();
    float sz = WF_EspSize();
    const ImU32 white = IM_COL32(255,255,255,255);
    for (auto &e : local) {
        ImVec2 scr;
        if (!WF_ToScreen(e.pos, v, scr)) continue;
        bool showName = variables::World::name;
        bool showDist = variables::World::distance;
        std::string distTxt = std::to_string((int)e.dist) + "m";
        ImU32 distCol = IM_COL32(120,255,120,255);
        if (e.category=="plant" && e.plantIdx>=0 && e.plantIdx<7) distCol = WF_ToU32(variables::World::plantsColor[e.plantIdx]);
        else if (e.category=="ore" && e.oreIdx>=0 && e.oreIdx<3) distCol = WF_ToU32(variables::World::oresColor[e.oreIdx]);
        else if (e.category=="animal" && e.animalIdx>=0 && e.animalIdx<3) distCol = WF_ToU32(variables::World::animalsColor[e.animalIdx]);
        else if (e.category=="soldier" && e.soldierIdx>=0 && e.soldierIdx<4) distCol = WF_ToU32(variables::World::soldiersColor[e.soldierIdx]);
        else if (e.category=="tool" && e.toolIdx>=0 && e.toolIdx<7) distCol = WF_ToU32(variables::World::toolsColor[e.toolIdx]);
        else if (e.category=="plant") distCol = IM_COL32(80,255,80,255);
        else if (e.category=="ore") distCol = IM_COL32(200,200,60,255);
        else if (e.category=="tool") distCol = IM_COL32(120,180,255,255);
        else if (e.category=="soldier") distCol = IM_COL32(255,80,80,255);
        else if (e.category=="animal") distCol = IM_COL32(255,160,80,255);

        bool needBox=false, needHealth=false;
        if (e.category=="animal") { needBox=variables::World::animalsBox; needHealth=variables::World::animalsHealth; }
        if (e.category=="soldier") { needBox=variables::World::soldiersBox; needHealth=variables::World::soldiersHealth; }
        if (e.category=="tool") { needBox=variables::World::toolsBox; }
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
        bool isNPC = (e.category=="animal" || e.category=="soldier");
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
