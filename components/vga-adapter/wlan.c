#include "wlan.h"
#include <driver/gpio.h>

#include "globalvars.h"
#include "pins.h"
#include "osd.h"
#include "webserver.h"


static int s_retry_num = 0;
esp_netif_t* net_if = NULL;
uint8_t current_mode = 0; /* 0=Offline    1=WPS   2=STA   3=AP */

static wifi_config_t wifi_config =
{
	.sta = {
		.threshold.authmode = WIFI_AUTH_WPA2_PSK,
		.sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK,
		.sae_h2e_identifier = "",
	},
};
static esp_wps_config_t wps_config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);

const char* TextConnecting[Language_count] = {"Verbinde...","Connecting..."};
const char* TextDiscon[Language_count] = {"Getrennt","Disconnected"};
const char* TextWpsRetry[Language_count] = {"WPS wiederholen","WPS retry"};
const char* TextWpsError[Language_count] = {"WPS Fehler","WPS error"};
const char* TextWpsTimeout[Language_count] = {"WPS Zeit\x84""berlauf","WPS time out"};
const char* TextConnected[Language_count] = {"Verb:%d.%d.%d.%d","Conn:%d.%d.%d.%d"};

// wlan Handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	switch (event_id)
	{
		case WIFI_EVENT_STA_START:
	        printf("WIFI_EVENT_STA_START\n");
			if (wlan_mode==2)
			{
				esp_wifi_connect();
			}
			snprintf(wlan_state,21,TextConnecting[Language]);
			force_status_type(2);
			break;
		case WIFI_EVENT_STA_DISCONNECTED:
	        printf("WIFI_EVENT_STA_DISCONNECTED\n");
			gpio_set_level(PIN_NUM_LED_WIFI,0);
			stop_webserver();
			if (s_retry_num < 10) 
			{
				esp_wifi_connect();
				s_retry_num++;
				snprintf(wlan_state,21,TextConnecting[Language]);
			} 
			else 
			{
				snprintf(wlan_state,21,TextDiscon[Language]);
			}
			force_status_type(2);
			break;
        case WIFI_EVENT_STA_WPS_ER_SUCCESS:
	        printf("WIFI_EVENT_STA_WPS_ER_SUCCESS\n");
			wifi_event_sta_wps_er_success_t *evt = (wifi_event_sta_wps_er_success_t *)event_data;
			if (evt) 
			{
				memcpy(wlan_ssid, evt->ap_cred[0].ssid, 32);
				memcpy(wlan_passwd, evt->ap_cred[0].passphrase, 64);
				memcpy(wifi_config.sta.ssid,wlan_ssid,32);
				memcpy(wifi_config.sta.password,wlan_passwd,64);
				ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
			}
			else
			{
				ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifi_config));
				memcpy(wlan_ssid, wifi_config.sta.ssid, 32);
				memcpy(wlan_passwd, wifi_config.sta.password, 64);
			}
			current_mode = 2;
			wlan_mode = 2;
			ESP_ERROR_CHECK(esp_wifi_wps_disable());
			snprintf(wlan_state,21,TextConnecting[Language]);
			force_status_type(2);
			esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_WPS_ER_FAILED:
	        printf("WIFI_EVENT_STA_WPS_ER_FAILED\n");
			s_retry_num++;
            ESP_ERROR_CHECK(esp_wifi_wps_disable());
			if (s_retry_num <=5)
			{
				snprintf(wlan_state,21,TextWpsRetry[Language]);
				ESP_ERROR_CHECK(esp_wifi_wps_enable(&wps_config));
				ESP_ERROR_CHECK(esp_wifi_wps_start(0));
			}
			else
			{
				snprintf(wlan_state,21,TextWpsError[Language]);
				wlan_mode = 0;
			}
			force_status_type(2);
            break;
        case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
	        printf("WIFI_EVENT_STA_WPS_ER_TIMEOUT\n");
			s_retry_num++;
            ESP_ERROR_CHECK(esp_wifi_wps_disable());
			if (s_retry_num <=5)
			{
				snprintf(wlan_state,21,TextWpsRetry[Language]);
				ESP_ERROR_CHECK(esp_wifi_wps_enable(&wps_config));
				ESP_ERROR_CHECK(esp_wifi_wps_start(0));
			}
			else
			{
				wlan_mode = 0;
				snprintf(wlan_state,21,TextWpsTimeout[Language]);
			}
			force_status_type(2);
            break;
        default:
            break;			
	}
}

