#include "ssm.h"
#include "blecent.h"
#include "c_ccm.h"
#include "ssm_cmd.h"

static const char * TAG = "ssm.c";

static uint8_t additional_data[] = { 0x00 };

struct ssm_env_tag * p_ssms_env = NULL;

void ssm_mem_deinit(void) {
    free(p_ssms_env);
}

void ssm_init(ssm_action ssm_action_cb) {
    p_ssms_env = (struct ssm_env_tag *) malloc(sizeof(struct ssm_env_tag));
    if (p_ssms_env == NULL) {
        ESP_LOGE(TAG, "[p_ssms_env][FAIL]");
    }
    memset(p_ssms_env, 0, sizeof(struct ssm_env_tag));
    p_ssms_env->number            = 0;
    p_ssms_env->ssm_cb__          = ssm_action_cb; // callback: ssm_action_handle
    p_ssms_env->ssm.conn_id       = 0xFF;          // 0xFF: not connected
    p_ssms_env->ssm.device_status = SSM_NOUSE;
}

void talk_to_ssm(sesame * ssm, uint8_t parsing_type) {
    ESP_LOGI(TAG, "[%x][talk_to_ssm] => [ssm->c_offset: %d]", ssm->conn_id, ssm->c_offset);
    ESP_LOG_BUFFER_HEX_LEVEL("[esp32][say]", ssm->b_buf, ssm->c_offset, ESP_LOG_INFO);
    if (parsing_type == SSM_SEG_PARSING_TYPE_CIPHERTEXT) {
        aes_ccm_encrypt_and_tag(ssm->cipher.ssm.ccm_key, (const unsigned char *) &ssm->cipher.ssm.encrypt, 13, additional_data, 1, ssm->b_buf, ssm->c_offset, ssm->b_buf, ssm->b_buf + ssm->c_offset, CCM_TAG_LENGTH);
        ssm->cipher.ssm.encrypt.count++;
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

static void ssm_initial_handle(sesame * ssm, uint8_t cmd_it_code) {
    // reset cipher
    ssm->cipher.ssm.encrypt.nouse = 0;
    ssm->cipher.ssm.decrypt.nouse = 0;
    memcpy(ssm->cipher.ssm.encrypt.random_code, ssm->b_buf, 4);
    memcpy(ssm->cipher.ssm.decrypt.random_code, ssm->b_buf, 4);
    ssm->cipher.ssm.encrypt.count = 0;
    ssm->cipher.ssm.decrypt.count = 0;

    if (p_ssms_env->ssm.device_secret[0] == 0) { // no device_secret
        ESP_LOGI(TAG, "[ssm][no device_secret]");
        send_reg_cmd_to_ssm(ssm);
        return;
    }
    send_login_cmd_to_ssm(ssm);
}

static void ssm_parse_publish(sesame * ssm, uint8_t cmd_it_code) {
    ESP_LOGI(TAG, "[ssm_parse_publish][%d]", cmd_it_code);
    if (cmd_it_code == SSM_ITEM_CODE_INITIAL) { // get 4 bytes random_code
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

static void ssm_parse_response(sesame * ssm, uint8_t cmd_it_code) {
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
        ssm_readHistoryCommand(ssm);
    }
    if (cmd_it_code == SSM_ITEM_CODE_REGISTRATION) {
        ESP_LOGI(TAG, "[%d][ssm][registration][ok]", ssm->conn_id);
        handle_reg_data_from_ssm(ssm);
    }
}

static void ssm_ble_receiver(sesame * ssm, const uint8_t * p_data, uint16_t len) {
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
        aes_ccm_auth_decrypt(ssm->cipher.ssm.ccm_key, (const unsigned char *) &ssm->cipher.ssm.decrypt, 13, additional_data, 1, ssm->b_buf, ssm->c_offset, ssm->b_buf, ssm->b_buf + ssm->c_offset, CCM_TAG_LENGTH);
        ssm->cipher.ssm.decrypt.count++;
    }

    uint8_t cmd_op_code = ssm->b_buf[0];
    uint8_t cmd_it_code = ssm->b_buf[1];
    ssm->c_offset       = ssm->c_offset - 2;
    memcpy(ssm->b_buf, ssm->b_buf + 2, ssm->c_offset);
    // ESP_LOG_BUFFER_HEX("[ssm][say]", ssm->b_buf, ssm->c_offset);
    ESP_LOGI(TAG, "[ssm][op:%x][it:%x]", cmd_op_code, cmd_it_code);
    if (cmd_op_code == SSM_OP_CODE_PUBLISH) {
        ssm_parse_publish(ssm, cmd_it_code);
    }
    if (cmd_op_code == SSM_OP_CODE_RESPONSE) {
        ssm_parse_response(ssm, cmd_it_code);
    }
    ssm->c_offset = 0;
}

void ssm_say_handler(const uint8_t * p_data, uint16_t len, uint8_t conn_id) {
    if (p_ssms_env->ssm.conn_id == conn_id) {
        ssm_ble_receiver(&p_ssms_env->ssm, p_data, len);
    }
}
