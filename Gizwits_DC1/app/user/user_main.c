#include "ets_sys.h"
#include "os_type.h"
#include "c_types.h"
#include "osapi.h"
#include "mem.h"
#include "gpio.h"
#include "user_interface.h"
#include "gagent_soc.h"
#include "user_devicefind.h"
#include "user_webserver.h"
#include "gizwits_product.h"
#include "driver/dc1_switch.h"
#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

#ifdef SERVER_SSL_ENABLE
#include "ssl/cert.h"
#include "ssl/private_key.h"
#else
#ifdef CLIENT_SSL_ENABLE
unsigned char *default_certificate;
unsigned int default_certificate_len = 0;
unsigned char *default_private_key;
unsigned int default_private_key_len = 0;
#endif
#endif


#define MAIN_DEBUG_ON

#if defined(MAIN_DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

bool smartconfig_mode = false;
os_timer_t OS_LED_Timer;

os_timer_t OS_Timer_Wifistate;
uint8 smartconfig_flag = 0;
bool connect_flag = 0;

//wifi信号灯闪动
void ICACHE_FLASH_ATTR led_flash(void){
    LOCAL bool status=0;
    wifi_led_switch(status);
    status=~status;
}

//总开关短按
void ICACHE_FLASH_ATTR key0_short(bool status){

    u8 i=0;
    INFO("key0ShortPress\n");
	if(status){
		currentDataPoint.valueswitch0 = 1;
		gizwitsHandle(&currentDataPoint);
	}else{
        currentDataPoint.valueswitch0 = 0;
        currentDataPoint.valueswitch1 = 0;
        currentDataPoint.valueswitch2 = 0;
        currentDataPoint.valueswitch3 = 0;
		gizwitsHandle(&currentDataPoint);
    }
    
}

//总开关长按
void ICACHE_FLASH_ATTR key0_long(void){
    INFO("key0LongPress\n");
	smartconfig_mode = true;
    os_timer_arm(&OS_LED_Timer, 100, 1);
    gizwitsSetMode(WIFI_AIRLINK_MODE);
}

//开关1按键短按
void ICACHE_FLASH_ATTR key1_short(bool status){
	INFO("key1LongPress\n");
    if(status){
        currentDataPoint.valueswitch0 = 1;
        currentDataPoint.valueswitch1 = 1;
		gizwitsHandle(&currentDataPoint);
    }else{
        currentDataPoint.valueswitch1 = 0;
		gizwitsHandle(&currentDataPoint);
    }
}

//开关2按键短按
void ICACHE_FLASH_ATTR key2_short(bool status){
	INFO("key2LongPress\n");
    if(status){
        currentDataPoint.valueswitch0 = 1;
        currentDataPoint.valueswitch2 = 1;
		gizwitsHandle(&currentDataPoint);
    }else{
        currentDataPoint.valueswitch2 = 0;
		gizwitsHandle(&currentDataPoint);
    }
}

//开关3按键短按
void ICACHE_FLASH_ATTR key3_short(bool status){
	INFO("key3LongPress\n");
    if(status){
        currentDataPoint.valueswitch0 = 1;
        currentDataPoint.valueswitch3 = 1;
		gizwitsHandle(&currentDataPoint);
    }else{
        currentDataPoint.valueswitch3 = 0;
		gizwitsHandle(&currentDataPoint);
    }
}

/**
 * 	WIFI连接回调
 */
void wifi_connect(void){

	INFO("wifi connect!\r\n");
	os_timer_disarm(&OS_LED_Timer);
	wifi_led_switch(1);

	uint8_t i;
	uint8_t gpio_ret;
	uint8_t switch_bit=0x80;

	gpio_ret = dc1_read_gpio();
	INFO("\n-------------------\n",currentDataPoint.valueswitch0);
	INFO("gpio_ret:%02X",gpio_ret);

	currentDataPoint.valueswitch0 = (gpio_ret & switch_bit) == switch_bit;
	INFO("valueswitch0:%d\n",currentDataPoint.valueswitch0);
	switch_bit = switch_bit >> 1;
	currentDataPoint.valueswitch1 = (gpio_ret & switch_bit) == switch_bit;
	INFO("valueswitch0:%d\n",currentDataPoint.valueswitch0);
	switch_bit = switch_bit >> 1;
	currentDataPoint.valueswitch2 = (gpio_ret & switch_bit) == switch_bit;
	INFO("valueswitch0:%d\n",currentDataPoint.valueswitch0);
	switch_bit = switch_bit >> 1;
	currentDataPoint.valueswitch3 = (gpio_ret & switch_bit) == switch_bit;
	INFO("valueswitch0:%d\n",currentDataPoint.valueswitch0);
	gizwitsHandle(&currentDataPoint);

}

