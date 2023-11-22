#include "blecent.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "ssm5.h"

static const char * TAG = "main.c";

static void ssm_action_handle(sesame * ssm) {
    ESP_LOGI(TAG, "ssm_action_handle: %s", SSM_STATUS_STR(ssm->device_status));
    if (ssm->device_status == SSM_CONNECTING) {
        ESP_LOGI(TAG, "ssm->device_status == SSM_CONNECTING");
    } else if (ssm->device_status == SSM_SCANNING) {
        ESP_LOGI(TAG, "ssm->device_status == SSM_SCANNING");
    } else if (ssm->device_status == SSM_LOGGIN) {
        ESP_LOGI(TAG, "ssm->device_status == SSM_LOGGIN");
    } else if (ssm->device_status == SSM_LOCKED) {
        ESP_LOGI(TAG, "ssm->device_status == SSM_LOCKED");
    } else if (ssm->device_status == SSM_UNLOCKED) {
        ESP_LOGI(TAG, "ssm->device_status == SSM_UNLOCKED");
        ssm_toggle_all(NULL, 0);
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "SesameSDK_ESP32 [11/22][001]");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { /* Initialize NVS — it is used to store PHY calibration data */
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ssm_init(ssm_action_handle);
    esp_ble_init();
}
