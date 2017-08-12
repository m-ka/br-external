#include <linux/module.h>
#include <linux/platform_device.h>

#define DRV_NAME                  "barx-storage"
#define SYSFS_DIR_NAME            "barx"

static const struct attribute_group dev_attr_root_group;
struct driver_data {
	unsigned int read_cntr;
	unsigned int write_cntr;
	unsigned int size;
};

static int init_storage(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct state *dpriv;

	ret = sysfs_create_group(&dev->kobj, &dev_attr_root_group);
	if (ret) {
		dev_err(dev, "sysfs group creation failed\n");
		return ret;
	}

	return 0;
}

static int remove_storage(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	sysfs_remove_group(&dev->kobj, &dev_attr_root_group);

	return 0;
}

static ssize_t read_cntr_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	int len;

	return len;
}

static ssize_t write_cntr_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	int len;

	return len;
}

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
MODULE_DESCRIPTION("Test driver implementing small storage accessable as a char device");
