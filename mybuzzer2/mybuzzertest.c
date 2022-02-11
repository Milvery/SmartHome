#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "mybuzzer.h"

int main(int argc, const char *argv[])
{
	int fd_beef;
	if((fd_beef = open("/dev/mybeef",O_RDWR)) < 0){
		perror("open error");
		exit(EXIT_FAILURE);
	}

	printf("choose\n");
	int choose=0;
	scanf("%d",&choose);
	switch(choose)
	{
		case 1:
			ioctl(fd_beef,beef_on,NULL);
			break;
		case 2:
			ioctl(fd_beef,beef_off,NULL);
			break;

	}
	close(fd_beef);
	return 0;
}

