// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if ((err & FEC_WR) == 0 || (uvpt[PGNUM(addr)] & PTE_COW) == 0) {
		panic("pgfault: faulting access was not a write and copy-on-write page\n");
	}
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	addr = ROUNDDOWN(addr, PGSIZE);
	r = sys_page_alloc(0, (void *)PFTEMP, PTE_U | PTE_W | PTE_P);
	if (r < 0) {
		panic("pgfault: sys_page_alloc failed %e\n", r);
	}
	memcpy(PFTEMP, addr, PGSIZE);
	r = sys_page_map(0, (void *)PFTEMP, 0, addr, PTE_U | PTE_W | PTE_P);
	if (r < 0) {
		panic("pgfault: sys_page_map failed %e\n", r);
	}
	r = sys_page_unmap(0, (void *)PFTEMP);
	if (r < 0) {
		panic("pgfault: sys_page_unmap failed %e\n", r);
	}
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int err;

	// LAB 4: Your code here.
	void *addr = (void *) (pn * PGSIZE);
	if (uvpt[pn] & PTE_SHARE) {
		// 0 uses the current env
		err = sys_page_map(0, addr, envid, addr, uvpt[pn] & PTE_SYSCALL);
		if (err < 0) {
			return err;
		}
	} else if((uvpt[pn] & PTE_COW) || (uvpt[pn] & PTE_W)) {
		err = sys_page_map(0, addr, envid,  addr, PTE_COW | PTE_U | PTE_P);
		if (err < 0) {
			return err;
		}
		err = sys_page_map(0, addr, 0,  addr, PTE_COW | PTE_U | PTE_P);
		if (err < 0) {
			return err;
		}
	}
	else{
		err = sys_page_map(0, addr, envid,  addr, PTE_U | PTE_P);
		if(err < 0) {
			return err;
		}
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	int err;
	uint32_t addr;
	extern void _pgfault_upcall(void);
	envid_t env_id;

	set_pgfault_handler(pgfault);
	env_id = sys_exofork();
	if (env_id < 0) {
		panic("fork:sys_exofork failed: %e\n ", env_id);
	} else if (env_id == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	
	for (addr = 0; addr < USTACKTOP; addr += PGSIZE) {
		if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)]) & PTE_P){
			err = duppage(env_id, PGNUM(addr));
			if (err < 0) {
				panic("fork: duppage failed: %e\n", err);
			}
		}
	}

	err = sys_page_alloc(env_id, (void *)(UXSTACKTOP - PGSIZE), PTE_P | PTE_W | PTE_U);
	if (err < 0) {
		panic("fork: set_pgfault_handler failed with err: %e\n", err);
	}
	err = sys_env_set_pgfault_upcall(env_id, _pgfault_upcall);
	if (err < 0) {
		panic("fork: set_pgfault_handler failed with err: %e\n", err);
	}
	err = sys_env_set_status(env_id, ENV_RUNNABLE);
	if (err < 0){
		panic("fork: sys_env_set_status failed with err: %e\n", err);
	}
	return env_id;

}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
