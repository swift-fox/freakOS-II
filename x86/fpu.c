#include "x86.h"

void fpu_init()
{
	struct cr0 cr0;
	struct cr4 cr4;

	/* Enable x87 FPU */
	cr0 = read_cr0();
	cr0.ts = 0;
	cr0.em = 0;
	cr0.mp = 1;
	cr0.ne = 1;
	write_cr0(cr0);

	/* Enable SSE */
	cr4 = read_cr4();
	cr4.osfxsr = 1;
	cr4.osxmmexcpt = 1;
	write_cr4(cr4);

	asm volatile("fninit");
}
