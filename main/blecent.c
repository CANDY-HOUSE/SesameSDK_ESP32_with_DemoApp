#include "blecent.h"
#include "candy.h"
#include "esp_central.h"
#include "esp_log.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"

#define BLECENT_SVC_ALERT_UUID (0xFD81)

struct ble_hs_adv_fields;
struct ble_gap_conn_desc;
struct ble_hs_cfg;
union ble_store_value;
union ble_store_key;

static const ble_uuid_t * sesame5_svc_uuid = BLE_UUID16_DECLARE(BLECENT_SVC_ALERT_UUID);
static const ble_uuid_t * sesame5_chr_uuid = BLE_UUID128_DECLARE(0x3e, 0x99, 0x76, 0xc6, 0xb4, 0xdb, 0xd3, 0xb6, 0x56, 0x98, 0xae, 0xa5, 0x02, 0x00, 0x86, 0x16);
static const ble_uuid_t * sesame5_ntf_uuid = BLE_UUID128_DECLARE(0x3e, 0x99, 0x76, 0xc6, 0xb4, 0xdb, 0xd3, 0xb6, 0x56, 0x98, 0xae, 0xa5, 0x03, 0x00, 0x86, 0x16);

static const char * TAG = "blecent.c";

static int blecent_on_write(uint16_t conn_handle) {
    const struct peer_dsc * dsc;
    uint8_t value[2];
    const struct peer * peer = peer_find(conn_handle);

    dsc = peer_dsc_find_uuid(peer, BLE_UUID16_DECLARE(BLECENT_SVC_ALERT_UUID), sesame5_ntf_uuid, BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
    if (dsc == NULL) {
        ESP_LOGE(TAG,
                 "Error: Peer lacks a CCCD for the Unread Alert "
                 "Status characteristic\n");
        goto err;
    }
    ESP_LOGI("blecent_on_write", "[conn_handle: %d][dsc->dsc.handle: %d]", conn_handle, dsc->dsc.handle);
    value[0] = 1;
    value[1] = 0;
    int rc   = ble_gattc_write_flat(conn_handle, dsc->dsc.handle, value, sizeof value, NULL, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG,
                 "Error: Failed to subscribe to characteristic; "
                 "rc=%d\n",
                 rc);
        goto err;
    }
    return ESP_OK;
err:
    return ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM); /* Terminate the connection. */
}

static void blecent_on_service_disc_complete(const struct peer * peer, int status, void * arg) {
    ESP_LOGI(TAG, "blecent_on_service_disc_complete");
    if (status != 0) {
        ESP_LOGE(TAG,
                 "Error: Service discovery failed; status=%d "
                 "conn_handle=%d\n",
                 status, peer->conn_handle);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return;
    }
    ESP_LOGI(TAG,
             "Service discovery complete; status=%d "
             "conn_handle=%d\n",
             status, peer->conn_handle);
    blecent_on_write(peer->conn_handle);
}

static int ble_gap_event_connect_handle(struct ble_gap_event * event, void * arg, struct ble_gap_conn_desc * desc) {
    ESP_LOGI(TAG, "[conn_handle: %d][event->connect.status: %d]", event->connect.conn_handle, event->connect.status);
    if (event->connect.status != 0) {
        ESP_LOGE(TAG, "Error: Connection failed; status=%d\n", event->connect.status);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connection established ");
    int rc = ble_gap_conn_find(event->connect.conn_handle, desc);
    assert(rc == 0);
    print_conn_desc(desc);
    ESP_LOGI(TAG, "get peer info success");
    rc = peer_add(event->connect.conn_handle);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to add peer; rc=%d\n", rc);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "ss5 conn_id: %d", event->connect.conn_handle);
    for (uint8_t i = 0; i < SSM_MAX_NUM; i++) {
        ESP_LOGI(TAG, "ssm[%d].addr", i);
        ESP_LOG_BUFFER_HEX_LEVEL("[ssm.addr]", p_ssms_env->ssm[i].addr, 6, ESP_LOG_INFO);
        if (memcmp(p_ssms_env->ssm[i].addr, (*desc).peer_id_addr.val, 6) == 0) {
            ESP_LOGI(TAG, "ssm[%d] connected", i);
            p_ssms_env->ssm[i].device_status = SSM_CONNECTED;              // set the device status
            p_ssms_env->ssm[i].conn_id       = event->connect.conn_handle; // save the connection handle
            break;
        }
    }
    rc = peer_disc_all(event->connect.conn_handle, blecent_on_service_disc_complete, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to discover services; rc=%d\n", rc);
        return ESP_FAIL;
    }

    return ESP_OK;
}

static int ble_gap_connect_event(struct ble_gap_event * event, void * arg) {
    // ESP_LOGI(TAG, "[ble_gap_connect_event: %d]", event->type);
    static struct ble_gap_conn_desc desc;
    int rc; // return code

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        return ble_gap_event_connect_handle(event, arg, &desc);

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "disconnect; reason=%d ", event->disconnect.reason);
        print_conn_desc(&event->disconnect.conn);
        peer_delete(event->disconnect.conn.conn_handle);
        return ESP_OK;

    case BLE_GAP_EVENT_CONN_UPDATE_REQ:
        printf("connection update request event; "
               "conn_handle=%d itvl_min=%d itvl_max=%d "
               "latency=%d supervision_timoeut=%d "
               "min_ce_len=%d max_ce_len=%d\n",
               event->conn_update_req.conn_handle, event->conn_update_req.peer_params->itvl_min, event->conn_update_req.peer_params->itvl_max, event->conn_update_req.peer_params->latency, event->conn_update_req.peer_params->supervision_timeout,
               event->conn_update_req.peer_params->min_ce_len, event->conn_update_req.peer_params->max_ce_len);
        *event->conn_update_req.self_params = *event->conn_update_req.peer_params;
        return ESP_OK;

    case BLE_GAP_EVENT_NOTIFY_RX:
        ssm_say_handler(event->notify_rx.om->om_data, event->notify_rx.om->om_len, event->notify_rx.conn_handle); // talk to sesame5
        return ESP_OK;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "mtu update event; conn_handle=%d cid=%d mtu=%d\n", event->mtu.conn_handle, event->mtu.channel_id, event->mtu.value);
        return ESP_OK;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        ESP_LOGI(TAG, "BLE_GAP_EVENT_REPEAT_PAIRING");
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        assert(rc == 0);
        ble_store_util_delete_peer(&desc.peer_id_addr);
        return BLE_GAP_REPEAT_PAIRING_RETRY;

    default:
        return ESP_OK;
    }
}

