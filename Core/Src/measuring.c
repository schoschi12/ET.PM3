/** ***************************************************************************
 * @file
 * @brief Measuring voltages with the ADC(s) in different configurations
 *
 * Contains functions to configure and start ADC with DMA. Also contains
 * interrupt handlers to manage finisched ADC conversions. Additionally,
 * functions to average multiple measurements as well as functions to
 * calculate the standard deviation are implemented in this file.
 *
 *
 *
 * ----------------------------------------------------------------------------
 * @author Janosch Dusoczky
 * @date 12.12.2021
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdio.h>
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#include "measuring.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#define ADC_DAC_RES		12			///< Resolution
#define ADC_NUMS		120			///< Number of samples
#define ADC_FS			1200		///< Sampling freq. => 12 samples for a 50Hz period
#define ADC_CLOCK		84000000	///< APB2 peripheral clock frequency
#define ADC_CLOCKS_PS	15			///< Clocks/sample: 3 hold + 12 conversion
#define TIM_CLOCK		84000000	///< APB1 timer clock frequency
#define TIM_TOP			9			///< Timer top value
#define TIM_PRESCALE	(TIM_CLOCK/ADC_FS/(TIM_TOP+1)-1) ///< Clock prescaler

/******************************************************************************
 * Variables
 *****************************************************************************/
bool MEAS_data_ready = false;			///< New data is ready
uint32_t MEAS_input_count = 1;			///< 1 or 2 input channels?
bool DAC_active = false;				///< DAC output active?

static uint32_t ADC_sample_count = 0;	///< Index for buffer
static uint32_t ADC_samples[2 * ADC_NUMS];///< ADC values of max. 2 input channels
static uint32_t DAC_sample = 0;			///< DAC output value

/******************************************************************************
 * Functions
 *****************************************************************************/
int get_ADC_NUMS(void) {
	return ADC_NUMS;
}

uint32_t* get_ADC_samples(void) {
	return ADC_samples;
}

/** ***************************************************************************
 * @brief Configure GPIOs in analog mode.
 *
 * @note The input number for the ADCs is not equal to the GPIO pin number!
 * - ADC3_IN4 = GPIO PF6
 * - ADC123_IN13 = GPIO PC3
 * - ADC12_IN5 = GPIO PA5
 * - DAC_OUT2 = GPIO PA5 (= same GPIO as ADC12_IN5)
 *****************************************************************************/
void MEAS_GPIO_analog_init(void) {
	__HAL_RCC_GPIOF_CLK_ENABLE();		// Enable Clock for GPIO port F
	GPIOF->MODER |= (GPIO_MODER_MODER6_Msk);// Analog mode for PF6 = ADC3_IN4
	__HAL_RCC_GPIOC_CLK_ENABLE();		// Enable Clock for GPIO port C
	GPIOC->MODER |= (GPIO_MODER_MODER3_Msk);// Analog mode for PC3 = ADC123_IN13
	__HAL_RCC_GPIOA_CLK_ENABLE();		// Enable Clock for GPIO port A
	GPIOA->MODER |= (GPIO_MODER_MODER5_Msk);	// Analog mode for PA5 ADC12_IN5
}

/** ***************************************************************************
 * @brief Resets the ADCs and the timer
 *
 * to make sure the different demos do not interfere.
 *****************************************************************************/
void ADC_reset(void) {
	RCC->APB2RSTR |= RCC_APB2RSTR_ADCRST;	// Reset ADCs
	RCC->APB2RSTR &= ~RCC_APB2RSTR_ADCRST;	// Release reset of ADCs
	TIM2->CR1 &= ~TIM_CR1_CEN;				// Disable timer
}

/** ***************************************************************************
 * @brief Configure the timer to trigger the ADC(s)
 *
 * @note For debugging purposes the timer interrupt might be useful.
 *****************************************************************************/
