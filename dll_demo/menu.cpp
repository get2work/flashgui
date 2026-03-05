#include "pch.h"
#include "menu.h"
#include <flashgui.h>

void menu::setup_menu() {

	const auto& t_main = window.start_tab("main"); {
		const auto& g_security = window.start_group(t_main, "security"); {
			window.add_checkbox(t_main, g_security, "enable anti-palantir", &b_anti_palantir);
			window.add_checkbox(t_main, g_security, "blackrock protection", &b_anti_blackrock);
			window.add_checkbox(t_main, g_security, "mega vpn", &b_mega_vpn);
			window.add_checkbox(t_main, g_security, "ddos shield", &b_ddos_shield);
			window.add_combo(t_main, g_security, "shield type", &i_shield_type, { "default", "gemini" });
		} window.end_group(t_main, g_security);
	}
	window.end_tab(t_main);

	const auto& t_misc = window.start_tab("misc"); {
		const auto& g_misc = window.start_group(t_misc, "misc"); {
			window.add_checkbox(t_misc, g_misc, "anti-rat", &b_anti_rat);
		} window.end_group(t_misc, g_misc);

		const auto& g_config = window.start_group(t_misc, "config");
	}
	window.end_tab(t_misc);

	const auto& t_hardware = window.start_tab("hardware"); {
		const auto& g_overclock = window.start_group(t_hardware, "overclock"); {
			window.add_checkbox(t_hardware, g_overclock, "gpu acceleration", &b_gpu_acceleration);
			window.add_keybind(t_hardware, g_overclock, "120% key", &keys[k_120gpu].key, false);
			window.add_checkbox(t_hardware, g_overclock, "anti-fry", &b_anti_fry);
			window.add_checkbox(t_hardware, g_overclock, "vram overclock", &b_vram_overclock);
			window.add_combo(t_hardware, g_overclock, "overclock mode", &i_overclock_mode, { "scared", "gaming", "idgaf", "till it blows" });
			window.add_slider(t_hardware, g_overclock, "cpu voltage", &f_cpu_voltage, 0.f, 30.f);
			window.add_slider(t_hardware, g_overclock, "cpu clock speed GHz", &f_cpu_clock_speed, 1.f, 20.f);
			window.add_slider(t_hardware, g_overclock, "max cpu temp", &f_max_cpu_temp, 0.f, 100.f);
			window.add_slider(t_hardware, g_overclock, "max gpu temp", &f_max_gpu_temp, 0.f, 100.f);
			window.add_slider(t_hardware, g_overclock, "gpu clock speed GHz", &f_gpu_clock_speed, 0.f, 100.f);
			window.add_slider(t_hardware, g_overclock, "ethernet speed GB/s", &f_ethernet_speed, 0.f, 2.f);
		}
		window.end_group(t_hardware, g_overclock);
		const auto& g_ai = window.start_group(t_hardware, "ai"); {
			window.add_checkbox(t_hardware, g_ai, "ai companion", &b_ai_companion);
			window.add_keybind(t_hardware, g_ai, "ai scan", &keys[k_ai_scan].key, false);
			window.add_checkbox(t_hardware, g_ai, "quantum model", &b_quantum_model);
			window.add_checkbox(t_hardware, g_ai, "starlink gpu", &b_starlink_gpu);
			window.add_slider(t_hardware, g_ai, "%power", &f_power, 0.f, 1.f);
			window.add_slider(t_hardware, g_ai, "vram GB", &f_vram, 0.f, 100.f);
		}
		window.end_group(t_hardware, g_ai);
	}
	window.end_tab(t_hardware);
	window.set_active(t_main);
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