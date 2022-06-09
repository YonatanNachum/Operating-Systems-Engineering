#include "ns.h"

#define debug 0
#define TAIL_INDEX_DD_FLAG 

extern union Nsipc nsipcbuf[64];

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	uint32_t req, whom;
	int perm, r, tail_index = TAIL_INDEX_DD_FLAG;
	while (1) {
		if ((tail_index & TAIL_INDEX_DD_FLAG) == 0) {
			sys_env_set_status(0, ENV_NOT_RUNNABLE);
			sys_yield();
		}
		tail_index &= ~TAIL_INDEX_DD_FLAG;
		req = ipc_recv((int32_t *) &whom, &nsipcbuf[tail_index], &perm);
		if (whom != ns_envid) {
			cprintf("output: enviroment %u sent a packet and ignored, only %u should send a packet",
				whom, ns_envid);
			continue;
		}
		if (req != NSREQ_OUTPUT) {
			cprintf("output: output enviroment recived a request of type %e, only %e requests are allowed",
				req, NSREQ_OUTPUT);
			continue;
		}
		while ((r = sys_try_transmit(nsipcbuf[tail_index].pkt.jp_data, nsipcbuf[tail_index].pkt.jp_len)) < 0) {
			if (r == -E_INVAL) {
				cprintf("output: packet size too big[%u]", nsipcbuf[tail_index].pkt.jp_len);
				break;
			}
			if (sys_time_msec() > SLEEP_MIN_MS) {
				sys_env_set_status(0, ENV_NOT_RUNNABLE);
			}
			sys_yield();
		}
		tail_index = r;
	}
}
