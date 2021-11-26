/*
 * strommessung.c
 *
 *  Created on: 26.11.2021
 *      Author: Luca Andretto
 */


/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdbool.h>
#include "strommessung.h"
#include "measuring.h"

uint32_t measure_current_HAL_Left()
{
	ADC3_IN6_DMA_init();
	ADC3_IN6_DMA_start();
					uint32_t *ADC_samples = get_ADC_samples();
					uint32_t left_max = 0;
					uint32_t right_max = 0;
					uint32_t left_min = 5000;
					uint32_t right_min = 5000;
					for (int i = 0; i < get_ADC_NUMS(); i++){
						if (ADC_samples[2*i] > left_max){
							left_max = ADC_samples[2*i];
						}
						if (ADC_samples[2*i+1] > right_max){
							right_max = ADC_samples[2*i+1];
						}
						if (ADC_samples[2*i] < left_min){
							left_min = ADC_samples[2*i];
						}
						if (ADC_samples[2*i+1] < right_min){
							right_min = ADC_samples[2*i+1];
						}

					}
		return (left_max-left_min);
}
uint32_t measure_current_HAL_Right()
{
//	 ADC3_IN11_IN6_scan_init();
//	 ADC3_IN11_IN6_scan_start();
//		uint32_t *ADC_samples = get_ADC_samples();
//		uint32_t left_max = 0;
//		uint32_t right_max = 0;
//		uint32_t left_min = 5000;
//		uint32_t right_min = 5000;
//		for (int i = 0; i < get_ADC_NUMS(); i++){
//			if (ADC_samples[2*i] > left_max){
//				left_max = ADC_samples[2*i];
//			}
//			if (ADC_samples[2*i+1] > right_max){
//				right_max = ADC_samples[2*i+1];
//			}
//			if (ADC_samples[2*i] < left_min){
//				left_min = ADC_samples[2*i];
//			}
//			if (ADC_samples[2*i+1] < right_min){
//				right_min = ADC_samples[2*i+1];
//			}
//
//		}
//		return (right_max-right_min);
	ADC3_IN11_DMA_init();
	ADC3_IN11_DMA_start();
						uint32_t *ADC_samples = get_ADC_samples();
						uint32_t left_max = 0;
						uint32_t right_max = 0;
						uint32_t left_min = 5000;
						uint32_t right_min = 5000;
						for (int i = 0; i < get_ADC_NUMS(); i++){
							if (ADC_samples[2*i] > left_max){
								left_max = ADC_samples[2*i];
							}
							if (ADC_samples[2*i+1] > right_max){
								right_max = ADC_samples[2*i+1];
							}
							if (ADC_samples[2*i] < left_min){
								left_min = ADC_samples[2*i];
							}
							if (ADC_samples[2*i+1] < right_min){
								right_min = ADC_samples[2*i+1];
							}

						}
			return (right_max-right_min);
}
