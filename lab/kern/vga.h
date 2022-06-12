#ifndef _VGA_H_
#define _VGA_H_
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

/* Reference: Intel OpenSource HD Graphics PRM */

#define SCREEN_BASE_ADDR        (uint8_t *)0xf00A0000
#define SCREEN_MAX_ADDR         (uint8_t *)0xf00AFFFF        
#define SCREEN_WIDTH            320
#define SCREEN_HEIGHT           200
#define NUM_COLORS              256

/* Macros for color pick */
#define PICK_COLOR_SIZE	20
#define PICK_NUM_COLORS	3
#define PICK_WIDTH	(PICK_COLOR_SIZE*PICK_NUM_COLORS)

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
};

struct GridPos
{
        int x;
        int y;
};

extern struct GridPos color_loc;
 
void vga_init(void);
void write_pixel(uint32_t x, uint32_t y, uint8_t color);

void remove_color_pick();
void draw_color_pick(int x, int y);
void draw_circle(uint16_t x, uint16_t y, uint16_t radius, uint8_t color);
void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color);
void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);
void draw_diamond(uint16_t x, uint16_t y, uint16_t radius, uint8_t color);

#endif /* _VGA_H_ */
