#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_log.h"
#include "communication.h"

static const char *TAG = "app_communication";
static uint32_t l_boot_id;

const uint8_t BROADCAST_ADDR[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

void init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void init_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

    ESP_LOGI(TAG, "Wifi initied in channel %d", CONFIG_ESPNOW_CHANNEL);
}

void espnow_add_peer(const uint8_t mac_addr[ESP_NOW_ETH_ALEN], bool encrypt)
{
    ESP_LOGI(TAG, "Adding peer: "MACSTR"", MAC2STR(mac_addr));

    if (esp_now_is_peer_exist(mac_addr)) {
        ESP_LOGI(TAG, "Peer already exists. No action needed.");
        return;
    }

    esp_now_peer_info_t peer;
    peer.channel = CONFIG_ESPNOW_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = encrypt;
    memcpy(peer.peer_addr, mac_addr, ESP_NOW_ETH_ALEN);
    if (encrypt) {
        memcpy(peer.lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
    }

    esp_err_t result = esp_now_add_peer(&peer);
    if (result == ESP_OK) {
        ESP_LOGI(TAG, "Peer added successfully!");
    } else {
        ESP_LOGE(TAG, "Failed to add peer: %s", esp_err_to_name(result));
    }
}

void espnow_send_msg(const uint8_t mac_addr[ESP_NOW_ETH_ALEN], uint8_t type, uint8_t *payload[])
{
    espnow_msg_t msg;
    msg.app_magic = APP_MAGIC;
    msg.boot_id = l_boot_id;
    msg.type = type;
    msg.seq = 0; // TODO: Implement seq
    if (payload != NULL) {
        memcpy(msg.payload, *payload, sizeof(*payload));
    }

    esp_now_send(mac_addr, (uint8_t *)&msg, sizeof(msg));
}

void init_espnow(uint32_t boot_id, esp_now_send_cb_t send_cb, esp_now_recv_cb_t recv_cb)
{
    l_boot_id = boot_id;

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(recv_cb));
    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK));

    ESP_LOGI(TAG, "ESP-NOW initied in channel %d", CONFIG_ESPNOW_CHANNEL);

    espnow_add_peer(BROADCAST_ADDR, false);
}
