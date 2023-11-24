#ifndef __SSM_H__
#define __SSM_H__

#include "candy.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SSM_MAX_NUM (1u)

#pragma pack(1)

typedef struct {
    int64_t count;
    uint8_t nouse;
    uint8_t random_code[4];
} SSM_CCM_NONCE;

typedef struct {
    uint8_t token[16];
    SSM_CCM_NONCE encrypt;
    SSM_CCM_NONCE decrypt;
} SesameBleCipher;

typedef struct mech_status_s {
    uint16_t battery;
    int16_t target;               // 馬達想到的地方
    int16_t position;             // 感測器同步到的最新角度
    uint8_t is_clutch_failed : 1; // 電磁鐵作棟是否成功(沒用到)
    uint8_t is_lock_range : 1;    // 在關鎖位置
    uint8_t is_unlock_range : 1;  // 在開鎖位置
    uint8_t is_critical : 1;      // 開關鎖時間超時，馬達停轉
    uint8_t is_stop : 1;          // 把手角度沒有變化
    uint8_t is_low_battery : 1;   // 低電量(<5V)
    uint8_t is_clockwise : 1;     // 馬達轉動方向
} mech_status_t;                  // total 7 bytes

typedef struct {
    uint8_t device_uuid[16];
    uint8_t public_key[64];
    uint8_t device_secret[16];
    uint8_t addr[6];
    volatile uint8_t device_status;
    SesameBleCipher cipher;
    mech_status_t mech_status;
    uint16_t c_offset;
    uint8_t b_buf[80]; /// max command size is register(80 Bytes).
    uint8_t conn_id;
} sesame;

typedef void (*ssm_action)(sesame * ssm);

struct ssm_env_tag {
    sesame ssm;
    ssm_action ssm_cb__;
};

#pragma pack()

extern struct ssm_env_tag * p_ssms_env;

void ssm_ble_receiver(sesame * ssm, const uint8_t * p_data, uint16_t len);

void talk_to_ssm(sesame * ssm, uint8_t parsing_type);

void ssm_mem_deinit(void);

void ssm_init(ssm_action ssm_action_cb);

#ifdef __cplusplus
}
#endif

#endif // __SSM_H__
