#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "x86.h"

#define DATA_PORT	0x60
#define CMD_PORT	0x64
#define STATUS_PORT	0x64

#define STATUS_OBF	(1 << 0)
#define STATUS_IBF	(1 << 1)

#define CMD_READ_CFG	0x20
#define CMD_WRITE_CFG	0x60
#define CMD_CTL_TEST	0xaa
#define CMD_PORT1_TEST	0xab
#define CMD_PORT1_DISABLE	0xad
#define CMD_PORT1_ENABLE	0xae
#define CMD_PORT2_TEST	0xa9
#define CMD_PORT2_DISABLE	0xa7
#define CMD_PORT2_ENABLE	0xa8
#define CMD_WRITE_PORT2	0xd4

#define CFG_PORT1_INT	(1 << 0)
#define CFG_PORT2_INT	(1 << 1)
#define CFG_PORT1_CLK	(1 << 4)
#define CFG_PORT2_CLK	(1 << 5)
#define CFG_TRANSLATION	(1 << 6)

static inline void outb(uint8_t data, uint16_t port) {
	asm volatile("outb %0, %1" : : "a"(data), "dN"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

static inline uint8_t read_status() {
	return inb(STATUS_PORT);
}

static inline void wait_read() {
	while(~read_status() & STATUS_OBF);
}

static inline void wait_write() {
	while(read_status() & STATUS_IBF);
}

static inline void command(uint8_t cmd) {
	wait_write();
	outb(cmd, CMD_PORT);
}

static inline uint8_t read_data() {
	wait_read();
	return inb(DATA_PORT);
}

static inline void write_data(uint8_t data) {
	wait_write();
	outb(data, DATA_PORT);
}

static uint8_t read_cfg() {
	command(CMD_READ_CFG);
	return read_data();
}

static void write_cfg(uint8_t data) {
	command(CMD_WRITE_CFG);
	write_data(data);
}

void printk(char * msg, ...);

/* Keyboard */

#define KBD_ERR	0x00
#define KBD_SELF_TEST_PASS	0xaa
#define KBD_ECHO	0xee
#define KBD_ACK	0xfa
#define	KBD_SELF_TEST_FAIL	0xfc
#define	KBD_SELF_TEST_FAIL2	0xfd
#define	KBD_RESEND	0xfe
#define	KBD_ERR2	0xff

#define KBD_CMD_LED	0xed
#define	KBD_CMD_ECHO	0xee
#define KBD_CMD_SCANCODE	0xf0
#define KBD_CMD_ID	0xf2
#define KBD_CMD_TYPE_RATE	0xf3
#define KBD_CMD_SCAN_ENABLE	0xf4
#define KBD_CMD_SCAN_DISABLE	0xf5
#define KBD_CMD_SET_DEFAULTS	0xf6
#define	KBD_CMD_RESEND	0xfe
#define KBD_CMD_RESET	0xff

#define KBD_SCANCODE_GET	0
#define KBD_SCANCODE_SET1	1
#define KBD_SCANCODE_SET2	2
#define KBD_SCANCODE_SET3	3

#define KBD_LED_SCROLL	(1 << 0)
#define KBD_LED_NUM	(1 << 1)
#define KBD_LED_CAPS	(1 << 2)

static uint8_t write_kbd(uint8_t data) {
	uint8_t ret;

	do {
		write_data(data);
		ret = read_data();
	} while(ret == KBD_RESEND);

	return ret;
}

static uint8_t write_mouse(uint8_t data) {
	uint8_t ret;

	do {
		command(CMD_WRITE_PORT2);
		write_data(data);
		ret = read_data();
	} while(ret == KBD_RESEND);

	return ret;
}

static void set_kbd_led(uint8_t leds) {
	write_kbd(KBD_CMD_LED);
	write_kbd(leds);
}

static void kbd_init() {
	uint8_t status;

	write_kbd(KBD_CMD_RESET);
	status = read_data();
	if (status != KBD_SELF_TEST_PASS) {
		printk("kbc: keyboard self-test failed. code: %x\n", (uint32_t)status);
		return;
	}

	write_kbd(KBD_CMD_SCAN_ENABLE);
}

static void mouse_init() {
	uint8_t status;

	write_mouse(KBD_CMD_RESET);
	status = read_data();
	if (status != KBD_SELF_TEST_PASS) {
		printk("kbc: mouse self-test failed. code: %x\n", (uint32_t)status);
		return;
	}

	status = read_data();
	if (status != 0x00) {
		printk("kbc: mouse port doesn't connect a mouse. code: %x\n", (uint32_t)status);
		return;
	}

	write_mouse(KBD_CMD_SCAN_ENABLE);
}

const char * const scancode_tbl[] = {
	/* 0x00 */ NULL, "f9", NULL, "f5", "f3", "f1", "f2", "f12",
	/* 0x08 */ NULL, "f10", "f8", "f6", "f4", "tab", "`", NULL,
	/* 0x10 */ NULL, "alt_l", "shift_l", NULL, "ctrl_l", "q", "1", NULL,
	/* 0x18 */ NULL, NULL, "z", "s", "a", "w", "2", NULL,
	/* 0x20 */ NULL, "c", "x", "d", "e", "4", "3", NULL,
	/* 0x28 */ NULL, " ", "v", "f", "t", "r", "5", NULL,
	/* 0x30 */ NULL, "n", "b", "h", "g", "y", "6", NULL,
	/* 0x38 */ NULL, NULL, "m", "j", "u", "7", "8", NULL,
	/* 0x40 */ NULL, ",", "k", "i", "o", "0", "9", NULL,
	/* 0x48 */ NULL, ".", "/", "l", ";", "p", "-", NULL,
	/* 0x50 */ NULL, NULL, "'", NULL, "[", "=", NULL, NULL,
	/* 0x58 */ "caps_lock", "shift_r", "\n", "]", NULL, "\\", NULL, NULL,
	/* 0x60 */ NULL, NULL, NULL, NULL, NULL, NULL, "backspace", NULL,
	/* 0x68 */ NULL, "1_keypad", NULL, "4_keypad", "7_keypad", NULL, NULL, NULL,
	/* 0x70 */ "0_keypad", "._keypad", "2_keypad", "5_keypad", "6_keypad", "8_keypad", "esc", "num_lock",
	/* 0x78 */ "f11", "+_keypad", "3_keypad", "-_keypad", "*_keypad", "9_keypad", "scroll_lock", NULL,
	/* 0x80 */ NULL, NULL, NULL, "f7"
};

const char * const scancode_extend_tbl[] = {
	/* 0x00 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x08 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x10 */ "www_search", "alt_r", NULL, NULL, "ctrl_r", "prev_track", NULL, NULL,
	/* 0x18 */ "www_fav", NULL, NULL, NULL, NULL, NULL, NULL, "gui_l",
	/* 0x20 */ "www_refresh", "vol_down", NULL, "mute", NULL, NULL, NULL, "gui_r",
	/* 0x28 */ "www_stop", NULL, NULL, "calc", NULL, NULL, NULL, "apps",
	/* 0x30 */ "www_forward", NULL, "vol_up", NULL, "play", NULL, NULL, "power",
	/* 0x38 */ "www_back", NULL, "www_home", "stop", NULL, NULL, NULL, "sleep",
	/* 0x40 */ "my_computer", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x48 */ "email", NULL, "/_keypad", NULL, NULL, "next_track", NULL, NULL,
	/* 0x50 */ "media_select", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x58 */ NULL, NULL, "enter_keypad", NULL, NULL, NULL, "wake", NULL,
	/* 0x60 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	/* 0x68 */ NULL, "end", NULL, "left", "home", NULL, NULL, NULL,
	/* 0x70 */ "insert", "delete", "down", NULL, "right", "up", NULL, NULL,
	/* 0x78 */ NULL, NULL, "page_down", NULL, "print_scr", "page_up"
};

#define SCANCODE_BUF_LEN	6
static uint8_t scancode_buffer[SCANCODE_BUF_LEN];
static size_t scancode_buffer_ptr = 0, pause_seq = 0;
static bool release = false, extend = false;

DEFINE_IRQ_HANDLER(kbd_irq_handler, {
	uint8_t scancode;

	scancode = read_data();
//	printk("SCANCODE: %x\n",  (uint32_t)scancode);

	/* Special routine to identify pause */
	switch(pause_seq) {
	default:
		if (scancode == 0xe1) {
			pause_seq = 1;
			goto out;
		}
		break;
	case 1:
		pause_seq = (scancode == 0x14) ? pause_seq + 1 : 0;
		goto out;
	case 2:
		pause_seq = (scancode == 0x77) ? pause_seq + 1 : 0;
		goto out;
	case 3:
		pause_seq = (scancode == 0xe1) ? pause_seq + 1 : 0;
		goto out;
	case 4:
		pause_seq = (scancode == 0xf0) ? pause_seq + 1 : 0;
		goto out;
	case 5:
		pause_seq = (scancode == 0x14) ? pause_seq + 1 : 0;
		goto out;
	case 6:
		pause_seq = (scancode == 0xf0) ? pause_seq + 1 : 0;
		goto out;
	case 7:
		if (scancode == 0x77) {
			printk("pause");
			pause_seq = 0;
		} else
			printk("BUG: unrecognized keyboard scancode: %x\n", (uint32_t)scancode);
		goto out;
	}

	if (scancode == 0xf0)
		release = true;
	else if(scancode == 0xe0)
		extend = true;
	else if(release)
		release = extend = false;
	else if (extend) {
		if (scancode <= sizeof(scancode_extend_tbl) / sizeof(char *) && scancode_extend_tbl[scancode])
			printk("%s", scancode_extend_tbl[scancode]);
		else
			printk("BUG: unrecognized keyboard scancode: %x\n", (uint32_t)scancode);
		extend = 0;
	} else if (scancode <= sizeof(scancode_tbl) / sizeof(char *) && scancode_tbl[scancode])
		printk("%s", scancode_tbl[scancode]);
	else
		printk("BUG: unrecognized keyboard scancode: %x\n", (uint32_t)scancode);

out:
	;
})

DEFINE_IRQ_HANDLER(mouse_irq_handler, {
	printk("MOUSE Interrupt! %x\n", (uint32_t)read_data());
})

void kbc_init() {
	uint8_t t;

	/* Disable devices */
	command(CMD_PORT1_DISABLE); 
	command(CMD_PORT2_DISABLE);

	/* Flush output buffer */
	while(read_status() & STATUS_OBF)
		// TODO: need delay
		inb(DATA_PORT);

	/* Set configuration byte */
	t = read_cfg();
	t &= ~(CFG_PORT1_INT | CFG_PORT2_INT | CFG_TRANSLATION);
	write_cfg(t);

	/* Test controller */
	command(CMD_CTL_TEST);
	t = read_data();
	if (t != 0x55) {
		printk("kbc: controller self-test failed. code: %x\n", (int32_t)t);
		return;
	}

	/* Test ports */
	command(CMD_PORT1_TEST);
	t = read_data();
	if (t) {
		printk("kbc: port 1 self-test failed. code: %x\n", (int32_t)t);
		return;
	}

	command(CMD_PORT2_TEST);
	t = read_data();
	if (t) {
		printk("kbc: port 2 self-test failed. code: %x\n", (int32_t)t);
		return;
	}

	/* Enable ports */
	command(CMD_PORT1_ENABLE);
	command(CMD_PORT2_ENABLE);

	/* Enable IRQ */
	t = read_cfg();
	t |= CFG_PORT1_INT | CFG_PORT2_INT;
	write_cfg(t);

	kbd_init();
	mouse_init();

	/* Setup IRQ handlers */
	register_intr_handler(0x21, kbd_irq_handler);
	ioapic_irq_setup(0x21, 1);

//	register_intr_handler(0x22, mouse_irq_handler);
//	ioapic_irq_setup(0x22, 2);
}
