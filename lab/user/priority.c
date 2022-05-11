#include <inc/lib.h>

volatile int counter;

void
umain(int argc, char **argv)
{
	int i, j;
	int seen;
	envid_t parent = sys_getenvid();

        envid_t child1 = fork();
        if (child1 < 0) {
                panic("fork failed: %e\n", child1);
        }
        if (child1 == 0) {
                sys_change_priority(10);
                for (i = 0; i < 20; ++i) {
                        cprintf("son\n");
                        sys_yield();
                }
        }
        else {
                for (i = 0; i < 20; ++i) {
                        cprintf("father\n");
                        sys_yield();
                }
        }
}
