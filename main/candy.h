#ifndef _CANDY_H_
#define _CANDY_H_

#include <stdint.h> // for uint8_t

#ifdef __cplusplus
extern "C" {
#endif

#define SSM_MAX_CHAC_LEN (BLE_MAX_OCTETS - 4 - 3)
#define CCM_TAG_LENGTH (4)

#define SSM5_SEG_PARSING_TYPE_APPEND_ONLY (0)
#define SSM5_SEG_PARSING_TYPE_PLAINTEXT (1)
#define SSM5_SEG_PARSING_TYPE_CIPHERTEXT (2)

typedef enum {
    SESAME_5      = 5,
    SESAME_BIKE_2 = 6,
    SESAME_5_PRO  = 7,
} candy_product_type;

typedef enum {
    SSM5_NOUSE        = 0,
    SSM5_DISCONNECTED = 1,
    SSM5_SCANNING     = 2,
    SSM5_CONNECTING   = 3,
    SSM5_CONNECTED    = 4,
    SSM5_LOGGIN       = 5,
    SSM5_LOCKED       = 6,
    SSM5_UNLOCKED     = 7,
    SSM5_MOVED        = 8,
} ss5_device_status;

#define SSM_STATUS_STR(status)                                                                                                                                                                                                                                \
    ((status) == SSM5_NOUSE              ? "NOUSE"                                                                                                                                                                                                            \
         : (status) == SSM5_DISCONNECTED ? "DISCONNECTED"                                                                                                                                                                                                     \
         : (status) == SSM5_SCANNING     ? "SCANNING"                                                                                                                                                                                                         \
         : (status) == SSM5_CONNECTING   ? "CONNECTING"                                                                                                                                                                                                       \
         : (status) == SSM5_CONNECTED    ? "CONNECTED"                                                                                                                                                                                                        \
         : (status) == SSM5_LOGGIN       ? "LOGGIN"                                                                                                                                                                                                           \
         : (status) == SSM5_LOCKED       ? "LOCKED"                                                                                                                                                                                                           \
         : (status) == SSM5_UNLOCKED     ? "UNLOCKED"                                                                                                                                                                                                         \
         : (status) == SSM5_MOVED        ? "MOVED"                                                                                                                                                                                                            \
                                         : "status_error")

typedef enum {
    SSM5_OP_CODE_RESPONSE = 0x07,
    SSM5_OP_CODE_PUBLISH  = 0x08,
} ssm5_op_code_e;

typedef enum {
    SSM5_ITEM_CODE_NONE                  = 0,
    SSM5_ITEM_CODE_REGISTRATION          = 1,
    SSM5_ITEM_CODE_LOGIN                 = 2,
    SSM5_ITEM_CODE_USER                  = 3,
    SSM5_ITEM_CODE_HISTORY               = 4,
    SSM5_ITEM_CODE_VERSION_DETAIL        = 5,
    SSM5_ITEM_CODE_DISCONNECT_REBOOT_NOW = 6,
    SSM5_ITEM_CODE_ENABLE_DFU            = 7,
    SSM5_ITEM_CODE_TIME                  = 8,
    SSM5_ITEM_CODE_BLE_CONNECTION_PARAM  = 9,
    SSM5_ITEM_CODE_BLE_ADV_PARAM         = 10,
    SSM5_ITEM_CODE_AUTOLOCK              = 11,
    SSM5_ITEM_CODE_SERVER_ADV_KICK       = 12,
    SSM5_ITEM_CODE_SESAME_TOKEN          = 13,
    SSM5_ITEM_CODE_INITIAL               = 14,
    SSM5_ITEM_CODE_IRER                  = 15,
    SSM5_ITEM_CODE_TIMEPHONE             = 16,
    SSM5_ITEM_CODE_MAGNET                = 17,
    SSM5_ITEM_CODE_BLE_ADV_PARAM_GET     = 18, /// nouse
    SSM5_ITEM_CODE_SENSOR_INVERVAL       = 19, /// nouse
    SSM5_ITEM_CODE_SENSOR_INVERVAL_GET   = 20, /// nouse

    SSM5_ITEM_CODE_MECH_SETTING = 80,
    SSM5_ITEM_CODE_MECH_STATUS  = 81,
    SSM5_ITEM_CODE_LOCK         = 82,
    SSM5_ITEM_CODE_UNLOCK       = 83,
    SSM5_ITEM_CODE_MOVE_TO,
    SSM5_ITEM_CODE_DRIVE_DIRECTION,
    SSM5_ITEM_CODE_STOP,
    SSM5_ITEM_CODE_DETECT_DIR,
    SSM5_ITEM_CODE_TOGGLE = 88,
    SSM5_ITEM_CODE_CLICK  = 89,

} ssm5_item_code_e;

#ifdef __cplusplus
}
#endif

#endif /* _CANDY_H_ */
