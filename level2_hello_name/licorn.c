#include <linux/module.h>
#include <linux/kernel.h>

static char licorn_name[128] = "";

static ssize_t print_licorn_name(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return sprintf(buf, "%s", licorn_name);
}

static ssize_t set_licorn_name(struct kobject *kobj, struct attribute *attr,
		const char *buf, size_t count)
{
	int len = count > 127? 127 : count;
	memcpy(licorn_name, buf, len);
	licorn_name[len] = 0;
	return count;
}

static ssize_t hello_licorn(struct kobject *kobj, struct attribute *attr,
		const char *buf, size_t count)
{
	printk(KERN_INFO "hello %s!\n", licorn_name);
	return count;
}

static ssize_t licorn_magic_show(struct kobject *obj, struct attribute *attr, char *buf)
{
	return print_licorn_name(obj, attr, buf);
}

static ssize_t licorn_store(struct kobject *obj, struct attribute *attr,
		const char *buf, size_t count)
{
	if (!strcmp(attr->name, "name")) {
		return set_licorn_name(obj, attr, buf, count);
	} else {
		return hello_licorn(obj, attr, buf, count);
	}
}

struct attribute name_file_attr = {
	.name = "name",
	.mode = 0666
};

struct attribute hello_file_attr = {
	.name = "hello",
	.mode = 0222
};

struct sysfs_ops module_file_ops = {
	.show = licorn_magic_show,
	.store = licorn_store
};

struct kobj_type module_type = {
	.sysfs_ops = &module_file_ops
};

struct kobject custom_dir;

static int __init start_licorn_factory(void)
{
	int ret;

	kobject_init(&custom_dir, &module_type);
	ret = kobject_add(&custom_dir, &THIS_MODULE->mkobj.kobj, "custom");
	if (ret) {
		printk("Error adding kobject\n");
		kobject_put(&custom_dir);
		return ret;
	}

	ret = sysfs_create_file(&custom_dir, &hello_file_attr);
	ret = sysfs_create_file(&custom_dir, &name_file_attr);

	printk(KERN_INFO "start a licorn factory!\n");
	return 0;
}

static void __exit stop_licorn_factory(void)
{
	sysfs_remove_file(&custom_dir, &hello_file_attr);
	sysfs_remove_file(&custom_dir, &name_file_attr);
	kobject_put(&custom_dir);
}


module_init(start_licorn_factory);
module_exit(stop_licorn_factory);

MODULE_AUTHOR("URLAB LICORN CORP");
MODULE_DESCRIPTION("The start of a one horned mighty adventure!");
MODULE_LICENSE("GPL"); // no kidding...
