#include "freecam.h"
#include "../../variables/variables.h"
#include "../../globals/globals.h"
#include "../../cache/cache.h"
#include "../../keys/keys.h"
#include "../../../memory/memory.h"
#include "../../../sdk/offsets.h"
#include "../../../sdk/sdk.h"
#include <windows.h>
#include <thread>
#include <chrono>
#include <cmath>
#include <atomic>
#include <mutex>

namespace Freecam {
static constexpr float PI = 3.14159265358979f;
static constexpr int CAM_TYPE_SCRIPTABLE = 6;

static std::atomic<bool> freecamThreadRunning{false};
static std::thread freecamThread;
static std::mutex freecamMutex;
static RBX::CFrame targetCameraCFrame{};
static RBX::CFrame targetPlayerCFrame{};
static uintptr_t cachedHrpAddr=0;
static uintptr_t cachedHrpPrimAddr=0;
static uintptr_t cachedHumanoidAddr=0;
static float savedWalkspeed=16.f;
static float savedWalkspeedCheck=16.f;
static bool shouldFreezePlayer=false;

static bool key_active(int key,int mode,bool& tog,bool& was){
    if(mode==2||key==0) return true;
    bool down=false;
    if(key>=ImGuiKey_NamedKey_BEGIN) down=Keys::IsKeyPressed(key);
    else down=(GetAsyncKeyState(key)&0x8000)!=0;
    if(mode==1){ if(down&&!was) tog=!tog; was=down; return tog; }
    was=down; return down;
}
static RBX::CFrame BuildCFrame(float pitch,float yaw,const RBX::Vec3& pos){
    float cy=std::cos(yaw), sy=std::sin(yaw);
    float cp=std::cos(pitch), sp=std::sin(pitch);
    RBX::CFrame cf{};
    cf.data[0]=cy; cf.data[1]=sy*sp; cf.data[2]=sy*cp;
    cf.data[3]=0; cf.data[4]=cp; cf.data[5]=-sp;
    cf.data[6]=-sy; cf.data[7]=cy*sp; cf.data[8]=cy*cp;
    cf.data[9]=pos.X; cf.data[10]=pos.Y; cf.data[11]=pos.Z;
    return cf;
}
static void CFrameToPitchYaw(const RBX::CFrame& cf,float& pitch,float& yaw){
    float sy = -cf.data[6];
    float cy = std::sqrt(cf.data[0]*cf.data[0] + cf.data[3]*cf.data[3]);
    yaw = std::atan2(sy, cy);
    pitch = std::atan2(-cf.data[5], cf.data[4]);
}
void FreecamLoop(){
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    while(freecamThreadRunning && Globals::running){
        RBX::CFrame localCam{}; RBX::CFrame localPlayer{}; bool localFreeze=false;
        uintptr_t localPrim=0, localHum=0;
        { std::lock_guard<std::mutex> lk(freecamMutex); localCam=targetCameraCFrame; localPlayer=targetPlayerCFrame; localFreeze=shouldFreezePlayer; localPrim=cachedHrpPrimAddr; localHum=cachedHumanoidAddr; }
        uintptr_t camAddr=0;
        if(Globals::workspace.Addr) camAddr=memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::CurrentCamera);
        if(camAddr){
            memory->write<int>(camAddr + Offsets::Camera::CameraType, CAM_TYPE_SCRIPTABLE);
            memory->write<std::uintptr_t>(camAddr + Offsets::Camera::CameraSubject, 0);
            memory->write<RBX::CFrame>(camAddr + Offsets::Camera::Rotation, localCam);
        }
        if(localFreeze && localHum) { memory->write<float>(localHum + Offsets::Humanoid::Walkspeed, 0.f); memory->write<float>(localHum + Offsets::Humanoid::WalkspeedCheck, 0.f); }
        if(localFreeze && localPrim){
            memory->write<RBX::CFrame>(localPrim + Offsets::Primitive::Rotation, localPlayer);
            memory->write<RBX::Vec3>(localPrim + 0x14c, RBX::Vec3{0,0,0});
            uint8_t f=memory->read<uint8_t>(localPrim + Offsets::Primitive::Flags); f|= (uint8_t)Offsets::PrimitiveFlags::Anchored; memory->write<uint8_t>(localPrim + Offsets::Primitive::Flags, f);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
bool IsActive(){ static bool tog=false,was=false; return variables::Freecam::enabled && key_active(variables::Freecam::key, variables::Freecam::keyMode, tog, was); }
void Shutdown(){ freecamThreadRunning=false; if(freecamThread.joinable()) freecamThread.join(); }
void Update(){
    static bool wasActive=false;
    static int savedCamType=5;
    static uintptr_t savedCamSubject=0;
    static RBX::CFrame savedCFrame{};
    static float pitch=0,yaw=0;
    static RBX::Vec3 position{};
    static bool hrpWasAnchored=false;
    static RBX::CFrame savedPlayerCFrame{};
    static bool keyTog=false,keyWas=false;
    auto restoreState=[&](){
        freecamThreadRunning=false; if(freecamThread.joinable()) freecamThread.join();
        auto cam = Globals::workspace.Addr ? RBX::RbxInstance{memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::CurrentCamera)} : RBX::RbxInstance{};
        if(cam.Addr){ memory->write<int>(cam.Addr + Offsets::Camera::CameraType, savedCamType); memory->write<std::uintptr_t>(cam.Addr + Offsets::Camera::CameraSubject, savedCamSubject); memory->write<RBX::CFrame>(cam.Addr + Offsets::Camera::Rotation, savedCFrame); }
        if(cachedHrpPrimAddr){ uint8_t f=memory->read<uint8_t>(cachedHrpPrimAddr + Offsets::Primitive::Flags); if(hrpWasAnchored) f|= (uint8_t)Offsets::PrimitiveFlags::Anchored; else f&=~(uint8_t)Offsets::PrimitiveFlags::Anchored; memory->write<uint8_t>(cachedHrpPrimAddr + Offsets::Primitive::Flags, f); memory->write<RBX::Vec3>(cachedHrpPrimAddr + 0x14c, RBX::Vec3{0,0,0}); cachedHrpAddr=0; cachedHrpPrimAddr=0; }
        else if(cachedHrpAddr){ RBX::RbxInstance hrp(cachedHrpAddr); hrp.SetAnchored(hrpWasAnchored); hrp.SetVelocity({0,0,0}); cachedHrpAddr=0; cachedHrpPrimAddr=0; }
        if(cachedHumanoidAddr){ memory->write<float>(cachedHumanoidAddr + Offsets::Humanoid::Walkspeed, savedWalkspeed); memory->write<float>(cachedHumanoidAddr + Offsets::Humanoid::WalkspeedCheck, savedWalkspeedCheck); cachedHumanoidAddr=0; }
        wasActive=false;
    };
    bool active = variables::Freecam::enabled && key_active(variables::Freecam::key, variables::Freecam::keyMode, keyTog, keyWas);
    if(!active){ if(wasActive) restoreState(); return; }
    if(!Globals::localPlayer.Addr || !Globals::workspace.Addr){ if(wasActive) restoreState(); return; }
    auto wsCamAddr = memory->read<std::uintptr_t>(Globals::workspace.Addr + Offsets::Workspace::CurrentCamera);
    RBX::RbxInstance camInst(wsCamAddr);
    if(!camInst.Addr) return;
    if(!wasActive){
        savedCamType = memory->read<int>(camInst.Addr + Offsets::Camera::CameraType);
        savedCamSubject = memory->read<std::uintptr_t>(camInst.Addr + Offsets::Camera::CameraSubject);
        savedCFrame = memory->read<RBX::CFrame>(camInst.Addr + Offsets::Camera::Rotation);

        {
            RBX::RbxInstance ch = Globals::localPlayer.GetModelInstance();
            RBX::RbxInstance hrpTmp = ch.Addr ? ch.FindFirstChild("HumanoidRootPart") : RBX::RbxInstance{};
            if(hrpTmp.Addr) position = hrpTmp.GetPos();
            else position = savedCFrame.GetPosition();
        }
        CFrameToPitchYaw(savedCFrame, pitch, yaw);
        RBX::RbxInstance character = Globals::localPlayer.GetModelInstance();
        if(character.Addr){
            if(variables::Freecam::freezeCharacter){
                RBX::RbxInstance hrp = character.FindFirstChild("HumanoidRootPart");
                if(hrp.Addr){ savedPlayerCFrame = memory->read<RBX::CFrame>(hrp.Addr ? memory->read<std::uintptr_t>(hrp.Addr + Offsets::BasePart::Primitive) + Offsets::Primitive::Rotation : 0); hrpWasAnchored = hrp.GetAnchored(); cachedHrpAddr=hrp.Addr; cachedHrpPrimAddr=hrp.GetPrimitivePtr(); hrp.SetAnchored(true); hrp.SetVelocity({0,0,0}); }
            }
            RBX::RbxInstance humanoid = character.FindFirstChild("Humanoid");
            if(humanoid.Addr){ cachedHumanoidAddr=humanoid.Addr; savedWalkspeed=memory->read<float>(humanoid.Addr + Offsets::Humanoid::Walkspeed); savedWalkspeedCheck=memory->read<float>(humanoid.Addr + Offsets::Humanoid::WalkspeedCheck); if(variables::Freecam::freezeCharacter){ memory->write<float>(humanoid.Addr + Offsets::Humanoid::Walkspeed, 0.f); memory->write<float>(humanoid.Addr + Offsets::Humanoid::WalkspeedCheck, 0.f); } }
        }
        { std::lock_guard<std::mutex> lk(freecamMutex); targetCameraCFrame=BuildCFrame(pitch,yaw,position); targetPlayerCFrame=savedPlayerCFrame; shouldFreezePlayer=variables::Freecam::freezeCharacter; }
        freecamThreadRunning=true; freecamThread=std::thread(FreecamLoop);
        wasActive=true;
    }
    bool rmb = (GetAsyncKeyState(VK_RBUTTON)&0x8000)!=0;
    int centerX = GetSystemMetrics(SM_CXSCREEN)/2, centerY = GetSystemMetrics(SM_CYSCREEN)/2;
    static bool hasCenter=false;
    if(rmb){
        POINT cur; GetCursorPos(&cur);
        if(!hasCenter){ SetCursorPos(centerX,centerY); hasCenter=true; }
        else {
            float dx = float(cur.x - centerX), dy = float(cur.y - centerY);
            if(dx||dy){ yaw -= dx * variables::Freecam::sensitivity; pitch -= dy * variables::Freecam::sensitivity; pitch = std::clamp(pitch,-1.48f,1.48f); }
            SetCursorPos(centerX,centerY);
        }
    } else hasCenter=false;
    float dt = std::clamp(ImGui::GetIO().DeltaTime,0.001f,0.05f);
    float moveSpeed = variables::Freecam::speed * dt * 60.f;
    float vertSpeed = moveSpeed;
    RBX::Vec3 forward{ std::sin(yaw), 0, std::cos(yaw) };

    float cy=std::cos(yaw), sy=std::sin(yaw), cp=std::cos(pitch), sp=std::sin(pitch);

    RBX::Vec3 fwd{ -sy*cp, sp, -cy*cp };
    RBX::Vec3 right{ cy, 0, -sy };
    if(GetAsyncKeyState('W')&0x8000){ position.X += fwd.X*moveSpeed; position.Y += fwd.Y*moveSpeed; position.Z += fwd.Z*moveSpeed; }
    if(GetAsyncKeyState('S')&0x8000){ position.X -= fwd.X*moveSpeed; position.Y -= fwd.Y*moveSpeed; position.Z -= fwd.Z*moveSpeed; }
    if(GetAsyncKeyState('A')&0x8000){ position.X -= right.X*moveSpeed; position.Y -= right.Y*moveSpeed; position.Z -= right.Z*moveSpeed; }
    if(GetAsyncKeyState('D')&0x8000){ position.X += right.X*moveSpeed; position.Y += right.Y*moveSpeed; position.Z += right.Z*moveSpeed; }
    if(GetAsyncKeyState(VK_SPACE)&0x8000) position.Y += vertSpeed;
    if(GetAsyncKeyState(VK_CONTROL)&0x8000) position.Y -= vertSpeed;
    {
        std::lock_guard<std::mutex> lk(freecamMutex);
        targetCameraCFrame=BuildCFrame(pitch,yaw,position);
        shouldFreezePlayer=variables::Freecam::freezeCharacter;
    }
}
void Start(){}
void Stop(){ Shutdown(); }
void Tick(){ Update(); }
}
