#define ESPNOW_QUEUE_SIZE 6

#define IS_BROADCAST_ADDR(addr) (memcmp(addr, BROADCAST_ADDR, ESP_NOW_ETH_ALEN) == 0)

extern const uint8_t BROADCAST_ADDR[ESP_NOW_ETH_ALEN];

typedef struct {
    uint8_t src_addr[ESP_NOW_ETH_ALEN];
    uint8_t *data;
    int data_len;
} espnow_recv_event_t;

#define APP_MAGIC 0XCA

typedef enum {
    MSG_DISCOVERY     = 0x01,
    MSG_PAIR_REQ      = 0x02,
    MSG_PAIR_ACK      = 0x03,
    MSG_HEARTBEAT     = 0x04,
    MSG_HEARTBEAT_ACK = 0x05,
    MSG_DATA          = 0x06,
} msg_type_t;

typedef struct __attribute__((packed)) {
    uint8_t  app_magic;
    uint8_t  type;
    uint32_t boot_id;
    uint32_t seq;
    uint8_t  payload[];
} espnow_msg_t;

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
} espnow_pair_task_param_t;

// Functions

void init_nvs(void);

void init_wifi(void);

void init_espnow(uint32_t boot_id, esp_now_send_cb_t send_cb, esp_now_recv_cb_t recv_cb);

void espnow_add_peer(const uint8_t mac_addr[ESP_NOW_ETH_ALEN], bool encrypt);

void espnow_send_msg(const uint8_t mac_addr[ESP_NOW_ETH_ALEN], uint8_t type, uint8_t *payload[]);
