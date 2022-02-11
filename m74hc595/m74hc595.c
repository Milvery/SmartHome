#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/fs.h>
//#include "m74hc595.h"
#include "m74hc595.h"

#include <linux/uaccess.h>
#include <linux/device.h>

/*
 &spi4 { 
    pinctrl-names = "default", "sleep"; 
    pinctrl-0 = <&spi4_pins_b>; 
    pinctrl-1 = <&spi4_sleep_pins_b>; 
    cs-gpios = <&gpioe 11 GPIO_ACTIVE_HIGH>;
    status = "okay";
    m74hc595@0{
    	compatible = "m74hc595";
    	reg = <0>;
        spi-max-frequency = <10000000>;
    };
};
*/
#define NAME "m74hc595"
int major = 0;
struct class *cls;
struct device *dev;
struct spi_device *gspi;
u8 code[] = {
	0x3f, //0
	0x06, //1
	0x5b, //2
	0x4f, //3
	0x66, //4
	0x6d, //5
	0x7d, //6
	0x07, //7
	0x7f, //8
	0x6f, //9
	0x77, //A
	0x7c, //b
	0x39, //c
	0x5e, //d
	0x79, //e
	0x71, //f
	0x76, //H
};

u8 which[] = {
	0x1, //sg0
	0x2, //sg1
	0x4, //sg2
	0x8, //sg3
};



int m74hc595_open(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

long m74hc595_ioctl(struct file *file, 
	unsigned int cmd, unsigned long args)
{	

	int ret;
	int	data;
	unsigned short data2 = 0;
	unsigned short data3 = 0;
	char hum_value[8]={0};
	char tem_value[8]={0};
//	printk("cmd=%#x",cmd);
//	printk("into m74hc595\n");

	switch(cmd){

		case READ_TEM_SPI://控制显示温度
			ret = copy_from_user((void*)&data,(void *)args,GET_CMD_SIZE(READ_TEM_SPI));
			if(ret){
				printk("copy tem_data from user error\n");
				return -EINVAL;
			}
				data2=data;
				tem_value[0] = which[0]; 
				tem_value[1] = code[data2 /100];			
				tem_value[2] = which[1]; 
				tem_value[3] = code[data2%100/10]|(0x1<<7);	
				tem_value[4] = which[2]; 
				tem_value[5] = code[data2%10];	
				tem_value[6] = which[3]; 
				tem_value[7] = code[12];	

				spi_write(gspi,tem_value,2);
				spi_write(gspi,&tem_value[2],2);
				spi_write(gspi,&tem_value[4],2);
				spi_write(gspi,&tem_value[6],2);

				break;
		case READ_HUM_SPI:  //读取湿度
			ret = copy_from_user((void*)&data,(void *)args,GET_CMD_SIZE(READ_HUM_SPI));
			if(ret){
				printk("copy hum_data from user error\n");
				return -EINVAL;
			}
				data3=data;
				hum_value[0] = which[0]; 
				hum_value[1] = code[data3 /100];			
				hum_value[2] = which[1]; 
				hum_value[3] = code[data3%100/10]|(0x1<<7);	
				hum_value[4] = which[2]; 
				hum_value[5] = code[data3%10];	
				hum_value[6] = which[3]; 
				hum_value[7] = code[16];	

				spi_write(gspi,hum_value,2);
				spi_write(gspi,&hum_value[2],2);
				spi_write(gspi,&hum_value[4],2);
				spi_write(gspi,&hum_value[6],2);			
				
			break;


		default:
				
				printk("ioctl error\n");break;
	}
	
	return 0;
}

int m74hc595_close(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}
struct file_operations fops = {
	.open = m74hc595_open,
	.unlocked_ioctl = m74hc595_ioctl,
	.release = m74hc595_close,
};

int	m74hc595_probe(struct spi_device *spi)
{
	u8 buf[2] = {0xf,0x0};
	printk("%s:%d\n",__func__,__LINE__);
	gspi = spi;
	spi_write(gspi,buf,ARRAY_SIZE(buf));
	major = register_chrdev(0,NAME,&fops);
	if(major < 0){
		printk("register chrdev error\n");
		return major;
	}

	cls = class_create(THIS_MODULE,NAME);
	if(IS_ERR(cls)){
		printk("class create  error\n");
		return PTR_ERR(cls);
	}
	dev = device_create(cls,NULL,MKDEV(major,0),NULL,NAME);
	if(IS_ERR(dev)){
		printk("device create  error\n");
		return PTR_ERR(dev);
	}
	return 0;
}

int	m74hc595_remove(struct spi_device *spi)
{
	printk("%s:%d\n",__func__,__LINE__);
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major,NAME);
	return 0;
}

const struct of_device_id of_match[] = {
	{.compatible = "hqyj,m74hc595",},
	{},
};
MODULE_DEVICE_TABLE(of,of_match);

const struct spi_device_id id_match[] = {
	{"id_m74hc595",0},
	{},
};
MODULE_DEVICE_TABLE(spi,id_match);

struct spi_driver m74hc595 = {
	.probe = m74hc595_probe,
	.remove = m74hc595_remove,
	.driver = {
		.name = "hello_m74hc595",
		.of_match_table = of_match,
	},	
	.id_table = id_match,
};
module_spi_driver(m74hc595);
MODULE_LICENSE("GPL");
