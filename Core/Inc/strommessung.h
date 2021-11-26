/*
 * strommessung.h
 *
 *  Created on: 26.11.2021
 *      Author: Luca Andretto
 */

#ifndef INC_STROMMESSUNG_H_
#define INC_STROMMESSUNG_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdbool.h>
#include "measuring.h"

uint32_t measure_current_HAL_Left(void);
uint32_t measure_current_HAL_Right(void);
uint32_t measure_current_HAL(void);

#endif /* INC_STROMMESSUNG_H_ */
