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
	ADC3_scan_init(11,6);
	ADC3_scan_start();

	uint32_t *ADC_samples = get_ADC_samples();
	uint32_t sum_channel_left = 0;
	uint32_t sum_channel_right = 0;

	for (int i = 0; i < (2*get_ADC_NUMS()+1); i+=2)
	{
		for(int j = 0; j < 4; j++ )
		{
			sum_channel_left = sum_channel_left + ADC_samples[i];
		}

	}

	sum_channel_left = sum_channel_left/60;	//Average over 60 measurements

	for (int i = 1; i < (2*get_ADC_NUMS()+1); i+=2)
	{
		sum_channel_right = sum_channel_right + ADC_samples[i];
	}

	sum_channel_right = sum_channel_right/60;

	*current_left = sum_channel_left;
	*current_right = sum_channel_right;
}
