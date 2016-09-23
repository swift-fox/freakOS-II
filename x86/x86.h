#ifndef _X86_H
#define _X86_H

#include <stdint.h>
#include <stddef.h>

#include "apic.h"

/* Control registers */
struct cr0 {
	uint16_t pe : 1, mp : 1, em : 1, ts : 1, et : 1, ne : 1, reserved0 : 10;
	uint16_t wp : 1, reserved1 : 1, am : 1, reserved2 : 10, nw : 1, cd : 1, pg : 1;
} __attribute__((packed, aligned(4)));

struct cr3 {
	uint32_t _2 : 3, pwt : 1, pcd : 1, _5 : 7, page_dir_addr : 20;
} __attribute__((packed, aligned(4)));

struct cr4 {
	uint8_t vme : 1, pvi : 1, tsd : 1, de : 1, pse : 1, pae : 1, mce : 1, pge : 1;
	uint8_t pce : 1, osfxsr : 1, osxmmexcpt : 1, umip : 1, _12 : 1, vmxe : 1, smxe : 1, _15 : 1;
	uint8_t fsgsbase : 1, pcide : 1, osxsave : 1, _19 : 1, smep : 1, smap : 1, pke : 1, _23 : 1;
	uint8_t _24;
} __attribute__((packed, aligned(4)));


/* System descriptors */
struct gate_desc {
	union {
		uint16_t limit0;
		uint16_t offset0;
	};

	union {
		uint16_t base0;
		uint16_t seg;
	};

	union {
		uint8_t base1;
		uint8_t reserved;
	};

	uint8_t type : 4, s : 1, dpl : 2, p : 1;

	union {
		struct {
			uint8_t limit1 : 4, avl : 1, l : 1, db : 1, g : 1, base2;
		};
		uint16_t offset1;
	};
} __attribute__((packed));

enum seg_desc_types {
	DATA_RD,
	DATA_RD_ACCESSED,
	DATA_RW,
	DATA_RW_ACCESSED,
	STACK_RD,
	STACK_RD_ACCESSED,
	STACK_RW,
	STACK_RW_ACCESSED,
	CODE,
	CODE_ACCESSED,
	CODE_RD,
	CODE_RD_ACCESSED,
	CODE_CONFORMING,
	CODE_CONFORMING_ACCESSED,
	CODE_RD_CONFORMING,
	CODE_RD_CONFORMING_ACCESSED,
};

enum system_desc_types {
	TSS_16_AVAILABLE = 1,
	LDT,
	TSS_16_BUSY,
	CALL_GATE_16,
	TASK_GATE,
	INTR_GATE_16,
	TRAP_GATE_16,
	TSS_32_AVAILABLE = 9,
	TSS_32_BUSY = 11,
	CALL_GATE,
	INTR_GATE = 14,
	TRAP_GATE
};

struct desc_reg {
	uint16_t pad;
	uint16_t limit;	// Always load or store with &desc_reg.limit
	uint32_t base;
} __attribute__((packed, aligned(4)));

/* Clang interrupt handler stack frame */
struct interrupt_frame
{
	uint32_t ip;
	uint32_t cs;
	uint32_t flags;
	uint32_t sp;
	uint32_t ss;
};

/* Platform init functions */
void terminal_init();
void setup_idt();
void fpu_init();
void ioapic_init();
void lapic_init();
void kbc_init();
void paging_init();

/* Interrupt-related routines */
void register_intr_handler(uint8_t nr, void * handler);
void ioapic_irq_setup(uint8_t irq_nr, uint8_t pin_nr);

#define DEFINE_INTR_HANDLER(name, code)	\
void name(struct interrupt_frame * frame) __attribute__((interrupt)) code

#define DEFINE_INTR_HANDLER_ERRCODE(name, code)	\
void name(struct interrupt_frame * frame, uint32_t error_code) __attribute__((interrupt)) code

#define DEFINE_IRQ_HANDLER(name, code)	\
void name(struct interrupt_frame * frame) __attribute__((interrupt)) {	\
code\
	lapic_write_reg(LAPIC_EOI, 0);\
}

/* Read and write control registers */
static inline struct cr0 read_cr0() {
	struct cr0 t;
	asm volatile("movl %%cr0, %0" : "=r" (t));
	return t;
}

static inline void write_cr0(struct cr0 t) {
	asm volatile("movl %0, %%cr0" : : "r" (t));
}

static inline struct cr3 read_cr3() {
	struct cr3 t;
	asm volatile("movl %%cr3, %0" : "=r" (t));
	return t;
}

static inline void write_cr3(struct cr3 t) {
	asm volatile("movl %0, %%cr3" : : "r" (t));
}

static inline struct cr4 read_cr4() {
	struct cr4 t;
	asm volatile("movl %%cr4, %0" : "=r" (t));
	return t;
}

static inline void write_cr4(struct cr4 t) {
	asm volatile("movl %0, %%cr4" : : "r" (t));
}

#endif
