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

void measure_current_HAL(uint32_t *current_left, uint32_t *current_right);

#endif /* INC_STROMMESSUNG_H_ */
