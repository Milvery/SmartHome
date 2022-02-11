#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "myioctl.h"
#include "../m74hc595/m74hc595.h"
char buf[128] = {0};

void delay_ms(int ms)
{
	int i,j;
	for(i = 0; i < ms;i++)
		for (j = 0; j < 1800; j++);
}


int main(int argc, const char *argv[])
{
	int hum,tem;
	float rhum,rtem;
	int d=2500;
	int i=2500;
	int fd2;
	int fd;
	int rtem_spi;
	int rhum_spi;

	if((fd = open("/dev/si7006",O_RDWR)) < 0){
		perror("open error");
		exit(EXIT_FAILURE);
	}

	if((fd2 = open("/dev/m74hc595",O_RDWR)) < 0){
		perror("open error");
		exit(EXIT_FAILURE);
	}

	while(1){
		while(1)
		{
			if(!i)
			{
				i=2500;
//				ioctl(fd,READ_HUM,&hum);	
				ioctl(fd,READ_TEM,&tem);	
//				rhum = 125*hum/65536.0-6;
				rtem = 175.72*tem/65536.0-46.85;

				printf("hum = %.2f%%,temp = %.2fC\n",rhum,rtem);
				rtem_spi=(int)(rtem*10);
//				rhum_spi=(int)(rhum*10);
			}
			i--;

			ioctl(fd2,READ_TEM_SPI,&rtem_spi);
			if(i==0)
				break;
		}

		while(1)
		{
			if(!d)
			{
				d=2500;
				ioctl(fd,READ_HUM,&hum);	
//				ioctl(fd,READ_TEM,&tem);	
				rhum = 125*hum/65536.0-6;
//				rtem = 175.72*tem/65536.0-46.85;

				printf("hum = %.2f%%,temp = %.2fC\n",rhum,rtem);
//				rtem_spi=(int)(rtem*10);
				rhum_spi=(int)(rhum*10);
			}
			d--;
			ioctl(fd2,READ_HUM_SPI,&rhum_spi);	
			if(d==0)
			{
				break;
			}
		}

	}

	close(fd);
	close(fd2);
	return 0;
}
