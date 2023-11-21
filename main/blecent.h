#pragma once

#include <host/ble_gap.h>
#include <host/ble_gatt.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ssm.h"

void esp_ble_init(void);

void esp_ble_gatt_write(sesame * ssm, uint8_t * value, uint16_t length);

#ifdef __cplusplus
}
#endif
