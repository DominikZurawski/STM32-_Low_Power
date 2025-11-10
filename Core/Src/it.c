#include "main.h"

extern TIM_HandleTypeDef htimer6;
extern UART_HandleTypeDef huart2;

void SysTick_Handler (void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htimer6);
}

void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}