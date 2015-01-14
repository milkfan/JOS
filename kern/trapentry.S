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
	.text;								\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps;							\
	.data;								\
	.long name

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.text;								\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps;							\
	.data;								\
	.long name

.data

.globl _idt_entry_code
.align 2
_idt_entry_code:

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(Entry_Divide_Error, T_DIVIDE); 
TRAPHANDLER_NOEC(Entry_Debug_Exception, T_DEBUG); 
TRAPHANDLER_NOEC(Entry_NMI_Exception, T_NMI); 
TRAPHANDLER_NOEC(Entry_Breakpoint, T_BRKPT); 
TRAPHANDLER_NOEC(Entry_Overflow, T_OFLOW); 
TRAPHANDLER_NOEC(Entry_Bound_Check, T_BOUND); 
TRAPHANDLER_NOEC(Entry_Illegal_Opcode, T_ILLOP); 
TRAPHANDLER_NOEC(Entry_Device_Not_Available, T_DEVICE); 
TRAPHANDLER(Entry_Double_Fault, T_DBLFLT); 
TRAPHANDLER(Entry_T_COPROC, -1);// Reserved, 9，占位 
TRAPHANDLER(Entry_Invalid_TSS, T_TSS); 
TRAPHANDLER(Entry_Segment_Not_Present, T_SEGNP); 
TRAPHANDLER(Entry_Stack_Exception, T_STACK); 
TRAPHANDLER(Entry_General_Protection_Fault, T_GPFLT); 
TRAPHANDLER(Entry_Page_Fault, T_PGFLT); 
TRAPHANDLER(Entry_T_RES, -1);// Reserved, 15，占位 
TRAPHANDLER_NOEC(Entry_Floating_Point_Error, T_FPERR); 
TRAPHANDLER(Entry_Alignment_Check, T_ALIGN); 
TRAPHANDLER_NOEC(Entry_Machine_Check, T_MCHK); 
TRAPHANDLER_NOEC(Entry_Simd_Floating_Point_Error, T_SIMDERR);

TRAPHANDLER_NOEC(Entry_Syscall, T_SYSCALL);

# for IRQ handler 
TRAPHANDLER_NOEC(routine_irq0, IRQ_OFFSET + 0); 
TRAPHANDLER_NOEC(routine_irq1, IRQ_OFFSET + 1); 
TRAPHANDLER_NOEC(routine_irq2, IRQ_OFFSET + 2); 
TRAPHANDLER_NOEC(routine_irq3, IRQ_OFFSET + 3); 
TRAPHANDLER_NOEC(routine_irq4, IRQ_OFFSET + 4); 
TRAPHANDLER_NOEC(routine_irq5, IRQ_OFFSET + 5); 
TRAPHANDLER_NOEC(routine_irq6, IRQ_OFFSET + 6); 
TRAPHANDLER_NOEC(routine_irq7, IRQ_OFFSET + 7); 
TRAPHANDLER_NOEC(routine_irq8, IRQ_OFFSET + 8); 
TRAPHANDLER_NOEC(routine_irq9, IRQ_OFFSET + 9);
TRAPHANDLER_NOEC(routine_irq10, IRQ_OFFSET + 10);
TRAPHANDLER_NOEC(routine_irq11, IRQ_OFFSET + 11);
TRAPHANDLER_NOEC(routine_irq12, IRQ_OFFSET + 12);
TRAPHANDLER_NOEC(routine_irq13, IRQ_OFFSET + 13);
TRAPHANDLER_NOEC(routine_irq14, IRQ_OFFSET + 14);
TRAPHANDLER_NOEC(routine_irq15, IRQ_OFFSET + 15);

/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	pushw $0x0
	pushw %ds
	pushw $0x0
	pushw %es 
	pushal

	movl $GD_KD, %eax
	movw %ax, %ds
	movw %ax, %es

	pushl %esp 
	call trap
