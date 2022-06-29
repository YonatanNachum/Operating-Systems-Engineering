#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	struct mouse_u_pkt pkt;
        int i = 0;
        enum vga_color color = VGA_BLUE;

        cprintf("Move mouse to check incoming packets\n");
        cprintf("Press on the rectangle to change his color\n");
        draw_string(20, 20, "PRESS ON THE RECTANGLE", VGA_CYAN );
        draw_rectangle(50, 50, 80, 80, color, color);
	while (i < 10) {
		if (sys_mouse_getp(&pkt) == 0) {
                        cprintf("x = %d, y = %d, click type = %d\n", pkt.x, pkt.y, pkt.type);
                        if (pkt.x >= 50 && pkt.x <= 130 &&
                            pkt.y >= 50 && pkt.y <= 130) {
                                color++;
                                draw_rectangle(50, 50, 80, 80, VGA_BLACK, VGA_BLACK);
                                draw_rectangle(50, 50, 80, 80, color, color);
                        }
                        ++i;
                }
	}
        clear_screen();
}
