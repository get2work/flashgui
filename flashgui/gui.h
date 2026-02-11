#pragma once
#include "renderer.h"
#include <DirectXMath.h>

class c_window {
public:
	c_window(const std::string& title, const DirectX::XMFLOAT2& pos, const DirectX::XMFLOAT2& size) : m_title(title), m_pos(pos), m_size(size)  {}
	~c_window() = default;

	DirectX::XMFLOAT2 get_pos() const { return m_pos; }
	DirectX::XMFLOAT2 get_size() const { return m_size; }
	std::string get_title() const { return m_title; }

private:
	std::string m_title;
	DirectX::XMFLOAT2 m_pos;
	DirectX::XMFLOAT2 m_size;
};

class c_gui
{
public:
	c_gui() = default;
	~c_gui() = default;

	c_window create_window(const std::string& title, int width, int height, int posx, int posy);

private:

};

