/*
 * main.c
 *
 * Created on: Nov 12, 2020
 * Author: Mauricio Barroso Benavides
 */

/* inclusions ----------------------------------------------------------------*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wps.h"

#include "driver/gpio.h"

#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/lwip_napt.h"

#include "button.h"
#include "buzzer.h"
#include "wifi.h"
#include "ws2812_led.h"


/* macros --------------------------------------------------------------------*/

#define DNS_IP_ADDR	0x08080808	/* 8.8.8.8 */

/* typedef -------------------------------------------------------------------*/

/* data declaration ----------------------------------------------------------*/

/* Tag for debugging */
static const char * TAG = "app";

/* Components instances */
static wifi_t wifi;
static button_t button;
static buzzer_t buzzer;

/* Application variables */
static TaskHandle_t tryToReconnectHandle = NULL;
static uint8_t updateLed = 0;

/* function declaration ------------------------------------------------------*/

/* Utils*/
static esp_err_t nvsInit(void);
static esp_err_t beepSuccess(void);
static esp_err_t beepFail(void);
static esp_err_t beepError(void);

/* RTOS tasks */
static void buttonEventsTask(void * arg);
static void disconnectClientTask(void * arg);
static void tryToReconnectTask(void * arg);

/* Event handlers */
static void wifiEventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data);
static void ipEventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data);
static void provEventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data);

/* main ----------------------------------------------------------------------*/

