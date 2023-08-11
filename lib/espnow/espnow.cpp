#include "espnow.hpp"
#include "../../include/globals.hpp"

#include <esp_err.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_now.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <vector>
#include <functional>
#include <string.h>
#include <map>
#include <cJSON.h>
#include <string>

static const char *TAG = "ESP_NOW";

typedef enum
{
    ESP_NOW_EVT_RECEIVE_DATA,
    ESP_NOW_EVT_SEND_DATA
} espNowEventId_e;

typedef enum
{
    ESP_NOW_QUERY_SOMEONE,
    ESP_NOW_QUERY_CONNECT_TO_THE_BASE,
    ESP_NOW_QUERY_START_MEASUREMENT,
    ESP_NOW_QUERY_STOP_MEASUREMENT,
} espNowQuery_e;

typedef enum
{
    ESP_NOW_RESPONSE_OK,
    ESP_NOW_RESPONSE_FAIL
} espNowResponse_e;

typedef enum
{
    ESP_NOW_TYPE_DATA,
    ESP_NOW_TYPE_QUERY,
    ESP_NOW_TYPE_RESPONSE
} espNowType_e;

typedef struct
{
    espNowEventId_e id;
    espNowType_e type;
    uint8_t *data;
    uint8_t dataLength;
    uint8_t *senderAddr;
} espNowEvent_t;

bool readyToSend = false;

bool isRecording = false;

static std::vector<espNow::SensorData> sensorList;

std::vector<uint8_t> base;

static std::vector<uint8_t> broadcastAddress = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static std::function<void(std::string, std::string)> callbackFunctionOnReceiveData;

std::function<void(std::vector<espNow::SensorData>)> callbackConnectedSensor;

QueueHandle_t receiveQueue = xQueueCreate(50, sizeof(espNowEvent_t));

