
#include <esp_check.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <string>

#include <lwip/err.h>
#include <lwip/sys.h>

static const char *TAG = "WIFI";
static const char *TAG_STA = "WIFI STATION";
static const char *TAG_AP = "WIFI ACCESS POINT";

/*
    Wifi configurations
*/
#define ESP_WIFI_STA_SSID "Redmi"
#define ESP_WIFI_STA_PASS "maxlefou"
#define ESP_MAXIMUM_RETRY 10

#define ESP_WIFI_AP_SSID "ESP_WIFI"
#define ESP_WIFI_AP_PASS "12345678"
#define ESP_WIFI_AP_CHANNEL 1
#define MAX_STA_CONN 8

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

static int s_retry_num = 0;

static void wifi_sta_event_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_STA, "connect to the AP fail");
            ESP_LOGI(TAG_STA, "retry to connect to the AP");
        }
        else
        {
            esp_wifi_connect();
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_STA, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
}

static void wifi_ap_event_handler(void *arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event =
            (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG_AP, "station " MACSTR " join, AID=%d", MAC2STR(event->mac),
                 event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event =
            (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG_AP, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac),
                 event->aid);
    }
}

namespace sta
{
    esp_err_t init(void)
    {

        esp_netif_create_default_wifi_sta();

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_RETURN_ON_ERROR(
            esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                &wifi_sta_event_handler, NULL,
                                                &instance_any_id),
            TAG_STA,
            "failed to register the event wifi sta any id to the wifi event handler");
        ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(
                                IP_EVENT, IP_EVENT_STA_GOT_IP,
                                &wifi_sta_event_handler, NULL, &instance_got_ip),
                            TAG_STA,
                            "failed to register the event wifi sta got an ip to the "
                            "wifi event handler");

        wifi_config_t wifi_config = {
            .sta =
                {
                    .ssid = ESP_WIFI_STA_SSID,
                    .password = ESP_WIFI_STA_PASS,
                    .threshold = {.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD},
                    .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
                },
        };
        ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), TAG_STA,
                            "failed to set wifi sta configuration");

        ESP_LOGI(TAG_STA, "wifi init sta finished");
        return ESP_OK;
    }
} // namespace sta

namespace ap
{
    esp_err_t init(void)
    {

        esp_netif_create_default_wifi_ap();

        ESP_RETURN_ON_ERROR(
            esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                &wifi_ap_event_handler, NULL, NULL),
            TAG_AP,
            "failed to register the event wifi ap any id to the wifi event handler");

        wifi_config_t wifi_config = {
            .ap = {.ssid = ESP_WIFI_AP_SSID,
                   .password = ESP_WIFI_AP_PASS,
                   .ssid_len = strlen(ESP_WIFI_AP_SSID),
                   .channel = ESP_WIFI_AP_CHANNEL,
                   .authmode = WIFI_AUTH_WPA_WPA2_PSK,
                   .max_connection = MAX_STA_CONN,
                   .pmf_cfg =
                       {
                           .required = false,
                       }},
        };
        if (strlen(ESP_WIFI_AP_PASS) == 0)
        {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }

        ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_AP, &wifi_config), TAG_AP,
                            "failed to set wifi ap configuration");

        ESP_LOGI(TAG_AP, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
                 ESP_WIFI_AP_SSID, ESP_WIFI_AP_PASS, ESP_WIFI_AP_CHANNEL);
        return ESP_OK;
    }
} // namespace ap

namespace wifi
{

    esp_err_t init()
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG,
                            "failed to init the wifi configuration");
        esp_err_t errSta = sta::init();
        esp_err_t errAp = ap::init();
        ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_APSTA), TAG,
                            "Failed to set wifi mode");
        if (errAp != ESP_OK)
        {
            return errAp;
        }
        else if (errSta != ESP_OK)
        {
            return errSta;
        }
        return ESP_OK;
    }
    esp_err_t start()
    {
        ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG,
                            "Failed to start the wifi interface");
        return ESP_OK;
    }
} // namespace wifi
