#pragma once
#include "../../src/sdk/offsets.h"
#include "../../src/sdk/math.h"
#include "../../src/memory/memory.h"
#include <string>
#include <vector>
#include <cmath>

namespace RBX {
inline void WriteString(std::uint64_t address, const std::string& value) {
    if (!address || !memory->IsConnected())
        return;
    const auto capacity = memory->read<std::uint64_t>(address + 0x18);
    const auto len = static_cast<std::uint64_t>(value.size());
    if (len > capacity) {
        void* mem = VirtualAllocEx(memory->get_process_handle(), nullptr, len + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!mem)
            return;
        const auto dst = reinterpret_cast<std::uint64_t>(mem);
        memory->write_raw(dst, value.data(), static_cast<std::size_t>(len));
        memory->write<char>(dst + len, '\0');
        memory->write<std::uint64_t>(address, dst);
        memory->write<std::uint64_t>(address + Offsets::Misc::StringLength, len);
        memory->write<std::uint64_t>(address + 0x18, len);
        return;
    }
    const std::uint64_t dst = capacity >= 16u ? memory->read<std::uint64_t>(address) : address;
    if (!dst)
        return;
    if (!value.empty())
        memory->write_raw(dst, value.data(), static_cast<std::size_t>(len));
    memory->write<char>(dst + len, '\0');
    memory->write<std::uint64_t>(address + Offsets::Misc::StringLength, len);
}

struct Vec2 {
    float X{0}, Y{0};
};

struct Vec3 {
    float X{0}, Y{0}, Z{0};
};

struct Vec4 {
    float X{0}, Y{0}, Z{0}, W{0};
};

struct Mat4 {
    float data[16]{};
};

struct CFrame {
    float data[12]{};
    Vec3 GetRightVector() const { return {data[0], data[3], data[6]}; }
    Vec3 GetUpVector() const { return {data[1], data[4], data[7]}; }
    Vec3 GetLookVector() const { return {data[2], data[5], data[8]}; }
    Vec3 GetPosition() const { return {data[9], data[10], data[11]}; }
};

class RbxInstance {
public:
    std::uintptr_t Addr = 0;

    RbxInstance() = default;
    explicit RbxInstance(std::uintptr_t addr) : Addr(addr) {}
    explicit operator bool() const { return Addr != 0; }

    std::string GetName() const {
        if (!Addr)
            return {};
        const auto container = memory->read<std::uintptr_t>(Addr + Offsets::Instance::NameContainer);
        if (!container)
            return {};
        return memory->read_string(container + Offsets::Instance::Name);
    }

    std::string GetClass() const {
        if (!Addr)
            return {};
        const auto desc = memory->read<std::uintptr_t>(Addr + Offsets::Instance::ClassDescriptor);
        if (!desc)
            return {};
        const auto ptr = memory->read<std::uintptr_t>(desc + Offsets::Instance::ClassName);
        return ptr ? memory->read_string(ptr) : std::string{};
    }

    RbxInstance GetParent() const {
        return Addr ? RbxInstance(memory->read<std::uintptr_t>(Addr + Offsets::Instance::Parent)) : RbxInstance{};
    }

    std::vector<RbxInstance> GetChildList() const {
        std::vector<RbxInstance> out;
        if (!Addr)
            return out;
        const auto list = memory->read<std::uintptr_t>(Addr + Offsets::Instance::ChildrenStart);
        if (!list)
            return out;
        const auto end = memory->read<std::uintptr_t>(list + Offsets::Instance::ChildrenEnd);
        auto cur = memory->read<std::uintptr_t>(list);
        if (!end || !cur || end < cur)
            return out;
        constexpr std::size_t kMax = 4096;
        out.reserve(64);
        for (std::uintptr_t p = cur, n = 0; p + sizeof(std::uintptr_t) <= end && n < kMax; p += 0x10, ++n) {
            const auto child = memory->read<std::uintptr_t>(p);
            if (child)
                out.emplace_back(child);
        }
        return out;
    }

    RbxInstance FindChild(const std::string& name) const {
        for (auto& c : GetChildList()) {
            if (c.GetName() == name)
                return c;
        }
        return {};
    }

    RbxInstance FindFirstChild(const std::string& name) const { return FindChild(name); }

    RbxInstance FindChildByClass(const std::string& cls) const {
        for (auto& c : GetChildList()) {
            if (c.GetClass() == cls)
                return c;
        }
        return {};
    }

    std::uintptr_t GetPrimitivePtr() const {
        return Addr ? memory->read<std::uintptr_t>(Addr + Offsets::BasePart::Primitive) : 0;
    }

    Vec3 GetPos() const {
        const auto prim = GetPrimitivePtr();
        return prim ? memory->read<Vec3>(prim + Offsets::Primitive::Position) : Vec3{};
    }

    CFrame GetCFrame() const {
        const auto prim = GetPrimitivePtr();
        return prim ? memory->read<CFrame>(prim + Offsets::Primitive::Rotation) : CFrame{};
    }

