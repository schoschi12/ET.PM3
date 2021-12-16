/** ***************************************************************************
 * @file
 * @brief See measuring.c
 *
 * Prefixes MEAS, ADC, DAC
 *
 *****************************************************************************/

#ifndef MEAS_H_
#define MEAS_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/******************************************************************************
 * Defines
 *****************************************************************************/
extern bool MEAS_data_ready;
uint32_t MEAS_input_count;

/******************************************************************************
 * Functions
 *****************************************************************************/

void MEAS_GPIO_analog_init(void);
void MEAS_timer_init(void);
void ADC_reset(void);

void ADC3_scan_init(unsigned long channel1, unsigned long channel2); //new modular
void ADC3_scan_start(void);
uint32_t* get_ADC_samples(void);
int get_ADC_NUMS(void);

void MEAS_show_data(void);
void MEAS_average(uint32_t *avg_left, uint32_t *avg_right);
double calculateSD(uint32_t *buffer, uint8_t repetitions);

void MEAS_show_data_current(double current);

#endif