static void callbackOnReceiveData(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len)
{
    espNowEvent_t evt;
    uint8_t *mac_addr = esp_now_info->src_addr;

    if (mac_addr == NULL || data == NULL || data_len <= 0)
    {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    evt.id = ESP_NOW_EVT_RECEIVE_DATA;
    evt.senderAddr = esp_now_info->src_addr; // ESP_NOW_ETH_ALEN

    evt.type = static_cast<espNowType_e>(data[0]);

    uint8_t dataTab[data_len - 1];
    for (uint8_t i = 1; i < data_len; i++)
    {
        dataTab[i - 1] = data[i];
    }

    evt.data = dataTab;
    evt.dataLength = data_len - 1;
    if (xQueueSend(receiveQueue, &evt, 1000 / portTICK_PERIOD_MS) != pdTRUE)
    {
        ESP_LOGW(TAG, "Send receive queue fail");
    }
}

static void callbackOnSendData(const uint8_t *mac_addr, esp_now_send_status_t status)
{
}

esp_err_t espNowPeer(std::vector<uint8_t> address)
{
    if (esp_now_is_peer_exist(address.data()))
    {
        return ESP_OK;
    }
    if (address.size() != ESP_NOW_ETH_ALEN)
    {
        ESP_LOGE(TAG, "espNowPeer failed with address length");
        return ESP_FAIL;
    }
    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = static_cast<esp_now_peer_info_t *>(malloc(sizeof(esp_now_peer_info_t)));
    if (peer == NULL)
    {
        ESP_LOGE(TAG, "Malloc peer information fail");
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = WIFI_PEER_CHANNEL;
#if IS_BASE
    peer->ifidx = static_cast<wifi_interface_t>(ESP_IF_WIFI_AP);
#else
    peer->ifidx = static_cast<wifi_interface_t>(ESP_IF_WIFI_STA);
#endif
    peer->encrypt = false;
    uint8_t *add = address.data();
    memcpy(peer->peer_addr, add, ESP_NOW_ETH_ALEN);
    esp_err_t err = esp_now_add_peer(peer);
    free(peer);
    ESP_RETURN_ON_ERROR(err, TAG, "failed to add peer address");
    ESP_LOGI(TAG, "success to peer address %X:%X:%X:%X:%X:%X", add[0], add[1], add[2], add[3], add[4], add[5]);
    return ESP_OK;
}

static bool isNameAlreadyExist(const std::string &nameToFind)
{
    for (const auto &sensorData : sensorList)
    {
        if (sensorData.sensorName == nameToFind)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief send data to base
 *
 * @param type type of data to send
 * @param DestinationAddr destination address
 * @param data data to send
 * @return esp_err_t ESP_OK if successful, ESP_ERR otherwise
 */
static esp_err_t sendData(espNowType_e type, std::vector<uint8_t> &DestinationAddr, std::vector<uint8_t> &data)
{
    std::vector<uint8_t> dataToTransmit;
    dataToTransmit.push_back(type);
    dataToTransmit.insert(dataToTransmit.end(), data.begin(), data.end());
    esp_err_t err = esp_now_send(DestinationAddr.data(), dataToTransmit.data(), dataToTransmit.size());
    return err;
}

static esp_err_t connectToTheBaseRequest(std::vector<uint8_t> &srcAddr, std::string &data, uint8_t query)
{
    ESP_LOGI(TAG, "received a connection request from %X:%X:%X:%X:%X:%X", srcAddr[0], srcAddr[1], srcAddr[2], srcAddr[3], srcAddr[4], srcAddr[5]);
    esp_err_t ret = espNowPeer(srcAddr);

    cJSON *jsonObject = cJSON_Parse(data.c_str());
    if (jsonObject == nullptr)
    {
        ESP_LOGE(TAG, "Error when parsing JSON");
        return ESP_FAIL;
    }

    // Extraire la valeur de l'objet "sensorName"
    cJSON *sensorNameObject = cJSON_GetObjectItem(jsonObject, "sensorName");
    if (sensorNameObject == nullptr || !cJSON_IsString(sensorNameObject))
    {
        ESP_LOGE(TAG, "Error when parsing JSON object : sensorName");
        cJSON_Delete(jsonObject);
        return ESP_FAIL;
    }

    // Récupérer la valeur de l'objet "sensorName"
    std::string sensorNameValue = sensorNameObject->valuestring;

    cJSON_Delete(jsonObject);

    // send a response to the base
    std::vector<uint8_t> response;
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error to pair the sensor");
        response.push_back(query);
        response.push_back(ESP_NOW_RESPONSE_FAIL);
    }
    else if (isNameAlreadyExist(sensorNameValue))
    {
        for (const auto &sensorData : sensorList)
        {
            if (sensorData.sensorName == sensorNameValue)
            {
                if (sensorData.sensorAddress == srcAddr)
                {
                    ESP_LOGI(TAG, "already added sensor name : %s", sensorNameValue.c_str());
                    response.push_back(query);
                    response.push_back(ESP_NOW_RESPONSE_OK);
                }
                else
                {
                    ESP_LOGE(TAG, "Error the sensor name already exist");
                    response.push_back(query);
                    response.push_back(ESP_NOW_RESPONSE_FAIL);
                }
            }
        }
    }
    else
    {
        ESP_LOGI(TAG, "added a new sensor name : %s", sensorNameValue.c_str());
        espNow::SensorData sensor = {.sensorId = static_cast<int>(sensorList.size()), .sensorName = sensorNameValue, .sensorAddress = srcAddr};
        sensorList.push_back(sensor);
        callbackConnectedSensor(sensorList);
        response.push_back(query);
        response.push_back(ESP_NOW_RESPONSE_OK);
    }
    sendData(ESP_NOW_TYPE_RESPONSE, srcAddr, response);
    return ESP_OK;
}

esp_err_t sendDataToTheBase(std::vector<uint8_t> data)
{
    if (readyToSend)
    {
        return sendData(ESP_NOW_TYPE_DATA, base, data);
    }
    return ESP_FAIL;
}

static void receiveTask(void *pvParameter)
{
    espNowEvent_t evt;
    for (;;)
    {
        if (xQueueReceive(receiveQueue, &evt, portMAX_DELAY) == pdTRUE)
        {

            // printf("Received add :");
            // for (int i = 0; i < 6; i++)
            // {
            //     printf(" %x ", evt.senderAddr[i]);
            // }
            // printf("\n");
            // printf("Received data :");
            // for (int i = 0; i < evt.dataLength; i++)
            // {
            //     printf(" %x ", evt.data[i]);
            // }
            // printf("\n");
            std::vector<uint8_t> srcAddr(evt.senderAddr, evt.senderAddr + 6);
            std::vector<uint8_t> receiveData(evt.data, evt.data + evt.dataLength);
            switch (evt.id)
            {
            case ESP_NOW_EVT_RECEIVE_DATA:
            {
                switch (evt.type)
                {
                case ESP_NOW_TYPE_DATA:
                {
                    std::string data(receiveData.begin(), receiveData.end());
                    for (const auto &sensorData : sensorList)
                    {
                        if (sensorData.sensorAddress == srcAddr && isRecording)
                        {
                            callbackFunctionOnReceiveData(sensorData.sensorName, data);
                        }
                    }
                }
                break;

                case ESP_NOW_TYPE_QUERY:
                {
                    if (receiveData[0] == ESP_NOW_QUERY_SOMEONE)
                    {
                        ESP_LOGI(TAG, "received a someone request from %X:%X:%X:%X:%X:%X", srcAddr[0], srcAddr[1], srcAddr[2], srcAddr[3], srcAddr[4], srcAddr[5]);
                        esp_err_t ret = espNowPeer(srcAddr);
                        std::vector<uint8_t> response;
                        response.push_back(receiveData[0]);
                        response.push_back(ESP_NOW_RESPONSE_OK);
                        sendData(ESP_NOW_TYPE_RESPONSE, srcAddr, response);
                    }
#if IS_BASE
                    if (receiveData[0] == ESP_NOW_QUERY_CONNECT_TO_THE_BASE)
                    {
                        uint8_t query = receiveData[0];
                        receiveData.erase(receiveData.begin());
                        std::string dataString(receiveData.begin(), receiveData.end());
                        connectToTheBaseRequest(srcAddr, dataString, query);
                    }
#else
                    if (receiveData[0] == ESP_NOW_QUERY_START_MEASUREMENT)
                    {
                        isRecording = true;
                        ESP_LOGI(TAG, "start measurement");
                        uint8_t query = receiveData[0];
                        receiveData.erase(receiveData.begin());
                        std::string dataString(receiveData.begin(), receiveData.end());
                        connectToTheBaseRequest(srcAddr, dataString, query);
                    }
                    if (receiveData[0] == ESP_NOW_QUERY_STOP_MEASUREMENT)
                    {
                        isRecording = false;
                        ESP_LOGI(TAG, "stop measurement");
                        uint8_t query = receiveData[0];
                        receiveData.erase(receiveData.begin());
                        std::string dataString(receiveData.begin(), receiveData.end());
                        connectToTheBaseRequest(srcAddr, dataString, query);
                    }
#endif
                }
                break;

                case ESP_NOW_TYPE_RESPONSE:
                {
#if !IS_BASE
                    if (evt.data[0] == ESP_NOW_QUERY_CONNECT_TO_THE_BASE)
                    {
                        if (evt.data[1] == ESP_NOW_RESPONSE_OK)
                        {
                            ESP_LOGI(TAG, "received response ok for the connection request from %X:%X:%X:%X:%X:%X", srcAddr[0], srcAddr[1], srcAddr[2], srcAddr[3], srcAddr[4], srcAddr[5]);
                            esp_err_t ret = espNowPeer(srcAddr);
                            if (ret == ESP_OK)
                            {
                                readyToSend = true;
                                base = srcAddr;
                                ESP_LOGI(TAG, "Start sending data to the base");
                            }
                        }
                    }
                    if (evt.data[0] == ESP_NOW_QUERY_SOMEONE)
                    {
                        if (evt.data[1] == ESP_NOW_RESPONSE_OK)
                        {
                            // updateLastDataTime[evt.senderAddr];
                        }
                    }
#endif
                }
                break;

                default:
                    ESP_LOGI(TAG, "Unknown type");
                    break;
                }
            }
            break;
            case ESP_NOW_EVT_SEND_DATA:
            {
            }
            break;
            default:
                ESP_LOGI(TAG, "Unknown event");
                break;
            }
        }
    }
    vTaskDelete(NULL);
}

// Cast char* to vector of uint8_t
static std::vector<uint8_t> charToUint8Vector(const char *data)
{
    size_t dataSize = strlen(data);
    std::vector<uint8_t> result(dataSize);
    for (size_t i = 0; i < dataSize; ++i)
    {
        result[i] = static_cast<uint8_t>(data[i]);
    }
    return result;
}

static void connectBaseTask(void *pvParameter)
{
    for (;;)
    {
        if (!readyToSend)
        {
            cJSON *jsonObject = cJSON_CreateObject();
            cJSON_AddStringToObject(jsonObject, "sensorName", SENSOR_NAME);
            char *jsonChar = cJSON_Print(jsonObject);
            std::vector<uint8_t> jsonBuf = charToUint8Vector(jsonChar);
            free(jsonChar);
            std::vector<uint8_t> buffer = {ESP_NOW_QUERY_CONNECT_TO_THE_BASE};
            buffer.insert(buffer.end(), jsonBuf.begin(), jsonBuf.end());

            esp_err_t err = sendData(ESP_NOW_TYPE_QUERY, broadcastAddress, buffer);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "failed to send the pair request");
            }
            cJSON_Delete(jsonObject);
        }
        vTaskDelay(1000 * 4 / portTICK_PERIOD_MS);
    }
}

namespace espNow
{
    esp_err_t init(std::function<void(std::string, std::string)> callbackFunction, std::function<void(std::vector<SensorData>)> callbackFunctionConnectedSensor)
    {
        callbackConnectedSensor = callbackFunctionConnectedSensor;
        callbackFunctionOnReceiveData = callbackFunction;
        ESP_RETURN_ON_ERROR(esp_now_init(), TAG, "failed to initialize esp now");
        ESP_RETURN_ON_ERROR(esp_now_register_recv_cb(callbackOnReceiveData), TAG, "failed to register receive callback");
        ESP_RETURN_ON_ERROR(esp_now_register_send_cb(callbackOnSendData), TAG, "failed to register receive callback");

        xTaskCreate(receiveTask, "esp_now_receive_task", 4048, NULL, 3, NULL);
        return ESP_OK;
    }

    /**
     * @brief     Start the esp now communication
     *
     * @return
     *          - true : peer exists
     *          - false : peer not exists
     */
    esp_err_t start()
    {
        espNowPeer(broadcastAddress);
#if !IS_BASE
        xTaskCreate(connectBaseTask, "esp_now_connect_base", 2024, NULL, 20, NULL);
#endif
        return ESP_OK;
    }

    esp_err_t sendDataSensor(std::string &data)
    {
#if !IS_BASE
        if (!readyToSend)
        {
            ESP_LOGW(TAG, "not connected to the base");
        }
        else
        {
            std::vector<uint8_t> dataVector(data.begin(), data.end());
            ESP_RETURN_ON_ERROR(sendData(ESP_NOW_TYPE_DATA, base, dataVector), TAG, "Send sensor data failed");
        }
        return ESP_OK;
#else
        ESP_LOGW(TAG, "it's the base");
        return ESP_OK;
#endif
    }
    void setReadyToSend(bool value)
    {
        readyToSend = value;
    }
    esp_err_t deletePeerAddress(std::vector<uint8_t> &macAddr)
    {
        for (int i = 0; i < sensorList.size(); i++)
        {
            if (sensorList[i].sensorAddress == macAddr)
            {
                sensorList.erase(sensorList.begin() + i);
                callbackConnectedSensor(sensorList);
                return ESP_OK;
            }
        }
        ESP_LOGE(TAG, "deletePeerAddress failed the sensor address does not exist in the peer list");
        return ESP_FAIL;
    }
    esp_err_t startMeasurement()
    {
        ESP_LOGI(TAG, "start measurement");
        isRecording = true;
        for (auto &sensor : sensorList)
        {
            std::vector<uint8_t> buffer = {ESP_NOW_QUERY_START_MEASUREMENT};
            esp_err_t err = sendData(ESP_NOW_TYPE_QUERY, sensor.sensorAddress, buffer);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "failed to send the query start measurement to sensor %s", sensor.sensorName.c_str());
            }
        }
        return ESP_OK;
    }
    esp_err_t stopMeasurement()
    {
        ESP_LOGI(TAG, "stop measurement");
        isRecording = false;
        for (auto &sensor : sensorList)
        {
            std::vector<uint8_t> buffer = {ESP_NOW_QUERY_STOP_MEASUREMENT};
            esp_err_t err = sendData(ESP_NOW_TYPE_QUERY, sensor.sensorAddress, buffer);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "failed to send the query stop measurement to sensor %s", sensor.sensorName.c_str());
            }
        }
        return ESP_OK;
    }
    bool getIsRecording()
    {
        return isRecording;
    }
} // namespace espNow
