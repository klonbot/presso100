// Главный обработчик логики
// MyLowLevelPutchar.c
// Рыбаков Д.Н.
//------------------------------------------------------------------------------

/* Library includes. */
#include "stm32f10x_lib.h"

int MyLowLevelPutchar(int x)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, x);
    return x;
}


