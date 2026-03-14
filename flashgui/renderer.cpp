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
	m_mode = render_mode::internal;

	try {
		m_dx->initialize(swapchain, cmd_queue, sync_interval, flags);

		m_dx->fonts = std::make_unique<c_fonts>();

		m_dx->fonts->initialize(m_dx->device);

		// Ensure bucket 0 exists for non-textured shapes (quads, circles, lines, triangles)
		if (im_instances.empty()) {
			im_instances.resize(1);
		}
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
	// Reset per-frame input flags before processing
	//process->begin_input_frame();

	auto now = std::chrono::steady_clock::now();

	++m_frame_count;

	if (now - m_last_fps_update >= std::chrono::seconds(1)) {
		m_fps = static_cast<int>(m_frame_count);
		m_frame_count = 0;
		m_last_fps_update = now;
	}

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
}

void c_renderer::post_present() {
	// update the frame index after presenting, so we know which backbuffer to render to for the next frame
	m_dx->frame_index = m_dx->swapchain->GetCurrentBackBufferIndex();
}

font_handle c_renderer::get_font(const std::wstring& family, int size_px, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style) {
	bool exists = false;
	
	auto handle =
		m_dx->fonts->get_or_create_font(family, weight, style, size_px, &exists,
			m_dx->device, m_dx->cmd_queue, m_dx->get_current_frame_resource());

	if (!exists) {
		// if the font was newly created, we need to add a new vector for its instances
		if (handle >= im_instances.size()) {
			im_instances.resize(size_t(handle) + 1ull);
		}
	}

	return handle;
}

void c_renderer::push_clip_rect(vec2i pos, vec2i size) {
	// Flush current instances before changing scissor
	m_dx->end_frame(im_instances);

	D3D12_RECT rect;
	rect.left = static_cast<LONG>(pos.x);
	rect.top = static_cast<LONG>(pos.y);
	rect.right = static_cast<LONG>(pos.x + size.x);
	rect.bottom = static_cast<LONG>(pos.y + size.y);

	m_clip_stack.push_back(rect);

	auto& cmd = m_dx->get_current_frame_resource().command_list;
	cmd->RSSetScissorRects(1, &rect);
}

void c_renderer::pop_clip_rect() {
	if (!m_clip_stack.empty())
		m_clip_stack.pop_back();

	// Flush before restoring
	m_dx->end_frame(im_instances);

	auto& cmd = m_dx->get_current_frame_resource().command_list;
	if (m_clip_stack.empty()) {
		cmd->RSSetScissorRects(1, &m_dx->scissor_rect);
	}
	else {
		cmd->RSSetScissorRects(1, &m_clip_stack.back());
	}
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

void c_renderer::draw_text(const std::string& text, vec2i pos, const wchar_t* font_family, int px_size, DirectX::XMFLOAT4 clr, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style) {
	if (process->needs_resize())
		return;
	font_handle font = get_font(font_family, px_size, weight, style);
	draw_text(text, pos, font, clr);
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

void c_renderer::draw_triangle(vec2i p1, vec2i p2, vec2i p3, DirectX::XMFLOAT4 clr) {
	if (process->needs_resize())
		return;

	// Compute bounding box that encloses all three vertices
	int min_x = std::min({ p1.x, p2.x, p3.x });
	int min_y = std::min({ p1.y, p2.y, p3.y });
	int max_x = std::max({ p1.x, p2.x, p3.x });
	int max_y = std::max({ p1.y, p2.y, p3.y });

	vec2i bbox_pos(min_x, min_y);
	vec2i bbox_size(max_x - min_x, max_y - min_y);

	// Pack the three screen-space vertices into uv (xy = p1, zw = p2) and inst_rot/inst_stroke for p3
	// The pixel shader uses sv_position (screen space) for the barycentric test
	im_instances.at(0).push_back(shape_instance(
		bbox_pos, bbox_size, clr,
		static_cast<float>(p3.x), static_cast<float>(p3.y),
		shape_type::triangle,
		DirectX::XMFLOAT4(
			static_cast<float>(p1.x), static_cast<float>(p1.y),
			static_cast<float>(p2.x), static_cast<float>(p2.y)
		)
	));
}

image_handle c_renderer::load_image(const uint8_t* rgba_pixels, uint32_t width, uint32_t height) {
	image_handle h = m_dx->fonts->load_image_rgba(
		rgba_pixels, width, height,
		m_dx->device, m_dx->cmd_queue, m_dx->get_current_frame_resource());

	// Ensure the instance bucket exists for this handle
	if (h >= im_instances.size()) {
		im_instances.resize(size_t(h) + 1);
	}

	return h;
}

void c_renderer::draw_image(image_handle img, vec2i pos, vec2i size, DirectX::XMFLOAT4 tint) {
	if (process->needs_resize())
		return;

	if (img >= im_instances.size())
		return;

	// Full UV rect [0,0]->[1,1] covers the entire image texture
	// Reuse shape_type 5 (text_quad) since the pixel shader already samples t0 with UV
	im_instances.at(img).push_back(shape_instance(
		pos, size, tint,
		0.f, 1.f,
		shape_type::image_quad,
		DirectX::XMFLOAT4(0.f, 0.f, 1.f, 1.f)
	));
}

float c_renderer::measure_text_width(const std::string& text, font_handle font) const {
	float width = 0.f;
	for (char c : text) {
		const font_glyph_info* g = m_dx->fonts->get_glyph_info(font, c);
		if (g) width += g->advance;
	}
	return width;
}

float c_renderer::measure_text_width(const std::string& text, const wchar_t* font_family, int px_size, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style) const {
	return measure_text_width(text, m_dx->fonts->get_or_create_font(font_family, weight, style, px_size, nullptr,
		m_dx->device, m_dx->cmd_queue, m_dx->get_current_frame_resource()));
}