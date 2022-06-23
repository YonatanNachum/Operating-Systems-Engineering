#ifndef JOS_INC_MOUSE_H
#define JOS_INC_MOUSE_H

#include <inc/types.h>

enum click_type{
	R_CLICK = 0,
	L_CLICK,
	DBL_CLICK,
	NO_CLICK,
};

struct mouse_u_pkt{
	int x;
	int y;
	enum click_type type;
};

#endif /* !JOS_INC_MOUSE_H */