void MEAS_timer_init(void) {
	__HAL_RCC_TIM2_CLK_ENABLE();		// Enable Clock for TIM2
	TIM2->PSC = TIM_PRESCALE;			// Prescaler for clock freq. = 1MHz
	TIM2->ARR = TIM_TOP;				// Auto reload = counter top value
	TIM2->CR2 |= TIM_CR2_MMS_1; 		// TRGO on update
	/* If timer interrupt is not needed, comment the following lines */
	TIM2->DIER |= TIM_DIER_UIE;			// Enable update interrupt
	NVIC_ClearPendingIRQ(TIM2_IRQn);	// Clear pending interrupt on line 0
	NVIC_EnableIRQ(TIM2_IRQn);			// Enable interrupt line 0 in the NVIC
}

/** ***************************************************************************
 * @brief Initialize ADC, timer and DMA for sequential acquisition = scan mode
 *
 * Uses ADC3 and DMA2_Stream1 channel2
 * @n The ADC3 trigger is set to TIM2 TRGO event
 * @n At each trigger both inputs are converted sequentially
 * and transfered to memory by the DMA.
 * @n As each conversion triggers the DMA, the number of transfers is doubled.
 * @n The DMA triggers the transfer complete interrupt when all data is ready.
 * @n The inputs used are ADC123_IN11 = GPIO PC1 and ADC3_IN6 = GPIO PF8
 *****************************************************************************/
void ADC3_scan_init(unsigned long channel1, unsigned long channel2) {
	MEAS_input_count = 2;				// Only 1 input is converted
	__HAL_RCC_ADC3_CLK_ENABLE();		// Enable Clock for ADC3
	ADC3->SQR1 |= ADC_SQR1_L_0;			// Convert 2 inputs
	ADC3->SQR3 |= (channel1 << ADC_SQR3_SQ1_Pos);// Input 13 = first conversion
	ADC3->SQR3 |= (channel2 << ADC_SQR3_SQ2_Pos);// Input 4 = second conversion
	ADC3->CR1 |= ADC_CR1_SCAN;			// Enable scan mode
	ADC3->CR2 |= (1UL << ADC_CR2_EXTEN_Pos);	// En. ext. trigger on rising e.
	ADC3->CR2 |= (6UL << ADC_CR2_EXTSEL_Pos);	// Timer 2 TRGO event
	ADC3->CR2 |= ADC_CR2_DMA;			// Enable DMA mode
	__HAL_RCC_DMA2_CLK_ENABLE();		// Enable Clock for DMA2
	DMA2_Stream1->CR &= ~DMA_SxCR_EN;	// Disable the DMA stream 1
	while (DMA2_Stream1->CR & DMA_SxCR_EN) {
		;
	}	// Wait for DMA to finish
	DMA2->LIFCR |= DMA_LIFCR_CTCIF1;	// Clear transfer complete interrupt fl.
	DMA2_Stream1->CR |= (2UL << DMA_SxCR_CHSEL_Pos);	// Select channel 2
	DMA2_Stream1->CR |= DMA_SxCR_PL_1;		// Priority high
	DMA2_Stream1->CR |= DMA_SxCR_MSIZE_1;	// Memory data size = 32 bit
	DMA2_Stream1->CR |= DMA_SxCR_PSIZE_1;	// Peripheral data size = 32 bit
	DMA2_Stream1->CR |= DMA_SxCR_MINC;	// Increment memory address pointer
	DMA2_Stream1->CR |= DMA_SxCR_TCIE;	// Transfer complete interrupt enable
	DMA2_Stream1->NDTR = 2 * ADC_NUMS;	// Number of data items to transfer
	DMA2_Stream1->PAR = (uint32_t) &ADC3->DR;	// Peripheral register address
	DMA2_Stream1->M0AR = (uint32_t) ADC_samples;// Buffer memory loc. address

}

/** ***************************************************************************
 * @brief Start DMA, ADC and timer
 *
 *****************************************************************************/
void ADC3_scan_start(void) {
	DMA2_Stream1->CR |= DMA_SxCR_EN;	// Enable DMA
	NVIC_ClearPendingIRQ(DMA2_Stream1_IRQn);	// Clear pending DMA interrupt
	NVIC_EnableIRQ(DMA2_Stream1_IRQn);	// Enable DMA interrupt in the NVIC
	ADC3->CR2 |= ADC_CR2_ADON;			// Enable ADC3
	TIM2->CR1 |= TIM_CR1_CEN;			// Enable timer
}

