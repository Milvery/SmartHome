#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include "myleds.h"

/*
myleds{
		compatible = "hqyj,leds";	
		core-leds{
			led1 = <&gpioz 5 0>;
			led2 = <&gpioz 6 0>;
			led3 = <&gpioz 7 0>;
		};
		extend-leds{
			led1 = <&gpioe 10 0>;
			led2 = <&gpiof 10 0>;
			led3 = <&gpioe 8 0>;
		};
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
struct device_node *pnode,*cnode,*enode;
char *ledpname[] = {"led1","led2","led3"};
unsigned int cgpiono[3] = {0};
unsigned int egpiono[3] = {0};

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
	}
	gpio_set_value(egpiono[0],!gpio_get_value(egpiono[0]));
	gpio_set_value(egpiono[2],!gpio_get_value(egpiono[2]));
//	gpio_set_value(egpiono[1],!gpio_get_value(egpiono[1]));

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

				gpio_direction_output(egpiono[0],1);
				gpio_direction_output(egpiono[2],1);
//				gpio_direction_output(egpiono[1],1);

		
			break;
		case LEDS_OFF:
	
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],0);

	}
			
		gpio_set_value(egpiono[0],0);
		gpio_set_value(egpiono[2],0);
//		gpio_set_value(egpiono[1],0);


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
	int i;

	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
//	del_timer(&mytimer);
	
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],0);
	
		gpio_free(cgpiono[i]);
	}
	gpio_set_value(egpiono[0],0);
	gpio_set_value(egpiono[2],0);
//	gpio_set_value(egpiono[1],0);
	gpio_free(egpiono[0]);
	gpio_free(egpiono[2]);
//	gpio_free(egpiono[1]);


	return 0;
}

const struct file_operations fops = {
	.open = myleds_open,
	.read = myleds_read,
	.unlocked_ioctl = myleds_ioctl,
	.release = myleds_close,
};

static int __init  myleds_init(void)
{
	int i,ret;
	dev_t devno;

	//3.解析设备数

	//3.1.获取父节点
	pnode = of_find_compatible_node(NULL,NULL,"hqyj,leds");
	if(pnode ==NULL){
		printk("get parent node error\n");
		return -ENODATA;
	}
	//3.2.获取子节点
	cnode = of_get_child_by_name(pnode,"core-leds");
	if(cnode ==NULL){
		printk("get core leds node error\n");
		return -ENODATA;
	}
	enode = of_get_child_by_name(pnode,"extend-leds");
	if(enode ==NULL){
		printk("get extend leds node error\n");
		return -ENODATA;
	}

		//3.3.解析到gpio号
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		cgpiono[i] = of_get_named_gpio(cnode,ledpname[i],0);
		if(cgpiono[i] < 0){
			printk("of get named gpio error\n");
			return cgpiono[i];
		}

		//4.使用gpio子系统的函数接口操作LED
		if((ret = gpio_request(cgpiono[i],NULL))<0){
			printk("gpio request error\n");
			return ret;
		}
	}	
		egpiono[0] = of_get_named_gpio(enode,ledpname[0],0);
		if(egpiono[0] < 0){
			printk("of get named gpio error\n");
			return egpiono[0];
		}

		egpiono[2] = of_get_named_gpio(enode,ledpname[2],0);
		if(egpiono[0] < 0){
			printk("of get named gpio error\n");
			return egpiono[2];
		}

/*
		egpiono[1] = of_get_named_gpio(enode,ledpname[1],0);
		if(egpiono[0] < 0){
			printk("of get named gpio error\n");
			return egpiono[2];
		}
*/
		if((ret = gpio_request(egpiono[0],NULL))<0){
			printk("gpio request error\n");
			return ret;
		}

		if((ret = gpio_request(egpiono[2],NULL))<0){
			printk("gpio request error\n");
			return ret;
		}
/*
		if((ret = gpio_request(egpiono[1],NULL))<0){
			printk("gpio request error\n");
			return ret;
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
//	}
	//5.初始化定时器
/*	mytimer.expires = jiffies + HZ;
	timer_setup(&mytimer,timer_function,0);
	add_timer(&mytimer);*/
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


static void __exit myleds_exit(void)
{
	int i;
//	del_timer(&mytimer);
	
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],0);
	
		gpio_free(cgpiono[i]);
	}

	gpio_set_value(egpiono[0],0);
	gpio_set_value(egpiono[2],0);
//	gpio_set_value(egpiono[1],0);

	gpio_free(egpiono[0]);
	gpio_free(egpiono[2]);
//	gpio_free(egpiono[1]);


	/*
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major,CNAME);*/
	for(i=0;i<COUNT;i++){
		device_destroy(cls,MKDEV(major,minor+i));
	}
	class_destroy(cls);

	cdev_del(cdev);

	unregister_chrdev_region(MKDEV(major,minor),COUNT);

	kfree(cdev);


}
module_init(myleds_init);
module_exit(myleds_exit);
MODULE_LICENSE("GPL");





