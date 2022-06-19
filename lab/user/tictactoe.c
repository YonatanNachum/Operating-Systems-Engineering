#include <inc/lib.h>

#define BOX_SIZE 40

#define PLAYER_1 1
#define PLAYER_2 2

#define UP_KEY   226
#define DOWN_KEY 227
#define RIGHT_KEY 229
#define LEFT_KEY 228
#define SPACE_KEY 32

uint8_t row = 0, col = 0;
uint8_t turn = PLAYER_1;

uint16_t player_1_moves = 0;
uint16_t player_2_moves = 0;

uint16_t grid_box_x = 0;
uint16_t grid_box_y = 0;
uint8_t player_1_cell_color = VGA_BRIGHT_RED;
uint8_t player_2_cell_color = VGA_BRIGHT_BLUE;

bool error = false;

uint8_t grid[3][3];
uint16_t game_board[3][3][2] = {
        {{180, 40}, {220, 40}, {260, 40}},
        {{180, 80}, {220, 80}, {260, 80}},
        {{180, 120}, {220, 120}, {260, 120}},
};


static void
sleep(int msec)
{
	unsigned current = sys_time_msec();
	unsigned end = current + msec;

	while (sys_time_msec() < end)
		sys_yield();
}

void update_cells()
{
        uint8_t box_center = BOX_SIZE / 2;
        uint8_t i, j;

        for (i = 0; i < 3; ++i) {
                for (j = 0; j < 3; ++j) {
                        if (grid[i][j] == PLAYER_1) {
                                draw_circle(game_board[i][j][0] + box_center, game_board[i][j][1] + box_center,
                                            15, player_1_cell_color, VGA_NO_COLOR);
                        } else if (grid[i][j] == PLAYER_2) {
                                draw_circle(game_board[i][j][0] + box_center, game_board[i][j][1] + box_center,
                                            15, player_2_cell_color, VGA_NO_COLOR);
                                //draw_rectangle(game_board[i][j][0] + 10, game_board[i][j][0] + 10, 10, 10,
                                //               player_2_cell_color, VGA_NO_COLOR);
                        }
                }
        }
}

void
draw_game_board()
{
        char str[32];

        draw_rectangle(game_board[0][0][0], game_board[0][0][1], BOX_SIZE, BOX_SIZE, VGA_WHITE, VGA_BLACK);
        draw_rectangle(game_board[0][1][0], game_board[0][1][1], BOX_SIZE, BOX_SIZE, VGA_WHITE, VGA_BLACK);
        draw_rectangle(game_board[0][2][0], game_board[0][2][1], BOX_SIZE, BOX_SIZE, VGA_WHITE, VGA_BLACK);

        draw_rectangle(game_board[1][0][0], game_board[1][0][1], BOX_SIZE, BOX_SIZE, VGA_WHITE, VGA_BLACK);
        draw_rectangle(game_board[1][1][0], game_board[1][1][1], BOX_SIZE, BOX_SIZE, VGA_WHITE, VGA_BLACK);
        draw_rectangle(game_board[1][2][0], game_board[1][2][1], BOX_SIZE, BOX_SIZE, VGA_WHITE, VGA_BLACK);

        draw_rectangle(game_board[2][0][0], game_board[2][0][1], BOX_SIZE, BOX_SIZE, VGA_WHITE, VGA_BLACK);
        draw_rectangle(game_board[2][1][0], game_board[2][1][1], BOX_SIZE, BOX_SIZE, VGA_WHITE, VGA_BLACK);
        draw_rectangle(game_board[2][2][0], game_board[2][2][1], BOX_SIZE, BOX_SIZE, VGA_WHITE, VGA_BLACK);

        update_cells();

        draw_rectangle(game_board[grid_box_x][grid_box_y][0], game_board[grid_box_x][grid_box_y][1],
                       BOX_SIZE, BOX_SIZE, VGA_GREEN, VGA_NO_COLOR);

        draw_string(0, 0, "TIC TAC TOE", VGA_YELLOW);

        draw_rectangle(1, 17, 102, 24, VGA_GREY, VGA_NO_COLOR);
      
        draw_string(3, 20, "P1 MOVES", VGA_BRIGHT_RED);
        itoa(player_1_moves, str);
        draw_string(90, 20, str, VGA_BRIGHT_RED);

        draw_string(3, 32, "P2 MOVES", VGA_BRIGHT_BLUE);
        itoa(player_2_moves, str);
        draw_string(90, 32, str, VGA_BRIGHT_BLUE);

        draw_string(3, 45, "TURN", VGA_CYAN);
        if (turn == PLAYER_1) {
                draw_string(45, 45, "P1", VGA_BRIGHT_CYAN);
        } else {
                draw_string(45, 45, "P2", VGA_BRIGHT_CYAN);
        }

        draw_rectangle(1, 70, 100, 60, VGA_GREY, VGA_NO_COLOR);

        // print_color_string("Keys", WHITE, BLACK);
        draw_string(1, 61, "KEYS", VGA_WHITE);


        draw_string(3, 75, "USE ARROWS", VGA_WHITE);

        draw_string(3, 100, "SELECT USE", VGA_WHITE);
        draw_string(3, 110, "SPACEBAR", VGA_WHITE);

        if (error == true) {
                draw_string(1, 150, "CELL IS ALREADY", VGA_RED);
                draw_string(1, 160, "SELECTED", VGA_RED);
                error = false;
        }
}

