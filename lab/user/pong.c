#include <inc/lib.h>

// size of bounced rectangle
#define RECT_SIZE      6

#define RECT_SPEED_X   6
#define RECT_SPEED_Y   6

// pad position, size & pads speeds
#define PAD_POS_X    320 - PAD_WIDTH - 1
#define PAD_WIDTH    6
#define PAD_HEIGHT   50
#define PAD_SPEED    20

#define UP_KEY   226
#define DOWN_KEY 227

// pads position y, will change on keys
uint16_t pad_pos_y = 2;
// score count
uint32_t score_count = 0;
int r;

static void
sleep(int msec)
{
	unsigned current = sys_time_msec();
	unsigned end = current + msec;

	while (sys_time_msec() < end)
		sys_yield();
}

// initialize game
static
void init_game()
{
        uint8_t b = 0;

        draw_string(120, 13, "PONG GAME", VGA_CYAN );
        draw_rectangle(100, 4, 120, 25, VGA_BLUE, VGA_NO_COLOR);
        draw_string(10, 50, "HOW TO PLAY", VGA_MAGENTA);
        draw_rectangle(2, 40, 235, 80, VGA_BROWN, VGA_NO_COLOR);
        draw_string(10, 70, "ARROW KEY UP", VGA_BRIGHT_RED);
        draw_string(30, 80, "TO MOVE BOTH PADS UP", VGA_WHITE);
        draw_string(10, 90, "ARROW KEY DOWN", VGA_BRIGHT_RED);
        draw_string(30, 100, "TO MOVE BOTH PADS DOWN", VGA_WHITE);
        draw_string(60, 160, "PRESS ENTER TO START", VGA_BRIGHT_GREEN);

        while (1) {
                b = sys_cgetc();
                if (b == '\n') {
                        break;
                }
                b = 0;
        }
        clear_screen();
}

// if lose then display final score & quit
static
void lose()
{
    uint8_t b = 0;
    char str[32];

    itoa(score_count, str);
    clear_screen();
    draw_string(120, 15, "NICE PLAY", VGA_BRIGHT_GREEN);
    draw_string(125, 45, "SCORE", VGA_WHITE);
    draw_string(180, 45, str, VGA_WHITE);
    draw_string(45, 130, "PRESS ENTER TO QUIT", VGA_YELLOW);

        while (1) {
                b = sys_cgetc();
                if (b == '\n') {
                        break;
                }
                b = 0;
        }
        clear_screen();
}

// move both pads simultaneously on pressed keys
static void
move_pads()
{
        uint8_t b;

        // draw both pads
        draw_rectangle(0, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VGA_YELLOW, VGA_YELLOW);
        draw_rectangle(PAD_POS_X, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VGA_YELLOW, VGA_YELLOW);

        b = sys_cgetc();
        // if down key pressed, move both pads down
        if (b == DOWN_KEY) {
                if (pad_pos_y < 200 - PAD_HEIGHT) {
                        pad_pos_y = pad_pos_y + PAD_SPEED;
                }
                draw_rectangle(0, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VGA_YELLOW, VGA_YELLOW);
                draw_rectangle(PAD_POS_X, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VGA_YELLOW, VGA_YELLOW);
        }
        // if up key pressed, move both pads up
        else if (b == UP_KEY) {
                if (pad_pos_y >= PAD_WIDTH) {
                        pad_pos_y = pad_pos_y - PAD_SPEED;
                }
                draw_rectangle(0, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VGA_YELLOW, VGA_YELLOW);
                draw_rectangle(PAD_POS_X, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VGA_YELLOW, VGA_YELLOW);
        }
}

// update score count text
static void
update_score_count()
{
    char str[32];

    itoa(score_count, str);
    draw_string(150, 2, str, VGA_WHITE);
}

void
pong_game()
{
        uint16_t rect_pos_x = RECT_SIZE + 20;
        uint16_t rect_pos_y = RECT_SIZE;
        uint16_t rect_speed_x = RECT_SPEED_X;
        uint16_t rect_speed_y = RECT_SPEED_Y;

        init_game();

        while (1) {
                // add speed values to positions
                rect_pos_x += rect_speed_x;
                rect_pos_y += rect_speed_y;

                // check if position x < left pad position x
                if (rect_pos_x - RECT_SIZE <= PAD_WIDTH + 1) {
                        // if position of rect is not between left pad position,
                        // then lose, bounced rect is not in y range of pad
                        if ((rect_pos_y > 0 && rect_pos_y < pad_pos_y) || 
                            (rect_pos_y <= 200 && rect_pos_y > pad_pos_y + PAD_HEIGHT)) {
                                lose();
                                return;
                        } else {
                                // set speed x to negative, means move opposite direction
                                rect_speed_x = -rect_speed_x;
                                // set position x to rect size
                                rect_pos_x = PAD_WIDTH + RECT_SIZE;
                                // increase score
                                score_count++;
                        }
                }
                // check if position x >= right pad position x
                else if (rect_pos_x + RECT_SIZE >= PAD_POS_X + RECT_SIZE - 1) {
                        // in range of y pad position
                        if ((rect_pos_y > 0 && rect_pos_y < pad_pos_y) || 
                            (rect_pos_y <= 200 && rect_pos_y > pad_pos_y + PAD_HEIGHT) || 
                            (rect_pos_y + RECT_SIZE > 0 && rect_pos_y + RECT_SIZE < pad_pos_y)) {
                                lose();
                                return;
                        } else {
                                // set speed x to negative, means move opposite direction
                                rect_speed_x = -rect_speed_x;
                                // set position x to minimum of pad position x - rect size
                                rect_pos_x = PAD_POS_X - RECT_SIZE;
                                // increase score
                                score_count++;
                        }
                }

                // change rect y position by checking boundries
                if (rect_pos_y - RECT_SIZE <= 0) {
                        rect_speed_y = -rect_speed_y;
                        rect_pos_y = RECT_SIZE;
                } else if(rect_pos_y + RECT_SIZE > 200 + RECT_SIZE) {
                        rect_speed_y = -rect_speed_y;
                        rect_pos_y = 200 - RECT_SIZE;
                }

                // clear screen for repaint
                clear_screen();
                // move pads on keys
                move_pads();
                // update score count
                update_score_count();
                // fill bounced rect
                draw_rectangle(rect_pos_x - RECT_SIZE, rect_pos_y - RECT_SIZE, RECT_SIZE, RECT_SIZE,
                               VGA_WHITE, VGA_WHITE);
                sleep(100);
        }
}

void
umain(int argc, char **argv)
{
        int i;

        // Spin for a bit to let the console quiet
	for (i = 0; i < 10; ++i)
		sys_yield();

        cprintf("Playing Ping Pong!\n");
	close(0);
	if ((r = opencons()) < 0)
		panic("opencons: %e", r);
	if (r != 0)
		panic("first opencons used fd %d", r);
	if ((r = dup(0, 1)) < 0)
		panic("dup: %e", r);
        
        pong_game();
}
