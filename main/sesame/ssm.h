#ifndef __SESAME_H__
#define __SESAME_H__

#include "candy.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SSM_MAX_NUM (4u)

#define SSM_KEY_LENGTH (offsetof(sesame, addr) + 6)
#pragma pack(1)

typedef struct {
    int64_t count;
    uint8_t nouse;
    uint8_t tk_app_ssm[4];
} SS5_CCM_NONCE;

extern uint8_t additional_data[];

typedef struct {
    uint8_t ccm_key[16];
    SS5_CCM_NONCE encrypt;
    SS5_CCM_NONCE decrypt;
} Sesame5BleCipher;

union ssm_cipher {
    Sesame5BleCipher ss5;
};

typedef struct {
    uint8_t device_uuid[16];
    uint8_t ss5_pub_key[64];
    uint8_t device_secret[16];
    uint8_t addr[6];
    uint8_t ss5_device_status;
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

void ssm_lock(sesame * ssm, uint8_t * tag, uint8_t tag_length);

void ssm_unlock(sesame * ssm, uint8_t * tag, uint8_t tag_length);

void ssm_unlock_all(uint8_t * tag, uint8_t tag_length);

void ssm_lock_all(uint8_t * tag, uint8_t tag_length);

void ssm_toggle_all(uint8_t * tag, uint8_t tag_length);

void ssm_disconnect(sesame * ssm);

void ssm_say_handler(const uint8_t * p_data, uint16_t len, uint8_t conn_id);

void talk_to_ssm(sesame * ssm, uint8_t parsing_type);

void add_ssm(uint8_t * addr);

#ifdef __cplusplus
}
#endif

#endif // __SESAME_H__
