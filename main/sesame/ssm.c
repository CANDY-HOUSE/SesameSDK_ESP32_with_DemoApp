#include "ssm.h"
#include "blecent.h"
#include "c_ccm.h"
#include "candy.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "nimble/ble.h"
#include "ssm5.h"
#include "uECC.h"
#include <aes-cbc-cmac.h>
#include <stdbool.h>
#include <string.h>

static const char * TAG = "ssm.c";

struct ssm_env_tag * p_ssms_env = NULL;

void ssm_mem_deinit(void) {
    free(p_ssms_env);
}

static void ssm_mem_init(void) {
    p_ssms_env = (struct ssm_env_tag *) malloc(sizeof(struct ssm_env_tag));
    if (p_ssms_env == NULL) {
        ESP_LOGE(TAG, "[p_ssms_env][FAIL]");
    }
    memset(p_ssms_env, 0, sizeof(struct ssm_env_tag));
    p_ssms_env->ssm_cb__ = NULL;
    p_ssms_env->number   = 0;
}

void ssm_init(ssm_action ssm_action_cb) {
    ssm_mem_init();                       // malloc p_ssms_env
    p_ssms_env->ssm_cb__ = ssm_action_cb; // callback: ssm_action_handle
    for (int i = 0; i < SSM_MAX_NUM; ++i) {
        p_ssms_env->ssm[i].device_status = SSM_NOUSE;
        p_ssms_env->ssm[i].conn_id       = 0xFF; // 0xFF: not connected
    }
}

uint8_t additional_data[] = { 0x00 };
void talk_to_ssm(sesame * ssm, uint8_t parsing_type) {
    ESP_LOGI(TAG, "[%x][talk_to_ssm] => [ssm->c_offset: %d]", ssm->conn_id, ssm->c_offset);
    ESP_LOG_BUFFER_HEX_LEVEL("[esp32][say]", ssm->b_buf, ssm->c_offset, ESP_LOG_INFO);
    if (parsing_type == SSM5_SEG_PARSING_TYPE_CIPHERTEXT) {
        aes_ccm_encrypt_and_tag(ssm->cipher.ss5.ccm_key, (const unsigned char *) &ssm->cipher.ss5.encrypt, 13, additional_data, 1, ssm->b_buf, ssm->c_offset, ssm->b_buf, ssm->b_buf + ssm->c_offset, CCM_TAG_LENGTH);
        ssm->cipher.ss5.encrypt.count++;
        ssm->c_offset = ssm->c_offset + CCM_TAG_LENGTH;
    }

    uint8_t * data    = ssm->b_buf;
    uint16_t remain   = ssm->c_offset;
    uint16_t len      = remain;
    uint8_t tmp_v[20] = { 0 };
    uint16_t len_l;

    while (remain) {
        if (remain <= 19) {
            tmp_v[0] = parsing_type << 1u;
            len_l    = 1 + remain;
        } else {
            tmp_v[0] = 0;
            len_l    = 20;
        }
        if (remain == len) {
            tmp_v[0] |= 1u;
        }
        memcpy(&tmp_v[1], data, len_l - 1);
        esp_ble_gatt_write(ssm, tmp_v, len_l);
        remain -= (len_l - 1);
        data += (len_l - 1);
    }
}

void ssm_disconnect(sesame * ssm) {
    ESP_LOGW(TAG, "[ssm][disconnect]");
    if (ssm->device_status >= SSM_CONNECTED) {
        ble_gap_terminate(ssm->conn_id, BLE_ERR_REM_USER_CONN_TERM); // disconnect
    }
}

void ssm_say_handler(const uint8_t * p_data, uint16_t len, uint8_t conn_id) {
    for (int i = 0; i < SSM_MAX_NUM; ++i) {
        if (p_ssms_env->ssm[i].conn_id == conn_id) {
            ss5_ble_receiver(&p_ssms_env->ssm[i], p_data, len);
            break;
        }
    }
}

void ssm_unlock_all(uint8_t * tag, uint8_t tag_length) {
    ESP_LOGI(TAG, "[ssm][ssm_unlock_all]");
    for (int i = 0; i < SSM_MAX_NUM; ++i) {
        ssm_unlock(&p_ssms_env->ssm[i], tag, tag_length);
    }
}

void ssm_lock_all(uint8_t * tag, uint8_t tag_length) {
    ESP_LOGI(TAG, "[ssm][ssm_lock_all]");
    for (int i = 0; i < SSM_MAX_NUM; ++i) {
        ssm_lock(&p_ssms_env->ssm[i], tag, tag_length);
    }
}

void ssm_toggle_all(uint8_t * tag, uint8_t tag_length) {
    ESP_LOGI(TAG, "[ssm][ssm_toggle_all]");
    for (int i = 0; i < SSM_MAX_NUM; ++i) {
        if (p_ssms_env->ssm[i].device_status == SSM_LOCKED) {
            ssm_unlock_all(tag, tag_length);
        } else if (p_ssms_env->ssm[i].device_status == SSM_UNLOCKED) {
            ssm_lock_all(tag, tag_length);
        }
    }
}

uint8_t tag_esp32[] = { 'S', 'E', 'S', 'A', 'M', 'E', ' ', 'E', 'S', 'P', '3', '2' };
void ssm_lock(sesame * ssm, uint8_t * tag, uint8_t tag_length) {
    ESP_LOGI(TAG, "[ssm][lock][ssm->device_status: %d]", ssm->device_status);
    if (ssm->device_status >= SSM_LOGGIN) {
        if (tag_length == 0) {
            ss5_lock(ssm, tag_esp32, sizeof(tag_esp32));
        } else {
            ss5_lock(ssm, tag, tag_length);
        }
    }
}

void ssm_unlock(sesame * ssm, uint8_t * tag, uint8_t tag_length) {
    ESP_LOGI(TAG, "[ssm][unlock][ssm->device_status: %d]", ssm->device_status);
    if (ssm->device_status >= SSM_LOGGIN) {
        if (tag_length == 0) {
            ss5_unlock(ssm, tag_esp32, sizeof(tag_esp32));
        } else {
            ss5_unlock(ssm, tag, tag_length);
        }
    }
}

void add_ssm(uint8_t * addr) {
    for (int i = 0; i < SSM_MAX_NUM; ++i) {
        if (p_ssms_env->ssm[i].device_status == SSM_NOUSE) {
            memcpy(p_ssms_env->ssm[i].addr, addr, 6);
            p_ssms_env->ssm[i].device_status = SSM_DISCONNECTED;
            p_ssms_env->ssm[i].conn_id       = 0xFF;
        }
    }
}
