#include "draw_manager.hpp"

c_draw_manager* g_render = new c_draw_manager();

void c_draw_manager::initialize()
{
    if (!g_interfaces)
        return;

    __try {
        verdana_font = create_font("Verdana", 12, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
        small_font = create_font("Small Fonts", 8, FW_MEDIUM, FONTFLAG_OUTLINE);

        auto engine = g_interfaces->get_engine_client();
        if (engine)
            engine->GetScreenSize(screen_width, screen_height);
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_draw_manager::draw_filled_rect(float x, float y, float width, float height, Color color)
{
    if (!g_interfaces || !g_interfaces->get_vgui_surface())
        return;
    __try {
        g_interfaces->get_vgui_surface()->DrawSetColor(color);
        g_interfaces->get_vgui_surface()->DrawFilledRect(static_cast<int>(x), static_cast<int>(y), static_cast<int>(x + width), static_cast<int>(y + height));
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_draw_manager::draw_outlined_rect(float x, float y, float width, float height, Color color)
{
    if (!g_interfaces || !g_interfaces->get_vgui_surface())
        return;
    __try {
        g_interfaces->get_vgui_surface()->DrawSetColor(color);
        g_interfaces->get_vgui_surface()->DrawOutlinedRect(static_cast<int>(x), static_cast<int>(y), static_cast<int>(x + width), static_cast<int>(y + height));
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_draw_manager::draw_line(int x, int y, int x2, int y2, Color color)
{
    if (!g_interfaces || !g_interfaces->get_vgui_surface())
        return;
    __try {
        g_interfaces->get_vgui_surface()->DrawSetColor(color);
        g_interfaces->get_vgui_surface()->DrawLine(x, y, x2, y2);
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

void c_draw_manager::draw_text(vgui::HFont font, int x, int y, Color color, DWORD flags, const char* msg, ...)
{
    if (!g_interfaces || !g_interfaces->get_vgui_surface() || !msg)
        return;

    char buffer[1024] = {0};
    va_list va_alist;
    va_start(va_alist, msg);
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, va_alist);
    va_end(va_alist);

    wchar_t wbuf[1024] = {0};
    int converted = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wbuf, 1024);
    if (converted <= 0)
        return;

    __try {
        g_interfaces->get_vgui_surface()->DrawSetTextFont(font);
        g_interfaces->get_vgui_surface()->DrawSetTextColor(color);
        g_interfaces->get_vgui_surface()->DrawSetTextPos(x, y);
        g_interfaces->get_vgui_surface()->DrawPrintText(wbuf, wcslen(wbuf));
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

Vector2D c_draw_manager::get_text_size(vgui::HFont font, const char* msg, ...)
{
    if (!g_interfaces || !g_interfaces->get_vgui_surface() || !msg)
        return Vector2D(0,0);

    char buffer[1024] = {0};
    va_list va_alist;
    va_start(va_alist, msg);
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, va_alist);
    va_end(va_alist);

    wchar_t wbuf[1024] = {0};
    MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wbuf, 1024);

    int width = 0, height = 0;
    __try {
        g_interfaces->get_vgui_surface()->GetTextSize(font, wbuf, width, height);
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    return Vector2D(static_cast<float>(width), static_cast<float>(height));
}

bool c_draw_manager::world_to_screen(const Vector& world, Vector& screen)
{
    if (!g_interfaces || !g_interfaces->get_engine_client())
        return false;

    auto screen_transform = [&]() -> bool {
        __try {
            auto& matrix = g_interfaces->get_engine_client()->WorldToScreenMatrix();

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
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    };

    if (screen_transform()) {
        int w = 0, h = 0;
        __try {
            g_interfaces->get_engine_client()->GetScreenSize(w, h);
        } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }

        if (w == 0 || h == 0)
            return false;

        screen.x = (w / 2.0f) + (screen.x * w) / 2.0f;
        screen.y = (h / 2.0f) - (screen.y * h) / 2.0f;

        return true;
    }

    return false;
}

vgui::HFont c_draw_manager::create_font(const char* name, int size, int weight, DWORD flags)
{
    if (!g_interfaces || !g_interfaces->get_vgui_surface() || !name)
        return 0;

    __try {
        vgui::HFont font = g_interfaces->get_vgui_surface()->CreateFont_();
        g_interfaces->get_vgui_surface()->SetFontGlyphSet(font, name, size, weight, 0, 0, flags);
        return font;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}
