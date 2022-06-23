#ifndef _MOUSE_H_
#define _MOUSE_H_
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/mouse.h>

void mouse_init(void);
void mouse_intr(void);
int mouse_getp(struct mouse_u_pkt *pkt);
void mouse_update_color_backup();

#endif /* _MOUSE_H_ */