// Handler connect
static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == IP_EVENT_STA_GOT_IP) 
    {
		gpio_set_level(PIN_NUM_LED_WIFI,1);
		start_webserver();
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		snprintf(wlan_state,22,TextConnected[Language], IP2STR(&event->ip_info.ip));
		force_status_type(2);
        s_retry_num = 0;
        printf("IP_EVENT_STA_GOT_IP (%d.%d.%d.%d)\n", IP2STR(&event->ip_info.ip));
    }
}

// Wlan nach AP's scannen
void wifi_scan(uint16_t* ap_count, wifi_ap_record_t* ap_list)
{
	uint8_t old_mode = current_mode;
	setup_wlan(0);
	current_mode = 2;
    esp_netif_init();
    esp_event_loop_create_default();

	net_if = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    uint16_t number = 64;
    memset(ap_list, 0, sizeof(wifi_ap_record_t) * number);


    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    esp_wifi_scan_start(NULL, true);

    esp_wifi_scan_get_ap_num(ap_count);
    esp_wifi_scan_get_ap_records(&number, ap_list);
	setup_wlan(old_mode);
}

// wlan starten
void setup_wlan(uint8_t new_mode)
{
	if (net_if!=NULL)
	{
		esp_netif_destroy_default_wifi(net_if);
		esp_wifi_stop();
		esp_event_loop_delete_default();
		esp_wifi_deinit();
		net_if=NULL;
		force_status_type(2);
		gpio_set_level(PIN_NUM_LED_WIFI,0);
	}
	snprintf(wlan_state,22,TextDiscon[Language]);
	stop_webserver();

	current_mode = 0;
	s_retry_num = 0;
	if (new_mode==1)
	{
		esp_netif_init();
		esp_event_loop_create_default();
		net_if = esp_netif_create_default_wifi_sta();
		wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
		ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	
		ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
		ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
		ESP_ERROR_CHECK(esp_wifi_start());
		ESP_ERROR_CHECK(esp_wifi_wps_enable(&wps_config));
		ESP_ERROR_CHECK(esp_wifi_wps_start(0));
		current_mode = 1;
	}

	if (new_mode==2)
	{
		esp_netif_init();
		esp_event_loop_create_default();
		net_if = esp_netif_create_default_wifi_sta();
		wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
		ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	
		ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
		ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
		memcpy(wifi_config.sta.ssid,wlan_ssid,32);
		memcpy(wifi_config.sta.password,wlan_passwd,64);
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
		ESP_ERROR_CHECK(esp_wifi_start());
		current_mode = 2;
	}

	if (new_mode==3)
	{
		esp_netif_init();
		esp_event_loop_create_default();
		net_if = esp_netif_create_default_wifi_ap();	
		wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
		ESP_ERROR_CHECK(esp_wifi_init(&cfg));
		uint8_t baseMac[6];
		esp_wifi_get_mac(WIFI_IF_AP, baseMac);
		uint8_t c = 0;
		for (uint8_t i=0;i<6;i++) c=c ^ baseMac[i];
		snprintf((char*)wifi_config.ap.ssid,64,"ESP32-VGA-%X",c);
		memcpy(ap_ssid,wifi_config.ap.ssid,64);
		wifi_config.ap.max_connection = 2;
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
		ESP_ERROR_CHECK(esp_wifi_start());
		gpio_set_level(PIN_NUM_LED_WIFI,1);
		start_webserver();
		snprintf(wlan_state,22,"Hotspot 192.168.4.1");
		force_status_type(2);
		current_mode = 3;
	}
}
