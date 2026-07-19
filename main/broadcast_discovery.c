#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_now.h"
#include "esp_log.h"
#include "communication.h"

static const char *TAG = "broadcast_discovery";
static TaskHandle_t broadcast_discovery_task_handle;

static void broadcast_discovery_task(void *pv_parameter)
{
    bool led_status = false;

    while (1) {
        led_status = !led_status;
        gpio_set_level(CONFIG_BLINK_GPIO, led_status);
        espnow_send_msg(BROADCAST_ADDR, MSG_DISCOVERY, NULL);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void init_broadcast_discovery(void)
{
    gpio_set_level(CONFIG_BLINK_GPIO, 0);

    if (broadcast_discovery_task_handle != NULL) {
        ESP_LOGW(TAG, "Broadcast discovery task already initied, no action required...");
        return;
    }

    xTaskCreate(broadcast_discovery_task, "broadcast_discovery_task", 1024, NULL, 5, &broadcast_discovery_task_handle);
    ESP_LOGI(TAG, "Broadcast discovery task initied...");
}

void deinit_broadcast_discovery(void)
{
    gpio_set_level(CONFIG_BLINK_GPIO, 0);

    if (broadcast_discovery_task_handle == NULL) {
        ESP_LOGW(TAG, "Broadcast discovery task not initied, no action required...");
        return;
    }

    vTaskDelete(broadcast_discovery_task_handle);
    broadcast_discovery_task_handle = NULL;
    ESP_LOGI(TAG, "Broadcast discovery task stopped...");
}