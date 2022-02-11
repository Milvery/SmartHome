#ifndef __MYFAN_H__
#define __MYFAN_H__

#define GPIOE_PHY_BASE 0x50006000 //(moder odr)
#define GPIOF_PHY_BASE 0x50007000 //(moder odr)
#define GPIOZ_PHY_BASE 0x54004000 //(moder odr)
#define RCC_ENB      0x50000a28 //rcc_enb
typedef struct { 
   volatile unsigned int MODER;   // 0x00 
   volatile unsigned int OTYPER;  // 0x04 
   volatile unsigned int OSPEEDR; // 0x08 
   volatile unsigned int PUPDR;   // 0x0C 
   volatile unsigned int IDR;	  // 0x10 
   volatile unsigned int ODR;	  // 0x14 
   volatile unsigned int BSRR;	  // 0x18 
   volatile unsigned int LCKR;	  // 0x1C  
   volatile unsigned int AFRL;	  // 0x20  
   volatile unsigned int AFRH;	  // 0x24 
   volatile unsigned int BRR;	  // 0x28 
   volatile unsigned int res; 
   volatile unsigned int SECCFGR; // 0x30 
}gpio_t;


#endif
