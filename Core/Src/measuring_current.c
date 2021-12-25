/** ***************************************************************************
 * @file measuring_current.c
 * @brief calculates current through a single phase wire
 *
 *
 * @author  Luca Andretto
 * @date	12.12.2021
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdbool.h>
#include "distance.h"
#include "measuring_current.h"
#include "measuring.h"
#include "menu.h"

/******************************************************************************
 * Functions
 *****************************************************************************/

/******************************************************************************
 * @brief measures the current and displays it
 *
 * Initiates multiple measurements based on REPETITIONS. Calculates the average
 * amplitudes of left and right hall sensor.
 * Calculates current in the mains cable. Displays the current.
 *
 * @return the current measured
 ******************************************************************************/
double measure_current_HAL(void) {
	double current = 0;
	double SD = 0;
	double SD_right = 0;
	uint32_t buffer_left[REPETITIONS]={0};
	uint32_t buffer_right[REPETITIONS]={0};

	uint32_t avg_left = 0;
	uint32_t avg_right = 0;

for(int j = 0; j<REPETITIONS; j++){ //loop to fill the buffer for std. deviation
	for (int i = 0; i < REPETITIONS; i++) {

	ADC3_scan_init(11, 6);
	ADC3_scan_start();
		while (MEAS_data_ready == false);
		MEAS_data_ready = false;
		MEAS_average(&avg_left, &avg_right);

	}
	avg_left = avg_left / REPETITIONS;
	avg_right = avg_right / REPETITIONS;

	buffer_left[j] = avg_left;
	buffer_right[j] = avg_right;
}

/* at a constant distance from the cable, the relationship between current and
 * measured voltage is linear.
 *
 * The function that describes the relationship is:
 * 		current = 0.0023*voltage - 0.4228A
 */
	if (avg_left > avg_right) {
		current = (double) (0.0023 * avg_left - 0.4228);
		SD = (0.0023 * calculateSD(&buffer_left[0], REPETITIONS) -0.2228);
	} else {
		current = (double) (0.0023 * avg_right - 0.4228);
		SD = (0.0023 * calculateSD(&buffer_right[0], REPETITIONS) -0.2228);
	}

	char text[16];
	snprintf(text, 15, "current: %.2lfA", (current));
	BSP_LCD_DisplayStringAt(0, 100, (uint8_t*) text, LEFT_MODE);
	snprintf(text, 15, "Std.dev: %.2lfA", (SD));
	BSP_LCD_DisplayStringAt(0, 120, (uint8_t*) text, LEFT_MODE);


	return current;

}
