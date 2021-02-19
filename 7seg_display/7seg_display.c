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

#include "7seg_display.h"

#define MODULE_NAME "7seg_display-gpio"


static int seg_probe(struct platform_device *pdev){

        pr_info("Driver Registered with Platform Core\n");

        return 0;
}

static int seg_remove(struct platform_device *pdev){

        pr_info("Driver unregistered\n");

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
MODULE_DESCRIPTION("7 segment Display Driver");
MODULE_LICENSE("GPL");
