/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

#include "app.h"
#include "app_wifi_prov.h"
#ifdef APP_USB_MSD
#include "app_usb_msd.h"
#endif /* APP_USB_MSD */
#ifdef AZURE_CLOUD_DEMO
    #include "app_azure.h"
#elif defined AWS_CLOUD_DEMO
    #include "app_aws.h"
#endif
#include "app_ctrl.h"
#include "app_commands.h"
#include "app_common.h"
#include "tcpip/tcpip_manager.h"

// *****************************************************************************

/* Wi-Fi connect callback */
static void wifiConnectCallback(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE assocHandle, WDRV_PIC32MZW_CONN_STATE currentState)
{
    switch (currentState) {
        case WDRV_PIC32MZW_CONN_STATE_DISCONNECTED:
            APP_PRNT("WiFi Reconnecting\r\n");
            appData.assocHandle = NULL;
            WIFI_DISCONNECTED;
            appData.wlanTaskState = APP_WLAN_RECONNECT;
            break;
        case WDRV_PIC32MZW_CONN_STATE_CONNECTED:
            APP_PRNT("WiFi Connected\r\n");
            appData.assocHandle = assocHandle;
            WIFI_CONNECTED;
            break;
        case WDRV_PIC32MZW_CONN_STATE_FAILED:
            APP_PRNT("WiFi connection failed\r\n");
            appData.assocHandle = NULL;
            WIFI_DISCONNECTED;
            appData.wlanTaskState = APP_WLAN_RECONNECT;        
            break;
        case WDRV_PIC32MZW_CONN_STATE_CONNECTING:
            break;
    }
}

/* TCP/IP stack event callback */
static void tcpipStackEventHandler(TCPIP_NET_HANDLE hNet, TCPIP_EVENT event, const void *fParam)
{
    if(event & TCPIP_EV_CONN_ESTABLISHED)
    {
        APP_DBG(SYS_ERROR_DEBUG, "%s - TCPIP_EV_CONN_ESTABLISHED\r\n",__func__);
        if(appData.appMode == APP_MODE_STA)   //STA
        {
            APP_manageLed(LED_BLUE, LED_ON, BLINK_MODE_INVALID);
            if(TCPIP_DHCP_IsEnabled(hNet) == false)
            {
                APP_DBG(SYS_ERROR_INFO, "Start DHCP Client\r\n");
                if(TCPIP_DHCPS_IsEnabled(hNet))
                    TCPIP_DHCPS_Disable(hNet);
                TCPIP_DHCP_Enable(hNet);
            }
            else
                APP_DBG(SYS_ERROR_INFO, "Already DHCP Client running...!\r\n");
        }
        else    //AP
        {
            AP_CONNECTED;       
            if(TCPIP_DHCPS_IsEnabled(hNet) == false)
            {
                APP_DBG(SYS_ERROR_INFO, "Start DHCP server\r\n");
                if(TCPIP_DHCP_IsEnabled(hNet))
                    TCPIP_DHCP_Disable(hNet);
                TCPIP_DHCPS_Enable(hNet);
            }
            else
                APP_DBG(SYS_ERROR_INFO, "Already DHCP server running...!\r\n");  
        }
    }
    else if(event & TCPIP_EV_CONN_LOST)
    {
        APP_DBG(SYS_ERROR_DEBUG, "%s - TCPIP_EV_CONN_LOST\r\n", __func__);
        APP_manageLed(LED_BLUE, LED_OFF, BLINK_MODE_INVALID);
        if(appData.appMode == APP_MODE_STA)   //STA
        {
            APP_DBG(SYS_ERROR_INFO, "Stop DHCP Client\r\n");
            TCPIP_DHCP_Disable(hNet);
            
            /* Re-connect */
            appData.wlanTaskState = APP_WLAN_CONFIG;
        }
        else    //AP
        {
            APP_DBG(SYS_ERROR_INFO, "Stop DHCP server\r\n");
            TCPIP_DHCPS_Disable(hNet);
            AP_DISCONNECTED; 
        }
    }
    else
    {
        APP_DBG(SYS_ERROR_ERROR, "%s Unknown event event = %d\r\n", __func__, event);
    }
}

