#include "ssm5.h"
#include "aes-cbc-cmac.h"
#include "c_ccm.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_random.h"
#include "uECC.h"
#include <string.h>

static const char * TAG = "ssm5.c";

static uint8_t ecc_private_esp32[32];

static void login_sesame(sesame * ssm) {
    ESP_LOGI(TAG, "[ssm][login][->]");
    ssm->b_buf[0] = SSM_ITEM_CODE_LOGIN;
    AES_CMAC(ssm->device_secret, (const unsigned char *) ssm->cipher.ss5.decrypt.tk_app_ssm, 4, ssm->cipher.ss5.ccm_key);
    memcpy(&ssm->b_buf[1], ssm->cipher.ss5.ccm_key, 4);
    ssm->c_offset = 5;
    talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_PLAINTEXT);
}

static int crypto_backend_micro_ecc_rng_callback(uint8_t * dest, unsigned size) {
    esp_fill_random(dest, (size_t) size);
    return 1;
}

static void register_sesame(sesame * ssm) {
    ESP_LOGI(TAG, "[ssm][register][->]");
    uECC_set_rng(crypto_backend_micro_ecc_rng_callback);
    uint8_t ecc_public_esp32[64];
    uECC_make_key_lit(ecc_public_esp32, ecc_private_esp32, uECC_secp256r1());
    ssm->c_offset = sizeof(ecc_public_esp32) + 1;
    ssm->b_buf[0] = SSM_ITEM_CODE_REGISTRATION;
    memcpy(ssm->b_buf + 1, ecc_public_esp32, sizeof(ecc_public_esp32));
    talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_PLAINTEXT);
}

void ss5_lock(sesame * ssm, uint8_t * tag, uint8_t tag_len) {
    ESP_LOGI(TAG, "[ssm][lock][tag_len: %d][tag: %s]", tag_len, tag);
    ssm->c_offset = tag_len + 2;
    ssm->b_buf[0] = SSM_ITEM_CODE_LOCK;
    ssm->b_buf[1] = tag_len;
    memcpy(ssm->b_buf + 2, tag, tag_len);
    talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_CIPHERTEXT);
}

void ss5_unlock(sesame * ssm, uint8_t * tag, uint8_t tag_len) {
    ESP_LOGI(TAG, "[ssm][unlock][tag_len: %d][tag: %s]", tag_len, tag);
    ssm->c_offset = tag_len + 2;
    ssm->b_buf[0] = SSM_ITEM_CODE_UNLOCK;
    ssm->b_buf[1] = tag_len;
    memcpy(ssm->b_buf + 2, tag, tag_len);
    talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_CIPHERTEXT);
}

void ss5_toggle(sesame * ssm, uint8_t * tag, uint8_t tag_len) {
    if (ssm->device_status == SSM_LOCKED) {
        ss5_unlock(ssm, tag, tag_len);
    } else {
        ss5_lock(ssm, tag, tag_len);
    }
}

void ss5_readHistoryCommand(sesame * ssm) {
    ESP_LOGI(TAG, "[readHistoryCommand]");
    ssm->c_offset = 2;
    ssm->b_buf[0] = SSM_ITEM_CODE_HISTORY;
    ssm->b_buf[1] = 1;
    talk_to_ssm(ssm, SSM_SEG_PARSING_TYPE_CIPHERTEXT);
}

static void ssm_initial_handle(sesame * ssm, uint8_t cmd_it_code) {
    // reset cipher
    ssm->cipher.ss5.encrypt.nouse = 0;
    ssm->cipher.ss5.decrypt.nouse = 0;
    memcpy(ssm->cipher.ss5.encrypt.tk_app_ssm, ssm->b_buf, 4);
    memcpy(ssm->cipher.ss5.decrypt.tk_app_ssm, ssm->b_buf, 4);
    ssm->cipher.ss5.encrypt.count = 0;
    ssm->cipher.ss5.decrypt.count = 0;

    uint8_t null_ssm_count = 0;
    for (uint8_t i = 0; i < SSM_MAX_NUM; i++) {
        if (p_ssms_env->ssm[i].device_secret[i] == 0) {
            null_ssm_count++;
        }
    }
    ESP_LOGI(TAG, "[ss5][null_ssm_count: %d]", null_ssm_count);
    if (null_ssm_count == SSM_MAX_NUM) { // no device_secret
        ESP_LOGI(TAG, "[ss5][no device_secret]");
        register_sesame(ssm);
        return;
    }
    login_sesame(ssm);
}

