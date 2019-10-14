/*
 * wifi driver
 * Author: HoGC 
 */
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "user_config.h"
#include "smartconfig.h"
#include "airkiss.h"
#include "driver/wifi.h"

//#define WIFI_DEBUG_ON

#if defined(WIFI_DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

wifconnect_cb_t w_connect = NULL;
wifdisconnect_cb_t w_disconnect = NULL;


bool connect_flag = 0;
os_timer_t OS_Timer_WifiTryConnect;

void ICACHE_FLASH_ATTR user_wifi_handle_event_cb(System_Event_t *evt)
{
    switch	(evt->event)	{
    case EVENT_STAMODE_CONNECTED:
        INFO("connect to ssid %s\n",evt->event_info.connected.ssid);
		connect_flag = 1;
		if(w_connect != NULL){
			w_connect();
		}
		os_timer_disarm(&OS_Timer_WifiTryConnect);	// 关闭定时器
        break;
    case EVENT_STAMODE_DISCONNECTED:
        INFO("disconnect from	ssid %s\n",evt->event_info.disconnected.ssid);
		if(connect_flag == 1){
			connect_flag = 0;
			if(w_disconnect != NULL){
				w_disconnect();
			}
			os_timer_arm(&OS_Timer_WifiTryConnect, 3000, 1);  // 使能定时器
		}
        break;
    default:
        break;
    }
}

/**
 * 尝试连接flash存储的wifi信息   
 * @retval None
 */
void ICACHE_FLASH_ATTR wifi_try_connect(void) {
	int ap_id;
	ap_id = wifi_station_get_current_ap_id();
	ap_id = ++ap_id % AP_INFO_MAX;
	INFO("AP_ID : %d", ap_id);
	wifi_station_disconnect();
	wifi_station_ap_change(ap_id);
}


/**
 * 连接目标AP
 * @param  ssid: 名字
 * @param  pass: 密码
 * @retval None
 */
void ICACHE_FLASH_ATTR wifi_connect(uint8_t* ssid, uint8_t* pass){

	struct station_config stationConf;

	wifi_set_opmode_current(STATION_MODE);

	os_memset(&stationConf, 0, sizeof(struct station_config));

	os_sprintf(stationConf.ssid, "%s", ssid);
	os_sprintf(stationConf.password, "%s", pass);

	wifi_station_set_config_current(&stationConf);

	wifi_station_connect();
}

/**
 * 设置wifi连接、断开回调，并自动尝试连接以储存的多个WIFI 
 * @param  u_connect_cb: 连接回调函数
 * @param  u_disconnect_cb: 断开连接回调函数
 * @retval None
 */
void ICACHE_FLASH_ATTR set_wifistate_cb(wifconnect_cb_t u_connect_cb, wifdisconnect_cb_t u_disconnect_cb){

	w_connect = u_connect_cb;
	w_disconnect = u_disconnect_cb;

	int i;
	struct station_config config[5];

	//设置wifi信息存储数量
	wifi_station_ap_number_set(AP_INFO_MAX);

	int ap_info_count = wifi_station_get_ap_info(config);
	INFO("ap_info_count = %d\n", ap_info_count);
	for(i = 0; i < ap_info_count; i++)
	{
		INFO("AP%d\n", i);
		INFO("ssid : %s\n", config[i].ssid);
		INFO("password : %s\n", config[i].password);
	}

	wifi_set_event_handler_cb(user_wifi_handle_event_cb);

	os_timer_disarm(&OS_Timer_WifiTryConnect);	// 关闭定时器
	os_timer_setfn(&OS_Timer_WifiTryConnect, (os_timer_func_t *) wifi_try_connect,NULL);// 设置定时器
	os_timer_arm(&OS_Timer_WifiTryConnect, 3000, 1);  // 使能定时器
}
