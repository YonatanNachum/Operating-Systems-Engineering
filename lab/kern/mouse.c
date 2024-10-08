#include <inc/x86.h>
#include <inc/memlayout.h>
#include <inc/kbdreg.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/mouse.h>
#include <kern/picirq.h>
#include <kern/vga.h>
#include <kern/time.h>

/***** PS/2 Controller IO Ports *****/
/* The PS/2 Controller itself uses 2 IO ports (IO ports 0x60 and 0x64).
 * Like many IO ports, reads and writes may access different internal registers.
 * Data port - The Data Port (IO Port 0x60) is used for reading data that was received
 * from a PS/2 device or from the PS/2 controller itself and writing data to a PS/2 device
 * or to the PS/2 controller itself.
 * Status register - The Status Register contains various flags that show the state of the PS/2 controller.
 * Command register - The Command Port (IO Port 0x64) is used for sending commands to the PS/2
 * Controller (not to PS/2 devices).
 * Referance: https://wiki.osdev.org/%228042%22_PS/2_Controller#PS.2F2_Controller_IO_Ports
 * 	      https://wiki.osdev.org/Mouse_Input#Set_Compaq_Status.2FEnable_IRQ12
 */
#define	MOUSE_DATA_REG		0x60
#define MOUSE_STATUS_REG	0x64 /* READ */
#define MOUSE_COMMAND_REG	0x64 /* WRITE */

#define MOUSE_HEIGHT 	18
#define MOUSE_WIDTH 	15

struct GridPos curr_pos = {0, 0};
struct GridPos prev_pos = {0, 0};

static struct {
	uint8_t x_sgn, y_sgn;
	uint8_t l_btn, r_btn, m_btn;
	uint8_t x_overflow, y_overflow;
	int  x_mov, y_mov;
	unsigned int tick;
} packet;

static int lastclicktick;
static int recovery;
static bool color_pick_on;

uint8_t mouse_pointer[MOUSE_HEIGHT][MOUSE_WIDTH] = {
        {2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0},
        {2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 0, 0, 0},
        {2, 1, 1, 1, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 1, 2, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0},
        {2, 1, 2, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0},
        {2, 2, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0}
};

uint8_t color_backup[MOUSE_HEIGHT][MOUSE_WIDTH] = {{0}};

uint8_t mouse_color[2];

#define BUFSIZE 512

static struct {
	struct mouse_u_pkt buf[BUFSIZE];
	uint32_t rpos;
	uint32_t wpos;
} m_pool;

/* Checks buffers status in the PS/2 controller:
 * type = 0 - 	Output buffer status (0 = empty, 1 = full)
		(must be set before attempting to read data from IO port 0x60)
 * type = 1 -   Input buffer status (0 = empty, 1 = full)
		(must be clear before attempting to write data to IO port 0x60 or IO port 0x64)
 */
void
mouse_wait(uint8_t type)
{
	uint32_t time_out = 100000;
	if (type == 0) {
		while (--time_out) {
	    		if((inb(MOUSE_STATUS_REG) & 1) == 1)
				return;
		}
    	} else {
		while (--time_out) {
	    		if((inb(MOUSE_STATUS_REG) & 2) == 0)
				return;
		}
   	}
}

/* Sending Bytes To Second PS/2 Port:
 * 1. Write the command 0xD4 to IO Port 0x64
 * 2. Set up some timer or counter to use as a time-out (mouse_wait)
 * 3. Poll bit 1 of the Status Register ("Input buffer empty/full") until it becomes clear,
 *    or until your time-out expires
 * 4. If the time-out expired, return an error
 * 5. Otherwise, write the data to the Data Port (IO port 0x60)
 */
void
mouse_write(uint8_t word)
{
	mouse_wait(1);
	outb(MOUSE_COMMAND_REG, 0xD4);

	mouse_wait(1);
	outb(MOUSE_DATA_REG, word);
}

uint8_t
mouse_read()
{
	mouse_wait(0);
	return inb(MOUSE_DATA_REG);
}

