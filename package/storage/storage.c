#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>

#define DEV_MAJOR                 251
#define DRV_NAME                  "barx-storage"
#define SYSFS_DIR_NAME            "barx"
#define OF_PROP_NAME              "size"

static const struct attribute_group dev_attr_root_group;
static struct file_operations driver_fops;
struct driver_data {
	unsigned int read_cntr;
	unsigned int write_cntr;
	unsigned int size;
	char *data;

	dev_t dev_num;
	struct cdev cdev;
	int device_created;
};

static void clean_up(struct device *dev, struct driver_data *d_data);
static unsigned int of_get_size(struct device *dev);
static int init_storage(struct platform_device *pdev);
static int remove_storage(struct platform_device *pdev);
static void clean_up(struct device *dev, struct driver_data *d_data);
static loff_t storage_lseek(struct file *file, loff_t offset, int orig);
static ssize_t storage_read(struct file *file, char *buff, size_t count, loff_t *off);
static ssize_t storage_write(struct file *file, const char *buff, size_t count, loff_t *off);
static int storage_release(struct inode *inode, struct file *file);

static unsigned int of_get_size(struct device *dev)
{
	const uint32_t *buff_size;
	struct device_node *node;
	unsigned int ret_val = 0;

	node = of_find_node_by_name(NULL, DRV_NAME);
	if (!node)
		return ret_val;
	buff_size = of_get_property(node, OF_PROP_NAME, NULL);
	if (!buff_size) {
		dev_err(dev, "storage size is not specified in device tree\n");
		return ret_val;
	}
	ret_val = be32_to_cpup(buff_size);

	return ret_val;
}

static int init_storage(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct driver_data *d_data;

	d_data = devm_kzalloc(dev, sizeof(struct driver_data), GFP_KERNEL);
	if (!d_data) {
		dev_err(dev, "could not allocate memory for driver structure\n");
		return -ENOMEM;
	}
	d_data->size = of_get_size(dev);
	if (d_data->size == 0) {
		dev_err(dev, "got inappropriate storage size from device tree\n");
		clean_up(dev, d_data);
		return -EINVAL;
	}
	d_data->data = devm_kzalloc(dev, d_data->size, GFP_KERNEL);
	if (!d_data->data) {
		dev_err(dev, "could not allocate memory for data storage\n");
		clean_up(dev, d_data);
		return -ENOMEM;
	}

	d_data->dev_num = MKDEV(DEV_MAJOR, 0);
	ret = register_chrdev_region(d_data->dev_num, 1, DRV_NAME);
	if (ret < 0) {
		dev_err(dev, "could not allocate device number\n");
		clean_up(dev, d_data);
		return ret;
	}

	ret = sysfs_create_group(&dev->kobj, &dev_attr_root_group);
	if (ret) {
		dev_err(dev, "sysfs group creation failed\n");
		clean_up(dev, d_data);
		return ret;
	}

	cdev_init(&d_data->cdev, &driver_fops);
	d_data->cdev.owner = THIS_MODULE;
	ret = cdev_add(&d_data->cdev, d_data->dev_num, 1);
	if (ret) {
		dev_err(dev, "Could not add character device\n");
		clean_up(dev, d_data);
		return ret;
	}
	dev_set_drvdata(dev, d_data);
	d_data->device_created = 1;

	return 0;
}

static int remove_storage(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct driver_data *d_data = dev_get_drvdata(dev);

	cdev_del(&d_data->cdev);
	unregister_chrdev_region(d_data->dev_num, 1);
	sysfs_remove_group(&dev->kobj, &dev_attr_root_group);
	devm_kfree(dev, d_data->data);
	devm_kfree(dev, d_data);

	return 0;
}

static void clean_up(struct device *dev, struct driver_data *d_data)
{
	if (d_data->device_created) {
		cdev_del(&d_data->cdev);
		sysfs_remove_group(&dev->kobj, &dev_attr_root_group);
	}
	if (MAJOR(d_data->dev_num))
		unregister_chrdev_region(d_data->dev_num, 1);
	if (d_data->data)
		devm_kfree(dev, d_data->data);
	if (d_data)
		devm_kfree(dev, d_data);
}

static int storage_open(struct inode *inode, struct file *file)
{
	struct driver_data *d_data = container_of(inode->i_cdev, struct driver_data, cdev);
	file->private_data = d_data;

	return 0;
}

static loff_t storage_lseek(struct file *file, loff_t offset, int orig)
{
	loff_t new_pos = 0;
	struct driver_data *d_data = file->private_data;

	switch (orig) {
		case SEEK_SET:
			new_pos = offset;
			break;
		case SEEK_CUR:
			new_pos = file->f_pos + offset;
			break;
		case SEEK_END:
			new_pos = d_data->size - offset;
			break;
	}
	if ((new_pos >= 0) && (new_pos <= d_data->size))
		file->f_pos = new_pos;
	else
		new_pos = -EINVAL;

	return new_pos;
}

static ssize_t storage_read(struct file *file, char *buff, size_t count, loff_t *off)
{
	ssize_t ret_val = 0;
	struct driver_data *d_data = file->private_data;

	if (*off >= d_data->size)
		return ret_val;
	if (*off + count > d_data->size)
		count = d_data->size - *off;

	if (copy_to_user(buff, &d_data->data[*off], count))
		return -EFAULT;
	*off += count;
	d_data->read_cntr++;
	ret_val = count;

	return ret_val;
}

static ssize_t storage_write(struct file *file, const char *buff, size_t count, loff_t *off)
{
	ssize_t ret_val = 0;
	struct driver_data *d_data = file->private_data;

	if (*off >= d_data->size)
		return ret_val;
	if (*off + count > d_data->size)
		count = d_data->size - *off;

	if (copy_from_user(&d_data->data[*off], buff, count))
		return -EFAULT;
	*off += count;
	d_data->write_cntr++;
	ret_val = count;

	return ret_val;
}

static int storage_release(struct inode *inode, struct file *file)
{
	/* nothing to be done here */
	return 0;
}

static ssize_t read_cntr_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	struct driver_data *d_data = dev_get_drvdata(dev);

	return sprintf(buff, "%u\n", d_data->read_cntr);
}

static ssize_t write_cntr_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	struct driver_data *d_data = dev_get_drvdata(dev);

	return sprintf(buff, "%u\n", d_data->write_cntr);
}

static struct file_operations driver_fops = {
	.owner = THIS_MODULE,
	.open = storage_open,
	.read = storage_read,
	.write = storage_write,
	.llseek = storage_lseek,
	.release = storage_release
};

static DEVICE_ATTR(read_cntr, S_IRUSR | S_IRGRP | S_IROTH, read_cntr_show, NULL);
static DEVICE_ATTR(write_cntr, S_IRUSR | S_IRGRP | S_IROTH, write_cntr_show, NULL);
static struct attribute *root_dev_attrs[] = {
	&dev_attr_read_cntr.attr,
	&dev_attr_write_cntr.attr,
	NULL
};

static const struct attribute_group dev_attr_root_group = {
	.attrs = root_dev_attrs,
	.name = SYSFS_DIR_NAME,
};

static const struct of_device_id barx_storage_of_match[] = {
	{ .compatible = "barx,barx-storage" },
	{ /* end of list */ }
};
MODULE_DEVICE_TABLE(of, barx_storage_of_match);

static struct platform_driver barx_storage = {
	.probe  = init_storage,
	.remove = remove_storage,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = barx_storage_of_match,
	},
};
module_platform_driver(barx_storage);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Test driver implementing small storage accessible as a char device");
