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

static void login_sesame5(sesame * ss5) {
    ESP_LOGI(TAG, "[ss5][login][->]");
    ss5->b_buf[0] = SSM5_ITEM_CODE_LOGIN;
    AES_CMAC(ss5->device_secret, (const unsigned char *) ss5->cipher.ss5.decrypt.tk_app_ssm, 4, ss5->cipher.ss5.ccm_key);
    memcpy(&ss5->b_buf[1], ss5->cipher.ss5.ccm_key, 4);
    ss5->c_offset = 5;
    talk_to_ssm(ss5, SSM5_SEG_PARSING_TYPE_PLAINTEXT);
}

static int crypto_backend_micro_ecc_rng_callback(uint8_t * dest, unsigned size) {
    esp_fill_random(dest, (size_t) size);
    return 1;
}

static void register_sesame5(sesame * ss5) {
    ESP_LOGI(TAG, "[ss5][register][->]");
    uECC_set_rng(crypto_backend_micro_ecc_rng_callback);
    uint8_t ecc_public_esp32[64];
    uECC_make_key_lit(ecc_public_esp32, ecc_private_esp32, uECC_secp256r1());
    ss5->c_offset = sizeof(ecc_public_esp32) + 1;
    ss5->b_buf[0] = SSM5_ITEM_CODE_REGISTRATION;
    memcpy(ss5->b_buf + 1, ecc_public_esp32, sizeof(ecc_public_esp32));
    talk_to_ssm(ss5, SSM5_SEG_PARSING_TYPE_PLAINTEXT);
}

void ss5_lock(sesame * ss5, uint8_t * tag, uint8_t tag_len) {
    ESP_LOGI(TAG, "[ss5][lock][tag_len: %d][tag: %s]", tag_len, tag);
    ss5->c_offset = tag_len + 2;
    ss5->b_buf[0] = SSM5_ITEM_CODE_LOCK;
    ss5->b_buf[1] = tag_len;
    memcpy(ss5->b_buf + 2, tag, tag_len);
    talk_to_ssm(ss5, SSM5_SEG_PARSING_TYPE_CIPHERTEXT);
}

void ss5_unlock(sesame * ss5, uint8_t * tag, uint8_t tag_len) {
    ESP_LOGI(TAG, "[ss5][unlock][tag_len: %d][tag: %s]", tag_len, tag);
    ss5->c_offset = tag_len + 2;
    ss5->b_buf[0] = SSM5_ITEM_CODE_UNLOCK;
    ss5->b_buf[1] = tag_len;
    memcpy(ss5->b_buf + 2, tag, tag_len);
    talk_to_ssm(ss5, SSM5_SEG_PARSING_TYPE_CIPHERTEXT);
}

void ss5_toggle(sesame * ss5, uint8_t * tag, uint8_t tag_len) {
    if (ss5->ss5_device_status == SSM5_LOCKED) {
        ss5_unlock(ss5, tag, tag_len);
    } else {
        ss5_lock(ss5, tag, tag_len);
    }
}

void ss5_readHistoryCommand(sesame * ss5) {
    ESP_LOGI(TAG, "[readHistoryCommand]");
    ss5->c_offset = 2;
    ss5->b_buf[0] = SSM5_ITEM_CODE_HISTORY;
    ss5->b_buf[1] = 1;
    talk_to_ssm(ss5, SSM5_SEG_PARSING_TYPE_CIPHERTEXT);
}

static void ssm_initial_handle(sesame * ss5, uint8_t cmd_it_code) {
    // reset cipher
    ss5->cipher.ss5.encrypt.nouse = 0;
    ss5->cipher.ss5.decrypt.nouse = 0;
    memcpy(ss5->cipher.ss5.encrypt.tk_app_ssm, ss5->b_buf, 4);
    memcpy(ss5->cipher.ss5.decrypt.tk_app_ssm, ss5->b_buf, 4);
    ss5->cipher.ss5.encrypt.count = 0;
    ss5->cipher.ss5.decrypt.count = 0;

    uint8_t null_ssm_count = 0;
    for (uint8_t i = 0; i < SSM_MAX_NUM; i++) {
        if (p_ssms_env->ssm[i].device_secret[i] == 0) {
            null_ssm_count++;
        }
    }
    ESP_LOGI(TAG, "[ss5][null_ssm_count: %d]", null_ssm_count);
    if (null_ssm_count == SSM_MAX_NUM) { // no device_secret
        ESP_LOGI(TAG, "[ss5][no device_secret]");
        register_sesame5(ss5);
        return;
    }
    login_sesame5(ss5);
}

