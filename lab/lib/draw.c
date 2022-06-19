#include <inc/lib.h>

int
draw_circle(uint16_t x, uint16_t y, uint16_t radius, enum vga_color border_color,
                 enum vga_color fill_color)
{
        struct draw_type draw;

        draw.shape = shape_circle;
        draw.border_color = border_color;
        draw.u.circle.x = x;
        draw.u.circle.y = y;
        draw.u.circle.radius = radius;
        draw.u.circle.fill_color = fill_color;
        return sys_draw(&draw);
}

int
draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color)
{
        struct draw_type draw;

        draw.shape = shape_line;
        draw.border_color = color;
        draw.u.line.x1 = x1;
        draw.u.line.y1 = y1;
        draw.u.line.x2 = x2;
        draw.u.line.y2 = y2;
        return sys_draw(&draw);
}

int
draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, enum vga_color border_color,
                    enum vga_color fill_color)
{
        struct draw_type draw;

        draw.shape = shape_rectangle;
        draw.border_color = border_color;
        draw.u.rectangle.x = x;
        draw.u.rectangle.y = y;
        draw.u.rectangle.height = height;
        draw.u.rectangle.width = width;
        draw.u.rectangle.fill_color = fill_color;
        return sys_draw(&draw);        
}

int
draw_diamond(uint16_t x, uint16_t y, uint16_t radius, enum vga_color border_color,
                 enum vga_color fill_color)
{
        struct draw_type draw;

        draw.shape = shape_diamond;
        draw.border_color = border_color;
        draw.u.circle.x = x;
        draw.u.circle.y = y;
        draw.u.circle.radius = radius;
        draw.u.circle.fill_color = fill_color;
        return sys_draw(&draw);
}

int
draw_string(uint16_t x, uint16_t y, char *str, enum vga_color color)
{
        struct draw_type draw;

        draw.shape = shape_string;
        draw.border_color = color;
        draw.u.string.x = x;
        draw.u.string.y = y;
        draw.u.string.str = str;
        return sys_draw(&draw);       
}

void
clear_screen(void)
{
        sys_clear_screen();
}