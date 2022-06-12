#include <inc/x86.h>
#include <inc/memlayout.h>
#include <inc/kbdreg.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/vga.h>
#include <kern/picirq.h>

/***** VGA I/O Ports *****/
#define	SEQUENCER_ADDR_REG		0x3C4
#define SEQUENCER_DATA_REG		0x3C5
#define SEQUENCER_RESET_IDX		0
#define SEQUENCER_CLOCK_IDX		1
#define SEQUENCER_MAP_IDX		2
#define SEQUENCER_CHAR_IDX		3
#define SEQUENCER_MEM_MODE_IDX		4

#define MISC_OUTPUT_REG_READ		0x3CC
#define MISC_OUTPUT_REG_WRITE		0x3C2

#define	GRAPHIC_ADDR_REG		0x3CE
#define GRAPHIC_DATA_REG		0x3CF
#define GRAPHIC_SET_RESET_IDX		0
#define GRAPHIC_ENABLE_IDX		1
#define GRAPHIC_COLOR_COMP_IDX		2
#define GRAPHIC_DATA_ROT_IDX		3
#define GRAPHIC_READ_IDX		4
#define GRAPHIC_MODE_IDX		5
#define GRAPHIC_MISC_IDX		6
#define GRAPHIC_CARE_IDX		7
#define GRAPHIC_MASK_IDX		8

#define	CRTC_ADDR_REG			0x3D4
#define CRTC_DATA_REG			0x3D5

/* ATTR_DATA_READ_REG is also data write*/
#define	ATTR_ADDR_REG			0x3C0
#define ATTR_DATA_READ_REG		0x3C1
#define ATTR_RESET_REG			0x3DA

struct GridPos color_loc = {0, 0};
uint8_t color_pick_backup[PICK_COLOR_SIZE][PICK_WIDTH] = {{0}};

static void clear_screen(void);

void
write_pixel(uint32_t x, uint32_t y, uint8_t color)
{
	uint8_t *off;
	off = SCREEN_BASE_ADDR + SCREEN_WIDTH * y + x;
	if (off < SCREEN_MAX_ADDR) {
		*off = color;
	}
}

static void
clear_screen (void)
{
        uint16_t x, y;
        for(y = 0; y < SCREEN_HEIGHT; y++) {
		for(x = 0; x < SCREEN_WIDTH; x++) {
			write_pixel(x, y, 0);
                }
        }
}

