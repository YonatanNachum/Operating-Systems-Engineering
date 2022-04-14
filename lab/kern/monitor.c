// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display backtrace info of the stack", mon_backtrace },
	{ "showmappings", "Display physical page mappings", mon_showmap },
	{ "ChangePagePerm", "Set, clear, or change the permissions of any mapping in the current address space", mon_change},
	{ "memoryDump", "Dump the contents of a range of memory given either a virtual or physical address range", mon_memdump},
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

static bool
string_to_address(char *s, uint32_t *num)
{
	int len = strlen(s);
	uint32_t digit;
	if (len < 3 || s[0] != '0' || s[1] != 'x') {
		return false;
	}
	int base = 1;
	--len;
	for (; len >= 2; len--)
	{
		if ((s[len] >= '0' && s[len] <= '9')) {
			digit = (s[len] - '0');
		} else if ((s[len] >= 'a' && s[len] <= 'f')) {
			digit = (s[len] - 'a' + 10);
		} else {
			return false;
		}
		*num += digit*base;
		base *= 16;
	}
	return true;
}

int
mon_showmap(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t begin, end, tmp;
	pte_t *pte;
	pde_t *pgdir = KADDR(rcr3());

	if (argc != 3) {
		cprintf("Invalid number of arguments: %d\n", argc);
		return 0;
	}
	begin = 0;
	end = 0;
	if ((!string_to_address(argv[1], &begin)) ||
	    (!string_to_address(argv[2], &end))) {
		cprintf("Invalid arguments: [%s] [%s]\n", argv[1], argv[2]);
		return 0;
	}
	begin = ROUNDDOWN(begin, PGSIZE);

	while (begin <= end) {
		pte = pgdir_walk(pgdir, (void *)begin, 0);
		if (pte == NULL) {
			cprintf("virtual:0x%x     physical: Not mapped", begin);
		} else {
			cprintf("virtual:0x%08x     physical:0x%08x      flags:0x%03x",
				begin, PTE_ADDR(*pte), PGOFF(*pte));
		}
		if (pte == NULL || *pte & PTE_PS) {
			cprintf("  (4MB page)\n");
			tmp = begin + PTSIZE;
		} else {
			cprintf("\n");
			tmp = begin + PGSIZE;
		}
		if (tmp > begin) {
			begin = tmp;
		} else {
			break;
		}
	}
	return 0;
}

static bool
update_perm(pte_t *pte, int argc, char **argv, uint32_t addr)
{
	uint8_t i;
	uint16_t add_perm = 0, del_perm = 0;

	for (i = 3; i < argc; ++i) {
		if (strcmp(argv[i], "W") == 0) {
			add_perm |= PTE_W;
		} else if (strcmp(argv[i], "~W") == 0) {
			del_perm |= PTE_W;
		} else if (strcmp(argv[i], "U") == 0) {
			add_perm |= PTE_U;
		} else if (strcmp(argv[i], "~U") == 0) {
			del_perm |= PTE_U;
		} else {
			cprintf("Invalid flag: %s, legal flags are: W, ~W, U, ~U\n", argv[i]);
			return false;
		}
	}
	*pte = *pte | add_perm;
	*pte = *pte & ~del_perm;
	return true;
}

int
mon_change(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t addr = 0;
	pte_t *pte;
	pde_t *pgdir = KADDR(rcr3());

	if (argc < 3) {
		cprintf("Invalid number of arguments: %d\n", argc);
		return 0;
	}

	if (strcmp(argv[1], "clear") == 0) {
		if (!string_to_address(argv[2], &addr)) {
			cprintf("Invalid address: %s\n", argv[2]);
			return 0;
		}
		cprintf("addr 0x%x\n", addr);
		if (argc != 3) {
			cprintf("Invalid number of arguments: %d\n", argc);
			return 0;
		}
		pte = pgdir_walk(pgdir, (void *)addr, 0);
		if (pte == NULL) {
			cprintf("virtual:0x%x Not mapped", addr);
			return 0;
		}
		*pte = *pte & ~(PTE_W | PTE_U);

	} else {
		if (strcmp(argv[1], "set") == 0) {
			if (!string_to_address(argv[2], &addr)) {
				cprintf("Invalid address: %s\n", argv[2]);
				return 0;
			}
			pte = pgdir_walk(pgdir, (void *)addr, 0);
			if (pte == NULL) {
				cprintf("virtual:0x%x Not mapped", addr);
				return 0;
			}
			if (!update_perm(pte, argc, argv, addr)) {
				return 0;
			}

		} else {
			cprintf("Invalid command: %s, Only clear and set are allowed\n", argv[1]);
			return 0;
		}
	}
	cprintf("virtual:0x%08x     physical:0x%08x      flags:0x%03x\n",
		addr, PTE_ADDR(*pte), PGOFF(*pte));
	return 0;
}

int
mon_memdump(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t begin, end, tmp;
	pte_t *pte;
	bool virt;

	pde_t *pgdir = KADDR(rcr3());

	if (argc != 4) {
		cprintf("Invalid number of arguments: %d\n", argc);
		return 0;
	}
	begin = 0;
	end = 0;
	if ((!string_to_address(argv[1], &begin)) ||
	    	(!string_to_address(argv[2], &end))) {
		cprintf("Invalid arguments: [%s] [%s]\n", argv[1], argv[2]);
		return 0;
	}
	if (strcmp(argv[3], "-v") == 0) {
		virt = true;
	} else {
		if (strcmp(argv[3], "-p") == 0) {
			virt = false;
		} else {
			cprintf("Invalid address flag: [%s], -v = virtual, -p = physical\n", argv[3]);
			return 0;
		}
	}
	begin = ROUNDDOWN(begin, PGSIZE);

	if (virt) {
		while (begin <= end) {
           		struct PageInfo *page = page_lookup(pgdir, (void *)begin, NULL);
			cprintf("virtual:0x%08x     ", begin);
			if (page) {
				cprintf("physical:0x%08x     ", page2pa(page) + PGOFF(begin));
				int j;
                		for (j = 0; j < 0x10; j += 4) {
                    			cprintf("%08lx ", *(long *)(begin + j));
                		}
               			cprintf("\n");
            		} else {
                		cprintf(" not mapped\n");
        		}
			begin += 0x10;
		}
	} else {
       		while (begin <= end) {
			cprintf("physical:0x%08x     ", begin);
            		int j;
            		for (j = 0; j < 0x10; j += 4) {
                		cprintf("%08lx ", *(long *)KADDR(begin + j));
            		}
			cprintf("\n");
			begin += 0x10;
       		}
        	cprintf("\n");
	}
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	uint32_t *ebp = (uint32_t*)read_ebp();
	uint32_t eip, i;
	uint32_t params[5] = {0};
	struct Eipdebuginfo info;

	cprintf("Stack backtrace:\n");
	while(ebp != 0) {
		eip = *(ebp + 1);
		for(i = 0;i < 5; ++i) {
			params[i] = *(ebp + 2 + i);
		}
		cprintf("ebp  %08x eip %08x args %08x %08x %08x %08x %08x\n", ebp, eip, params[0], params[1],
			params[2], params[3], params[4]);
		if (debuginfo_eip(eip, &info) == 0){
			cprintf("%s:%u: %.*s+%u\n",info.eip_file, info.eip_line, info.eip_fn_namelen,
				info.eip_fn_name, eip-info.eip_fn_addr);
		}
		ebp = (uint32_t*)(*ebp);
	}
	/* Stack initialized with ebp with value 0 */
	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
