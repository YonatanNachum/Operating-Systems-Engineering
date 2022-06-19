#ifndef JOS_INC_VGA_H
#define JOS_INC_VGA_H

#include <inc/types.h>

// Colors
enum vga_color {
        VGA_BLACK = 0,
        VGA_BLUE,
        VGA_GREEN,
        VGA_CYAN,
        VGA_RED,
        VGA_MAGENTA,
        VGA_BROWN,
        VGA_GREY,
        VGA_DARK_GREY,
        VGA_BRIGHT_BLUE,
        VGA_BRIGHT_GREEN,
        VGA_BRIGHT_CYAN,
        VGA_BRIGHT_RED,
        VGA_BRIGHT_MAGENTA,
        VGA_YELLOW,
        VGA_WHITE,

	VGA_NO_COLOR
};

enum shape_type{
	shape_circle = 0,
	shape_line,
	shape_rectangle,
	shape_diamond,
	shape_string,
};

struct draw_type {
	enum shape_type shape;
	enum vga_color border_color;
	union {
		struct draw_circle {
			uint16_t x;
			uint16_t y;
			uint16_t radius;
			enum vga_color fill_color;
		} circle;

		struct draw_diamond {
			uint16_t x;
			uint16_t y;
			uint16_t radius;
			enum vga_color fill_color;
		} diamond;

		struct draw_line {
			uint16_t x1;
			uint16_t y1;
			uint16_t x2;
			uint16_t y2;
		} line;

		struct draw_rectangle {
			uint16_t x;
			uint16_t y;
			uint16_t width;
			uint16_t height;
			enum vga_color fill_color;
		} rectangle;

		struct draw_string {
			uint16_t x;
			uint16_t y;
			char *str;
		} string;
	} u;
};

#endif /* !JOS_INC_VGA_H */
