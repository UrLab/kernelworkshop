#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

static int howmany = 1;

struct love {
	char data[128];
	struct list_head more_love;
};

struct licorn {
	struct list_head rainbows;
	struct cdev device;
	int minor, readed;
};

static dev_t dev_licorn_entry;
static int dev_licorn_major;
struct licorn * licorns;

static void printk_all_love(struct licorn *l)
{
	struct love *ptr;

	list_for_each_entry(ptr, &l->rainbows, more_love) {
		printk(KERN_INFO "elem  %s\n", ptr->data);
	}
}

static ssize_t get_some_love(struct licorn * l, char __user *buf, const int max_len)
{
	int len, r;
	struct love * ptr;

	if (list_empty(&l->rainbows)) {
		return 0;
	} else {
		ptr = list_first_entry(&l->rainbows, struct love, more_love);
		len = strlen(ptr->data);
		len = (len > max_len? max_len : len) + 1;
		r = copy_to_user(buf, ptr->data, len);
		if (r) {
			printk(KERN_INFO "something bad happened\n");
		}
		list_del(&ptr->more_love);
		kfree(ptr);
		return len;
	}
}

static ssize_t add_love(struct licorn * l, const char __user *buf, size_t count)
{
	int len, r;
	struct love * new;

	// new love object
	len = count > 127? 127 : count;
	new = kmalloc(sizeof(struct love), GFP_KERNEL);
	r = copy_from_user(new->data, buf, len);
	if (r) {
		printk(KERN_INFO "add love error\n");
	}

	// truncate \n if needed
	if (len > 1 && new->data[len-1] == '\n')
		new->data[len-1] = 0;

	// ensure null-terminaison
	new->data[len] = 0;

	// add love object to rainbows collections
	list_add(&new->more_love, &l->rainbows);
	return count;
}

static int licorn_open(struct inode *inode, struct file *filp)
{
	struct licorn * l;
	l = container_of(inode->i_cdev, struct licorn, device);
	l->readed = 0;
	filp->private_data = l;
	return 0; // success \o/
}

static ssize_t licorn_read(struct file * filp, char __user * buf, size_t len, loff_t * o)
{
	int llen;
	struct licorn * l = filp->private_data;

	printk(KERN_INFO "licorn read():\n");
	printk_all_love(l);

	if (l->readed) {
		// easy way, block on the next read, without reopen
		return 0;
	} else {
		llen = get_some_love(l, buf, len);
		*o += llen;
		return llen;
	}
}

static ssize_t licorn_write(struct file * filp, const char __user *buf, size_t len, loff_t * o)
{
	struct licorn * l = filp->private_data;
	printk(KERN_INFO "licorn write():\n");
	printk_all_love(l);
	return add_love(l, buf, len);
}

struct file_operations licorn_fops = {
	.owner = THIS_MODULE,
	.read = licorn_read,
	.write = licorn_write,
	.open = licorn_open
};

static int __init start_licorn_factory(void)
{
	int ret, i, devno;

	// get a major number
	ret = alloc_chrdev_region(&dev_licorn_entry, 0, howmany, "licorn");
	if (ret) {
		printk(KERN_WARNING "could not alloc a device region\n");
		return ret;
	}
	dev_licorn_major = MAJOR(dev_licorn_entry);

	// alloc all our instance
	// FIXME should check return code
	licorns = kmalloc(sizeof(struct licorn) * 4, GFP_KERNEL);
	for (i = 0; i < howmany; ++i) {
		licorns[i].minor = i;
		INIT_LIST_HEAD(&licorns[i].rainbows);
		cdev_init(&licorns[i].device, &licorn_fops);
		licorns[i].device.owner = THIS_MODULE;
		devno = MKDEV(dev_licorn_major, licorns[i].minor);
		ret = cdev_add(&licorns[i].device, devno, 1);
		if (ret) {
			printk(KERN_WARNING "wrong cdev add for %d dev\n", i);
		}
	}

	printk(KERN_INFO "start a licorn factory!\n");
	return 0;
}

static void __exit stop_licorn_factory(void)
{
	int i;

	for (i = 0; i < howmany; ++i) {
		// FIXME should kfree non empty list
		cdev_del(&licorns[i].device);
	}

	unregister_chrdev_region(dev_licorn_entry, howmany);
}


module_init(start_licorn_factory);
module_exit(stop_licorn_factory);
module_param(howmany, int, S_IRUGO);

MODULE_AUTHOR("URLAB LICORN CORP");
MODULE_DESCRIPTION("The start of a one horned mighty adventure!");
MODULE_LICENSE("GPL"); // no kidding...
