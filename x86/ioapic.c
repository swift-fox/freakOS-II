#include <stdint.h>

#include "apic.h"

static volatile uint32_t * ioapic_regsel = (uint32_t *) 0xfec00000;
static volatile uint32_t * ioapic_iowin = (uint32_t *) 0xfec00010;

static inline uint32_t read_ioapic_reg(uint8_t reg) {
	*ioapic_regsel = reg;
	return *ioapic_iowin;
}

static inline void write_ioapic_reg(uint8_t reg, uint32_t val) {
	*ioapic_regsel = reg;
	*ioapic_iowin = val;
}

static inline void read_red_table(uint8_t nr, struct redtbl * val) {
	uint32_t * p = (uint32_t *) val;
	uint8_t reg = IOAPIC_REDTBL_BASE + nr * 2;

	p[1] = read_ioapic_reg(reg + 1);
	p[0] = read_ioapic_reg(reg);
}

static inline void write_red_table(uint8_t nr, struct redtbl * val) {
	uint32_t * p = (uint32_t *) val;
	uint8_t reg = IOAPIC_REDTBL_BASE + nr * 2;

	write_ioapic_reg(reg + 1, p[1]);
	write_ioapic_reg(reg, p[0]);
}

void ioapic_irq_setup(uint8_t irq_nr, uint8_t pin_nr) {
	struct redtbl t = {
		.vector = irq_nr,
		.delivery_mode = IOAPIC_DELMOD_FIXED,
		.dest_mode = IOAPIC_DESTMOD_PHYSICAL,
		.polarity = IOAPIC_ACTIVE_HIGH,
		.trigger_mode = IOAPIC_TRIG_EDGE,
		.masked = 0,
		.dest = 0
	};

	write_red_table(pin_nr, &t);
}

void ioapic_init() {
	struct redtbl t = {
		.delivery_mode = IOAPIC_DELMOD_FIXED,
		.dest_mode = IOAPIC_DESTMOD_PHYSICAL,
		.polarity = IOAPIC_ACTIVE_LOW,
		.trigger_mode = IOAPIC_TRIG_EDGE,
		.masked = 1,
		.dest = 0
	};

	for (uint8_t i = 0; i < 24; i++) {
		t.vector = i + 0x20;
		write_red_table(i, &t);
	}
}
