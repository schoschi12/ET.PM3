/** ***************************************************************************
 * @file
 * @brief calculates distance and angle and displays them
 *
 * Contains the calibration table for distance and angle measurement.
 * Initiates measurement, calculates average and displays the values to the display.
 * 3 Modes are available: 1-Wire, 2-Wire, 3-Wire. Depending on the mode selected,
 * between 10 and 30 samples are averaged to stabilize the measurement.
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
uint16_t amplitudeLUT[3][11] = {
		{ 3780, 3713, 3527, 3125, 2815, 2560, 2250, 1955, 1915, 1785, 1700 },// LUT Amplitude in ADC counts (1-Wire)
		{ 3370, 3060, 2460, 1945, 1685, 1465, 1345, 1250, 1140, 1080, 1010 },// LUT Amplitude in ADC counts (2-Wire)
		{ 2690, 1863, 1495, 1295, 1200, 1115, 1055,  965,  890,  825,  775 } // LUT Amplitude in ADC counts (3-Wire)
		};
uint16_t repetitions[] = {10, 20, 30};uint16_t distanceLUT[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };// LUT distance in mm
double factorLUT[] = { 0.855, 0.955, 1.065, 1.17 };							// factor LUT calculated as left distance / right distance
int angleLUT[] = { 45, 22.5, 0, -22.5, -45 };								// angle LUT in °
uint32_t buffer[STDDEVSIZE] = { 0 }; 										// FIFO buffer for values used for std deviation calculation
uint32_t SD_raw; 															// raw ADC value of std. deviation
double SD_calc;																// calculated distance value of std. deviation in mm

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
void calc_distance(uint8_t table_index, uint16_t left, uint16_t right,
		double *distance) {

	int index = 0;
	float average = ((float) left + (float) right) / 2 * correction;
	while (average < amplitudeLUT[table_index][index]
			&& index < (sizeof(distanceLUT) / 2 - 2)) {						// find corresponding range of amplitude
		index++;
	}
	if ((amplitudeLUT[table_index][index] - average)
			< (average - amplitudeLUT[table_index][index + 1])) {
		*distance = distanceLUT[index];
	} else {
		*distance = distanceLUT[index + 1];
	}
	SD_calc = (double) SD_raw * (distanceLUT[index + 1] - distanceLUT[index])
			/ (amplitudeLUT[table_index][index]
					- amplitudeLUT[table_index][index + 1]);				// calculate std. deviation in mm

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
void measure_distance(uint8_t table_index) {

	uint32_t avg_left = 0;
	uint32_t avg_right = 0;
	double distance;
	int angle;
	for (int i = 0; i < repetitions[table_index]; i++) {
		ADC3_scan_init(13, 4);
		ADC3_scan_start();
		while (MEAS_data_ready == false)
			;
		MEAS_data_ready = false;
		MEAS_average(&avg_left, &avg_right);
	}
	avg_right /= repetitions[table_index];
	avg_left /= repetitions[table_index];

	calc_distance(table_index, avg_left, avg_right, &distance);
	calc_angle(avg_left, avg_right, &angle);

	for (int i = 0; i < (STDDEVSIZE - 1); i++) {
		buffer[i] = buffer[i + 1];
	}
	buffer[STDDEVSIZE - 1] = (avg_left + avg_right) / 2;
	SD_raw = calculateSD(&buffer[0], STDDEVSIZE);

	/* Clear the arrow */
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(75, 148, 100, 102);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

	char text[16];
	snprintf(text, 15, "Mode:%4d-Wire", (table_index + 1));
	BSP_LCD_DisplayStringAt(0, 190, (uint8_t*) text, LEFT_MODE);
	if (buffer[0] != 0) {
		snprintf(text, 15, "Std.dev:%.2lfmm", SD_calc);
		BSP_LCD_DisplayStringAt(0, 210, (uint8_t*) text, LEFT_MODE);
	}
	snprintf(text, 15, "Angle: %4ddeg", (int) angle);
	BSP_LCD_DisplayStringAt(0, 230, (uint8_t*) text, LEFT_MODE);
	if (distance >= 100) {
		snprintf(text, 15, "Dist: >100 mm");
	} else {
		snprintf(text, 15, "Dist: %4d mm", (int) (distance));
	}
	BSP_LCD_DisplayStringAt(0, 250, (uint8_t*) text, LEFT_MODE);
	draw_arrow(360 + angle, 90, 70);
}
