#include "wifidetect.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_console.h"
#include <string.h>

#define DEFAULT_SCAN_LIST_SIZE 10

// Declarations
static void register_wifidetect(void);
static int do_wifidetect_cmd(int argc, char **argv);
static void wifi_scan(void);

// Definitions
void wifidetect_setup(void)
{
    register_wifidetect();
}

/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_scan_start(NULL, true);

    printf("Max AP number ap_info can hold = %u \r\n", number);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    printf("Total APs scanned = %u, actual AP number ap_info holds = %u \r\n", ap_count, number);
    for (int i = 0; i < number; i++) {
        printf("SSID \t%s\r\n", ap_info[i].ssid);
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    esp_netif_destroy_default_wifi(sta_netif);
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
}

static int do_wifidetect_cmd(int argc, char **argv)
{
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  wifi_scan();

  return 0;
}

static void register_wifidetect(void)
{
  const esp_console_cmd_t wifidetect_cmd = {
      .command = "wifidetect",
      .help = "Lists all available wifi networks.",
      .hint = NULL,
      .func = &do_wifidetect_cmd,
      .argtable = NULL
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&wifidetect_cmd));
}