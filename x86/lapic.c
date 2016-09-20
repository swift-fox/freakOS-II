#include <stddef.h>
#include <stdint.h>

#include "apic.h"
#include "x86.h"

void printk(char * msg, ...);

void print_debug() {
/*	printk("ISR: ");
	for (uint32_t i = 0; i < 0x80; i += 0x10) {
		printk("%x ", lapic_read_reg(LAPIC_ISR_BASE + i));
	}
	printk("\n");

	printk("TMR: ");
	for (uint32_t i = 0; i < 0x80; i += 0x10) {
		printk("%x ", lapic_read_reg(LAPIC_TMR_BASE + i));
	}
	printk("\n");
*/
	printk("IRR: ");
	for (uint32_t i = 0; i < 0x80; i += 0x10) {
		printk("%x ", lapic_read_reg(LAPIC_IRR_BASE + i));
	}
	printk("\n");

//	printk("ESR: %x\n", lapic_read_reg(LAPIC_ESR));
}

void set_lapic_timer() {
	lapic_write_reg(LAPIC_TIMER_CURRENT, 0);
	lapic_write_reg(LAPIC_TIMER_INITIAL, 4096);

	struct lvt timer_lvt;

	*(uint32_t *)&timer_lvt = lapic_read_reg(LAPIC_LVT_TIMER);
	timer_lvt.vector = 0x30;
	timer_lvt.masked = 0;
	timer_lvt.timer_mode = LAPIC_TIMER_PERIODIC;

	lapic_write_reg(LAPIC_LVT_TIMER, *(uint32_t *)&timer_lvt);
}

DEFINE_IRQ_HANDLER(timer_irq_handler, {
	printk("TIMER int!\n");
})

void lapic_init() {
	uint32_t t;

	t = lapic_read_reg(LAPIC_SPURIOUS);
	t |= LAPIC_SPURIOUS_APIC_EN;
	lapic_write_reg(LAPIC_SPURIOUS, t);

//	set_lapic_timer();

//	register_intr_handler(0x30, timer_irq_handler);

//	print_debug();
}