    RbxInstance GetModelRef() const {
        return RbxInstance(memory->read<std::uintptr_t>(Addr + Offsets::Player::ModelInstance));
    }

    RbxInstance GetLocalPlayer() const {
        return RbxInstance(memory->read<std::uintptr_t>(Addr + Offsets::Player::LocalPlayer));
    }

    RbxInstance GetModelInstance() const { return GetModelRef(); }
    std::uintptr_t GetPart() const { return GetPrimitivePtr(); }
    bool GetAnchored() const {
        auto prim = GetPrimitivePtr();
        if (!prim) return false;
        uint8_t f = memory->read<uint8_t>(prim + Offsets::Primitive::Flags);
        return (f & (uint8_t)Offsets::PrimitiveFlags::Anchored) != 0;
    }
    void SetAnchored(bool v) const {
        auto prim = GetPrimitivePtr();
        if (!prim) return;
        uint8_t f = memory->read<uint8_t>(prim + Offsets::Primitive::Flags);
        uint8_t bit = (uint8_t)Offsets::PrimitiveFlags::Anchored;
        uint8_t n = v ? (f | bit) : (f & ~bit);
        if (n != f) memory->write<uint8_t>(prim + Offsets::Primitive::Flags, n);
    }
    void SetVelocity(RBX::Vec3 v) const {
        auto prim = GetPrimitivePtr();
        if (prim) memory->write<RBX::Vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity, v);
    }
    void SetCFrame(const RBX::CFrame& cf) const {
        auto prim = GetPrimitivePtr();
        if (prim) memory->write<RBX::CFrame>(prim + Offsets::Primitive::Rotation, cf);
    }
    void SetCameraType(int v) const { if(Addr) memory->write<int>(Addr + Offsets::Camera::CameraType, v); }
    int GetCameraType() const { return Addr ? memory->read<int>(Addr + Offsets::Camera::CameraType) : 0; }
    void SetCameraSubject(std::uintptr_t v) const { if(Addr) memory->write<std::uintptr_t>(Addr + Offsets::Camera::CameraSubject, v); }
    std::uintptr_t GetCameraSubject() const { return Addr ? memory->read<std::uintptr_t>(Addr + Offsets::Camera::CameraSubject) : 0; }
    RBX::CFrame GetCameraCFrame() const { return Addr ? memory->read<RBX::CFrame>(Addr + Offsets::Camera::Rotation) : RBX::CFrame{}; }
    void SetCameraCFrame(const RBX::CFrame& cf) const { if(Addr) memory->write<RBX::CFrame>(Addr + Offsets::Camera::Rotation, cf); }
    RbxInstance GetCurrentCamera() const { return RbxInstance(Addr ? memory->read<std::uintptr_t>(Addr + Offsets::Workspace::CurrentCamera) : 0); }

    void SetSize(const rbx::vector3_t& size) const {
        const auto part = GetPart();
        if (part)
            memory->write<rbx::vector3_t>(part + Offsets::Primitive::Size, size);
    }

    void SetAnimationId(const std::string& id) const {
        if (Addr)
            WriteString(Addr + Offsets::Misc::AnimationId, id);
    }

    float CalcDistance(const Vec3& p) const {
        const Vec3 c = GetPos();
        const float dx = c.X - p.X, dy = c.Y - p.Y, dz = c.Z - p.Z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
};

class RenderEngine : public RbxInstance {
public:
    using RbxInstance::RbxInstance;

    Mat4 GetViewMat() const {
        return Addr ? memory->read<Mat4>(Addr + Offsets::VisualEngine::ViewMatrix) : Mat4{};
    }

    Vec2 WorldToViewport(const Vec3& w) const {
        Vec2 screen{};
        const Mat4 m = GetViewMat();
        const float qw = w.X * m.data[12] + w.Y * m.data[13] + w.Z * m.data[14] + m.data[15];
        if (qw < 0.1f)
            return screen;
        const float qx = w.X * m.data[0] + w.Y * m.data[1] + w.Z * m.data[2] + m.data[3];
        const float qy = w.X * m.data[4] + w.Y * m.data[5] + w.Z * m.data[6] + m.data[7];
        const float sw = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        const float sh = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
        screen.X = (sw * 0.5f * (qx / qw)) + (sw * 0.5f);
        screen.Y = -(sh * 0.5f * (qy / qw)) + (sh * 0.5f);
        return screen;
    }
};

inline void ModifyWalkSpeed(const RbxInstance& h, float v) {
    if (!h.Addr)
        return;
    memory->write(h.Addr + Offsets::Humanoid::Walkspeed, v);
    memory->write(h.Addr + Offsets::Humanoid::WalkspeedCheck, v);
}

inline void ModifyJumpPower(const RbxInstance& h, float v) {
    if (!h.Addr)
        return;
    memory->write(h.Addr + Offsets::Humanoid::JumpPower, v);
    memory->write(h.Addr + 0x1AC, v);
}
}
