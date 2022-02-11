#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>


struct device_node *pnode,*cnode;

unsigned int ret2;

static int __init  mybuzzer_init(void)
{
	int ret;
	//1.获取父节点
	pnode = of_find_compatible_node(NULL,NULL,"hqyj,mypwm1");
	if(pnode ==NULL){
		printk("get parent node error\n");
		return -ENODATA;
	}

	//2.获取子节点
	cnode = of_get_child_by_name(pnode,"core-mypwm1");
	if(cnode ==NULL){
		printk("get core leds node error\n");
		return -ENODATA;
	}
/*	
	//3.解析到gpio号
	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		cgpiono[i] = of_get_named_gpio(cnode,buzzerpname[i],0);
		if(cgpiono[i] < 0){
			printk("of get named gpio error\n");
			return cgpiono[i];
		}
*/

	//3.解析到gpio号
		ret2 = of_get_named_gpio(cnode,"buzzer1",0);
		if(ret2 < 0){
			printk("of get named gpio error\n");
			return ret2;
		}


		//4.使用gpio子系统的函数接口操作LED
		if((ret = gpio_request(ret2,NULL))<0){
			printk("gpio request error\n");
			return ret;
		}

//		if((ret = gpio_request(egpiono[i],NULL))<0){
//			printk("gpio request error\n");
//			return ret;
//		}
		gpio_direction_output(ret2,1);

//		gpio_direction_output(egpiono[i],1);
		
//	}
	
	return 0;
}

static void __exit mybuzzer_exit(void)
{
//	int i;
//	for(i=0;i<ARRAY_SIZE(cgpiono);i++){
		gpio_set_value(ret2,0);

		gpio_free(ret2);
//	}
}
module_init(mybuzzer_init);
module_exit(mybuzzer_exit);
MODULE_LICENSE("GPL");





