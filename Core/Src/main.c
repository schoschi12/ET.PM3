/** ***************************************************************************
 * @file
 * @brief Sets up the microcontroller, the clock system and the peripherals.
 *
 * Initialization is done for the system, the blue user button, the user LEDs,
 * and the LCD display with the touchscreen.
 * @n Then the code enters an infinite while-loop, where it checks for
 * user input and starts the requested demonstration.
 *
 * @author  Hanspeter Hochreutener, hhrt@zhaw.ch
 * @date	17.06.2021
 *****************************************************************************/


/******************************************************************************
 * Includes
 *****************************************************************************/
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#include "main.h"
#include "pushbutton.h"
#include "menu.h"
#include "measuring.h"
#include "strommessung.h"
#include "distance.h"


/******************************************************************************
 * Defines
 *****************************************************************************/


/******************************************************************************
 * Variables
 *****************************************************************************/
RTC_HandleTypeDef hrtc;
#define RtcHandle &hrtc

/******************************************************************************
 * Functions
 *****************************************************************************/
static void SystemClock_Config(void);	///< System Clock Configuration
static void gyro_disable(void);			///< Disable the onboard gyroscope
static void MX_RTC_Init(void);
static void MX_GPIO_Init(void);

/** ***************************************************************************
 * @brief  Main function
 * @return not used because main ends in an infinite loop
 *
 * Initialization and infinite while loop
 *****************************************************************************/
int main(void) {
	HAL_Init();							// Initialize the system

	SystemClock_Config();				// Configure system clocks

	BSP_LCD_Init();						// Initialize the LCD display
	BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, LCD_FRAME_BUFFER);
	BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
	BSP_LCD_DisplayOn();
	BSP_LCD_Clear(LCD_COLOR_WHITE);

	BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());	// Touchscreen
	/* Uncomment next line to enable touchscreen interrupt */
	// BSP_TS_ITConfig();					// Enable Touchscreen interrupt

	PB_init();							// Initialize the user pushbutton
	PB_enableIRQ();						// Enable interrupt on user pushbutton

	BSP_LED_Init(LED3);					// Toggles in while loop
	BSP_LED_Init(LED4);					// Is toggled by user button

	MENU_draw();						// Draw the menu
	MENU_hint();						// Show hint at startup

	gyro_disable();						// Disable gyro, use those analog inputs

	MEAS_GPIO_analog_init();			// Configure GPIOs in analog mode
	MEAS_timer_init();					// Configure the timer

	MX_GPIO_Init();
	MX_RTC_Init();						// Configure RTC

    RTC_TimeTypeDef sTime;

    sTime.Hours = 1;
    sTime.Minutes = 1;
    sTime.Seconds = 0;
    HAL_RTC_SetTime(RtcHandle, &sTime, RTC_FORMAT_BCD);

	/* Infinite while loop */
	while (1) {							// Infinitely loop in main function
		BSP_LED_Toggle(LED3);			// Visual feedback when running

		if (MEAS_data_ready) {			// Show data if new data available
			MEAS_data_ready = false;
			MEAS_show_data();
		}

		if (PB_pressed()) {				// Check if user pushbutton was pressed
			DAC_active = !DAC_active;	// Toggle DAC on/off
			if (DAC_active) {
				DAC_init();
				BSP_LED_On(LED4);
			} else {
				DAC_reset();
				BSP_LED_Off(LED4);
			}
		}

		/* Comment next line if touchscreen interrupt is enabled */
		MENU_check_transition();

		int counter = 0;

		switch (MENU_get_transition()) {	// Handle user menu choice
		case MENU_NONE:					// No transition => do nothing
			break;
		case MENU_ZERO:
			ADC3_IN4_single_init();
			ADC3_IN4_single_read();
			break;
		case MENU_ONE:
			ADC3_IN4_timer_init();
			ADC3_IN4_timer_start();
			break;
		case MENU_TWO:
			ADC3_IN4_DMA_init();
			ADC3_IN4_DMA_start();
			break;
		case MENU_THREE:
			ADC1_IN13_ADC2_IN5_dual_init();
			ADC1_IN13_ADC2_IN5_dual_start();
			break;
		case MENU_FOUR:
			//ADC2_IN13_IN5_scan_init();
			//ADC2_IN13_IN5_scan_start();
			uint32_t current_left, current_right;
			 measure_current_HAL( &current_left, &current_right);
			break;
		case MENU_FIVE:
//			while(1){
//				ADC3_IN13_IN4_scan_init();
//				ADC3_IN13_IN4_scan_start();
//				while(MEAS_data_ready == false);
//				MEAS_data_ready = false;
//				uint32_t *ADC_samples = get_ADC_samples();
//				uint32_t left_max = 0;
//				uint32_t right_max = 0;
//				uint32_t left_min = 5000;
//				uint32_t right_min = 5000;
//				for (int i = 0; i < get_ADC_NUMS(); i++){
//					if (ADC_samples[2*i] > left_max){
//						left_max = ADC_samples[2*i];
//					}
//					if (ADC_samples[2*i+1] > right_max){
//						right_max = ADC_samples[2*i+1];
//					}
//					if (ADC_samples[2*i] < left_min){
//						left_min = ADC_samples[2*i];
//					}
//					if (ADC_samples[2*i+1] < right_min){
//						right_min = ADC_samples[2*i+1];
//					}
//
//				}
//
//				measure_current_HAL_Left();
//				//display result
//				const uint32_t Y_OFFSET = 260;
//				const uint32_t X_SIZE = 240;
//				uint32_t data;
//				uint32_t data_last;
//				/* Clear the display */
//				BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
//				BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET+1);
//				/* Write first 2 samples as numbers */
//				BSP_LCD_SetFont(&Font24);
//				BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
//				BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
//				char text[16];
//				snprintf(text, 15, "LEFT: %4d", (int)(left_max-left_min));
//				BSP_LCD_DisplayStringAt(0, 50, (uint8_t *)text, LEFT_MODE);
//				snprintf(text, 15, "RIGHT: %4d", (int)(right_max-right_min));
//				BSP_LCD_DisplayStringAt(0, 80, (uint8_t *)text, LEFT_MODE);
//				HAL_Delay(200);
//
//			}
			while(1)
			{
				measure_distance();
				//HAL_Delay(200);
			}

			break;
		default:						// Should never occur
			break;
		}

		HAL_Delay(200);					// Wait or sleep
	}
}