void
mouse_init(void)
{
	uint8_t statustemp;

	/* Enable second PS/2 port (only if 2 PS/2 ports supported) */
	mouse_wait(1);
	outb(MOUSE_COMMAND_REG, 0xA8);
	mouse_read();

	/* Commands 0x20 and 0x60 let you read and write the PS/2 Controller Configuration Byte:
	* Bit 0 - First PS/2 port interrupt (1 = enabled, 0 = disabled)
	* Bit 1 - Second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2 ports supported)
	*/
	mouse_wait(1);
	outb(MOUSE_COMMAND_REG, 0x20);
	statustemp = mouse_read() | 2;
	mouse_wait(1);
	outb(MOUSE_COMMAND_REG, 0x60);
	mouse_wait(1);
	outb(MOUSE_DATA_REG, statustemp);

    	/* Set Defaults */
    	mouse_write(0xF6);
    	mouse_read();

    	/* Set Sample rate to 100 */
	mouse_write(0xF3);
    	mouse_read();
    	mouse_write(10);
    	mouse_read();

    	// /* Set Resolution */
	// mouse_write(0xE8);
    	// mouse_read();
    	// mouse_write(0x3);
    	// mouse_read();

    	/* Enable Data Reporting */
    	mouse_write(0xF4);
	statustemp = mouse_read();
    	if(statustemp != 0xFA) {
        	panic("error: failed to enable mouse packet streaming\n");
    	}

	irq_setmask_8259A(irq_mask_8259A & ~(1<<12));

    	mouse_color[0] = VGA_GREY;
	mouse_color[1] = VGA_WHITE;

    	lastclicktick = -1000;
	curr_pos.x = SCREEN_WIDTH / 2;
	curr_pos.y = SCREEN_HEIGHT / 2;
}

void mouse_update_color_backup()
{
    	int i, j;
	uint8_t *addr;
	int x = curr_pos.x;
	int y = curr_pos.y;

    	for (i = 0; i < MOUSE_HEIGHT; i++) {
        	if (y + i >= SCREEN_HEIGHT || y + i < 0) {
            		break;
        	}
        	for (j = 0; j < MOUSE_WIDTH; j++) {
            		if (x + j >= SCREEN_WIDTH || x + j < 0) {
                		break;
            		}
			
            		uint8_t temp = mouse_pointer[i][j];
            		if (temp) {
                		addr = SCREEN_BASE_ADDR + (y + i) * SCREEN_WIDTH + x + j;
				color_backup[i][j] = *addr;
            		}
        	}
    	}
}

void 
drawMouse(int x, int y, bool recover_old) {
    	int i, j;
	uint8_t *addr;
    	for (i = 0; i < MOUSE_HEIGHT; i++) {
        	if (y + i >= SCREEN_HEIGHT || y + i < 0) {
            		break;
        	}
        	for (j = 0; j < MOUSE_WIDTH; j++) {
            		if (x + j >= SCREEN_WIDTH || x + j < 0) {
                		break;
            		}
			
            		uint8_t temp = mouse_pointer[i][j];
            		if (temp) {
                		addr = SCREEN_BASE_ADDR + (y + i) * SCREEN_WIDTH + x + j;
				if (recover_old) {
					*addr = color_backup[i][j];
				} else {
					color_backup[i][j] = *addr;
					*addr = mouse_color[temp - 1];
				}	
            		}
        	}
    	}
}

static void
mouse_command()
{
	struct mouse_u_pkt pkt;

	if (packet.x_overflow || packet.y_overflow) {
		return;
	}

	int btns = packet.l_btn | (packet.r_btn << 1) | (packet.m_btn << 2);

	if (packet.x_mov || packet.y_mov) {
		prev_pos.x = curr_pos.x;
		prev_pos.y = curr_pos.y;
		curr_pos.x += (packet.x_mov >> 5);
		curr_pos.y += -(packet.y_mov >> 5);
		if (curr_pos.x < 0) {
			curr_pos.x = 0;	
		}
		if (curr_pos.x > SCREEN_WIDTH) {
			curr_pos.x = SCREEN_WIDTH;	
		}
		if (curr_pos.y < 0) {
			curr_pos.y = 0;	
		}
		if (curr_pos.y > SCREEN_HEIGHT) {
			curr_pos.y = SCREEN_HEIGHT;	
		}
		drawMouse(prev_pos.x, prev_pos.y, true);
		drawMouse(curr_pos.x, curr_pos.y, false);

		//lastclicktick = -1000;
  	}
	if (btns) {
		if (packet.l_btn) {
			if (packet.tick - lastclicktick < 200) {
				//cprintf("double\n");
				pkt.type = DBL_CLICK;
			} else {
				//cprintf("single\n");
				pkt.type = L_CLICK;
			}
			//remove_color_pick();
		}
		if (packet.r_btn) { 
			//cprintf("Right Click\n");
			pkt.type = R_CLICK;
			//draw_color_pick(curr_pos.x + MOUSE_WIDTH, curr_pos.y);
		}
		if (packet.m_btn) {
			cprintf("Middle\n");
			drawMouse(prev_pos.x, prev_pos.y, true);
			drawMouse(curr_pos.x, curr_pos.y, false);
			curr_pos.x = SCREEN_WIDTH / 2;
			curr_pos.y = SCREEN_HEIGHT / 2;
		}
		pkt.x = prev_pos.x;
		pkt.y = prev_pos.y;
		m_pool.buf[m_pool.wpos++] = pkt;
		if (m_pool.wpos == BUFSIZE)
			m_pool.wpos = 0;		
		lastclicktick = packet.tick;
	}
}

