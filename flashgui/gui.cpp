#include "pch.h"
#include "gui.h"
#include "include/flashgui.h"

using namespace fgui;

void c_window::update() {
	//if (!menu::keys[kmenu_open].val)
	//	return;

	POINT p;
	GetCursorPos(&p);
	ScreenToClient(process->window.handle, &p);

	old_mouse = mouse;
	mouse = vec2i(static_cast<int>(p.x), static_cast<int>(p.y));

	l_click = GetAsyncKeyState(VK_LBUTTON);
	r_click = GetAsyncKeyState(VK_RBUTTON);

	if (!dragging) {
		update_elements();
	}

	can_drag = interaction_type == itt_null;

	if (l_click && mouse_in_region(pos, size) && can_drag)
		dragging = true;
	else if (!l_click)
		dragging = false;

	if (dragging) {
		auto delta = mouse - old_mouse;
		pos += delta;
	}

	pos.clamp_to_screen(process->window.get_size(), size);
}


void c_window::draw() {
	//if (!menu::keys[kmenu_open].val)
	//	return;

	//draw window
	render->draw_quad_outline(pos, size, accent);
	render->draw_quad(pos + 1, size - 1, bg);
	render->draw_quad_outline(pos + 5, size - 11, accent);

	//title

	render->draw_text(title[0], vec2i(pos.x + 50, pos.y + 28), 0, {1.f, 1.f, 1.f, 1.f});
	render->draw_text(title[1], vec2i(pos.x + 50, pos.y + 48), 0, accent);

	//diagonal line
	render->draw_line(vec2i(pos.x + 100, pos.y + 8), vec2i(pos.x + 100, pos.y + size.y - 9), accent);

	//horizontal line
	render->draw_line(vec2i(pos.x + 10, pos.y + 75), vec2i(pos.x + 95, pos.y + 75), accent);

	//draw tabs
	for (const auto& tab : tabs) {
		render->draw_text(tab._label, pos + tab._pos + vec2i(0, 8), 0, tab._color);
	}

	if (tabs[active_tab].groups.empty())
		return;

	const auto& groups = tabs[active_tab].groups;

	//pipeline that shit
	for (size_t i = 1; i < groups.size() + 1; i++) {
		draw_group(groups[i - 1]);

		if (i < groups.size())
			draw_group(groups[i]);

		i++;
	}

	if (ovrlaypos.x) {
		render->draw_quad_outline(ovrlaypos + vec2i(0, 31), vec2i(100, combo_to_overlay._size), combo_to_overlay._color);
		render->draw_quad(ovrlaypos + vec2i(1, 32), vec2i(99, combo_to_overlay._size - 1), {0.04f, 0.04f, 0.04f, 1.f});
		for (size_t i = 0; i < combo_to_overlay._options.size(); i++) {
			render->draw_text(combo_to_overlay._options[i], ovrlaypos + vec2i(5, 33 + 8 + (static_cast<int>(i) * 16)), 0, {1.f ,1.f, 1.f, 1.f});
		}
		ovrlaypos.x = 0;
	}
}

void c_window::draw_group(const c_group& group) {
	render->draw_quad_outline(pos + group._pos, group._size, accent);
	render->draw_line(vec2i(pos + group._pos) + vec2i(5, 20),
		pos + group._pos + vec2i(group._size.x - 5, 20),
		accent
	);
	render->draw_text(group._label, pos + group._pos + vec2i(group._size.x / 2, 5), 0, {1.f, 1.f, 1.f, 1.f});

	//draw elements
	for (const auto& cur_elem : group.elements) {
		draw_element(cur_elem, pos + group._pos + cur_elem._pos);
	}
}

void c_window::draw_element(const c_element& element, const vec2i& cur_pos) {

	if (element._item_type == check) {
		render->draw_quad_outline(cur_pos, vec2i(10, 10), accent);
		render->draw_text(element._label, cur_pos + vec2i(15, 7), 0, {1.f, 1.f, 1.f, 1.f});

		if (element._active) {
			render->draw_quad(cur_pos + vec2i(2, 2), vec2i(7, 7), accent);
		}
	}
	else if (element._item_type == slider) {
		render->draw_quad_outline(cur_pos + vec2i(0, 15), vec2i(100, 10), accent);
		render->draw_quad(cur_pos + vec2i(2, 17), vec2i(element._size - 3, 7), element._color);
		render->draw_text(element._label + element._valuetext, cur_pos + vec2i(0, 8), 0, {1.f, 1.f, 1.f, 1.f});
	}
	else if (element._item_type == keybind) {
		render->draw_quad_outline(cur_pos + vec2i(0, 15), vec2i(100, 16), accent);
		render->draw_text(element._valuetext, cur_pos + vec2i(50, 17) + vec2i(0, 8), 0, {1.f, 1.f, 1.f, 1.f});
		render->draw_text(element._label, cur_pos + vec2i(0, 5), 0, {1.f, 1.f, 1.f, 1.f});
	}
	else if (element._item_type == combo) {
		render->draw_quad_outline(cur_pos + vec2i(0, 15), vec2i(100, 16), element._color);
		render->draw_text(element._label, cur_pos + vec2i(0, 8), 0, {1.f, 1.f, 1.f, 1.f});
		render->draw_text(element._valuetext, cur_pos + vec2i(5, 17) + vec2i(0, 8), 0, {1.f, 1.f, 1.f, 1.f});

		if (element._active) {
			combo_to_overlay = element;
			ovrlaypos = cur_pos;
		}
		else {
			//draw::triangle(cur_pos + vec2(87, 19), 8, accent);
		}
	}
}