/* Initialize VGA to 320x200x256 */
void
vga_init(void)
{
	uint8_t i;
	/* ___Sequencer Registers:___ */
	/* Reset Register:
	 * SR - 1
	 * AR - 1
	 */
	outb(SEQUENCER_ADDR_REG, SEQUENCER_RESET_IDX);
	outb(SEQUENCER_DATA_REG, 0x03);

	/* Clocking mode Register:
	 * 9/8DM - 1
	 */
	outb(SEQUENCER_ADDR_REG, SEQUENCER_CLOCK_IDX);
	outb(SEQUENCER_DATA_REG, 0x01);

	/* Map Mask Register:
	 * Memory Plane Write Enable - all
	 */
	outb(SEQUENCER_ADDR_REG, SEQUENCER_MAP_IDX);
	outb(SEQUENCER_DATA_REG, 0x0F);

	/* Character Map Select Register: */
	outb(SEQUENCER_ADDR_REG, SEQUENCER_CHAR_IDX);
	outb(SEQUENCER_DATA_REG, 0x00);	

	/* Sequencer Memory Mode Register:
	 * Ext.mem - 1
	 * O/E Dis - 1
	 * Chain 4 - 1
	 * */
	outb(SEQUENCER_ADDR_REG, SEQUENCER_MEM_MODE_IDX);
	outb(SEQUENCER_DATA_REG, 0x0E);	


	/* ___Miscellaneous Output Register:___
	 * I/OAS - 1
	 * RAM En - 1
	 * HSYNCP - 1
	 * VSYNCP - 1
	 */
	outb(MISC_OUTPUT_REG_WRITE, 0x63);


	/* ___Graphics Registers:___ */
	/* Set/Reset Register: */
	outb(GRAPHIC_ADDR_REG, GRAPHIC_SET_RESET_IDX);
	outb(GRAPHIC_DATA_REG, 0x0);
	
	/* Set/Reset Register: */
	outb(GRAPHIC_ADDR_REG, GRAPHIC_ENABLE_IDX);
	outb(GRAPHIC_DATA_REG, 0x0);
	
	/* Color Compare Register: */
	outb(GRAPHIC_ADDR_REG, GRAPHIC_COLOR_COMP_IDX);
	outb(GRAPHIC_DATA_REG, 0x0);

	/* Data Rotate Register: */
	outb(GRAPHIC_ADDR_REG, GRAPHIC_DATA_ROT_IDX);
	outb(GRAPHIC_DATA_REG, 0x0);
	
	/* Read Map Select Register: */
	outb(GRAPHIC_ADDR_REG, GRAPHIC_READ_IDX);
	outb(GRAPHIC_DATA_REG, 0x0);

	/* Graphics Mode Register: 
	 * Write Mode - 0
	 * Read Mode - 0
	 * Shift256 - 1
	 */
	outb(GRAPHIC_ADDR_REG, GRAPHIC_MODE_IDX);
	outb(GRAPHIC_DATA_REG, 0x40);

	/* Miscellaneous Graphics Register:
	 * Alpha Dis - 1
	 * Memory Map select - 01
	 */
	outb(GRAPHIC_ADDR_REG, GRAPHIC_MISC_IDX);
	outb(GRAPHIC_DATA_REG, 0x05);

	/* Color Don't Care Register:
	 * Color Don't Care - all
	 */
	outb(GRAPHIC_ADDR_REG, GRAPHIC_CARE_IDX);
	outb(GRAPHIC_DATA_REG, 0x0F);

	/* Bit Mask Register :
	 * Bit Mask - all
	 */
	outb(GRAPHIC_ADDR_REG, GRAPHIC_MASK_IDX);
	outb(GRAPHIC_DATA_REG, 0xFF);


	/* ___CRT Controller Registers:____ */
	/* Values: http://www.osdever.net/FreeVGA/vga/crtcreg.htm#11 */
	unsigned char crtc_values[] = {
		0x5F, 0x4F, 0x50 | 0x80, 0x82, 0x54, 0x80, 0xBF, 0x1F,
            	0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           	0x9C & ~(0x80), 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
            	0xFF
	};
	/* Certain CRTC registers can be protected from read or write access for compatibility with programs
	 * written prior to the VGA's existence. This protection is controlled via the Enable Vertical Retrace
	 * Access and CRTC Registers Protect Enable fields.
	 */
	outb(CRTC_ADDR_REG, 0x03);
	outb(CRTC_DATA_REG, inb(CRTC_DATA_REG) | 0x80);

	outb(CRTC_ADDR_REG, 0x11);
	outb(CRTC_DATA_REG, inb(CRTC_DATA_REG) & ~(0x80));

	for(i = 0; i < 25; i++) {
		outb(CRTC_ADDR_REG, i);
		outb(CRTC_DATA_REG, crtc_values[i]);
    	}
	/* Lock CRTC */
	outb(CRTC_ADDR_REG, 0x03);
	outb(CRTC_DATA_REG, inb(CRTC_DATA_REG) & ~(0x80));

	outb(CRTC_ADDR_REG, 0x11);
	outb(CRTC_DATA_REG, inb(CRTC_DATA_REG) | 0x80);


	/* ___Attribute Controller Registers:____ */
	/* Values: http://www.osdever.net/FreeVGA/vga/attrreg.htm */
	unsigned char attr_values[] = {
        	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        	0x41, 0x00, 0x0F, 0x00, 0x00
	};
	for(i = 0; i < 21; i++) {
		/* The index and the data are written to the same port, one after another.
		 * A flip-flop inside the card keeps track of whether the next write will be
		 * handled is an index or data. Because there is no standard method of determining
		 * the state of this flip-flop, the ability to reset the flip-flop such that the next 
		 * write will be handled as an index is provided. his is accomplished by reading ATTR_RESET_REG.
		 */	
		inb(ATTR_RESET_REG);
		outb(ATTR_ADDR_REG, i);
		outb(ATTR_ADDR_REG, attr_values[i]);
	}

	/* lock 16-color palette and unblank display */
	inb(ATTR_RESET_REG);
	outb(ATTR_ADDR_REG, 0x20);
        clear_screen();
}

