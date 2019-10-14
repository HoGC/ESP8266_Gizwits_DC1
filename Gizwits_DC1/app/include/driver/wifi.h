/*
 * wifi driver
 * Author: HoGC 
 */
#ifndef USER_WIFI_H_
#define USER_WIFI_H_
#include "ets_sys.h"
#include "os_type.h"


#define AP_INFO_MAX		2

typedef void (*wifconnect_cb_t)(void);
typedef void (*wifdisconnect_cb_t)(void);

void ICACHE_FLASH_ATTR wifi_connect(uint8_t* ssid, uint8_t* pass);
void ICACHE_FLASH_ATTR set_wifistate_cb(wifconnect_cb_t connect_cb, wifdisconnect_cb_t disconnect_cb);

#endif /* _WIFI_H_ */
