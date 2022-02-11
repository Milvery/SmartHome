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
int curminor;
struct device_node *pnode,*cnode,*enode;
char *ledpname[] = {"led1","led2","led3"};
unsigned int cgpiono[3] = {0};
unsigned int egpiono[3] = {0};

struct timer_list mytimer;


int myleds_open(struct inode *inode, struct file *filp)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	curminor = MINOR(inode->i_rdev);

	return 0;
}


ssize_t myleds_read(struct file *filp,
	char __user *ubuf, size_t size, loff_t *offs)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

long myleds_ioctl(struct file *file,
		unsigned int cmd,unsigned long arg)
{
	int i,ret;

	switch (cmd){
		case LEDS_ON:
            ret = copy_from_user((void *)&data,
                (void *)args,GET_CMD_SIZE(LED1_OP));                            
            if(ret){
                printk("copy data from user error\n");
                return -EINVAL;
            }
            data==1?gpio_set_value(gpiono,1):gpio_set_value(gpiono,0);

			break;
		case LEDS_OFF:
			
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],0);
		gpio_set_value(egpiono[i],0);

	}
			

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
	
/*	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(cgpiono[i],0);
		gpio_set_value(egpiono[i],0);

		gpio_free(cgpiono[i]);
		gpio_free(egpiono[i]);
	}*/



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
	//1.�������
	cdev = cdev_alloc();
	if(cdev == NULL){
		printk("cdev alloc error\n");
		ret = -ENOMEM;
		goto ERR1;
	}

	//2.�����ʼ��
	cdev_init(cdev,&fops);

			//3.�豸�ŵ�����
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

			//4.ע��
		ret = cdev_add(cdev,MKDEV(major,minor),COUNT);
		if(ret){
			printk("cdev register error\n");
			goto ERR3;
		}

			//5.�Զ������豸�ڵ�
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

}





static int __init  myleds_init(void)
{

	//3.�����豸��

	//3.1.��ȡ���ڵ�
	pnode = of_find_compatible_node(NULL,NULL,"hqyj,leds");
	if(pnode ==NULL){
		printk("get parent node error\n");
		return -ENODATA;
	}
	//3.2.��ȡ�ӽڵ�
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

		//3.3.������gpio��
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
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
		//4.ʹ��gpio��ϵͳ�ĺ����ӿڲ���LED
		if((ret = gpio_request(cgpiono[i],NULL))<0){
			printk("gpio request error\n");
			return ret;
		}
		if((ret = gpio_request(egpiono[i],NULL))<0){
			printk("gpio request error\n");
			return ret;
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





