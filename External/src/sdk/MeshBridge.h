#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include "MeshMath.h"

#include "sdk/sdk.h"
#include "sdk/offsets.h"
#include "memory/memory.h"

namespace {
    struct MemoryBridge {
        template<typename T>
        T Read(uintptr_t addr) const {
            if (!addr) return {};
            return memory->read<T>(addr);
        }
        template<typename T>
        bool Write(uintptr_t addr, T val) const {
            if (!addr) return false;
            return memory->write<T>(addr, val);
        }
        std::string ReadString(uintptr_t addr) const {
            if (!addr) return "";
            return memory->read_string(addr);
        }
        void ReadRaw(uintptr_t addr, void* out, size_t sz) const {
            if (!addr || !out || !sz) return;
            memory->read_raw(addr, out, (std::uint32_t)sz);
        }
        bool IsValid(uintptr_t addr) const { return addr != 0; }
    } inline g_Memory;



    inline uintptr_t CachedPrimitive(uintptr_t part)
    {
        if (!part)
            return 0;
        struct Entry { uintptr_t prim; std::uint64_t tick; };
        static std::unordered_map<uintptr_t, Entry> s_cache;
        static std::uint64_t s_gc = 0;
        const std::uint64_t now = GetTickCount64();
        if (now - s_gc > 10000)
        {
            for (auto it = s_cache.begin(); it != s_cache.end(); )
            {
                if (now - it->second.tick > 10000)
                    it = s_cache.erase(it);
                else
                    ++it;
            }
            s_gc = now;
        }
        auto it = s_cache.find(part);
        if (it != s_cache.end() && now - it->second.tick < 5000)
            return it->second.prim;
        uintptr_t prim = memory->read<uintptr_t>(part + Offsets::BasePart::Primitive);
        if (prim)
            s_cache[part] = { prim, now };
        else if (it != s_cache.end())
            s_cache.erase(it);
        return prim;
    }

    inline Mesh::Vector3 CachedSize(uintptr_t part)
    {
        struct Entry { Mesh::Vector3 sz; std::uint64_t tick; };
        static std::unordered_map<uintptr_t, Entry> s_cache;
        static std::uint64_t s_gc = 0;
        const std::uint64_t now = GetTickCount64();
        if (now - s_gc > 30000)
        {
            for (auto it = s_cache.begin(); it != s_cache.end(); )
            {
                if (now - it->second.tick > 30000)
                    it = s_cache.erase(it);
                else
                    ++it;
            }
            s_gc = now;
        }
        auto it = s_cache.find(part);
        if (it != s_cache.end() && now - it->second.tick < 30000)
            return it->second.sz;
        Mesh::Vector3 sz{};
        const uintptr_t prim = CachedPrimitive(part);
        if (prim)
        {
            sz = memory->read<Mesh::Vector3>(prim + Offsets::Primitive::Size);
            s_cache[part] = { sz, now };
        }
        else if (it != s_cache.end())
            s_cache.erase(it);
        return sz;
    }
}

namespace Cheat {
    using namespace Mesh;

#ifdef GetClassName
#undef GetClassName
#endif

    class Instance {

    public:
        uintptr_t address;
        Instance() : address(0) {}
        Instance(uintptr_t addr) : address(addr) {}
        virtual ~Instance() = default;

        std::string GetName() const {
            if (!address) return "";
            return RBX::RbxInstance(address).GetName();
        }
        std::string GetClassName() const {
            if (!address) return "";
            return RBX::RbxInstance(address).GetClass();
        }
        std::vector<Instance> GetChildren() const {
            std::vector<Instance> res;
            if (!address) return res;
            for (auto& c : RBX::RbxInstance(address).GetChildList()) {
                res.push_back(Instance(c.Addr));
            }
            return res;
        }
        std::shared_ptr<Instance> FindFirstChild(const std::string& name) const {
            if (!address) return nullptr;
            uintptr_t addr = RBX::RbxInstance(address).FindFirstChild(name).Addr;
            return addr ? std::make_shared<Instance>(addr) : nullptr;
        }
        std::shared_ptr<Instance> GetParent() const {
            if (!address) return nullptr;
            uintptr_t addr = RBX::RbxInstance(address).GetParent().Addr;
            return addr ? std::make_shared<Instance>(addr) : nullptr;
        }
        bool operator==(const Instance& other) const { return address == other.address; }
    };

