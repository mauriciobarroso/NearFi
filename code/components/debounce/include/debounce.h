/*
 * beep.h
 *
 * Created on: Oct 6, 2020
 * Author: Mauricio Barroso Benavides
 */

#ifndef _DEBOUNCE_H_
#define _DEBOUNCE_H_

/* inclusions ----------------------------------------------------------------*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* macros --------------------------------------------------------------------*/

#define DEBOUNCE_TIME_SHORT		30		/*<! Debounce time in ms */
#define DEBOUNCE_TIME_MEDIUM	2000	/*<! Debounce time in ms */
#define DEBOUNCE_TIME_LONG		6000	/*<! Debounce time in ms */
#define DEBOUNCE_POLLING_TIME	10		/*<! Polling time in ms */

/* typedef -------------------------------------------------------------------*/

typedef enum
{
	DS_UP,
	DS_DOWN,
	DS_FALLING,
	DS_RISING,
} debounce_state_e;

typedef struct
{
	debounce_state_e debounce_state;
	uint32_t debounce_trigger;
	gpio_num_t gpio_pin;
	TickType_t tick_counter;
	uint8_t falling_counter;
	uint8_t rising_counter;
	void ( * function_short ) ( void * );
	void ( * function_medium ) ( void * );
	void ( * function_long ) ( void * );
	void * arg_short;
	void * arg_medium;
	void * arg_long;
} debounce_t;

/* external data declaration -------------------------------------------------*/

/* external functions declaration --------------------------------------------*/

void debounce_init ( debounce_t * const me );
void debounce_exti ( debounce_t * const me );
void debounce_switch ( debounce_t * const me );

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */

/* end of file ---------------------------------------------------------------*/

#endif /* #ifndef _DEBOUNCE_H_ */