void ss5_parse_publish(sesame * ssm, uint8_t cmd_it_code) {
    ESP_LOGI(TAG, "[ss5_parse_publish][%d]", cmd_it_code);
    if (cmd_it_code == SSM_ITEM_CODE_INITIAL) {
        ssm_initial_handle(ssm, cmd_it_code);
    }
    if (cmd_it_code == SSM_ITEM_CODE_MECH_STATUS) {
        // ESP_LOG_BUFFER_HEX_LEVEL("MECH_STATUS", ssm->b_buf, 32, ESP_LOG_INFO);
        bool isInLockRange       = (ssm->b_buf[6] & 2u) > 0u;
        bool isInUnlockRange     = (ssm->b_buf[6] & 4u) > 0u;
        device_status lockStatus = isInLockRange ? SSM_LOCKED : isInUnlockRange ? SSM_UNLOCKED : SSM_MOVED;
        // ESP_LOGI("[ssm][mech_status]", "[%d][ssm][current lockStatus: %d][ssm->device_status: %d]", ssm->conn_id, lockStatus, ssm->device_status);
        memcpy(ssm->mechStatus, ssm->b_buf, 7);
        if (ssm->device_status != lockStatus) {
            ssm->device_status = lockStatus;
            p_ssms_env->ssm_cb__(ssm); // callback: ssm_action_handle
        }
    }
}

void ss5_parse_response(sesame * ssm, uint8_t cmd_it_code) {
    ssm->c_offset = ssm->c_offset - 1;
    memcpy(ssm->b_buf, ssm->b_buf + 1, ssm->c_offset);

    if (cmd_it_code == SSM_ITEM_CODE_LOGIN) {
        ESP_LOGI(TAG, "[%d][ssm][login][ok]", ssm->conn_id);
        ssm->device_status = SSM_LOGGIN;
        p_ssms_env->ssm_cb__(ssm); // callback: ssm_action_handle
    }
    if (cmd_it_code == SSM_ITEM_CODE_HISTORY) {
        ESP_LOGI(TAG, "[%d][ssm][hisdataLength: %d]", ssm->conn_id, ssm->c_offset);
        if (ssm->c_offset == 0) { //循環讀取 避免沒取完歷史
            return;
        }
        ss5_readHistoryCommand(ssm);
    }
    if (cmd_it_code == SSM_ITEM_CODE_REGISTRATION) {
        ESP_LOGI(TAG, "[%d][ssm][registration][ok]", ssm->conn_id);
        memcpy(ssm->public_key, &ssm->b_buf[13], 64);
        // ESP_LOG_BUFFER_HEX("public_key", ss5->public_key, 64);
        uint8_t ecdh_secret_ss5[32];
        uECC_shared_secret_lit(ssm->public_key, ecc_private_esp32, ecdh_secret_ss5, uECC_secp256r1());
        memcpy(ssm->device_secret, ecdh_secret_ss5, 16); // ss5 device_secret
        ESP_LOG_BUFFER_HEX("deviceSecret", ssm->device_secret, 16);
        AES_CMAC(ssm->device_secret, (const unsigned char *) ssm->cipher.ss5.decrypt.tk_app_ssm, 4, ssm->cipher.ss5.ccm_key);
        ssm->device_status = SSM_LOGGIN;
        p_ssms_env->ssm_cb__(ssm);
    }
}

void ss5_ble_receiver(sesame * ssm, const uint8_t * p_data, uint16_t len) {
    if (p_data[0] & 1u) {
        ssm->c_offset = 0;
    }
    memcpy(&ssm->b_buf[ssm->c_offset], p_data + 1, len - 1);
    ssm->c_offset += len - 1;
    if (p_data[0] >> 1u == SSM_SEG_PARSING_TYPE_APPEND_ONLY) {
        return;
    }
    if (p_data[0] >> 1u == SSM_SEG_PARSING_TYPE_CIPHERTEXT) {
        ssm->c_offset = ssm->c_offset - CCM_TAG_LENGTH;
        aes_ccm_auth_decrypt(ssm->cipher.ss5.ccm_key, (const unsigned char *) &ssm->cipher.ss5.decrypt, 13, additional_data, 1, ssm->b_buf, ssm->c_offset, ssm->b_buf, ssm->b_buf + ssm->c_offset, CCM_TAG_LENGTH);
        ssm->cipher.ss5.decrypt.count++;
    }

    uint8_t cmd_op_code = ssm->b_buf[0];
    uint8_t cmd_it_code = ssm->b_buf[1];
    ssm->c_offset       = ssm->c_offset - 2;
    memcpy(ssm->b_buf, ssm->b_buf + 2, ssm->c_offset);
    // ESP_LOG_BUFFER_HEX("[ssm][say]", ssm->b_buf, ssm->c_offset);
    ESP_LOGI(TAG, "[ssm][op:%x][it:%x]", cmd_op_code, cmd_it_code);
    if (cmd_op_code == SSM_OP_CODE_PUBLISH) {
        ss5_parse_publish(ssm, cmd_it_code);
    }
    if (cmd_op_code == SSM_OP_CODE_RESPONSE) {
        ss5_parse_response(ssm, cmd_it_code);
    }
    ssm->c_offset = 0;
}
