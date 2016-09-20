#ifndef _APIC_H
#define _APIC_H

#define LAPIC_BASE	0xfee00000
#define LAPIC_REG(offset)	(LAPIC_BASE + (offset))

#define LAPIC_ID	LAPIC_REG(0x020)
#define LAPIC_VER	LAPIC_REG(0x030)
#define LAPIC_TPR	LAPIC_REG(0x080)
#define LAPIC_APR	LAPIC_REG(0x090)
#define LAPIC_PPR	LAPIC_REG(0x0a0)
#define LAPIC_EOI	LAPIC_REG(0x0b0)
#define LAPIC_RRD	LAPIC_REG(0x0c0)
#define LAPIC_LOGICAL_DEST	LAPIC_REG(0x0d0)
#define LAPIC_DEST_MODE	LAPIC_REG(0x0e0)
#define LAPIC_SPURIOUS	LAPIC_REG(0x0f0)
#define LAPIC_ISR_BASE	LAPIC_REG(0x100)
#define LAPIC_TMR_BASE	LAPIC_REG(0x180)
#define LAPIC_IRR_BASE	LAPIC_REG(0x200)
#define LAPIC_ESR	LAPIC_REG(0x280)
#define LAPIC_LVT_CMCI	LAPIC_REG(0x2f0)
#define LAPIC_ICR_BASE	LAPIC_REG(0x300)
#define LAPIC_LVT_TIMER	LAPIC_REG(0x320)
#define LAPIC_LVT_THERMAL	LAPIC_REG(0x330)
#define LAPIC_LVT_PERF	LAPIC_REG(0x340)
#define LAPIC_LVT_LINT0	LAPIC_REG(0x350)
#define LAPIC_LVT_LINT1	LAPIC_REG(0x360)
#define LAPIC_LVT_ERR	LAPIC_REG(0x370)
#define LAPIC_TIMER_INITIAL	LAPIC_REG(0x380)
#define LAPIC_TIMER_CURRENT	LAPIC_REG(0x390)
#define LAPIC_TIMER_DIV	LAPIC_REG(0x3e0)

#define LAPIC_DELMOD_FIXED	0
#define LAPIC_DELMOD_SMI	2
#define LAPIC_DELMOD_NMI	4
#define LAPIC_DELMOD_INIT	5
#define LAPIC_DELMOD_EXT	7

#define LAPIC_TRIG_EDGE	0
#define LAPIC_TRIG_LEVEL	1

#define LAPIC_ACTIVE_HIGH	0
#define LAPIC_ACTIVE_LOW	1

#define LAPIC_TIMER_ONE_SHOT	0
#define LAPIC_TIMER_PERIODIC	1
#define LAPIC_TIMER_TSC_DEADLINE	2

#define LAPIC_SPURIOUS_APIC_EN	0x00000100

#define IOAPIC_ID	0x00
#define IOAPIC_VER	0x01
#define IOAPIC_ARB	0x02
#define IOAPIC_REDTBL_BASE	0x10

#define IOAPIC_TRIG_EDGE	0
#define IOAPIC_TRIG_LEVEL	1

#define IOAPIC_ACTIVE_HIGH	0
#define IOAPIC_ACTIVE_LOW	1

#define IOAPIC_DESTMOD_PHYSICAL	0
#define IOAPIC_DESTMOD_LOGICAL	1

#define IOAPIC_DELMOD_FIXED	0
#define IOAPIC_DELMOD_LOWPRI	1
#define IOAPIC_DELMOD_SMI	2
#define IOAPIC_DELMOD_NMI	4
#define IOAPIC_DELMOD_INIT	5
#define IOAPIC_DELMOD_EXT	7

#define lapic_read_reg(reg)	(*((uint32_t *)(reg)))
#define lapic_write_reg(reg, val)	(*((uint32_t *)(reg)) = (val))

// Local APIC local interrupt vector table
struct lvt {
	uint8_t vector;
	uint8_t delivery_mode : 3, _11 : 1, status : 1, polarity: 1, remote_irr : 1, trigger_mode : 1;
	uint8_t masked : 1, timer_mode : 2, _19 : 5;
	uint8_t _24;
} __attribute__((packed));

// IOAPIC IO redirection table
struct redtbl {
	uint16_t vector : 8, delivery_mode : 3, dest_mode : 1, status : 1, polarity : 1, remote_irr : 1, trigger_mode : 1;
	uint16_t masked : 1, reserved0 : 15;
	uint16_t reserved1;
	uint16_t reserved2 : 8, dest : 8;
} __attribute__((packed));

#endif
