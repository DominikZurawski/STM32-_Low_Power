#include <string.h>
#include "stm32l4xx_hal.h"
#include "main.h"

void GPIO_Init(void);
void Error_handler(void);
void TIMER6_Init(void);
void UART2_Init(void);
void SystemClock_Config(uint8_t clock_freq);
void GPIO_AnalogConfig(void); //<- turn off analog pins when sleep mode is on

TIM_HandleTypeDef htimer6;
UART_HandleTypeDef huart2;
extern uint8_t some_data[];

int main(void)
{
	HAL_Init();
	//SystemClock_Config(SYS_CLOCK_FREQ_50_MHZ);
	GPIO_Init();
	//HAL_SuspendTick();
	UART2_Init();
	TIMER6_Init();
	GPIO_AnalogConfig();

	HAL_PWR_EnableSleepOnExit();  // <- Here we are turn on sleep mode with SLEEPONEXIT register
	//SCB->SCR |= ( 1 << 1);      // or

	/* lets start with fresh Status register of Timer to avoid any spurious interrupts */
    TIM6->SR = 0;

	//Lets start the timer in interrupt mode
	HAL_TIM_Base_Start_IT(&htimer6);

	while(1);

	return 0;
}

void SystemClock_Config(uint8_t clock_freq )
{
	RCC_OscInitTypeDef osc_init;
	RCC_ClkInitTypeDef clk_init;

	uint32_t FLatency =0;

	osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	osc_init.HSIState = RCC_HSI_ON;
	osc_init.HSICalibrationValue = 16;
	osc_init.PLL.PLLState = RCC_PLL_ON;
	osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSI;

	switch(clock_freq) {
        case SYS_CLOCK_FREQ_50_MHZ:{
            osc_init.PLL.PLLM = 4;
            osc_init.PLL.PLLN = 25;
            osc_init.PLL.PLLR= 2;
            osc_init.PLL.PLLP = 2;
            osc_init.PLL.PLLQ  = 2;

            clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
                                RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
            clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
            clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
            clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
            clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

            FLatency = FLASH_ACR_LATENCY_1WS;
            break;
    }
    case SYS_CLOCK_FREQ_80_MHZ: {
        osc_init.PLL.PLLM = 2;
        osc_init.PLL.PLLN = 20;
        osc_init.PLL.PLLR=2;
        osc_init.PLL.PLLP = 2;
        osc_init.PLL.PLLQ  = 2;

        clk_init.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
                            RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
        clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
        clk_init.APB1CLKDivider = RCC_HCLK_DIV2;
        clk_init.APB2CLKDivider = RCC_HCLK_DIV2;

        FLatency = FLASH_ACR_LATENCY_2WS;

        break;
    }

	default:
	  return;
	}

	if(HAL_RCC_OscConfig(&osc_init) != HAL_OK)
	{
		Error_handler();
	}


	if(HAL_RCC_ClockConfig(&clk_init,FLatency) != HAL_OK)
	{
		Error_handler();
	}

	//Systick configuration

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void GPIO_AnalogConfig(void)
{
	GPIO_InitTypeDef GpioA;

	uint32_t gpio_pins = GPIO_PIN_0 | GPIO_PIN_1 |GPIO_PIN_4  | \
                       GPIO_PIN_5 | GPIO_PIN_6 |GPIO_PIN_7  | \
                       GPIO_PIN_8 | GPIO_PIN_9 |GPIO_PIN_10 | \
                       GPIO_PIN_11| GPIO_PIN_12|GPIO_PIN_15;

	GpioA.Pin = gpio_pins;
	GpioA.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA,&GpioA);
}

void GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef ledgpio ;
  ledgpio.Pin = GPIO_PIN_5;
  ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
  ledgpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA,&ledgpio);

  ledgpio.Pin = GPIO_PIN_12;
  ledgpio.Mode = GPIO_MODE_OUTPUT_PP;
  ledgpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA,&ledgpio);
}

void UART2_Init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	if ( HAL_UART_Init(&huart2) != HAL_OK )
	{
		Error_handler();
	}
}

void TIMER6_Init(void)
{
	htimer6.Instance = TIM6;
	htimer6.Init.Prescaler = 4999;
	htimer6.Init.Period = 100-1;
	if( HAL_TIM_Base_Init(&htimer6) != HAL_OK )
	{
		Error_handler();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if ( HAL_UART_Transmit(&huart2,(uint8_t*)some_data,(uint16_t)strlen((char*)some_data),HAL_MAX_DELAY) != HAL_OK)
  {
    Error_handler();
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);
}

void Error_handler(void)
{
	while(1);
}

