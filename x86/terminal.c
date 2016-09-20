#include <stddef.h>
#include <stdint.h>

#define make_color(fg, bg) (fg | bg << 4)
#define make_entry(c, color) ((uint16_t)c | (uint16_t)color << 8)

enum vga_color {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

static const size_t vga_width = 80;
static const size_t vga_height = 25;
static const size_t tab_size = 8;
static const uint8_t default_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);

static uint16_t* terminal_buffer = (uint16_t*) 0xB8000;
static size_t row, col;

void (*kputc)(char c);

static void scroll_up() {
	size_t i;

	for (i = 0; i < (vga_height - 1) * vga_width; i++)
		terminal_buffer[i] = terminal_buffer[i + vga_width];

	for ( ; i < vga_height * vga_width; i++)
		terminal_buffer[i] = make_entry(' ', default_color);
}

static void terminal_putchar(char c) {
	switch (c) {
	case '\n':
		row++;
		col = 0;
		break;
	case '\t':
		col += tab_size - col % tab_size;
		break;
	default:
		terminal_buffer[row * vga_width + col] = make_entry(c, default_color);
		col++;
	}

	if (col >= vga_width) {
		row++;
		col = 0;
	}

	if (row >= vga_height) {
		scroll_up();
		row = vga_height - 1;
	}
}

void terminal_init() {
	kputc = terminal_putchar;

	row = 0;
	col = 0;

	for (size_t i = 0; i < vga_height * vga_width; i++)
		terminal_buffer[i] = make_entry(' ', default_color);
}
