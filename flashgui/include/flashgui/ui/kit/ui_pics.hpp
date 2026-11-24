#pragma once

#include <filesystem>

namespace fgui::ui {

const int32 UI_PIC_SHOWN = 1;
const int32 UI_PIC_NONE = 2;
const int32 UI_PIC_LOADING = 0;
const int64 UI_PIC_IDLE_MS = 120000;

struct ui_pic_t {
	int64 tex = 0;
	float64 aspect = 1.0;
	bool busy = false;
	int32 fails = 0;
	int64 retry_at = 0;
	int64 drawn_at = 0;
	int64 idle_ms = UI_PIC_IDLE_MS;
};

struct ui_pic_job_t {};
struct ui_pic_worker_t {};
struct ui_pic_kept_t {};

struct ui_pics_t {
	std::map<std::string, ui_pic_t> pics;
	std::vector<ui_pic_worker_t> workers;
	std::vector<ui_pic_job_t> jobs;
	int64 job_seq = 0;
	std::vector<std::string> asked;
	int64 restart_at = 0;
	int64 sweep_at = 0;
	std::map<std::string, int64> bad;
	std::map<std::string, ui_pic_kept_t> kept;
	std::string bad_file;
	bool bad_read = false;
	std::map<int64, std::string> decoding;
};

inline ui_pics_t g_ui_pics;

inline void ui_pic_pump() {}

inline void ui_pic_tick() {
	if (!fgui::render)
		return;
	const int64 now = ui_sys_ms();
	std::vector<fgui::image_handle> expired;
	for (auto it = g_ui_pics.pics.begin(); it != g_ui_pics.pics.end();) {
		const ui_pic_t& pic = it->second;
		if (pic.idle_ms > 0 && pic.drawn_at > 0 && now - pic.drawn_at >= pic.idle_ms) {
			expired.push_back(static_cast<fgui::image_handle>(pic.tex));
			it = g_ui_pics.pics.erase(it);
		} else {
			++it;
		}
	}
	fgui::render->unload_images(expired);
}

inline int64 ui_pic_tex(const std::string& path, bool ask, int64 idle_ms) {
	auto found = g_ui_pics.pics.find(path);
	if (found != g_ui_pics.pics.end()) {
		if (ask) {
			found->second.drawn_at = ui_sys_ms();
			found->second.idle_ms = idle_ms;
		}
		return found->second.tex;
	}
	if (!ask || !fgui::render || path.empty() || !std::filesystem::exists(path))
		return 0;
	const fgui::image_handle handle = fgui::render->load_image(path);
	const auto dimensions = fgui::render->image_size(handle);
	ui_pic_t pic;
	pic.tex = handle;
	pic.aspect = dimensions.first > 0
		? static_cast<float64>(dimensions.second) / static_cast<float64>(dimensions.first) : 1.0;
	pic.drawn_at = ui_sys_ms();
	pic.idle_ms = idle_ms;
	g_ui_pics.pics.emplace(path, pic);
	return pic.tex;
}

inline int32 ui_pic(const std::string& path, float64 x, float64 y, float64 size) {
	const int64 tex = ui_pic_tex(path, true, UI_PIC_IDLE_MS);
	if (tex == 0)
		return UI_PIC_NONE;
	const auto found = g_ui_pics.pics.find(path);
	if (found == g_ui_pics.pics.end())
		return UI_PIC_NONE;
	ui_gfx_texture_fit(tex, found->second.aspect, x, y, size, UI_WHITE);
	return UI_PIC_SHOWN;
}

inline void ui_pic_forget(const std::string& prefix) {
	if (!fgui::render)
		return;
	std::vector<fgui::image_handle> forgotten;
	for (auto it = g_ui_pics.pics.begin(); it != g_ui_pics.pics.end();) {
		if (it->first.compare(0, prefix.size(), prefix) == 0) {
			forgotten.push_back(static_cast<fgui::image_handle>(it->second.tex));
			it = g_ui_pics.pics.erase(it);
		} else {
			++it;
		}
	}
	fgui::render->unload_images(forgotten);
}

inline void ui_pic_set_fault_file(const std::string& path) {
	g_ui_pics.bad_file = path;
	g_ui_pics.bad_read = true;
}

inline void ui_pic_shutdown() {
	if (fgui::render) {
		std::vector<fgui::image_handle> images;
		images.reserve(g_ui_pics.pics.size());
		for (const auto& entry : g_ui_pics.pics)
			images.push_back(static_cast<fgui::image_handle>(entry.second.tex));
		fgui::render->unload_images(images);
	}
	g_ui_pics.pics.clear();
	g_ui_pics.asked.clear();
	g_ui_pics.workers.clear();
	g_ui_pics.jobs.clear();
	g_ui_pics.bad.clear();
	g_ui_pics.kept.clear();
	g_ui_pics.decoding.clear();
	g_ui_pics.job_seq = 0;
	g_ui_pics.restart_at = 0;
	g_ui_pics.sweep_at = 0;
}

inline int64 ui_pic_count() {
	return static_cast<int64>(g_ui_pics.pics.size());
}

inline int64 ui_pic_bytes() {
	if (!fgui::render)
		return 0;
	int64 bytes = 0;
	for (const auto& entry : g_ui_pics.pics) {
		const auto size = fgui::render->image_size(static_cast<fgui::image_handle>(entry.second.tex));
		bytes += static_cast<int64>(size.first) * static_cast<int64>(size.second) * 4;
	}
	return bytes;
}

}
