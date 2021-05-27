/*
 * Author : Milan Shah
 * This driver handles read/write operations on 24C32N eeprom(4096 bytes)
 * 24C32N should be connected on pin 19 (SCL) and pin 20(SDA) of beagle bone
 * black P9 header. Driver uses sysfs for user interaction.
 * GNU General Public License
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>

struct at24c32_prv {
        struct i2c_client *client;
        struct kobject *at24c32_kobj;
        char *ptr;
        unsigned int size;
        unsigned int pagesize;
        unsigned int address_width;
        unsigned int page_no;
};

static unsigned read_limit = 32;
static unsigned write_max = 32;
static unsigned write_timeout = 25;

static ssize_t at24c32_eeprom_read(struct i2c_client *client, char *buf,
		unsigned offset, size_t count)
{
	struct i2c_msg msg[2];
	u8 msgbuf[2];
	unsigned long timeout, read_time;
	int status;
	memset(msg, 0, sizeof(msg));
	memset(msgbuf, '\0', sizeof(msgbuf));
	if (count > read_limit)
		count = read_limit;

	msgbuf[0] = offset >> 8;
	msgbuf[1] = offset;

	msg[0].addr = client->addr;

	msg[0].flags = 0;
	msg[0].buf = msgbuf;

	msg[0].len = 2;
	
	msg[1].addr = client->addr;
	msg[1].flags = 1;
	msg[1].buf = buf;
	msg[1].len = count;

	timeout = jiffies + msecs_to_jiffies(write_timeout);
	do {
		read_time = jiffies;

		status = i2c_transfer(client->adapter, msg, 2);
		if (status == 2)
			status = count;
		if (status == count) {
			return count;
		}
		msleep(1);
	} while (time_before(read_time, timeout));

	return -ETIMEDOUT;
}

static ssize_t at24c32_eeprom_write(struct i2c_client *client, const char *buf,
				 unsigned offset, size_t count)
{
	struct i2c_msg msg;
	unsigned long timeout, write_time;
	ssize_t status;
	char msgbuf[40];

	memset(msgbuf, '\0', sizeof(msgbuf));
	if (count >= write_max)
		count = write_max;

	msg.addr = client->addr;

	msg.flags = 0;
	msg.buf = msgbuf;

	msg.buf[0] = (offset >> 8);
	msg.buf[1] = offset;

	memcpy(&msg.buf[2], buf, count);

	msg.len = count + 2;

	status = i2c_transfer(client->adapter, &msg, 1);
                if (status == 1)
                        status = count;
                if (status == count)
                        return count;

	timeout = jiffies + msecs_to_jiffies(write_timeout);
	do {
		write_time = jiffies;
		status = i2c_transfer(client->adapter, &msg, 1);
		if (status == 1)
			status = count;
		if (status == count)
			return count;
		msleep(1);
	} while (time_before(write_time, timeout));

	return -ETIMEDOUT;
}

static ssize_t
at24c32_sys_write(struct kobject *kobj, struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	ssize_t ret;
	struct at24c32_prv *at24c32 = i2c_get_clientdata(kobj_to_i2c_client(kobj));

	loff_t off = at24c32->page_no * 32;

	if (count > write_max) {
		pr_info("write length must be <= 32\n");
		return -EFBIG;
	}
	ret = at24c32_eeprom_write(at24c32->client, buf, (int)off, count);
	if (ret < 0) {
		pr_info("write failed\n");
		return ret;
	}

	return count;
}

static ssize_t
at24c32_sys_read(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	ssize_t ret;
	struct at24c32_prv *at24c32 = i2c_get_clientdata(kobj_to_i2c_client(kobj));
	
	loff_t off = at24c32->page_no * 32;
	
	ret = at24c32_eeprom_read(at24c32->client, buf, (int)off, at24c32->pagesize);
	if (ret < 0) {
		pr_info("error in reading\n");
		return ret;
	}
	
	return at24c32->pagesize;
}

static ssize_t
at24c32_get_offset(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct at24c32_prv *at24c32 = i2c_get_clientdata(kobj_to_i2c_client(kobj));
	pr_info("Page: %d\n", at24c32->page_no);

	return strlen(buf);
}

static ssize_t
at24c32_set_offset(struct kobject *kobj, struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int long tmp;
	struct at24c32_prv *at24c32 = i2c_get_clientdata(kobj_to_i2c_client(kobj));

	if (!kstrtol(buf, 16, &tmp)) {
		if (tmp < 128) {
			at24c32->page_no = tmp;
			pr_info("Page: %d\n", at24c32->page_no);
		} else {
			pr_info("128 pages are available\n");
			pr_info("Choose pages from 0x0 - 0x80\n");
		}
	}

	return count;
}

static struct kobj_attribute at24c32_rw = __ATTR(at24c32, 0660, at24c32_sys_read,
						at24c32_sys_write);
static struct kobj_attribute at24c32_offset = __ATTR(offset, 0660,
						at24c32_get_offset, at24c32_set_offset);

static struct attribute *attrs[] = {
        &at24c32_rw.attr,
        &at24c32_offset.attr,
        NULL,
};

static struct attribute_group attr_group = {
        .attrs = attrs,
};

static int at24c32_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct at24c32_prv *at24c32 = NULL;
	struct device *dev = &client->dev;

	pr_info("Device at24c32 probed......\n");

	at24c32 = devm_kzalloc(dev, sizeof(struct at24c32_prv), GFP_KERNEL);
	if (at24c32 == NULL) {
		pr_info("Requested memory not allocated\n");
		return -ENOMEM;
	}

	at24c32->client = client;

	ret = device_property_read_u32(dev, "size", &at24c32->size);
	if (ret) {
		dev_err(dev, "Error: missing \"size\" property\n");
		return -ENODEV;
	}

	ret = device_property_read_u32(dev, "pagesize", &at24c32->pagesize);
	if (ret) {
		dev_err(dev, "Error: missing \"pagesize\" property\n");
		return -ENODEV;
	}

	ret = device_property_read_u32(dev, "address-width", &at24c32->address_width);
	if (ret) {
		dev_err(dev, "Error: missing \"address-width\" property\n");
		return -ENODEV;
	}

	at24c32->at24c32_kobj = kobject_create_and_add("at24c32_eeprom", NULL);
	if(!at24c32->at24c32_kobj)
		return -ENOMEM;
	
	ret = sysfs_create_group(at24c32->at24c32_kobj, &attr_group);
	if(ret)
		kobject_put(at24c32->at24c32_kobj);
	
	i2c_set_clientdata(client, at24c32);

	pr_info("Sysfs entry created for AT24C32 device \n");
	pr_info("	SIZE		:%d\n", at24c32->size);
	pr_info("	PAGESIZE	:%d\n", at24c32->pagesize);
	pr_info("	address-width	:%d\n", at24c32->address_width);

	return ret;
}

static int at24c32_remove(struct i2c_client *client)
{
	struct at24c32_prv *at24c32 = i2c_get_clientdata(client);

	kobject_put(at24c32->at24c32_kobj);
	
	pr_info("at24c32_remove\n");

	return 0;
}

static const struct i2c_device_id at24c32_id[] = {
	{ "24c32", 0x50 },
	{ /* END OF LIST */ }
};

MODULE_DEVICE_TABLE(i2c, at24c32_id);

static const struct of_device_id at24c32_of_match[] = {
	{ .compatible = "atmel,at24c32" },
	{ /* END OF LIST */}
};

MODULE_DEVICE_TABLE(of, at24c32_of_match);

static struct i2c_driver at24c32_driver = {
	.driver = {
		.name = "eeprom-at24c32",
		.of_match_table = of_match_ptr(at24c32_of_match),
		.owner = THIS_MODULE,
	},
	.probe = at24c32_probe,
	.remove = at24c32_remove,
	.id_table = at24c32_id,
};

module_i2c_driver(at24c32_driver);

MODULE_DESCRIPTION("Driver for at24c32 eeprom");
MODULE_AUTHOR("Milan Shah");
MODULE_LICENSE("GPL");
