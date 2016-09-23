void arch_init();
void printk(char * msg, ...);

void main()
{
	arch_init();

	printk("Kernel initialized.\n");

//	for(;;)
//	print_debug();
}
