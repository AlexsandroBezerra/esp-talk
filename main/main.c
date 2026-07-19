#include "freertos/FreeRTOS.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_random.h"
#include "driver/gpio.h"
#include "communication.h"
#include "broadcast_discovery.h"

static const char *TAG = "app_main";

static uint32_t boot_id;
static QueueHandle_t s_espnow_recv_queue = NULL;

static void recv_task(void *pvParameter)
{
    espnow_recv_event_t event;

    while (xQueueReceive(s_espnow_recv_queue, &event, portMAX_DELAY) == pdTRUE) {
        espnow_msg_t *msg = (espnow_msg_t *)event.data;

        if (msg->app_magic != APP_MAGIC) {
            continue;
        }

        ESP_LOGI(TAG, "Receive data from: "MACSTR", type: %X", MAC2STR(event.src_addr), msg->type);
        
        free(event.data);
    }
}

static void send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
    if (tx_info == NULL) {
        ESP_LOGE(TAG, "Send callback arg error");
        return;
    }

    uint8_t *des_addr = tx_info->des_addr;

    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "Send success to " MACSTR, MAC2STR(des_addr));
    } else {
        ESP_LOGE(TAG, "Send fail to " MACSTR, MAC2STR(des_addr));
    }
}

static void recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    espnow_recv_event_t event;
    uint8_t *src_addr = recv_info->src_addr;

    if (src_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive callback arg error");
        return;
    }

    espnow_msg_t *msg = (espnow_msg_t *)data;
    if (msg->app_magic != APP_MAGIC) {
        ESP_LOGW(TAG, "Invalid mesage, discarding...");
        return;
    }

    event.data_len = len;
    event.data = malloc(len);
    if (event.data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(event.src_addr, src_addr, ESP_NOW_ETH_ALEN);
    memcpy(event.data, data, ESP_NOW_ETH_ALEN);

    if (xQueueSend(s_espnow_recv_queue, &event, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Send recv_event to queue failed");
        free(event.data);
    }
}

void app_main(void)
{
    boot_id = esp_random();
    gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);
    s_espnow_recv_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(espnow_recv_event_t));
    if (s_espnow_recv_queue == NULL) {
        ESP_LOGE(TAG, "Create queue fail");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    init_nvs();
    init_wifi();
    init_espnow(boot_id, send_cb, recv_cb);

    xTaskCreate(recv_task, "recv_task", 2048, NULL, 4, NULL);

    init_broadcast_discovery();
    vTaskDelay(pdMS_TO_TICKS(2000));
    deinit_broadcast_discovery();
}
