#ifndef __SSM_LINK_H__
#define __SSM_LINK_H__

#include "candy.h"
#include <stdbool.h>
#include <stddef.h>

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

union ssm_cipher {
    SesameBleCipher ssm;
};

typedef struct {
    uint8_t device_uuid[16];
    uint8_t public_key[64];
    uint8_t device_secret[16];
    uint8_t addr[6];
    uint8_t device_status;
    union ssm_cipher cipher;
    uint8_t mech_status[7];
    uint16_t c_offset;
    uint8_t b_buf[80]; /// max command size is register(80 Bytes).
    uint8_t conn_id;
} sesame;

typedef void (*ssm_action)(sesame * ssm);

struct ssm_env_tag {
    sesame ssm;
    uint8_t number;
    ssm_action ssm_cb__;
};

#pragma pack()

extern struct ssm_env_tag * p_ssms_env;

void ssm_init(ssm_action ssm_action_cb);

void ssm_disconnect(sesame * ssm);

void ssm_ble_receiver(sesame * ssm, const uint8_t * p_data, uint16_t len);

void talk_to_ssm(sesame * ssm, uint8_t parsing_type);

#ifdef __cplusplus
}
#endif

#endif // __SSM_LINK_H__
