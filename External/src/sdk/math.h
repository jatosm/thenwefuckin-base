// discord.gg/thenwefuckin
#pragma once
#include <windows.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <limits>
#include <string>

namespace rbx {
	struct vector2_t {
		float x, y;

		vector2_t() : x(0.f), y(0.f) {}
		vector2_t(float x, float y) : x(x), y(y) {}

		vector2_t operator+(const vector2_t& other) const {
			return vector2_t(x + other.x, y + other.y);
		}

		vector2_t operator-(const vector2_t& other) const {
			return vector2_t(x - other.x, y - other.y);
		}

		vector2_t operator*(float scalar) const {
			return vector2_t(x * scalar, y * scalar);
		}

		vector2_t operator/(float scalar) const {
			return vector2_t(x / scalar, y / scalar);
		}

		bool operator<(const vector2_t& other) const {
			return (x < other.x) || (x == other.x && y < other.y);
		}

		bool operator>(const vector2_t& other) const {
			return (x > other.x) || (x == other.x && y > other.y);
		}

		vector2_t& operator+=(const vector2_t& other) {
			x += other.x;
			y += other.y;
			return *this;
		}

		vector2_t& operator-=(const vector2_t& other) {
			x -= other.x;
			y -= other.y;
			return *this;
		}

		vector2_t& operator*=(float scalar) {
			x *= scalar;
			y *= scalar;
			return *this;
		}

		vector2_t& operator/=(float scalar) {
			x /= scalar;
			y /= scalar;
			return *this;
		}

		float dot(const vector2_t& other) const {
			return x * other.x + y * other.y;
		}

		float magnitude() const {
			return sqrtf(x * x + y * y);
		}

		vector2_t normalize() const {
			float mag = magnitude();
			return mag > 0.f ? *this * (1.f / mag) : vector2_t(0.f, 0.f);
		}

		float distance(const vector2_t& other) const {
			float dx = x - other.x;
			float dy = y - other.y;
			return sqrtf(dx * dx + dy * dy);
		}

		vector2_t lerp(const vector2_t& other, float t) const {
			return *this + (other - *this) * t;
		}
	};

	struct vector2_t_int16 {
		int16_t x, y;
		vector2_t_int16() : x(0), y(0) {}
		vector2_t_int16(int16_t x, int16_t y) : x(x), y(y) {}

		static int16_t cast_int16(float value) {
			auto min_val = (float)std::numeric_limits<int16_t>::lowest();
			auto max_val = (float)(std::numeric_limits<int16_t>::max)();
			value = std::clamp(value, min_val, max_val);
			return (int16_t)std::lround(value);
		}

		vector2_t_int16 operator+(const vector2_t_int16& other) const {
			return vector2_t_int16(x + other.x, y + other.y);
		}

		vector2_t_int16 operator-(const vector2_t_int16& other) const {
			return vector2_t_int16(x - other.x, y - other.y);
		}

		vector2_t_int16 operator*(const vector2_t_int16& other) const {
			return vector2_t_int16(x * other.x, y * other.y);
		}

		vector2_t_int16 operator/(const vector2_t_int16& other) const {
			return vector2_t_int16(x / other.x, y / other.y);
		}

		vector2_t_int16 operator*(float scalar) const {
			return vector2_t_int16(cast_int16(x * scalar), cast_int16(y * scalar));
		}

		vector2_t_int16 operator/(float scalar) const {
			return vector2_t_int16(cast_int16(x / scalar), cast_int16(y / scalar));
		}
	};

	struct vector3_t {
		float x, y, z;

		vector3_t() : x(0.f), y(0.f), z(0.f) {}
		vector3_t(float x, float y, float z) : x(x), y(y), z(z) {}

		vector3_t operator+(const vector3_t& other) const {
			return vector3_t(x + other.x, y + other.y, z + other.z);
		}

		vector3_t operator-(const vector3_t& other) const {
			return vector3_t(x - other.x, y - other.y, z - other.z);
		}

		vector3_t operator*(float scalar) const {
			return vector3_t(x * scalar, y * scalar, z * scalar);
		}

		vector3_t operator/(float scalar) const {
			return vector3_t(x / scalar, y / scalar, z / scalar);
		}

