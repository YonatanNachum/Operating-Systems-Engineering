#include "ns.h"

extern union Nsipc nsipcbuf;


void
sleep(int msec)
{
	unsigned now = sys_time_msec();
	unsigned end = now + msec;

	if ((int)now < 0 && (int)now > -MAXERROR)
		panic("sys_time_msec: %e", (int)now);
	if (end < now)
		panic("sleep: wrap");

	while (sys_time_msec() < end)
		sys_yield();
}


void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	int32_t req = 0, whom;

	// MUST HAVE THIS TO PREVENT PAGE FUALTS IN KERNEL
	memset(nsipcbuf._pad, 0, sizeof(nsipcbuf));

	while (1) {
		while ((nsipcbuf.pkt.jp_len = sys_receive(nsipcbuf.pkt.jp_data)) == -E_RX_POOL_EMPTY) {
			sys_yield();
		}
		ipc_send(ns_envid, NSREQ_INPUT, &nsipcbuf, PTE_P | PTE_U);
		// do{
		// 	req = ipc_recv((int32_t *) &whom, NULL, NULL);
		// 	if (whom != ns_envid) {
		// 		req = 0;
		// 		continue;
		// 	}
		// } while(req != NSREQ_COMPLETE);
		sleep(50);
	}
}

