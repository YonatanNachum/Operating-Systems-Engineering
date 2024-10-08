// System call stubs.

#include <inc/syscall.h>
#include <inc/lib.h>

static inline int32_t
syscall(int num, int check, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret;

	// Generic system call: pass system call number in AX,
	// up to five parameters in DX, CX, BX, DI, SI.
	// Interrupt kernel with T_SYSCALL.
	//
	// The "volatile" tells the assembler not to optimize
	// this instruction away just because we don't use the
	// return value.
	//
	// The last clause tells the assembler that this can
	// potentially change the condition codes and arbitrary
	// memory locations.

	asm volatile("int %1\n"
		: "=a" (ret)
		: "i" (T_SYSCALL),
		  "a" (num),
		  "d" (a1),
		  "c" (a2),
		  "b" (a3),
		  "D" (a4),
		  "S" (a5)
		: "cc", "memory");

	if(check && ret > 0)
		panic("syscall %d returned %d (> 0)", num, ret);

	return ret;
}

void
sys_cputs(const char *s, size_t len)
{
	syscall(SYS_cputs, 0, (uint32_t)s, len, 0, 0, 0);
}

int
sys_cgetc(void)
{
	return syscall(SYS_cgetc, 0, 0, 0, 0, 0, 0);
}

int
sys_env_destroy(envid_t envid)
{
	return syscall(SYS_env_destroy, 1, envid, 0, 0, 0, 0);
}

envid_t
sys_getenvid(void)
{
	 return syscall(SYS_getenvid, 0, 0, 0, 0, 0, 0);
}

void
sys_yield(void)
{
	syscall(SYS_yield, 0, 0, 0, 0, 0, 0);
}

int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	return syscall(SYS_page_alloc, 1, envid, (uint32_t) va, perm, 0, 0);
}

int
sys_page_map(envid_t srcenv, void *srcva, envid_t dstenv, void *dstva, int perm)
{
	return syscall(SYS_page_map, 1, srcenv, (uint32_t) srcva, dstenv, (uint32_t) dstva, perm);
}

int
sys_page_unmap(envid_t envid, void *va)
{
	return syscall(SYS_page_unmap, 1, envid, (uint32_t) va, 0, 0, 0);
}

// sys_exofork is inlined in lib.h

int
sys_env_set_status(envid_t envid, int status)
{
	return syscall(SYS_env_set_status, 1, envid, status, 0, 0, 0);
}

int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	return syscall(SYS_env_set_trapframe, 1, envid, (uint32_t) tf, 0, 0, 0);
}

int
sys_env_set_pgfault_upcall(envid_t envid, void *upcall)
{
	return syscall(SYS_env_set_pgfault_upcall, 1, envid, (uint32_t) upcall, 0, 0, 0);
}

int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, int perm)
{
	return syscall(SYS_ipc_try_send, 0, envid, value, (uint32_t) srcva, perm, 0);
}

int
sys_ipc_recv(void *dstva)
{
	return syscall(SYS_ipc_recv, 1, (uint32_t)dstva, 0, 0, 0, 0);
}

unsigned int
sys_time_msec(void)
{
	return (unsigned int) syscall(SYS_time_msec, 0, 0, 0, 0, 0, 0);
}

int
sys_change_priority(uint8_t p)
{
	return syscall(SYS_change_priority, 0, p, 0, 0, 0, 0);
}

/* Returns the next possibly free desc index with bit 30 
 * indicating if the desc if free, if 0 the tx desc pool is full.
 * return value bit map:
 * [0-5] - desc index
 * [6-29] - reserved
 * [30] - desc free bit
 * [31] - reserved
 */
int
sys_try_transmit(void *data, uint16_t len)
{
	return syscall(SYS_try_transmit, 0, (uint32_t)data, len, 0, 0, 0);
}

int 
sys_receive(uint32_t *buf_idx)
{
	return syscall(SYS_receive, 0, (uint32_t)buf_idx, 0, 0, 0, 0);
}

int 
sys_env_set_type(enum EnvType type)
{
	return syscall(SYS_env_set_type, 0, (uint32_t)type, 0, 0, 0, 0);
}

int 
sys_get_mac_addr(uint8_t *addr)
{
	return syscall(SYS_get_mac_addr, 0, (uint32_t)addr, 0, 0, 0, 0);
}

/* Returns the index of the shape in the kernel data structures for furthur use
 * or -E_INVAL if there is an error.
 */
int 
sys_draw(struct draw_type *shape)
{
	return syscall(SYS_draw, 0, (uint32_t)shape, 0, 0, 0, 0);
}

int 
sys_clear_screen(void)
{
	return syscall(SYS_clear_screen, 0, 0, 0, 0, 0, 0);
}

int
sys_free_rx_buf()
{
	return syscall(SYS_free_rx_buf, 0, 0, 0, 0, 0, 0);
}

int
sys_mouse_getp(struct mouse_u_pkt *pkt)
{
	return syscall(SYS_mouse_getp, 0, (uint32_t)pkt, 0, 0, 0, 0); 
}
