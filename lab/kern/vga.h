#ifndef _VGA_H_
#define _VGA_H_
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#define SCREEN_BASE_ADDR        (uint8_t *)0xf00A0000
#define SCREEN_WIDTH            320
#define SCREEN_HEIGHT           200
#define NUM_COLORS              256

void vga_init(void);
void draw_pixel(uint32_t x, uint32_t y);

#endif /* _VGA_H_ */
