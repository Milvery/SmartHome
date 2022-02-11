#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "myfan.h"

#define CNAME "myfan"
#define COUNT 3
int major=0;
unsigned int minor=0;

struct cdev *cdev;
struct class *cls;
struct device *dev;
gpio_t_2 *gpioe = NULL;
tim1_t *TIM1 = NULL;
//unsigned int * rcc;

rcc_t *rcc=NULL;

int myfan_open(struct inode *inode, struct file *filp)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	//ӳ���ַgpio
	gpioe = ioremap(GPIOE_PHY_BASE,sizeof(gpio_t_2));
	if(gpioe == NULL){
		printk("ioremap gpioe base error\n");
		return -ENOMEM;
	}

	//ӳ���ַrcc
	rcc = ioremap(RCC_ENB_2,sizeof(rcc_t));
	if(rcc == NULL){
		printk("ioremap rcc error\n");
		return -ENOMEM;
	}

	
	//ӳ���ַ��TIMER
	TIM1 = ioremap(TIM1_BASE,sizeof(tim1_t));
	if(TIM1 == NULL){
		printk("ioremap timer base error\n");
		return -ENOMEM;
	}

	//��ʼ��fan
	// 1. ����GPIOE9��TIM1��ʱ��ʹ��   RCC_MP_AHB4ENSETR  RCC_MP_APB2ENSETR
	rcc->MP_AHB4ENSETR |= (0x1 << 4);
	rcc->MP_APB2ENSETR |= (0x1 << 0);
	// 2. ����PE9����Ϊ���ù���       GPIOE_MODER
	gpioe->MODER &= (~(0x3 << 18));
	gpioe->MODER |= (0x2 << 18);
	// 3. ����PE9����ΪTIM1_CH1����   GPIOE_AFRH
	gpioe->AFRH &= (~(0xF << 4));
	gpioe->AFRH |= (0x1 << 4);
	// 4. ����Ԥ��Ƶ�Ĵ�����TIM1_PSC[15:0]  CK_PSC = 209MHz  
	// 		�ṩ��TIM1��ʱ��Դ��Ƶ����209MHz
	TIM1->PSC = 209-1;
	// 5. ����PWM���������յ�����  TIM1_ARR[16:0]
	// 		�õ�һ��1000-2000Hz�ķ���
	TIM1->ARR = 1000;
	// 5. ����PWM������ռ�ձ�   TIM1_CCR1[16:0]
	TIM1->CCR1 = 1000;
	// 6. ����TIM1_CH1ͨ��ΪPWM1ģʽ  
	// 		TIM1_CCMR1[16]  TIM1_CCMR1[6:4]  pwmģʽ1  = 0b0110
	TIM1->CCMR1 &= (~(0x1 << 16));
	TIM1->CCMR1 &= (~(0x7 << 4));
	TIM1->CCMR1 |= (0x6 << 4);
	// 7. ����TIM1_CH1ͨ��ʹ��TIMx_CCR1Ԥ����ʹ�ܼĴ���
	// 		TIM1_CCMR1[3] = 0x1
	TIM1->CCMR1 |= (0x1 << 3);
	// 8. ����TIM1_CH1ͨ�����PWM�����ļ��ԣ�
	// 	  TIM1_CCER[3] = 0b0	TIM1_CCER[1] = 0x1 or 0x0
	TIM1->CCER &= (~(0x1 << 3));
	TIM1->CCER |= (0x1 << 1);
	// 9. ����TIM1 _CH1ͨ�������ʹ��λ�����PWM���� 
	// 		TIM1 _CCER[0] = 0x1
	TIM1 ->CCER &= ~(0x1 << 0);
	//	TIM1->CCER |= (0x1 << 2);
	// 10. ����TIM1 _CH1ͨ����Ԥװ�ؼĴ����Ļ�������ʹ��
	// 		TIM1 _CR1[7] = 0x1
	TIM1 ->CR1 |= (0x1 << 7);
	// 11. ���ö�ʱ���ļ�����ʽ�����ض���
	TIM1 ->CR1 &= (~(0x3 << 5));
	// 12 ���ö�ʱ�������ķ��򣬲��õݼ�����/�������� 
	TIM1 ->CR1 |= (0x1 << 4);

	TIM1->BDTR |= (0x1 << 15);

	// 13. ʹ��TIM1 _CH1������ 
	// 		TIM1 _CR1[0] = 0x1
	TIM1 ->CR1 &= ~(0x1 << 0);

	return 0;
}

long myfan_ioctl(struct file *filp,
	unsigned int cmd, unsigned long args)
{
//	int ret;
//	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	switch(cmd)
	{
		case fan_on:
		// 5. ����PWM������ռ�ձ�   TIM1_CCR1[16:0]
		TIM1->CCR1 = 5;
		// 9. ����TIM1 _CH1ͨ�������ʹ��λ�����PWM���� 
		// 		TIM1 _CCER[0] = 0x1
		TIM1 ->CCER |= (0x1 << 0);
		//	TIM1->CCER |= (0x1 << 2);

		// 13. ʹ��TIM1 _CH1������ 
		// 		TIM1 _CR1[0] = 0x1
		TIM1 ->CR1 |= (0x1 << 0);
			break;

		case fan_off:
		TIM1->CCR1 = 1000;
		TIM1 ->CCER &= ~(0x1 << 0);
		TIM1 ->CR1 &= ~(0x1 << 0);
			break;
	}

	return 0;	
}


int myfan_close(struct inode *inode, struct file *filp)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

/*	TIM1->CCR1 = 1000;
	TIM1 ->CCER &= ~(0x1 << 0);
	TIM1 ->CR1 &= ~(0x1 << 0);

	iounmap(gpioe);
	iounmap(rcc);
	iounmap(TIM1);
*/
	return 0;
}

const struct file_operations fops = {
	.open = myfan_open,
	.unlocked_ioctl = myfan_ioctl,
	.release = myfan_close,
};

static int __init myfan_init(void)
{
	int ret,i;
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

		//4.ע��
		ret = cdev_add(cdev,MKDEV(major,minor),COUNT);
		if(ret){
			printk("cdev register error\n");
			goto ERR3;
		}

	//5.�Զ������豸�ڵ�
	cls = class_create(THIS_MODULE,"myfan");
	if(IS_ERR(cls)){
		printk("class create error\n");
		ret = PTR_ERR(cls);
		goto ERR4;
	}
	for(i=0;i<COUNT;i++){
		dev = device_create(cls,NULL,MKDEV(major,minor+i),
			NULL,"myfan%d",i);
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


static void __exit myfan_exit(void)
{
	int i;
	TIM1->CCR1 = 1000;
	TIM1 ->CCER &= ~(0x1 << 0);
	TIM1 ->CR1 &= ~(0x1 << 0);

	iounmap(gpioe);
	iounmap(rcc);
	iounmap(TIM1);

	for(i=0;i<COUNT;i++){
		device_destroy(cls,MKDEV(major,minor+i));
	}
	class_destroy(cls);

	cdev_del(cdev);

	unregister_chrdev_region(MKDEV(major,minor),COUNT);

	kfree(cdev);
}

module_init(myfan_init);
module_exit(myfan_exit);
MODULE_LICENSE("GPL");


