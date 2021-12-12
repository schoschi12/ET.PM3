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

/******************************************************************************
 * Defines
 *****************************************************************************/
extern bool MEAS_data_ready;
uint32_t MEAS_input_count;
extern bool DAC_active;

/******************************************************************************
 * Functions
 *****************************************************************************/

void MEAS_GPIO_analog_init(void);
void MEAS_timer_init(void);
void DAC_reset(void);
void DAC_init(void);
void DAC_increment(void);
void ADC_reset(void);
void ADC3_IN4_single_init(void);
void ADC3_IN4_single_read(void);
void ADC3_IN4_timer_init(void);
void ADC3_IN4_timer_start(void);
void ADC3_IN4_DMA_init(void);
void ADC3_IN4_DMA_start(void);
void ADC1_IN13_ADC2_IN5_dual_init(void);
void ADC1_IN13_ADC2_IN5_dual_start(void);
void ADC2_IN13_IN5_scan_init(void);
void ADC2_IN13_IN5_scan_start(void);
void ADC3_IN13_IN4_scan_init(void);
void ADC3_IN13_IN4_scan_start(void);

void ADC3_scan_init(unsigned long channel1, unsigned long channel2); //new modular
void ADC3_scan_start(void);
uint32_t* get_ADC_samples(void);
int get_ADC_NUMS(void);

void MEAS_show_data(void);
void MEAS_average(uint32_t *avg_left, uint32_t *avg_right);

void MEAS_show_data_current(double current);

#endif