/**
 * 	WIFI断开回调
 */
void wifi_disconnect(void){
	INFO("wifi disconnect!\r\n");
	if(smartconfig_mode != true){
		os_timer_arm(&OS_LED_Timer, 500, 1);
	}
}

/**
 * WIFI连接状态检查
 */
void ICACHE_FLASH_ATTR wifi_check(void) {
	uint8 getState;
	LOCAL uint8 count = 0;
	LOCAL uint8 ap_id = 0;
	struct ip_info ipConfig;
	if (smartconfig_flag != 1) {
		wifi_get_ip_info(STATION_IF, &ipConfig);
		getState = wifi_station_get_connect_status();
		//查询 ESP8266 WiFi station 接口连接 AP 的状态
		if (getState == STATION_GOT_IP && ipConfig.ip.addr != 0) {
			if (connect_flag == 0) {
				count = 0;
				connect_flag = 1;
                smartconfig_mode = false;
				INFO("wifi connect!\r\n");
				os_timer_arm(&OS_Timer_Wifistate, 5000, 1);
				wifi_connect();
			}
		} else {
			count++;
			if (count > 10) {
				count = 0;
				ap_id = wifi_station_get_current_ap_id();
				ap_id = ++ap_id % 2;
				INFO("AP_ID : %d", ap_id);
				wifi_station_ap_change(ap_id);
			}
			if(connect_flag == 1){
				connect_flag = 0;
				INFO("wifi disconnect!\r\n");
				os_timer_arm(&OS_Timer_Wifistate, 1000, 1);
				wifi_disconnect();
			}

		}
	}
}

//wifi连接检查
void ICACHE_FLASH_ATTR set_wifi_check(void){

	int i;
	struct station_config config[5];
	int	count = wifi_station_get_ap_info(config);
	INFO("ap_num = %d\n", count);
	for(i = 0; i < count; i++)
	{
		INFO("AP%d\n", i+1);
		INFO("ssid : %s\n", config[i].ssid);
		INFO("password : %s\n", config[i].password);
	}

	os_timer_disarm(&OS_Timer_Wifistate);	// 关闭定时器
	os_timer_setfn(&OS_Timer_Wifistate, (os_timer_func_t *) wifi_check, NULL);// 设置定时器
	os_timer_arm(&OS_Timer_Wifistate, 500, 1);  // 使能定时器
}


/**
* @brief user_rf_cal_sector_set

* Use the 636 sector (2544k ~ 2548k) in flash to store the RF_CAL parameter
* @param none
* @return none
*/
uint32_t ICACHE_FLASH_ATTR user_rf_cal_sector_set()
{
    return 636;
}

/**
* @brief program entry function

* In the function to complete the user-related initialization
* @param none
* @return none
*/
void ICACHE_FLASH_ATTR user_init(void)
{
    uint32_t system_free_size = 0;

    wifi_station_set_auto_connect(1);
    wifi_station_ap_number_set(2);
    wifi_set_sleep_type(NONE_SLEEP_T);//set none sleep mode
    espconn_tcp_set_max_con(10);
    uart_init_3(115200,115200);
    UART_SetPrintPort(0);

    gizwitsInit();  

    set_wifi_check();

	dc1_init(key0_short,key0_long,key1_short,NULL,key2_short,NULL,key3_short,NULL);

    os_timer_disarm(&OS_LED_Timer);
    os_timer_setfn(&OS_LED_Timer, (os_timer_func_t *)led_flash, NULL);
    os_timer_arm(&OS_LED_Timer, 500, 1);
}
