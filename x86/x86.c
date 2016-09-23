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

struct multiboot_info_tag
{
	uint32_t type;
	uint32_t size;
};

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4

struct multiboot_mmap_entry
{
	uint64_t addr;
	uint64_t len;
	uint32_t type;
	uint32_t _;
} __attribute__((packed));

struct multiboot_mmap_tag
{
	uint32_t type;
	uint32_t size;
	uint32_t entry_size;
	uint32_t entry_version;
	struct multiboot_mmap_entry entries[0];
};

void printk(char * msg, ...);

uint32_t multiboot_info_magic;
char * multiboot_info_addr;

void map_region(uint32_t addr, size_t len);

void parse_boot_info() {
	uint32_t size;
	struct multiboot_info_tag * tag;

	if (multiboot_info_magic != 0x36d76289)
		return;	// Did not boot from multiboot2 loader

	size = *(uint32_t *)multiboot_info_addr;

	printk("boot_info @ %x, size: %d\n", (uint32_t)multiboot_info_addr, size);

	for (size_t o = 8; o < size; o += (tag->size + 7) & ~7) {	// Tags' alignment is 8
		tag = (struct multiboot_info_tag *)(multiboot_info_addr + o);

		printk("TAG: %d, %d\n", tag->type, tag->size);

		if (tag->type == 6) {
			struct multiboot_mmap_tag * mmap_tag = (struct multiboot_mmap_tag *)tag;

			if (mmap_tag->entry_version != 0)
				continue;	// Unsupported mmap tag version

			for (size_t i = 0; i < (mmap_tag->size - sizeof(*mmap_tag)) / mmap_tag->entry_size; i++) {
				printk("addr: %lx\tlen: %luk\ttype: %ld\n", (unsigned long)mmap_tag->entries[i].addr, (unsigned long)mmap_tag->entries[i].len>>10, mmap_tag->entries[i].type);
				if (mmap_tag->entries[i].type == 1)
					map_region((uint32_t)mmap_tag->entries[i].addr, (uint32_t)mmap_tag->entries[i].len);
			}
		}
	}
}

void start_kernel();

void paging_enable();

void arch_init() {
	/* Setup FPU as early as possible, because clang generates code that uses FPU. */
	fpu_init();

	terminal_init();
	interrupt_init();

	paging_init();
	parse_boot_info();

	map_region(0, 1 << 20);	// For VGA
	map_region(0xf0000000, 1 << 28);	// For APIC

	paging_enable();

	kbc_init();

	start_kernel();
}
