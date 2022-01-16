/*
 * debounce.c
 *
 * Created on: Nov 16, 2020
 * Author: Mauricio Barroso Benavides
 */

/* inclusions ----------------------------------------------------------------*/

#include "debounce.h"

/* macros --------------------------------------------------------------------*/

/* typedef -------------------------------------------------------------------*/

/* internal data declaration -------------------------------------------------*/

/* external data declaration -------------------------------------------------*/

/* internal functions declaration --------------------------------------------*/

/* external functions definition ---------------------------------------------*/

void  debounce_init ( debounce_t * const me )
{

}

void debounce_exti ( debounce_t * const me )
{
	TickType_t elapsed_time = 0;

	switch ( me->debounce_state )
	{
		case DS_FALLING:
			if ( gpio_get_level ( me->gpio_pin ) == me->debounce_trigger )
			{
				me->tick_counter = xTaskGetTickCountFromISR ();
				me->debounce_state = DS_RISING;
			}
			break;
		case DS_RISING:
			if ( gpio_get_level ( me->gpio_pin ) == !me->debounce_trigger )
			{
				elapsed_time = xTaskGetTickCountFromISR () - me->tick_counter;

				if ( elapsed_time >= DEBOUNCE_TIME_SHORT && elapsed_time <= DEBOUNCE_TIME_MEDIUM
						&& me->function_short != NULL )
					me->function_short ( me->arg_short );
				else if ( ( elapsed_time >= DEBOUNCE_TIME_MEDIUM && elapsed_time <= DEBOUNCE_TIME_LONG )
						&& me->function_medium != NULL )
					me->function_medium ( me->arg_medium );
				else if ( elapsed_time >= DEBOUNCE_TIME_LONG
						&& me->function_long != NULL )
					me->function_long ( me->arg_long );
			}
			me->debounce_state = DS_FALLING;
			break;
		default:
			break;
	}
}

void debounce_switch ( debounce_t * const me )
{
	TickType_t elapsed_time = 0;

	switch ( me->debounce_state )
	{
		case DS_UP:
			if ( gpio_get_level ( me->gpio_pin ) == me->debounce_trigger )
				me->debounce_state = DS_FALLING;
			break;
		case DS_DOWN:
			if ( gpio_get_level ( me->gpio_pin ) == !me->debounce_trigger )
				me->debounce_state = DS_RISING;
			break;
		case DS_FALLING:
			if ( me->falling_counter >= DEBOUNCE_TIME_SHORT )
			{
				if ( gpio_get_level ( me->gpio_pin ) == me->debounce_trigger )
				{
					me->debounce_state = DS_DOWN;
					me->tick_counter = xTaskGetTickCount ();
				}
				else
					me->debounce_state = DS_UP;

				me->falling_counter = 0;
			}
			me->falling_counter++;
			break;
		case DS_RISING:
			if ( me->rising_counter >= DEBOUNCE_TIME_SHORT )
			{
				if ( gpio_get_level ( me->gpio_pin ) == !me->debounce_trigger )
				{
					me->debounce_state = DS_UP;

					elapsed_time = xTaskGetTickCount () - me->tick_counter;

					if ( elapsed_time >= DEBOUNCE_TIME_SHORT && elapsed_time <= DEBOUNCE_TIME_MEDIUM )
						me->function_short ( me->arg_short );
					else if ( elapsed_time >= DEBOUNCE_TIME_MEDIUM )
						me->function_medium ( me->arg_medium );
				}
				else
					me->debounce_state = DS_DOWN;

				me->rising_counter = 0;
			}
			me->rising_counter++;
			break;
		default:
			break;
	}
}

/* internal functions definition ---------------------------------------------*/

/* end of file ---------------------------------------------------------------*/
