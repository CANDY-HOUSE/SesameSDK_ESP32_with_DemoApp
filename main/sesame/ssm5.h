#ifndef __SESAME_V5_H__
#define __SESAME_V5_H__

#include "ssm.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t ecc_private_esp32[32];

void ssm_readHistoryCommand(sesame * ssm);

void ssm_unlock_all(uint8_t * tag, uint8_t tag_length);

void ssm_lock_all(uint8_t * tag, uint8_t tag_length);

void ssm_toggle_all(uint8_t * tag, uint8_t tag_length);

void login_sesame(sesame * ssm);

void register_sesame(sesame * ssm);

#ifdef __cplusplus
}
#endif

#endif // __SESAME_V5_H__
