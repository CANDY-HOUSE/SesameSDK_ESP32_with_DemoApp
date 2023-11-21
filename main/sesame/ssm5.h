#ifndef __SESAME_V5_H__
#define __SESAME_V5_H__

#include "ssm.h"

#ifdef __cplusplus
extern "C" {
#endif

void ss5_lock(sesame * ss5, uint8_t * tag, uint8_t tag_len);

void ss5_unlock(sesame * ss5, uint8_t * tag, uint8_t tag_len);

void ss5_toggle(sesame * ss5, uint8_t * tag, uint8_t tag_len);

void ss5_parse_publish(sesame * ss5, uint8_t cmd_it_code);

void ss5_parse_response(sesame * ss5, uint8_t cmd_it_code);

void ss5_ble_receiver(sesame * ss5, const uint8_t * p_data, uint16_t len);

void ss5_readHistoryCommand(sesame * ss5);

#ifdef __cplusplus
}
#endif

#endif // __SESAME_V5_H__
