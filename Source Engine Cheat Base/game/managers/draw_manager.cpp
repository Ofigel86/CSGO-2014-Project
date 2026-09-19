#include "draw_manager.hpp"

c_draw_manager* g_render = new c_draw_manager;

void c_draw_manager::initialize()
{
	verdana_font = create_font("Verdana", 12, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	small_font = create_font("Small Fonts", 8, FW_MEDIUM, FONTFLAG_OUTLINE);

	g_interfaces->get_engine_client()->GetScreenSize(screen_width, screen_height);
}

void c_draw_manager::draw_filled_rect(float x, float y, float width, float height, Color color)
{
	g_interfaces->get_vgui_surface()->DrawSetColor(color);
	g_interfaces->get_vgui_surface()->DrawFilledRect(x, y, x + width, y + height);
}

void c_draw_manager::draw_outlined_rect(float x, float y, float width, float height, Color color)
{
	g_interfaces->get_vgui_surface()->DrawSetColor(color);
	g_interfaces->get_vgui_surface()->DrawOutlinedRect(x, y, x + width, y + height);
}

void c_draw_manager::draw_line(int x, int y, int x2, int y2, Color color)
{
	g_interfaces->get_vgui_surface()->DrawSetColor(color);
	g_interfaces->get_vgui_surface()->DrawLine(x, y, x2, y2);
}

void c_draw_manager::draw_text(vgui::HFont font, int x, int y, Color color, DWORD flags, const char* msg, ...)
{
	va_list va_alist;
	char buffer[1024];
	va_start(va_alist, msg);
	_vsnprintf_s(buffer, sizeof(buffer), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];

	MultiByteToWideChar(CP_UTF8, 0, buffer, 256, wbuf, 256);

	int width, height;
	g_interfaces->get_vgui_surface()->GetTextSize(font, wbuf, width, height);

	g_interfaces->get_vgui_surface()->DrawSetTextFont(font);
	g_interfaces->get_vgui_surface()->DrawSetTextColor(color);
	g_interfaces->get_vgui_surface()->DrawSetTextPos(x, y);
	g_interfaces->get_vgui_surface()->DrawPrintText(wbuf, wcslen(wbuf));
}

Vector2D c_draw_manager::get_text_size(vgui::HFont font, const char* msg, ...)
{
	va_list va_alist;
	char buffer[1024];
	va_start(va_alist, msg);
	_vsnprintf_s(buffer, sizeof(buffer), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];

	MultiByteToWideChar(CP_UTF8, 0, buffer, 256, wbuf, 256);

	int width, height;
	g_interfaces->get_vgui_surface()->GetTextSize(font, wbuf, width, height);

	return Vector2D(width, height);
}

bool c_draw_manager::world_to_screen(const Vector& world, Vector& screen)
{
	auto screen_transform = [&world, &screen]() -> bool {
		static auto& matrix = g_interfaces->get_engine_client()->WorldToScreenMatrix();

		screen.x = matrix[0][0] * world.x + matrix[0][1] * world.y + matrix[0][2] * world.z + matrix[0][3];
		screen.y = matrix[1][0] * world.x + matrix[1][1] * world.y + matrix[1][2] * world.z + matrix[1][3];
		screen.z = 0.0f;

		float w = matrix[3][0] * world.x + matrix[3][1] * world.y + matrix[3][2] * world.z + matrix[3][3];

		if (w < 0.001f) {
			screen.x *= 100000;
			screen.y *= 100000;
			return false;
		}

		screen.x /= w;
		screen.y /= w;

		return true;
	};

	if (screen_transform()) {
		int w, h;
		g_interfaces->get_engine_client()->GetScreenSize(w, h);

		screen.x = (w / 2.0f) + (screen.x * w) / 2.0f;
		screen.y = (h / 2.0f) - (screen.y * h) / 2.0f;

		return true;
	}

	return false;
}

vgui::HFont c_draw_manager::create_font(const char* name, int size, int weight, DWORD flags)
{
	vgui::HFont font = g_interfaces->get_vgui_surface()->CreateFont_();
	g_interfaces->get_vgui_surface()->SetFontGlyphSet(font, name, size, weight, 0, 0, flags);

	return font;
}