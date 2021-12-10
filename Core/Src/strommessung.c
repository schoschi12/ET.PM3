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
#include "distance.h"
#include "math.h"

void measure_current_HAL(uint32_t *current_left, uint32_t *current_right) {
	while (1) {
		uint32_t avg_left = 0;
		uint32_t avg_right = 0;

		double U_halsensor = 0;
		double B_field = 0;
		double current = 0;
		double pi = 3.14159265459;

		ADC3_scan_init(11, 6);
		ADC3_scan_start();

		for (int i = 0; i < 20; i++) {
			//while (MEAS_data_ready == false);
			//	MEAS_data_ready = false;
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

		avg_left = avg_left / 20;
		avg_right = avg_right / 20;

		if (avg_left > avg_right) {
			current = (double) (0.0023 * avg_left - 0.4228);
		} else {
			current = (double) (0.0023 * avg_right - 0.4228);
		}
		/********************************************************/
		//
		//	U_halsensor = avg_left*(3.3/2^12)
		//	u_luft = 1+4e-7
		//	u_0 = 1,2566e-6 [u_0]= N/A^2
		//	Hallsensor -90mV/mT
		//  Distanc: r = 0.01 [r]=m
		// 	B = (U_halsensor/(90mV/mT)/1000
		// 	B = u_0*u_luft*I/(2*pi*r)
		//	I = B*2*pi*r/(u_0*u_luft)
		//display result$
		const double u_luft = 1 + 4e-7;
		const double u_0 = 1.2566e-6;
		const uint16_t amplification = 501;
		const uint16_t resolution = 4096;

//	U_halsensor = (double) (2300*(3.0/(resolution))); //1.68V
//	U_halsensor = U_halsensor/amplification;
//	B_field = (U_halsensor/0.09)/1000;
//	current = (B_field*2*pi*0.01)/(u_0)+0.7;

		const uint32_t Y_OFFSET = 260;
		const uint32_t X_SIZE = 240;
		uint32_t data;
		uint32_t data_last;
		/* Clear the display */
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET + 1);
		/* Write first 2 samples as numbers */
		BSP_LCD_SetFont(&Font24);
		BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		char text[16];

		snprintf(text, 15, "left : %4d", (int) (avg_left));
		BSP_LCD_DisplayStringAt(0, 60, (uint8_t*) text, LEFT_MODE);
		snprintf(text, 15, "right: %4d", (int) (avg_right));
		BSP_LCD_DisplayStringAt(0, 80, (uint8_t*) text, LEFT_MODE);
		snprintf(text, 15, "current: %.2lfA", (current));
		BSP_LCD_DisplayStringAt(0, 100, (uint8_t*) text, LEFT_MODE);

		HAL_Delay(200);					// Wait or sleep

		//*current_left = sum_channel_left;
		//*current_right = sum_channel_right;
	}
}