//perform as many calculations as possible to reduce stress at c_window::draw
void c_window::update_elements() {

	if (interaction_type == itt_combo && !l_click) {
		interaction_type = itt_wait;
	}
	else if (!l_click && interaction_type && interaction_type != itt_wait) {
		interaction_type = itt_null;
		key_wait = false;
		active_element = NULL;
	}

	//pipeline that shit
	if (l_click && !tabs[active_tab].groups.empty()) {
		//check elements
		for (size_t i = 1; i < tabs[active_tab].groups.size() + 1; i++) {
			function_group(tabs[active_tab].groups[i - 1]);

			if (i < tabs[active_tab].groups.size())
				function_group(tabs[active_tab].groups[i]);
			i++;
		}
	}

	if (interaction_type)
		return;

	//check tabs
	for (int i = 0; i < (int)tabs.size(); i++) {
		if (l_click && mouse_in_region(pos + vec2i(0, tabs[i]._pos.y - 15), vec2i(85, 30))) {
			interaction_type = itt_tab;
			active_tab = i;
		}

		if (active_tab == i) {
			tabs[i]._color = accent;
		}
		else {
			tabs[i]._color = { 1.f, 1.f, 1.f, 1.f };
		}
	}
}

void c_window::function_group(c_group& group) {
	for (size_t i = 0; i < group.elements.size(); i++) {

		const auto& element_pos = pos + group._pos + group.elements[i]._pos;
		const auto& elementid = (uintptr_t)&group.elements[i];
		const auto& item_type = group.elements[i]._item_type;

		if (item_type == check) {
			if (!interaction_type && mouse_in_region(element_pos - vec2i(0, 5), vec2i(85, 15))) {
				interaction_type = itt_check;

				//flip value of variable
				*(bool*)group.elements[i]._value = !*(bool*)group.elements[i]._value;

				//flip checkbox value in menu
				group.elements[i]._active = *(bool*)group.elements[i]._value;
			}
		}
		else if (item_type == slider) {
			if ((mouse_in_region(element_pos + vec2i(0, 15), vec2i(100, 10))
				|| interaction_type == itt_slider && active_element == elementid)) {
				//set interaction type
				interaction_type = itt_slider;

				//calculate mouse pos in accordance to slider width
				const int& width = std::min(100, std::max(0, (100 + (mouse.x - (element_pos.x + 100)))));

				//clamp width to fit in container
				group.elements[i]._size = std::max(4, width);

				const float& new_val = std::fminf(group.elements[i]._max, std::fmaxf(group.elements[i]._min, ((width / 100.f) * group.elements[i]._max)));

				//assign new value
				*(float*)group.elements[i]._value = new_val;

				//set slider color to accent if active
				group.elements[i]._color = active_accent;

				//text and shit
				group.elements[i]._valuetext = ": (" + std::format("{:.2f}", new_val) + ")";

				//update active_element
				active_element = elementid;
			}
			else if (active_element != elementid) {
				group.elements[i]._color = accent;
			}
		}
		else if (item_type == combo) {
			if (interaction_type != itt_wait
				&& active_element == NULL
				&& (mouse_in_region(element_pos + vec2i(0, 15), vec2i(100, 16)))) {
				//set interaction type
				interaction_type = itt_combo;

				group.elements[i]._active = true;
				group.elements[i]._color = active_accent;
				active_element = elementid;
			}
			else if (interaction_type == itt_wait && active_element == elementid && mouse_in_region(element_pos + vec2i(0, 31), vec2i(100, group.elements[i]._size))) {

				const auto& new_val = int((mouse.y - (element_pos.y + 31.f)) / 16.f);

				*(int*)group.elements[i]._value = new_val;
				group.elements[i]._valuetext = group.elements[i]._options[*(int*)group.elements[i]._value];

				interaction_type = itt_null;
				group.elements[i]._color = accent;
				group.elements[i]._active = false;
			}
			else if (interaction_type == itt_wait && active_element == elementid) {
				interaction_type = itt_null;
				group.elements[i]._color = accent;
				group.elements[i]._active = false;
			}
		}
		else if (item_type == keybind) {
			//clicked box
			if (interaction_type != itt_wait && active_element == NULL && (mouse_in_region(element_pos + vec2i(0, 15), vec2i(100, 16)))) {
				interaction_type = itt_wait;

				group.elements[i]._valuetext = "...";
				group.elements[i]._active = true;
				group.elements[i]._color = active_accent;

				active_element = elementid;
				key_wait = true;
			}
		}
	}
}

bool c_window::mouse_in_region(const vec2i& item_pos, const vec2i& item_size) {
	bool in_x = false, in_y = false;

	if (mouse.x > item_pos.x && mouse.x < item_pos.x + item_size.x)
		in_x = true;

	if (mouse.y > item_pos.y && mouse.y < item_pos.y + item_size.y)
		in_y = true;

	return in_x && in_y;
}

std::array<std::string, 2> c_window::split_title(std::string title) {
	bool found_space = false;
	std::string text1 = "", text2 = "";

	for (size_t i = 0; i < title.length(); i++) {
		if (title[i] != ' ') {
			if (!found_space)
				text1 += title[i];
			else
				text2 += title[i];
		}
		else {
			found_space = true;
			continue;
		}
	}
	return std::array<std::string, 2> { text1, text2 };
}