static void 
draw_dots(int xc, int yc, int x, int y, uint8_t color) 
{ 
	write_pixel(xc+x, yc+y, color); 
	write_pixel(xc-x, yc+y, color); 
	write_pixel(xc+x, yc-y, color); 
	write_pixel(xc-x, yc-y, color); 
	write_pixel(xc+y, yc+x, color); 
	write_pixel(xc-y, yc+x, color); 
	write_pixel(xc+y, yc-x, color); 
	write_pixel(xc-y, yc-x, color); 
} 

void 
draw_circle(uint16_t x, uint16_t y, uint16_t radius, uint8_t color)
{
	int x2 = 0, y2 = radius;
	/* d is a pool of iterations for drawing dots with changing x2 but without changing y2, when d
	 * is positive it means we draw enough dots with the same y2 w.r.t to the radius.
	 */
	int d = 3 - 2 * radius;
	draw_dots(x, y, x2, y2, color);
	while (y2 >= x2) {	
		x2++;
		if (d > 0) {
			y2--;
			d = d + 4 * (x2 - y2) + 10;
		} else {
			d = d + 4 * x2 + 6;
		} 
		draw_dots(x, y, x2, y2, color);
  	} 
}

void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color)
{
	uint16_t i;

	if (y1 == y2) {
    		for(i = x1; i <= x2; i++) {
      			write_pixel(i, y1, color);
		}
    		return;
  	}

  	if (x1 == x2) {
    		for (i = y1; i <= y2; i++) {
      			write_pixel(x1, i, color);
    		}
    		return;
  	}
}

void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color)
{
  	draw_line(x, y, x, y + height, color);
  	draw_line(x, y, x + width, y, color);
  	draw_line(x + width, y, x + width, y + height, color);
  	draw_line(x, y + height, x + width, y + height, color);
}

void draw_diamond(uint16_t x, uint16_t y, uint16_t radius, uint8_t color)
{
	uint16_t x2 = 0, y2 = radius;
  	uint16_t d = 3 - 2 * radius;
  	draw_dots(x, y, x2, y2, color);
  	while (y2 >= x2) {
    		x2++;
      		y2--;
      		d = d + 4 * (x2 - y2) + 10;
    		draw_dots(x, y, x2, y2, color);
  	} 
}

static uint8_t
write_pixel_ret(uint32_t x, uint32_t y, uint8_t color)
{
	uint8_t *off;
	uint8_t old_color = 0;

	off = SCREEN_BASE_ADDR + SCREEN_WIDTH * y + x;
	if (off < SCREEN_MAX_ADDR) {
		old_color = *off;
		*off = color;
	}
	return old_color;
}

void draw_color_pick(int x, int y)
{
	uint8_t c, i, j, old_color=0;
	color_loc.x = x;
	color_loc.y = y;
	uint8_t colors[PICK_NUM_COLORS] = {VGA_WHITE, VGA_RED, VGA_BLUE};
	for (c = 0; c < PICK_NUM_COLORS; ++c) {
		for (i = 0; i < PICK_COLOR_SIZE; ++i) {
			for (j = 0; j < PICK_COLOR_SIZE; ++j) {
				old_color = write_pixel_ret(x + i + c * PICK_COLOR_SIZE, y + j, colors[c]);
				color_pick_backup[j][i + c * PICK_COLOR_SIZE] = old_color;
			}
		}
	}
}

void remove_color_pick()
{
	uint8_t c, i, j;

	for (c = 0; c < PICK_NUM_COLORS; ++c) {
		for (i = 0; i < PICK_COLOR_SIZE; ++i) {
			for (j = 0; j < PICK_COLOR_SIZE; ++j) {
				write_pixel(color_loc.x + i + c * PICK_COLOR_SIZE, color_loc.y + j,
					    color_pick_backup[j][i + c * PICK_COLOR_SIZE]);
			}
		}
	}
	color_loc.x = 0;
	color_loc.y = 0;
}