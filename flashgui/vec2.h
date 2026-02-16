#pragma once

#include <DirectXMath.h>

namespace fgui {

	class vec2i {
	public:
		int x, y;
		vec2i() : x(0), y(0) {}
		vec2i(const int& x, const int& y) : x(x), y(y) {}
		vec2i(const float& x, const float& y) : x(static_cast<int>(x)), y(static_cast<int>(y)) {}
		vec2i(const DirectX::XMFLOAT2& vec) : x(static_cast<int>(vec.x)), y(static_cast<int>(vec.y)) {}
		vec2i(const DirectX::XMINT2& vec) : x(vec.x), y(vec.y) {}
		vec2i(const vec2i&) = default;

		vec2i& operator=(const vec2i&) = default;

		//make fully compatible with directx math types
		operator DirectX::XMFLOAT2() const {
			return DirectX::XMFLOAT2(static_cast<float>(x), static_cast<float>(y));
		}
		operator DirectX::XMINT2() const {
			return DirectX::XMINT2(x, y);
		}

	};

	class vec2f {
	public:
		float x, y;
		vec2f() : x(0), y(0) {}
		vec2f(const int& x, const int& y) : x(static_cast<float>(x)), y(static_cast<float>(y)) {}
		vec2f(const float& x, const float& y) : x(x), y(y) {}
		vec2f(const DirectX::XMFLOAT2& vec) : x(vec.x), y(vec.y) {}
		vec2f(const DirectX::XMINT2& vec) : x(static_cast<float>(vec.x)), y(static_cast<float>(vec.y)) {}
		vec2f(const vec2f&) = default;
		vec2f(const vec2i& vec) : x(static_cast<float>(vec.x)), y(static_cast<float>(vec.y)) {}

		vec2f& operator=(const vec2f&) = default;

		//make fully compatible with directx math types
		operator DirectX::XMFLOAT2() const {
			return DirectX::XMFLOAT2(x, y);
		}

		operator vec2i() const {
			//return rounded
			return vec2i(static_cast<int>(x + 0.5f), static_cast<int>(y + 0.5f));
		}

		operator DirectX::XMINT2() const {
			return DirectX::XMINT2(static_cast<int>(x), static_cast<int>(y));
		}

	};
}