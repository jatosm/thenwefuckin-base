#pragma once
#include <cstdint>
#include <string>

namespace Offsets {
inline std::string ClientVersion = "version-4310300497aa4917";

namespace Adornment {
inline constexpr uintptr_t Adornee = 0xB8;
inline constexpr uintptr_t Effect = 0xF8;
inline constexpr uintptr_t FillColor = 0xD0;
inline constexpr uintptr_t LineThickness = 0xF0;
inline constexpr uintptr_t ModelModifier = 0x100;
inline constexpr uintptr_t OutlineColor = 0xDC;
inline constexpr uintptr_t Prop = 0xB0;
inline constexpr uintptr_t ReservedId = 0xF4;
}
namespace AirProperties {
inline constexpr uintptr_t AirDensity = 0x18;
inline constexpr uintptr_t GlobalWind = 0x3C;
}
namespace Alloc {
inline constexpr uintptr_t Malloc = 0x17F37B0;
}
namespace Animator {
inline constexpr uintptr_t ActiveAnimations = 0xAB0;
}
namespace Atmosphere {
inline constexpr uintptr_t Color = 0xB8;
inline constexpr uintptr_t Decay = 0xC4;
inline constexpr uintptr_t Density = 0xD0;
inline constexpr uintptr_t Glare = 0xD4;
inline constexpr uintptr_t Haze = 0xD8;
inline constexpr uintptr_t Offset = 0xDC;
}
namespace Attachment {
inline constexpr uintptr_t Position = 0xC4;
}
namespace Attribute {
inline constexpr uintptr_t Key = 0x0;
inline constexpr uintptr_t Size = 0x58;
inline constexpr uintptr_t TypeIdRva = 0x87948E4;
inline constexpr uintptr_t TypeIdRvaNew = 0x87949D4;
inline constexpr uintptr_t Value = 0x8;
}
namespace AttributesMap {
inline constexpr uintptr_t Attributes = 0x10;
inline constexpr uintptr_t Length = 0x0;
}
namespace BasePart {
inline constexpr uintptr_t CastShadow = 0x135;
inline constexpr uintptr_t Color3 = 0x1A8;
inline constexpr uintptr_t Locked = 0x136;
inline constexpr uintptr_t Massless = 0x137;
inline constexpr uintptr_t Primitive = 0x188;
inline constexpr uintptr_t Reflectance = 0x10C;
inline constexpr uintptr_t Shape = 0x1B8;
inline constexpr uintptr_t Transparency = 0x130;
}
namespace Beam {
inline constexpr uintptr_t Attachment0 = 0x160;
inline constexpr uintptr_t Attachment1 = 0x170;
inline constexpr uintptr_t Brightness = 0x180;
inline constexpr uintptr_t CurveSize0 = 0x184;
inline constexpr uintptr_t CurveSize1 = 0x188;
inline constexpr uintptr_t LightEmission = 0x18C;
inline constexpr uintptr_t LightInfluence = 0x190;
inline constexpr uintptr_t TextureLength = 0x19C;
inline constexpr uintptr_t TextureSpeed = 0x1A4;
inline constexpr uintptr_t Width0 = 0x1A8;
inline constexpr uintptr_t Width1 = 0x1AC;
inline constexpr uintptr_t ZOffset = 0x1B0;
}
namespace BloomEffect {
inline constexpr uintptr_t Enabled = 0xB0;
inline constexpr uintptr_t Intensity = 0xB8;
inline constexpr uintptr_t Size = 0xBC;
inline constexpr uintptr_t Threshold = 0xC0;
}
namespace BlurEffect {
inline constexpr uintptr_t Enabled = 0xB0;
inline constexpr uintptr_t Size = 0xB8;
}
namespace ByteCode {
inline constexpr uintptr_t Pointer = 0x18;
inline constexpr uintptr_t Size = 0x28;
}
namespace CachedItem {
inline constexpr uintptr_t FileMeshData = 0x40;
}
namespace Camera {
inline constexpr uintptr_t CFrame = 0xD8;
inline constexpr uintptr_t CameraSubject = 0xC8;
inline constexpr uintptr_t CameraType = 0x138;
inline constexpr uintptr_t FieldOfView = 0x140;
inline constexpr uintptr_t Position = 0xFC;
inline constexpr uintptr_t Rotation = 0xD8;
inline constexpr uintptr_t Viewport = 0x28C;
inline constexpr uintptr_t ViewportInt16 = 0x28C;
inline constexpr uintptr_t ViewportSize = 0x2CC;
}
namespace CharacterMesh {
inline constexpr uintptr_t BaseTextureId = 0xC8;
inline constexpr uintptr_t BodyPart = 0x148;
inline constexpr uintptr_t MeshId = 0xF8;
inline constexpr uintptr_t OverlayTextureId = 0x128;
}
namespace Chat {
inline constexpr uintptr_t IsFocused = 0x154;
}
namespace ClassDescriptor {
inline constexpr uintptr_t ClassName = 0x8;
inline constexpr uintptr_t Creator = 0x230;
inline constexpr uintptr_t EventDescriptors = 0x88;
inline constexpr uintptr_t FunctionDescriptors = 0xD0;
inline constexpr uintptr_t PropertyDescriptors = 0x40;
}
namespace ClickDetector {
inline constexpr uintptr_t MaxActivationDistance = 0xE8;
inline constexpr uintptr_t MouseIcon = 0xC8;
}
namespace Clothing {
inline constexpr uintptr_t Color3 = 0x120;
inline constexpr uintptr_t Template = 0x100;
}
namespace ColorCorrectionEffect {
inline constexpr uintptr_t Brightness = 0xC4;
inline constexpr uintptr_t Contrast = 0xC8;
inline constexpr uintptr_t Enabled = 0xB0;
inline constexpr uintptr_t TintColor = 0xB8;
}
namespace ColorGradingEffect {
inline constexpr uintptr_t Enabled = 0xB0;
inline constexpr uintptr_t TonemapperPreset = 0xB8;
}
namespace Creator {
inline constexpr uintptr_t MapEnd = 0x84CEC38;
inline constexpr uintptr_t MapStart = 0x84CEC30;
}
namespace DataModel {
inline constexpr uintptr_t CreatorId = 0x188;
inline constexpr uintptr_t GameId = 0x190;
inline constexpr uintptr_t GameLoaded = 0x5E0;
inline constexpr uintptr_t JobId = 0x120;
inline constexpr uintptr_t PlaceId = 0x198;
inline constexpr uintptr_t ServerIP = 0x5C8;
inline constexpr uintptr_t ToRenderView1 = 0x1D0;
inline constexpr uintptr_t ToRenderView2 = 0x8;
inline constexpr uintptr_t ToRenderView3 = 0x28;
inline constexpr uintptr_t Workspace = 0x160;
}
namespace DepthOfFieldEffect {
inline constexpr uintptr_t Enabled = 0xB0;
inline constexpr uintptr_t FarIntensity = 0xB8;
inline constexpr uintptr_t FocusDistance = 0xBC;
inline constexpr uintptr_t InFocusRadius = 0xC0;
inline constexpr uintptr_t NearIntensity = 0xC4;
}
namespace Descriptor {
inline constexpr uintptr_t Name = 0x8;
}
namespace DragDetector {
inline constexpr uintptr_t MaxActivationDistance = 0xE8;
inline constexpr uintptr_t MaxDragAngle = 0x2A8;
inline constexpr uintptr_t MaxForce = 0x2AC;
inline constexpr uintptr_t MaxTorque = 0x2B0;
inline constexpr uintptr_t MinDragAngle = 0x2B4;
inline constexpr uintptr_t Responsiveness = 0x2C0;
}
namespace FakeDataModel {
inline constexpr uintptr_t Pointer = 0x8E42C98;
inline constexpr uintptr_t RealDataModel = 0x1F8;
}
namespace FastClusterEntity {
inline constexpr uintptr_t AlphaByte = 0x14;
inline constexpr uintptr_t BBoxMaxX = 0xA4;
inline constexpr uintptr_t BBoxMaxY = 0xA8;
inline constexpr uintptr_t BBoxMaxZ = 0xAC;
inline constexpr uintptr_t BBoxMinX = 0x98;
inline constexpr uintptr_t BBoxMinY = 0x9C;
inline constexpr uintptr_t BBoxMinZ = 0xA0;
inline constexpr uintptr_t ContextPtr = 0x8;
inline constexpr uintptr_t DecalMaterialPtr = 0x48;
inline constexpr uintptr_t MaterialPtr = 0x20;
inline constexpr uintptr_t PrimitiveIndexArrayPtr = 0x80;
inline constexpr uintptr_t RenderQueueId = 0x10;
inline constexpr uintptr_t TechniqueArrayPtr = 0x70;
inline constexpr uintptr_t VTableRva = 0x6C861F8;
namespace Context {
inline constexpr uintptr_t PrimitivePoolPtr = 0x1A0;
}
namespace PrimitivePool {
inline constexpr uintptr_t ArrayBase = 0x20;
}
namespace PrimitiveRecord {
inline constexpr uintptr_t Stride = 0x30;
inline constexpr uintptr_t Translation = 0x24;
}
}
namespace FileMeshData {
inline constexpr uintptr_t AabbMax = 0x18C;
inline constexpr uintptr_t AabbMin = 0x180;
inline constexpr uintptr_t Faces = 0x30;
inline constexpr uintptr_t FacesEnd = 0x38;
inline constexpr uintptr_t Vertices = 0x0;
inline constexpr uintptr_t VerticesEnd = 0x8;
}
namespace Fire {
inline constexpr uintptr_t FireProximityPrompt = 0x3140800;
}
namespace FunctionDescriptor {
inline constexpr uintptr_t Function = 0x80;
}
namespace Functions {
inline constexpr uintptr_t Clone = 0x1651AA0;
inline constexpr uintptr_t Destroy = 0x1651AC0;
inline constexpr uintptr_t FindPartOnRay = 0xED67B0;
inline constexpr uintptr_t FindPartOnRayWithIgnoreList = 0xED6830;
inline constexpr uintptr_t FindPartOnRayWithWhitelist = 0xED68C0;
inline constexpr uintptr_t FireServer = 0xCB2C30;
inline constexpr uintptr_t Print = 0x1CD9D90;
inline constexpr uintptr_t RaisePropertyChanged = 0xF84350;
inline constexpr uintptr_t Raycast = 0xECDDF0;
inline constexpr uintptr_t SetParent = 0xB259E0;
inline constexpr uintptr_t SetParentInternal = 0x1D21EE0;
inline constexpr uintptr_t SetParent_User = 0x1CD37F0;
inline constexpr uintptr_t Shapecast = 0xECF7B0;
}
namespace GuiBase2D {
inline constexpr uintptr_t AbsolutePosition = 0x10C;
inline constexpr uintptr_t AbsoluteRotation = 0xE8;
inline constexpr uintptr_t AbsoluteSize = 0x0;
}
namespace GuiObject {
inline constexpr uintptr_t Active = 0x5A8;
inline constexpr uintptr_t AnchorPoint = 0x558;
inline constexpr uintptr_t AutomaticSize = 0x560;
inline constexpr uintptr_t BackgroundColor3 = 0x540;
inline constexpr uintptr_t BackgroundTransparency = 0x54C;
inline constexpr uintptr_t BorderColor3 = 0x54C;
inline constexpr uintptr_t BorderMode = 0x568;
inline constexpr uintptr_t BorderSizePixel = 0x56C;
inline constexpr uintptr_t ClipsDescendants = 0x5A9;
inline constexpr uintptr_t GuiState = 0x578;
inline constexpr uintptr_t Interactable = 0x5AB;
inline constexpr uintptr_t LayoutOrder = 0x57C;
inline constexpr uintptr_t Position = 0x510;
inline constexpr uintptr_t Rotation = 0xE8;
inline constexpr uintptr_t Selectable = 0x5AC;
inline constexpr uintptr_t SelectionOrder = 0x598;
inline constexpr uintptr_t Size = 0x530;
inline constexpr uintptr_t SizeConstraint = 0x5A0;
inline constexpr uintptr_t Visible = 0x5AD;
inline constexpr uintptr_t ZIndex = 0x5A4;
}
namespace Highlight {
inline constexpr uintptr_t SetEnabled = 0x0;
inline constexpr uintptr_t Adornee = 0xB8;
inline constexpr uintptr_t DepthMode = 0xE0;
inline constexpr uintptr_t Effect = 0xF8;
inline constexpr uintptr_t Enabled = 0xE8;
inline constexpr uintptr_t FillColor = 0xC8;
inline constexpr uintptr_t FillColor_User = 0xD0;
inline constexpr uintptr_t FillTransparency = 0xE4;
inline constexpr uintptr_t LineThickness = 0xF0;
inline constexpr uintptr_t ModelModifier = 0x100;
inline constexpr uintptr_t OutlineColor = 0xD4;
inline constexpr uintptr_t OutlineColor_User = 0xDC;
inline constexpr uintptr_t OutlineTransparency = 0xEC;
inline constexpr uintptr_t Prop = 0xB0;
inline constexpr uintptr_t ReservedId = 0xF4;
}
namespace HopperBin {
inline constexpr uintptr_t BinType = 0x468;
}
namespace Humanoid {
inline constexpr uintptr_t AutoJumpEnabled = 0x1D4;
inline constexpr uintptr_t AutoRotate = 0x1D5;
inline constexpr uintptr_t AutomaticScalingEnabled = 0x1D6;
inline constexpr uintptr_t BreakJointsOnDeath = 0xC5;
inline constexpr uintptr_t CameraOffset = 0x128;
inline constexpr uintptr_t DisplayDistanceType = 0x180;
inline constexpr uintptr_t DisplayName = 0xB8;
inline constexpr uintptr_t EvaluateStateMachine = 0x1D8;
inline constexpr uintptr_t Health = 0x190;
inline constexpr uintptr_t HealthDisplayDistance = 0x188;
inline constexpr uintptr_t HealthDisplayType = 0x18C;
inline constexpr uintptr_t HipHeight = 0x194;
inline constexpr uintptr_t HumanoidRootPart = 0x470;
inline constexpr uintptr_t HumanoidState = 0x8B8;
inline constexpr uintptr_t HumanoidStateID = 0x20;
inline constexpr uintptr_t IsWalking = 0x95F;
inline constexpr uintptr_t Jump = 0x1DA;
inline constexpr uintptr_t JumpHeight = 0x1A0;
inline constexpr uintptr_t JumpPower = 0x1A4;
inline constexpr uintptr_t MaxHealth = 0x1A8;
inline constexpr uintptr_t MaxSlopeAngle = 0x1AC;
inline constexpr uintptr_t MoveDirection = 0x140;
inline constexpr uintptr_t MoveToPart = 0x118;
inline constexpr uintptr_t MoveToPoint = 0x164;
inline constexpr uintptr_t NameDisplayDistance = 0x1B0;
inline constexpr uintptr_t NameOcclusion = 0x1B4;
inline constexpr uintptr_t PlatformStand = 0x1DC;
inline constexpr uintptr_t PlatformStatePointer = 0xB97E6C1F;
inline constexpr uintptr_t RequiresNeck = 0x1DD;
inline constexpr uintptr_t RigType = 0x1C0;
inline constexpr uintptr_t SeatPart = 0x108;
inline constexpr uintptr_t Sit = 0x1DD;
inline constexpr uintptr_t TargetPoint = 0x14C;
inline constexpr uintptr_t UseJumpPower = 0x1E0;
inline constexpr uintptr_t WalkSpeed = 0x1D0;
inline constexpr uintptr_t WalkSpeedCheck = 0x3B4;
inline constexpr uintptr_t WalkToPoint = 0x164;
inline constexpr uintptr_t Walkspeed = 0x1D0;
inline constexpr uintptr_t WalkspeedCheck = 0x3B4;
}
namespace ICreator {
inline constexpr uintptr_t Create = 0x0;
}
namespace InputObject {
inline constexpr uintptr_t MousePosition = 0xD4;
}
namespace Instance {
inline constexpr uintptr_t ParentLocked = 0x70;
inline constexpr uintptr_t AttributeContainer = 0x40;
inline constexpr uintptr_t AttributeList = 0x10;
inline constexpr uintptr_t AttributeToNext = 0x58;
inline constexpr uintptr_t AttributeToValue = 0x18;
inline constexpr uintptr_t ChildrenEnd = 0x8;
inline constexpr uintptr_t ChildrenStart = 0x78;
inline constexpr uintptr_t ChildrenStride = 0x10;
inline constexpr uintptr_t ClassByName = 0x4849190;
inline constexpr uintptr_t ClassDescriptor = 0x18;
inline constexpr uintptr_t ClassName = 0x8;
inline constexpr uintptr_t ComponentMap = 0x38;
inline constexpr uintptr_t Creator_create = 0x0;
inline constexpr uintptr_t Creator_isCreatable = 0x10;
inline constexpr uintptr_t FromExisting = 0x80F7C0;
inline constexpr uintptr_t Name = 0x8;
inline constexpr uintptr_t NameContainer = 0x70;
inline constexpr uintptr_t New = 0x19C1D80;
inline constexpr uintptr_t Parent = 0x68;
inline constexpr uintptr_t SetParent = 0x7E9D20;
inline constexpr uintptr_t This = 0x8;
inline constexpr uintptr_t WhJobNopSlot = 0x10;
}
namespace Lighting {
inline constexpr uintptr_t Ambient = 0xD0;
inline constexpr uintptr_t Atmosphere = 0x1D8;
inline constexpr uintptr_t Brightness = 0x118;
inline constexpr uintptr_t ClockTime = 0xC8;
inline constexpr uintptr_t ColorShift_Bottom = 0xE8;
inline constexpr uintptr_t ColorShift_Top = 0xDC;
inline constexpr uintptr_t EnvironmentDiffuseScale = 0x11C;
inline constexpr uintptr_t EnvironmentSpecularScale = 0x120;
inline constexpr uintptr_t ExposureCompensation = 0x124;
inline constexpr uintptr_t FogColor = 0xF4;
inline constexpr uintptr_t FogEnd = 0x12C;
inline constexpr uintptr_t FogStart = 0x130;
inline constexpr uintptr_t GeographicLatitude = 0x134;
inline constexpr uintptr_t GlobalShadows = 0x144;
inline constexpr uintptr_t GradientBottom = 0x190;
inline constexpr uintptr_t GradientTop = 0x150;
inline constexpr uintptr_t LightColor = 0x15C;
inline constexpr uintptr_t LightDirection = 0x168;
inline constexpr uintptr_t MoonPosition = 0x184;
inline constexpr uintptr_t OutdoorAmbient = 0x100;
inline constexpr uintptr_t ShadowSoftness = 0x13C;
inline constexpr uintptr_t Sky = 0x1C8;
inline constexpr uintptr_t Source = 0x174;
inline constexpr uintptr_t SunPosition = 0x178;
}
namespace LightingParameters {
inline constexpr uintptr_t GeographicLatitude = 0x134;
inline constexpr uintptr_t LightColor = 0x15C;
inline constexpr uintptr_t LightDirection = 0x168;
inline constexpr uintptr_t SkyAmbient = 0x150;
inline constexpr uintptr_t SkyAmbient2 = 0x138;
inline constexpr uintptr_t Source = 0x174;
inline constexpr uintptr_t TrueMoonPosition = 0x184;
inline constexpr uintptr_t TrueSunPosition = 0x178;
}
namespace LocalScript {
inline constexpr uintptr_t ByteCode = 0x190;
inline constexpr uintptr_t Bytecode = 0x190;
inline constexpr uintptr_t Hash = 0x1A0;
}
namespace LruHolder {
inline constexpr uintptr_t MemEnforcedLRUCache = 0x20;
}
namespace LruNode {
inline constexpr uintptr_t CachedItem = 0x38;
inline constexpr uintptr_t MeshId = 0x10;
inline constexpr uintptr_t Next = 0x0;
}
namespace LuaState {
inline constexpr uintptr_t Base = 0x28;
inline constexpr uintptr_t Global = 0x20;
inline constexpr uintptr_t Top = 0x8;
inline constexpr uintptr_t TypeTag = 0x0;
}
namespace Luau {
inline constexpr uintptr_t lua_getglobal = 0x0;
inline constexpr uintptr_t require_impl = 0x0;
}
namespace LuauGlobal {
inline constexpr uintptr_t GCthreshold = 0x48;
inline constexpr uintptr_t currentwhite = 0x58;
inline constexpr uintptr_t dummynode = 0x6D60DB8;
inline constexpr uintptr_t gcopages = 0x2F0;
inline constexpr uintptr_t gcopages_end = 0x2F8;
inline constexpr uintptr_t gcopages_large = 0x2F0;
inline constexpr uintptr_t gcopages_sizeclass = 0x0;
inline constexpr uintptr_t gcpause = 0x38;
inline constexpr uintptr_t gcstate = 0x59;
inline constexpr uintptr_t gcstepmul = 0x3C;
inline constexpr uintptr_t gcstepsize = 0x40;
inline constexpr uintptr_t gray = 0x10;
inline constexpr uintptr_t grayagain = 0x18;
inline constexpr uintptr_t page_next_all = 0x8;
inline constexpr uintptr_t page_next_free = 0x18;
inline constexpr uintptr_t strt_hash = 0x0;
inline constexpr uintptr_t strt_size = 0xC;
inline constexpr uintptr_t totalbytes = 0x50;
inline constexpr uintptr_t weak = 0x20;
}
namespace LuauObject {
inline constexpr uintptr_t marked = 0x2;
inline constexpr uintptr_t page_block = 0x24;
inline constexpr uintptr_t page_data = 0x40;
inline constexpr uintptr_t page_next = 0x8;
inline constexpr uintptr_t page_size = 0x20;
inline constexpr uintptr_t table_array = 0x28;
inline constexpr uintptr_t table_gclist = 0x20;
inline constexpr uintptr_t table_lsz = 0x7;
inline constexpr uintptr_t table_node = 0x18;
inline constexpr uintptr_t table_sizearray = 0x8;
inline constexpr uintptr_t tt = 0xC;
}
namespace MaterialColors {
inline constexpr uintptr_t Asphalt = 0x30;
inline constexpr uintptr_t Basalt = 0x27;
inline constexpr uintptr_t Brick = 0xF;
inline constexpr uintptr_t Cobblestone = 0x33;
inline constexpr uintptr_t Concrete = 0xC;
inline constexpr uintptr_t CrackedLava = 0x2D;
inline constexpr uintptr_t Glacier = 0x1B;
inline constexpr uintptr_t Grass = 0x6;
inline constexpr uintptr_t Ground = 0x2A;
inline constexpr uintptr_t Ice = 0x36;
inline constexpr uintptr_t LeafyGrass = 0x39;
inline constexpr uintptr_t Limestone = 0x3F;
inline constexpr uintptr_t Mud = 0x24;
inline constexpr uintptr_t Pavement = 0x42;
inline constexpr uintptr_t Rock = 0x18;
inline constexpr uintptr_t Salt = 0x3C;
inline constexpr uintptr_t Sand = 0x12;
inline constexpr uintptr_t Sandstone = 0x21;
inline constexpr uintptr_t Slate = 0x9;
inline constexpr uintptr_t Snow = 0x1E;
inline constexpr uintptr_t WoodPlanks = 0x15;
}
namespace MaterialLayer {
inline constexpr uintptr_t ColorData = 0x24;
inline constexpr uintptr_t FillModeByte = 0x11;
inline constexpr uintptr_t Flags2 = 0x20;
inline constexpr uintptr_t MatFlags = 0x18;
inline constexpr uintptr_t Param = 0x1C;
inline constexpr uintptr_t Stride = 0x88;
}
namespace MemEnforcedLRUCache {
inline constexpr uintptr_t Head = 0x8;
}
namespace MeshContentProvider {
inline constexpr uintptr_t AssetID = 0x10;
inline constexpr uintptr_t Cache = 0xD8;
inline constexpr uintptr_t LRUCache = 0x20;
inline constexpr uintptr_t LruHolder = 0xD8;
inline constexpr uintptr_t MeshData = 0x40;
inline constexpr uintptr_t ToMeshData = 0x40;
}
namespace MeshData {
inline constexpr uintptr_t FaceEnd = 0x38;
inline constexpr uintptr_t FaceStart = 0x30;
inline constexpr uintptr_t VertexEnd = 0x8;
inline constexpr uintptr_t VertexStart = 0x0;
}
namespace MeshPart {
inline constexpr uintptr_t MeshId = 0x310;
inline constexpr uintptr_t Texture = 0x340;
inline constexpr uintptr_t TextureId = 0x340;
}
namespace Misc {
inline constexpr uintptr_t AnimationId = 0xC0;
inline constexpr uintptr_t StringLength = 0x10;
inline constexpr uintptr_t Value = 0xB8;
}
namespace Model {
inline constexpr uintptr_t PrimaryPart = 0x258;
inline constexpr uintptr_t Scale = 0x144;
}
namespace ModuleScript {
inline constexpr uintptr_t ByteCode = 0x138;
inline constexpr uintptr_t Bytecode = 0x138;
inline constexpr uintptr_t Hash = 0x148;
inline constexpr uintptr_t IsCoreScript = 0x0;
inline constexpr uintptr_t IsRobloxScript = 0x168;
}
namespace MouseService {
inline constexpr uintptr_t InputObject = 0xF0;
inline constexpr uintptr_t InputObject2 = 0x100;
inline constexpr uintptr_t MousePosition = 0xD4;
inline constexpr uintptr_t SensitivityPointer = 0x0;
}
namespace ParticleEmitter {
inline constexpr uintptr_t Brightness = 0x21C;
inline constexpr uintptr_t Drag = 0x220;
inline constexpr uintptr_t LightEmission = 0x238;
inline constexpr uintptr_t LightInfluence = 0x23C;
inline constexpr uintptr_t Rate = 0x248;
inline constexpr uintptr_t Texture = 0x1C0;
inline constexpr uintptr_t TimeScale = 0x25C;
inline constexpr uintptr_t VelocityInheritance = 0x260;
inline constexpr uintptr_t ZOffset = 0x264;
}
namespace Player {
inline constexpr uintptr_t AccountAge = 0x35C;
inline constexpr uintptr_t Character = 0x298;
inline constexpr uintptr_t DisplayName = 0x138;
inline constexpr uintptr_t HealthDisplayDistance = 0x394;
inline constexpr uintptr_t LocalPlayer = 0x130;
inline constexpr uintptr_t LocaleId = 0x118;
inline constexpr uintptr_t MaxZoomDistance = 0x368;
inline constexpr uintptr_t MinZoomDistance = 0x36C;
inline constexpr uintptr_t ModelInstance = 0x298;
inline constexpr uintptr_t NameDisplayDistance = 0x3A4;
inline constexpr uintptr_t Team = 0x2D8;
inline constexpr uintptr_t TeamColor = 0x3B0;
inline constexpr uintptr_t UserId = 0xD0;
}
namespace PlayerConfigurer {
inline constexpr uintptr_t Pointer = 0x0;
}
namespace Players {
inline constexpr uintptr_t LocalPlayer = 0x130;
}
namespace Primitive {
inline constexpr uintptr_t AssemblyAngularVelocity = 0xEC;
inline constexpr uintptr_t AssemblyLinearVelocity = 0xE0;
inline constexpr uintptr_t CFrame = 0xB0;
inline constexpr uintptr_t Flags = 0x1BE;
inline constexpr uintptr_t Material = 0x0;
inline constexpr uintptr_t Orientation = 0xB0;
inline constexpr uintptr_t Part = 0x218;
inline constexpr uintptr_t Position = 0xD4;
inline constexpr uintptr_t PrimitiveFlags = 0x1BE;
inline constexpr uintptr_t Rotation = 0xB0;
inline constexpr uintptr_t Size = 0x1C4;
inline constexpr uintptr_t Validate = 0x6;
}
namespace PrimitiveFlags {
inline constexpr uintptr_t Anchored = 0x2;
inline constexpr uintptr_t CanCollide = 0x8;
inline constexpr uintptr_t CanQuery = 0x20;
inline constexpr uintptr_t CanTouch = 0x10;
}
namespace PropertyDescriptor {
inline constexpr uintptr_t GetSetImpl = 0x90;
inline constexpr uintptr_t TType = 0x68;
}
namespace ProximityPrompt {
inline constexpr uintptr_t ActionText = 0xB0;
inline constexpr uintptr_t Enabled = 0x136;
inline constexpr uintptr_t HoldDuration = 0x120;
inline constexpr uintptr_t KeyboardKeyCode = 0x124;
inline constexpr uintptr_t MaxActivationDistance = 0x128;
inline constexpr uintptr_t ObjectText = 0xD0;
inline constexpr uintptr_t RequiresLineOfSight = 0x137;
}
namespace Reflection {
inline constexpr uintptr_t ClassDescCreatable = 0x10;
inline constexpr uintptr_t ClassDescFlags = 0x1BC;
inline constexpr uintptr_t CreatorTable = 0x89C8388;
inline constexpr uintptr_t EntryValue = 0x8;
inline constexpr uintptr_t NameRegistry = 0x88BF5B8;
inline constexpr uintptr_t NameTable = 0x50;
inline constexpr uintptr_t TableEmpty = 0x20;
inline constexpr uintptr_t TableEnd = 0x8;
inline constexpr uintptr_t TableStart = 0x0;
inline constexpr uintptr_t TableStride = 0x10;
}
namespace ReflectionType {
inline constexpr uintptr_t AdReward = 0x60;
inline constexpr uintptr_t AnimTrackMetadata = 0x66;
inline constexpr uintptr_t AnimTrackPlayState = 0x65;
inline constexpr uintptr_t AnimTrackWeight = 0x67;
inline constexpr uintptr_t AnimationContext = 0x54;
inline constexpr uintptr_t AnimationMask = 0x4D;
inline constexpr uintptr_t AnimationMaskModifier = 0x5B;
inline constexpr uintptr_t AnimationPose = 0x4E;
inline constexpr uintptr_t Array = 0x24;
inline constexpr uintptr_t ArticulatedJoint = 0x53;
inline constexpr uintptr_t AssetContentMap = 0x61;
inline constexpr uintptr_t Axes = 0x16;
inline constexpr uintptr_t BinaryString = 0x1E;
inline constexpr uintptr_t Bool = 0x1;
inline constexpr uintptr_t BrickColor = 0x1C;
inline constexpr uintptr_t Buffer = 0x56;
inline constexpr uintptr_t CSGPropertyData = 0x48;
inline constexpr uintptr_t CatalogSearchParams = 0x46;
inline constexpr uintptr_t CellId = 0x19;
inline constexpr uintptr_t ClipEvaluator = 0x4F;
inline constexpr uintptr_t CollectionHandle = 0x20;
inline constexpr uintptr_t Color3 = 0x11;
inline constexpr uintptr_t Color3uint8 = 0x12;
inline constexpr uintptr_t ColorSequence = 0x2A;
inline constexpr uintptr_t ColorSequenceKeypoint = 0x2B;
inline constexpr uintptr_t Connection = 0x30;
inline constexpr uintptr_t Content = 0x5C;
inline constexpr uintptr_t ContentId = 0x31;
inline constexpr uintptr_t CoordinateFrame = 0x10;
inline constexpr uintptr_t DateTime = 0x40;
inline constexpr uintptr_t DebugTable = 0x45;
inline constexpr uintptr_t DescribedBase = 0x32;
inline constexpr uintptr_t Dictionary = 0x25;
inline constexpr uintptr_t DockWidgetPluginGuiInfo = 0x38;
inline constexpr uintptr_t Double = 0x5;
inline constexpr uintptr_t Enum = 0x21;
inline constexpr uintptr_t EventInstance = 0x36;
inline constexpr uintptr_t Faces = 0x15;
inline constexpr uintptr_t FacsReplicationData = 0x5A;
inline constexpr uintptr_t Float = 0x4;
inline constexpr uintptr_t FloatCurveKey = 0x3C;
inline constexpr uintptr_t Font = 0x4A;
inline constexpr uintptr_t Function = 0x29;
inline constexpr uintptr_t GenericFunction = 0x28;
inline constexpr uintptr_t GuidData = 0x1A;
inline constexpr uintptr_t Instance = 0x8;
inline constexpr uintptr_t InstanceRef = 0x51;
inline constexpr uintptr_t Instances = 0x9;
inline constexpr uintptr_t Int = 0x2;
inline constexpr uintptr_t Int64 = 0x3;
inline constexpr uintptr_t Integer = 0x57;
inline constexpr uintptr_t LazyTable = 0x44;
inline constexpr uintptr_t Map = 0x26;
inline constexpr uintptr_t NetAssetHandle = 0x5D;
inline constexpr uintptr_t NetAssetRef = 0x5E;
inline constexpr uintptr_t Null = 0x0;
inline constexpr uintptr_t NumberRange = 0x2C;
inline constexpr uintptr_t NumberSequence = 0x2D;
inline constexpr uintptr_t NumberSequenceKeypoint = 0x2E;
inline constexpr uintptr_t Object = 0x5F;
inline constexpr uintptr_t OpenCloudModel = 0x50;
inline constexpr uintptr_t OptionalCoordinateFrame = 0x47;
inline constexpr uintptr_t OverlapParams = 0x43;
inline constexpr uintptr_t Path2DControlPoint = 0x58;
inline constexpr uintptr_t PathWaypoint = 0x3B;
inline constexpr uintptr_t PhysicalProperties = 0x1B;
inline constexpr uintptr_t PluginDrag = 0x39;
inline constexpr uintptr_t Property = 0x22;
inline constexpr uintptr_t ProtectedString = 0x7;
inline constexpr uintptr_t Random = 0x3A;
inline constexpr uintptr_t Ray = 0xA;
inline constexpr uintptr_t RaycastParams = 0x41;
inline constexpr uintptr_t RaycastResult = 0x42;
inline constexpr uintptr_t Rect2D = 0xF;
inline constexpr uintptr_t RefType = 0x33;
inline constexpr uintptr_t Region3 = 0x17;
inline constexpr uintptr_t Region3int16 = 0x18;
inline constexpr uintptr_t ReplicationPV = 0x59;
inline constexpr uintptr_t RotationCurveKey = 0x3D;
inline constexpr uintptr_t ScopedInstanceIdentity = 0x68;
inline constexpr uintptr_t Secret = 0x55;
inline constexpr uintptr_t SecurityCapabilities = 0x52;
inline constexpr uintptr_t SharedString = 0x3F;
inline constexpr uintptr_t SharedTable = 0x4B;
inline constexpr uintptr_t SharedTableIterator = 0x4C;
inline constexpr uintptr_t SlimReplicationData = 0x62;
inline constexpr uintptr_t String = 0x6;
inline constexpr uintptr_t Surface = 0x1F;
inline constexpr uintptr_t SystemAddress = 0x1D;
inline constexpr uintptr_t Tuple = 0x23;
inline constexpr uintptr_t TweenInfo = 0x37;
inline constexpr uintptr_t UDim = 0x13;
inline constexpr uintptr_t UDim2 = 0x14;
inline constexpr uintptr_t UniqueId = 0x49;
inline constexpr uintptr_t User = 0x63;
inline constexpr uintptr_t ValueCurveKey = 0x3E;
inline constexpr uintptr_t Variant = 0x27;
inline constexpr uintptr_t Vector2 = 0xB;
inline constexpr uintptr_t Vector2int16 = 0xD;
inline constexpr uintptr_t Vector3 = 0xC;
inline constexpr uintptr_t Vector3int16 = 0xE;
inline constexpr uintptr_t WebViewParams = 0x64;
}
namespace RenderJob {
inline constexpr uintptr_t FakeDataModel = 0x38;
inline constexpr uintptr_t FrameDt = 0xC0;
inline constexpr uintptr_t FrameDtAlt = 0xB8;
inline constexpr uintptr_t RealDataModel = 0x1F0;
inline constexpr uintptr_t RenderView = 0x1D8;
}
namespace RenderQueue {
inline constexpr uintptr_t AlwaysOnTop = 0xD;
inline constexpr uintptr_t AlwaysOnTopAdorns = 0xE;
inline constexpr uintptr_t Decals = 0x2;
inline constexpr uintptr_t Glass = 0x8;
inline constexpr uintptr_t GlassTint = 0x7;
inline constexpr uintptr_t OnTopReadOnlyDepth = 0xC;
inline constexpr uintptr_t OnTopWithDepth = 0xB;
inline constexpr uintptr_t Opaque = 0x0;
inline constexpr uintptr_t OpaqueAdorns = 0x4;
inline constexpr uintptr_t OpaqueCasters = 0x3;
inline constexpr uintptr_t OpaqueWithAlpha = 0x5;
inline constexpr uintptr_t Screen = 0xF;
inline constexpr uintptr_t ScreenOnTopOfBlur = 0x10;
inline constexpr uintptr_t Terrain = 0x1;
inline constexpr uintptr_t Transparent = 0x9;
inline constexpr uintptr_t TransparentCasters = 0xA;
inline constexpr uintptr_t Water = 0x6;
}
namespace RenderView {
inline constexpr uintptr_t DeviceD3D11 = 0x0;
inline constexpr uintptr_t LightingValid = 0x278;
inline constexpr uintptr_t SkyValid = 0x28D;
inline constexpr uintptr_t SkyboxValid = 0x28D;
inline constexpr uintptr_t VisualEngine = 0x0;
}
namespace RobloxString {
inline constexpr uintptr_t Size = 0x10;
inline constexpr uintptr_t SsoCapacity = 0xF;
}
namespace RunService {
inline constexpr uintptr_t HeartbeatFPS = 0xC0;
}
namespace Script {
inline constexpr uintptr_t ByteCode = 0x190;
}
namespace ScriptContext {
inline constexpr uintptr_t LuaState = 0x28;
inline constexpr uintptr_t LuaState2 = 0x28;
inline constexpr uintptr_t LuaStateAlt = 0xE8;
inline constexpr uintptr_t RequireBypass = 0xA58;
inline constexpr uintptr_t VmEncryptedLuaState = 0xD0;
inline constexpr uintptr_t VmWrapper = 0x220;
inline constexpr uintptr_t VmWrapper2 = 0x528;
inline constexpr uintptr_t VmWrapperBig = 0x440;
}
namespace Seat {
inline constexpr uintptr_t Occupant = 0x218;
}
namespace Sky {
inline constexpr uintptr_t DrawCubeCallB = 0x36EB58B;
inline constexpr uintptr_t DrawAdv = 0x36F3E6B;
inline constexpr uintptr_t DrawCube = 0x36EF530;
inline constexpr uintptr_t DrawCubeCallA = 0x36EB87C;
inline constexpr uintptr_t MoonAngularSize = 0x244;
inline constexpr uintptr_t MoonTextureId = 0xC8;
inline constexpr uintptr_t SkyboxBk = 0xF8;
inline constexpr uintptr_t SkyboxDn = 0x128;
inline constexpr uintptr_t SkyboxFt = 0x158;
inline constexpr uintptr_t SkyboxLf = 0x188;
inline constexpr uintptr_t SkyboxOrientation = 0x238;
inline constexpr uintptr_t SkyboxRt = 0x1B8;
inline constexpr uintptr_t SkyboxUp = 0x1E8;
inline constexpr uintptr_t StarCount = 0x248;
inline constexpr uintptr_t SunAngularSize = 0x23C;
inline constexpr uintptr_t SunTextureId = 0x218;
}
namespace Sound {
inline constexpr uintptr_t IsPlaying = 0x140;
inline constexpr uintptr_t Looped = 0x13D;
inline constexpr uintptr_t PlaybackSpeed = 0x11C;
inline constexpr uintptr_t RollOffMaxDistance = 0x120;
inline constexpr uintptr_t RollOffMinDistance = 0x124;
inline constexpr uintptr_t SoundId = 0xC8;
inline constexpr uintptr_t Volume = 0x130;
}
namespace SpawnLocation {
inline constexpr uintptr_t AllowTeamChangeOnTouch = 0x3D;
inline constexpr uintptr_t ForcefieldDuration = 0x1E8;
}
namespace SpecialMesh {
inline constexpr uintptr_t MeshId = 0xF8;
inline constexpr uintptr_t Offset = 0xB8;
inline constexpr uintptr_t Scale = 0xC4;
inline constexpr uintptr_t TextureId = 0x128;
}
namespace SunRaysEffect {
inline constexpr uintptr_t Enabled = 0xB0;
inline constexpr uintptr_t Intensity = 0xB8;
inline constexpr uintptr_t Spread = 0xBC;
}
namespace SurfaceAppearance {
inline constexpr uintptr_t ColorMap = 0xC8;
inline constexpr uintptr_t EmissiveStrength = 0x1F4;
inline constexpr uintptr_t MetalnessMap = 0x128;
inline constexpr uintptr_t NormalMap = 0x158;
inline constexpr uintptr_t RoughnessMap = 0x188;
}
namespace TaskScheduler {
inline constexpr uintptr_t JobEnd = 0xD0;
inline constexpr uintptr_t JobName = 0x18;
inline constexpr uintptr_t JobStart = 0xC8;
inline constexpr uintptr_t MaxFPS = 0xB0;
inline constexpr uintptr_t MaxFps = 0xB0;
inline constexpr uintptr_t Pointer = 0x8BDD8E8;
}
namespace Team {
inline constexpr uintptr_t BrickColor = 0xB8;
inline constexpr uintptr_t TeamColor = 0xB8;
}
namespace TechniqueArray {
inline constexpr uintptr_t BeginOffset = 0x0;
inline constexpr uintptr_t EndOffset = 0x8;
}
namespace Terrain {
inline constexpr uintptr_t GrassLength = 0x1F0;
inline constexpr uintptr_t MaterialColors = 0x4B8;
inline constexpr uintptr_t WaterColor = 0x1E0;
inline constexpr uintptr_t WaterReflectance = 0x1F8;
inline constexpr uintptr_t WaterTransparency = 0x1FC;
inline constexpr uintptr_t WaterWaveSize = 0x200;
inline constexpr uintptr_t WaterWaveSpeed = 0x204;
}
namespace TextButton {
inline constexpr uintptr_t AutoButtonColor = 0x9DC;
inline constexpr uintptr_t ContentText = 0xE18;
inline constexpr uintptr_t Font = 0x6;
inline constexpr uintptr_t LineHeight = 0xF30;
inline constexpr uintptr_t LocalizedText = 0xE18;
inline constexpr uintptr_t MaxVisibleGraphemes = 0x114C;
inline constexpr uintptr_t Modal = 0x9DD;
inline constexpr uintptr_t RichText = 0x102E;
inline constexpr uintptr_t Selected = 0x9DE;
inline constexpr uintptr_t Text = 0xE18;
inline constexpr uintptr_t TextColor3 = 0x1130;
inline constexpr uintptr_t TextDirection = 0xFD0;
inline constexpr uintptr_t TextScaled = 0xE01;
inline constexpr uintptr_t TextSize = 0x1154;
inline constexpr uintptr_t TextStrokeColor3 = 0x113C;
inline constexpr uintptr_t TextStrokeTransparency = 0x1158;
inline constexpr uintptr_t TextTransparency = 0x115C;
inline constexpr uintptr_t TextTruncate = 0x1160;
inline constexpr uintptr_t TextWrapped = 0x1028;
inline constexpr uintptr_t TextXAlignment = 0x1164;
inline constexpr uintptr_t TextYAlignment = 0xF78;
}
namespace TextLabel {
inline constexpr uintptr_t ContentText = 0xB98;
inline constexpr uintptr_t Font = 0x6;
inline constexpr uintptr_t LineHeight = 0xCB0;
inline constexpr uintptr_t LocalizedText = 0xB98;
inline constexpr uintptr_t MaxVisibleGraphemes = 0xECC;
inline constexpr uintptr_t RichText = 0xDAE;
inline constexpr uintptr_t Text = 0xB98;
inline constexpr uintptr_t TextColor3 = 0xEB0;
inline constexpr uintptr_t TextDirection = 0xD50;
inline constexpr uintptr_t TextScaled = 0xDA6;
inline constexpr uintptr_t TextSize = 0xED4;
inline constexpr uintptr_t TextStrokeColor3 = 0xEBC;
inline constexpr uintptr_t TextStrokeTransparency = 0xED8;
inline constexpr uintptr_t TextTransparency = 0xEDC;
inline constexpr uintptr_t TextTruncate = 0xEE0;
inline constexpr uintptr_t TextWrapped = 0xDA8;
inline constexpr uintptr_t TextXAlignment = 0xEE4;
inline constexpr uintptr_t TextYAlignment = 0xCF8;
}
namespace Textures {
inline constexpr uintptr_t Decal_Texture = 0x1E0;
inline constexpr uintptr_t Texture_Texture = 0x1E0;
}
namespace Tool {
inline constexpr uintptr_t CanBeDropped = 0x4B8;
inline constexpr uintptr_t Enabled = 0x475;
inline constexpr uintptr_t Grip = 0x4AC;
inline constexpr uintptr_t GripForward = 0x4A0;
inline constexpr uintptr_t GripPos = 0x4AC;
inline constexpr uintptr_t GripRight = 0x488;
inline constexpr uintptr_t GripUp = 0x494;
inline constexpr uintptr_t ManualActivationOnly = 0x295;
inline constexpr uintptr_t RequiresHandle = 0x4BB;
inline constexpr uintptr_t Tooltip = 0x468;
}
namespace Types {
inline constexpr uintptr_t AllTypes = 0x89C8C98;
}
namespace UserInputService {
inline constexpr uintptr_t WindowInputState = 0x2C0;
}
namespace Value {
inline constexpr uintptr_t Value = 0xB8;
}
namespace VehicleSeat {
inline constexpr uintptr_t MaxSpeed = 0x228;
inline constexpr uintptr_t Occupant = 0x208;
inline constexpr uintptr_t SteerFloat = 0x22C;
inline constexpr uintptr_t ThrottleFloat = 0x230;
inline constexpr uintptr_t Torque = 0x234;
inline constexpr uintptr_t TurnSpeed = 0x238;
}
namespace VisualEngine {
inline constexpr uintptr_t Dimensions = 0xB10;
inline constexpr uintptr_t FakeDataModel = 0xAF0;
inline constexpr uintptr_t Pointer = 0x846F768;
inline constexpr uintptr_t RenderView = 0xC30;
inline constexpr uintptr_t ViewMatrix = 0x1B0;
}
namespace Weld {
inline constexpr uintptr_t Part0 = 0x118;
inline constexpr uintptr_t Part1 = 0x128;
}
namespace WeldConstraint {
inline constexpr uintptr_t Part0 = 0xB8;
inline constexpr uintptr_t Part1 = 0xC8;
}
namespace WindowInputState {
inline constexpr uintptr_t CapsLock = 0x40;
inline constexpr uintptr_t CurrentTextBox = 0x48;
}
namespace Workspace {
inline constexpr uintptr_t CurrentCamera = 0x4B8;
inline constexpr uintptr_t ReadOnlyGravity = 0xA00;
inline constexpr uintptr_t World = 0x410;
}
namespace World {
inline constexpr uintptr_t AirProperties = 0x240;
inline constexpr uintptr_t FallenPartsDestroyHeight = 0x220;
inline constexpr uintptr_t Gravity = 0x22C;
inline constexpr uintptr_t Primitives = 0x2B0;
inline constexpr uintptr_t WorldSteps = 0x728;
inline constexpr uintptr_t worldStepsPerSec = 0x728;
}
namespace WorldRoot {
inline constexpr uintptr_t RaycastBoundDesc = 0x8217310;
inline constexpr uintptr_t RaycastBoundFn = 0x80;
}
namespace GeometryD3D11 { inline constexpr uintptr_t VTableRva = 0x6C80000; }
namespace FastClusterBinding { inline constexpr uintptr_t VTableRva = 0x6C81000; inline constexpr uintptr_t Owner = 0x18; }
namespace FastCluster { inline constexpr uintptr_t VTableRva = 0x6C82000; inline constexpr uintptr_t VTableRvaSub = 0x6C82008; inline constexpr uintptr_t BindingSubobject = 0x10; inline constexpr uintptr_t EntityBegin = 0x20; inline constexpr uintptr_t EntityEnd = 0x28; }
namespace DeviceD3D11Gfx { inline constexpr uintptr_t VTableRva = 0x0; inline constexpr uintptr_t DevicePtr = 0x10; inline constexpr uintptr_t ContextPtr = 0x18; inline constexpr uintptr_t SwapChainPtr = 0x20; inline constexpr uintptr_t ContextObj = 0x28; }
namespace FFlag { inline constexpr uintptr_t RenderFastClusterOcclusionCulling = 0x0; }
}
