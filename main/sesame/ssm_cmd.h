#ifndef __SSM_CMD_H__
#define __SSM_CMD_H__

#include "ssm_link.h"

#ifdef __cplusplus
extern "C" {
#endif

void ssm_readHistoryCommand(sesame * ssm);

void ssm_unlock_all(uint8_t * tag, uint8_t tag_length);

void ssm_lock_all(uint8_t * tag, uint8_t tag_length);

void ssm_toggle_all(uint8_t * tag, uint8_t tag_length);

void login_sesame(sesame * ssm);

void register_sesame(sesame * ssm);

void handle_reg_ssm(sesame * ssm);

#ifdef __cplusplus
}
#endif

#endif // __SSM_CMD_H__
