#ifndef __SESAME_V5_H__
#define __SESAME_V5_H__

#include "ssm.h"

#ifdef __cplusplus
extern "C" {
#endif

void ssm_ble_receiver(sesame * ssm, const uint8_t * p_data, uint16_t len);

void ss5_readHistoryCommand(sesame * ssm);

void ssm_unlock_all(uint8_t * tag, uint8_t tag_length);

void ssm_lock_all(uint8_t * tag, uint8_t tag_length);

void ssm_toggle_all(uint8_t * tag, uint8_t tag_length);

#ifdef __cplusplus
}
#endif

#endif // __SESAME_V5_H__