/** ***************************************************************************
 * @brief Interrupt handler for the timer 2
 *
 * @note This interrupt handler was only used for debugging purposes
 * and to increment the DAC value.
 *****************************************************************************/
void TIM2_IRQHandler(void) {
	TIM2->SR &= ~TIM_SR_UIF;			// Clear pending interrupt flag
}

/** ***************************************************************************
 * @brief Interrupt handler for the ADCs
 *
 * Reads one sample from the ADC3 DataRegister and transfers it to a buffer.
 * @n Stops when ADC_NUMS samples have been read.
 *****************************************************************************/
void ADC_IRQHandler(void) {
	if (ADC3->SR & ADC_SR_EOC) {		// Check if ADC3 end of conversion
		ADC_samples[ADC_sample_count++] = ADC3->DR;	// Read input channel 1 only
		if (ADC_sample_count >= ADC_NUMS) {		// Buffer full
			TIM2->CR1 &= ~TIM_CR1_CEN;	// Disable timer
			ADC3->CR2 &= ~ADC_CR2_ADON;	// Disable ADC3
			ADC_reset();
			MEAS_data_ready = true;
		}

	}
}

/** ***************************************************************************
 * @brief Interrupt handler for DMA2 Stream1
 *
 * The samples from the ADC3 have been transfered to memory by the DMA2 Stream1
 * and are ready for processing.
 *****************************************************************************/
void DMA2_Stream1_IRQHandler(void) {
	if (DMA2->LISR & DMA_LISR_TCIF1) {	// Stream1 transfer compl. interrupt f.
		NVIC_DisableIRQ(DMA2_Stream1_IRQn);	// Disable DMA interrupt in the NVIC
		NVIC_ClearPendingIRQ(DMA2_Stream1_IRQn);// Clear pending DMA interrupt
		DMA2_Stream1->CR &= ~DMA_SxCR_EN;	// Disable the DMA
		while (DMA2_Stream1->CR & DMA_SxCR_EN) {
			;
		}	// Wait for DMA to finish
		DMA2->LIFCR |= DMA_LIFCR_CTCIF1;// Clear transfer complete interrupt fl.
		TIM2->CR1 &= ~TIM_CR1_CEN;		// Disable timer
		ADC3->CR2 &= ~ADC_CR2_ADON;		// Disable ADC3
		ADC3->CR2 &= ~ADC_CR2_DMA;		// Disable DMA mode
		ADC_reset();
		MEAS_data_ready = true;
	}
}

/** ***************************************************************************
 * @brief Interrupt handler for DMA2 Stream3
 *
 * The samples from the ADC3 have been transfered to memory by the DMA2 Stream1
 * and are ready for processing.
 *****************************************************************************/
void DMA2_Stream3_IRQHandler(void) {
	if (DMA2->LISR & DMA_LISR_TCIF3) {	// Stream3 transfer compl. interrupt f.
		NVIC_DisableIRQ(DMA2_Stream3_IRQn);	// Disable DMA interrupt in the NVIC
		NVIC_ClearPendingIRQ(DMA2_Stream3_IRQn);// Clear pending DMA interrupt
		DMA2_Stream3->CR &= ~DMA_SxCR_EN;	// Disable the DMA
		while (DMA2_Stream3->CR & DMA_SxCR_EN) {
			;
		}	// Wait for DMA to finish
		DMA2->LIFCR |= DMA_LIFCR_CTCIF3;// Clear transfer complete interrupt fl.
		TIM2->CR1 &= ~TIM_CR1_CEN;		// Disable timer
		ADC2->CR2 &= ~ADC_CR2_ADON;		// Disable ADC2
		ADC2->CR2 &= ~ADC_CR2_DMA;		// Disable DMA mode
		ADC_reset();
		MEAS_data_ready = true;
	}
}

/** ***************************************************************************
 * @brief Interrupt handler for DMA2 Stream4
 *
 * Here the interrupt handler is used together with ADC1 and ADC2
 * in dual mode where they sample simultaneously.
 * @n The samples from both ADCs packed in a 32 bit word have been transfered
 * to memory by the DMA2 and are ready for unpacking.
 * @note In dual ADC mode two values are combined (packed) in a single uint32_t
 * ADC_CDR[31:0] = ADC2_DR[15:0] | ADC1_DR[15:0]
 * and are therefore extracted before further processing.
 *****************************************************************************/