/* DHCP event callback */
void tcpipDhcpEventHandler(TCPIP_NET_HANDLE hNet, TCPIP_DHCP_EVENT_TYPE evType, const void* param)
{
    switch(evType)
    {
        case DHCP_EVENT_BOUND:
        {
            TCPIP_DHCP_INFO DhcpInfo;
            if(TCPIP_DHCP_InfoGet(hNet, &DhcpInfo))
            {
                APP_PRNT("Received IP Address: = %d.%d.%d.%d \r\n",
                        DhcpInfo.dhcpAddress.v[0], DhcpInfo.dhcpAddress.v[1], DhcpInfo.dhcpAddress.v[2], DhcpInfo.dhcpAddress.v[3]);
                APP_PRNT("DHCP Server Address: = %d.%d.%d.%d \r\n",
                        DhcpInfo.serverAddress.v[0], DhcpInfo.serverAddress.v[1], DhcpInfo.serverAddress.v[2], DhcpInfo.serverAddress.v[3]);
                    
                IP_ADDR_OBTAINED;
            }
            break;
        }
        
        case DHCP_EVENT_CONN_ESTABLISHED:
        {
            APP_DBG(SYS_ERROR_INFO, "%s - connection to the DHCP server re-established\r\n", __func__);
            break;
        }
        
        case DHCP_EVENT_CONN_LOST:
        {
            APP_DBG(SYS_ERROR_INFO, "%s - connection to the DHCP server lost\r\n", __func__);
            IP_ADDR_LOST;
            break;
        }
        
        default:
            break;
    }
}

// *****************************************************************************

void *APP_Calloc(size_t num, size_t size) {
    void *p = NULL;

    if (num != 0 && size != 0) {
        p = OSAL_Malloc(size * num);

        if (p != NULL) {
            memset(p, 0, size * num);
        }
    }
    return p;
}

/* Store Wi-Fi configurations to global g_wifiConfig struct */
bool APP_WifiConfig(char *ssid, char *pass, WIFI_AUTH auth, uint8_t channel)
{
    bool ret = true;
    uint8_t ssidLength = strlen((const char *) ssid);
    uint8_t pskLength = strlen(pass);

    WDRV_PIC32MZW_BSSCtxSetChannel(&g_wifiConfig.bssCtx, (WDRV_PIC32MZW_CHANNEL_ID)channel);
    APP_DBG(SYS_ERROR_INFO, "SSID is %s \r\n", ssid);

    if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSCtxSetSSID(&g_wifiConfig.bssCtx, (uint8_t * const) ssid, ssidLength)) {
        switch (auth) {
            case OPEN:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetOpen(&g_wifiConfig.authCtx)) {
                    APP_DBG(SYS_ERROR_ERROR, "APP_WFI: ERROR- Unable to set Authentication\r\n");
                    ret = false;
                }
                break;
            }
            case WPA2:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&g_wifiConfig.authCtx, (uint8_t *) pass, pskLength, WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL)) {
                    APP_DBG(SYS_ERROR_ERROR, "ERROR -Unable to set authentication to WPA2\r\n");
                    ret = false;
                }
                break;
            }
            case WPAWPA2MIXED:
            {
                if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&g_wifiConfig.authCtx, (uint8_t *) pass, pskLength, WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL)) {
                    APP_DBG(SYS_ERROR_ERROR, "ERROR - Unable to set authentication to WAPWPA2 MIXED\r\n");
                    ret = false;
                }
                break;
            }
            case WEP:
            {
                //TODO
                break;
            }
            default:
            {
                APP_DBG(SYS_ERROR_ERROR, "ERROR-Set Authentication type");
                ret = false;
            }
        }
    }
    return ret;
}

// *****************************************************************************

