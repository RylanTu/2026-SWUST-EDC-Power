#include "main.h"
#include "AT24CXX.h"


#define AT24CXX_TIMEOUT 200
#define AT24CXX_MEMADD_SIZE I2C_MEMADD_SIZE_8BIT


static void I2C_Soft_Init(void);
static void I2C_Soft_Start(void);
static void I2C_Soft_Stop(void);
static uint8_t I2C_Soft_WaitAck(void);
static void I2C_Soft_Ack(void);
static void I2C_Soft_NAck(void);
static void I2C_Soft_SendByte(uint8_t byte);
static uint8_t I2C_Soft_ReadByte(uint8_t ack);


static uint8_t AT24CXX_IsDeviceReady(void);  // 检测设备是否就绪
static uint8_t AT24CXX_WriteOneByte(uint16_t memAddress,uint8_t byteData) ;
static uint8_t AT24CXX_ReadOneByte(uint16_t memAddress,uint8_t *byteData) ;
static uint8_t AT24CXX_ReadBytes(uint16_t memAddress,uint8_t *pBuffer,uint16_t BufferLen);
static uint8_t AT24CXX_WriteInOnePage(uint16_t memAddress,uint8_t *pBuffer,uint16_t BufferLen);
static void  Write_SET_VAL(uint16_t Set_cv,uint16_t Set_cc);  // 写入设定值到EEPROM
static void  Read_SET_VAL(void);  // 从EEPROM读取设定值


static void I2C_Soft_Delay(void)
{
    uint8_t i = 15; // 延时循环计数
    while(i--);
}

// 初始化软件I2C的GPIO引脚
static void I2C_Soft_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIOB时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // 配置SCL和SDA引脚为开漏输出模式
    GPIO_InitStruct.Pin = IIC_SCL_Pin | IC_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(IIC_SCL_GPIO_Port, &GPIO_InitStruct);
    
    // 设置引脚为高电平（空闲状态）
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_SET);
}

// 产生I2C起始信号
static void I2C_Soft_Start(void)
{
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_RESET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
    I2C_Soft_Delay();
}

// 产生I2C停止信号
static void I2C_Soft_Stop(void)
{
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_SET);
    I2C_Soft_Delay();
}

// 等待从机应答信号
static uint8_t I2C_Soft_WaitAck(void)
{
    uint8_t retry = 0;
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_SET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET);
    I2C_Soft_Delay();
    
    while(HAL_GPIO_ReadPin(IC_SDA_GPIO_Port, IC_SDA_Pin))
    {
        retry++;
        if(retry > 250)
        {
            I2C_Soft_Stop();
            return 1;
        }
    }
    
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
    I2C_Soft_Delay();
    return 0;
}

// 主机发送应答信号（ACK）
static void I2C_Soft_Ack(void)
{
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_RESET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
    I2C_Soft_Delay();
}

// 主机发送非应答信号（NACK）
static void I2C_Soft_NAck(void)
{
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_SET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET);
    I2C_Soft_Delay();
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
    I2C_Soft_Delay();
}

// 发送一个字节数据
static void I2C_Soft_SendByte(uint8_t byte)
{
    uint8_t i = 8;
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
    I2C_Soft_Delay();
    
    while(i--)
    {
        if(byte & 0x80)
            HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_RESET);
        
        byte <<= 1;
        I2C_Soft_Delay();
        HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET);
        I2C_Soft_Delay();
        HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
        I2C_Soft_Delay();
    }
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_SET);
}

// 接收一个字节数据
static uint8_t I2C_Soft_ReadByte(uint8_t ack)
{
    uint8_t i = 8;
    uint8_t byte = 0;
    
    HAL_GPIO_WritePin(IC_SDA_GPIO_Port, IC_SDA_Pin, GPIO_PIN_SET);
    I2C_Soft_Delay();
    
    while(i--)
    {
        byte <<= 1;
        HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
        I2C_Soft_Delay();
        HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET);
        I2C_Soft_Delay();
        
        if(HAL_GPIO_ReadPin(IC_SDA_GPIO_Port, IC_SDA_Pin))
            byte |= 0x01;
    }
    
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET);
    
    if(ack)
        I2C_Soft_Ack();
    else
        I2C_Soft_NAck();
        
    return byte;
}

AT24CXX_t AT24CXX =
{  
    AT24CXX_IsDeviceReady,
    AT24CXX_WriteOneByte,
    AT24CXX_ReadOneByte,
    AT24CXX_ReadBytes,
    AT24CXX_WriteInOnePage,
    Write_SET_VAL,         // 写入设定值到EEPROM
    Read_SET_VAL          // 从EEPROM读取设定值
};

static uint8_t AT24CXX_IsDeviceReady(void)
{
    I2C_Soft_Init();
    
    I2C_Soft_Start();
    I2C_Soft_SendByte(AT24CXX_ADDRESS);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    I2C_Soft_Stop();
    return HAL_OK;
}

static uint8_t AT24CXX_WriteOneByte(uint16_t memAddress,uint8_t byteData)
{
    I2C_Soft_Start();
    I2C_Soft_SendByte(AT24CXX_ADDRESS);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    I2C_Soft_SendByte(memAddress);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    I2C_Soft_SendByte(byteData);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    I2C_Soft_Stop();
    HAL_Delay(10); // 等待EEPROM写入完成
    return HAL_OK;
}

