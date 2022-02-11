#ifndef __M74HC595_H__
#define __M74HC595_H__

#define READ_TEM_SPI _IOWR('f',0,int)
#define READ_HUM_SPI  _IOWR('f',1,int)
//#define READ_HUM _IOR('a',0,int)
//#define READ_TEM _IOR('a',1,int)
//#define GET_CMD_SIZE(cmd) ((cmd>>16)&0x3fff) 
#define GET_CMD_SIZE(cmd) ((cmd>>16)&0x3fff)
#endif
