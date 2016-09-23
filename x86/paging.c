#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "x86.h"

struct pte {
	uint32_t present : 1, rw : 1, us : 1, pwt : 1, pcd : 1, accessed : 1, dirty : 1, pat : 1;
	uint32_t global : 1, _9 : 3, page_phy_addr : 20;
} __attribute__((packed));

struct pte page_dir[1024] __attribute__((aligned(4096)));
struct pte page_table[1024][1024] __attribute__((aligned(4096)));

void printk(char * msg, ...);

static inline struct pte make_pte(uint32_t addr, bool present) {
	struct pte ret = {
		.present = present,
		.rw = 1,
		.us = 1,
		.pwt = 0,
		.pcd = 0,
		.accessed = 0,
		.dirty = 0,
		.pat = 0,
		.global = 0,
		.page_phy_addr = (uint32_t)addr >> 12
	};
	return ret;
}

DEFINE_INTR_HANDLER_ERRCODE(page_fault_handler, {
	printk("Page fault! Code: %d\n", error_code);
	printk("EIP: %x, CS: %x\n", frame->ip, frame->cs);
	printk("EFLAGS: %x\n", frame->flags);
	printk("ESP: %x, SS: %x\n", frame->sp, frame->ss);
})

DEFINE_INTR_HANDLER_ERRCODE(general_protection_handler, {
	printk("General protection! %d\n", error_code);
})

size_t total_avail_mem = 0;

void map_region(uint32_t addr, size_t len) {
	size_t pd_idx, pe_idx;
	uint32_t end = addr + len + 0xfff;

	if (end < addr)	// Integer overflow
		end = 0xfffff000;
	else
		end &= ~0xfff;

	addr &= ~0xfff;

	pd_idx = (addr & 0xffc00000) >> 22;
	pe_idx = (addr & 0x003ff000) >> 12;

	while (addr < end) {
		page_dir[pd_idx] = make_pte((uint32_t)&page_table[pd_idx], true);

		while (addr < end && pe_idx < 1024) {
			page_table[pd_idx][pe_idx] = make_pte(addr, true);

			addr += (1 << 12);
			pe_idx++;
		}

		pd_idx++;
		pe_idx = 0;
	}

	if (end == 0xfffff000) {
		page_dir[1023] = make_pte((uint32_t)&page_table[1023], true);
		page_table[1023][1023] = make_pte(end, true);
	}
}

void paging_init() {
	struct cr3 cr3;

	printk("sizeof(struct pte) = %d\n", sizeof(struct pte));
	printk("sizeof(page_dir) = %d\n", sizeof(page_dir));
	printk("page_dir @ %x\n", page_dir);
	printk("sizeof(struct cr3) = %d\n", sizeof(struct cr3));

	for (size_t i = 0; i < 1024; i++)
		page_dir[i] = make_pte((uint32_t)&page_table[i], false);

	register_intr_handler(0x0d, general_protection_handler);
	register_intr_handler(0x0e, page_fault_handler);

	cr3 = read_cr3();
	cr3.page_dir_addr = (uint32_t)page_dir >> 12;
	write_cr3(cr3);
}

void paging_enable() {
	struct cr0 cr0;

	cr0 = read_cr0();
	cr0.pg = 1;
	write_cr0(cr0);	
}
