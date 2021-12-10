#include "distance.h"

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
		1915, 1785, 1700 };
uint16_t distanceLUT[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
float factorLUT[] = { 0.29, 0.7, 1, 1.1, 1.32 };
int angleLUT[] = { -90, -45, 0, 45, 90 };
static float distance;
static float angle;

void calc_distance(uint16_t left, uint16_t right) {
	int index = 0;
	float average = ((float) left + (float) right) / 2 * correction;
	while (average < amplitudeLUT[index]
			&& index < (sizeof(distanceLUT) / 2 - 2)) {
		index++;
	}
	if ((amplitudeLUT[index] - average) < (average - amplitudeLUT[index + 1])) {
		distance = distanceLUT[index];
	} else {
		distance = distanceLUT[index + 1];
	}
	//float weight = (average - distanceLUT[0][index]) / (distanceLUT[0][index + 1] - distanceLUT[0][index]);
	//distance = (weight * distanceLUT[1][index] + (1 - weight) * distanceLUT[1][index + 1]) / 2;
}

void calc_angle(uint16_t left, uint16_t right) {
	int index = 0;
	float factor = (float) left / (float) right;
	while (factor > factorLUT[index]
			&& index < ((sizeof(factorLUT) / sizeof(factorLUT[0])) - 2)) {
		index++;
	}
	if ((factorLUT[index] - factor) < (factor - factorLUT[index + 1])) {
		angle = angleLUT[index];
	} else {
		angle = angleLUT[index + 1];
	}
}

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

void measure_distance() {
//	for (int i = 270; i < 360; i += 5){
//		draw_arrow(i, 40, 90);
//		HAL_Delay(200);
//	}

	uint32_t avg_left = 0;
	uint32_t avg_right = 0;
	for (int i = 0; i < 10; i++) {
//		ADC3_IN13_IN4_scan_init();
//		ADC3_IN13_IN4_scan_start();
		ADC3_scan_init(13, 4);
		ADC3_scan_start();
		while (MEAS_data_ready == false);
		MEAS_data_ready = false;
		uint32_t *ADC_samples = get_ADC_samples();
		uint32_t left_max = 0;
		uint32_t right_max = 0;
		uint32_t left_min = 5000;
		uint32_t right_min = 5000;
		double left_value, right_value;
		int mov_avg = 4;
		for (int i = 2; i < (get_ADC_NUMS() - 2); i++) {
//			right_value = 0;
//			left_value = 0;
//			for (int j = 0; j <= mov_avg; j++){
//				right_value += ADC_samples[2*(j - mov_avg) + 1];
//				left_value += ADC_samples[2*(j - mov_avg)];
//			}
//
//			right_value /= (mov_avg * 2 + 1);
//			left_value /= (mov_avg * 2 + 1);
			left_value = (ADC_samples[2*(i-2)+1] + ADC_samples[2*(i-1)+1] + ADC_samples[2*i+1] + ADC_samples[2*(i+1)+1] + ADC_samples[2*(i+2)+1]) / 5;
			right_value = (ADC_samples[2*(i-2)] + ADC_samples[2*(i-1)] + ADC_samples[2*i] + ADC_samples[2*(i+1)] + ADC_samples[2*(i+2)]) / 5;

			if ((uint32_t)left_value > left_max) {
				left_max = (uint32_t)left_value;
			}
			if ((uint32_t)right_value > right_max) {
				right_max = (uint32_t)right_value;
			}
			if ((uint32_t)left_value < left_min) {
				left_min = (uint32_t)left_value;
			}
			if ((uint32_t)right_value < right_min) {
				right_min = (uint32_t)right_value;
			}

		}
		avg_left += (left_max - left_min);
		avg_right += (right_max - right_min);
	}
	avg_right /= 10;
	avg_left /= 10;

	calc_distance(avg_left, avg_right);
	calc_angle(avg_left, avg_right);

	//display result
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
	snprintf(text, 15, "ANGLE: %4d", (int) angle);
	BSP_LCD_DisplayStringAt(0, 200, (uint8_t*) text, LEFT_MODE);
	snprintf(text, 15, "LEFT: %4d", (int) (avg_left));
	BSP_LCD_DisplayStringAt(0, 220, (uint8_t*) text, LEFT_MODE);
	snprintf(text, 15, "RIGHT: %4d", (int) (avg_right));
	BSP_LCD_DisplayStringAt(0, 240, (uint8_t*) text, LEFT_MODE);

	if (distance >= 100) {
		snprintf(text, 15, "Dist: >100 mm");
	} else {
		snprintf(text, 15, "Dist: %4d mm", (int) (distance));
	}
	BSP_LCD_DisplayStringAt(0, 260, (uint8_t*) text, LEFT_MODE);
	draw_arrow(360 + angle, /*((uint16_t)(BSP_LCD_GetXSize() / 2))*/90, 90);

}
