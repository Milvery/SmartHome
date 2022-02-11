#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "myleds.h"

int main(int argc, const char *argv[])
{
	int fd_LEDS;
	int hum=0;
	unsigned int number;
	if((fd_LEDS = open("/dev/myleds1",O_RDWR)) < 0){
		perror("open error");
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		ioctl(fd_LEDS,LEDS_OFF,&hum);
	}
	close(fd_LEDS);
	return 0;
}

