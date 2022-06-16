#include <inc/x86.h>
#include <inc/memlayout.h>
#include <inc/kbdreg.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/bitmap.h>
#include <kern/vga.h>


// 0 = no-pixel, 1 = pixel

uint8_t bitmaps_0_9[10][BITMAP_SIZE] = {
  // 0
  {0b01111110, 0b11000011, 0b11000111, 0b11001111,
   0b11011011, 0b11110011, 0b11100011, 0b01111110},

  // 1
  {0b00011000, 0b00111000, 0b00111000, 0b00011000,
   0b00011000, 0b00011000, 0b01111110, 0b01111110},

  // 2
  {0b00111100, 0b11100111, 0b00000111, 0b00000110,
   0b00011110, 0b01111000, 0b11100011, 0b11111111},

  // 3
  {0b00111100, 0b11100111, 0b00000111, 0b00011100,
   0b00000111, 0b00000111, 0b11100110, 0b00111100},

  // 4
  {0b00011100, 0b00111100, 0b01101100, 0b11001100,
   0b11001100, 0b11111110, 0b00001100, 0b00011110},

  // 5
  {0b11111111, 0b11000000, 0b11111110, 0b00000111,
   0b00000111, 0b00000111, 0b11000110, 0b01111110},

  // 6
  {0b00011100, 0b00110000, 0b01100000, 0b11000000,
   0b11111110, 0b11000011, 0b11000011, 0b01111110},

  // 7
  {0b11111111, 0b11111111, 0b11000111, 0b00000111,
   0b00001110, 0b00111000, 0b00111000, 0b00111000},

  // 8
  {0b01111110, 0b01111110, 0b11000011, 0b11000011,
   0b01111110, 0b11000011, 0b11000011, 0b01111110},

  // 9
  {0b01111110, 0b11000011, 0b11000011, 0b01111111,
   0b00000011, 0b00000111, 0b00001110, 0b01111100},
};


uint8_t bitmaps_A_Z[26][BITMAP_SIZE] = {
  // A
  {0b00011000, 0b00111100, 0b01100110, 0b11000011,
   0b11000011, 0b11111111, 0b11000011, 0b11000011},

  // B
  {0b11111100, 0b01100110, 0b01100110, 0b01111100,
   0b01111100, 0b01100110, 0b01100110, 0b11111100},

  // C
  {0b00111110, 0b01100011, 0b11000000, 0b11000000,
   0b11000000, 0b11000000, 0b01100011, 0b00111110},

  // D
  {0b11111100, 0b01100110, 0b01100011, 0b01100011,
   0b01100011, 0b01100011, 0b01100110, 0b11111100},
  
  // E
  {0b11111111, 0b01100001, 0b01100000, 0b01100100,
   0b01111100, 0b01100100, 0b01100001, 0b11111111},

  // F
  {0b11111111, 0b01100001, 0b01100000, 0b01100100,
   0b01111100, 0b01100100, 0b01100000, 0b11110000},

  // G
  {0b00111110, 0b01100011, 0b11000000, 0b11000000,
   0b11000111, 0b11100011, 0b01100011, 0b00111111},

  // H
  {0b11000011, 0b11000011, 0b11000011, 0b11000011,
   0b11111111, 0b11000011, 0b11000011, 0b11000011},

  // I
  {0b01111110, 0b00011000, 0b00011000, 0b00011000,
   0b00011000, 0b00011000, 0b00011000, 0b01111110},

  // J
  {0b00001111, 0b00000110, 0b00000110, 0b00000110,
   0b00000110, 0b11000110, 0b11000110, 0b01111100},

   // K
  {0b11100011, 0b01100011, 0b01100110, 0b01101100,
   0b01111000, 0b01101100, 0b01100110, 0b11100011},

  // L
  {0b11110000, 0b01100000, 0b01100000, 0b01100000,
   0b01100000, 0b01100001, 0b01100011, 0b11111111},

  // M
  {0b11000011, 0b11100111, 0b11111111, 0b11111111,
   0b11011011, 0b11000011, 0b11000011, 0b11000011},

  // N
  {0b11000011, 0b11100011, 0b11110011, 0b11011011,
   0b11001111, 0b11000111, 0b11000011, 0b11000011},

  // O
  {0b00111100, 0b01100110, 0b11000011, 0b11000011,
   0b11000011, 0b11000011, 0b01100110, 0b00111100},

  // P
  {0b11111110, 0b01100011, 0b01100011, 0b01100011,
   0b01111110, 0b01100000, 0b01100000, 0b11110000},

  // Q
  {0b01111110, 0b11000011, 0b11000011, 0b11000011,
   0b11000011, 0b11001111, 0b01111110, 0b00001111},

  // R
  {0b11111110, 0b01100011, 0b01100011, 0b01111110,
   0b01101110, 0b01100110, 0b01100011, 0b11100011},

  // S
  {0b01111110, 0b11000011, 0b11100000, 0b01110000,
   0b00011111, 0b00000011, 0b11000110, 0b01111100},

  // T
  {0b11111111, 0b10011001, 0b00011000, 0b00011000,
   0b00011000, 0b00011000, 0b00011000, 0b00111100},

  // U
  {0b11000011, 0b11000011, 0b11000011, 0b11000011,
   0b11000011, 0b11000011, 0b11000011, 0b11111111},

  // V
  {0b11000011, 0b11000011, 0b11000011, 0b11000011,
   0b11000011, 0b11000011, 0b00111100, 0b00011000},

  // W
  {0b11000011, 0b11000011, 0b11000011, 0b11000011,
   0b11011011, 0b11111111, 0b11100111, 0b11000011},

  // X
  {0b11000011, 0b11000011, 0b01100110, 0b01100110,
   0b00111100, 0b00111100, 0b01100111, 0b11000011},

  // Y
  {0b11000011, 0b11000011, 0b11000011, 0b11000011,
   0b01111110, 0b000011000, 0b00011000, 0b00111100},

  // Z
  {0b11111111, 0b11000011, 0b10000110, 0b00001100,
   0b00011000, 0b00110001, 0b01100011, 0b11111111}

};