void app_main(void)
{
	ESP_LOGI(TAG, "Initializing components instances");

    /* Initialize NVS */
	ESP_ERROR_CHECK(nvsInit());

	/* Initialize WS2812B LED */
	ESP_ERROR_CHECK(ws2812_led_init());
	ws2812_led_set_rgb(63, 0, 63);	/* Purple */

	/* Initialize Button instance */
	ESP_ERROR_CHECK(button_Init(&button));

	/* Initialize Buzzer instance */
	ESP_ERROR_CHECK(buzzer_Init(&buzzer));

	/* Initialize Wifi instance */
	wifi.wifiEventHandler = wifiEventHandler;
	wifi.ipEventHandler = ipEventHandler;
	wifi.provEventHandler = provEventHandler;
	ESP_ERROR_CHECK(wifi_Init(&wifi));

	/* Create RTOS tasks */
	ESP_LOGI(TAG, "Creating RTOS tasks");

	xTaskCreate(buttonEventsTask, "Button Events Task", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(disconnectClientTask, "Disconnect Client Task", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 1, NULL);
}

/* function definition -------------------------------------------------------*/

/* Utils */
static esp_err_t nvsInit(void) {
    esp_err_t ret = nvs_flash_init();

    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    return ret;
}

static esp_err_t beepSuccess(void) {
	esp_err_t ret = ESP_OK;

	ret = buzzer_Beep(&buzzer, 1, 200);

	if(ret != ESP_OK) {
		return ESP_FAIL;
	}

	ret = buzzer_Beep(&buzzer, 1, 50);

	return ret;
}

static esp_err_t beepFail(void) {
	esp_err_t ret = ESP_OK;

	ret = buzzer_Beep(&buzzer, 3, 50);

	return ret;
}

static esp_err_t beepError(void) {
	esp_err_t ret = ESP_OK;

	ret = buzzer_Beep(&buzzer, 4, 50);

	return ret;
}

/* RTOS tasks */
static void buttonEventsTask(void * arg) {
	EventBits_t bits;
	const EventBits_t bitsWaitFor = (BUTTON_SHORT_PRESS_BIT |
									 BUTTON_MEDIUM_PRESS_BIT |
									 BUTTON_LONG_PRESS_BIT);

	for(;;) {
		/* Wait until some bit is set */
		bits = xEventGroupWaitBits(button.eventGroup, bitsWaitFor, pdTRUE, pdFALSE, portMAX_DELAY);

		if(bits & BUTTON_SHORT_PRESS_BIT) {
			ESP_LOGI(TAG, "BUTTON_SHORT_PRESS_BIT set!");

			beepSuccess();

			esp_restart();
		}

		else if(bits & BUTTON_MEDIUM_PRESS_BIT) {
			ESP_LOGI(TAG, "BUTTON_MEDIUM_PRESS_BIT set!");

			/* Activate buzzer */
			beepError();

			/* Erase any stored Wi-Fi credential  */
			ESP_LOGI(TAG, "Erasing Wi-Fi credentials");

			esp_err_t ret;

			nvs_handle_t nvs_handle;
			ret = nvs_open("nvs.net80211", NVS_READWRITE, &nvs_handle);

			if(ret == ESP_OK)
				nvs_erase_all(nvs_handle);

			/* Close NVS */
			ret = nvs_commit(nvs_handle);
			nvs_close(nvs_handle);

			if(ret == ESP_OK)
				/* Restart device */
				esp_restart();
		}
		else if(bits & BUTTON_LONG_PRESS_BIT) {
			ESP_LOGI(TAG, "BUTTON_LONG_PRESS_BIT set!");
		}
		else {
			ESP_LOGI(TAG, "Button unexpected Event");
		}
	}
}

static void disconnectClientTask(void * arg) {
	wifi_sta_list_t sta;

	for(;;) {
		esp_wifi_ap_get_sta_list(&sta);

		/* Ask for stations RSSI and drop out if that is less than
		 * CONFIG_APP_RSSI_THRESHOLD_DROP_OUT */
		for(uint8_t i = 0; i < sta.num; i++) {
			ESP_LOGI(TAG, "station "MACSTR", RSSI: %d", MAC2STR (sta.sta[i].mac), sta.sta[i].rssi);

			if(sta.sta[i].rssi <= CONFIG_APP_RSSI_THRESHOLD_DROP_OUT) {
				uint16_t aid = 0;
				esp_wifi_ap_get_sta_aid(sta.sta[i].mac, &aid);
				ESP_LOGI(TAG, "Good bye" MACSTR, MAC2STR(sta.sta[i].mac));
				esp_wifi_deauth_sta(aid);
			}
		}

		/* Change LED color according the station list */
		if(updateLed) {
			if(sta.num < CONFIG_WIFI_AP_MAX_STA_CONN) {
				ws2812_led_set_rgb(0, 127, 0);	/* Green */
			}
			else if(sta.num == CONFIG_WIFI_AP_MAX_STA_CONN) {
				ws2812_led_set_rgb(82, 127, 0);	/* Orange */
			}
		}

		/* Wait 3 seg to update */
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

static void tryToReconnectTask(void * arg)
{
	TickType_t last_time_wake = xTaskGetTickCount();

	for(;;)
	{
		/* Try connecting to Wi-Fi router using stored credentials. If connection is successful
		 * then the task delete itself, in other cases this function is executed again*/
		ESP_LOGI(TAG, "Unable to connect. Retrying...");
		esp_wifi_connect();

		/* Wait 20 sec to try reconnecting */
		vTaskDelayUntil(&last_time_wake, pdMS_TO_TICKS(CONFIG_APP_RECONNECTION_TIME));
	}
}


/* Event handlers */
static void wifiEventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data) {
	switch(event_id) {
		case WIFI_EVENT_STA_CONNECTED: {
			if(tryToReconnectHandle != NULL) {
				vTaskDelete(tryToReconnectHandle);
				tryToReconnectHandle = NULL;
			}

			break;
		}

		case WIFI_EVENT_STA_DISCONNECTED: {
			/* Create task to reconnect to AP and set RGB led in blue color */
			if(tryToReconnectHandle == NULL) {
				xTaskCreate(tryToReconnectTask, "Try To Reconnect Task", configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 1, &tryToReconnectHandle);
			}

			updateLed = 0;

	        ws2812_led_set_rgb(127, 0, 0);	/* Red */

	        break;
		}

		case WIFI_EVENT_AP_STACONNECTED: {
			wifi_event_ap_staconnected_t * event = (wifi_event_ap_staconnected_t *)event_data;

			ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);

			/* If the station connected is not close to ESP32-S2, then is connection is rejected */
			wifi_sta_list_t sta;

			esp_wifi_ap_get_sta_list(&sta);

			for(uint8_t i = 0; i < sta.num; i++) {
				ESP_LOGI(TAG, "station "MACSTR", RSSI: %d", MAC2STR(sta.sta[i].mac), sta.sta[i].rssi);

					ESP_LOGI(TAG, "list[%d]: "MACSTR"", i, MAC2STR(sta.sta[i].mac));
					ESP_LOGI(TAG, "event: "MACSTR"", MAC2STR(event->mac));
				if(!strncmp((const char *)sta.sta[i].mac, (const char *)event->mac, 6)) {
					if(sta.sta[i].rssi <= CONFIG_APP_RSSI_THRESHOLD_JOIN) {
						ESP_LOGI(TAG, "RSSI less than RSSI threshold");
						esp_wifi_deauth_sta(event->aid);

						beepFail();
					}
					else {
						beepSuccess();
					}
				}
			}

			break;
		}

		default:
			ESP_LOGI(TAG, "Other event");
			break;
	}
}

static void ipEventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data) {
	switch(event_id) {
		case IP_EVENT_STA_GOT_IP: {
			ip_event_got_ip_t * event = (ip_event_got_ip_t *)event_data;

			ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));

			/* Initialize NAT */
			ip_napt_enable(htonl(0xC0A80401), 1);	/* 192.168.4.1 */
			ESP_LOGI (TAG, "NAT is enabled");

			ws2812_led_set_rgb(0, 127, 0);	/* Green */

			updateLed = 1;

			break;
		}
		default: {
			ESP_LOGI(TAG, "Other event");

			break;
		}
	}
}

