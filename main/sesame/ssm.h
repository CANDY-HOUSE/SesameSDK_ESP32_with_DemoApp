#ifndef __SESAME_H__
#define __SESAME_H__

#include "candy.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SSM_MAX_NUM (4u)

#pragma pack(1)

typedef struct {
    int64_t count;
    uint8_t nouse;
    uint8_t tk_app_ssm[4];
} SSM_CCM_NONCE;

typedef struct {
    uint8_t ccm_key[16];
    SSM_CCM_NONCE encrypt;
    SSM_CCM_NONCE decrypt;
} SesameBleCipher;

union ssm_cipher {
    SesameBleCipher ss5;
};

typedef struct {
    uint8_t device_uuid[16];
    uint8_t public_key[64];
    uint8_t device_secret[16];
    uint8_t addr[6];
    uint8_t device_status;
    union ssm_cipher cipher;
    uint8_t mechStatus[7];
    uint16_t c_offset;
    uint8_t b_buf[80]; /// max command size is register(80 Bytes).
    uint8_t conn_id;
} sesame;

typedef void (*ssm_action)(sesame * ssm);

struct ssm_env_tag {
    sesame ssm[SSM_MAX_NUM];
    uint8_t number;
    ssm_action ssm_cb__;
};

#pragma pack()

extern struct ssm_env_tag * p_ssms_env;

void ssm_init(ssm_action ssm_action_cb);

void ssm_disconnect(sesame * ssm);

void ssm_say_handler(const uint8_t * p_data, uint16_t len, uint8_t conn_id);

void talk_to_ssm(sesame * ssm, uint8_t parsing_type);

#ifdef __cplusplus
}
#endif

#endif // __SESAME_H__
