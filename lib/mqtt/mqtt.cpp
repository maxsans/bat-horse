#include "mqtt.hpp"
#include "../../include/globals.hpp"

#include <map>
#include <esp_log.h>
#include <mqtt_client.h>
#include <esp_log.h>
#include <esp_check.h>

static const char *TAG = "MQTT";

#define BROKER_URL MQTT_BROKER_URL
#define QOS 2

bool isConnectedToBroker = false;

std::map<std::string, std::function<void(std::string, std::string)>> mapCallbackTopic;

std::vector<std::string> notSubscribeTopic;

esp_mqtt_client_handle_t client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    std::string topic(event->topic, event->topic + event->topic_len);
    std::string data(event->data, event->data + event->data_len);
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        isConnectedToBroker = true;
        if (!notSubscribeTopic.empty())
        {
            for (auto subTopic : notSubscribeTopic)
            {
                if (esp_mqtt_client_subscribe(client, subTopic.c_str(), QOS) != -1)
                {
                    ESP_LOGI(TAG, "success to subscribe to : %s", subTopic.c_str());
                }
                else
                {
                    ESP_LOGI(TAG, "failed to subscribe to : %s", subTopic.c_str());
                }
            }
        }

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        isConnectedToBroker = false;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");

        mapCallbackTopic[topic](topic, data);
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

namespace mqtt
{
    esp_err_t init()
    {
        esp_log_level_set("*", ESP_LOG_INFO);
        esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
        esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
        esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
        esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
        esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
        esp_log_level_set("outbox", ESP_LOG_VERBOSE);
        return ESP_OK;
    }

    esp_err_t start()
    {
        esp_mqtt_client_config_t mqtt_cfg = {
            .broker = {.address = {.uri = BROKER_URL, .port = 1883}}, .task = {.priority = 3}};

        client = esp_mqtt_client_init(&mqtt_cfg);
        /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
        ESP_RETURN_ON_ERROR(esp_mqtt_client_register_event(client, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID), mqtt_event_handler, NULL), TAG, "failed to register mqtt event");
        ESP_RETURN_ON_ERROR(esp_mqtt_client_start(client), TAG, "Failed to start mqtt client");
        return ESP_OK;
    }

    esp_err_t subscribe(std::string &topic, std::function<void(std::string, std::string)> callbackFunction)
    {
        if (!isConnectedToBroker)
        {
            notSubscribeTopic.push_back(topic);
            mapCallbackTopic[topic] = callbackFunction;
        }
        else
        {
            if (esp_mqtt_client_subscribe(client, topic.c_str(), QOS) != -1)
            {
                mapCallbackTopic[topic] = callbackFunction;
                ESP_LOGI(TAG, "success to subscribe to : %s", topic.c_str());
                return ESP_OK;
            }
            else
            {
                ESP_LOGI(TAG, "failed to subscribe to : %s", topic.c_str());
                return ESP_FAIL;
            }
        }
        return ESP_OK;
    }

    esp_err_t publish(std::string &topic, std::string &data)
    {
        if (esp_mqtt_client_enqueue(client, topic.c_str(), data.c_str(), data.size(), QOS, false, true) != -1)
        {
            ESP_LOGI(TAG, "success to publish to : %s", topic.c_str());
            return ESP_OK;
        }
        else
        {
            ESP_LOGI(TAG, "failed to publish to : %s", topic.c_str());
            return ESP_FAIL;
        }

        return ESP_OK;
    }
} // namespace mqtt