/** ***************************************************************************
 * @brief System Clock Configuration
 *
 *****************************************************************************/
static void SystemClock_Config(void){
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	/* Configure the main internal regulator output voltage */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/* Initialize High Speed External Oscillator and PLL circuits */
	//RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);
	/* Initialize gates and clock dividers for CPU, AHB and APB busses */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
	/* Initialize PLL and clock divider for the LCD */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	 PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/* Set clock prescaler for ADCs */
	ADC->CCR |= ADC_CCR_ADCPRE_0;
}


/** ***************************************************************************
 * @brief Disable the GYRO on the microcontroller board.
 *
 * @note MISO of the GYRO is connected to PF8 and CS to PC1.
 * @n Some times the GYRO goes into an undefined mode at startup
 * and pulls the MISO low or high thus blocking the analog input on PF8.
 * @n The simplest solution is to pull the CS of the GYRO low for a short while
 * which is done with the code below.
 * @n PF8 is also reconfigured.
 * @n An other solution would be to remove the GYRO
 * from the microcontroller board by unsoldering it.
 *****************************************************************************/
static void gyro_disable(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();		// Enable Clock for GPIO port C
	/* Disable PC1 and PF8 first */
	GPIOC->MODER &= ~GPIO_MODER_MODER1; // Reset mode for PC1
	GPIOC->MODER |= GPIO_MODER_MODER1_0;	// Set PC1 as output
	GPIOC->BSRR |= GPIO_BSRR_BR1;		// Set GYRO (CS) to 0 for a short time
	HAL_Delay(10);						// Wait some time
	GPIOC->MODER |= GPIO_MODER_MODER1_Msk; // Analog mode PC1 = ADC123_IN11
	__HAL_RCC_GPIOF_CLK_ENABLE();		// Enable Clock for GPIO port F
	GPIOF->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED8;	// Reset speed of PF8
	GPIOF->AFR[1] &= ~GPIO_AFRH_AFSEL8;			// Reset alternate func. of PF8
	GPIOF->PUPDR &= ~GPIO_PUPDR_PUPD8;			// Reset pulup/down of PF8
	HAL_Delay(10);						// Wait some time
	GPIOF->MODER |= GPIO_MODER_MODER8_Msk; // Analog mode for PF6 = ADC3_IN4
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
 void MX_RTC_Init(void)
 {

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
   // Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    //Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    //Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

 void MX_GPIO_Init(void)
 {
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	 /* GPIO Ports Clock Enable */
	   __GPIOG_CLK_ENABLE();

	 /*Configure GPIO pin : PB14 */
	  GPIO_InitStruct.Pin = GPIO_PIN_14;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     // digital Output
	  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
 }

