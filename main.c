#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "myleds2/myleds.h"
#include "motor/mymotor.h"
#include "si7006-ok/myioctl.h"
#include "m74hc595/m74hc595.h"
#include "mybuzzer2/mybuzzer.h"
#include "fan-pwm2/myfan.h"


typedef struct
{
	int fd_beef;
	int fd_fan;
}FileMsg;

//int flag=0;

void* callBack_Control(void* arg)
{
	//LED灯
	int fd_leds=open("/dev/myleds1",O_RDWR);
	if(fd_leds<0)
	{
		perror("open leds error");
		exit(EXIT_FAILURE);
	}
/*
	//BUZZER蜂鸣器
	int fd_beef;                                                 
    if((fd_beef = open("/dev/mybeef",O_RDWR)) < 0){ 
        perror("open error");
        exit(EXIT_FAILURE);
    }

	//fan风扇
    int fd_fan;
    if((fd_fan = open("/dev/myfan0",O_RDWR)) < 0){
        perror("open error");
        exit(EXIT_FAILURE);
    }                     

*/	

//蜂鸣器和风扇
	FileMsg info = *(FileMsg*)arg;
	int fd_beef = info.fd_beef;
	int fd_fan = info.fd_fan;

	//motor马达
	int fd_motor; 
	if((fd_motor = open("/dev/mymotor1",O_RDWR)) < 0){ 
		perror("open error");
		exit(EXIT_FAILURE);                                            
	}  
	char buf_motor[28]={0};


	while(1)
	{
		system("clear");
		printf("欢迎进入智能家居系统\n");
		printf("******1.LEDS_ON******\n");
		printf("******2.LEDS_OFF*****\n");
		printf("******3.BUZZER_ON****\n");
		printf("******4.BUZZER_OFF***\n");
		printf("******5.MOTOR_ON*****\n");
		printf("******6.MOTOR_OFF****\n");
		printf("******7.FAN_ON*******\n");
		printf("******8.FAN_OFF******\n");

		int choose;
		printf("请输入您的命令:\n");
		scanf("%d",&choose);
		while(getchar()!=10);
		switch(choose)
		{
			case 1:
				//LEDS_ON
				ioctl(fd_leds,LEDS_ON,NULL);
				break;
			case 2:
				//LEDS_OFF
				ioctl(fd_leds,LEDS_OFF,NULL);
				break;
			case 3:
				//BUZZER_ON
			//	flag=1;	
				ioctl(fd_beef,beef_on,NULL);
				
				break;
			case 4:
				//BUZZER_OFF
		//		flag=1;
				
				ioctl(fd_beef,beef_off,NULL); 
				
				break;
			case 5:
				//MOTOR_ON
				buf_motor[0]=1;
				write(fd_motor,buf_motor,sizeof(buf_motor)); 
				break;
			case 6:
				//MOTOR_OFF
				buf_motor[0]=0;
				write(fd_motor,buf_motor,sizeof(buf_motor)); 
				break;
			case 7:
				//FAN_ON
				ioctl(fd_fan,fan_on,NULL);
				break;
			case 8:
				//fan_off
		//		flag=1;	
				ioctl(fd_fan,fan_off,NULL);   
				break;
		}
		printf("输入任意字符清屏\n");
		while(getchar()!=10);
	}


	close(fd_leds);
	close(fd_beef);
	close(fd_motor);
	close(fd_fan);
	pthread_exit(NULL);

}

void* callBack_Warning(void* arg)
{
	//温度、湿度

	int hum,tem;
    float rhum,rtem;                                              
    int d=2500;
    int i=2500;

	int fd_m74hc595;
    int fd_si7006;
    int rtem_spi;
    int rhum_spi;
                                                                  
    if((fd_si7006 = open("/dev/si7006",O_RDWR)) < 0){ 
        perror("open error");
        exit(EXIT_FAILURE);
    }   

    if((fd_m74hc595 = open("/dev/m74hc595",O_RDWR)) < 0){ 
        perror("open error");
        exit(EXIT_FAILURE);
    }   


	//蜂鸣器和风扇
	FileMsg info = *(FileMsg*)arg;
	int fd_beef = info.fd_beef;
	int fd_fan = info.fd_fan;

    while(1){
        while(1)
        {
            if(!i)
            {
                i=2500;
                ioctl(fd_si7006,READ_TEM,&tem);    
                rtem = 175.72*tem/65536.0-46.85;
				if(rtem>14)
				{
					//FAN_ON
					ioctl(fd_fan,fan_on,NULL);
					//BUZZER_ON
					ioctl(fd_beef,beef_on,NULL);
				}
		//		else
		//		{
		//			flag=1;
					//FAN_OFF
		//			ioctl(fd_fan,fan_off,NULL);
					//BUZZER_OFF
		//			ioctl(fd_beef,beef_off,NULL);
		//		}

            //  printf("hum = %.2f%%,temp = %.2fC\n",rhum,rtem);
                rtem_spi=(int)(rtem*10);
            }
            i--;

            ioctl(fd_m74hc595,READ_TEM_SPI,&rtem_spi);
            if(i==0)
                break;
        }

        while(1)
        {
            if(!d)
            {
                d=2500;
                ioctl(fd_si7006,READ_HUM,&hum);    
                rhum = 125*hum/65536.0-6;

              //printf("hum = %.2f%%,temp = %.2fC\n",rhum,rtem);
                rhum_spi=(int)(rhum*10);
            }
            d--;
            ioctl(fd_m74hc595,READ_HUM_SPI,&rhum_spi);  
            if(d==0)
            {
                break;
            }
        }

    }   

	close(fd_m74hc595);
	close(fd_si7006);
	close(fd_beef);
	close(fd_fan);
	pthread_exit(NULL);
}




int main(int argc, const char *argv[])
{
	//BUZZER蜂鸣器
	int fd_beef;                                                 
    if((fd_beef = open("/dev/mybeef",O_RDWR)) < 0){ 
        perror("open error");
        exit(EXIT_FAILURE);
    }

	//fan风扇
    int fd_fan;
    if((fd_fan = open("/dev/myfan0",O_RDWR)) < 0){
        perror("open error");
        exit(EXIT_FAILURE);
    }                     

	FileMsg info;
	info.fd_beef = fd_beef;
	info.fd_fan = fd_fan;

	pthread_t tid1,tid2;
	if(pthread_create(&tid1,NULL,callBack_Control,(void*)&info)!=0)
	{
		perror("pthread_create");
		return -1;
	}
	if(pthread_create(&tid2,NULL,callBack_Warning,(void*)&info)!=0)
	{
		perror("pthread_create");
		return -1;
	}


//	close(fd_beef);
//	close(fd_fan);
	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);

	return 0;
}



