    class BasePart : public Instance {
    public:
        BasePart(uintptr_t addr) : Instance(addr) {}

        uintptr_t GetPrimitive() const {
            return CachedPrimitive(address);
        }




        bool GetFrameData(Vector3& pos, Matrix4x4& rot, Vector3& sz) const {
            pos = {}; rot = Matrix4x4(); sz = {};
            const uintptr_t prim = CachedPrimitive(address);
            if (!prim)
                return false;
            struct FrameData {
                float r[9];
                Vector3 p;
            };
            static_assert(sizeof(FrameData) == 48, "unexpected primitive layout");
            FrameData fd{};
            if (!memory->read_raw(prim + Offsets::Primitive::Rotation, &fd, sizeof(fd)))
                return false;
            rot = Matrix4x4(
                fd.r[0], fd.r[1], fd.r[2], 0.f,
                fd.r[3], fd.r[4], fd.r[5], 0.f,
                fd.r[6], fd.r[7], fd.r[8], 0.f,
                0.f, 0.f, 0.f, 1.f
            );
            pos = fd.p;
            sz = CachedSize(address);
            return true;
        }

        Vector3 GetPosition() const {
            const uintptr_t prim = CachedPrimitive(address);
            if (!prim) return {};
            return memory->read<Vector3>(prim + Offsets::Primitive::Position);
        }

        Vector3 GetSize() const {
            const uintptr_t prim = CachedPrimitive(address);
            if (!prim) return {};
            return memory->read<Vector3>(prim + Offsets::Primitive::Size);
        }

        Matrix4x4 GetRotation() const {
            const uintptr_t prim = CachedPrimitive(address);
            if (!prim) return {};
            float rot[9];
            g_Memory.ReadRaw(prim + Offsets::Primitive::Rotation, &rot, sizeof(rot));
            return Matrix4x4(
                rot[0], rot[1], rot[2], 0.f,
                rot[3], rot[4], rot[5], 0.f,
                rot[6], rot[7], rot[8], 0.f,
                0.f, 0.f, 0.f, 1.f
            );
        }

        float GetTransparency() const {
            if (!address) return 0.f;
            return memory->read<float>(address + Offsets::BasePart::Transparency);
        }

        Color3 GetColor() const {
            if (!address) return Color3(0.8f, 0.8f, 0.8f);
            Color3 direct = memory->read<Color3>(address + Offsets::BasePart::Color3);
            if (direct.r > 0.001f || direct.g > 0.001f || direct.b > 0.001f)
                return direct;
            return Color3(0.8f, 0.8f, 0.8f);
        }
    };

    class MeshPart : public Instance {
    public:
        MeshPart(uintptr_t addr) : Instance(addr) {}
        std::string GetMeshId() const {
            if (!address) return "Unknown";
            std::string s = memory->read_string(address + Offsets::MeshPart::MeshId);
            if (!s.empty() && s != "Unknown") return s;
            uintptr_t mesh = memory->read<uintptr_t>(address + Offsets::MeshPart::MeshId);
            if (!mesh) return "Unknown";
            return memory->read_string(mesh);
        }
    };

    class SpecialMesh : public Instance {
    public:
        SpecialMesh(uintptr_t addr) : Instance(addr) {}
        std::string GetMeshId() const {
            if (!address) return "Unknown";
            std::string s = memory->read_string(address + Offsets::SpecialMesh::MeshId);
            if (!s.empty() && s != "Unknown") return s;
            uintptr_t mesh = memory->read<uintptr_t>(address + Offsets::SpecialMesh::MeshId);
            if (!mesh) return "Unknown";
            return memory->read_string(mesh);
        }
        Vector3 GetScale() const {
            if (!address) return Vector3(1.f, 1.f, 1.f);
            return memory->read<Vector3>(address + Offsets::SpecialMesh::Scale);
        }
        Vector3 GetOffset() const {
            if (!address) return Vector3(0.f, 0.f, 0.f);
            return memory->read<Vector3>(address + Offsets::SpecialMesh::Offset);
        }
    };
}