// write pixels of 0-9 bits from right-to-left
void draw_num_bitmaps(uint16_t index, uint16_t x, uint16_t y, uint8_t color)
{
	uint8_t i;
	uint16_t temp = 0, pix = 0;

  	for (i = 0; i < BITMAP_SIZE; i++) {
    		temp = x;
    		x += BITMAP_SIZE;
    		pix = bitmaps_0_9[index][i];
    		while (pix > 0) {
      			if (pix & 1) {
        			write_pixel(x, y, color);
      			}
      			pix >>= 1;
      			x--;  
    		}
    		x = temp;
    		y++;
  	}
}

// draw pixels of A-Z bits from right-to-left
void
draw_alpha_bitmaps(uint16_t index, uint16_t x, uint16_t y, uint8_t color)
{
	uint8_t i;
	uint16_t temp = 0, pix = 0;

  	for (i = 0; i < BITMAP_SIZE; i++) {
    		temp = x;
    		x += BITMAP_SIZE;
    		pix = bitmaps_A_Z[index][i];
    		while (pix > 0) {
      			if (pix & 1) {
        			write_pixel(x, y, color);
     			}
      			pix >>= 1;
     			x--;  
    		}
    		x = temp;
    		y++;
  	}
}

void
draw_char(uint16_t x, uint16_t y, char ch, uint8_t color)
{
	if (ch >= '0' && ch <= '9') {
    		draw_num_bitmaps(ch - '0', x, y, color);
  	} else if(ch >= 'A' && ch <= 'Z') {
    		draw_alpha_bitmaps(ch - 'A', x, y, color);
  	}
}

/* The drawing algorithm works as follows:
 * Each letter is of size BITMAP_SIZE X BITMAP_SIZE. on each iteration we take one letter from the input
 * sentence, recognize it and call the appropriate drawing function.
 * Each entry of one of the bitmaps arrays is for one letter, the entry for the letter itself is made up from
 * BITMAP_SIZE entries, one for each line of the BITMAP_SIZE lines composing the letter which mean if you stack
 * the entries for a letter from left to right you will get a drawing of the letter itself.
 * The drawing of each letter start from the right pixel in each line because it macthing the LSB in the entry.
 */
void
draw_string(uint16_t x, uint16_t y, char *str, uint8_t color)
{
	uint32_t index = 0;

  	while (str[index]) {
    		draw_char(x, y, str[index], color);
    		x += BITMAP_SIZE + 1;
    		index++;
  	}
}
