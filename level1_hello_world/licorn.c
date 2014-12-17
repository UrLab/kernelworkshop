#include <linux/module.h>
#include <linux/kernel.h>

static int __init start_licorn_factory(void)
{
	printk(KERN_INFO "Hello world\n");
	return 0;
}

static void __exit stop_licorn_factory(void)
{
	printk(KERN_INFO "Goodbye world\n");
}


module_init(start_licorn_factory);
module_exit(stop_licorn_factory);

MODULE_AUTHOR("URLAB LICORN CORP");
MODULE_DESCRIPTION("The start of a one horned mighty adventure!");
MODULE_LICENSE("GPL"); // no kidding...
