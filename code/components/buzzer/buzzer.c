/*
 * buzzer.c
 *
 * Created on: Aug 13, 2021
 * Author: Mauricio Barroso Benavides
 */

/* inclusions ----------------------------------------------------------------*/

#include "include/buzzer.h"
#include "esp_log.h"

/* macros --------------------------------------------------------------------*/

#define BUZZER_UP_MS_DEFAULT	100
#define BUZZER_DOWN_MS_DEFAULT	100
#define BUZZER_TIMES_DEFAULT	0

/* typedef -------------------------------------------------------------------*/

/* internal data declaration -------------------------------------------------*/

static const char * TAG = "Buzzer";

/* external data declaration -------------------------------------------------*/

/* internal functions declaration --------------------------------------------*/

/* external functions definition ---------------------------------------------*/

esp_err_t buzzer_Init(buzzer_t * const me) {
	ESP_LOGI(TAG, "Initializing buzzer component...");

	esp_err_t ret = ESP_OK;

	me->gpio = CONFIG_BUZZER_PIN;
	me->high = BUZZER_UP_MS_DEFAULT;
	me->low = BUZZER_DOWN_MS_DEFAULT;
	me->times = BUZZER_TIMES_DEFAULT;

	/* Configure the buzzer GPIO pin */
	gpio_config_t gpio = {
			.pin_bit_mask = (1ULL << me->gpio),
			.mode = GPIO_MODE_OUTPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_DISABLE
	};

	ret = gpio_config(&gpio);

	return ret;
}

esp_err_t buzzer_Beep(buzzer_t * const me, uint8_t times, uint32_t duration) {
	esp_err_t ret = ESP_OK;

	/* Fill the data in the structure with the parameters */
	me->times = times;
	me->high = duration;

	/* Activate the buzzer the number of times that indicate the instance */
	for(uint8_t i = 0; i < me->times * 2; i++) {
		if(i % 2 == 0) {
			ret = gpio_set_level(me->gpio, 1);
			vTaskDelay(pdMS_TO_TICKS(me->high));
		}
		else {
			ret = gpio_set_level(me->gpio, 0);
			vTaskDelay(pdMS_TO_TICKS(me->low));
		}
	}

	return ret;
}

/* internal functions definition ---------------------------------------------*/

/* end of file ---------------------------------------------------------------*/
