#pragma once

#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <functional>
#include <format>
#include <map>
#include <set>
#include <string>
#include <vector>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace fgui::ui {

using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using float32 = float;
using float64 = double;

using std::max;
using std::min;

inline std::string ui_to_lower(std::string text) {
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return text;
}

struct vec2 {
	float64 x = 0.0;
	float64 y = 0.0;

	vec2() = default;
	vec2(float64 x_value, float64 y_value) : x(x_value), y(y_value) {}
};

struct color {
	uint8 r = 0;
	uint8 g = 0;
	uint8 b = 0;
	uint8 a = 255;

	color() = default;
	color(int red, int green, int blue, int alpha = 255)
		: r(static_cast<uint8>(red)), g(static_cast<uint8>(green)),
		  b(static_cast<uint8>(blue)), a(static_cast<uint8>(alpha)) {}

	color lerp(const color& other, float64 amount) const {
		const float64 t = std::clamp(amount, 0.0, 1.0);
		return color(
			static_cast<int>(std::lround(static_cast<float64>(r) + (other.r - r) * t)),
			static_cast<int>(std::lround(static_cast<float64>(g) + (other.g - g) * t)),
			static_cast<int>(std::lround(static_cast<float64>(b) + (other.b - b) * t)),
			static_cast<int>(std::lround(static_cast<float64>(a) + (other.a - a) * t)));
	}

	color with_alpha(uint8 alpha) const {
		color result = *this;
		result.a = alpha;
		return result;
	}

	bool operator==(const color&) const = default;
};

namespace vk {
inline constexpr int64 a = 'A';
inline constexpr int64 backspace = VK_BACK;
inline constexpr int64 decimal = VK_DECIMAL;
inline constexpr int64 enter = VK_RETURN;
inline constexpr int64 escape = VK_ESCAPE;
inline constexpr int64 k0 = '0';
inline constexpr int64 k9 = '9';
inline constexpr int64 lbutton = VK_LBUTTON;
inline constexpr int64 mbutton = VK_MBUTTON;
inline constexpr int64 numpad0 = VK_NUMPAD0;
inline constexpr int64 numpad9 = VK_NUMPAD9;
inline constexpr int64 rbutton = VK_RBUTTON;
inline constexpr int64 space = VK_SPACE;
inline constexpr int64 subtract = VK_SUBTRACT;
inline constexpr int64 xbutton1 = VK_XBUTTON1;
inline constexpr int64 xbutton2 = VK_XBUTTON2;
inline constexpr int64 z = 'Z';
}

}
