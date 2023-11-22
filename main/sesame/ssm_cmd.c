#include "aes-cbc-cmac.h"
#include "c_ccm.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_random.h"
#include "ssm_cmd.h"
#include "uECC.h"
#include <string.h>


static const char * TAG    = "ssm5.c";
static uint8_t tag_esp32[] = { 'S', 'E', 'S', 'A', 'M', 'E', ' ', 'E', 'S', 'P', '3', '2' };

uint8_t ecc_private_esp32[32];

static int crypto_backend_micro_ecc_rng_callback(uint8_t * dest, unsigned size) {
    esp_fill_random(dest, (size_t) size);
    return 1;
}

void register_sesame(sesame * ssm) {
    ESP_LOGI(TAG, "[ssm][register][->]");
    uECC_set_rng(crypto_backend_micro_ecc_rng_callback);
    uint8_t ecc_public_esp32[64];
    uECC_make_key_lit(ecc_public_esp32, ecc_private_esp32, uECC_secp256r1());
    ssm->c_offset = sizeof(ecc_public_esp32) + 1;
    ssm->b_buf[0] = SSM_ITEM_CODE_REGISTRATION;
    memcpy(ssm->b_buf + 1, ecc_public_esp32, sizeof(ecc_public_esp32));
    talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_PLAINTEXT);
}

void login_sesame(sesame * ssm) {
    ESP_LOGI(TAG, "[ssm][login][->]");
    ssm->b_buf[0] = SSM_ITEM_CODE_LOGIN;
    AES_CMAC(ssm->device_secret, (const unsigned char *) ssm->cipher.ss5.decrypt.tk_app_ssm, 4, ssm->cipher.ss5.ccm_key);
    memcpy(&ssm->b_buf[1], ssm->cipher.ss5.ccm_key, 4);
    ssm->c_offset = 5;
    talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_PLAINTEXT);
}

void ssm_readHistoryCommand(sesame * ssm) {
    ESP_LOGI(TAG, "[readHistoryCommand]");
    ssm->c_offset = 2;
    ssm->b_buf[0] = SSM_ITEM_CODE_HISTORY;
    ssm->b_buf[1] = 1;
    talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_CIPHERTEXT);
}

void ssm_lock(sesame * ssm, uint8_t * tag, uint8_t tag_length) {
    ESP_LOGI(TAG, "[ssm][lock][ssm->device_status: %d]", ssm->device_status);
    if (ssm->device_status >= SSM_LOGGIN) {
        if (tag_length == 0) {
            tag        = tag_esp32;
            tag_length = sizeof(tag_esp32);
        }
        ssm->c_offset = tag_length + 2;
        ssm->b_buf[0] = SSM_ITEM_CODE_LOCK;
        ssm->b_buf[1] = tag_length;
        memcpy(ssm->b_buf + 2, tag, tag_length);
        talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_CIPHERTEXT);
    }
}

void ssm_unlock(sesame * ssm, uint8_t * tag, uint8_t tag_length) {
    ESP_LOGI(TAG, "[ssm][unlock][ssm->device_status: %d]", ssm->device_status);
    if (ssm->device_status >= SSM_LOGGIN) {
        if (tag_length == 0) {
            tag        = tag_esp32;
            tag_length = sizeof(tag_esp32);
        }
        ssm->c_offset = tag_length + 2;
        ssm->b_buf[0] = SSM_ITEM_CODE_UNLOCK;
        ssm->b_buf[1] = tag_length;
        memcpy(ssm->b_buf + 2, tag, tag_length);
        talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_CIPHERTEXT);
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