static void
restore_game_data_to_default()
{       
        uint8_t i,j;

        for (i = 0; i < 3; i++) {
                for (j = 0; j < 3; j++) {
                        grid[i][j] = 0;
                }
        }

        row = 0;
        col = 0;
        turn = PLAYER_1;

        player_1_moves = 0;
        player_2_moves = 0;

        grid_box_x = 0;
        grid_box_y = 0;
}

static int 
get_winner()
{
        int winner = 0;
        int i;
        //each row
        for (i = 0; i < 3; i++)
        {
                if ((grid[i][0] & grid[i][1] & grid[i][2]) == PLAYER_1)
                {
                        winner = PLAYER_1;
                        break;
                }
                else if ((grid[i][0] & grid[i][1] & grid[i][2]) == PLAYER_2)
                {
                        winner = PLAYER_2;
                        break;
                }
        }
        //each column
        if (winner == 0)
        {
                for (i = 0; i < 3; i++)
                {
                        if ((grid[0][i] & grid[1][i] & grid[2][i]) == PLAYER_1)
                        {
                                winner = PLAYER_1;
                                break;
                        }
                        else if ((grid[0][i] & grid[1][i] & grid[2][i]) == PLAYER_2)
                        {
                                winner = PLAYER_2;
                                break;
                        }
                }
        }

        if (winner == 0)
        {
                if ((grid[0][0] & grid[1][1] & grid[2][2]) == PLAYER_1)
                        winner = PLAYER_1;
                else if ((grid[0][0] & grid[1][1] & grid[2][2]) == PLAYER_2)
                        winner = PLAYER_2;
                if ((grid[2][0] & grid[1][1] & grid[0][2]) == PLAYER_1)
                        winner = PLAYER_1;
                else if ((grid[2][0] & grid[1][1] & grid[0][2]) == PLAYER_2)
                        winner = PLAYER_2;
        }

        return winner;
}

static
void finish()
{
    uint8_t b = 0;

    clear_screen();
    draw_string(120, 15, "NICE PLAY", VGA_BRIGHT_GREEN);
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

void
tic_tac_toe_game()
{
        uint8_t keycode = 0;
        int winner = 0;

        restore_game_data_to_default();

        draw_game_board();

        do {
                keycode = getchar();
                switch (keycode) {
                case RIGHT_KEY:
                        if (grid_box_y < 2) {
                                grid_box_y++;
                                col++;
                        }
                        break;

                case LEFT_KEY:
                        if (grid_box_y > 0) {
                                grid_box_y--;
                                col--;
                        }
                        // else
                        // {
                        //         grid_inner_box_x = 30;
                        //         col = 0;
                        // }
                        break;

                case DOWN_KEY:
                        if (grid_box_x < 2) {
                                grid_box_x++;
                                row++;
                        }
                        break;

                case UP_KEY:
                        if (grid_box_x > 0) {
                                grid_box_x--;
                                row--;
                        }
                        break;

                case SPACE_KEY:
                        if (grid[row][col] > 0) {
                                error = true;
                        }
                        if (turn == PLAYER_1) {
                                grid[row][col] = PLAYER_1;
                                player_1_moves++;
                                turn = PLAYER_2;
                        } else if (turn == PLAYER_2) {
                                grid[row][col] = PLAYER_2;
                                player_2_moves++;
                                turn = PLAYER_1;
                        }
                        break;
                }
                clear_screen();
                draw_game_board();
                if (player_1_moves >= 3 || player_2_moves >= 3) {
                        winner = get_winner();
                        if (winner == PLAYER_1) {
                                draw_rectangle(5, 148, 72, 20, VGA_GREEN, VGA_NO_COLOR);
                                draw_rectangle(7, 150, 68, 16, VGA_GREEN, VGA_NO_COLOR);
                                draw_string(8, 154, "P1 WINS", VGA_BRIGHT_GREEN);
                                break;
                        } else if (winner == PLAYER_2) {
                                draw_rectangle(5, 148, 72, 20, VGA_GREEN, VGA_NO_COLOR);
                                draw_rectangle(7, 150, 68, 16, VGA_GREEN, VGA_NO_COLOR);
                                draw_string(8, 154, "P2 WINS", VGA_BRIGHT_GREEN);
                                break;
                        }
                }
                if (player_1_moves + player_2_moves == 9) {
                        draw_rectangle(5, 148, 72, 20, VGA_CYAN, VGA_NO_COLOR);
                        draw_rectangle(7, 150, 68, 16, VGA_CYAN, VGA_NO_COLOR);
                        draw_string(22, 154, "DRAW", VGA_BRIGHT_CYAN);
                        break;
                }
        } while (keycode > 0);
        sleep(1000);
        finish();
}


void
umain(int argc, char **argv)
{
        int i, r;

        cprintf("Playing Tic-Tac-Toe!\n");

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

        tic_tac_toe_game();
}
