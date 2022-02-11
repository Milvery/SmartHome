#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
//#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/uaccess.h>


#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/io.h>

#include "myleds.h"

/*
	myplatform{
		compatible="hqyj,hello2";
		led1=<&gpioe 10 0>;
		reg=<0x12345678 0x40>;
		interrupt-parent=<&gpiof>;
		interrupts=<9 0>;
	};

*/

#define CNAME "myleds"
#define COUNT 3

int gpiono;
int major = 0;
unsigned int minor=0;
struct cdev *cdev;
struct class *cls;
struct device *dev;
//struct device_node *pnode,*cnode,*enode;
char *ledpname[] = {"led1","led2","led3"};
char *ledpname_2[] = {"led4","led5","led6"};
unsigned int cgpiono[1] = {0};
//unsigned int egpiono[1] = {0};

//struct timer_list mytimer;


int myleds_open(struct inode *inode, struct file *filp)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}


ssize_t myleds_read(struct file *filp,
	char __user *ubuf, size_t size, loff_t *offs)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

/*
void timer_function(struct timer_list *timer_list)
{
	int i;
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],!gpio_get_value(cgpiono[i]));
		gpio_set_value(egpiono[i],!gpio_get_value(egpiono[i]));
	}
	
	mod_timer(&mytimer,jiffies+HZ);
}
*/
long myleds_ioctl(struct file *file,
		unsigned int cmd,unsigned long arg)
{
	int i;

	switch (cmd){
		case LEDS_ON:


	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
				gpio_direction_output(cgpiono[i],1);
	}
//	for(i=0;i<ARRAY_SIZE(egpiono);i++){
//				gpio_direction_output(egpiono[i],1);
//	}

	//5.初始化定时器
//	mytimer.expires = jiffies + HZ;
//	timer_setup(&mytimer,timer_function,0);
//	add_timer(&mytimer);
		

			break;
		case LEDS_OFF:
			
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],0);

	}
//	for(i=0;i<ARRAY_SIZE(egpiono);i++){
//		gpio_set_value(egpiono[i],0);

//	}
		


	//5.初始化定时器
//	mytimer.expires = jiffies + HZ;
//	timer_setup(&mytimer,timer_function,0);
//	add_timer(&mytimer);

		break;
		default:
			printk("input cmd error\n");
			return -EINVAL;
	}
	return 0;
}


int myleds_close(struct inode *inode, struct file *filp)
{
//	int i;

	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
//	del_timer(&mytimer);
	
/*	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],0);

		gpio_free(cgpiono[i]);
	}
	for(i=0;i<ARRAY_SIZE(egpiono);i++){
		gpio_set_value(egpiono[i],0);

		gpio_free(egpiono[i]);
	}

*/

	return 0;
}

const struct file_operations fops = {
	.open = myleds_open,
	.read = myleds_read,
	.unlocked_ioctl = myleds_ioctl,
	.release = myleds_close,
};


int pdrv_probe(struct platform_device *pdev)
{
	int i,ret;
	dev_t devno;
	printk("%s:%d\n",__func__,__LINE__);
	//获取资源
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		cgpiono[i]=of_get_named_gpio(pdev->dev.of_node,ledpname[i],0);
		if(cgpiono[i]<0)
		{
			printk("get gpio number error\n");
			return cgpiono[i];
		}


	//申请gpio
	ret=gpio_request(cgpiono[i],NULL);
	if(ret<0){
		printk("gpio request error\n");
		return ret;
	}



	gpio_direction_output(cgpiono[i],0);
}
/*
	//获取资源
	for(i=0;i<ARRAY_SIZE(egpiono);i++){
		egpiono[i]=of_get_named_gpio(pdev->dev.of_node,ledpname_2[i],0);
		if(egpiono[i]<0)
		{
			printk("get gpio number error\n");
			return cgpiono[i];
		}

	//申请gpio
	ret=gpio_request(egpiono[i],NULL);
	if(ret<0){
		printk("gpio request error\n");
		return ret;
	}

	gpio_direction_output(egpiono[i],0);
}
*/
	//1.分配对象
	cdev = cdev_alloc();
	if(cdev == NULL){
		printk("cdev alloc error\n");
		ret = -ENOMEM;
		goto ERR1;
	}

	//2.对象初始化
	cdev_init(cdev,&fops);

	//3.设备号的申请
	if(major > 0){
		ret = register_chrdev_region(MKDEV(major,minor),COUNT,CNAME);
		if(ret){
			printk("static:alloc device number error\n");
			goto ERR2;
		}
	}else{
		ret = alloc_chrdev_region(&devno,minor,COUNT,CNAME);

		if(ret){
			printk("dynamic:alloc device number error\n");
			goto ERR2;
		}

		major = MAJOR(devno);
		minor = MINOR(devno);
		}
		printk("major = %d,minor = %d\n",major,minor);

		//4.注册
		ret = cdev_add(cdev,MKDEV(major,minor),COUNT);
		if(ret){
			printk("cdev register error\n");
			goto ERR3;
		}

			//5.自动创建设备节点
	cls = class_create(THIS_MODULE,"myleds");
	if(IS_ERR(cls)){
		printk("class create error\n");
		ret = PTR_ERR(cls);
		goto ERR4;
	}
	for(i=0;i<COUNT;i++){
		dev = device_create(cls,NULL,MKDEV(major,minor+i),
			NULL,"myleds%d",i);
		if(IS_ERR(dev)){
			printk("device create error\n");
			ret = PTR_ERR(dev);
			goto ERR5;
		}
	}
	return 0;

