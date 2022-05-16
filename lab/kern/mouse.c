#include <inc/x86.h>
#include <inc/memlayout.h>
#include <inc/kbdreg.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/mouse.h>
#include <kern/picirq.h>
#include <kern/vga.h>
#include <kern/time.h>


struct mousePos {
	int x;
	int y;
} MousePos;

static struct {
	int x_sgn, y_sgn, x_mov, y_mov;
	int l_btn, r_btn, m_btn;
	int x_overflow, y_overflow;
	unsigned int tick;
} packet;

static int count;
static int recovery;
static int lastbtn, lastdowntick, lastclicktick;

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
	/* Disable Devices */
	//mouse_wait(1);
	//outb(MOUSE_COMMAND_REG, 0xAD);
	// mouse_wait(1);
	// outb(MOUSE_COMMAND_REG, 0xA7);

	/* Flush The Output Buffer */
	// mouse_wait(0);

	/* Set the Controller Configuration Byte -
	 * disable all IRQs and disable translation 
	 */
	// mouse_wait(1);
	// outb(MOUSE_COMMAND_REG, 0x20);
	// statustemp = mouse_read() & ~(0x43);
	// mouse_wait(1);
	// outb(MOUSE_COMMAND_REG, 0x60);
	// mouse_wait(1);
	// outb(MOUSE_DATA_REG, statustemp);
	
	// /* Perform Controller Self Test */
	// mouse_wait(1);
	// outb(MOUSE_COMMAND_REG, 0xAA);
	// if (mouse_read() != 0x55) {
	// 	//panic("PS/2 controller self test failed\n");
	// }
	// mouse_wait(1);
	// outb(MOUSE_COMMAND_REG, 0x60);
	// mouse_wait(1);
	// outb(MOUSE_DATA_REG, statustemp);

	// /* Perform Interface Tests */
	// mouse_wait(1);
	// outb(MOUSE_COMMAND_REG, 0xA9);
	// if (mouse_read() != 0) {
	// 	//panic("PS/2 controller interface test failed\n");
	// }

	/* Enable first PS/2 port */
	//mouse_wait(1);
	//outb(MOUSE_COMMAND_REG, 0xAE);

	/* Enable second PS/2 port (only if 2 PS/2 ports supported) */
	mouse_wait(1);
	outb(MOUSE_COMMAND_REG, 0xA8);

	/* Commands 0x20 and 0x60 let you read and write the PS/2 Controller Configuration Byte:
	* Bit 0 - First PS/2 port interrupt (1 = enabled, 0 = disabled)
	* Bit 1 - Second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2 ports supported)
	*/
	mouse_wait(1);
	outb(MOUSE_COMMAND_REG, 0x20);
	statustemp = mouse_read() | 2;
	//cprintf("status: 0x%x!!\n", statustemp);
	mouse_wait(1);
	outb(MOUSE_COMMAND_REG, 0x60);
	mouse_wait(1);
	outb(MOUSE_DATA_REG, statustemp);

	// /* Reset mouse */
	// mouse_write(0XFF);
	// if (mouse_read() != 0xFA) {
	// 	//panic("Mouse reset failed\n");
	// }

    	/* Set Defaults */
    	mouse_write(0xF6);
    	mouse_read();

    	/* Set Sample rate to 10 */
	/*mouse_write(0xF3);
    	mouse_read();
    	mouse_write(10);
    	mouse_read();*/

    	/* Enable Data Reporting */
    	mouse_write(0xF4);
	irq_setmask_8259A(irq_mask_8259A & ~(1<<12));

    	count = 0;
    	lastclicktick = lastdowntick = -1000;
}

typedef struct message {
    int msg_type;
    int params[10];
} message;
/*
void
genMouseUpMessage(int btns)
{
  message msg;
  msg.msg_type = M_MOUSE_UP;
  msg.params[0] = btns;
  handleMessage(&msg);
}

void
genMouseMessage()
{
	if (packet.x_overflow || packet.y_overflow) {
		return;
	}

  	int btns = packet.l_btn | (packet.r_btn << 1) | (packet.m_btn << 2);
  	message msg;
  	if (packet.x_mov || packet.y_mov) {
		msg.msg_type = M_MOUSE_MOVE;
		msg.params[0] = packet.x_mov;
		msg.params[1] = packet.y_mov;
		msg.params[2] = btns;
		lastdowntick = lastclicktick = -1000;
		if (btns != lastbtn) {
			genMouseUpMessage(btns);
		}
  	} else if (btns) {
		msg.msg_type = M_MOUSE_DOWN;
		msg.params[0] = btns;
		lastdowntick = packet.tick;
	} else if (packet.tick - lastdowntick < 30) {
		if (lastbtn & 1) {
			msg.msg_type = M_MOUSE_LEFT_CLICK;
		} else {
			msg.msg_type = M_MOUSE_RIGHT_CLICK;
		}
    		if (packet.tick - lastclicktick < 60) {
      			msg.msg_type = M_MOUSE_DBCLICK;
      			lastclicktick = -1000;
    		} else {
			lastclicktick = packet.tick;
		}
	} else {
    		genMouseUpMessage(btns);
	}
	lastbtn = btns;
	handleMessage(&msg);
}*/

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
	cprintf("hello\n");
	int state, first_byte = 0;
	unsigned int ticks = time_msec() / 10;

	/* As long as the Input buffer is full keep reading*/
	while (((state = inb(MOUSE_STATUS_REG)) & 1) == 1) {
		int data = inb(MOUSE_DATA_REG);
		count++;

		if (recovery == 0 && (data & 255) == 0) {
			recovery = 1;
		} else if (recovery == 1 && (data & 255) == 0) {
			recovery = 2;
		} else if ((data & 255) == 12) {
			recovery = 0;
		} else {
			recovery = -1;
		}

    		switch (count) {
		case 1: 
			if(data & 0x08) {
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
			packet.y_mov = data - ((first_byte << 3) & 0x100);;
			packet.tick = ticks;
			break;
		}
	}
}