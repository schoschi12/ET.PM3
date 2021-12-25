/** ***************************************************************************
 * @file shutdown.c
 * @brief initialize the timerbased shutdown
 *
 * Contains the initialization of timer 7 and the preparation of the GPIO
 *
 * @author  Luca Andretto
 * @date	25.12.2021
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "shutdown.h"

/******************************************************************************
 * Variables
 *****************************************************************************/
static uint32_t timer_value_ms = 0;

/******************************************************************************
 * Functions
 *****************************************************************************/

/** ***************************************************************************
 * @brief prepares PA5 as opendrain output to turn of the device after a
 * defined period of time to safe battery power
 *****************************************************************************/
void prepare_shutdown(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__GPIOA_CLK_ENABLE();


	/*Configure GPIO pin : PA5 */
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;    // Open Drain
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

/** ***************************************************************************
 * @brief function to activatie the opendrain pin PA5 for the shutdown, this
 * state is visualized with the red led (LED4)
 *****************************************************************************/
void initiate_shutdown(void) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	BSP_LED_On(LED4);
}

/** ***************************************************************************
 * @brief function to check for the time the device is running
 * if the device runs longer than 20 seconds the shutdown will be initialized
 *****************************************************************************/
void check_time(void) {
	if (timer_value_ms > 20000) {
		initiate_shutdown();
	}
}

/** ***************************************************************************
 * @brief reset of the upcounting timer value to 0
 *****************************************************************************/
void reset_timer7_value(void) {
	timer_value_ms = 0;
}

/** ***************************************************************************
 * @brief initialize timercounter 7 to a milisecond based timer with enabled
 * interrupt
 * 84MHz --> x*ms
 *****************************************************************************/
void tim_TIM7_periodicConfig(uint32_t msPeriod) {
	//Enable TIM7 timer
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
	//1 Pulse mode enable
	TIM7->CR1 &= ~(TIM_CR1_OPM);
	//Mode --> RESET
	TIM7->CR2 &= ~(TIM_CR2_MMS);

	//Prescaler
	TIM7->PSC = (msPeriod * 10) - 1;
	//Period
	TIM7->ARR = 8400 - 1; //->10kHz

	//Clear Update Interrupt
	TIM7->SR &= ~(1UL << 0);
	//Enable update interrupt
	TIM7->DIER |= 0x01;
	//Enable NVIC Interrupt
	NVIC_EnableIRQ(TIM7_IRQn);
	//Start perodic timer
	TIM7->CR1 |= 0x01;
}

/** ***************************************************************************
 * @brief interrupt service routine of timercounter 7. Occurs periodically and
 * counts timer_value_ms up.
 *****************************************************************************/
void TIM7_IRQHandler(void) {
	if (TIM7->SR & 0x01) {
		TIM7->SR &= ~(1UL); //reset flag
		timer_value_ms += 1;
		if (timer_value_ms > 30000) {
			timer_value_ms = 0;
		}
	}
}
