/**
************************************************************
* @file         gizwits_product.c
* @brief        Control protocol processing, and platform-related hardware initialization
* @author       Gizwits
* @date         2017-07-19
* @version      V03030000
* @copyright    Gizwits
*
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值ֵ|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/
#include <stdio.h>
#include <string.h>
#include "gizwits_product.h"
#include "driver/dc1.h"
/** User area The current device state structure */
dataPoint_t currentDataPoint;

/**@name Gizwits User Interface
* @{
*/

/**
* @brief Event handling interface

* Description:

* 1. Users can customize the changes in WiFi module status

* 2. Users can add data points in the function of event processing logic, such as calling the relevant hardware peripherals operating interface

* @param [in] info: event queue
* @param [in] data: protocol data
* @param [in] len: protocol data length
* @return NULL
* @ref gizwits_protocol.h
*/
int8_t ICACHE_FLASH_ATTR gizwitsEventProcess(eventInfo_t *info, uint8_t *data, uint32_t len)
{
    uint8_t i = 0;
    dataPoint_t * dataPointPtr = (dataPoint_t *)data;
    moduleStatusInfo_t * wifiData = (moduleStatusInfo_t *)data;

    if((NULL == info) || (NULL == data))
    {
        GIZWITS_LOG("!!! gizwitsEventProcess Error \n");
        return -1;
    }

    for(i = 0; i < info->num; i++)
    {
        switch(info->event[i])
        {
        case EVENT_switch0 :
            currentDataPoint.valueswitch0 = dataPointPtr->valueswitch0;
            GIZWITS_LOG("Evt: EVENT_switch0 %d \n", currentDataPoint.valueswitch0);
            if(0x01 == currentDataPoint.valueswitch0)
            {
                set_switch(0,1);
            }
            else
            {
                set_switch(0,0);
                currentDataPoint.valueswitch1 = 0;
                currentDataPoint.valueswitch2 = 0;
                currentDataPoint.valueswitch3 = 0;
                gizwitsHandle(&currentDataPoint);
            }
            break;
        case EVENT_switch1 :
            currentDataPoint.valueswitch1 = dataPointPtr->valueswitch1;
            GIZWITS_LOG("Evt: EVENT_switch1 %d \n", currentDataPoint.valueswitch1);
            if(0x01 == currentDataPoint.valueswitch1)
            {
                set_switch(1,1);
                currentDataPoint.valueswitch0 = 1;
                gizwitsHandle(&currentDataPoint);
            }
            else
            {
                set_switch(1,0);
            }
            break;
        case EVENT_switch2 :
            currentDataPoint.valueswitch2 = dataPointPtr->valueswitch2;
            GIZWITS_LOG("Evt: EVENT_switch2 %d \n", currentDataPoint.valueswitch2);
            if(0x01 == currentDataPoint.valueswitch2)
            {
                set_switch(2,1);
                currentDataPoint.valueswitch0 = 1;
                gizwitsHandle(&currentDataPoint);
            }
            else
            {
                set_switch(2,0);
            }
            break;
        case EVENT_switch3 :
            currentDataPoint.valueswitch3 = dataPointPtr->valueswitch3;
            GIZWITS_LOG("Evt: EVENT_switch3 %d \n", currentDataPoint.valueswitch3);
            if(0x01 == currentDataPoint.valueswitch3)
            {
                set_switch(3,1);
                currentDataPoint.valueswitch0 = 1;
                gizwitsHandle(&currentDataPoint);
            }
            else
            {
                set_switch(3,0);
            }
            break;



        case WIFI_SOFTAP:
            break;
        case WIFI_AIRLINK:
            break;
        case WIFI_STATION:
            break;
        case WIFI_CON_ROUTER:
            GIZWITS_LOG("@@@@ connected router\n");
 
            break;
        case WIFI_DISCON_ROUTER:
            GIZWITS_LOG("@@@@ disconnected router\n");
 
            break;
        case WIFI_CON_M2M:
            GIZWITS_LOG("@@@@ connected m2m\n");
			setConnectM2MStatus(0x01);
 
            break;
        case WIFI_DISCON_M2M:
            GIZWITS_LOG("@@@@ disconnected m2m\n");
			setConnectM2MStatus(0x00);
 
            break;
        case WIFI_RSSI:
            GIZWITS_LOG("@@@@ RSSI %d\n", wifiData->rssi);
            break;
        case TRANSPARENT_DATA:
            GIZWITS_LOG("TRANSPARENT_DATA \n");
            break;
        case MODULE_INFO:
            GIZWITS_LOG("MODULE INFO ...\n");
            break;
            
        default:
            break;
        }
    }
    system_os_post(USER_TASK_PRIO_2, SIG_UPGRADE_DATA, 0);
    
    return 0; 
}


/**
* User data acquisition

* Here users need to achieve in addition to data points other than the collection of data collection, can be self-defined acquisition frequency and design data filtering algorithm

* @param none
* @return none
*/
void ICACHE_FLASH_ATTR userHandle(void)
{

    uint16_t electric_data[3];  
    get_electric_data(electric_data);
    
    currentDataPoint.valuevoltage = (float)electric_data[0]/10;
    currentDataPoint.valuecurrent = (float)electric_data[1]/10;
    currentDataPoint.valuepower = (float)electric_data[2]/10;

    system_os_post(USER_TASK_PRIO_2, SIG_UPGRADE_DATA, 0);
}


/**
* Data point initialization function

* In the function to complete the initial user-related data
* @param none
* @return none
* @note The developer can add a data point state initialization value within this function
*/
void ICACHE_FLASH_ATTR userInit(void)
{
    gizMemset((uint8_t *)&currentDataPoint, 0, sizeof(dataPoint_t));

 	/** Warning !!! DataPoint Variables Init , Must Within The Data Range **/ 
    
    currentDataPoint.valueswitch0 = false;
    currentDataPoint.valueswitch1 = false;
    currentDataPoint.valueswitch2 = false;
    currentDataPoint.valueswitch3 = false;
    currentDataPoint.valuecurrent = 0;
    currentDataPoint.valuevoltage = 0;
    currentDataPoint.valuepower = 0;
    
}


