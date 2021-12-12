/** ***************************************************************************
 * @file
 * @brief See strommessung.c
 *
 *****************************************************************************/

#ifndef INC_STROMMESSUNG_H_
#define INC_STROMMESSUNG_H_
#define REPETITIONS 5

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdbool.h>
#include "measuring.h"

double measure_current_HAL(void);

#endif /* INC_STROMMESSUNG_H_ */
