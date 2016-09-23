.set MAGIC, 0xE85250D6	# multiboot2 magic number
.set ARCH, 0	# architecture (0: intel 32 bit protected mode)
.set HEADER_LEN, multiboot_header_end - multiboot_header	# length of the multiboot2 header
.set CHECKSUM, -(MAGIC + ARCH + HEADER_LEN)	# checksum

.section .multiboot
.align 8
multiboot_header:
.long MAGIC
.long ARCH
.long HEADER_LEN
.long CHECKSUM

# The end tag
.short 0	# type: 0
.short 0	# flags: 0
.long 8		# size: 8
multiboot_header_end:

.section .bss
.align 16

stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
	// Save multiboot boot info address
	mov %eax, multiboot_info_magic
	mov %ebx, multiboot_info_addr

	mov $stack_top, %esp
	call arch_init
1:	hlt
	jmp 1b

.size _start, . - _start
