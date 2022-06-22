#ifndef _MOUSE_H_
#define _MOUSE_H_
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/mouse.h>

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
 */
#define	MOUSE_DATA_REG		0x60
#define MOUSE_STATUS_REG	0x64 /* READ */
#define MOUSE_COMMAND_REG	0x64 /* WRITE */

void mouse_init(void);
void mouse_intr(void);
int mouse_getp(struct mouse_u_pkt *pkt);

#endif /* _MOUSE_H_ */
