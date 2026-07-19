idf_component_register(SRCS "main.c" "communication.c" "broadcast_discovery.c"
                    PRIV_REQUIRES esp_driver_gpio nvs_flash esp_event esp_netif esp_wifi
                    INCLUDE_DIRS ".")