static void blecent_connect_sesame5(const struct ble_hs_adv_fields * fields, void * disc) {
    ble_addr_t * addr = &((struct ble_gap_disc_desc *) disc)->addr;
    int8_t rssi       = ((struct ble_gap_disc_desc *) disc)->rssi;
    if (rssi < -60) { // RSSI threshold
        return;
    }
    if (fields->mfg_data[0] == 0x5A && fields->mfg_data[1] == 0x05) { // SSM
        if (fields->mfg_data[4] == 0x00) {                            // unregistered SSM
            ESP_LOGI(TAG, "find unregistered SSM[%d]", fields->mfg_data[2]);
            ESP_LOG_BUFFER_HEX_LEVEL("find SSM", addr->val, 6, ESP_LOG_INFO);
            add_ssm(addr->val);
        } else { // registered SSM
            ESP_LOGW(TAG, "find registered SSM[%d]", fields->mfg_data[2]);
            return;
        }
    } else { // not SSM
        // ESP_LOGW(TAG, "not SSM !! ");
        return;
    }
    ble_gap_disc_cancel(); /* Scanning must be stopped before a connection can be nitiated. */
    addr = &((struct ble_gap_disc_desc *) disc)->addr;
    ESP_LOGI(TAG, "addr_type=%d addr=%s", addr->type, addr_str(addr->val));
    int rc = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, addr, 10000, NULL, ble_gap_connect_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error: Failed to connect to device; rc=%d\n", rc);
        return;
    }
}

static int ble_gap_disc_event(struct ble_gap_event * event, void * arg) {
    // ESP_LOGI(TAG, "[ble_gap_disc_event: %d]", event->type);
    struct ble_hs_adv_fields fields;
    int rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
    if (rc != 0) {
        return ESP_FAIL;
    }
    blecent_connect_sesame5(&fields, &event->disc);
    return ESP_OK;
}

static void blecent_scan(void) {
    ESP_LOGI("[conn_ssm]", "esp32 central scan device");

    struct ble_gap_disc_params disc_params;
    disc_params.filter_duplicates = 1;
    disc_params.passive           = 1;
    disc_params.itvl              = 0;
    disc_params.window            = 0;
    disc_params.filter_policy     = 0;
    disc_params.limited           = 0;
    int rc                        = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &disc_params, ble_gap_disc_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error initiating GAP discovery procedure; rc=0x%x\n", rc);
    }
}

static void blecent_host_task(void * param) {
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void esp_ble_init(void) {
    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init nimble %d ", ret);
        return;
    }
    ble_hs_cfg.sync_cb = blecent_scan;
    int rc             = peer_init(SSM_MAX_NUM, 64, 64, 64);
    assert(rc == 0);
    nimble_port_freertos_init(blecent_host_task);
}

void esp_ble_gatt_write(sesame * ssm, uint8_t * value, uint16_t length) {
    const struct peer_chr * chr;
    const struct peer * peer;
    peer = peer_find(ssm->conn_id);
    chr  = peer_chr_find_uuid(peer, sesame5_svc_uuid, sesame5_chr_uuid);
    if (chr == NULL) {
        ESP_LOGE(TAG, "Error: Peer doesn't have the subscribable characteristic\n");
        return;
    }
    int rc = ble_gattc_write_flat(ssm->conn_id, chr->chr.val_handle, value, length, NULL, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG,
                 "Error: Failed to write to the subscribable characteristic; "
                 "rc=%d\n",
                 rc);
    }
}
