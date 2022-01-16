/*
 * buzzer.h
 *
 * Created on: Aug 13, 2021
 * Author: Mauricio Barroso Benavides
 */

#ifndef _BUZZER_H_
#define _BUZZER_H_

/* inclusions ----------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* macros --------------------------------------------------------------------*/

/* typedef -------------------------------------------------------------------*/

/**
 * @brief Buzzer parameters.
 */
typedef struct {
	uint8_t times;		/*!< Number of times the buzzer sounds */
	uint32_t high;		/*!< Time in ms the buzzer GPIO is in high level */
	uint32_t low;		/*!< Time in ms the buzzer GPIO is in low level */
	gpio_num_t gpio;	/*!< GPIO number for buzzer */
} buzzer_t;

/* external data declaration -------------------------------------------------*/

/* external functions declaration --------------------------------------------*/

/**
 * @brief Buzzer instance initialization function
 *
 * @param me Instance of the buzzer component
 *
 * @return
 *		- ESP_OK Success
 *		- ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t buzzer_Init(buzzer_t * const me);

/**
 * @brief Buzzer output sound
 *
 * @param me Instance of the buzzer component
 *
 * @param times Number of times the buzzer sounds
 *
 * @param duration Time in ms that the beep sounds
 *
 * @return
 *		- ESP_OK Success
 *		- ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t buzzer_Beep(buzzer_t * const me, uint8_t times, uint32_t duration);

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */

/* end of file ---------------------------------------------------------------*/

#endif /* #ifndef _BUZZER_H_ */
