#include "blecent.h"
#include "nvs_flash.h"
#include "ssm_cmd.h"

static const char * TAG = "main.c";

static void ssm_action_handle(sesame * ssm) {
    ESP_LOGI(TAG, "[ssm][status][%s]", SSM_STATUS_STR(ssm->device_status));
    if (ssm->device_status == SSM_UNLOCKED) {
        ssm_lock_unlock(SSM_ITEM_CODE_LOCK, NULL, 0);
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "SesameSDK_ESP32 [11/23][087]");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { /* Initialize NVS — it is used to store PHY calibration data */
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ssm_init(ssm_action_handle);
    esp_ble_init();
}
