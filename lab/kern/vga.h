#ifndef _VGA_H_
#define _VGA_H_
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/vga.h>

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

struct GridPos
{
        int x;
        int y;
};

extern struct GridPos color_loc;
 
void vga_init(void);
void write_pixel(uint32_t x, uint32_t y, uint8_t color);
void clear_screen(void);

void remove_color_pick();
void draw_color_pick(int x, int y);
void draw_circle(struct draw_circle circle, uint8_t border_color);
void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t border_color);
void draw_rectangle(struct draw_rectangle rectangle, uint8_t border_color);
void draw_diamond(struct draw_diamond diamond, uint8_t border_color);

#endif /* _VGA_H_ */
