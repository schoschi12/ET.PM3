/** ***************************************************************************
 * @file
 * @brief See shutdown.c
 *
 *
 *****************************************************************************/
#ifndef INC_SHUTDOWN_H_
#define INC_SHUTDOWN_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "main.h"

/******************************************************************************
 * Functions
 *****************************************************************************/
void initiate_shutdown(void);
void prepare_shutdown(void);
void check_time(void);
void tim_TIM7_periodicConfig(uint32_t msPeriod);

#endif /* INC_SHUTDOWN_H_ */
