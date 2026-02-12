#pragma once
#include "renderer.h"
#include <DirectXMath.h>
#include "vec2.h"

namespace fgui {

	class c_checkbox {

	};

	class c_slider {

	};

	class c_textbox {
	};

	class c_button {
	};

	class c_combobox {
	};

	class c_multibox {

	};

	class c_group {
	public:
		c_group(const vec2i& pos, const vec2i& size) : m_pos(pos), m_size(size) {}
		~c_group() = default;

		vec2i get_pos() const { return m_pos; }
		vec2i get_size() const { return m_size; }

	private:
		vec2i m_pos;
		vec2i m_size;
	};

	class c_window {
	public:
		c_window(const std::string& title, const vec2i& pos, const vec2i& size) : m_title(title), m_pos(pos), m_size(size)  {}
		~c_window() = default;

		vec2i get_pos() const { return m_pos; }
		vec2i get_size() const { return m_size; }
		std::string get_title() const { return m_title; }

	private:
		std::string m_title;
		vec2i m_pos;
		vec2i m_size;
	};

	class c_gui
	{
	public:
		c_gui() = default;
		~c_gui() = default;

		//c_window create_window(const std::string& title, int width, int height, int posx, int posy);

	private:

	};

} // namespace fgui
