#include <stddef.h>
#include <stdint.h>

#include "x86.h"

#define NR_INTR 256

static inline struct gate_desc make_intr_gate(uint32_t addr, uint16_t seg, uint8_t dpl, uint8_t present) {
	struct gate_desc ret = {
		.offset0 = addr,
		.offset1 = addr >> 16,
		.seg = seg,
		.reserved = 0,
		.type = INTR_GATE,
		.s = 0,
		.dpl = dpl,
		.p = present
	};

	return ret;
}

void printk(char * msg, ...);

struct gate_desc idt[NR_INTR] __attribute__((aligned(8)));

void register_intr_handler(uint8_t nr, void * handler) {
	if (nr > 0xff) {
		printk("interrupt: invalid interrupt vector: %u\n", nr);
		return;
	}

	idt[nr] = make_intr_gate((uint32_t)handler, 0x10, 0, 1);
}

DEFINE_INTR_HANDLER(default_intr_handler, {
	printk("Interrupt!\n");
})

void setup_idt() {
	size_t i;

	struct desc_reg idtr = {
		.limit = sizeof(idt) - 1,
		.base = (uint32_t)idt
	};

	for (i = 0; i < NR_INTR; i++)
		idt[i] = make_intr_gate((uint32_t)default_intr_handler, 0x10, 0, 1);

	asm volatile("lidt %0\n"
		"sti"
		:
		: "m" (idtr.limit)
	);
}
