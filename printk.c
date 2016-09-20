#include <stdarg.h>
#include <stdlib.h>

extern void (*kputc)(char c);

static const size_t conv_buf_len = sizeof(long) * 8 + 10;
static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static void reverse(char * s, size_t len)
{
	char t;
	size_t i;

	if (!len)
		return;

	len--;

	for (i = 0; i < len; i++, len--) {
		t = s[i];
		s[i] = s[len];
		s[len] = t;
	}
}

static char * utoa(unsigned int val, char * buf, int base)
{
	size_t i;
	unsigned int r;

	if (base < 2 || base > 36) {
		buf[0] = '\0';
		return NULL;
	}

	i = 0;
	do {
		r = val % base;
		val /= base;
		buf[i++] = digits[r];
	} while(val);
	buf[i] = '\0';

	reverse(buf, i);

	return buf;
}

static char * itoa(int val, char * buf, int base)
{
	if (base < 2 || base > 36) {
		buf[0] = '\0';
		return NULL;
	}

	if (val < 0) {
		buf[0] = '-';
		utoa(-val, buf + 1, base);
	} else {
		utoa(val, buf, base);
	}

	return buf;
}

static char * ultoa(unsigned long val, char * buf, int base)
{
	size_t i;
	unsigned int r;

	if (base < 2 || base > 36) {
		buf[0] = '\0';
		return NULL;
	}

	i = 0;
	do {
		r = val % base;
		val /= base;
		buf[i++] = digits[r];
	} while(val);
	buf[i] = '\0';

	reverse(buf, i);

	return buf;
}

static char * ltoa(long val, char * buf, int base)
{
	if (base < 2 || base > 36) {
		buf[0] = '\0';
		return NULL;
	}

	if (val < 0) {
		buf[0] = '-';
		ultoa(-val, buf + 1, base);
	} else {
		ultoa(val, buf, base);
	}

	return buf;
}

static void kputs(char * s)
{
	while(*s)
		kputc(*s++);
}

void printk(char * msg, ...)
{
	union {
		int i;
		unsigned int u;
		long l;
		unsigned long ul;
		char * s;
	} t;

	char conv_buf[conv_buf_len];
	va_list args;

	va_start(args, msg);

	while(*msg) {
		if (*msg == '%') {
			switch (*(msg + 1)) {
			case 'l':
				switch (*(msg + 2)) {
				case 'd':
					t.l = va_arg(args, long);
					ltoa(t.l, conv_buf, 10);
					kputs(conv_buf);
					msg += 3;
					continue;
				case 'u':
					t.ul = va_arg(args, unsigned long);
					ultoa(t.ul, conv_buf, 10);
					kputs(conv_buf);
					msg += 3;
					continue;
				case 'x':
					t.ul = va_arg(args, unsigned long);
					ultoa(t.ul, conv_buf, 16);
					kputs(conv_buf);
					msg += 3;
					continue;
				}
				break;
			case 'd':
				t.i = va_arg(args, int);
				itoa(t.i, conv_buf, 10);
				kputs(conv_buf);
				msg += 2;
				continue;
			case 'u':
				t.u = va_arg(args, unsigned int);
				utoa(t.u, conv_buf, 10);
				kputs(conv_buf);
				msg += 2;
				continue;
			case 'x':
				t.u = va_arg(args, unsigned int);
				utoa(t.u, conv_buf, 16);
				kputs(conv_buf);
				msg += 2;
				continue;
			case 's':
				t.s = va_arg(args, char *);
				kputs(t.s);
				msg += 2;
				continue;
			case 'c':
				t.i = va_arg(args, int);
				kputc((char)t.i);
				msg += 2;
				continue;
			case '%':
				kputc('%');
				msg += 2;
				continue;
			}
		}

		kputc(*msg);
		msg++;
	}

	va_end(args);
}
