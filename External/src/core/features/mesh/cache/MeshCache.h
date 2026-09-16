#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace Cheat {
namespace Visuals {

#pragma pack(push, 1)
struct MeshVertex {
	float pos[3];
	float normal[3];
	float uv[2];
	std::uint32_t tangent;
	std::uint32_t color;
};
static_assert(sizeof(MeshVertex) == 40, "MeshVertex");

struct MeshFace {
	std::uint32_t indices[3];
};
static_assert(sizeof(MeshFace) == 12, "MeshFace");
#pragma pack(pop)

struct CachedMesh {
	std::string asset_id;
	std::vector<MeshVertex> vertices;
	std::vector<MeshFace> faces;
};


class MeshCache {
public:
	static MeshCache& Get();

	void Refresh(bool force = false);

	std::shared_ptr<const CachedMesh> FindShared(const std::string& asset_id) const;
	bool Find(const std::string& asset_id, CachedMesh& out) const;
	std::size_t Count() const;

private:
	MeshCache() = default;

	mutable std::mutex mtx_;
	std::unordered_map<std::string, std::shared_ptr<CachedMesh>> cache_;
	std::uint64_t last_refresh_{ 0 };
};

std::string CleanAssetId(const std::string& raw);

}
}
