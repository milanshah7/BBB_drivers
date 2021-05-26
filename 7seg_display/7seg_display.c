/***************************************

* Name          : 7seg_display.c
* Created on    : Thursday 28 January 2021 02:40:08 PM IST
* Author        : Milan Shah <mshah.opensource@gmail.com>
* Description   :

****************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>

#include "7seg_display.h"

#define MODULE_NAME "7seg_display-gpio"
#define LABEL_NAME "7seg_display"

struct seg_dev_data {
	char label[10];
	struct gpio_desc *desc;
};

struct seg_drv_data {
	int deviceCount;
	struct class *seg_class;
	struct device **seg_dev;
} drv_data;

ssize_t value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct seg_dev_data *v_data = dev_get_drvdata(dev);
	int value;
	value = gpiod_get_value(v_data->desc);
	return sprintf(buf, "%d\n", value);
}

ssize_t value_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct seg_dev_data *v_data = dev_get_drvdata(dev);
	int ret = 0;
	long value;

	ret = kstrtol(buf, 0, &value);
	if(ret)
		return ret;

	gpiod_set_value(v_data->desc, value);

	return count;
}

ssize_t label_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct seg_dev_data *d_data = dev_get_drvdata(dev);
	return sprintf(buf, "%s\n", d_data->label);
}

static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RO(label);

static struct attribute *seg_attrs[] =
{
	&dev_attr_value.attr,
	&dev_attr_label.attr,
	NULL
};

static struct attribute_group seg_attr_group = 
{
	.attrs = seg_attrs
};

static const struct attribute_group *seg_attr_groups[] =
{
	&seg_attr_group,
	NULL
};

static int fetch_details_from_child_node(struct device *dev,
					  struct device_node *node,
					  struct seg_dev_data *data)
{
	int ret = 0;
	const char *labelName;

	/* fetch label from child node */
	ret = of_property_read_string(node, "label", &labelName);
	if(ret < 0) {
		dev_warn(dev, "Missing Label...\n");
		return ret;
	}
	else {
		strcpy(data->label, labelName);
		dev_info(dev,
			"GPIO Label : %s...\n", data->label);
	}

	/* fetch gpio descriptor from child node */
	data->desc = 
		devm_fwnode_get_gpiod_from_child(dev,
						"7seg",
						&node->fwnode,
						GPIOD_ASIS,
						data->label);
	if(IS_ERR(data->desc)){
		ret = PTR_ERR(data->desc);
		dev_err(dev,
			"No GPIO has been assigned to requested fun.\n");
		return ret;
	}

	return ret;
}

static int seg_probe(struct platform_device *pdev){

	int ret, i = 0;

	struct device_node *parentNode = pdev->dev.of_node;
	struct device_node *childNode = NULL;
	struct device *seg_dev = &pdev->dev;
	struct seg_dev_data *dev_data;

	/* Get the count of total child nodes associated with a parent */
	drv_data.deviceCount = of_get_child_count(parentNode);
	if(drv_data.deviceCount == 0) {
		dev_err(seg_dev, "No Device Found...\n");
		return -EINVAL;
	}
	else {
		dev_info(seg_dev,
			"Total Devices Found : %d...\n", drv_data.deviceCount);
		/* Increase the count by 1 as we need one more device */
		drv_data.deviceCount++;
	}

	/* Create a class file "bbb_drivers" */
	drv_data.seg_class = class_create(THIS_MODULE, "bbb_drivers");
	if(IS_ERR(drv_data.seg_class)) {
		pr_err("Error in creating class \n");
		return PTR_ERR(drv_data.seg_class);
	}

	drv_data.seg_dev = 
		devm_kzalloc(seg_dev,
			     sizeof(struct device *) * drv_data.deviceCount,
			     GFP_KERNEL);

	for_each_available_child_of_node(parentNode, childNode){

		dev_data = devm_kzalloc(seg_dev,
					sizeof(*dev_data),
					GFP_KERNEL);
		if(dev_data == NULL){
			dev_err(seg_dev, "Cannot allocate memory...\n");
			return -ENOMEM;
		}

		/* fetch GPIO details from device tree node */
		ret = fetch_details_from_child_node(seg_dev,
						    childNode,
						    dev_data);
		if(ret < 0){
			dev_err(seg_dev,
				"failed to fetch details from child node\n");
			return ret;
		}

		/* set the gpio direction to output */
		ret = gpiod_direction_output(dev_data->desc, 0);
		if(ret) {
			dev_err(seg_dev, "GPIO Direction Set Failed...\n");
			return ret;
		}

		/* Create devices under /sys/class/bbb_drivers */
		drv_data.seg_dev[i] =
			device_create_with_groups(drv_data.seg_class,
						  seg_dev,
						  0,
						  (void *)&dev_data,
						  seg_attr_groups,
						  dev_data->label);
		if(IS_ERR(drv_data.seg_dev[i])) {
			dev_err(seg_dev,
				"Error in device_create...\n");
			return PTR_ERR(drv_data.seg_dev[i]);
		}
		i++;
	}

	/* Create a device under /sys/class/bbb_drivers */
	drv_data.seg_dev[i] = 
		device_create_with_groups(drv_data.seg_class,
					 seg_dev,
					 0,
					 (void *)&dev_data,
					 seg_attr_groups,
					 LABEL_NAME);
	if(IS_ERR(drv_data.seg_dev[i])) {
		dev_err(seg_dev,
			"Error in device_create...\n");
		return PTR_ERR(drv_data.seg_dev[i]);
	}

        pr_info("Driver Registered Successfully with Platform Core\n");

        return ret;
}

static int seg_remove(struct platform_device *pdev)
{
	struct device *seg_dev = &pdev->dev;
	int i;

	/* Unregister all the devices */
	for(i = 0; i < drv_data.deviceCount; i++){
		device_unregister(drv_data.seg_dev[i]);
	}

	devm_kfree(seg_dev, drv_data.seg_dev);

	/* Destroy the class file bbb_drivers */
	class_destroy(drv_data.seg_class);

        pr_info("Driver Successfully Unregistered\n");

        return 0;
}

static struct of_device_id seg_of_mtable[] = {
        { .compatible = "7_segment" },
        { },
};

MODULE_DEVICE_TABLE(of, seg_of_mtable);

static struct platform_driver seg_driver = {
        .driver = {
                .name = MODULE_NAME,
                .owner = THIS_MODULE,
                .of_match_table = seg_of_mtable,
        },
        .probe = seg_probe,
        .remove = seg_remove,

};

module_platform_driver(seg_driver);

MODULE_AUTHOR("Milan Shah <milan.opensource@gmail.com>");
MODULE_DESCRIPTION("7-segment Display Driver");
MODULE_LICENSE("GPL");
