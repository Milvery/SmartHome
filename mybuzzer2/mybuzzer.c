#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include "mybuzzer.h"

#define CNAME "mybeef"
int gpiono;
int major=0;
unsigned int minor=0;
struct class *cls;
struct device *dev;
struct cdev *cdev;
unsigned int minor;



int beef_open(struct inode *inode,struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

long beef_ioctl(struct file *filp,
		unsigned int cmd,unsigned long args)
{
	switch(cmd){
		case beef_on:
			gpio_direction_output(gpiono,1);
			break;
		case beef_off:
			gpio_direction_output(gpiono,0);
			break;
	}
	return 0;
}

int beef_close(struct inode *inode,struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

const struct file_operations fops={
	.open=beef_open,
	.release=beef_close,
	.unlocked_ioctl=beef_ioctl,
};

int pdrv_probe(struct platform_device *pdev)
{
	int ret;
	dev_t devno;
	printk("%s:%d\n",__func__,__LINE__);
	//获取资源
	gpiono=of_get_named_gpio(pdev->dev.of_node,"beef1",0);
	if(gpiono<0)
	{
		printk("get gpio number error\n");
		return gpiono;
	}

	//申请gpio
	ret=gpio_request(gpiono,NULL);
	if(ret<0){
		printk("gpio request error\n");
		return ret;
	}

	gpio_direction_output(gpiono,0);
	//分配对象
		cdev = cdev_alloc();
	if(cdev==NULL)
	{
		printk("cdev alloc error\n");
		ret = -ENOMEM;
		goto ERR1;
	}
	//对象初始化
	cdev_init(cdev,&fops);

	//设备号的申请
	if(major > 0){
		ret=register_chrdev_region(MKDEV(major,minor),1,CNAME);
		if(ret){
			printk("dynamic:alloc device number error\n");
			goto ERR2;
		}
	}else{
			ret = alloc_chrdev_region(&devno,minor,1,CNAME);
			if(ret){
				printk("dynamic:alloc device number error\n");
				goto ERR2;
			}
		major=MAJOR(devno);
		minor=MINOR(devno);
	}

	//注册字符设备驱动
	ret=cdev_add(cdev,MKDEV(major,minor),1);
	if(ret){
		printk("cdev register error\n");
		goto ERR3;
	}

	//自动创建设备节点
	cls=class_create(THIS_MODULE,"mybeef");
	if(IS_ERR(cls)){
		printk("class create error\n");
		ret = PTR_ERR(cls);
		goto ERR4;
	}

	dev=device_create(cls,NULL,MKDEV(major,minor),NULL,"mybeef");

	if(IS_ERR(dev)){
			printk("device create error\n");
			ret = PTR_ERR(dev);
			goto ERR5;
	}
	return 0;

ERR5:
	device_destroy(cls,MKDEV(major,minor));
	class_destroy(cls);
ERR4:
	cdev_del(cdev);
ERR3:
	unregister_chrdev_region(MKDEV(major,minor),1);
ERR2:
	kfree(cdev);
ERR1:
	return ret;

/*
	//注册字符设备驱动
	major = register_chrdev(0,CNAME,&fops);
	if(major<0){
		printk("register chrdev error\n");
		ret = major;
		return ret;
	}

	//创建设备节点
	cls=class_create(THIS_MODULE,CNAME);
	if(IS_ERR(cls)){
		printk("class create error\n");
		return PTR_ERR(cls);
	}
	dev=device_create(cls,NULL,MKDEV(major,0),NULL,CNAME);
	if(IS_ERR(dev)){
		printk("device create error\n");
		return PTR_ERR(dev);
	}
*/

}


int pdrv_remove(struct platform_device *pdev)
{
	printk("%s:%d\n",__func__,__LINE__);

	device_destroy(cls,MKDEV(major,minor));

	class_destroy(cls);

	cdev_del(cdev);

	unregister_chrdev_region(MKDEV(major,minor),1);

	kfree(cdev);
	gpio_free(gpiono);

	return 0;
}

struct platform_device_id idtable[] = {
	{"hello0",0},
	{"hello1",1},
	{"hello2",2},
	{/*END*/}, 
};

struct of_device_id oftable[] = {
	{.compatible="hqyj,hello2",},
	{/*end*/}
};

struct platform_driver pdrv={
	.probe=pdrv_probe,
	.remove=pdrv_remove,
	.driver={
		.name="woyaogaoxin",
		.of_match_table=oftable,
	},
	.id_table=idtable,
};

module_platform_driver(pdrv);
MODULE_LICENSE("GPL");



