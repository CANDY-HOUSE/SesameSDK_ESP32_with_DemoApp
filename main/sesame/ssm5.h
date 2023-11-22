#ifndef __SESAME_V5_H__
#define __SESAME_V5_H__

#include "ssm.h"

#ifdef __cplusplus
extern "C" {
#endif

void ss5_lock(sesame * ssm, uint8_t * tag, uint8_t tag_len);

void ss5_unlock(sesame * ssm, uint8_t * tag, uint8_t tag_len);

void ssm_ble_receiver(sesame * ssm, const uint8_t * p_data, uint16_t len);

void ss5_readHistoryCommand(sesame * ssm);

#ifdef __cplusplus
}
#endif

#endif // __SESAME_V5_H__
