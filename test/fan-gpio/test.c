#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

void delay_ms(int ms)
{
	int i,j;
	for(i = 0; i < ms;i++)
		for (j = 0; j < 1800; j++);
}

char buf[128] = {0};
int main(int argc, const char *argv[])
{
	int fd;
	if((fd = open("/dev/myfan0",O_RDWR)) < 0){
		perror("open error");
		exit(EXIT_FAILURE);
	}
	while(1){
	//	buf[0] = !buf[0];
		buf[0] = 1;
		write(fd,buf,sizeof(buf));
		delay_ms(20000);

		sleep(1);
	}

	close(fd);
	return 0;
}

