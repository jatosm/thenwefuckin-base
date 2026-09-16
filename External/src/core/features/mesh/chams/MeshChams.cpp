#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "MeshChams.h"
#include "core/features/mesh/cache/MeshCache.h"
#include "core/features/mesh/shader/MeshDxShader.h"
#include "core/features/mesh/parser/MeshParser.h"
#include "core/variables/variables.h"
#include "sdk/MeshBridge.h"

using namespace Mesh;

#undef GetClassName

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Cheat {
namespace Visuals {
namespace MeshChams {
namespace {

bool W2S(const Matrix4x4& m, const Vector2& dim, float sx, float sy,
         const Vector3& p, Vector2& out)
{
	float w = p.x * m.m[3][0] + p.y * m.m[3][1] + p.z * m.m[3][2] + m.m[3][3];
	if (w < 0.01f)
		return false;
	float x = p.x * m.m[0][0] + p.y * m.m[0][1] + p.z * m.m[0][2] + m.m[0][3];
	float y = p.x * m.m[1][0] + p.y * m.m[1][1] + p.z * m.m[1][2] + m.m[1][3];
	float inv = 1.f / w;
	x *= inv;
	y *= inv;
	out.x = ((dim.x * 0.5f) + (x * dim.x * 0.5f)) * sx;
	out.y = ((dim.y * 0.5f) - (y * dim.y * 0.5f)) * sy;
	return true;
}

std::string ReadMeshIdAt(std::uint64_t addr)
{
	if (!g_Memory.IsValid(addr))
		return {};
	std::string s = g_Memory.ReadString(addr);
	if (!s.empty() && s != "Unknown")
		return s;
	std::uint64_t p = g_Memory.Read<std::uint64_t>(addr);
	if (!g_Memory.IsValid(p))
		return {};
	s = g_Memory.ReadString(p);
	if (s == "Unknown")
		return {};
	return s;
}

int BodyPartIndex(const std::string& name)
{
	if (name == "Torso" || name == "UpperTorso" || name == "LowerTorso")
		return 1;
	if (name == "Left Arm" || name == "LeftUpperArm" || name == "LeftLowerArm" || name == "LeftHand")
		return 2;
	if (name == "Right Arm" || name == "RightUpperArm" || name == "RightLowerArm" || name == "RightHand")
		return 3;
	if (name == "Left Leg" || name == "LeftUpperLeg" || name == "LeftLowerLeg" || name == "LeftFoot")
		return 4;
	if (name == "Right Leg" || name == "RightUpperLeg" || name == "RightLowerLeg" || name == "RightFoot")
		return 5;
	return -1;
}

std::string FindCharacterMeshId(std::uint64_t character, int body_part)
{
	if (body_part < 0 || !g_Memory.IsValid(character))
		return {};
	for (const auto& c : Instance(character).GetChildren())
	{
		if (c.GetClassName() != "CharacterMesh")
			continue;
		const int bp = g_Memory.Read<int>(c.address + Offsets::CharacterMesh::BodyPart);
		if (bp != body_part)
			continue;
		std::string id = ReadMeshIdAt(c.address + Offsets::CharacterMesh::MeshId);
		if (!id.empty())
			return id;
	}
	return {};
}

struct ResolveResult {
	std::string mesh_id;
	Vector3 scale{ 1.f, 1.f, 1.f };
	Vector3 offset{ 0.f, 0.f, 0.f };
	bool fit_to_part{ false };
	bool is_special{ false };
};

ResolveResult Resolve(const MeshParser::Entry& e)
{
	ResolveResult r{};
	if (!e.part)
		return r;

	if (e.class_name == "MeshPart")
	{
		r.mesh_id = ReadMeshIdAt(e.part + Offsets::MeshPart::MeshId);
		r.fit_to_part = true;
	}

	if (e.special_mesh)
	{
		r.is_special = true;
		if (r.mesh_id.empty())
			r.mesh_id = ReadMeshIdAt(e.special_mesh + Offsets::SpecialMesh::MeshId);
		r.scale = g_Memory.Read<Vector3>(e.special_mesh + Offsets::SpecialMesh::Scale);
		r.offset = g_Memory.Read<Vector3>(e.special_mesh + Offsets::SpecialMesh::Offset);
		auto clamp_sc = [](float& v) {
			if (!std::isfinite(v) || v < 1e-4f) v = 1.f;
			if (v > 50.f) v = 50.f;
		};
		clamp_sc(r.scale.x);
		clamp_sc(r.scale.y);
		clamp_sc(r.scale.z);
		r.fit_to_part = false;
	}

	if (r.mesh_id.empty() && !e.mesh_id.empty())
		r.mesh_id = e.mesh_id;

	if (r.mesh_id.empty() && e.kind == MeshParser::Kind::Body)
	{
		r.mesh_id = FindCharacterMeshId(e.character, BodyPartIndex(e.name));
		if (!r.mesh_id.empty())
			r.fit_to_part = true;
	}

	if (e.name == "Head")
	{
		const bool have = !r.mesh_id.empty() && (bool)MeshCache::Get().FindShared(r.mesh_id);
		if (!have)
		{
			r.mesh_id = "rbxasset://avatar/heads/head.mesh";
			r.fit_to_part = false;
			if (!r.is_special)
			{
				r.scale = { 1.f, 1.f, 1.f };
				r.offset = { 0.f, 0.f, 0.f };
			}
		}
		else if (r.is_special)
		{
			r.fit_to_part = false;
		}
	}

	return r;
}

std::shared_ptr<const CachedMesh> LookupMesh(const std::string& mesh_id)
{
	if (mesh_id.empty())
		return nullptr;
	return MeshCache::Get().FindShared(mesh_id);
}

struct FitCacheKey {
	std::uint64_t part{ 0 };
	std::size_t mesh_h{ 0 };
	int qx{ 0 }, qy{ 0 }, qz{ 0 };
	int flags{ 0 };
	bool operator==(const FitCacheKey& o) const
	{
		return part == o.part && mesh_h == o.mesh_h && qx == o.qx && qy == o.qy &&
			qz == o.qz && flags == o.flags;
	}
};
struct FitCacheKeyHash {
	std::size_t operator()(const FitCacheKey& k) const
	{
		std::size_t h = (std::size_t)k.part ^ (k.mesh_h * 0x9e3779b97f4a7c15ull);
		h ^= ((std::size_t)k.qx << 1) ^ ((std::size_t)k.qy << 11) ^ ((std::size_t)k.qz << 21);
		h ^= (std::size_t)k.flags * 0x85ebca77u;
		return h;
	}
};
struct FitCacheVal {
	Vector3 ms{ 1.f, 1.f, 1.f };
	Vector3 off{ 0.f, 0.f, 0.f };
};

FitCacheKey MakeFitKey(
	std::uint64_t part,
	const std::string& mesh_id,
	const Vector3& sz,
	const Vector3& scale,
	int flags)
{
	FitCacheKey k;
	k.part = part;
	k.mesh_h = std::hash<std::string>{}(mesh_id);
	k.qx = (int)std::lround(sz.x * 100.f) ^ ((int)std::lround(scale.x * 100.f) << 16);
	k.qy = (int)std::lround(sz.y * 100.f) ^ ((int)std::lround(scale.y * 100.f) << 16);
	k.qz = (int)std::lround(sz.z * 100.f) ^ ((int)std::lround(scale.z * 100.f) << 16);
	k.flags = flags;
	return k;
}

bool IsAccessoryKind(MeshParser::Kind k)
{
	return k == MeshParser::Kind::Accessory ||
		k == MeshParser::Kind::Hair ||
		k == MeshParser::Kind::Face;
}

bool MeshAabb(const CachedMesh& mesh, const Vector3& ms, float out_min[3], float out_max[3])
{
	out_min[0] = out_min[1] = out_min[2] = FLT_MAX;
	out_max[0] = out_max[1] = out_max[2] = -FLT_MAX;
	const int n = (int)mesh.vertices.size();
	if (n <= 0)
		return false;
	int step = 1;
	if (n > 16000)
		step = (n + 15999) / 16000;
	for (int i = 0; i < n; i += step)
	{
		const float* p = mesh.vertices[i].pos;
		if (!std::isfinite(p[0]) || !std::isfinite(p[1]) || !std::isfinite(p[2]))
			continue;
		float px = p[0] * ms.x, py = p[1] * ms.y, pz = p[2] * ms.z;
		out_min[0] = (std::min)(out_min[0], px); out_max[0] = (std::max)(out_max[0], px);
		out_min[1] = (std::min)(out_min[1], py); out_max[1] = (std::max)(out_max[1], py);
		out_min[2] = (std::min)(out_min[2], pz); out_max[2] = (std::max)(out_max[2], pz);
	}
	if (n > 1)
	{
		for (int i : { 0, n - 1 })
		{
			const float* p = mesh.vertices[i].pos;
			if (!std::isfinite(p[0])) continue;
			float px = p[0] * ms.x, py = p[1] * ms.y, pz = p[2] * ms.z;
			out_min[0] = (std::min)(out_min[0], px); out_max[0] = (std::max)(out_max[0], px);
			out_min[1] = (std::min)(out_min[1], py); out_max[1] = (std::max)(out_max[1], py);
			out_min[2] = (std::min)(out_min[2], pz); out_max[2] = (std::max)(out_max[2], pz);
		}
	}
	return out_min[0] <= out_max[0];
}

void FitScaleToPart(const CachedMesh& mesh, Vector3& ms, const Vector3& sz)
{
	if (sz.x <= 0.01f || sz.y <= 0.01f || sz.z <= 0.01f)
		return;
	float mn[3], mx[3];
	if (!MeshAabb(mesh, ms, mn, mx))
		return;
	float ax = mx[0] - mn[0], ay = mx[1] - mn[1], az = mx[2] - mn[2];
	if (ax < 1e-5f || ay < 1e-5f || az < 1e-5f)
		return;
	const float rx = sz.x / ax, ry = sz.y / ay, rz = sz.z / az;
	if (rx > 0.88f && rx < 1.12f && ry > 0.88f && ry < 1.12f && rz > 0.88f && rz < 1.12f)
		return;
	auto apply = [](float& m, float r) {
		if (r < 0.001f) r = 0.001f;
		if (r > 500.f) r = 500.f;
		m *= r;
	};
	apply(ms.x, rx);
	apply(ms.y, ry);
	apply(ms.z, rz);
}

void RecenterOffset(const CachedMesh& mesh, const Vector3& ms, Vector3& off)
{
	float mn[3], mx[3];
	if (!MeshAabb(mesh, ms, mn, mx))
		return;
	off.x -= (mn[0] + mx[0]) * 0.5f;
	off.y -= (mn[1] + mx[1]) * 0.5f;
	off.z -= (mn[2] + mx[2]) * 0.5f;
}

void SanityFitIfHuge(const CachedMesh& mesh, Vector3& ms, const Vector3& sz, bool is_acc)
{
	float mn[3], mx[3];
	if (!MeshAabb(mesh, ms, mn, mx))
		return;
	const float ext = (std::max)(mx[0] - mn[0], (std::max)(mx[1] - mn[1], mx[2] - mn[2]));
	const float part_ext = (std::max)(sz.x, (std::max)(sz.y, sz.z));
	const bool huge = ext > 40.f || (is_acc && part_ext > 0.05f && ext > part_ext * 5.f);
	if (huge)
		FitScaleToPart(mesh, ms, sz);
}

bool IsClassicHeadMesh(const std::string& id)
{
	return id.find("heads/head.mesh") != std::string::npos ||
	       CleanAssetId(id) == "head.mesh";
}

void FitClassicR6Head(const CachedMesh& mesh, Vector3& ms, const Vector3& sz,
                      const Vector3& sm_scale)
{
	float mn[3], mx[3];
	const Vector3 one{ 1.f, 1.f, 1.f };
	if (!MeshAabb(mesh, one, mn, mx))
		return;
	const float ext = (std::max)(mx[0] - mn[0], (std::max)(mx[1] - mn[1], mx[2] - mn[2]));
	if (ext < 1e-5f)
		return;

	float sy = sm_scale.y;
	if (!std::isfinite(sy) || sy < 0.25f || sy > 3.f)
		sy = 1.25f;

	float target = (sz.y > 0.1f) ? (sz.y * sy) : sy;
	if (target < 0.2f) target = 0.2f;
	if (target > 3.f) target = 3.f;

	const float s = target / ext;
	ms = { s, s, s };
}

void ApplyVisualFit(
	const MeshParser::Entry& e,
	const ResolveResult& rr,
	const CachedMesh& mesh,
	Vector3& ms,
	Vector3& off,
	const Vector3& sz)
{
	const bool is_acc = IsAccessoryKind(e.kind);

	if (e.name == "Head" && IsClassicHeadMesh(rr.mesh_id))
	{
		FitClassicR6Head(mesh, ms, sz, rr.scale);
		if (!std::isfinite(off.x) || !std::isfinite(off.y) || !std::isfinite(off.z) ||
			std::fabs(off.x) > 2.f || std::fabs(off.y) > 2.f || std::fabs(off.z) > 2.f)
			off = { 0.f, 0.f, 0.f };
		RecenterOffset(mesh, ms, off);
		return;
	}

	if (rr.is_special)
		return;

	if (rr.fit_to_part)
	{
		FitScaleToPart(mesh, ms, sz);
		if (!is_acc)
			RecenterOffset(mesh, ms, off);
		return;
	}

	if (!is_acc)
		SanityFitIfHuge(mesh, ms, sz, false);
}

Matrix4x4 MakeWorld(const Vector3& pos, const Matrix4x4& rot,
                    const Vector3& ms, const Vector3& off)
{
	Matrix4x4 w{};
	w.m[0][0] = rot.m[0][0] * ms.x;
	w.m[0][1] = rot.m[0][1] * ms.y;
	w.m[0][2] = rot.m[0][2] * ms.z;
	w.m[0][3] = pos.x + rot.m[0][0] * off.x + rot.m[0][1] * off.y + rot.m[0][2] * off.z;
	w.m[1][0] = rot.m[1][0] * ms.x;
	w.m[1][1] = rot.m[1][1] * ms.y;
	w.m[1][2] = rot.m[1][2] * ms.z;
	w.m[1][3] = pos.y + rot.m[1][0] * off.x + rot.m[1][1] * off.y + rot.m[1][2] * off.z;
	w.m[2][0] = rot.m[2][0] * ms.x;
	w.m[2][1] = rot.m[2][1] * ms.y;
	w.m[2][2] = rot.m[2][2] * ms.z;
	w.m[2][3] = pos.z + rot.m[2][0] * off.x + rot.m[2][1] * off.y + rot.m[2][2] * off.z;
	w.m[3][0] = 0.f;
	w.m[3][1] = 0.f;
	w.m[3][2] = 0.f;
	w.m[3][3] = 1.f;
	return w;
}

Matrix4x4 MakeBoxWorld(const Vector3& pos, const Matrix4x4& rot, const Vector3& sz)
{
	return MakeWorld(pos, rot, sz, { 0.f, 0.f, 0.f });
}

void DrawBoxFallback(
	ImDrawList* dl,
	const Vector3& pos,
	const Matrix4x4& rot,
	const Vector3& sz,
	const Matrix4x4& view,
	const Vector2& vp,
	float scale_x,
	float scale_y,
	ImU32 fill)
{
	if (sz.x < 0.01f && sz.y < 0.01f && sz.z < 0.01f)
		return;
	const Vector3 h{ sz.x * 0.5f, sz.y * 0.5f, sz.z * 0.5f };
	const Vector3 lc[8] = {
		{ -h.x, -h.y, -h.z }, { -h.x, -h.y,  h.z },
		{ -h.x,  h.y, -h.z }, { -h.x,  h.y,  h.z },
		{  h.x, -h.y, -h.z }, {  h.x, -h.y,  h.z },
		{  h.x,  h.y, -h.z }, {  h.x,  h.y,  h.z },
	};
	ImVec2 sp[8];
	bool sv[8]{};
	bool any = false;
	for (int i = 0; i < 8; ++i)
	{
		Vector3 wc{
			pos.x + rot.m[0][0] * lc[i].x + rot.m[0][1] * lc[i].y + rot.m[0][2] * lc[i].z,
			pos.y + rot.m[1][0] * lc[i].x + rot.m[1][1] * lc[i].y + rot.m[1][2] * lc[i].z,
			pos.z + rot.m[2][0] * lc[i].x + rot.m[2][1] * lc[i].y + rot.m[2][2] * lc[i].z,
		};
		Vector2 sc;
		sv[i] = W2S(view, vp, scale_x, scale_y, wc, sc);
		sp[i] = sv[i] ? ImVec2(sc.x, sc.y) : ImVec2(-9999.f, -9999.f);
		any = any || sv[i];
	}
	if (!any)
		return;
	static const int tris[12][3] = {
		{ 0, 1, 3 }, { 0, 3, 2 }, { 4, 6, 7 }, { 4, 7, 5 },
		{ 0, 4, 5 }, { 0, 5, 1 }, { 2, 3, 7 }, { 2, 7, 6 },
		{ 0, 2, 6 }, { 0, 6, 4 }, { 1, 5, 7 }, { 1, 7, 3 },
	};
	for (const auto& t : tris)
	{
		if (!sv[t[0]] || !sv[t[1]] || !sv[t[2]])
			continue;
		ImVec2 a = sp[t[0]], b = sp[t[1]], c = sp[t[2]];
		float cr = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		if (cr < 0.15f)
			continue;
		dl->AddTriangleFilled(a, b, c, fill);
	}
}

struct ScreenTri {
	ImVec2 a, b, c;
};

void PushOutlineFromPartBox(
	std::vector<ScreenTri>& out,
	const Vector3& pos,
	const Matrix4x4& rot,
	const Vector3& sz,
	const Matrix4x4& view,
	const Vector2& vp,
	float scale_x,
	float scale_y)
{
	if (sz.x < 0.01f && sz.y < 0.01f && sz.z < 0.01f)
		return;
	const Vector3 h{ sz.x * 0.5f, sz.y * 0.5f, sz.z * 0.5f };
	const Vector3 lc[8] = {
		{ -h.x, -h.y, -h.z }, { -h.x, -h.y,  h.z },
		{ -h.x,  h.y, -h.z }, { -h.x,  h.y,  h.z },
		{  h.x, -h.y, -h.z }, {  h.x, -h.y,  h.z },
		{  h.x,  h.y, -h.z }, {  h.x,  h.y,  h.z },
	};
	ImVec2 sp[8];
	bool sv[8]{};
	bool any = false;
	for (int i = 0; i < 8; ++i)
	{
		Vector3 wc{
			pos.x + rot.m[0][0] * lc[i].x + rot.m[0][1] * lc[i].y + rot.m[0][2] * lc[i].z,
			pos.y + rot.m[1][0] * lc[i].x + rot.m[1][1] * lc[i].y + rot.m[1][2] * lc[i].z,
			pos.z + rot.m[2][0] * lc[i].x + rot.m[2][1] * lc[i].y + rot.m[2][2] * lc[i].z,
		};
		Vector2 sc;
		sv[i] = W2S(view, vp, scale_x, scale_y, wc, sc);
		sp[i] = sv[i] ? ImVec2(sc.x, sc.y) : ImVec2(-9999.f, -9999.f);
		any = any || sv[i];
	}
	if (!any)
		return;
	static const int tris[12][3] = {
		{ 0, 1, 3 }, { 0, 3, 2 }, { 4, 6, 7 }, { 4, 7, 5 },
		{ 0, 4, 5 }, { 0, 5, 1 }, { 2, 3, 7 }, { 2, 7, 6 },
		{ 0, 2, 6 }, { 0, 6, 4 }, { 1, 5, 7 }, { 1, 7, 3 },
	};
	for (const auto& t : tris)
	{
		if (!sv[t[0]] || !sv[t[1]] || !sv[t[2]])
			continue;
		ImVec2 a = sp[t[0]], b = sp[t[1]], c = sp[t[2]];
		float cr = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		if (cr < 0.15f)
			continue;
		out.push_back({ a, b, c });
	}
}

bool PointInTri(const ImVec2& p, const ImVec2& a, const ImVec2& b, const ImVec2& c)
{
	const float x0 = c.x - a.x, y0 = c.y - a.y;
	const float x1 = b.x - a.x, y1 = b.y - a.y;
	const float x2 = p.x - a.x, y2 = p.y - a.y;
	const float dot00 = x0 * x0 + y0 * y0;
	const float dot01 = x0 * x1 + y0 * y1;
	const float dot02 = x0 * x2 + y0 * y2;
	const float dot11 = x1 * x1 + y1 * y1;
	const float dot12 = x1 * x2 + y1 * y2;
	const float inv = dot00 * dot11 - dot01 * dot01;
	if (std::fabs(inv) < 1e-12f)
		return false;
	const float u = (dot11 * dot02 - dot01 * dot12) / inv;
	const float v = (dot00 * dot12 - dot01 * dot02) / inv;
	return u >= 0.f && v >= 0.f && (u + v) <= 1.f;
}

void RasterizeTriMask(std::vector<std::uint8_t>& mask, int gw, int gh,
                      float origin_x, float origin_y, float cell,
                      const ScreenTri& t)
{
	const float minx = (std::min)(t.a.x, (std::min)(t.b.x, t.c.x));
	const float miny = (std::min)(t.a.y, (std::min)(t.b.y, t.c.y));
	const float maxx = (std::max)(t.a.x, (std::max)(t.b.x, t.c.x));
	const float maxy = (std::max)(t.a.y, (std::max)(t.b.y, t.c.y));

	int x0 = (int)std::floor((minx - origin_x) / cell);
	int y0 = (int)std::floor((miny - origin_y) / cell);
	int x1 = (int)std::floor((maxx - origin_x) / cell);
	int y1 = (int)std::floor((maxy - origin_y) / cell);
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= gw) x1 = gw - 1;
	if (y1 >= gh) y1 = gh - 1;

	for (int y = y0; y <= y1; ++y)
	{
		for (int x = x0; x <= x1; ++x)
		{
			const ImVec2 p{
				origin_x + ((float)x + 0.5f) * cell,
				origin_y + ((float)y + 0.5f) * cell
			};
			if (PointInTri(p, t.a, t.b, t.c))
				mask[(std::size_t)y * (std::size_t)gw + (std::size_t)x] = 1;
		}
	}
}

ImU32 OutlineCol(float r, float g, float b, float a)
{
	if (a < 0.f) a = 0.f;
	if (a > 1.f) a = 1.f;
	int R = (int)(r * 255.f + 0.5f);
	int G = (int)(g * 255.f + 0.5f);
	int B = (int)(b * 255.f + 0.5f);
	int A = (int)(a * 255.f + 0.5f);
	if (R > 255) R = 255;
	if (G > 255) G = 255;
	if (B > 255) B = 255;
	if (A > 255) A = 255;
	return IM_COL32(R, G, B, A);
}

void SoftBrush(ImDrawList* dl, const ImVec2& c, float radius, float cr, float cg, float cb, float ca)
{
	if (radius < 0.5f || ca < 0.003f)
		return;
	constexpr int k_layers = 10;
	constexpr int k_segs = 20;
	for (int i = k_layers; i >= 1; --i)
	{
		const float t = (float)i / (float)k_layers;
		const float fall = 1.f - t;
		const float a = ca * fall * fall * fall * 1.55f;
		if (a < 0.003f)
			continue;
		dl->AddCircleFilled(c, radius * t, OutlineCol(cr, cg, cb, a), k_segs);
	}
}

void SoftStroke(
	ImDrawList* dl,
	const ImVec2& a,
	const ImVec2& b,
	float glow,
	float cr, float cg, float cb, float ca)
{
	if (ca < 0.003f || glow < 0.5f)
		return;
	constexpr int k_layers = 8;
	for (int i = k_layers; i >= 1; --i)
	{
		const float t = (float)i / (float)k_layers;
		const float fall = 1.f - t;
		const float alpha = ca * fall * fall * 1.1f;
		if (alpha < 0.003f)
			continue;
		dl->AddLine(a, b, OutlineCol(cr, cg, cb, alpha), glow * t);
	}
	dl->AddLine(a, b, OutlineCol(cr, cg, cb, ca * 0.55f), 1.8f);
}

void DrawCharacterContour(
	ImDrawList* dl,
	const std::vector<ScreenTri>& tris,
	float cr, float cg, float cb, float ca,
	int style)
{
	if (!dl || tris.empty())
		return;
	if (style < 0) style = 0;
	if (style > 3) style = 3;
	if (ca <= 0.001f)
		return;

	float minx = FLT_MAX, miny = FLT_MAX, maxx = -FLT_MAX, maxy = -FLT_MAX;
	for (const auto& t : tris)
	{
		for (const ImVec2* p : { &t.a, &t.b, &t.c })
		{
			minx = (std::min)(minx, p->x);
			miny = (std::min)(miny, p->y);
			maxx = (std::max)(maxx, p->x);
			maxy = (std::max)(maxy, p->y);
		}
	}
	if (!(minx < maxx) || !(miny < maxy))
		return;

	const float body_w = maxx - minx;
	const float body_h = maxy - miny;
	if (body_w < 2.f || body_h < 2.f)
		return;

	constexpr int k_max_dim = 192;
	float cell = (std::max)(body_w, body_h) / (float)k_max_dim;
	if (cell < 0.7f) cell = 0.7f;

	minx -= 2.f; miny -= 2.f; maxx += 2.f; maxy += 2.f;
	const float bw = maxx - minx;
	const float bh = maxy - miny;

	int gw = (int)std::ceil(bw / cell) + 1;
	int gh = (int)std::ceil(bh / cell) + 1;
	if (gw < 4 || gh < 4)
		return;
	if (gw > 280) gw = 280;
	if (gh > 280) gh = 280;

	const std::size_t n = (std::size_t)gw * (std::size_t)gh;
	static std::vector<std::uint8_t> mask;
	mask.assign(n, 0);
	for (const auto& t : tris)
		RasterizeTriMask(mask, gw, gh, minx, miny, cell, t);

	auto filled = [&](int x, int y) -> bool {
		if (x < 0 || y < 0 || x >= gw || y >= gh)
			return false;
		return mask[(std::size_t)y * (std::size_t)gw + (std::size_t)x] != 0;
	};

	struct Edge { ImVec2 a, b; };
	static std::vector<Edge> edges;
	edges.clear();
	if (edges.capacity() < 1024)
		edges.reserve(1024);
	static std::vector<ImVec2> joints;
	joints.clear();
	if (joints.capacity() < 512)
		joints.reserve(512);

	for (int y = 0; y < gh; ++y)
	{
		int x = 0;
		while (x < gw)
		{
			if (!filled(x, y) || filled(x, y - 1))
			{
				++x;
				continue;
			}
			const int x0 = x;
			while (x + 1 < gw && filled(x + 1, y) && !filled(x + 1, y - 1))
				++x;
			const float py = miny + (float)y * cell;
			edges.push_back({
				ImVec2(minx + (float)x0 * cell, py),
				ImVec2(minx + (float)(x + 1) * cell, py)
			});
			++x;
		}
		x = 0;
		while (x < gw)
		{
			if (!filled(x, y) || filled(x, y + 1))
			{
				++x;
				continue;
			}
			const int x0 = x;
			while (x + 1 < gw && filled(x + 1, y) && !filled(x + 1, y + 1))
				++x;
			const float py = miny + (float)(y + 1) * cell;
			edges.push_back({
				ImVec2(minx + (float)x0 * cell, py),
				ImVec2(minx + (float)(x + 1) * cell, py)
			});
			++x;
		}
	}
	for (int x = 0; x < gw; ++x)
	{
		int y = 0;
		while (y < gh)
		{
			if (!filled(x, y) || filled(x - 1, y))
			{
				++y;
				continue;
			}
			const int y0 = y;
			while (y + 1 < gh && filled(x, y + 1) && !filled(x - 1, y + 1))
				++y;
			const float px = minx + (float)x * cell;
			edges.push_back({
				ImVec2(px, miny + (float)y0 * cell),
				ImVec2(px, miny + (float)(y + 1) * cell)
			});
			++y;
		}
		y = 0;
		while (y < gh)
		{
			if (!filled(x, y) || filled(x + 1, y))
			{
				++y;
				continue;
			}
			const int y0 = y;
			while (y + 1 < gh && filled(x, y + 1) && !filled(x + 1, y + 1))
				++y;
			const float px = minx + (float)(x + 1) * cell;
			edges.push_back({
				ImVec2(px, miny + (float)y0 * cell),
				ImVec2(px, miny + (float)(y + 1) * cell)
			});
			++y;
		}
	}

	if (edges.empty())
		return;

	for (const auto& e : edges)
	{
		joints.push_back(e.a);
		joints.push_back(e.b);
	}

	const float tsec = (float)ImGui::GetTime();
	const float cx = minx + bw * 0.5f;
	const float cy = miny + bh * 0.5f;

	float fade = variables::ESP::meshChamsOutlineFade;
	if (fade < 0.35f) fade = 0.35f;
	if (fade > 3.f) fade = 3.f;

	float glow = (std::max)(body_w, body_h) * 0.062f * fade;
	if (glow < 7.f) glow = 7.f;
	if (glow > 48.f) glow = 48.f;

	float intensity = 0.78f * ca * (0.55f + 0.45f * (fade < 1.f ? fade : 1.f));
	float glow_mul = 1.f;

	switch (style)
	{
	case 1:
	{
		const float p = 0.55f + 0.45f * (0.5f + 0.5f * std::sin(tsec * 3.6f));
		intensity *= 0.55f + 0.7f * p;
		glow_mul = 0.75f + 0.45f * p;
		break;
	}
	case 2:
		intensity *= 0.85f;
		break;
	case 3:
		glow_mul = 1.28f;
		intensity *= 0.95f;
		break;
	default:
		break;
	}

	glow *= glow_mul;

	const ImDrawListFlags bak = dl->Flags;
	dl->Flags |= ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;

	auto edge_mod = [&](const ImVec2& a, const ImVec2& b) -> float {
		const float mx = (a.x + b.x) * 0.5f;
		const float my = (a.y + b.y) * 0.5f;
		if (style == 2)
		{
			const float phase = (mx + my) * 0.035f - tsec * 2.8f;
			return 0.45f + 0.55f * (0.5f + 0.5f * std::sin(phase));
		}
		if (style == 3)
		{
			const float ang = std::atan2(my - cy, mx - cx);
			return 0.5f + 0.5f * (0.5f + 0.5f * std::sin(ang * 4.f + tsec * 2.6f));
		}
		return 1.f;
	};

	for (const auto& e : edges)
	{
		const float mod = edge_mod(e.a, e.b);
		SoftStroke(dl, e.a, e.b, glow, cr, cg, cb, intensity * mod);
	}
	const float brush_r = glow * 0.58f;
	for (std::size_t i = 0; i < joints.size(); i += 4)
	{
		const ImVec2& p = joints[i];
		float mod = 1.f;
		if (style == 2)
		{
			const float phase = (p.x + p.y) * 0.035f - tsec * 2.8f;
			mod = 0.45f + 0.55f * (0.5f + 0.5f * std::sin(phase));
		}
		else if (style == 3)
		{
			const float ang = std::atan2(p.y - cy, p.x - cx);
			mod = 0.5f + 0.5f * (0.5f + 0.5f * std::sin(ang * 4.f + tsec * 2.6f));
		}
		SoftBrush(dl, p, brush_r, cr, cg, cb, intensity * 0.38f * mod);
	}

	dl->Flags = bak;
}

}

const char* const* OutlineStyleNames()
{
	static const char* k_names[] = {
		"soft breath",
		"pulse wave",
		"flow ribbon",
		"neon swirl",
	};
	return k_names;
}

int OutlineStyleNameCount()
{
	return 4;
}

bool ExpandBounds(
	std::uint64_t character,
	const Matrix4x4& view,
	const Vector2& viewport,
	float scale_x,
	float scale_y,
	float& min_x, float& max_x,
	float& min_y, float& max_y,
	Vector3& wmin, Vector3& wmax)
{
	if (!g_Memory.IsValid(character))
		return false;

	MeshCache::Get().Refresh(false);
	const auto parts = MeshParser::CollectForBounds(character);
	if (parts.empty())
		return false;

	bool any = false;
	auto clip_w = [&](const Vector3& p) -> float {
		return p.x * view.m[3][0] + p.y * view.m[3][1] + p.z * view.m[3][2] + view.m[3][3];
	};

	auto push_screen = [&](const Vector3& wc) {
		Vector2 sc;
		if (!W2S(view, viewport, scale_x, scale_y, wc, sc))
			return;
		min_x = (std::min)(min_x, sc.x);
		max_x = (std::max)(max_x, sc.x);
		min_y = (std::min)(min_y, sc.y);
		max_y = (std::max)(max_y, sc.y);
		any = true;
	};

	auto push_obb8 = [&](const Vector3 world[8]) {
		float cw[8];
		for (int i = 0; i < 8; ++i)
		{
			wmin.x = (std::min)(wmin.x, world[i].x); wmax.x = (std::max)(wmax.x, world[i].x);
			wmin.y = (std::min)(wmin.y, world[i].y); wmax.y = (std::max)(wmax.y, world[i].y);
			wmin.z = (std::min)(wmin.z, world[i].z); wmax.z = (std::max)(wmax.z, world[i].z);
			cw[i] = clip_w(world[i]);
			if (cw[i] >= 0.01f)
				push_screen(world[i]);
		}
		static const int k_edges[12][2] = {
			{ 0, 1 }, { 1, 3 }, { 3, 2 }, { 2, 0 },
			{ 4, 5 }, { 5, 7 }, { 7, 6 }, { 6, 4 },
			{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
		};
		for (const auto& e : k_edges)
		{
			const int a = e[0], b = e[1];
			const bool a_in = cw[a] >= 0.01f;
			const bool b_in = cw[b] >= 0.01f;
			if (a_in == b_in)
				continue;
			const float t = (0.01f - cw[a]) / (cw[b] - cw[a]);
			if (t < 0.f || t > 1.f)
				continue;
			push_screen({
				world[a].x + (world[b].x - world[a].x) * t,
				world[a].y + (world[b].y - world[a].y) * t,
				world[a].z + (world[b].z - world[a].z) * t,
			});
		}
	};

	auto make_world = [&](const Vector3& pos, const Matrix4x4& rot,
	                      float lx, float ly, float lz) -> Vector3 {
		return {
			pos.x + rot.m[0][0] * lx + rot.m[0][1] * ly + rot.m[0][2] * lz,
			pos.y + rot.m[1][0] * lx + rot.m[1][1] * ly + rot.m[1][2] * lz,
			pos.z + rot.m[2][0] * lx + rot.m[2][1] * ly + rot.m[2][2] * lz,
		};
	};

	auto push_part_obb = [&](const Vector3& pos, const Matrix4x4& rot, const Vector3& sz) {
		if (sz.x < 0.01f && sz.y < 0.01f && sz.z < 0.01f)
			return;
		const Vector3 h{ sz.x * 0.5f, sz.y * 0.5f, sz.z * 0.5f };
		Vector3 world[8];
		const Vector3 lc[8] = {
			{ -h.x, -h.y, -h.z }, { -h.x, -h.y,  h.z },
			{ -h.x,  h.y, -h.z }, { -h.x,  h.y,  h.z },
			{  h.x, -h.y, -h.z }, {  h.x, -h.y,  h.z },
			{  h.x,  h.y, -h.z }, {  h.x,  h.y,  h.z },
		};
		for (int i = 0; i < 8; ++i)
			world[i] = make_world(pos, rot, lc[i].x, lc[i].y, lc[i].z);
		push_obb8(world);
	};

	for (const auto& e : parts)
	{
		BasePart bp(e.part);
		Vector3 pos = bp.GetPosition();
		Vector3 sz = bp.GetSize();
		Matrix4x4 rot = bp.GetRotation();

		const bool is_acc =
			e.kind == MeshParser::Kind::Accessory ||
			e.kind == MeshParser::Kind::Hair ||
			e.kind == MeshParser::Kind::Face;

		ResolveResult rr = Resolve(e);
		auto mesh = LookupMesh(rr.mesh_id);
		const bool have = mesh && !mesh->vertices.empty();

		push_part_obb(pos, rot, sz);

		if (!have)
			continue;

		Vector3 ms = rr.scale;
		Vector3 off = rr.offset;
		(void)is_acc;
		ApplyVisualFit(e, rr, *mesh, ms, off, sz);

		const int n = (int)mesh->vertices.size();
		int step = n / 96;
		if (step < 1) step = 1;
		for (int i = 0; i < n; i += step)
		{
			const float* p = mesh->vertices[i].pos;
			if (!std::isfinite(p[0]) || !std::isfinite(p[1]) || !std::isfinite(p[2]))
				continue;
			const float lx = p[0] * ms.x + off.x;
			const float ly = p[1] * ms.y + off.y;
			const float lz = p[2] * ms.z + off.z;
			const Vector3 wc = make_world(pos, rot, lx, ly, lz);
			wmin.x = (std::min)(wmin.x, wc.x); wmax.x = (std::max)(wmax.x, wc.x);
			wmin.y = (std::min)(wmin.y, wc.y); wmax.y = (std::max)(wmax.y, wc.y);
			wmin.z = (std::min)(wmin.z, wc.z); wmax.z = (std::max)(wmax.z, wc.z);
			if (clip_w(wc) >= 0.01f)
				push_screen(wc);
		}
	}

	return any;
}

void Draw(
	ImDrawList* dl,
	std::uint64_t character,
	const Matrix4x4& view,
	const Vector2& viewport,
	float scale_x,
	float scale_y,
	ImU32 fill_col)
{
	if (!g_Memory.IsValid(character))
		return;

	const bool use_shader = MeshDxShader::IsFrameValid();
	const bool want_outline = !use_shader && variables::ESP::meshChamsOutline && dl != nullptr;
	const bool want_fill_imgui = !use_shader && dl != nullptr;
	if (!use_shader && !dl)
		return;

	Matrix4x4 live_view = view;

	const ULONGLONG now = GetTickCount64();

	struct PartsCache {
		ULONGLONG t{ 0 };
		std::vector<MeshParser::Entry> parts;
	};
	static std::unordered_map<std::uint64_t, PartsCache> s_parts;
	static ULONGLONG s_parts_gc = 0;
	if (now - s_parts_gc > 5000ull)
	{
		for (auto it = s_parts.begin(); it != s_parts.end(); )
		{
			if (now - it->second.t > 5000ull)
				it = s_parts.erase(it);
			else
				++it;
		}
		s_parts_gc = now;
	}

	const std::vector<MeshParser::Entry>* parts_ptr = nullptr;
	{
		auto& pc = s_parts[character];
		if (pc.parts.empty() || now - pc.t > 5000ull + (character % 1700ull))
		{
			pc.parts = MeshParser::CollectDrawable(character);
			pc.t = now;
		}
		parts_ptr = &pc.parts;
	}
	const auto& parts = *parts_ptr;
	if (parts.empty())
		return;

	struct CachedResolve {
		ResolveResult r;
		ULONGLONG t{ 0 };
	};


	static std::unordered_map<std::uint64_t, CachedResolve> s_resolve;
	static ULONGLONG s_cache_gc = 0;
	if (now - s_cache_gc > 10000ull)
	{
		for (auto it = s_resolve.begin(); it != s_resolve.end(); )
		{
			if (now - it->second.t > 10000ull)
				it = s_resolve.erase(it);
			else
				++it;
		}
		s_cache_gc = now;
	}

	ImDrawListFlags bak = 0;
	if (want_fill_imgui || want_outline)
	{
		bak = dl->Flags;
		if (want_fill_imgui)
			dl->Flags &= ~ImDrawListFlags_AntiAliasedFill;
	}

	static std::vector<ScreenTri> outline_tris;
	outline_tris.clear();
	if (want_outline && outline_tris.capacity() < 4096)
		outline_tris.reserve(4096);

	static std::unordered_map<FitCacheKey, FitCacheVal, FitCacheKeyHash> s_fit;
	static ULONGLONG s_fit_gc = 0;
	if (now - s_fit_gc > 3000ull)
	{
		s_fit.clear();
		s_fit_gc = now;
	}

	bool need_force_mcp = false;

	for (const auto& e : parts)
	{
		BasePart bp(e.part);
		Vector3 pos, sz;
		Matrix4x4 rot;
		if (!bp.GetFrameData(pos, rot, sz))
			continue;

		const bool is_acc =
			e.kind == MeshParser::Kind::Accessory ||
			e.kind == MeshParser::Kind::Hair ||
			e.kind == MeshParser::Kind::Face;

		ResolveResult rr;
		auto it = s_resolve.find(e.part);
		if (it != s_resolve.end() && now - it->second.t < 5000ull + (e.part % 900ull))
			rr = it->second.r;
		else
		{
			rr = Resolve(e);

			if (e.name == "Head")
			{
				std::string real;
				if (e.special_mesh)
					real = ReadMeshIdAt(e.special_mesh + Offsets::SpecialMesh::MeshId);
				else if (e.class_name == "MeshPart")
					real = ReadMeshIdAt(e.part + Offsets::MeshPart::MeshId);

				if (!real.empty() && MeshCache::Get().FindShared(real))
				{
					rr.mesh_id = real;
					if (e.special_mesh)
					{
						rr.fit_to_part = false;
						rr.is_special = true;
						rr.scale = g_Memory.Read<Vector3>(e.special_mesh + Offsets::SpecialMesh::Scale);
						rr.offset = g_Memory.Read<Vector3>(e.special_mesh + Offsets::SpecialMesh::Offset);
						auto clamp_sc = [](float& v) {
							if (!std::isfinite(v) || v < 1e-4f) v = 1.f;
							if (v > 50.f) v = 50.f;
						};
						clamp_sc(rr.scale.x);
						clamp_sc(rr.scale.y);
						clamp_sc(rr.scale.z);
					}
					else
					{
						rr.fit_to_part = true;
						rr.scale = { 1.f, 1.f, 1.f };
						rr.offset = { 0.f, 0.f, 0.f };
					}
				}
				else if (!real.empty())
				{
					need_force_mcp = true;
				}
			}

			if (!rr.mesh_id.empty())
				s_resolve[e.part] = { rr, now };
			else
				s_resolve.erase(e.part);
		}

		auto mesh = LookupMesh(rr.mesh_id);
		if (!mesh || mesh->faces.empty())
		{
			if (!rr.mesh_id.empty())
				need_force_mcp = true;

			mesh = LookupMesh("rbxasset://avatar/heads/head.mesh");
			if (e.name == "Head" && mesh && !mesh->faces.empty())
			{
				rr.mesh_id = "rbxasset://avatar/heads/head.mesh";
				rr.fit_to_part = false;
				rr.scale = { 1.f, 1.f, 1.f };
				rr.offset = { 0.f, 0.f, 0.f };
			}
			else
			{
				if (!is_acc && e.name != "Head")
				{
					if (use_shader)
						MeshDxShader::QueueBox(MakeBoxWorld(pos, rot, sz));
					else
					{
						DrawBoxFallback(dl, pos, rot, sz, live_view, viewport, scale_x, scale_y, fill_col);
					}
					if (want_outline)
					{
						PushOutlineFromPartBox(
							outline_tris, pos, rot, sz,
							live_view, viewport, scale_x, scale_y);
					}
				}
				continue;
			}
		}

		Vector3 ms = rr.scale;
		Vector3 off = rr.offset;
		const int fit_flags =
			(is_acc ? 1 : 0) | (rr.is_special ? 2 : 0) | (rr.fit_to_part ? 4 : 0);
		const FitCacheKey fk = MakeFitKey(e.part, rr.mesh_id, sz, rr.scale, fit_flags);
		if (auto fit_it = s_fit.find(fk); fit_it != s_fit.end())
		{
			ms = fit_it->second.ms;
			off = fit_it->second.off;
		}
		else
		{
			ApplyVisualFit(e, rr, *mesh, ms, off, sz);
			s_fit[fk] = { ms, off };
		}

		if (use_shader)
			MeshDxShader::QueueMesh(rr.mesh_id, MakeWorld(pos, rot, ms, off));

		if (!want_fill_imgui && !want_outline)
			continue;

		const int vtx_count = (int)mesh->vertices.size();
		if (vtx_count <= 0 || vtx_count > 50000)
			continue;

		const int fac_total = (int)mesh->faces.size();
		if (fac_total <= 0)
			continue;

		int fac_stride = 1;
		int fac_budget = want_fill_imgui ? 12000 : 1800;
		if (fac_total > fac_budget)
			fac_stride = (fac_total + fac_budget - 1) / fac_budget;




		static std::vector<char> need;
		static std::vector<ImVec2> screen;
		static std::vector<char> ok;
		need.assign((std::size_t)vtx_count, 0);
		for (int i = 0; i < fac_total; i += fac_stride)
		{
			const auto& f = mesh->faces[i];
			if (f.indices[0] < (std::uint32_t)vtx_count) need[f.indices[0]] = 1;
			if (f.indices[1] < (std::uint32_t)vtx_count) need[f.indices[1]] = 1;
			if (f.indices[2] < (std::uint32_t)vtx_count) need[f.indices[2]] = 1;
		}

		screen.resize((std::size_t)vtx_count);
		ok.assign((std::size_t)vtx_count, 0);
		const float ox = viewport.x * scale_x;
		const float oy = viewport.y * scale_y;

		for (int i = 0; i < vtx_count; ++i)
		{
			if (!need[i])
				continue;
			const float* p = mesh->vertices[i].pos;
			float lx = p[0] * ms.x + off.x;
			float ly = p[1] * ms.y + off.y;
			float lz = p[2] * ms.z + off.z;
			Vector3 wc{
				pos.x + rot.m[0][0] * lx + rot.m[0][1] * ly + rot.m[0][2] * lz,
				pos.y + rot.m[1][0] * lx + rot.m[1][1] * ly + rot.m[1][2] * lz,
				pos.z + rot.m[2][0] * lx + rot.m[2][1] * ly + rot.m[2][2] * lz,
			};
			Vector2 sc;
			if (!W2S(live_view, viewport, scale_x, scale_y, wc, sc))
				continue;
			if (sc.x < -ox || sc.x > ox * 2.f || sc.y < -oy || sc.y > oy * 2.f)
				continue;
			screen[i] = ImVec2(sc.x, sc.y);
			ok[i] = 1;
		}

		for (int i = 0; i < fac_total; i += fac_stride)
		{
			const std::uint32_t i0 = mesh->faces[i].indices[0];
			const std::uint32_t i1 = mesh->faces[i].indices[1];
			const std::uint32_t i2 = mesh->faces[i].indices[2];
			if (i0 >= (std::uint32_t)vtx_count || i1 >= (std::uint32_t)vtx_count ||
			    i2 >= (std::uint32_t)vtx_count)
				continue;
			if (!ok[i0] || !ok[i1] || !ok[i2])
				continue;

		ImVec2 a = screen[i0], b = screen[i1], c = screen[i2];
		if (want_fill_imgui)
			dl->AddTriangleFilled(a, b, c, fill_col);
			if (want_outline)
				outline_tris.push_back({ a, b, c });
		}
	}

	if (want_outline && !outline_tris.empty())
	{
		const float* oc = reinterpret_cast<const float*>(&variables::ESP::meshChamsOutlineColor);
		DrawCharacterContour(
			dl, outline_tris,
			oc[0], oc[1], oc[2], oc[3],
			variables::ESP::meshChamsOutlineStyle);
	}

	if (need_force_mcp)
	{
		static ULONGLONG s_force_mcp = 0;
		if (now - s_force_mcp > 400ull)
		{
			s_force_mcp = now;
			MeshCache::Get().Refresh(true);
		}
	}

	if ((want_fill_imgui || want_outline) && dl)
		dl->Flags = bak;
}

}
}
}
