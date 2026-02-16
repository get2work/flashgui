#pragma once
#include "renderer.h"
#include <DirectXMath.h>
#include "vec2.h"

namespace fgui {

	enum m_item {
		check,
		slider,
		combo,
		keybind,
		multi,
		button,
		list,
		color
	};

	enum interaction_types {
		itt_null,
		itt_dragwin,
		itt_check,
		itt_slider,
		itt_combo,
		itt_wait,
		itt_tab
	};

	inline std::array<std::string, 166> keynames{
		"none",
		"m1",
		"m2",
		"cl",
		"m3",
		"m4",
		"m5",
		"?",
		"bk",
		"tab",
		"?",
		"?",
		"clr",
		"rtn",
		"?",
		"?",
		"shft",
		"ctrl",
		"menu",
		"pause",
		"capital",
		"kana",
		"?",
		"juna",
		"final",
		"kanji",
		"?",
		"escape",
		"convert",
		"nonconvert",
		"accept",
		"modechange",
		"space",
		"prior",
		"next",
		"end",
		"home",
		"left",
		"up",
		"right",
		"down",
		"select",
		"print",
		"execute",
		"snapshot",
		"insert",
		"delete",
		"help",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"?",
		"?",
		"?",
		"?",
		"?",
		"?",
		"?",
		"a",
		"b",
		"c",
		"d",
		"e",
		"f",
		"g",
		"h",
		"i",
		"j",
		"k",
		"l",
		"m",
		"n",
		"o",
		"p",
		"q",
		"r",
		"s",
		"t",
		"u",
		"v",
		"w",
		"x",
		"y",
		"z",
		"lwin",
		"rwin",
		"apps",
		"unknown",
		"sleep",
		"numpad0",
		"numpad1",
		"numpad2",
		"numpad3",
		"numpad4",
		"numpad5",
		"numpad6",
		"numpad7",
		"numpad8",
		"numpad9",
		"multiply",
		"add",
		"separator",
		"subtract",
		"decimal",
		"divide",
		"f1",
		"f2",
		"f3",
		"f4",
		"f5",
		"f6",
		"f7",
		"f8",
		"f9",
		"f10",
		"f11",
		"f12",
		"f13",
		"f14",
		"f15",
		"f16",
		"f17",
		"f18",
		"f19",
		"f20",
		"f21",
		"f22",
		"f23",
		"f24",
		"?",
		"?",
		"?",
		"?",
		"?",
		"?",
		"?",
		"?",
		"numlock",
		"scroll",
		"oem_nec_equal",
		"oem_fj_masshou",
		"oem_fj_touroku",
		"oem_fj_loya",
		"oem_fj_roya",
		"?",
		"?",
		"?",
		"?",
		"?",
		"?",
		"?",
		"?",
		"?",
		"lshift",
		"rshift",
		"lcontrol",
		"rcontrol",
		"lmenu",
		"rmenu"
	};

	class c_element
	{
	public:

		int _item_type = 0;
		vec2i _pos{};

		std::string _label = "";
		uintptr_t _value = NULL;
		DirectX::XMFLOAT4 _color = { 0.f, 0.f, 1.f, 1.f };

		bool _active = false;

		float _max = 0.f;
		float _min = 0.f;
		int _size = 0;
		int _height = 15;

		std::string _valuetext = "";
		std::vector<std::string> _options;
	};

	class c_checkbox : public c_element {
	public:
		c_checkbox(const std::string& label, bool* value) {
			_label = label;
			_item_type = check;
			_value = (uintptr_t)value;
			_active = *(bool*)value;
			_height = 15;
		}
	};

	class c_slider : public c_element {
	public:
		c_slider(const std::string& label, float* value, const float& min, const float& max) {
			_label = label;
			_item_type = slider;
			_value = (uintptr_t)value;
			_valuetext = ": (" + std::format("{:.2f}", *value) + ")";
			_active = false;
			_max = max;
			_min = min;
			_size = std::min(100, std::max(0, int(100 * static_cast<int>(*value / max))));
			_height = 30;
		}
	};

	class c_combo : public c_element {
	public:
		c_combo(const std::string& label, int* value, const std::vector<std::string>& options) {
			_label = label;
			_item_type = combo;
			_value = (uintptr_t)value;
			_valuetext = options[*value];
			_active = false;
			_size = (static_cast<int>(options.size()) * 16);
			_options = options;
			_height = 39;
		}
	};

	class c_keybind : public c_element {
	public:
		c_keybind(const std::string& label, int* value, bool&& embed) {
			_label = label;
			_item_type = keybind;
			_value = (uintptr_t)value;
			_valuetext = keynames[*value];
			_active = false;
			_height = 39;
		}
	};

