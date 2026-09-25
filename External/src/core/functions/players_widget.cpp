// discord.gg/thenwefuckin
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "players_widget.h"
#include "players/players.h"
#include "../../render/menu/library.h"
#include "../../../ext/imgui/imgui.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "../../../ext/stb/stb_image.h"
#include "../../sdk/offsets.h"
#include "../../core/variables/variables.h"
#include "../../memory/memory.h"
#include <d3d11.h>
#include <windows.h>
#include <winhttp.h>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#pragma comment(lib, "winhttp.lib")
namespace PlayersWidget {
namespace { bool g_open = false; ID3D11Device* g_dev=nullptr; }
void SetOpen(bool open){ g_open=open; }
bool IsOpen(){ return g_open; }
namespace {
struct Thumb { ID3D11ShaderResourceView* tex=nullptr; int w=0,h=0; bool loading=false; };
std::unordered_map<long long, Thumb> g_thumbs;
std::mutex g_thumbMtx;
std::unordered_set<long long> g_pending;
std::string HttpGet(const std::wstring& host, const std::wstring& path){
    std::string out;
    HINTERNET hS=WinHttpOpen(L"jatos/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr,nullptr,0);
    if(!hS) return out;
    HINTERNET hC=WinHttpConnect(hS, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT,0);
    if(!hC){ WinHttpCloseHandle(hS); return out; }
    HINTERNET hR=WinHttpOpenRequest(hC,L"GET",path.c_str(),nullptr,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE);
    if(!hR){ WinHttpCloseHandle(hC); WinHttpCloseHandle(hS); return out; }
    if(!WinHttpSendRequest(hR,WINHTTP_NO_ADDITIONAL_HEADERS,0,WINHTTP_NO_REQUEST_DATA,0,0,0)){ WinHttpCloseHandle(hR); WinHttpCloseHandle(hC); WinHttpCloseHandle(hS); return out; }
    if(!WinHttpReceiveResponse(hR,nullptr)){ WinHttpCloseHandle(hR); WinHttpCloseHandle(hC); WinHttpCloseHandle(hS); return out; }
    DWORD sz=0; std::vector<char> buf;
    do{
        DWORD avail=0; if(!WinHttpQueryDataAvailable(hR,&avail)) break; if(!avail) break;
        size_t old=buf.size(); buf.resize(old+avail);
        DWORD read=0; if(!WinHttpReadData(hR,buf.data()+old,avail,&read)) break;
        buf.resize(old+read);
        if(read==0) break;
    }while(true);
    out.assign(buf.begin(), buf.end());
    WinHttpCloseHandle(hR); WinHttpCloseHandle(hC); WinHttpCloseHandle(hS);
    return out;
}
std::string ExtractImageUrl(const std::string& json){
    auto p=json.find("imageUrl");
    if(p==std::string::npos) return {};
    auto q=json.find("http",p);
    if(q==std::string::npos) return {};
    auto e=json.find('"',q);
    if(e==std::string::npos) e=json.size();
    std::string url=json.substr(q,e-q);
    size_t pos=url.find("\\u0026"); while(pos!=std::string::npos){ url.replace(pos,6,"&"); pos=url.find("\\u0026",pos+1); }
    return url;
}
void RequestThumb(long long uid){
    if(uid<=0) return;
    {
        std::lock_guard<std::mutex> lk(g_thumbMtx);
        if(g_thumbs.find(uid)!=g_thumbs.end() || g_pending.find(uid)!=g_pending.end()) return;
        g_pending.insert(uid);
        g_thumbs[uid]={nullptr,0,0,true};
    }
    std::thread([uid]{
        std::string json=HttpGet(L"thumbnails.roblox.com", L"/v1/users/avatar?userIds="+std::to_wstring(uid)+L"&type=AvatarBust&size=420x420&format=Png&isCircular=false");
        std::string imgUrl=ExtractImageUrl(json);
        if(imgUrl.empty()){
            std::lock_guard<std::mutex> lk(g_thumbMtx); g_pending.erase(uid); auto it=g_thumbs.find(uid); if(it!=g_thumbs.end()) it->second.loading=false; return;
        }

        std::string host,path;
        {
            std::string tmp=imgUrl;
            if(tmp.rfind("https://",0)==0) tmp=tmp.substr(8);
            else if(tmp.rfind("http://",0)==0) tmp=tmp.substr(7);
            auto sl=tmp.find('/'); if(sl!=std::string::npos){ host=tmp.substr(0,sl); path=tmp.substr(sl); } else { host=tmp; path="/"; }
        }
        std::wstring whost(host.begin(), host.end()), wpath(path.begin(), path.end());
        std::string imgData=HttpGet(whost,wpath);
        if(imgData.empty()){
            std::lock_guard<std::mutex> lk(g_thumbMtx); g_pending.erase(uid); auto it=g_thumbs.find(uid); if(it!=g_thumbs.end()) it->second.loading=false; return;
        }
        int w=0,h=0,ch=0; unsigned char* pix=stbi_load_from_memory((const unsigned char*)imgData.data(), (int)imgData.size(), &w,&h,&ch,4);
        if(!pix || !g_dev){
            if(pix) stbi_image_free(pix);
            std::lock_guard<std::mutex> lk(g_thumbMtx); g_pending.erase(uid); auto it=g_thumbs.find(uid); if(it!=g_thumbs.end()) it->second.loading=false; return;
        }
        D3D11_TEXTURE2D_DESC desc{}; desc.Width=w; desc.Height=h; desc.MipLevels=1; desc.ArraySize=1; desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count=1; desc.Usage=D3D11_USAGE_DEFAULT; desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA sd{}; sd.pSysMem=pix; sd.SysMemPitch=w*4;
        ID3D11Texture2D* tex=nullptr; ID3D11ShaderResourceView* srv=nullptr;
        if(SUCCEEDED(g_dev->CreateTexture2D(&desc,&sd,&tex))){
            D3D11_SHADER_RESOURCE_VIEW_DESC vd{}; vd.Format=desc.Format; vd.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D; vd.Texture2D.MipLevels=1;
            if(SUCCEEDED(g_dev->CreateShaderResourceView(tex,&vd,&srv))){
                std::lock_guard<std::mutex> lk(g_thumbMtx);
                auto &t=g_thumbs[uid]; if(t.tex) t.tex->Release(); t.tex=srv; t.w=w; t.h=h; t.loading=false;
            }
            if(tex) tex->Release();
        }
        stbi_image_free(pix);
        std::lock_guard<std::mutex> lk(g_thumbMtx); g_pending.erase(uid);
    }).detach();
}
long long GetUserIdForName(const std::string& name){
    if(name.empty() || !Globals::players.Addr) return 0;
    auto plr = Globals::players.FindChild(name);
    if(!plr.Addr) return 0;
    long long uid = memory->read<long long>(plr.Addr + Offsets::Player::UserId);
    return uid>0?uid:0;
}
}
void RenderWindow(ID3D11Device* device){
    if(device) g_dev=device;
    if(!g_open) return;
    const imGuiCustom::Theme& theme=imGuiCustom::GetTheme();
    ImFont* font=imGuiCustom::GetFonts().CascadiaMonoBL?imGuiCustom::GetFonts().CascadiaMonoBL:ImGui::GetFont();
    const float fs=12.f*imGuiCustom::g_fontScale;
    ImGui::SetNextWindowSize(ImVec2(680.f,560.f),ImGuiCond_Always);
    {
        ImVec2 pos(700.f,80.f);
        ImGuiWindow* mainW=ImGui::FindWindowByName("jatos");
        if(mainW){ pos.x=mainW->Pos.x+mainW->Size.x+20.f; pos.y=mainW->Pos.y; const ImVec2 disp=ImGui::GetIO().DisplaySize; if(pos.x+680.f>disp.x) pos.x=disp.x-680.f-10.f; if(pos.y+560.f>disp.y) pos.y=disp.y-560.f-10.f; if(pos.x<0) pos.x=10.f; if(pos.y<0) pos.y=10.f; }
        ImGui::SetNextWindowPos(pos,ImGuiCond_FirstUseEver);
    }
    if(!ImGui::Begin("##players_widget",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse)){ ImGui::End(); return; }
    const ImVec2 origin=ImVec2(std::floor(ImGui::GetWindowPos().x),std::floor(ImGui::GetWindowPos().y));
    const ImVec2 winMax=origin+ImVec2(680.f,560.f);
    ImDrawList* draw=ImGui::GetWindowDrawList();
    draw->AddRectFilled(origin,winMax,imGuiCustom::ColorU32(theme.WindowBg),0.0f);
    draw->AddRect(origin,winMax,imGuiCustom::OutlineBlack(),0.0f,0,1.0f);
    draw->AddRect(origin+ImVec2(1.f,1.f),winMax-ImVec2(1.f,1.f),imGuiCustom::OutlineInner(),0.0f,0,1.0f);
    draw->AddRectFilled(origin,origin+ImVec2(680.f,1.5f),imGuiCustom::ColorU32(ImVec4(0.5373f,0.7647f,0.7490f,1.0f)),0.0f);
    const ImVec2 title_sz=font->CalcTextSizeA(fs,FLT_MAX,0.f,"Players");
    draw->AddText(font,fs,origin+ImVec2(std::floor((680.f-title_sz.x)*0.5f),3.5f),imGuiCustom::ColorU32(theme.TextBright),"Players");
    ImGui::PushFont(font);
    const ImVec2 cardLMin=origin+ImVec2(6.f,20.f);
    const ImVec2 cardLMax=cardLMin+ImVec2(312.f,532.f);
    const ImVec2 cardRMin=origin+ImVec2(324.f,20.f);
    const ImVec2 cardRMax=cardRMin+ImVec2(350.f,532.f);
    draw->AddRectFilled(cardLMin,cardLMax,imGuiCustom::ColorU32(theme.CardBg),0.0f);
    draw->AddRect(cardLMin,cardLMax,imGuiCustom::OutlineBlack(),0.0f,0,1.0f);
    draw->AddRect(cardLMin+ImVec2(1.f,1.f),cardLMax-ImVec2(1.f,1.f),imGuiCustom::OutlineInner(),0.0f,0,1.0f);
    draw->AddRectFilled(cardRMin,cardRMax,imGuiCustom::ColorU32(theme.CardBg),0.0f);
    draw->AddRect(cardRMin,cardRMax,imGuiCustom::OutlineBlack(),0.0f,0,1.0f);
    draw->AddRect(cardRMin+ImVec2(1.f,1.f),cardRMax-ImVec2(1.f,1.f),imGuiCustom::OutlineInner(),0.0f,0,1.0f);
    static std::vector<PlayersTab::Entry> entries;
    static auto lastGather=std::chrono::steady_clock::now()-std::chrono::seconds(1);
    auto now=std::chrono::steady_clock::now();
    if(std::chrono::duration_cast<std::chrono::milliseconds>(now-lastGather).count()>150){ entries=PlayersTab::Gather(); lastGather=now; }
    if(PlayersTab::selected.empty() && !entries.empty()) PlayersTab::selected=entries.front().name;
    {
        static float pwScroll=0.f;
        ImDrawList* fg=ImGui::GetWindowDrawList();
        const float rowH=20.f; const float listW=292.f; const float fsz=12.f*imGuiCustom::g_fontScale;
        ImVec2 listPos=cardLMin+ImVec2(6.f,6.f);
        const float visibleH=cardLMax.y-listPos.y-6.f;
        const float contentH=(float)entries.size()*(rowH+2.f);
        float maxScroll=contentH-visibleH; if(maxScroll<0.f) maxScroll=0.f;
        ImVec2 mp=ImGui::GetIO().MousePos;
        bool hoverList=(mp.x>=cardLMin.x&&mp.x<=cardLMax.x&&mp.y>=cardLMin.y&&mp.y<=cardLMax.y)&&ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        if(hoverList){
            float wheel=ImGui::GetIO().MouseWheel;
            if(wheel!=0.f&&!imGuiCustom::PopupBlocking()) pwScroll-=wheel*22.f;
        }
        if(pwScroll<0.f) pwScroll=0.f;
        if(pwScroll>maxScroll) pwScroll=maxScroll;
        fg->PushClipRect(cardLMin+ImVec2(4.f,4.f),cardLMax-ImVec2(4.f,4.f),true);
        ImVec2 cursor=ImVec2(listPos.x,listPos.y-pwScroll);
        for(size_t i=0;i<entries.size();++i){
            auto& e=entries[i];
            ImVec2 rowMin=cursor; ImVec2 rowMax=rowMin+ImVec2(listW,rowH);
            cursor.y+=rowH+2.f;
            if(rowMax.y<cardLMin.y+4.f||rowMin.y>cardLMax.y-4.f) continue;
            ImGui::SetCursorScreenPos(rowMin); ImGui::PushID((int)i);
            bool pressed=ImGui::InvisibleButton("##prow",ImVec2(listW,rowH));
            bool hovered=ImGui::IsItemHovered(); ImGuiID id=ImGui::GetItemID();
            float hoverAnim=imGuiCustom::AnimateFloat(id,hovered,16.f);
            float selAnim=imGuiCustom::AnimateFloat(id+1,e.name==PlayersTab::selected,16.f);
            if(pressed) PlayersTab::selected=e.name;
            bool isTgt=PlayersTab::IsMarked(e.name); bool isFr=PlayersTab::IsFriend(e.name);
            ImVec4 bg=imGuiCustom::LerpColor(theme.ControlBg,theme.ControlInactive,hoverAnim*0.45f);
            if(selAnim>0.01f) bg=theme.ControlInactive;
            if(hoverAnim>0.01f||selAnim>0.01f) fg->AddRectFilled(rowMin,rowMax,imGuiCustom::ColorU32(bg),0.0f);
            fg->AddRect(rowMin,rowMax,imGuiCustom::OutlineBlack(),0.0f,0,1.0f);
            fg->AddRect(rowMin+ImVec2(1.f,1.f),rowMax-ImVec2(1.f,1.f),imGuiCustom::OutlineInner(),0.0f,0,1.0f);
            float tx=rowMin.x+6.f;
            if(isTgt){ fg->AddText(font,fsz,ImVec2(tx,rowMin.y+3.f),imGuiCustom::ColorU32(theme.TextBright),"[T]"); tx+=font->CalcTextSizeA(fsz,FLT_MAX,0.0f,"[T] ").x; }
            if(isFr){ fg->AddText(font,fsz,ImVec2(tx,rowMin.y+3.f),IM_COL32(100,220,120,255),"[F]"); tx+=font->CalcTextSizeA(fsz,FLT_MAX,0.0f,"[F] ").x; }
            ImVec4 txtCol=imGuiCustom::LerpColor(theme.Text,theme.TextBright,hoverAnim*0.35f+selAnim*0.4f);
            fg->AddText(font,fsz,ImVec2(tx,rowMin.y+3.f),imGuiCustom::ColorU32(txtCol),e.name.c_str());
            ImGui::PopID();
        }
        if(entries.empty()) fg->AddText(font,fsz,listPos,imGuiCustom::ColorU32(theme.Text),"No players found");
        fg->PopClipRect();
    }
    {
        ImDrawList* fg=ImGui::GetWindowDrawList();
        fg->PushClipRect(cardRMin+ImVec2(6.f,6.f),cardRMax-ImVec2(6.f,6.f),true);
        if(PlayersTab::selected.empty()){
            fg->AddText(font,12.f*imGuiCustom::g_fontScale,cardRMin+ImVec2(10.f,10.f),imGuiCustom::ColorU32(theme.Text),"Select a player");
        } else {
            const PlayersTab::Entry* e=PlayersTab::Find(entries,PlayersTab::selected);
            ImVec2 cur=cardRMin+ImVec2(10.f,10.f);

            const float previewSize=300.f;
            const ImVec2 previewMin=ImVec2(cardRMin.x + (350.f-previewSize)*0.5f, cur.y);
            const ImVec2 previewMax=previewMin+ImVec2(previewSize,previewSize);

            long long uid=GetUserIdForName(PlayersTab::selected);
            if(uid>0) RequestThumb(uid);
            Thumb thumb{}; bool hasThumb=false;
            {
                std::lock_guard<std::mutex> lk(g_thumbMtx);
                auto it=g_thumbs.find(uid);
                if(it!=g_thumbs.end() && it->second.tex){ thumb=it->second; hasThumb=true; }
            }
            static float pwZoom=1.0f; static long long pwUid=-1;
            if(pwUid!=uid){ pwUid=uid; pwZoom=1.0f; }
            ImGui::SetCursorScreenPos(previewMin);
            ImGui::InvisibleButton("##preview_interact", ImVec2(previewSize,previewSize));
            bool hover = ImGui::IsItemHovered();
            if(hover){
                float wheel = ImGui::GetIO().MouseWheel;
                if(wheel!=0){ pwZoom += wheel*0.12f; if(pwZoom<1.f) pwZoom=1.f; if(pwZoom>3.f) pwZoom=3.f; }
                if(ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) pwZoom=1.f;
            }
            if(hasThumb && thumb.tex){
                float uvScale = 1.f/pwZoom;
                ImVec2 uvMin = ImVec2(0.5f - 0.5f*uvScale, 0.5f - 0.5f*uvScale);
                ImVec2 uvMax = ImVec2(0.5f + 0.5f*uvScale, 0.5f + 0.5f*uvScale);

                fg->PushClipRect(previewMin, previewMax, true);
                fg->AddImage((ImTextureID)thumb.tex, previewMin, previewMax, uvMin, uvMax);
                fg->PopClipRect();
            } else {
                const char* txt=uid<=0?"No UserId": ( [&]{ std::lock_guard<std::mutex> lk(g_thumbMtx); auto it=g_thumbs.find(uid); return it!=g_thumbs.end() && it->second.loading; }() ? "Loading..." : "No thumbnail");
                ImVec2 tsz=font->CalcTextSizeA(12.f*imGuiCustom::g_fontScale,FLT_MAX,0.f,txt);
                fg->AddText(font,12.f*imGuiCustom::g_fontScale, ImVec2(previewMin.x+(previewSize-tsz.x)*0.5f, previewMin.y+(previewSize-tsz.y)*0.5f), imGuiCustom::ColorU32(theme.Text), txt);
            }

            if(variables::ESP::enabled){
                if(variables::ESP::boxes){
                    ImU32 col=imGuiCustom::ColorU32(variables::ESP::boxColor);
                    fg->AddRect(previewMin,previewMax,col,0.0f,0,1.5f);
                    if(variables::ESP::boxFilled){
                        ImU32 fcol=imGuiCustom::ColorU32(variables::ESP::boxFillColor);
                        fg->AddRectFilled(previewMin,previewMax, fcol & 0x55FFFFFF,0.0f);
                    }
                }

                const PlayerCache::CachedPlayer* cp=nullptr;
                for(auto &p: PlayerCache::players) if(p.name==PlayersTab::selected){ cp=&p; break; }
                if(!cp) for(auto &p: CbCache::players) if(p.name==PlayersTab::selected){ cp=&p; break; }
            if(!cp) for(auto &p: MlCache::players) if(p.name==PlayersTab::selected){ cp=&p; break; }
                if(cp && cp->isValid && cp->maxHealth>0){
                    float pct=cp->health/cp->maxHealth; pct=pct<0?0:(pct>1?1:pct);
                    ImVec2 hbMin=ImVec2(previewMin.x-6.f, previewMin.y);
                    ImVec2 hbMax=ImVec2(previewMin.x-2.f, previewMax.y);
                    fg->AddRectFilled(hbMin,hbMax, IM_COL32(30,30,30,200),0.0f);
                    float h=(hbMax.y-hbMin.y)*pct;
                    ImVec2 fillMin=ImVec2(hbMin.x, hbMax.y - h);
                    fg->AddRectFilled(fillMin, hbMax, imGuiCustom::ColorU32(variables::ESP::healthColor),0.0f);
                }
                if(variables::ESP::skeleton){
                    ImVec2 c=ImVec2((previewMin.x+previewMax.x)*0.5f, (previewMin.y+previewMax.y)*0.5f);
                    fg->AddCircle(c, 3.f, imGuiCustom::ColorU32(variables::ESP::skeletonColor), 8, 1.5f);
                }
            }

            const float bottomTextH = 86.f;
            const float buttonsH = 46.f;
            cur.y = cardRMax.y - 10.f - buttonsH - bottomTextH - 2.f;

            fg->AddText(font,13.f*imGuiCustom::g_fontScale,cur,imGuiCustom::ColorU32(theme.TextBright),PlayersTab::selected.c_str());
            cur.y+=18.f;
            bool isMarked=PlayersTab::IsMarked(PlayersTab::selected); bool isFriend=PlayersTab::IsFriend(PlayersTab::selected);
            if(isMarked||isFriend){
                std::string tags; if(isMarked) tags+="[TARGET] "; if(isFriend) tags+="[FRIEND]";
                fg->AddText(font,11.f*imGuiCustom::g_fontScale,cur,isMarked?imGuiCustom::ColorU32(theme.Accent):IM_COL32(100,220,120,255),tags.c_str());
                cur.y+=14.f;
            }
            fg->AddLine(cur,cur+ImVec2(330.f,0.f),imGuiCustom::OutlineInner(),1.f); cur.y+=8.f;
            const PlayerCache::CachedPlayer* cp2=nullptr;
            for(auto &p: PlayerCache::players) if(p.name==PlayersTab::selected){ cp2=&p; break; }
            if(!cp2) for(auto &p: CbCache::players) if(p.name==PlayersTab::selected){ cp2=&p; break; }
            auto drawRow=[&](const char* label,const char* val,ImU32 col=0){
                fg->AddText(font,12.f*imGuiCustom::g_fontScale,cur,imGuiCustom::ColorU32(theme.Text),label);
                ImVec2 sz=font->CalcTextSizeA(12.f*imGuiCustom::g_fontScale,FLT_MAX,0.0f,val);
                fg->AddText(font,12.f*imGuiCustom::g_fontScale,ImVec2(cardRMax.x-10.f-sz.x,cur.y),col?col:imGuiCustom::ColorU32(theme.TextBright),val);
                cur.y+=16.f;
            };
            if(cp2 && cp2->isValid){
                drawRow("Status","Alive",IM_COL32(100,220,120,255));
                if(cp2->maxHealth>0){ char b[32]; snprintf(b,sizeof(b),"%.0f / %.0f",cp2->health,cp2->maxHealth); drawRow("Health",b); }
                RBX::Vec3 myPos=PlayerCache::localPlayerPos; if(myPos.X==0&&myPos.Y==0&&myPos.Z==0) myPos=CbCache::localPos;
                RBX::Vec3 their=cp2->position; if(their.X==0&&their.Y==0&&their.Z==0) their=PlayersTab::PartPosOf(cp2->rootPartAddr?cp2->rootPartAddr:cp2->headAddr);
                float d=sqrtf((their.X-myPos.X)*(their.X-myPos.X)+(their.Y-myPos.Y)*(their.Y-myPos.Y)+(their.Z-myPos.Z)*(their.Z-myPos.Z));
                char db[32]; snprintf(db,sizeof(db),"%.0f studs",d); drawRow("Distance",db);
                if(!cp2->teamName.empty()) drawRow("Team",cp2->teamName.c_str());
                if(cp2->tool!="None" && !cp2->tool.empty()) drawRow("Weapon",cp2->tool.c_str());
            } else drawRow("Status","In Server",imGuiCustom::ColorU32(theme.Text));
            cur.y = cardRMax.y - 10.f - buttonsH;
            float btnW=(330.f-6.f)*0.5f;
            auto btn=[&](const char* label,ImVec2 pos,ImVec2 sz,bool active=false){
                ImGui::SetCursorScreenPos(pos); ImVec2 wpos=ImGui::GetWindowPos(); ImVec2 rel(pos.x-wpos.x,pos.y-wpos.y); return imGuiCustom::ButtonPos(label,rel,sz,active);
            };
            bool isM=PlayersTab::IsMarked(PlayersTab::selected); bool isF=PlayersTab::IsFriend(PlayersTab::selected);
            if(btn(isM?"Untarget##pw_t":"Target##pw_t",cur,ImVec2(btnW,20.f))){ if(isM) PlayersTab::target.clear(); else PlayersTab::target=PlayersTab::selected; }
            if(btn(isF?"Unfriend##pw_f":"Friend##pw_f",ImVec2(cur.x+btnW+6.f,cur.y),ImVec2(btnW,20.f))){ if(isF) PlayersTab::friends.erase(PlayersTab::selected); else PlayersTab::friends.insert(PlayersTab::selected); }
            cur.y+=26.f;
            if(btn("Teleport to Player##pw_tp",cur,ImVec2(330.f,20.f))){ if(e) PlayersTab::TeleportTo(PlayersTab::ResolveTargetPos(*e)); else { PlayersTab::Entry tmp{PlayersTab::selected,0,0,0}; PlayersTab::TeleportTo(PlayersTab::ResolveTargetPos(tmp)); } }
        }
        fg->PopClipRect();
    }
    ImGui::PopFont(); ImGui::End();
}
}
