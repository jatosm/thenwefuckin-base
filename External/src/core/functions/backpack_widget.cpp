#ifndef IMGUI_DEFINE_MATH_OPERATORS

#define IMGUI_DEFINE_MATH_OPERATORS

#endif

#include "backpack_widget.h"

#include "../../render/menu/library.h"

#include "../../../ext/imgui/imgui.h"

#define STB_IMAGE_IMPLEMENTATION

#define STB_IMAGE_STATIC

#define STBI_ONLY_PNG

#define STBI_NO_STDIO

#include "../../../ext/stb/stb_image.h"

#include <filesystem>

#include "../../core/variables/variables.h"

#include "../../core/cache/cache.h"

#include "../../core/cache/cb_cache.h"

#include "../../core/cache/pf_cache.h"

#include "../../core/functions/aim/aim.h"

#include "../../core/globals/globals.h"

#include <iostream>

#include <windows.h>

#include "../../memory/memory.h"

#include "../../sdk/sdk.h"

#include "../../sdk/offsets.h"

#include <d3d11.h>

#include <string>

#include <vector>

#include <unordered_map>

#include <initializer_list>

#include <cstdio>

#include <chrono>

#include "hotbar_icons/icons_embedded.h"

namespace BackpackWidget{

static bool g_open=true;

static ImVec2 g_pos=ImVec2(-1,-1);

static bool g_drag=false;

static ImVec2 g_dragOff={0,0};

static ID3D11Device* g_dev=nullptr;

struct IconTex{ ID3D11ShaderResourceView* tex=nullptr; int w=0; int h=0; };

static std::unordered_map<std::string, IconTex> g_iconMap;

static bool g_iconsLoaded=false;

static std::string NormIcon(const std::string& s){ std::string o; o.reserve(s.size()); for(char c: s){ if(c==' '||c=='_'||c=='-'||c=='\''||c=='"') continue; if(c>='A'&&c<='Z') c+=(char)('a'-'A'); o.push_back(c); } return o; }

static void EnsureIcons(){
 if(g_iconsLoaded||!g_dev) return;
 g_iconsLoaded=true;
 unsigned decoded=0, failed=0;
 for(unsigned i=0;i<HOTBAR_ICON_COUNT;++i){
  const char* key=HOTBAR_ICONS[i].key;
  const unsigned char* data=HOTBAR_ICONS[i].data;
  unsigned len=HOTBAR_ICONS[i].len;
  if(!key||!data||!len){ failed++; continue; }
  int w=0,h=0,ch=0; unsigned char* pix=stbi_load_from_memory(data,(int)len,&w,&h,&ch,4);
  if(!pix){ failed++; continue; }
  decoded++;
  D3D11_TEXTURE2D_DESC desc{}; desc.Width=(UINT)w; desc.Height=(UINT)h; desc.MipLevels=1; desc.ArraySize=1; desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count=1; desc.Usage=D3D11_USAGE_DEFAULT; desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
  D3D11_SUBRESOURCE_DATA sd{}; sd.pSysMem=pix; sd.SysMemPitch=(UINT)(w*4);
  ID3D11Texture2D* tex=nullptr; ID3D11ShaderResourceView* srv=nullptr;
  if(SUCCEEDED(g_dev->CreateTexture2D(&desc,&sd,&tex))){
   D3D11_SHADER_RESOURCE_VIEW_DESC vd{}; vd.Format=desc.Format; vd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D; vd.Texture2D.MipLevels=1;
   if(SUCCEEDED(g_dev->CreateShaderResourceView(tex,&vd,&srv))) g_iconMap[NormIcon(key)]={srv,w,h};
   if(tex) tex->Release();
  } else failed++;
  stbi_image_free(pix);
 }
  (void)decoded; (void)failed;
}

static IconTex* CategoryFallback(const std::string& k){
 auto has=[&](const char* s){ return k.find(s)!=std::string::npos; };
 auto anyKey=[&](std::initializer_list<const char*> subs)->IconTex*{
  for(auto &kv: g_iconMap){ for(auto s: subs){ if(kv.first.find(s)!=std::string::npos) return &kv.second; } } return nullptr;
 };
 if(has("armor")||has("armour")){ if(auto* t=anyKey({"armourplate","bonearmour","heavypadding","steelchest","militarychest","woodchest","salvagedchest"})) return t; }
 if(has("helm")||has("hat")||has("cap")||has("mask")||has("balaclava")||has("hardhat")){ if(auto* t=anyKey({"helm","hat","cap","mask","balaclava","hardhat"})) return t; }
 if(has("boot")||has("shoe")){ if(auto* t=anyKey({"boot"})) return t; }
 if(has("glove")){ if(auto* t=anyKey({"glove"})) return t; }
 if(has("pant")||has("leg")||has("jean")){ if(auto* t=anyKey({"pant","leg"})) return t; }
 if(has("shirt")||has("chest")||has("jacket")||has("hoodie")||has("tunic")||has("poncho")||has("tanktop")){ if(auto* t=anyKey({"shirt","chest","jacket","hoodie","tunic","poncho","tanktop"})) return t; }
 return nullptr;
}
static const char* AliasIconKey(const std::string& k){
 if(k=="woodhelmet"||k=="woodenhelmet"||k=="woodenhelm") return "woodhelm";
 if(k=="woodlegs"||k=="woodenlegs"||k=="woodleggings") return "woodleg";
 if(k=="woodchestplate"||k=="woodenchest"||k=="woodenarmor") return "woodchest";
 if(k=="bow"||k=="woodbow") return "woodenbow";
 if(k=="ak"||k=="ak47"||k=="fallenak") return "fallenak47";
 if(k=="cross bow") return "crossbow";
 return nullptr;
}

static IconTex* FindIcon(const std::string& name){
 EnsureIcons();
 std::string k=NormIcon(name);
 if(k.empty()) return nullptr;
 if(const char* a=AliasIconKey(k)) k=a;
 auto it=g_iconMap.find(k);
 if(it!=g_iconMap.end()) return &it->second;
 while(!k.empty() && k.back()>='0' && k.back()<='9') k.pop_back();
 it=g_iconMap.find(k);
 if(it!=g_iconMap.end()) return &it->second;
 if(k.size()>=4){
  IconTex* best=nullptr; size_t bestLen=0;
  for(auto &kv: g_iconMap){
   const std::string& key=kv.first;
   if(key.size()<4) continue;
   if(k.find(key)!=std::string::npos || key.find(k)!=std::string::npos){
    size_t n=key.size()<k.size()?key.size():k.size();
    if(n>bestLen){ bestLen=n; best=&kv.second; }
   }
  }
  if(best) return best;
 }
 if(auto* t=CategoryFallback(k)) return t;
 return nullptr;
}

static bool IsHotbarJunk(const std::string& n){
 std::string k=NormIcon(n);
 if(k.empty()||k=="holstermodel"||k=="holster") return true;
 static const char* skip[]={
  "hair","head","torso","uppertorso","lowertorso","humanoid","humanoidrootpart",
  "lefthand","righthand","leftarm","rightarm","leftleg","rightleg","leftfoot","rightfoot",
  "leftupperarm","rightupperarm","leftlowerarm","rightlowerarm",
  "leftupperleg","rightupperleg","leftlowerleg","rightlowerleg",
  "animate","health","bodycolors","face","rootpart","character"
 };
 for(auto s: skip) if(k==s) return true;
 return false;
}

static void DrawHotbarItem(ImDrawList* draw, ImFont* font, const ImVec2& sMin, float slotW, float slotH, const std::string& rawName, const ImVec4& textCol, const ImVec4& emptyCol){
 if(rawName.empty()){
  const char* txt2="Empty";
  ImVec2 tsz2=font->CalcTextSizeA(9.f*imGuiCustom::g_fontScale, FLT_MAX, 0.f, txt2);
  ImVec2 tpos2=ImVec2(sMin.x + (slotW-tsz2.x)*0.5f, sMin.y + (slotH-tsz2.y)*0.5f);
  draw->AddText(font,9.f*imGuiCustom::g_fontScale, tpos2, imGuiCustom::ColorU32(emptyCol), txt2);
  return;
 }
 std::string lookup=rawName;
 if(lookup.rfind("rbxassetid://",0)==0) lookup=lookup.substr(13);
 IconTex* icon=FindIcon(lookup);
 if(icon&&icon->tex){
  float iconSz=40.f;
  ImVec2 iconMin=ImVec2(sMin.x+(slotW-iconSz)*0.5f, sMin.y+(slotH-iconSz)*0.5f+2.f);
  draw->AddImage((ImTextureID)icon->tex, iconMin, iconMin+ImVec2(iconSz,iconSz));
  return;
 }
 std::string txt=lookup;
 if(txt.size()>10) txt=txt.substr(0,9)+".";
 ImVec2 tsz=font->CalcTextSizeA(10.f*imGuiCustom::g_fontScale, FLT_MAX, 0.f, txt.c_str());
 ImVec2 tpos=ImVec2(sMin.x + (slotW-tsz.x)*0.5f, sMin.y + (slotH-tsz.y)*0.5f);
 draw->AddText(font,10.f*imGuiCustom::g_fontScale, tpos, imGuiCustom::ColorU32(textCol), txt.c_str());
}

void SetOpen(bool o){g_open=o;}

bool IsOpen(){return g_open;}

namespace {

constexpr uintptr_t OffGuiImage=0x988;

std::string ReadGuiString(uintptr_t addr,uintptr_t off){

 if(!addr) return {};

 for(uintptr_t o : {off, off+8, off-8, off+0x10, off-0x10}){

  std::string s=memory->read_string(addr+o);

  if(!s.empty() && s.find("rbxasset")!=std::string::npos) return s;

  uintptr_t ptr=memory->read<std::uintptr_t>(addr+o);

  if(ptr){ s=memory->read_string(ptr); if(!s.empty() && s.find("rbxasset")!=std::string::npos) return s; }

 }

 std::string s=memory->read_string(addr+off);

 if(!s.empty()) return s;

 uintptr_t ptr=memory->read<std::uintptr_t>(addr+off);

 if(ptr) s=memory->read_string(ptr);

 return s;

}

std::string GetImageAsset(uintptr_t labelAddr){ return ReadGuiString(labelAddr, OffGuiImage); }

uintptr_t FindInventoryFrame(){

 static uintptr_t cached=0; static auto last=std::chrono::steady_clock::now()-std::chrono::seconds(5);

 auto now=std::chrono::steady_clock::now();

 if(cached && std::chrono::duration_cast<std::chrono::milliseconds>(now-last).count()<2000){

  RBX::RbxInstance inst(cached);

  if(!inst.GetClass().empty()) return cached;

 }

 if(!Globals::players.Addr) return cached;

 uintptr_t lp=memory->read<std::uintptr_t>(Globals::players.Addr + Offsets::Player::LocalPlayer);

 if(!lp) return cached;

 RBX::RbxInstance player(lp);

 auto playerGui=player.FindChild("PlayerGui");

 if(!playerGui.Addr) return cached;

 std::vector<uintptr_t> stack; stack.push_back(playerGui.Addr);

 size_t vis=0;

 while(!stack.empty() && vis<1500){

  uintptr_t cur=stack.back(); stack.pop_back(); vis++;

  RBX::RbxInstance inst(cur);

  std::string name=inst.GetName();

  std::string cls=inst.GetClass();

  if((cls=="Frame"||cls=="ScrollingFrame") && (name.find("Backpack")!=std::string::npos || name.find("Hotbar")!=std::string::npos || name.find("Inventory")!=std::string::npos)){

   cached=cur; last=now; return cur;

  }

  auto kids=inst.GetChildList();

  for(auto &c: kids) if(c.Addr) stack.push_back(c.Addr);

 }

 cached=0; last=now; return cached;

}

std::vector<std::string> CollectHotbar(){

 static std::vector<std::string> cachedOut(6,"");

 static std::string cachedTargetDbg;

 static auto lastCollect=std::chrono::steady_clock::now()-std::chrono::seconds(1);

 static auto lastDbg=std::chrono::steady_clock::now()-std::chrono::seconds(1);

 auto now=std::chrono::steady_clock::now();

 std::string curTargetDbg;

 uintptr_t curLocked=0; bool curHas=false; bool curEnabled=false;

 curEnabled=variables::Aimbot::enabled;

 curHas=Aimbot::hasTarget;

 curLocked=Aimbot::lockedPlayerAddr;

 if(curEnabled && curHas && curLocked!=0){

  for(auto &pc : PlayerCache::players){ if(pc.isValid && pc.characterAddr==curLocked){ curTargetDbg=pc.name; break; } }

  if(curTargetDbg.empty()) for(auto &pc : PlayerCache::players){ if(pc.isValid && pc.playerAddr==curLocked){ curTargetDbg=pc.name; break; } }

  if(curTargetDbg.empty()) for(auto &pc : CbCache::players){ if(pc.isValid && (pc.characterAddr==curLocked || pc.playerAddr==curLocked || pc.rootPartAddr==curLocked)){ curTargetDbg=pc.name; break; } }

  if(curTargetDbg.empty()) for(auto &pc : PfCache::players){ if(pc.isValid && pc.modelAddr==curLocked){ curTargetDbg=pc.name; break; } }

 }

 bool needUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now-lastCollect).count()>300 || curTargetDbg!=cachedTargetDbg;

