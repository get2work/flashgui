#include "pch.h"
#include "menu.h"
#include <flashgui.h>

void menu::setup_menu() {

	const auto& t_esp = window.start_tab("esp"); {
		const auto& g_players = window.start_group(t_esp, "players"); {
			window.add_checkbox(t_esp, g_players, "enable", &esp_enable);
			window.add_checkbox(t_esp, g_players, "health", &esp_health);
			window.add_checkbox(t_esp, g_players, "box", &esp_box);
			window.add_checkbox(t_esp, g_players, "name", &esp_name);
			window.add_checkbox(t_esp, g_players, "weapon", &esp_weapon);
			window.add_checkbox(t_esp, g_players, "recoil crosshair", &esp_recoil);
			window.add_checkbox(t_esp, g_players, "chams", &cham_enable);
			window.add_combo(t_esp, g_players, "chams material", &cham_mat, { "default", "flat" });
		} window.end_group(t_esp, g_players);
	}
	window.end_tab(t_esp);

	const auto& t_misc = window.start_tab("misc"); {
		const auto& g_misc = window.start_group(t_misc, "misc"); {
			window.add_checkbox(t_misc, g_misc, "bhop", &misc_bhop);
		} window.end_group(t_misc, g_misc);

		const auto& g_config = window.start_group(t_misc, "config");
	}
	window.end_tab(t_misc);

	const auto& t_legit = window.start_tab("legitbot"); {
		const auto& gl_aim = window.start_group(t_legit, "aim"); {
			window.add_checkbox(t_legit, gl_aim, "enable", &lb_enable);
			window.add_keybind(t_legit, gl_aim, "key", &keys[klb_enable].key, false);
			window.add_checkbox(t_legit, gl_aim, "smoke check", &lb_smokecheck);
			window.add_checkbox(t_legit, gl_aim, "flash check", &lb_flashcheck);
			window.add_combo(t_legit, gl_aim, "primary hitbox", &lb_hitbox, { "head", "chest", "stomach", "nearest" });
			window.add_slider(t_legit, gl_aim, "fov", &lb_fov, 0.f, 30.f);
			window.add_slider(t_legit, gl_aim, "smoothing", &lb_smoothing, 1.f, 20.f);
			window.add_slider(t_legit, gl_aim, "deviation", &lb_deviation, 0.f, 1.f);
			window.add_slider(t_legit, gl_aim, "rcs yaw", &lb_recoilyaw, 0.f, 1.25f);
			window.add_slider(t_legit, gl_aim, "rcs pitch", &lb_recoilpitch, 0.f, 1.25f);
			window.add_slider(t_legit, gl_aim, "rcs randomization", &lb_recoilrand, 0.f, 1.f);
		}
		window.end_group(t_legit, gl_aim);
		const auto& gl_trigger = window.start_group(t_legit, "trigger"); {
			window.add_checkbox(t_legit, gl_trigger, "enable", &tb_enable);
			window.add_keybind(t_legit, gl_trigger, "key", &keys[ktb_enable].key, false);
			window.add_checkbox(t_legit, gl_trigger, "smoke check", &tb_smokecheck);
			window.add_checkbox(t_legit, gl_trigger, "flash check", &tb_flashcheck);
			window.add_slider(t_legit, gl_trigger, "delay", &tb_delay, 0.f, 1.f);
			window.add_slider(t_legit, gl_trigger, "hitchance", &tb_hitchance, 0.f, 100.f);
		}
		window.end_group(t_legit, gl_trigger);
	}
	window.end_tab(t_legit);

	window.set_active(t_esp);
}

void menu::update() {
	if (GetAsyncKeyState(VK_DELETE)) {
		//hack::unload = TRUE;
		return;
	}

	for (size_t i = 0; i < keys.size(); i++) {
		if (keys[i].km == km_toggle) {
			if (GetAsyncKeyState(keys[i].key) & 1)
				keys[i].val = !keys[i].val;
		}
		else
			keys[i].val = GetAsyncKeyState(keys[i].key);
	}

	if (window.key_wait) {
		for (int i = VK_LBUTTON; i != VK_RMENU; i++) {
			if (!GetAsyncKeyState(i))
				continue;

			assert(window.active_element != NULL);

			auto& keybind = *(fgui::c_keybind*)window.active_element;

			*(int*)keybind._value = i;
			keybind._valuetext = fgui::keynames[i];

			window.interaction_type = fgui::itt_null;
			keybind._color = window.accent;
			keybind._active = false;

			window.active_element = NULL;
			window.key_wait = false;
			break;
		}
	}

	window.update();
}