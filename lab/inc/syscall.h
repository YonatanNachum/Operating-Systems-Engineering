#ifndef JOS_INC_SYSCALL_H
#define JOS_INC_SYSCALL_H

/* system call numbers */
enum {
	SYS_cputs = 0,
	SYS_cgetc,
	SYS_getenvid,
	SYS_env_destroy,
	SYS_page_alloc,
	SYS_page_map,
	SYS_page_unmap,
	SYS_exofork,
	SYS_env_set_status,
	SYS_env_set_trapframe,
	SYS_env_set_pgfault_upcall, 
	SYS_yield,
	SYS_ipc_try_send,
	SYS_ipc_recv,
	SYS_time_msec,
	SYS_change_priority,
	SYS_try_transmit,
	SYS_receive,
	SYS_env_set_type,
	SYS_get_mac_addr,
	SYS_draw,
	SYS_clear_screen,
	SYS_free_rx_buf,
	SYS_mouse_getp,
	NSYSCALLS
};

#endif /* !JOS_INC_SYSCALL_H */