/*Mouse-to-host communication:
 * When in the streaming mode of operation, mouse will periodically send the data
 *packets containing the following information:
 * 1. Button state - 3 bits representing the state of the left, right and middle buttons.
 * 2. Relative position - two 9-bit 2's-complement numbers representing the amount of movement
      that has occurred since the last transmission.
 * Data packet format illustrated:
 *  	Bit 7		Bit 6		Bit 5	Bit 4	Bit 3		Bit 2		Bit 1		Bit 0
Byte 0	Y overflow	X overflow	Y sign	X sign	Always 1	Middle button	Right button	Left button
Byte 1	********************************************* X movement ***********************************************
Byte 2	********************************************* Y movement ***********************************************
 *
 *  */
void
mouse_intr(void)
{
	int state, count = 0, first_byte = 0;
	unsigned int ticks = time_msec();

	/* As long as the Input buffer is full keep reading*/
	while (((state = inb(MOUSE_STATUS_REG)) & 1) == 1) {
		int data = inb(MOUSE_DATA_REG);
		count++;

	  	if (recovery == 0 && (data & 255) == 0)
		  	recovery = 1;
	  	else if (recovery == 1 && (data & 255) == 0)
		  	recovery = 2;
	  	else if ((data & 255) == 12)
		  	recovery = 0;
	  	else
		 	recovery = -1;

    		switch (count) {
		case 1: 
			if(data & 0x08) {
				ticks = time_msec();
				packet.y_overflow = (data >> 7) & 0x1;
				packet.x_overflow = (data >> 6) & 0x1;
				packet.y_sgn = (data >> 5) & 0x1;
				packet.x_sgn = (data >> 4) & 0x1;
				packet.m_btn = (data >> 2) & 0x1;
				packet.r_btn = (data >> 1) & 0x1;
				packet.l_btn = (data >> 0) & 0x1;
				first_byte = data;
			} else {
				count = 0;
			}
			break;

		case 2:
			packet.x_mov = data - ((first_byte << 4) & 0x100);
			break;

		case 3:
			packet.y_mov = data - ((first_byte << 3) & 0x100); 
			packet.tick = ticks;
			break;

		default:
			count = 0;
			break;
		}
		if (recovery == 2) {
			count = 0;
			recovery = -1;
	  	} else if (count == 3) {
			count = 0;
			mouse_command();
	  	}
	}
}

// Here we manage the mouse input buffer,
// where we stash packets received from the mouse
// whenever the corresponding interrupt occurs.

void
mouse_poll(void)
{
	uint8_t status;
	if (((status = inb(MOUSE_STATUS_REG)) & 1) == 0)
		return;

	/* Checks if this is a mouse packet and if so process it*/
	if (status & 0x20) {
		mouse_intr();
	}
}

// return the next input packet if there was a click, or 0 if none waiting
int
mouse_getp(struct mouse_u_pkt *pkt)
{
	// poll for any pending input packets,
	// so that this function works even when interrupts are disabled
	mouse_poll();

	// grab the next mouse packet from the input buffer.
	if (m_pool.rpos != m_pool.wpos) {
		*pkt = m_pool.buf[m_pool.rpos++];
		if (m_pool.rpos == BUFSIZE)
			m_pool.rpos = 0;
		return 0;
	}
	pkt->type = NO_CLICK;
	return -1;
}
