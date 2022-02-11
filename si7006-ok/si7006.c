#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "myioctl.h"
#define NAME "si7006"
struct i2c_client *gclient;
int major = 0;
struct class *cls;
struct device *dev;
int i2c_read_serial_firware(unsigned short reg)
{
	int ret;
	unsigned char val;
	char r_buf[] = {((reg>>8)&0xff),(reg&0xff)};
	struct i2c_msg r_msg[] = {
		[0] = {
			.addr = gclient->addr,
			.flags= 0,
			.len  = 2,
			.buf  = r_buf,
		},
		[1] = {
			.addr = gclient->addr,
			.flags= 1,
			.len  = 1,
			.buf  = &val,
		},
	};

	ret = i2c_transfer(gclient->adapter,r_msg,ARRAY_SIZE(r_msg));
	if(ret != ARRAY_SIZE(r_msg)){
		printk("i2c transfer error\n");
		return -EAGAIN;
	}

	return val;
}
int i2c_read_hum_tem(unsigned char reg)
{
	int ret;
	unsigned short val;
	struct i2c_msg r_msg[] = {
		[0] = {
			.addr = gclient->addr,
			.flags= 0,
			.len  = 1,
			.buf  = &reg,
		},
		[1] = {
			.addr = gclient->addr,
			.flags= 1,
			.len  = 2,
			.buf  =(char *)&val,
		},
	};
	ret = i2c_transfer(gclient->adapter,r_msg,ARRAY_SIZE(r_msg));
	if(ret != ARRAY_SIZE(r_msg)){
		printk("i2c transfer error\n");
		return -EAGAIN;
	}

	val = val >> 8 | val <<8; //高低位交换
	
	return val;
}
//字符设备驱动open
//字符设备驱动ioctl
//  READ_HUM
//  hum = i2c_read_hum_tem(0xe5)   copy_to_user
//  READ_TEM
//  tem = i2c_read_hum_tem(0xe3)   copy_to_user
//字符设备驱动close

int si7006_open(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}
long si7006_ioctl(struct file *file, 
	unsigned int cmd, unsigned long arg)
{
	int data,ret;
	switch(cmd){
		case READ_HUM:
			data = i2c_read_hum_tem(0xe5);
			if(data < 0){
				printk("i2c read hum error\n");
				return data;
			}	
			data = data & 0xffff;
			ret = copy_to_user((void *)arg,(void *)&data,sizeof(int));
			if(ret){
				printk("copy data to user error\n");
				return -EIO;
			}
			break;
		case READ_TEM:
			data = i2c_read_hum_tem(0xe3);
			if(data < 0){
				printk("i2c read tmp error\n");
				return data;
			}	
			data = data & 0xffff;
			ret = copy_to_user((void *)arg,(void *)&data,sizeof(int));
			if(ret){
				printk("copy data to user error\n");
				return -EIO;
			}
			break;
		default:
			printk("input cmd error\n");
			return -EINVAL;
	}
	
	return 0;
}

int si7006_close(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}
const struct file_operations fops = {
	.open = si7006_open,
	.unlocked_ioctl = si7006_ioctl,
	.release = si7006_close,
};


int si7006_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	unsigned int data;
	gclient = client;
	printk("%s:%d\n",__func__,__LINE__);
	data = i2c_read_serial_firware(0xfcc9);
	printk("serial(0x06) = %#x\n",data);
	data = i2c_read_serial_firware(0x84b8);
	printk("firware(0x20) = %#x\n",data);

	//注册字符设备驱动
	major = register_chrdev(0,NAME,&fops);
	if(major < 0){
		printk("register char device driver error\n");
		return -EAGAIN;
	}

	//自动创建设备节点
	cls = class_create(THIS_MODULE,NAME);
	if(IS_ERR(cls)){
		printk("class create error\n");
		return PTR_ERR(cls);
	}
	dev = device_create(cls,NULL,MKDEV(major,0),NULL,NAME);
	if(IS_ERR(dev)){
		printk("device create error\n");
		return PTR_ERR(dev);
	}
	return 0;
}
int si7006_remove(struct i2c_client *client)
{
	printk("%s:%d\n",__func__,__LINE__);

	device_destroy(cls,MKDEV(major,0));

	class_destroy(cls);

	unregister_chrdev(major,NAME);

	return 0;
}

const struct of_device_id of_table[] = {
	{.compatible = "hqyj,si7006",},
	{/*end*/},
};
MODULE_DEVICE_TABLE(of,of_table);

struct i2c_driver si7006 = {
	.probe = si7006_probe,
	.remove = si7006_remove,
	.driver = {
		.name = "haha",
		.of_match_table = of_table,	
	}
};


module_i2c_driver(si7006);
MODULE_LICENSE("GPL");