void APP_Initialize ( void )
{    
    SET_WIFI_CREDENTIALS(VALID_CREDENTIALS_SRC_AP,CREDENTIALS_UNINITIALIZED);
    SET_WIFI_CREDENTIALS(VALID_CREDENTIALS_SRC_USB_MSD,CREDENTIALS_UNINITIALIZED);    
    APP_InitializeWifiProv();
    APP_InitializeUSBMSD();
    APP_InitializeWlan();
    APP_InitializeCtrl();
    APP_Commands_Init();
}

void APP_InitializeWlan ( void )
{
    appData.wlanTaskState = APP_WLAN_INIT;
    appData.assocHandle = NULL;
    WIFI_DISCONNECTED;
    NTP_NOT_DONE;
    IP_ADDR_LOST;
    appData.appMode = APP_MODE_STA; //STA
    SW1_PRESSED(false);
    SW2_PRESSED(false);
    if(SWITCH1_Get() == SWITCH1_STATE_PRESSED)
         SW1_PRESSED(true);
    if(SWITCH2_Get() == SWITCH1_STATE_PRESSED)
         SW2_PRESSED(true);
    WDRV_PIC32MZW_BSSCtxSetDefaults(&g_wifiConfig.bssCtx);   
}

// *****************************************************************************

void APP_TaskWlan(void)
{
    SYS_STATUS tcpipStat;
    bool status;
    TCPIP_NET_HANDLE netH;
    int i, nNets;

    switch (appData.wlanTaskState) 
    {
        /* Wait for Wi-Fi module init */
        case APP_WLAN_INIT:
        {
            if (SYS_STATUS_READY == WDRV_PIC32MZW_Status(sysObj.drvWifiPIC32MZW1)) {
                APP_DBG(SYS_ERROR_INFO, "WiFi driver opened!\r\n");
                appData.wlanTaskState = APP_WLAN_WDRV_INIT_READY;
            }
            break;
        }

        /* Wi-Fi module open */
        case APP_WLAN_WDRV_INIT_READY:
        {
            appData.wdrvHandle = WDRV_PIC32MZW_Open(0, 0);
            if (DRV_HANDLE_INVALID != appData.wdrvHandle) {
                APP_DBG(SYS_ERROR_DEBUG, "wait for TCP/IP stack init!\r\n");
                appData.wlanTaskState = APP_WLAN_WAIT_FOR_TCPIP_INIT;
            }
            break;
        }
    
        /* TCP/IP stack init*/
        case APP_WLAN_WAIT_FOR_TCPIP_INIT:
        {
            tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);

            if (tcpipStat < 0) { // some error occurred
                APP_DBG(SYS_ERROR_ERROR, "TCP/IP stack initialization failed!\r\n");
                appData.wlanTaskState = APP_WLAN_ERROR;
            } else if (SYS_STATUS_READY == tcpipStat) {
                nNets = TCPIP_STACK_NumberOfNetworksGet();
                for (i = 0; i < nNets; i++) {
                    netH = TCPIP_STACK_IndexToNet(i);
                    appData.TCPIPEventHandle = TCPIP_STACK_HandlerRegister(netH, TCPIP_EV_CONN_ALL, tcpipStackEventHandler, NULL);
                    appData.TCPIPDHCPHandle = TCPIP_DHCP_HandlerRegister (netH, tcpipDhcpEventHandler, NULL);
                }

                appData.wlanTaskState = APP_WLAN_CHECK_CREDENTIALS;
            }
            break;
        }
        
        /* Check on Wi-Fi from USB MSD */
        case APP_WLAN_CHECK_CREDENTIALS:
        {
            if (SW1_IS_PRESSED && !SW2_IS_PRESSED){
                APP_PRNT("Go to provisioning mode\r\n");
                if (SW1_IS_PRESSED)
                    SW1_PRESSED(false);
                appData.appMode = APP_MODE_AP; 
                appData.wlanTaskState = APP_WLAN_WAIT_VALID_CREDENTIALS; 
                SECONDARY_TASKS_GO_PROV;              
            }
            else if(CHECK_WIFI_CREDENTIALS(VALID_CREDENTIALS_SRC_USB_MSD) == CREDENTIALS_VALID){
                APP_PRNT("Go to normal mode\r\n");
                appData.appMode = APP_MODE_STA;
                appData.wlanTaskState = APP_WLAN_CONFIG;                 
                /* Put other tasks in idle state*/
                SECONDARY_TASKS_GO_IDLE;
            }
            break; 
        }
       
        /* WLAN task pending for AP provisioning (or USB MSD provisioning) */
        case APP_WLAN_WAIT_VALID_CREDENTIALS:
        {  
            if(CHECK_WIFI_CREDENTIALS(VALID_CREDENTIALS_SRC_AP) == CREDENTIALS_VALID)
            {
                APP_DBG(SYS_ERROR_INFO, "WLAN task got valid credentials\r\n");
                SECONDARY_TASKS_PROV_DONE;
                appData.wlanTaskState = APP_WLAN_SYNC_WITH_AP;
            }
            break;
        }
        
        /* Wait until AP is disabled */
        case APP_WLAN_SYNC_WITH_AP:
        {
            if(SECONDARY_TASKS_WAIT_SYNC)
                break;
            APP_DBG(SYS_ERROR_DEBUG, "Synced with AP task\r\n");
            appData.wlanTaskState = APP_WLAN_CONFIG;
            break;
        }
         
        /* Configure and connect */
        case APP_WLAN_CONFIG:
        {
            APP_PRNT("Connecting to Wi-Fi (%s)\r\n",wifi.ssid);
            APP_manageLed(LED_BLUE, LED_F_BLINK, BLINK_MODE_PERIODIC);
            if (APP_WifiConfig((char*)wifi.ssid, 
                                (char*)wifi.key, 
                                (WIFI_AUTH)wifi.auth, 
                                WDRV_PIC32MZW_CID_ANY)) 
            {
                appData.appMode = APP_MODE_STA;
                if (WDRV_PIC32MZW_STATUS_OK == WDRV_PIC32MZW_BSSConnect(appData.wdrvHandle, 
                                                                        &g_wifiConfig.bssCtx, 
                                                                        &g_wifiConfig.authCtx, 
                                                                        wifiConnectCallback)) 
                {
                    appData.wlanTaskState = APP_WLAN_WAIT_FOR_SNTP_INIT;
                    break;
                }
        
            }
            APP_DBG(SYS_ERROR_ERROR, "Failed connecting to Wi-Fi\r\n");
            appData.wlanTaskState = APP_WLAN_ERROR;
            break;
        }

        /* Wait for SNTP*/
        case APP_WLAN_WAIT_FOR_SNTP_INIT:
        {
            TCPIP_SNTP_RESULT res = TCPIP_SNTP_TimeStampStatus();
            if(SNTP_RES_OK == res)
            {
                APP_DBG(SYS_ERROR_INFO, "SNTP initialized\r\n");
                NTP_DONE;
                appData.wlanTaskState = APP_WLAN_IDLE;
            }
            break;
        }
        
        /* Idle */
        case APP_WLAN_IDLE:
        {
            break;
        }
        
        /* Reconnect */
        case APP_WLAN_RECONNECT:
        {
            TCPIP_STACK_HandlerDeregister(appData.TCPIPEventHandle);
            TCPIP_DHCP_HandlerDeRegister(appData.TCPIPDHCPHandle);
            WDRV_PIC32MZW_Close(appData.wdrvHandle);
            appData.wlanTaskState = APP_WLAN_INIT;
            break;
        }      
        
        /* Error */
        case APP_WLAN_ERROR:
        {
            break;
        }
        
        default:
        {
            break;
        }
    }
}



/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    APP_TaskCtrl();
    APP_TaskWlan();
    APP_TaskWifiProv();
    APP_TaskTcpServer();
    APP_TaskUSBMSD();
}


/*******************************************************************************
 End of File
 */