		vector3_t& operator+=(const vector3_t& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		vector3_t& operator-=(const vector3_t& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		vector3_t& operator*=(float scalar) {
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		vector3_t& operator/=(float scalar) {
			x /= scalar;
			y /= scalar;
			z /= scalar;
			return *this;
		}

		float dot(const vector3_t& other) const {
			return x * other.x + y * other.y + z * other.z;
		}

		vector3_t cross(const vector3_t& other) const {
			return vector3_t(
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x
			);
		}

		float magnitude() const {
			return sqrtf(x * x + y * y + z * z);
		}

		vector3_t normalize() const {
			float mag = magnitude();
			return mag > 0.f ? *this * (1.f / mag) : vector3_t(0.f, 0.f, 0.f);
		}

		float distance(const vector3_t& other) const {
			float dx = x - other.x;
			float dy = y - other.y;
			float dz = z - other.z;
			return sqrtf(dx * dx + dy * dy + dz * dz);
		}

		vector3_t lerp(const vector3_t& other, float t) const {
			return *this + (other - *this) * t;
		}
	};

	struct vector4_t {
		float x, y, z, w;

		vector4_t() : x(0.f), y(0.f), z(0.f), w(0.f) {}
		vector4_t(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

		vector4_t operator+(const vector4_t& other) const {
			return vector4_t(x + other.x, y + other.y, z + other.z, w + other.w);
		}

		vector4_t operator-(const vector4_t& other) const {
			return vector4_t(x - other.x, y - other.y, z - other.z, w - other.w);
		}

		vector4_t operator*(float scalar) const {
			return vector4_t(x * scalar, y * scalar, z * scalar, w * scalar);
		}

		vector4_t operator/(float scalar) const {
			return vector4_t(x / scalar, y / scalar, z / scalar, w / scalar);
		}

		vector4_t& operator+=(const vector4_t& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}

		vector4_t& operator-=(const vector4_t& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
			return *this;
		}

		vector4_t& operator*=(float scalar) {
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}

		vector4_t& operator/=(float scalar) {
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
			return *this;
		}

		float dot(const vector4_t& other) const {
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		float magnitude() const {
			return sqrtf(x * x + y * y + z * z + w * w);
		}

		vector4_t normalize() const {
			float mag = magnitude();
			return mag > 0.f ? *this * (1.f / mag) : vector4_t(0.f, 0.f, 0.f, 0.f);
		}

		float distance(const vector4_t& other) const {
			float dx = x - other.x;
			float dy = y - other.y;
			float dz = z - other.z;
			float dw = w - other.w;
			return sqrtf(dx * dx + dy * dy + dz * dz + dw * dw);
		}

		vector4_t lerp(const vector4_t& other, float t) const {
			return *this + (other - *this) * t;
		}
	};

	struct matrix3_t {
		float data[9];

		matrix3_t() {
			for (int i = 0; i < 9; ++i) data[i] = 0.f;
		}

		matrix3_t operator+(const matrix3_t& other) const {
			matrix3_t result;
			for (int i = 0; i < 9; ++i) result.data[i] = data[i] + other.data[i];
			return result;
		}

		matrix3_t operator-(const matrix3_t& other) const {
			matrix3_t result;
			for (int i = 0; i < 9; ++i) result.data[i] = data[i] - other.data[i];
			return result;
		}

		matrix3_t operator*(const matrix3_t& other) const {
			matrix3_t result;
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					for (int k = 0; k < 3; ++k)
						result.data[i * 3 + j] += data[i * 3 + k] * other.data[k * 3 + j];
			return result;
		}

		matrix3_t operator*(float scalar) const {
			matrix3_t result;
			for (int i = 0; i < 9; ++i) result.data[i] = data[i] * scalar;
			return result;
		}

		vector3_t operator*(const vector3_t& v) const {
			return vector3_t(
				data[0] * v.x + data[1] * v.y + data[2] * v.z,
				data[3] * v.x + data[4] * v.y + data[5] * v.z,
				data[6] * v.x + data[7] * v.y + data[8] * v.z
			);
		}

		matrix3_t lerp(const matrix3_t& other, float t) const {
			matrix3_t res;
			for (int i = 0; i < 9; ++i) {
				res.data[i] = data[i] + (other.data[i] - data[i]) * t;
			}
			return res;
		}

		matrix3_t lerp_smooth(const matrix3_t& other, float x, float y) const {
			matrix3_t res;
			for (int i = 0; i < 9; ++i) {
				if (i % 3 == 0)
					res.data[i] = data[i] + (other.data[i] - data[i]) * x;
				else
					res.data[i] = data[i] + (other.data[i] - data[i]) * y;
			}
			return res;
		}

		static matrix3_t from_axis_angle(const vector3_t& axis, float angle) {
			matrix3_t res;
			auto c = cosf(angle);
			auto s = sinf(angle);
			auto t = 1.f - c;
			auto x = axis.x;
			auto y = axis.y;
			auto z = axis.z;
			res.data[0] = t * x * x + c;
			res.data[1] = t * x * y - s * z;
			res.data[2] = t * x * z + s * y;
			res.data[3] = t * x * y + s * z;
			res.data[4] = t * y * y + c;
			res.data[5] = t * y * z - s * x;
			res.data[6] = t * x * z - s * y;
			res.data[7] = t * y * z + s * x;
			res.data[8] = t * z * z + c;
			return res;
		}
	};

	struct matrix4_t {
		float data[16];

		matrix4_t() {
			for (int i = 0; i < 16; ++i) data[i] = 0.f;
		}

		matrix4_t operator+(const matrix4_t& other) const {
			matrix4_t result;
			for (int i = 0; i < 16; ++i) result.data[i] = data[i] + other.data[i];
			return result;
		}

		matrix4_t operator-(const matrix4_t& other) const {
			matrix4_t result;
			for (int i = 0; i < 16; ++i) result.data[i] = data[i] - other.data[i];
			return result;
		}

		matrix4_t operator*(const matrix4_t& other) const {
			matrix4_t result;
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					for (int k = 0; k < 4; ++k)
						result.data[i * 4 + j] += data[i * 4 + k] * other.data[k * 4 + j];
			return result;
		}

		matrix4_t operator*(float scalar) const {
			matrix4_t result;
			for (int i = 0; i < 16; ++i) result.data[i] = data[i] * scalar;
			return result;
		}

		vector3_t operator*(const vector3_t& v) const {
			float x = data[0] * v.x + data[1] * v.y + data[2] * v.z + data[3];
			float y = data[4] * v.x + data[5] * v.y + data[6] * v.z + data[7];
			float z = data[8] * v.x + data[9] * v.y + data[10] * v.z + data[11];
			return vector3_t(x, y, z);
		}

		vector4_t operator*(const vector4_t& v) const {
			return vector4_t(
				data[0] * v.x + data[1] * v.y + data[2] * v.z + data[3] * v.w,
				data[4] * v.x + data[5] * v.y + data[6] * v.z + data[7] * v.w,
				data[8] * v.x + data[9] * v.y + data[10] * v.z + data[11] * v.w,
				data[12] * v.x + data[13] * v.y + data[14] * v.z + data[15] * v.w
			);
		}
	};

	struct color3_t {
		float r, g, b;

		color3_t operator*(const color3_t& v) const {
			float rr = r * v.r;
			float gg = g * v.g;
			float bb = b * v.b;
			return color3_t(rr, gg, bb);
		}

		color3_t operator*(const float& v) const {
			float rr = r * v;
			float gg = g * v;
			float bb = b * v;
			return color3_t(rr, gg, bb);
		}

		color3_t(float rd = 0.f, float gr = 0.f, float bl = 0.f) : r(rd), g(gr), b(bl) {}

		static color3_t from_rgb(float r, float g, float b) {
			return color3_t(r / 255.f, g / 255.f, b / 255.f);
		}

		static color3_t to_rgb(float r, float g, float b) {
			return color3_t(r * 255.f, g * 255.f, b * 255.f);
		}

		static color3_t from_hex(std::string hex) {
			auto temp = hex;
			if (temp[0] == '#') temp = temp.substr(1);
			if (temp.length() != 6) return color3_t();
			auto r = stof(temp.substr(0, 2));
			auto g = stof(temp.substr(2, 2));
			auto b = stof(temp.substr(4, 2));
			return from_rgb(r, g, b);
		}

		std::string to_hex() {
			char hex[8];
			auto rd = (int)(round(std::clamp(r, 0.f, 1.f) * 255.f));
			auto gr = (int)(round(std::clamp(g, 0.f, 1.f) * 255.f));
			auto bl = (int)(round(std::clamp(b, 0.f, 1.f) * 255.f));
			std::snprintf(hex, sizeof(hex), "#%02x%02x%02x", rd, gr, bl);
			return std::string(hex);
		}
	};

	struct cframe_t {
		matrix3_t rotation = matrix3_t();
		vector3_t position = vector3_t();
	};

	inline vector3_t multiply(const matrix3_t& m, const vector3_t& v) {
		return {
			m.data[0] * v.x + m.data[1] * v.y + m.data[2] * v.z,
			m.data[3] * v.x + m.data[4] * v.y + m.data[5] * v.z,
			m.data[6] * v.x + m.data[7] * v.y + m.data[8] * v.z
		};
	}

	struct udim_t {
		float scale;
		int offset;

		bool equals(const udim_t& other, float epsilon = 0.01f) const {
			return abs(scale - other.scale) < epsilon && offset == other.offset;
		}

		bool operator==(const udim_t& other) const { return equals(other); }
	};

	struct udim2_t {
		udim_t x;
		udim_t y;

		bool equals(const udim2_t& other, float epsilon = 0.01f) const {
			return x.equals(other.x, epsilon) && y.equals(other.y, epsilon);
		}

		bool operator==(const udim2_t& other) const { return equals(other); }
	};

	inline float cross(const vector2_t& a, const vector2_t& b, const vector2_t& c) {
		return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	}

	inline std::vector<vector2_t> convex_hull(std::vector<vector2_t> points) {
		if (points.size() <= 1) return points;
		std::sort(points.begin(), points.end(),
			[](const vector2_t& a, const vector2_t& b) {
				return a.x < b.x || (a.x == b.x && a.y < b.y);
			});
		std::vector<vector2_t> hull;
		for (const auto& p : points) {
			while (hull.size() >= 2 &&
				cross(hull[hull.size() - 2], hull.back(), p) <= 0)
				hull.pop_back();
			hull.push_back(p);
		}
		size_t lower_size = hull.size();
		for (int i = (int)points.size() - 2; i >= 0; --i) {
			while (hull.size() > lower_size &&
				cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0)
				hull.pop_back();
			hull.push_back(points[i]);
		}
		hull.pop_back();
		return hull;
	}
}