static uint8_t AT24CXX_ReadOneByte(uint16_t memAddress,uint8_t *byteData)
{
    // 发送写地址，指定读取位置
    I2C_Soft_Start();
    I2C_Soft_SendByte(AT24CXX_ADDRESS);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    I2C_Soft_SendByte(memAddress);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    // 重新起始，切换为读操作
    I2C_Soft_Start();
    I2C_Soft_SendByte(AT24CXX_ADDRESS | 0x01); // 读操作地址
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    *byteData = I2C_Soft_ReadByte(0); // 读取一个字节，发送NACK表示结束
    I2C_Soft_Stop();
    
    return HAL_OK;
}

static uint8_t AT24CXX_ReadBytes(uint16_t memAddress,uint8_t *pBuffer,uint16_t BufferLen)
{
    if(BufferLen > MEM_SIZE_24CXX)
    {
        return HAL_ERROR;
    }
    
    // 发送写地址，指定读取起始位置
    I2C_Soft_Start();
    I2C_Soft_SendByte(AT24CXX_ADDRESS);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    I2C_Soft_SendByte(memAddress);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    // 重新起始，切换为读操作
    I2C_Soft_Start();
    I2C_Soft_SendByte(AT24CXX_ADDRESS | 0x01); // 读操作地址
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    // 循环读取多个字节
    while(BufferLen--)
    {
        if(BufferLen == 0)
            *pBuffer = I2C_Soft_ReadByte(0); // 最后一字节，发送NACK
        else
            *pBuffer = I2C_Soft_ReadByte(1); // 非最后字节，发送ACK
        pBuffer++;
    }
    
    I2C_Soft_Stop();
    return HAL_OK;
}

static uint8_t AT24CXX_WriteInOnePage(uint16_t memAddress,uint8_t *pBuffer,uint16_t BufferLen)
{
    if(BufferLen > PAGE_SIZE_24CXX)
    {
        return HAL_ERROR;
    }
    
    I2C_Soft_Start();
    I2C_Soft_SendByte(AT24CXX_ADDRESS);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    I2C_Soft_SendByte(memAddress);
    if(I2C_Soft_WaitAck())
        return HAL_ERROR;
    
    while(BufferLen--)
    {
        I2C_Soft_SendByte(*pBuffer++);
        if(I2C_Soft_WaitAck())
            return HAL_ERROR;
    }
    
    I2C_Soft_Stop();
    HAL_Delay(10); // 等待EEPROM页写入完成
    return HAL_OK;
}

static void Write_SET_VAL(uint16_t Set_cv,uint16_t Set_cc)
{
    uint8_t Set_cv_Buffer[2]={0};
    uint8_t Set_cc_Buffer[2]={0};
    Set_cv_Buffer[0] = (Set_cv & 0xFF00) >> 8;
    Set_cv_Buffer[1] = (Set_cv & 0x00FF);
    if(AT24CXX.AT24CXX_WriteInOnePage(0x03,Set_cv_Buffer,2)==HAL_OK)
    {
        printf("WriteBytes Set_cv_Buffer0:0x%x!!!\r\n\r\n",Set_cv_Buffer[0]);
        printf("WriteBytes Set_cv_Buffer1:0x%x!!!\r\n\r\n",Set_cv_Buffer[1]);        
    }
    Set_cc_Buffer[0] = (Set_cc & 0xFF00) >> 8;
    Set_cc_Buffer[1] = (Set_cc & 0x00FF);
    if(AT24CXX.AT24CXX_WriteInOnePage(0x05,Set_cc_Buffer,2)==HAL_OK)
    {
        printf("WriteBytes Set_cc_Buffer0:0x%x!!!\r\n\r\n",Set_cc_Buffer[0]);
        printf("WriteBytes Set_cc_Buffer1:0x%x!!!\r\n\r\n",Set_cc_Buffer[1]);        
    }
}

static void Read_SET_VAL(void)
{
    uint8_t Set_cv_Buffer[2]={0};
    uint8_t Set_cc_Buffer[2]={0};
    uint16_t Read_Set_cv,Read_Set_cc;
    if(AT24CXX.AT24CXX_ReadBytes(0x03,Set_cv_Buffer,2)==HAL_OK)
    {
        printf("ReadBytes Set_cv_Buffer0:0x%x!!!\r\n\r\n",Set_cv_Buffer[0]); 
        printf("ReadBytes Set_cv_Buffer1:0x%x!!!\r\n\r\n",Set_cv_Buffer[1]);    
    }
    Read_Set_cv=((Set_cv_Buffer[0]<<8)&0xFF00)|Set_cv_Buffer[1];
    if(Read_Set_cv < 1)
    {
        Read_Set_cv = 500;
    }
    else if(Read_Set_cv > 2700)
    {
        Read_Set_cv = 2700;
    }
    printf("ReadBytes Set_cv_Buffer3:0x%x!!!\r\n\r\n",Read_Set_cv);
    Function_SET.Set_VOUT=Read_Set_cv;          
    if(AT24CXX.AT24CXX_ReadBytes(0x05,Set_cc_Buffer,2)==HAL_OK)
    {
        printf("ReadBytes Set_cc_Buffer0:0x%x!!!\r\n\r\n",Set_cc_Buffer[0]); 
        printf("ReadBytes Set_cc_Buffer1:0x%x!!!\r\n\r\n",Set_cc_Buffer[1]);    
    }
    Read_Set_cc=((Set_cc_Buffer[0]<<8)&0xFF00)|Set_cc_Buffer[1];
    if(Read_Set_cc < 1)
    {
        Read_Set_cc = 3000;
    }
    else if(Read_Set_cc > 7500)
    {
        Read_Set_cc = 7500;
    }
    printf("ReadBytes Set_cc_Buffer3:0x%x!!!\r\n\r\n",Read_Set_cc);
    Function_SET.Set_IOUT=Read_Set_cc;
}