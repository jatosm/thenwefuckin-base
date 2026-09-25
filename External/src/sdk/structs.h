// discord.gg/thenwefuckin
#include <cstdint>
#include <string>
#include "math.h"

using Vector2 = rbx::vector2_t;
using Vector3 = rbx::vector3_t;
using Matrix3x3 = rbx::matrix3_t;
using UDim2 = rbx::udim2_t;
using ViewMatrix_t = rbx::matrix4_t;

namespace Structs {
inline std::string ClientVersion = "version-c5aecda2245e4fae";

struct Humanoid {
    char pad_0[32];
    int HumanoidStateID;
    char pad_1[228];
    uintptr_t SeatPart;
    char pad_2[8];
    uintptr_t MoveToPart;
    char pad_3[8];
    Vector3 CameraOffset;
    char pad_4[12];
    Vector3 MoveDirection;
    Vector3 TargetPoint;
    char pad_5[12];
    Vector3 MoveToPoint;
    char pad_6[16];
    int DisplayDistanceType;
    int FloorMaterial;
    float HealthDisplayDistance;
    int HealthDisplayType;
    float Health;
    float HipHeight;
    char pad_7[8];
    float JumpHeight;
    float JumpPower;
    float MaxHealth;
    float MaxSlopeAngle;
    float NameDisplayDistance;
    int NameOcclusion;
    char pad_8[8];
    int RigType;
    char pad_9[12];
    float Walkspeed;
    bool AutoJumpEnabled;
    bool AutoRotate;
    bool AutomaticScalingEnabled;
    bool BreakJointsOnDeath;
    bool EvaluateStateMachine;
    char pad_10[1];
    bool Jump;
    char pad_11[1];
    bool PlatformStand;
    bool Sit;
    bool RequiresNeck;
    char pad_12[1];
    bool UseJumpPower;
    char pad_13[475];
    float WalkspeedCheck;
    char pad_14[72];
    double WalkTimer;
    char pad_15[104];
    uintptr_t HumanoidRootPart;
    char pad_16[1048];
    int HumanoidState;
    char pad_17[163];
    bool IsWalking;
};
}
