#include <stdint.h>

#include "x86.h"

void fpu_init()
{
	struct cr0 cr0;
	struct cr4 cr4;

	/* Enable x87 FPU */
	asm volatile("movl %%cr0, %0" : "=r" (cr0));
	cr0.ts = 0;
	cr0.em = 0;
	cr0.mp = 1;
	cr0.ne = 1;
	asm volatile("movl %0, %%cr0" : : "r" (cr0));

	/* Enable SSE */
	asm volatile("movl %%cr4, %0" : "=r" (cr4));
	cr4.osfxsr = 1;
	cr4.osxmmexcpt = 1;
	asm volatile("movl %0, %%cr4" : : "r" (cr4));

	asm volatile("fninit");
}
