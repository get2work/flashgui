#pragma once

#include <flashgui.h>
#include <vector>

enum e_keybinds : int {
	kmenu_open = 0,
	k_120gpu = 1,
	k_ai_scan = 2
};

enum e_km : int {
	km_toggle = 0,
	km_hold = 1
};

struct key_bind {

	key_bind(const int& k, const bool& def_val = FALSE, const int& key_mode = km_toggle) {
		key = k;
		val = def_val;
		km = key_mode;
	}

	int key = NULL;
	bool val = false;
	int km = km_toggle;
};

namespace menu {

	inline std::vector<key_bind> keys = {
		key_bind(VK_INSERT, TRUE, km_toggle),
		key_bind(VK_XBUTTON1, FALSE, km_hold),
		key_bind(VK_XBUTTON2, FALSE, km_hold)
	};

	// esp
	inline bool b_anti_palantir = true;
	inline bool b_anti_blackrock = true;
	inline bool b_mega_vpn = true;
	inline bool b_ddos_shield = true;
	inline int i_shield_type = 0;
		
	inline bool b_gpu_acceleration = false;
	inline bool b_anti_fry = false;
	inline bool b_vram_overclock = false;
	inline int i_overclock_mode = 0;
	inline float f_cpu_voltage = 0.f;
	inline float f_cpu_clock_speed = 0.f;
	inline float f_max_cpu_temp = 0.f;
	inline float f_max_gpu_temp = 0.f;
	inline float f_gpu_clock_speed = 0.f;
	inline float f_ethernet_speed = 0.f;
	inline bool b_ai_companion = false;
	inline bool b_quantum_model = false;
	inline bool b_starlink_gpu = false;
	inline float f_power = 0.f;
	inline float f_vram = 0.f;

	inline std::vector<int> lb_multiselect = { 0, 1, 2 };

	inline bool b_anti_rat = false;

	inline auto window = fgui::c_window("drk csgo", fgui::vec2i(450, 450));

	void setup_menu();

	void update();
}