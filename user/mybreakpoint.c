// program to cause a breakpoint trap

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("this is before int 3\n");
	asm volatile("int $3");
	cprintf("this is after int 3 (1)\n");
	cprintf("this is after int 3 (2)\n");
}

