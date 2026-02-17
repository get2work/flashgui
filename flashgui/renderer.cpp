#include "pch.h"
#include "renderer.h"
#include "include/flashgui.h"

using namespace fgui;

LRESULT CALLBACK hk::window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (process) {
		if (process->window_proc(hwnd, msg, wparam, lparam))
			return true;
	}
	return DefWindowProcA(hwnd, msg, wparam, lparam);
}

void c_renderer::initialize(IDXGISwapChain3* swapchain, ID3D12CommandQueue* cmd_queue, UINT sync_interval, UINT flags) {
	m_mode = (swapchain && cmd_queue) ? render_mode::hooked : render_mode::standalone;

	try {
		m_dx->initialize(swapchain, cmd_queue, sync_interval, flags);

		m_dx->fonts = std::make_unique<c_fonts>();

		m_dx->fonts->initialize(m_dx->device);
	}
	catch (const std::exception& e) {
		std::cerr << "Exception thrown during renderer initialization!\n" << e.what() << std::endl;
	}
}

void c_renderer::release_resources() {
	m_dx->release_resources();
}

void c_renderer::create_resources(bool create_heap_and_buffers) {

	if (create_heap_and_buffers)
		m_dx->create_backbuffers();

	m_dx->create_resources();
}

int c_renderer::get_fps() const {
	return m_fps;
}

void c_renderer::wait_for_gpu() {
	m_dx->wait_for_gpu();
}


void c_renderer::begin_frame() {
	auto now = std::chrono::steady_clock::now();

	++m_frame_count;

	if (now - m_last_fps_update >= std::chrono::seconds(1)) {
		m_fps = static_cast<int>(m_frame_count); // frames rendered in the last second
		m_frame_count = 0;
		m_last_fps_update = now;
	}

	// called at the beginning of each frame. Prepares command list and render target.
	m_dx->begin_frame();
}

//TODO: optimize by multithreading upload and draw calls, optimize draws as well
void c_renderer::end_frame() {
	if (process->needs_resize()) {
		// Reset the resize flag
		process->resize_complete();
		return;
	}

	m_dx->end_frame(im_instances);

	//im_instances.clear();
}

void c_renderer::post_present() {
	// update the frame index after presenting, so we know which backbuffer to render to for the next frame
	if (m_dx->swapchain)
		m_dx->frame_index = m_dx->swapchain->GetCurrentBackBufferIndex();
}

font_handle c_renderer::get_or_create_font(const std::wstring& family, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, int size_px) {
	bool exists = false;
	auto handle =
		m_dx->fonts->get_or_create_font(family, weight, style, size_px, &exists,
			m_dx->device, m_dx->cmd_queue, m_dx->get_current_frame_resource());

	if (!exists) {
		// if the font was newly created, we need to add a new vector for its instances
		if (handle >= im_instances.size()) {
			im_instances.resize(handle + 1);
		}
	}

	return handle;
}

std::vector<std::wstring> c_renderer::get_font_families() const {
	return m_dx->fonts->enumerate_families();
}

void c_renderer::draw_quad(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float outline_width, float rotation) {
	if (process->needs_resize())
		return;
	im_instances.at(0).push_back(shape_instance(pos, size, clr, rotation, outline_width, shape_type::quad));
}

void c_renderer::draw_line(vec2i start, vec2i end, DirectX::XMFLOAT4 clr, float width) {
	if (process->needs_resize())
		return;
	im_instances.at(0).push_back(shape_instance(start, end, clr, 0.f, width, shape_type::line));
}

void c_renderer::draw_quad_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float width, float rotation) {
	if (process->needs_resize())
		return;
	im_instances.at(0).push_back(shape_instance(pos, size, clr, rotation, width, shape_type::quad_outline));
}

void c_renderer::draw_circle(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	if (process->needs_resize())
		return;
	im_instances.at(0).push_back(shape_instance(pos, size, clr, angle, outline_width, shape_type::circle));
}
void c_renderer::draw_circle_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	if (process->needs_resize())
		return;
	im_instances.at(0).push_back(shape_instance(pos, size, clr, angle, outline_width, shape_type::circle_outline));
}

void c_renderer::draw_text(const std::string& text, vec2i pos, font_handle font, DirectX::XMFLOAT4 clr) {
	if (process->needs_resize())
		return;


	// use float cursor for sub-pixel advances
	vec2f cursor = pos;

	// atlas size used to compute glyph pixel extents from UVs.
	// if we change atlas size generation, consider exposing atlas size via the fonts API.
	const int atlas_size = 512;

	for (char c : text) {
		const font_glyph_info* p_glyph = m_dx->fonts->get_glyph_info(font, c);
		if (!p_glyph) {
			continue;
		}
		const font_glyph_info& glyph = *p_glyph;

		// Calculate actual glyph dimensions from UV coords (convert to pixels)
		float uv_w = glyph.u1 - glyph.u0;
		float uv_h = glyph.v1 - glyph.v0;
		int glyph_w = static_cast<int>(std::lround(uv_w * atlas_size));
		int glyph_h = static_cast<int>(std::lround(uv_h * atlas_size));

		// apply baseline offset for vertical alignment and horizontal offset for bitmap origin
		vec2f glyph_pos = cursor;
		glyph_pos.x += static_cast<float>(glyph.offset_x); // use glyph offset_x so bitmap aligns to pen
		glyph_pos.y += glyph.offset_y;

		im_instances.at(font).push_back(shape_instance(glyph_pos,
			vec2i(glyph_w, glyph_h),
			clr,
			0.f,
			1.f,
			shape_type::text_quad,
			DirectX::XMFLOAT4(glyph.u0, glyph.v0, glyph.u1, glyph.v1)
		));

		// advance pen by the glyph's float advance (preserves fractional advances and kerning)
		cursor.x += glyph.advance;
	}
}
/*
void c_renderer::remove_shape(shape_instance* instance) {
	auto it = std::find_if(instances.begin(), instances.end(),
		[instance](const shape_instance& inst) { return &inst == instance; });
	if (it != instances.end()) {
		instances.erase(it);
	}
}

shape_instance* c_renderer::add_quad(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float outline_width, float rotation) {
	instances.push_back(shape_instance(pos, size, clr, rotation, outline_width, shape_type::quad));
		return &instances.back();
}



shape_instance* c_renderer::add_line(vec2i start, vec2i end, DirectX::XMFLOAT4 clr, float width) {
	instances.push_back(shape_instance(start, end, clr, 0.f, width, shape_type::line));
	return &instances.back();
}



shape_instance* c_renderer::add_quad_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float width, float rotation) {
	instances.push_back(shape_instance(pos, size, clr, rotation, width, shape_type::quad_outline));
	return &instances.back();
}



shape_instance* c_renderer::add_circle(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	instances.push_back(shape_instance(pos, size, clr, angle, outline_width, shape_type::circle));
	return &instances.back();
}



shape_instance* c_renderer::add_circle_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	instances.push_back(shape_instance(pos, size, clr, angle, outline_width, shape_type::circle_outline));
	return &instances.back();
}

*/