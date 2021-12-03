#include "distance.h"

uint16_t points[] = { 25, 0, 25, 100, 0, 25, 50, 25 };
uint16_t n_points[8];
float sine;
float cosine;
int x_int;
int y_int;
float distanceLUT[10][10] = { { 3760, 3650, 3275, 2750, 2475, 2175, 2025, 1925,
		1800, 1575, 1450 }, { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 } };
static float distance;
static float direction;

void calc_distance(uint16_t left, uint16_t right) {
	int index = 0;
	float average = ((float) left + (float) right) / 2;
	while (average < distanceLUT[0][index]
			&& index < (sizeof(distanceLUT[0]) - 1)) {
		index++;
	}
	if ((distanceLUT[0][index] - average)
			< (average - distanceLUT[0][index + 1])) {
		distance = distanceLUT[1][index];
	} else {
		distance = distanceLUT[1][index + 1];
	}
	//float weight = (average - distanceLUT[0][index]) / (distanceLUT[0][index + 1] - distanceLUT[0][index]);
	//distance = (weight * distanceLUT[1][index] + (1 - weight) * distanceLUT[1][index + 1]) / 2;
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
//					ADC3_scan_init(13, 5);
//					ADC3_scan_start();
	uint32_t avg_left = 0;
	uint32_t avg_right = 0;
	for (int i = 0; i < 10; i++) {
		ADC3_IN13_IN4_scan_init();
		ADC3_IN13_IN4_scan_start();
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
	avg_right /= 10;
	avg_left /= 10;

	calc_distance(avg_left, avg_right);

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
	snprintf(text, 15, "LEFT: %4d", (int) (avg_left));
	BSP_LCD_DisplayStringAt(0, 50, (uint8_t*) text, LEFT_MODE);
	snprintf(text, 15, "RIGHT: %4d", (int) (avg_right));
	BSP_LCD_DisplayStringAt(0, 80, (uint8_t*) text, LEFT_MODE);

	snprintf(text, 15, "Dist: %4d", (int) (distance));
	BSP_LCD_DisplayStringAt(0, 100, (uint8_t*) text, LEFT_MODE);
	HAL_Delay(200);

}
