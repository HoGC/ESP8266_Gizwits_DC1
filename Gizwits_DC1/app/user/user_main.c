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
#include "driver/uart.h"
#include "driver/wifi.h"
#include "driver/dc1.h"
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


//#define MIAN_DEBUG_ON

#if defined(MIAN_DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

bool smartconfig_mode = false;
os_timer_t OS_Timer_LED;
os_timer_t OS_Timer_AIRLINK;

//WIFI信号灯闪动
void ICACHE_FLASH_ATTR led_flash(void){
    static bool status=0;
    wifi_led_switch(status);
    status=~status;
}

//配网超时、停止配网模式
void ICACHE_FLASH_ATTR stop_airlink(void){
	smartconfig_mode = false;
	gizwitsSetMode(WIFI_RESET_MODE);
	os_timer_arm(&OS_Timer_LED, 500, 1);
}

//总开关短按回调
void ICACHE_FLASH_ATTR key0_short(bool status){
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

//总开关长按回调
void ICACHE_FLASH_ATTR key0_long(void){
    INFO("key0LongPress\n");
	smartconfig_mode = true;
    os_timer_arm(&OS_Timer_LED, 100, 1);
    gizwitsSetMode(WIFI_AIRLINK_MODE);
	os_timer_disarm(&OS_Timer_AIRLINK);
    os_timer_setfn(&OS_Timer_AIRLINK, (os_timer_func_t *)stop_airlink, NULL);
    os_timer_arm(&OS_Timer_AIRLINK, 60000, 0);
}

//开关1短按回调
void ICACHE_FLASH_ATTR key1_short(bool status){
    if(status){
        currentDataPoint.valueswitch0 = 1;
        currentDataPoint.valueswitch1 = 1;
		gizwitsHandle(&currentDataPoint);
    }else{
        currentDataPoint.valueswitch1 = 0;
		gizwitsHandle(&currentDataPoint);
    }
}

//开关2短按回调
void ICACHE_FLASH_ATTR key2_short(bool status){
    if(status){
        currentDataPoint.valueswitch0 = 1;
        currentDataPoint.valueswitch2 = 1;
		gizwitsHandle(&currentDataPoint);
    }else{
        currentDataPoint.valueswitch2 = 0;
		gizwitsHandle(&currentDataPoint);
    }
}

//开关3短按回调
void ICACHE_FLASH_ATTR key3_short(bool status){
    if(status){
        currentDataPoint.valueswitch0 = 1;
        currentDataPoint.valueswitch3 = 1;
		gizwitsHandle(&currentDataPoint);
    }else{
        currentDataPoint.valueswitch3 = 0;
		gizwitsHandle(&currentDataPoint);
    }
}


//WIFI连接回调
void wifi_connect_cb(void){
	INFO("wifi connect!\r\n");
	if(smartconfig_mode){
		smartconfig_mode = false;
		os_timer_disarm(&OS_Timer_AIRLINK);
	}
	os_timer_disarm(&OS_Timer_LED);
	wifi_led_switch(1);
}

//WIFI断开回调
void wifi_disconnect_cb(void){
	INFO("wifi disconnect!\r\n");
	if(smartconfig_mode != true){
		os_timer_arm(&OS_Timer_LED, 500, 1);
	}
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
    wifi_set_sleep_type(NONE_SLEEP_T);//set none sleep mode
    espconn_tcp_set_max_con(10);

    UART_SetPrintPort(1);
	uart_init(4800, 115200);
	os_delay_us(60000);
	UART_SetParity(0,EVEN_BITS);
	system_uart_swap();
	set_uart_cb(dc1_uart_data_handler);

    gizwitsInit();  

    set_wifistate_cb(wifi_connect_cb, wifi_disconnect_cb);

	dc1_init(key0_short,key0_long,key1_short,NULL,key2_short,NULL,key3_short,NULL);

    os_timer_disarm(&OS_Timer_LED);
    os_timer_setfn(&OS_Timer_LED, (os_timer_func_t *)led_flash, NULL);
    os_timer_arm(&OS_Timer_LED, 500, 1);
}
