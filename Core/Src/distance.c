#include "distance.h"

uint16_t points[] = {25, 0, 25, 100, 0, 25, 50, 25};
uint16_t n_points[8];
//uint16_t x_top = 0;
//uint16_t y_top = -50;
//uint16_t x_bot = 0;
//uint16_t y_bot = 50;
//uint16_t x_left = -25;
//uint16_t y_left = -25;
//uint16_t x_right = 25;
//uint16_t y_right = -25;
float sine;
float cosine;
int x_int;
int y_int;


void draw_arrow(uint16_t direction, uint16_t x_pos, uint16_t y_pos){
	arm_sin_cos_f32((float)direction, &sine, &cosine);

	for (int i = 0; i < 4; i ++){
		n_points[2*i] = (uint16_t)(((float)points[2*i] - 25) * cosine - ((float)points[2*i+1] - 50) * sine + 25);
		n_points[2*i+1] = (uint16_t)(((float)points[2*i] - 25) * sine + ((float)points[2*i+1] - 50) *cosine + 50);
	}
	BSP_LCD_DrawLine(x_pos + n_points[0], y_pos + n_points[1], x_pos + n_points[2], y_pos + n_points[3]);
	BSP_LCD_DrawLine(x_pos + n_points[0], y_pos + n_points[1], x_pos + n_points[4], y_pos + n_points[5]);
	BSP_LCD_DrawLine(x_pos + n_points[0], y_pos + n_points[1], x_pos + n_points[6], y_pos + n_points[7]);
}
