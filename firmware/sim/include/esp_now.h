/**
 * esp_now.h – Simulation shim using UDP sockets.
 *
 * ESP-NOW packets are sent/received over localhost UDP:
 *   - Transmitter listens on port 6001, sends to port 6002
 *   - Receiver   listens on port 6002, sends to port 6001
 *
 * Compile with -DSIM_TX for the transmitter role or -DSIM_RX for receiver.
 */

#ifndef SIM_ESP_NOW_H
#define SIM_ESP_NOW_H

#include <cstdint>

/* ── ESP-NOW types ──────────────────────────────────────────────────────── */

#define ESP_NOW_ETH_ALEN 6
#define ESP_NOW_MAX_DATA_LEN 250

typedef enum {
    ESP_NOW_SEND_SUCCESS = 0,
    ESP_NOW_SEND_FAIL    = 1,
} esp_now_send_status_t;

typedef void (*esp_now_recv_cb_t)(const uint8_t *mac, const uint8_t *data, int len);
typedef void (*esp_now_send_cb_t)(const uint8_t *mac, esp_now_send_status_t status);

typedef struct {
    uint8_t peer_addr[ESP_NOW_ETH_ALEN];
    uint8_t channel;
    bool encrypt;
} esp_now_peer_info_t;

/* ── ESP-NOW API ────────────────────────────────────────────────────────── */

int esp_now_init();
int esp_now_deinit();
int esp_now_register_recv_cb(esp_now_recv_cb_t cb);
int esp_now_register_send_cb(esp_now_send_cb_t cb);
int esp_now_send(const uint8_t *peer_addr, const uint8_t *data, int len);
int esp_now_add_peer(const esp_now_peer_info_t *peer);
int esp_now_del_peer(const uint8_t *peer_addr);
bool esp_now_is_peer_exist(const uint8_t *peer_addr);

/* ── Simulation control ─────────────────────────────────────────────────── */

#ifdef SIM_TX
#define SIM_LISTEN_PORT 6001
#define SIM_SEND_PORT   6002
#else
#define SIM_LISTEN_PORT 6002
#define SIM_SEND_PORT   6001
#endif

#endif /* SIM_ESP_NOW_H */