	class c_group
	{
	public:
		c_group(const std::string& label) {
			_label = label;
		}
		std::string _label;
		vec2i _size{};
		vec2i _pos{};
		std::vector<c_element> elements;
	};

	class c_tab
	{
	public:
		c_tab(const std::string& label, const vec2i& pos, const vec2i& win_size) {
			_label = label;
			_pos = pos;
			_space = win_size;
		}
		std::string _label = "";
		vec2i _pos{};
		vec2i _size{};
		vec2i _space{};
		DirectX::XMFLOAT4 _color = { 1.f, 1.f, 1.f, 1.f };
		std::vector<c_group> groups;
	};

	class c_window
	{
	public:
		c_window(const std::string& in_title, const vec2i& in_size = vec2i(500, 600)) {
			title = split_title(in_title);
			size = in_size;
		}

		void draw();
		void draw_group(const c_group& group);
		void draw_element(const c_element& element, const vec2i& cur_pos);
		void update();

		bool mouse_in_region(const vec2i& item_pos, const vec2i& item_size);

		inline int start_tab(const std::string& label) {

			const auto& center = ((size.y + 75.f) / 2.f) - (tabs.size() * 30);
			const auto& tab = c_tab(label, vec2i(50, static_cast<int>(center)), vec2i(size.x - 116, size.y - 22));

			tabs.push_back(tab);
			return static_cast<int>(tabs.size()) - 1;
		}

		inline int start_group(const int& tab, const std::string& label) {
			tabs[tab].groups.push_back(c_group(label));
			return static_cast<int>(tabs[tab].groups.size()) - 1;
		}

		inline void add_checkbox(const int& tab, const int& group, const std::string& label, bool* value) {
			tabs[tab].groups[group].elements.push_back(c_checkbox(label, value));
		}

		inline void add_slider(const int& tab, const int& group, const std::string& label, float* value, const float& min, const float& max) {
			tabs[tab].groups[group].elements.push_back(c_slider(label, value, min, max));
		}

		inline void add_combo(const int& tab, const int& group, const std::string& label, int* value, const std::vector<std::string>& options) {
			tabs[tab].groups[group].elements.push_back(c_combo(label, value, options));
		}

		inline void add_keybind(const int& tab, const int& group, const std::string& label, int* value, bool&& embed) {
			tabs[tab].groups[group].elements.push_back(c_keybind(label, value, std::move(embed)));
		}

		inline void end_group(const int& tab, const int& g) {
			auto last_pos = tabs[tab].groups[g]._pos + vec2i(10, 30);

			for (int i = 0; i < (int)tabs[tab].groups[g].elements.size(); i++) {

				tabs[tab].groups[g].elements[i]._pos = last_pos;

				last_pos += vec2i(0, static_cast<int>(tabs[tab].groups[g].elements[i]._height));
			}
		}

		inline void end_tab(int tab) {
			for (int i = 0; i < (int)tabs[tab].groups.size(); i++) {
				tabs[tab].groups[i]._pos = vec2i(
					105 + ((i + 2) % 2) * (tabs[tab]._space.x / 2),
					(i > 1 ? tabs[tab]._space.y / 2 : 0) + 10
				);
				tabs[tab].groups[i]._size = vec2i(
					(tabs[tab].groups.size() > 1 ? tabs[tab]._space.x / 2 : tabs[tab]._space.x),
					(tabs[tab].groups.size() > 2 ? tabs[tab]._space.y / 2 : tabs[tab]._space.y)
				);
			}
		}

		int active_tab = 0;
		DirectX::XMFLOAT4 bg = { 0.05f, 0.05f, 0.08f, 1.f };
		DirectX::XMFLOAT4 accent = { 0.f, 0.f, 1.f, 1.f };
		DirectX::XMFLOAT4 active_accent = { 0.678f, 0.847f, 0.902f, 1.f };

		int interaction_type = itt_null;
		uintptr_t active_element = NULL;
		bool dragging = false;
		bool l_click = false;
		bool r_click = false;
		bool can_drag = false;

		bool key_wait = false;

		vec2i pos = vec2i(50, 50);
		vec2i size = vec2i(500, 600);
		vec2i mouse = vec2i(0, 0);
		vec2i old_mouse = vec2i(0, 0);

		std::vector<c_tab> tabs;

		std::array<std::string, 2> title = { "", "" };

		inline void set_active(int tab) {
			tabs[tab]._color = accent;
		}

	private:
		c_element combo_to_overlay;
		vec2i ovrlaypos{};

		std::array<std::string, 2> split_title(std::string title);
		void update_elements();
		void function_group(c_group& group);
	};

} // namespace fgui
