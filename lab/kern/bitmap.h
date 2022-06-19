#ifndef _BITMAP_H_
#define _BITMAP_H_
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/types.h>

#define BITMAP_SIZE 8

void draw_char(uint16_t x, uint16_t y, char ch, uint8_t color);
void draw_string(uint16_t x, uint16_t y,  char *str, uint8_t color);

#endif