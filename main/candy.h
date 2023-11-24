#ifndef __CANDY_H__
#define __CANDY_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SSM_OP_CODE_STR(op_code) ((op_code) == 7 ? "response" : (op_code) == 8 ? "publish" : "unknown")
#define SSM_ITEM_CODE_STR(code)                                                                                                                                                                                                                               \
    ((code) == SSM_ITEM_CODE_NONE                        ? "SSM_ITEM_CODE_NONE"                                                                                                                                                                               \
         : (code) == SSM_ITEM_CODE_REGISTRATION          ? "SSM_ITEM_CODE_REGISTRATION"                                                                                                                                                                       \
         : (code) == SSM_ITEM_CODE_LOGIN                 ? "SSM_ITEM_CODE_LOGIN"                                                                                                                                                                              \
         : (code) == SSM_ITEM_CODE_USER                  ? "SSM_ITEM_CODE_USER"                                                                                                                                                                               \
         : (code) == SSM_ITEM_CODE_HISTORY               ? "SSM_ITEM_CODE_HISTORY"                                                                                                                                                                            \
         : (code) == SSM_ITEM_CODE_VERSION_DETAIL        ? "SSM_ITEM_CODE_VERSION_DETAIL"                                                                                                                                                                     \
         : (code) == SSM_ITEM_CODE_DISCONNECT_REBOOT_NOW ? "SSM_ITEM_CODE_DISCONNECT_REBOOT_NOW"                                                                                                                                                              \
         : (code) == SSM_ITEM_CODE_ENABLE_DFU            ? "SSM_ITEM_CODE_ENABLE_DFU"                                                                                                                                                                         \
         : (code) == SSM_ITEM_CODE_TIME                  ? "SSM_ITEM_CODE_TIME"                                                                                                                                                                               \
         : (code) == SSM_ITEM_CODE_INITIAL               ? "SSM_ITEM_CODE_INITIAL"                                                                                                                                                                            \
         : (code) == SSM_ITEM_CODE_MAGNET                ? "SSM_ITEM_CODE_MAGNET"                                                                                                                                                                             \
         : (code) == SSM_ITEM_CODE_MECH_SETTING          ? "SSM_ITEM_CODE_MECH_SETTING"                                                                                                                                                                       \
         : (code) == SSM_ITEM_CODE_MECH_STATUS           ? "SSM_ITEM_CODE_MECH_STATUS"                                                                                                                                                                        \
         : (code) == SSM_ITEM_CODE_LOCK                  ? "SSM_ITEM_CODE_LOCK"                                                                                                                                                                               \
         : (code) == SSM_ITEM_CODE_UNLOCK                ? "SSM_ITEM_CODE_UNLOCK"                                                                                                                                                                             \
         : (code) == SSM2_ITEM_OPS_TIMER_SETTING         ? "SSM2_ITEM_OPS_TIMER_SETTING"                                                                                                                                                                      \
                                                         : "UNKNOWN_ITEM_CODE")
#define SSM_STATUS_STR(status)                                                                                                                                                                                                                                \
    ((status) == SSM_NOUSE              ? "NOUSE"                                                                                                                                                                                                             \
         : (status) == SSM_DISCONNECTED ? "DISCONNECTED"                                                                                                                                                                                                      \
         : (status) == SSM_SCANNING     ? "SCANNING"                                                                                                                                                                                                          \
         : (status) == SSM_CONNECTING   ? "CONNECTING"                                                                                                                                                                                                        \
         : (status) == SSM_CONNECTED    ? "CONNECTED"                                                                                                                                                                                                         \
         : (status) == SSM_LOGGIN       ? "LOGGIN"                                                                                                                                                                                                            \
         : (status) == SSM_LOCKED       ? "LOCKED"                                                                                                                                                                                                            \
         : (status) == SSM_UNLOCKED     ? "UNLOCKED"                                                                                                                                                                                                          \
         : (status) == SSM_MOVED        ? "MOVED"                                                                                                                                                                                                             \
                                        : "status_error")

#define SSM_MAX_CHAC_LEN (BLE_MAX_OCTETS - 4 - 3)
#define CCM_TAG_LENGTH (4)

#define SSM_SEG_PARSING_TYPE_APPEND_ONLY (0)
#define SSM_SEG_PARSING_TYPE_PLAINTEXT (1)
#define SSM_SEG_PARSING_TYPE_CIPHERTEXT (2)

typedef enum {
    SESAME_5 = 5,
    SESAME_BIKE_2 = 6,
    SESAME_5_PRO = 7,
} candy_product_type;

typedef enum {
    SSM_NOUSE = 0,
    SSM_DISCONNECTED = 1,
    SSM_SCANNING = 2,
    SSM_CONNECTING = 3,
    SSM_CONNECTED = 4,
    SSM_LOGGIN = 5,
    SSM_LOCKED = 6,
    SSM_UNLOCKED = 7,
    SSM_MOVED = 8,
} device_status_t;

typedef enum {
    SSM_OP_CODE_RESPONSE = 0x07,
    SSM_OP_CODE_PUBLISH = 0x08,
} ssm_op_code_e;

typedef enum {
    SSM_ITEM_CODE_NONE = 0,
    SSM_ITEM_CODE_REGISTRATION = 1,
    SSM_ITEM_CODE_LOGIN = 2,
    SSM_ITEM_CODE_USER = 3,
    SSM_ITEM_CODE_HISTORY = 4,
    SSM_ITEM_CODE_VERSION_DETAIL = 5,
    SSM_ITEM_CODE_DISCONNECT_REBOOT_NOW = 6,
    SSM_ITEM_CODE_ENABLE_DFU = 7,
    SSM_ITEM_CODE_TIME = 8,
    SSM_ITEM_CODE_INITIAL = 14,
    SSM_ITEM_CODE_MAGNET = 17,
    SSM_ITEM_CODE_MECH_SETTING = 80,
    SSM_ITEM_CODE_MECH_STATUS = 81,
    SSM_ITEM_CODE_LOCK = 82,
    SSM_ITEM_CODE_UNLOCK = 83,
    SSM2_ITEM_OPS_TIMER_SETTING = 92,
} ssm_item_code_e;

#ifdef __cplusplus
}
#endif

#endif /* __CANDY_H__ */
