/*
 * bitec_button.c
 *
 * Created on: Mar 29, 2021
 * Author: Mauricio Barroso Benavides
 */

/* inclusions ----------------------------------------------------------------*/

#include "include/button.h"
#include "esp_log.h"

/* macros --------------------------------------------------------------------*/

#define SHORT_TIME	pdMS_TO_TICKS(CONFIG_BUTTON_DEBOUNCE_SHORT_TIME)
#define MEDIUM_TIME	pdMS_TO_TICKS(CONFIG_BUTTON_DEBOUNCE_MEDIUM_TIME)
#define LONG_TIME	pdMS_TO_TICKS(CONFIG_BUTTON_DEBOUNCE_LONG_TIME)

/* typedef -------------------------------------------------------------------*/

/* internal data declaration -------------------------------------------------*/

static const char * TAG = "Button";

/* external data declaration -------------------------------------------------*/

/* internal functions declaration --------------------------------------------*/

static void isr_handler(void * arg);

/* external functions definition ---------------------------------------------*/

esp_err_t button_Init(button_t * const me) {
	ESP_LOGI(TAG, "Initializing button component...");

	esp_err_t ret;

	/* Create Wi-Fi event group */
	me->eventGroup = xEventGroupCreate();

	/* Initialize button GPIO */
	gpio_config_t gpio_conf;
	gpio_conf.pin_bit_mask = 1ULL << CONFIG_BUTTON_PIN;
	gpio_conf.mode = GPIO_MODE_INPUT;

	me->mode = FALLING_MODE;

	if(me->mode == FALLING_MODE) {
		gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
		gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
		me->state = FALLING_STATE;
	}
	else if(me->mode == RISING_MODE) {
		gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
		gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
		me->state = RISING_STATE;
	}

	gpio_conf.intr_type = GPIO_INTR_ANYEDGE;
	ret = gpio_config(&gpio_conf);

	if(ret != ESP_OK) {
		return ret;
	}

	/* Install ISR service and add ISR handler */
	gpio_install_isr_service(0);

	if(ret != ESP_OK) {
		return ret;
	}

	ret = gpio_isr_handler_add(CONFIG_BUTTON_PIN, isr_handler, (void *)me);

	if(ret != ESP_OK) {
		return ret;
	}

	/*  */
	me->pin = CONFIG_BUTTON_PIN;
	me->tick_counter = 0;

	return ESP_OK;
}

/* internal functions definition ---------------------------------------------*/

static void isr_handler(void * arg) {
	button_t * button = (button_t *)arg;

	TickType_t elapsed_time = 0;

	switch(button->state) {
		case FALLING_STATE:
			if(gpio_get_level(button->pin) == button->mode) {
				button->tick_counter = xTaskGetTickCountFromISR();
				button->state = RISING_STATE;
			}

			break;
		case RISING_STATE:
			if(gpio_get_level(button->pin ) == !button->mode) {
				elapsed_time = xTaskGetTickCountFromISR() - button->tick_counter;

				if(elapsed_time < MEDIUM_TIME) {
					xEventGroupSetBits(button->eventGroup, BUTTON_SHORT_PRESS_BIT);
				}
				else if((elapsed_time >= MEDIUM_TIME && elapsed_time < LONG_TIME)) {
					xEventGroupSetBits(button->eventGroup, BUTTON_MEDIUM_PRESS_BIT);
				}
				else if(elapsed_time >= LONG_TIME) {
					xEventGroupSetBits(button->eventGroup, BUTTON_LONG_PRESS_BIT);
				}

				elapsed_time = 0;
			}

			button->state = FALLING_STATE;

			break;

		default:
			break;
	}

	portYIELD_FROM_ISR();
}

/* end of file ---------------------------------------------------------------*/
