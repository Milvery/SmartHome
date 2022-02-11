#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "myfan.h"

int main(int argc, const char *argv[])
{
	int fd_fan;
	if((fd_fan = open("/dev/myfan0",O_RDWR)) < 0){
		perror("open error");
		exit(EXIT_FAILURE);
	}
	int cmd;
	printf("choose\n");
	scanf("%d",&cmd);
	while(getchar()!=10);
	switch(cmd)
	{
	//	int cmd;
		case 1:
			ioctl(fd_fan,fan_on,NULL);
			break;
		case 2:
			ioctl(fd_fan,fan_off,NULL);
			break;
	}
	close(fd_fan);
	return 0;
}

