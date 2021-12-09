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

void measure_current_HAL(uint32_t *current_left, uint32_t *current_right)
{

	uint32_t avg_left = 0;
	uint32_t avg_right = 0;
		for (int i = 0; i < 10; i++) {
	ADC3_scan_init(11,6);
	ADC3_scan_start();
			while (MEAS_data_ready == false);
			MEAS_data_ready = false;
			uint32_t *ADC_samples = get_ADC_samples();
			uint32_t left_max = 0;
			uint32_t right_max = 0;
			uint32_t left_min = 5000;
			uint32_t right_min = 5000;
			for (int i = 0; i < get_ADC_NUMS(); i++) {
				if (ADC_samples[2 * i] > left_max) {
					left_max = ADC_samples[2 * i];
				}
				if (ADC_samples[2 * i + 1] > right_max) {
					right_max = ADC_samples[2 * i + 1];
				}
				if (ADC_samples[2 * i] < left_min) {
					left_min = ADC_samples[2 * i];
				}
				if (ADC_samples[2 * i + 1] < right_min) {
					right_min = ADC_samples[2 * i + 1];
				}

			}
			avg_left += (left_max - left_min);
			avg_right += (right_max - right_min);
		}
		avg_right /= 10; // 3000/Verstärkung (4095)
		avg_left /= 10;


	*current_left = sum_channel_left;
	*current_right = sum_channel_right;
}
