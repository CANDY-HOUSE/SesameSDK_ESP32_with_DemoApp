#ifndef __SSM_CMD_H__
#define __SSM_CMD_H__

#include "ssm.h"

#ifdef __cplusplus
extern "C" {
#endif

void send_read_history_cmd_to_ssm(sesame * ssm);

void ssm_unlock_all(uint8_t * tag, uint8_t tag_length);

void ssm_lock_all(uint8_t * tag, uint8_t tag_length);

void ssm_toggle_all(uint8_t * tag, uint8_t tag_length);

void send_login_cmd_to_ssm(sesame * ssm);

void send_reg_cmd_to_ssm(sesame * ssm);

void handle_reg_data_from_ssm(sesame * ssm);

#ifdef __cplusplus
}
#endif

#endif // __SSM_CMD_H__
