#ifndef __BLECENT_H__
#define __BLECENT_H__

#include <host/ble_gap.h>
#include <host/ble_gatt.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ssm.h"

void esp_ble_gatt_write(sesame * ssm, uint8_t * value, uint16_t length);

void esp_ble_init(void);

#ifdef __cplusplus
}
#endif

#endif // __BLECENT_H__
