#include "MyApplication.h"

/* Private function prototypes -----------------------------------------------*/
//使用Printf一定要加\n

#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)	
#endif /* __GNUC__ */

  // 实现__io_putchar，将字符通过USART1发送
int __io_putchar(int ch)
{
    // 等待串口发送缓冲区为空（HAL库函数：等待发送完成）
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;  // 返回字符，满足函数返回值要求
}
//PUTCHAR_PROTOTYPE
//{
//	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
// 
//	return ch;
//}
__attribute__((weak)) int _write(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		__io_putchar(*ptr++);
	}
	return len;
}
