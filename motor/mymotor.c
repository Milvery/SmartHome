#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "mymotor.h"


#define CNAME "mymotor"
#define COUNT 3
int major=0;
unsigned int minor=0;


struct cdev *cdev;
struct class *cls;
struct device *dev;
int curminor;
char kbuf[128] = {0};
gpio_t *gpiof = NULL;
unsigned int * rcc;

#define motor_ON (gpiof->ODR   |=(1<<6)) //motor on
#define motor_OFF (gpiof->ODR  &=~(1<<6)) //motor off

int mycdev_open(struct inode *inode, struct file *filp)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	curminor = MINOR(inode->i_rdev);

	return 0;
}

ssize_t mycdev_read(struct file *filp, 
	char __user *ubuf, size_t size, loff_t *offs)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}


ssize_t mycdev_write(struct file *filp,
	const char __user *ubuf, size_t size, loff_t *offs)
{
	int ret;
//	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size > sizeof(kbuf)) size = sizeof(kbuf);
	ret = copy_from_user(kbuf,ubuf,size);
	if(ret){
		printk("copy data from user error\n");
		return -EINVAL;
	}
	kbuf[0] == 1?motor_ON:motor_OFF;

	return size;	
}


int mycdev_close(struct inode *inode, struct file *filp)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}

const struct file_operations fops = {
	.open = mycdev_open,
	.read = mycdev_read,
	.write = mycdev_write,
	.release = mycdev_close,
};

static int __init mycdev_init(void)
{
	int ret,i;
	dev_t devno;

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
	cls = class_create(THIS_MODULE,"mymotor");
	if(IS_ERR(cls)){
		printk("class create error\n");
		ret = PTR_ERR(cls);
		goto ERR4;
	}
	for(i=0;i<COUNT;i++){
		dev = device_create(cls,NULL,MKDEV(major,minor+i),
			NULL,"mymotor%d",i);
		if(IS_ERR(dev)){
			printk("device create error\n");
			ret = PTR_ERR(dev);
			goto ERR5;
		}
	}
	//1.映射地址
	gpiof = ioremap(GPIOF_PHY_BASE,sizeof(gpio_t));
	if(gpiof == NULL){
		printk("ioremap gpiof base error\n");
		return -ENOMEM;
	}
	
	rcc = ioremap(RCC_ENB,4);
	if(rcc == NULL){
		printk("ioremap rcc error\n");
		return -ENOMEM;
	}

		//2.初始化motor
	*rcc |= (1<<5);  //rcc enable gpiof
	gpiof->MODER &=~(3<<12);//clear bit 12-13
	gpiof->MODER |=(1<<12); //output
	gpiof->ODR	 &=~(1<<6); // motor off

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


static void __exit mycdev_exit(void)
{
	int i;
	gpiof->ODR   &=~(1<<6); //motor off
	iounmap(gpiof);
	iounmap(rcc);
	
	for(i=0;i<COUNT;i++){
		device_destroy(cls,MKDEV(major,minor+i));
	}
	class_destroy(cls);

	cdev_del(cdev);

	unregister_chrdev_region(MKDEV(major,minor),COUNT);

	kfree(cdev);
}

module_init(mycdev_init);
module_exit(mycdev_exit);
MODULE_LICENSE("GPL");


