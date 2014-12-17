#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>

struct love {
	char data[128];
	struct list_head more_love;
};

struct list_head rainbows;

void list_all_love(void)
{
	struct love * ptr;

	list_for_each_entry(ptr, &rainbows, more_love) {
		printk(KERN_INFO "elem: %s\n", ptr->data);
	}
}

static ssize_t get_love(struct kobject *kobj, struct attribute *attr, char *buf)
{
	int len;
	struct love * ptr;

	if (list_empty(&rainbows)) {
		return 0;
	} else {
		ptr = list_first_entry(&rainbows, struct love, more_love);
		len = sprintf(buf, "%s", ptr->data);
		list_del(&ptr->more_love);
		kfree(ptr);
		return len;
	}
}

static ssize_t add_love(struct kobject *kobj, struct attribute *attr,
		const char *buf, size_t count)
{
	// FIXME: missing lock..

	int len;
	struct love * new;

	// new love object
	len = count > 127? 127 : count;
	new = kmalloc(sizeof(struct love), GFP_KERNEL);
	memcpy(new->data, buf, len);
	new->data[len] = 0;

	// add love object to rainbows collections
	list_add(&new->more_love, &rainbows);

	// dmesg all the loves
	list_all_love();

	return count;
}

struct attribute stack_file_attr = {
	.name = "rainbow_stack",
	.mode = 0666
};

struct sysfs_ops module_file_ops = {
	.show = get_love,
	.store = add_love
};

struct kobj_type module_type = {
	.sysfs_ops = &module_file_ops
};

struct kobject custom_dir;

static int __init start_licorn_factory(void)
{
	int ret;

	INIT_LIST_HEAD(&rainbows);
	kobject_init(&custom_dir, &module_type);
	ret = kobject_add(&custom_dir, &THIS_MODULE->mkobj.kobj, "custom");
	if (ret) {
		printk("Error adding kobject\n");
		kobject_put(&custom_dir);
		return ret;
	}

	ret = sysfs_create_file(&custom_dir, &stack_file_attr);
	if (ret) {
		printk("Error creating file\n");
		kobject_put(&custom_dir);
		return ret;
	}

	printk(KERN_INFO "start a licorn factory!\n");
	return 0;
}

static void __exit stop_licorn_factory(void)
{
	// FIXME: free nonempty list at the end
	sysfs_remove_file(&custom_dir, &stack_file_attr);
	kobject_put(&custom_dir);
}


module_init(start_licorn_factory);
module_exit(stop_licorn_factory);

MODULE_AUTHOR("URLAB LICORN CORP");
MODULE_DESCRIPTION("The start of a one horned mighty adventure!");
MODULE_LICENSE("GPL"); // no kidding...
