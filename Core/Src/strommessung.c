/** ***************************************************************************
 * @file
 * @brief calculates distance and angle and displays them
 *
 * Contains the calibration table for distance and angle measurement.
 * Initiates measurement, calculates average and displays the values to the display.
 *
 * @author  Luca Andretto
 * @date	12.12.2021
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdbool.h>
#include "stm32f429i_discovery_lcd.h"
#include "strommessung.h"
#include "measuring.h"
#include "distance.h"
#include "menu.h"

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * @brief measures the current and displays it
 *
 * Initiates multiple measurements based on REPETITIONS. Calculates the average amplitudes of left and right hall sensor.
 * Calculates current in the mains cable. Displays the current.
 *
 * @return the current measured
 */
double measure_current_HAL(void) {
	double current = 0;
	//while(1){

	uint32_t avg_left = 0;
	uint32_t avg_right = 0;
//
//	ADC3_scan_init(11, 6);
//	ADC3_scan_start();

	for (int i = 0; i < REPETITIONS; i++) {

		ADC3_scan_init(11, 6);
		ADC3_scan_start();
		while (MEAS_data_ready == false)
			;
		MEAS_data_ready = false;
		MEAS_average(&avg_left, &avg_right);

//		uint32_t *ADC_samples = get_ADC_samples();
//		uint32_t left_max = 0;
//		uint32_t right_max = 0;
//		uint32_t left_min = 5000;
//		uint32_t right_min = 5000;
//		double left_value, right_value;
//
//		for (int i = 0; i < get_ADC_NUMS(); i++) {
//			if (ADC_samples[2 * i] > left_max) {
//				left_max = ADC_samples[2 * i];
//			}
//			if (ADC_samples[2 * i + 1] > right_max) {
//				right_max = ADC_samples[2 * i + 1];
//			}
//			if (ADC_samples[2 * i] < left_min) {
//				left_min = ADC_samples[2 * i];
//			}
//			if (ADC_samples[2 * i + 1] < right_min) {
//				right_min = ADC_samples[2 * i + 1];
//			}
//
////				int mov_avg = 4;
////						for (int i = 2; i < (get_ADC_NUMS() - 2); i++) {
////				//			right_value = 0;
////				//			left_value = 0;
////				//			for (int j = 0; j <= mov_avg; j++){
////				//				right_value += ADC_samples[2*(j - mov_avg) + 1];
////				//				left_value += ADC_samples[2*(j - mov_avg)];
////				//			}
////				//
////				//			right_value /= (mov_avg * 2 + 1);
////				//			left_value /= (mov_avg * 2 + 1);
////							left_value = (ADC_samples[2*(i-2)+1] + ADC_samples[2*(i-1)+1] + ADC_samples[2*i+1] + ADC_samples[2*(i+1)+1] + ADC_samples[2*(i+2)+1]) / 5;
////							right_value = (ADC_samples[2*(i-2)] + ADC_samples[2*(i-1)] + ADC_samples[2*i] + ADC_samples[2*(i+1)] + ADC_samples[2*(i+2)]) / 5;
////
////							if ((uint32_t)left_value > left_max) {
////								left_max = (uint32_t)left_value;
////							}
////							if ((uint32_t)right_value > right_max) {
////								right_max = (uint32_t)right_value;
////							}
////							if ((uint32_t)left_value < left_min) {
////								left_min = (uint32_t)left_value;
////							}
////							if ((uint32_t)right_value < right_min) {
////								right_min = (uint32_t)right_value;
////							}
////
//		}
//		avg_left += (left_max - left_min);
//		avg_right += (right_max - right_min);
	}

	avg_left = avg_left / REPETITIONS;
	avg_right = avg_right / REPETITIONS;

	if (avg_left > avg_right) {
		current = (double) (0.0023 * avg_left - 0.4228);
	} else {
		current = (double) (0.0023 * avg_right - 0.4228);
	}

	const uint32_t Y_OFFSET = 200;
	const uint32_t X_SIZE = 240;
	uint32_t data;
	uint32_t data_last;
	/* Clear the display */
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET + 1 + 60);
	/* Write first 2 samples as numbers */
	MENU_draw();
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

	return current;

}