static void provEventHandler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data) {
	switch(event_id) {
		case WIFI_PROV_START: {
			ESP_LOGI(TAG, "Provisioning started");

			ws2812_led_set_rgb(0, 0, 127);	/* Blue */

			break;
		}

		case WIFI_PROV_CRED_RECV: {
			wifi_sta_config_t * wifi_sta_cfg = (wifi_sta_config_t *)event_data;
			ESP_LOGI(TAG, "Credentials received, SSID: %s & Password: %s", (const char *) wifi_sta_cfg->ssid, (const char *) wifi_sta_cfg->password);

			break;
		}

		case WIFI_PROV_CRED_SUCCESS: {
			ESP_LOGI(TAG, "Provisioning successful");

			beepSuccess();

			break;
		}

		case WIFI_PROV_END: {
			/* De-initialize manager once provisioning is finished */
			wifi_prov_mgr_deinit();

			break;
		}

		case WIFI_PROV_CRED_FAIL: {
			beepFail();

			esp_restart();	/* Restart the device */

			break;
		}

		case WIFI_PROV_DEINIT: {
			char * ssid = CONFIG_WIFI_AP_SSID;
			ssid = malloc((strlen(CONFIG_WIFI_AP_SSID) + 7 + 1) * sizeof(* ssid));

			if(ssid == NULL) {
				ssid = CONFIG_WIFI_AP_SSID;
			}
			else {
				uint8_t eth_mac[6];
				esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
				sprintf(ssid, "%s_%02X%02X%02X", CONFIG_WIFI_AP_SSID, eth_mac[3], eth_mac[4], eth_mac[5]);
			}

			wifi_config_t wifi_config_ap = {
					.ap = {
							.ssid_len = strlen(ssid),
							.channel = CONFIG_WIFI_AP_CHANNEL,
							.password = CONFIG_WIFI_AP_PASS,
							.max_connection = CONFIG_WIFI_AP_MAX_STA_CONN,
							.authmode = WIFI_AUTH_OPEN
					},
			};
			strcpy((char *)wifi_config_ap.ap.ssid, ssid);

			ESP_ERROR_CHECK(esp_wifi_stop());
			ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
			ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
			ip_addr_t dnsserver;
			dnsserver.u_addr.ip4.addr = htonl(DNS_IP_ADDR);
			dhcps_offer_t dhcps_dns_value = OFFER_DNS;
			dhcps_set_option_info(6, &dhcps_dns_value, sizeof(dhcps_dns_value));
			dnsserver.type = IPADDR_TYPE_V4;
			dhcps_dns_setserver(&dnsserver);
			ESP_ERROR_CHECK(esp_wifi_start());
			ESP_ERROR_CHECK(esp_wifi_connect());

			free(ssid);

			break;
		}
		default: {
			ESP_LOGI(TAG, "Other event");

			break;
		}
	}
}

/* end of file ---------------------------------------------------------------*/
