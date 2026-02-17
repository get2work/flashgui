#pragma once

#include <flashgui.h>
#include <vector>

enum e_keybinds : int {
	kmenu_open = 0,
	klb_enable = 1,
	ktb_enable = 2,
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
	inline bool esp_enable = true;
	inline bool esp_health = true;
	inline bool esp_box = true;
	inline bool esp_name = true;
	inline bool esp_weapon = true;
	inline bool esp_recoil = true;

	// chams
	inline bool cham_enable = true;
	inline int cham_mat = 0;

	inline bool lb_enable = true;
	inline bool lb_flashcheck = true;
	inline bool lb_smokecheck = true;
	inline int lb_hitbox = 0;
	inline std::vector<int> lb_hboxes = { 0, 1, 2 };
	inline float lb_fov = 3.f;
	inline float lb_smoothing = 5.f;
	inline float lb_deviation = 0.2f;

	inline float lb_recoilyaw = 0.9f;
	inline float lb_recoilpitch = 1.1f;
	inline float lb_recoilrand = 0.2f;

	inline bool tb_enable = true;
	inline bool tb_smokecheck = true;
	inline bool tb_flashcheck = true;
	inline float tb_delay = 0.100f;
	inline float tb_hitchance = 70.f;

	inline bool misc_bhop = false;

	inline auto window = fgui::c_window("drk csgo", fgui::vec2i(450, 450));

	void setup_menu();

	void update();
}