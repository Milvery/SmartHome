#ifndef __MYIOCTL_H__
#define __MYIOCTL_H__

#define READ_HUM _IOR('a',0,int)
#define READ_TEM _IOR('a',1,int)
#define GET_CMD_SIZE(cmd) ((cmd>>16)&0x3fff)
#endif