 if(!needUpdate) return cachedOut;

 lastCollect=now; cachedTargetDbg=curTargetDbg;

 std::vector<std::string> out(6,"");

 std::string debugSrc="none";

 if(!curTargetDbg.empty()){

  debugSrc="target:"+curTargetDbg;

  if(Globals::players.Addr){

   auto plr=Globals::players.FindChild(curTargetDbg);

   if(plr.Addr){

    auto backpack=plr.FindChild("Backpack");

    if(backpack.Addr){

     for(auto &it: backpack.GetChildList()){

      if(it.Addr==0) continue;

      std::string n=it.GetName(); if(n.empty()||IsHotbarJunk(n)) continue;

      for(int k=0;k<6;++k) if(out[k].empty()){ out[k]=n; break; }

      bool full=true; for(int k=0;k<6;++k) if(out[k].empty()) full=false; if(full) break;

     }

    }

    auto ch=plr.GetModelRef();
    if(ch.Addr){
     std::vector<uintptr_t> cstack; cstack.push_back(ch.Addr);
     std::vector<std::string> armorFallback;
     size_t cvis=0;
     while(!cstack.empty() && cvis<1000){
      uintptr_t cur=cstack.back(); cstack.pop_back(); cvis++;
      RBX::RbxInstance inst(cur);
      if(cur!=ch.Addr){
       std::string cls=inst.GetClass();
       if(cls=="Model"){
        auto h=inst.FindChild("Handle");
        if(!h.Addr) h=inst.FindChild("Main");
        std::string n=inst.GetName();
        if(!n.empty() && !IsHotbarJunk(n)){
         if(h.Addr){
          bool already=false; for(int k=0;k<6;++k) if(out[k]==n) already=true;
          if(!already){ for(int k=0;k<6;++k) if(out[k].empty()){ out[k]=n; break; } }
         } else {
          bool already=false; for(auto &a: armorFallback) if(a==n) already=true; for(int k=0;k<6;++k) if(out[k]==n) already=true;
          if(!already) armorFallback.push_back(n);
         }
        }
       } else if(cls=="Tool" || cls=="HopperBin"){
        std::string n=inst.GetName(); if(!n.empty() && !IsHotbarJunk(n)){
         bool already=false; for(int k=0;k<6;++k) if(out[k]==n) already=true;
         if(!already){ for(int k=0;k<6;++k) if(out[k].empty()){ out[k]=n; break; } }
        }
       }
      }
      for(auto &cc: inst.GetChildList()) if(cc.Addr) cstack.push_back(cc.Addr);
      bool full=true; for(int k=0;k<6;++k) if(out[k].empty()) full=false; if(full) break;
     }
     for(auto &n: armorFallback){
      bool full=true; for(int k=0;k<6;++k) if(out[k].empty()) full=false; if(full) break;
      for(int k=0;k<6;++k) if(out[k].empty()){ out[k]=n; break; }
     }
    }

   }

  }

 } else {

  debugSrc="no target";

 }

