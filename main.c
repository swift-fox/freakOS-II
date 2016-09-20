void platform_init();
void printk(char * msg, ...);

void kernel_init()
{
	printk("Kernel initialized.\n");

//	for(;;)
//	print_debug();
}
