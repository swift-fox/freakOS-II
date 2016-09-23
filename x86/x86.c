#include "x86.h"

static inline void disable_8259() {
	asm volatile("outb %%al, $0xa1\n"
		"outb %%al, $0x21"
		: :"a"(0xff)
	);
//	asm volatile("outb %0, %1" : : "a"((uint8_t)0x70), "dN"((uint16_t)0x22));
//	asm volatile("outb %0, %1" : : "a"((uint8_t)0x01), "dN"((uint16_t)0x23));
}

static void interrupt_init() {
	asm volatile("cli");

	disable_8259();
	setup_idt();
	lapic_init();
	ioapic_init();

	asm volatile("sti");
}
/*
void register_irq(unsigned int irq_nr, void * handler) {
	apic_register_intr_handler(irq_nr, handler);
}
*/

void arch_init() {
	/* Setup FPU as early as possible, because clang generates code that uses FPU. */
	fpu_init();

	terminal_init();
	interrupt_init();
	paging_init();

	kbc_init();
}