  (void)now; (void)lastDbg; (void)curHas; (void)curEnabled; (void)curLocked; (void)curTargetDbg; (void)debugSrc;

 cachedOut=out; return out;

}

}

void RenderWindow(ID3D11Device* dev){

 if(dev) g_dev=dev;

 if(!g_open) return;

 const auto& theme=imGuiCustom::GetTheme();

 ImFont* font=imGuiCustom::GetFonts().CascadiaMonoBL?imGuiCustom::GetFonts().CascadiaMonoBL:ImGui::GetFont();

 const float fs=12.f*imGuiCustom::g_fontScale;

 ImGui::SetNextWindowSize(ImVec2(420.f,110.f),ImGuiCond_Always);

 {

  const ImVec2 disp=ImGui::GetIO().DisplaySize;

  if(g_pos.x<0) g_pos=ImVec2(disp.x*0.5f-210.f, disp.y-112.f);

  ImGuiIO& io=ImGui::GetIO();

  ImVec2 mouse=io.MousePos;

  ImVec2 titleMin=g_pos;

  ImVec2 titleMax=g_pos+ImVec2(420.f,15.f);

  bool hoverTitle = mouse.x>=titleMin.x && mouse.x<=titleMax.x && mouse.y>=titleMin.y && mouse.y<=titleMax.y;

  if(hoverTitle && ImGui::IsMouseClicked(0)){ g_drag=true; g_dragOff=ImVec2(mouse.x-g_pos.x, mouse.y-g_pos.y); }

  if(g_drag){

   if(ImGui::IsMouseDown(0)){ g_pos=ImVec2(mouse.x-g_dragOff.x, mouse.y-g_dragOff.y); if(g_pos.x<0) g_pos.x=0; if(g_pos.y<0) g_pos.y=0; if(g_pos.x+420.f>disp.x) g_pos.x=disp.x-420.f; if(g_pos.y+110.f>disp.y) g_pos.y=disp.y-110.f; }

   else g_drag=false;

  }

  ImGui::SetNextWindowPos(g_pos,ImGuiCond_Always);

 }

 if(!ImGui::Begin("##backpack_widget",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse)){ImGui::End();return;}

 const ImVec2 origin=ImVec2(std::floor(ImGui::GetWindowPos().x),std::floor(ImGui::GetWindowPos().y));

 const ImVec2 winMax=origin+ImVec2(420.f,110.f);

 ImDrawList* draw=ImGui::GetWindowDrawList();

 draw->AddRectFilled(origin,winMax,imGuiCustom::ColorU32(theme.WindowBg),0.0f);

 draw->AddRect(origin,winMax,imGuiCustom::OutlineBlack(),0.0f,0,1.0f);

 draw->AddRect(origin+ImVec2(1.f,1.f),winMax-ImVec2(1.f,1.f),imGuiCustom::OutlineInner(),0.0f,0,1.0f);

 draw->AddRectFilled(origin,origin+ImVec2(420.f,1.5f),imGuiCustom::ColorU32(ImVec4(0.5373f,0.7647f,0.7490f,1.0f)),0.0f);

 const ImVec2 title_sz=font->CalcTextSizeA(fs,FLT_MAX,0.f,"Hotbar");

 draw->AddText(font,fs,origin+ImVec2(std::floor((420.f-title_sz.x)*0.5f),3.5f),imGuiCustom::ColorU32(theme.TextBright),"Hotbar");

 ImGui::PushFont(font);

 const ImVec2 panelMin=origin+ImVec2(6.f,20.f);

 const ImVec2 panelMax=panelMin+ImVec2(408.f,82.f);

 draw->AddRectFilled(panelMin,panelMax,imGuiCustom::ColorU32(theme.CardBg),0.0f);

 draw->AddRect(panelMin,panelMax,imGuiCustom::OutlineBlack(),0.0f,0,1.0f);

 draw->AddRect(panelMin+ImVec2(1.f,1.f),panelMax-ImVec2(1.f,1.f),imGuiCustom::OutlineInner(),0.0f,0,1.0f);

 const float slotW=58.f, slotH=58.f, gap=6.f;

 ImVec2 gridMin=ImVec2(std::floor(panelMin.x + (408.f - (6*slotW+5*gap))*0.5f), std::floor(panelMin.y + (82.f-slotH)*0.5f));

 auto hotbar=CollectHotbar();

 for(int i=0;i<6;++i){

  ImVec2 sMin=ImVec2(std::floor(gridMin.x + i*(slotW+gap)), std::floor(gridMin.y));

  ImVec2 sMax=sMin+ImVec2(slotW,slotH);

  draw->AddRectFilled(sMin,sMax,imGuiCustom::ColorU32(theme.ControlBg),0.0f);

  draw->AddRect(sMin,sMax,imGuiCustom::OutlineBlack(),0.0f,0,1.0f);

  draw->AddRect(sMin+ImVec2(1.f,1.f),sMax-ImVec2(1.f,1.f),imGuiCustom::OutlineInner(),0.0f,0,1.0f);

  char num[4]; snprintf(num,sizeof(num),"%d",i+1);

  draw->AddText(font,12.f*imGuiCustom::g_fontScale, sMin+ImVec2(6.f,4.f), imGuiCustom::ColorU32(ImVec4(0.5f,0.5f,0.5f,1.f)), num);

  DrawHotbarItem(draw, font, sMin, slotW, slotH, hotbar[i], theme.TextBright, ImVec4(0.45f,0.45f,0.45f,1.f));

 }

 ImGui::PopFont();

 ImGui::End();

}

