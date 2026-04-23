#ifndef __AT24CXX_H
#define __AT24CXX_H		

#include "MyApplication.h"


#define AT24CXX_ADDRESS 0xA0  //地址

#define I2C_HANDLE hi2c1
#define PAGE_SIZE_24CXX  0x08
#define MEM_SIZE_24CXX (uint16_t)256



//定义结构体类型
typedef struct
{
    uint8_t (*AT24CXX_IsDeviceReady)(void);  //设备就绪
    uint8_t (*AT24CXX_WriteOneByte)(uint16_t ,uint8_t) ;  //在任意地址写一个字节
    uint8_t (*AT24CXX_ReadOneByte)(uint16_t ,uint8_t *) ;
    uint8_t (*AT24CXX_ReadBytes)(uint16_t ,uint8_t* ,uint16_t );
    uint8_t (*AT24CXX_WriteInOnePage)(uint16_t ,uint8_t *,uint16_t );
    void  (*Write_SET_VAL)(uint16_t,uint16_t);  //写设置电压 电流
    void  (*Read_SET_VAL)(void);  //读设置电压 电流(2026.4.16 RylanTu:为什么这个之前也没发现)


}AT24CXX_t;

/* extern variables-----------------------------------------------------------*/
extern AT24CXX_t  AT24CXX;

#endif