ERR5:
	for(--i;i>=0;i--){
		device_destroy(cls,MKDEV(major,minor+i));
	}
	class_destroy(cls);

ERR4:
	cdev_del(cdev);
ERR3:
	unregister_chrdev_region(MKDEV(major,minor),COUNT);
ERR2:
	kfree(cdev);
ERR1:
	return ret;

}


int pdrv_remove(struct platform_device *pdev)
{
	int i;

	printk("%s:%d\n",__func__,__LINE__);

	for(i=0;i<COUNT;i++){
		device_destroy(cls,MKDEV(major,minor+i));
	}

	class_destroy(cls);

	cdev_del(cdev);

	unregister_chrdev_region(MKDEV(major,minor),3);

	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],0);

		gpio_free(cgpiono[i]);
	}

/*	for(i=0;i<ARRAY_SIZE(egpiono);i++){
		gpio_set_value(egpiono[i],0);

		gpio_free(egpiono[i]);
	}
*/
	return 0;
}

struct of_device_id oftable[] = {
	{.compatible = "hqyj,hello0",},
	{.compatible = "hqyj,hello1",},
	{.compatible = "hqyj,hello2",},
	{/*end*/}
};



struct platform_driver pdrv={
	.probe=pdrv_probe,
	.remove=pdrv_remove,
	.driver={
		.name="myplatform",
		.of_match_table=oftable,	
	},
};

module_platform_driver(pdrv);
MODULE_LICENSE("GPL");

/*
static int __init  myleds_init(void)
{
	int i,ret;
	dev_t devno;


		//3.3.解析到gpio号
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
//	for(i=0;i<3;i++){
		cgpiono[i] = of_get_named_gpio(cnode,ledpname[i],0);
		if(cgpiono[i] < 0){
			printk("of get named gpio error\n");
			return cgpiono[i];
		}
		egpiono[i] = of_get_named_gpio(enode,ledpname[i],0);
		if(egpiono[i] < 0){
			printk("of get named gpio error\n");
			return egpiono[i];
		}
		//4.使用gpio子系统的函数接口操作LED
		if((ret = gpio_request(cgpiono[i],NULL))<0){
			printk("gpio request error\n");
			return ret;
		}
		if((ret = gpio_request(egpiono[i],NULL))<0){
			printk("gpio request error\n");
			return ret;
		}

	}	

}

*/
/*
static void __exit myleds_exit(void)
{
	int i;
	del_timer(&mytimer);
	
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],0);
		gpio_set_value(egpiono[i],0);

		gpio_free(cgpiono[i]);
		gpio_free(egpiono[i]);
	}
	
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major,CNAME);:w

	for(i=0;i<COUNT;i++){
		device_destroy(cls,MKDEV(major,minor+i));
	}
	class_destroy(cls);

	cdev_del(cdev);

	unregister_chrdev_region(MKDEV(major,minor),COUNT);

	kfree(cdev);


}

*/


