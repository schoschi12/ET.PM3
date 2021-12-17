/** ***************************************************************************
 * @file
 * @brief calculates distance and angle and displays them
 *
 * Contains the calibration table for distance and angle measurement.
 * Initiates measurement, calculates average and displays the values to the display.
 *
 * @author  Janosch Dusoczky
 * @date	12.12.2021
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "distance.h"

/******************************************************************************
 * Variables
 *****************************************************************************/
uint16_t points[] = { 25, 0, 25, 100, 0, 25, 50, 25 };
uint16_t n_points[8];
float sine;
float cosine;
int x_int;
int y_int;
float correction = 1.0;
//uint16_t amplitudeLUT[] = {3760, 3650, 3275, 2750, 2475, 2175, 2025, 1925, 1800, 1575, 1450};
//uint16_t amplitudeLUT[] = {3800, 3722, 3585, 3400, 3143, 2837, 2630, 2475, 2370, 2304, 2180};
uint16_t amplitudeLUT[] = { 3780, 3713, 3527, 3125, 2815, 2560, 2250, 1955,
		1915, 1785, 1700 };						// LUT Amplitude in ADC counts
uint16_t distanceLUT[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };// LUT distance in mm
double factorLUT[] = { 0.855, 0.955, 1.065, 1.17 };	// factor LUT calculated as left distance / right distance
int angleLUT[] = { 45, 22.5, 0, -22.5, -45 };			// angle LUT in °
uint32_t buffer[STDDEVSIZE] = { 0 }; // FIFO buffer for values used for std deviation calculation
uint32_t SD_raw; 					// raw ADC value of std. deviation
uint32_t SD_calc;			// calculated distance value of std. deviation in um

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * @brief calculates distance and standarddeviation based on calibration table.
 *
 * Averages the amplitude of both sensing pads and searches for corresponding distance. Saves distance in global variable.
 *
 * @param amplitude of left sensing pad
 * @param amplitude of right sensing pad
 */
void calc_distance(uint16_t left, uint16_t right, double *distance) {

	int index = 0;
	float average = ((float) left + (float) right) / 2 * correction;
	while (average < amplitudeLUT[index]
			&& index < (sizeof(distanceLUT) / 2 - 2)) {
		index++;
	}
	if ((amplitudeLUT[index] - average) < (average - amplitudeLUT[index + 1])) {
		*distance = distanceLUT[index];
	} else {
		*distance = distanceLUT[index + 1];
	}
	SD_calc = 1000 * (distanceLUT[index] - distanceLUT[index + 1]) * SD_raw
	/ (amplitudeLUT[index] - amplitudeLUT[index + 1]);

}

/**
 * @brief calculates angle based on calibration table.
 *
 * Calculates the ratio between the amplitudes of the left and right sensing pads. Searches for the corresponding angle in the calibration table. Saves angle in global variable.
 *
 * @param amplitude of left sensing pad
 * @param amplitude of right sensing pad
 */
void calc_angle(uint16_t left, uint16_t right, int *angle) {
	double factor = (double) left / (double) right;
	for (int i = 0; i < 4; i++) {
		if (factor < factorLUT[i]) {
			*angle = angleLUT[i];
			return;
		} else {
			*angle = angleLUT[4];
		}
	}
}

/**
 * @brief draws arrow based on angle and position on display
 *
 * @param direction of the arrow
 * @param x_pos of the center of the arrow in pixels
 * @param y_pos of the center of the arrow in pixels
 */
void draw_arrow(uint16_t direction, uint16_t x_pos, uint16_t y_pos) {

	arm_sin_cos_f32((float) direction, &sine, &cosine);

	for (int i = 0; i < 4; i++) {
		n_points[2 * i] = (uint16_t) (((float) points[2 * i] - 25) * cosine
				- ((float) points[2 * i + 1] - 50) * sine + 25);
		n_points[2 * i + 1] = (uint16_t) (((float) points[2 * i] - 25) * sine
				+ ((float) points[2 * i + 1] - 50) * cosine + 50);
	}
	BSP_LCD_DrawLine(x_pos + n_points[0], y_pos + n_points[1],
			x_pos + n_points[2], y_pos + n_points[3]);
	BSP_LCD_DrawLine(x_pos + n_points[0], y_pos + n_points[1],
			x_pos + n_points[4], y_pos + n_points[5]);
	BSP_LCD_DrawLine(x_pos + n_points[0], y_pos + n_points[1],
			x_pos + n_points[6], y_pos + n_points[7]);
}

/**
 * @brief measures the distance and displays it
 *
 * Initiates multiple measurements based on REPETITIONS. Calculates the average amplitudes of left and right sensing pad.
 * Calculates distance and angle of the mains cable. Displays distance and angle.
 */
void measure_distance() {
//	for (int i = 270; i < 360; i += 5){
//		draw_arrow(i, 40, 90);
//		HAL_Delay(200);
//	}

	uint32_t avg_left = 0;
	uint32_t avg_right = 0;
	double distance;
	int angle;
	for (int i = 0; i < REPETITIONS; i++) {
		ADC3_scan_init(13, 4);
		ADC3_scan_start();
		while (MEAS_data_ready == false)
			;
		MEAS_data_ready = false;
		MEAS_average(&avg_left, &avg_right);
	}
	avg_right /= REPETITIONS;
	avg_left /= REPETITIONS;

	calc_distance(avg_left, avg_right, &distance);
	calc_angle(avg_left, avg_right, &angle);

	for (int i = 0; i < (STDDEVSIZE - 1); i++) {
		buffer[i] = buffer[i + 1];
	}
	buffer[STDDEVSIZE - 1] = (avg_left + avg_right) / 2;
	SD_raw = calculateSD(&buffer[0], STDDEVSIZE);

	//display result
	//const uint32_t Y_OFFSET = 260;
	//const uint32_t X_SIZE = 240;
	//uint32_t data;
	//uint32_t data_last;
	/* Clear the display */
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(40, 15, 140, 140);
	//MENU_draw();
	/* Write first 2 samples as numbers */
	//BSP_LCD_SetFont(&Font24);
	//BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

	char text[16];
	if (buffer[0] != 0) {
		snprintf(text, 15, "STDEV: %4d count", SD_raw);
		BSP_LCD_DisplayStringAt(0, 140, (uint8_t*) text, LEFT_MODE);
		snprintf(text, 15, "STDEV: %4d um", SD_calc);
		BSP_LCD_DisplayStringAt(0, 160, (uint8_t*) text, LEFT_MODE);
	}
	snprintf(text, 15, "ANGLE: %4d", (int) angle);
	BSP_LCD_DisplayStringAt(0, 180, (uint8_t*) text, LEFT_MODE);
	snprintf(text, 15, "LEFT: %4d", (int) (avg_left));
	BSP_LCD_DisplayStringAt(0, 200, (uint8_t*) text, LEFT_MODE);
	snprintf(text, 15, "RIGHT: %4d", (int) (avg_right));
	BSP_LCD_DisplayStringAt(0, 220, (uint8_t*) text, LEFT_MODE);

	if (distance >= 100) {
		snprintf(text, 15, "Dist: >100 mm");
	} else {
		snprintf(text, 15, "Dist: %4d mm", (int) (distance));
	}
	BSP_LCD_DisplayStringAt(0, 240, (uint8_t*) text, LEFT_MODE);
	draw_arrow(360 + angle, /*((uint16_t)(BSP_LCD_GetXSize() / 2))*/90, 90);

}