void ss5_parse_publish(sesame * ss5, uint8_t cmd_it_code) {
    ESP_LOGI(TAG, "[ss5_parse_publish][%d]", cmd_it_code);
    if (cmd_it_code == SSM5_ITEM_CODE_INITIAL) {
        ssm_initial_handle(ss5, cmd_it_code);
    }
    if (cmd_it_code == SSM5_ITEM_CODE_MECH_STATUS) {
        // ESP_LOG_BUFFER_HEX_LEVEL("MECH_STATUS", ss5->b_buf, 32, ESP_LOG_INFO);
        bool isInLockRange           = (ss5->b_buf[6] & 2u) > 0u;
        bool isInUnlockRange         = (ss5->b_buf[6] & 4u) > 0u;
        ss5_device_status lockStatus = isInLockRange ? SSM5_LOCKED : isInUnlockRange ? SSM5_UNLOCKED : SSM5_MOVED;
        // ESP_LOGI("[ss5][mech_status]", "[%d][ss5][current lockStatus: %d][ss5->ss5_device_status: %d]", ss5->conn_id, lockStatus, ss5->ss5_device_status);
        memcpy(ss5->mechStatus, ss5->b_buf, 7);
        if (ss5->ss5_device_status != lockStatus) {
            ss5->ss5_device_status = lockStatus;
            p_ssms_env->ssm_cb__(ss5); // callback: ssm_action_handle
        }
    }
}

void ss5_parse_response(sesame * ss5, uint8_t cmd_it_code) {
    ss5->c_offset = ss5->c_offset - 1;
    memcpy(ss5->b_buf, ss5->b_buf + 1, ss5->c_offset);

    if (cmd_it_code == SSM5_ITEM_CODE_LOGIN) {
        ESP_LOGI(TAG, "[%d][ss5][login][ok]", ss5->conn_id);
        ss5->ss5_device_status = SSM5_LOGGIN;
        p_ssms_env->ssm_cb__(ss5); // callback: ssm_action_handle
    }
    if (cmd_it_code == SSM5_ITEM_CODE_HISTORY) {
        ESP_LOGI(TAG, "[%d][ss5][hisdataLength: %d]", ss5->conn_id, ss5->c_offset);
        if (ss5->c_offset == 0) { //循環讀取 避免沒取完歷史
            return;
        }
        ss5_readHistoryCommand(ss5);
    }
    if (cmd_it_code == SSM5_ITEM_CODE_REGISTRATION) {
        ESP_LOGI(TAG, "[%d][ss5][registration][ok]", ss5->conn_id);
        memcpy(ss5->ss5_pub_key, &ss5->b_buf[13], 64);
        // ESP_LOG_BUFFER_HEX("ss5_pub_key", ss5->ss5_pub_key, 64);
        uint8_t ecdh_secret_ss5[32];
        uECC_shared_secret_lit(ss5->ss5_pub_key, ecc_private_esp32, ecdh_secret_ss5, uECC_secp256r1());
        memcpy(ss5->device_secret, ecdh_secret_ss5, 16); // ss5 device_secret
        ESP_LOG_BUFFER_HEX("deviceSecret", ss5->device_secret, 16);
        AES_CMAC(ss5->device_secret, (const unsigned char *) ss5->cipher.ss5.decrypt.tk_app_ssm, 4, ss5->cipher.ss5.ccm_key);
        ss5->ss5_device_status = SSM5_LOGGIN;
        p_ssms_env->ssm_cb__(ss5);
    }
}

void ss5_ble_receiver(sesame * ss5, const uint8_t * p_data, uint16_t len) {
    if (p_data[0] & 1u) {
        ss5->c_offset = 0;
    }
    memcpy(&ss5->b_buf[ss5->c_offset], p_data + 1, len - 1);
    ss5->c_offset += len - 1;
    if (p_data[0] >> 1u == SSM5_SEG_PARSING_TYPE_APPEND_ONLY) {
        return;
    }
    if (p_data[0] >> 1u == SSM5_SEG_PARSING_TYPE_CIPHERTEXT) {
        ss5->c_offset = ss5->c_offset - CCM_TAG_LENGTH;
        aes_ccm_auth_decrypt(ss5->cipher.ss5.ccm_key, (const unsigned char *) &ss5->cipher.ss5.decrypt, 13, additional_data, 1, ss5->b_buf, ss5->c_offset, ss5->b_buf, ss5->b_buf + ss5->c_offset, CCM_TAG_LENGTH);
        ss5->cipher.ss5.decrypt.count++;
    }

    uint8_t cmd_op_code = ss5->b_buf[0];
    uint8_t cmd_it_code = ss5->b_buf[1];
    ss5->c_offset       = ss5->c_offset - 2;
    memcpy(ss5->b_buf, ss5->b_buf + 2, ss5->c_offset);
    // ESP_LOG_BUFFER_HEX("[ss5][say]", ss5->b_buf, ss5->c_offset);
    ESP_LOGI(TAG, "[ss5][op:%x][it:%x]", cmd_op_code, cmd_it_code);
    if (cmd_op_code == SSM5_OP_CODE_PUBLISH) {
        ss5_parse_publish(ss5, cmd_it_code);
    }
    if (cmd_op_code == SSM5_OP_CODE_RESPONSE) {
        ss5_parse_response(ss5, cmd_it_code);
    }
    ss5->c_offset = 0;
}
