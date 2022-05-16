/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
 /*
TRAPHANDLER_NOEC(divide_handler, T_DIVIDE)
TRAPHANDLER_NOEC(debug_handler, T_DEBUG)
TRAPHANDLER_NOEC(nmi_handler, T_NMI)
TRAPHANDLER_NOEC(brkpt_handler, T_BRKPT)
TRAPHANDLER_NOEC(oflow_handler, T_OFLOW)
TRAPHANDLER_NOEC(bound_handler, T_BOUND)
TRAPHANDLER_NOEC(illop_handler, T_ILLOP)
TRAPHANDLER_NOEC(device_handler, T_DEVICE)
TRAPHANDLER(tss_handler, T_TSS)
TRAPHANDLER(dblflt_handler, T_DBLFLT)
TRAPHANDLER(segnp_handler, T_SEGNP)
TRAPHANDLER(stack_handler, T_STACK)
TRAPHANDLER(gpflt_handler, T_GPFLT)
TRAPHANDLER(pgflt_handler, T_PGFLT)
TRAPHANDLER_NOEC(fperr_handler, T_FPERR)
TRAPHANDLER(align_handler, T_ALIGN)
TRAPHANDLER_NOEC(mchk_handler, T_MCHK)
TRAPHANDLER_NOEC(simderr_handler, T_SIMDERR)
TRAPHANDLER_NOEC(syscall_handler, T_SYSCALL)*/


###################################################################
# Challange 1
###################################################################
#define UNI_TRAPHANDLER(name, num)					\
    .data;								\
        .long name;							\
    .text; 								\
        .globl name;		/* define global symbol for 'name' */	\
    	.type name, @function;	/* symbol type is function */		\
    	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
        .if (!(num == 8 || num == 17 || (num >= 10 && num <= 14))); 	\
        pushl $0; 							\
        .endif;     							\
    	pushl $(num);							\
    	jmp _alltraps

.data
    .p2align 2
    .global handlers
handlers:
UNI_TRAPHANDLER(divide_handler, T_DIVIDE)
UNI_TRAPHANDLER(debug_handler, T_DEBUG)
UNI_TRAPHANDLER(nmi_handler, T_NMI)
UNI_TRAPHANDLER(brkpt_handler, T_BRKPT)
UNI_TRAPHANDLER(oflow_handler, T_OFLOW)
UNI_TRAPHANDLER(bound_handler, T_BOUND)
UNI_TRAPHANDLER(illop_handler, T_ILLOP)
UNI_TRAPHANDLER(device_handler, T_DEVICE)
UNI_TRAPHANDLER(dblflt_handler, T_DBLFLT)
.data
	.space 4
UNI_TRAPHANDLER(tss_handler, T_TSS)
UNI_TRAPHANDLER(segnp_handler, T_SEGNP)
UNI_TRAPHANDLER(stack_handler, T_STACK)
UNI_TRAPHANDLER(gpflt_handler, T_GPFLT)
UNI_TRAPHANDLER(pgflt_handler, T_PGFLT)
.data
	.space 4
UNI_TRAPHANDLER(fperr_handler, T_FPERR)
UNI_TRAPHANDLER(align_handler, T_ALIGN)
UNI_TRAPHANDLER(mchk_handler, T_MCHK)
UNI_TRAPHANDLER(simderr_handler, T_SIMDERR)
.data
	.space 48
UNI_TRAPHANDLER(timer_handler, IRQ_OFFSET + IRQ_TIMER)
UNI_TRAPHANDLER(kbd_handler, IRQ_OFFSET + IRQ_KBD)
.data
	.space 8
UNI_TRAPHANDLER(serial_handler, IRQ_OFFSET + IRQ_SERIAL)
.data
	.space 8
UNI_TRAPHANDLER(spurious_handler, IRQ_OFFSET + IRQ_SPURIOUS)
.data
	.space 12
UNI_TRAPHANDLER(e1000_handler, IRQ_OFFSET + IRQ_E1000)
UNI_TRAPHANDLER(mouse_handler, IRQ_OFFSET + IRQ_MOUSE)
.data
	.space 4
UNI_TRAPHANDLER(ide_handler, IRQ_OFFSET + IRQ_IDE)
.data
	.space 4
UNI_TRAPHANDLER(syscall_handler, T_SYSCALL)
.data
	.space 8
UNI_TRAPHANDLER(ide_error, IRQ_OFFSET + IRQ_ERROR)


/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	pushl %ds
	pushl %es
	pushal
	movw $GD_KD, %ax
	movw %ax, %ds
	movw %ax, %es
	pushl %esp
	call trap