void DMA2_Stream4_IRQHandler(void) {
	if (DMA2->HISR & DMA_HISR_TCIF4) {	// Stream4 transfer compl. interrupt f.
		NVIC_DisableIRQ(DMA2_Stream4_IRQn);	// Disable DMA interrupt in the NVIC
		NVIC_ClearPendingIRQ(DMA2_Stream4_IRQn);// Clear pending DMA interrupt
		DMA2_Stream4->CR &= ~DMA_SxCR_EN;	// Disable the DMA
		while (DMA2_Stream4->CR & DMA_SxCR_EN) {
			;
		}	// Wait for DMA to finish
		DMA2->HIFCR |= DMA_HIFCR_CTCIF4;// Clear transfer complete interrupt fl.
		TIM2->CR1 &= ~TIM_CR1_CEN;		// Disable timer
		ADC1->CR2 &= ~ADC_CR2_ADON;		// Disable ADC1
		ADC2->CR2 &= ~ADC_CR2_ADON;		// Disable ADC2
		ADC->CCR &= ~ADC_CCR_DMA_1;		// Disable DMA mode
		/* Extract combined samples */
		for (int32_t i = ADC_NUMS - 1; i >= 0; i--) {
			ADC_samples[2 * i + 1] = (ADC_samples[i] >> 16);
			ADC_samples[2 * i] = (ADC_samples[i] & 0xffff);
		}
		ADC_reset();
		MEAS_data_ready = true;
	}
}

/**
 * @brief Applies 5 point moving average to values in ADC_DMA buffer.
 *
 * Calculates difference between maximum and minimum value.
 *
 * @param pointer where amplitude of left sensing pad is to be stored.
 * @param pointer where amplitude of right sensing pad is to be stored.
 */
void MEAS_average(uint32_t *avg_left, uint32_t *avg_right) {
	uint32_t *ADC_samples = get_ADC_samples();
	uint32_t left_max = 0;
	uint32_t right_max = 0;
	uint32_t left_min = 5000;
	uint32_t right_min = 5000;
	double left_value, right_value;
	int mov_avg = 4;
	for (int i = 2; i < (get_ADC_NUMS() - 2); i++) {
		left_value = (ADC_samples[2 * (i - 2) + 1]
				+ ADC_samples[2 * (i - 1) + 1] + ADC_samples[2 * i + 1]
				+ ADC_samples[2 * (i + 1) + 1] + ADC_samples[2 * (i + 2) + 1])
				/ 5;
		right_value = (ADC_samples[2 * (i - 2)] + ADC_samples[2 * (i - 1)]
				+ ADC_samples[2 * i] + ADC_samples[2 * (i + 1)]
				+ ADC_samples[2 * (i + 2)]) / 5;

		if ((uint32_t) left_value > left_max) {
			left_max = (uint32_t) left_value;
		}
		if ((uint32_t) right_value > right_max) {
			right_max = (uint32_t) right_value;
		}
		if ((uint32_t) left_value < left_min) {
			left_min = (uint32_t) left_value;
		}
		if ((uint32_t) right_value < right_min) {
			right_min = (uint32_t) right_value;
		}

	}
	*avg_left += (left_max - left_min);
	*avg_right += (right_max - right_min);
}

/**
 * @brief Calculates the standard deviation of the values in the ADC_DMA buffer.
 *
 * SD = sqrt(sum(x_i - mean)^2)
 *
 * @return double with standard deviation of stored values from ADC_DMA buffer.
 */

double calculateSD(uint32_t *buffer, uint8_t repetitions) {
	double sum = 0.0, mean, SD = 0.0;
	uint8_t i;

	for (i = 0; i < repetitions; ++i) {
		sum += (double) buffer[i];
	}
	mean = sum / repetitions;

	for (i = 0; i < repetitions; ++i) {
		SD += pow((double) buffer[i] - mean, 2);
	}
	return sqrt(SD / repetitions);
}
