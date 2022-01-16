/*
 * bitec_wifi.h
 *
 * Created on: Mar 23, 2021
 * Author: Mauricio Barroso Benavides
 */

#ifndef _WIFI_H_
#define _WIFI_H_

/* inclusions ----------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_err.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* macros --------------------------------------------------------------------*/

/* Wi-Fi event bits */
#define WIFI_EVENT_WIFI_READY_BIT				BIT0	/*!< ESP32 WiFi ready */
#define WIFI_EVENT_SCAN_DONE_BIT				BIT1	/*!< ESP32 finish scanning AP */
#define WIFI_EVENT_STA_START_BIT				BIT2	/*!< ESP32 station start */
#define WIFI_EVENT_STA_STOP_BIT					BIT3	/*!< ESP32 station stop */
#define WIFI_EVENT_STA_CONNECTED_BIT			BIT4	/*!< ESP32 station connected to AP */
#define WIFI_EVENT_STA_DISCONNECTED_BIT			BIT5	/*!< ESP32 station disconnected from AP */
#define WIFI_EVENT_STA_AUTHMODE_CHANGE_BIT		BIT6	/*!< the auth mode of AP connected by ESP32 station changed */
#define WIFI_EVENT_STA_WPS_ER_SUCCESS_BIT		BIT7	/*!< ESP32 station wps succeeds in enrollee mode */
#define WIFI_EVENT_STA_WPS_ER_FAILED_BIT		BIT8	/*!< ESP32 station wps fails in enrollee mode */
#define WIFI_EVENT_STA_WPS_ER_TIMEOUT_BIT		BIT9	/*!< ESP32 station wps timeout in enrollee mode */
#define WIFI_EVENT_STA_WPS_ER_PIN_BIT			BIT10	/*!< ESP32 station wps pin code in enrollee mode */
#define WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP_BIT	BIT11	/*!< ESP32 station wps overlap in enrollee mode */

#define WIFI_EVENT_AP_START_BIT					BIT12	/*!< ESP32 soft-AP start */
#define WIFI_EVENT_AP_STOP_BIT					BIT13	/*!< ESP32 soft-AP stop */
#define WIFI_EVENT_AP_STACONNECTED_BIT			BIT14	/*!< a station connected to ESP32 soft-AP */
#define WIFI_EVENT_AP_STADISCONNECTED_BIT		BIT15	/*!< a station disconnected from ESP32 soft-AP */
#define WIFI_EVENT_AP_PROBEREQRECVED_BIT		BIT16	/*!< Receive probe request packet in soft-AP interface */

/* Wi-Fi provisioning event bits */
#define WIFI_PROV_INIT_BIT						BIT0	/*!< Emitted when the manager is initialized */
#define WIFI_PROV_START_BIT						BIT1	/*!< Indicates that provisioning has started */
#define WIFI_PROV_CRED_RECV_BIT					BIT2	/*!< Emitted when Wi-Fi AP credentials are received via `protocomm
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * endpoint `wifi_config`. The event data in this case is a pointer
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * to the corresponding `wifi_sta_config_t` structure */
#define	WIFI_PROV_CRED_FAIL_BIT					BIT3	/*!< Emitted when device fails to connect to the AP of which the
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * credentials were received earlier on event `WIFI_PROV_CRED_RECV`.
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * The event data in this case is a pointer to the disconnection
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * reason code with type `wifi_prov_sta_fail_reason_t */
#define WIFI_PROV_CRED_SUCCESS_BIT				BIT4	/*!< Emitted when device successfully connects to the AP of which the
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * credentials were received earlier on event `WIFI_PROV_CRED_RECV */
#define	WIFI_PROV_END_BIT						BIT5	/*!< Signals that provisioning service has stopped */
#define WIFI_PROV_DEINIT_BIT					BIT6	/*!< Signals that manager has been de-initialized */

/* IP event bits */
#define IP_EVENT_STA_GOT_IP_BIT					BIT0	/*!< station got IP from connected AP */
#define IP_EVENT_STA_LOST_IP_BIT				BIT1	/*!< station lost IP and the IP is reset to 0 */
#define IP_EVENT_AP_STAIPASSIGNED_BIT			BIT2	/*!< soft-AP assign an IP to a connected station */
#define IP_EVENT_GOT_IP6_BIT					BIT3	/*!< station or ap or ethernet interface v6IP addr is preferred */
#define IP_EVENT_ETH_GOT_IP_BIT					BIT4	/*!< ethernet got IP from connected AP */
#define IP_EVENT_PPP_GOT_IP_BIT					BIT5	/*!< PPP interface got IP */
#define IP_EVENT_PPP_LOST_IP_BIT				BIT6	/*!< PPP interface lost IP */

/* typedef -------------------------------------------------------------------*/

typedef void (* wifiEventHandler_t)(void *, esp_event_base_t, int32_t, void *);
typedef void (* ipEventHandler_t)(void *, esp_event_base_t, int32_t, void *);
typedef void (* provEventHandler_ts)(void *, esp_event_base_t, int32_t, void *);
typedef struct
{
	wifiEventHandler_t wifiEventHandler;
	ipEventHandler_t ipEventHandler;
	provEventHandler_ts provEventHandler;
	void * wifiEventData;
	void * ipEventData;
	void * provEventData;
} wifi_t; /* todo: write descriptions */

/* external data declaration -------------------------------------------------*/


/* external functions declaration --------------------------------------------*/

esp_err_t wifi_Init(wifi_t * const me);

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */

/* end of file ---------------------------------------------------------------*/

#endif /* #ifndef _WIFI_H_ */