void RenderOverlay(ImDrawList* draw, ID3D11Device* devParam){

 if(devParam) g_dev=devParam;

 if(!g_open) return;

 const auto& theme=imGuiCustom::GetTheme();

 ImFont* font=imGuiCustom::GetFonts().CascadiaMonoBL?imGuiCustom::GetFonts().CascadiaMonoBL:ImGui::GetFont();

 auto hotbar=CollectHotbar();

 static ImVec2 ovPos=ImVec2(-1,-1); static bool ovDrag=false; static ImVec2 ovOff={0,0};

 ImVec2 disp = ImGui::GetIO().DisplaySize;

 float winW=420.f, winH=110.f;

 if(ovPos.x<0) ovPos=ImVec2(std::floor(disp.x*0.5f - winW*0.5f), std::floor(disp.y - winH - 10.f));

 ImVec2 mouse=ImGui::GetIO().MousePos;

 if(mouse.x>=ovPos.x && mouse.x<=ovPos.x+winW && mouse.y>=ovPos.y && mouse.y<=ovPos.y+15.f && ImGui::IsMouseClicked(0)){ ovDrag=true; ovOff=ImVec2(mouse.x-ovPos.x, mouse.y-ovPos.y); }

 if(ovDrag){ if(ImGui::IsMouseDown(0)){ ovPos=ImVec2(mouse.x-ovOff.x, mouse.y-ovOff.y); if(ovPos.x<0) ovPos.x=0; if(ovPos.y<0) ovPos.y=0; if(ovPos.x+winW>disp.x) ovPos.x=disp.x-winW; if(ovPos.y+winH>disp.y) ovPos.y=disp.y-winH; } else ovDrag=false; }

 ImVec2 origin = ImVec2(std::floor(ovPos.x), std::floor(ovPos.y));

 ImVec2 winMax = origin + ImVec2(winW, winH);

 draw->AddRectFilled(origin, winMax, imGuiCustom::ColorU32(theme.WindowBg), 0.0f);

 draw->AddRect(origin, winMax, imGuiCustom::OutlineBlack(), 0.0f, 0, 1.0f);

 draw->AddRect(origin+ImVec2(1.f,1.f), winMax-ImVec2(1.f,1.f), imGuiCustom::OutlineInner(),0.0f,0,1.0f);

 draw->AddRectFilled(origin, origin+ImVec2(winW,1.5f), imGuiCustom::ColorU32(ImVec4(0.5373f,0.7647f,0.7490f,1.0f)),0.0f);

 const float fs2=12.f*imGuiCustom::g_fontScale;

 ImVec2 tsz2=font->CalcTextSizeA(fs2, FLT_MAX, 0.f, "Hotbar");

 draw->AddText(font, fs2, ImVec2(std::floor(origin.x+(winW-tsz2.x)*0.5f), origin.y+3.5f), imGuiCustom::ColorU32(theme.TextBright), "Hotbar");

 ImVec2 panelMin=origin+ImVec2(6.f,20.f);

 ImVec2 panelMax=panelMin+ImVec2(408.f,82.f);

 draw->AddRectFilled(panelMin, panelMax, imGuiCustom::ColorU32(theme.CardBg),0.0f);

 draw->AddRect(panelMin, panelMax, imGuiCustom::OutlineBlack(),0.0f,0,1.0f);

 draw->AddRect(panelMin+ImVec2(1.f,1.f), panelMax-ImVec2(1.f,1.f), imGuiCustom::OutlineInner(),0.0f,0,1.0f);

 float slotW=58.f, slotH=58.f, gap=6.f;

 ImVec2 gridMin=ImVec2(std::floor(panelMin.x + (408.f - (6*slotW+5*gap))*0.5f), std::floor(panelMin.y + (82.f-slotH)*0.5f));

 for(int i=0;i<6;++i){

  ImVec2 sMin=ImVec2(std::floor(gridMin.x + i*(slotW+gap)), std::floor(gridMin.y));

  ImVec2 sMax=sMin+ImVec2(slotW,slotH);

  draw->AddRectFilled(sMin, sMax, imGuiCustom::ColorU32(theme.ControlBg),0.0f);

  draw->AddRect(sMin, sMax, imGuiCustom::OutlineBlack(),0.0f,0,1.0f);

  draw->AddRect(sMin+ImVec2(1.f,1.f), sMax-ImVec2(1.f,1.f), imGuiCustom::OutlineInner(),0.0f,0,1.0f);

  char num[4]; snprintf(num,sizeof(num),"%d",i+1);

  draw->AddText(font, 12.f*imGuiCustom::g_fontScale, sMin+ImVec2(6.f,4.f), imGuiCustom::ColorU32(ImVec4(0.5f,0.5f,0.5f,1.f)), num);

  DrawHotbarItem(draw, font, sMin, slotW, slotH, hotbar[i], theme.TextBright, ImVec4(0.45f,0.45f,0.45f,1.f));

 }

}

}
