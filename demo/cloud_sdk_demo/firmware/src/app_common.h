/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_COMMON_H
#define _APP_COMMON_H

// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "wdrv_pic32mzw.h"
#include "wdrv_pic32mzw_sta.h"
#include "wdrv_pic32mzw_authctx.h"
#include "wdrv_pic32mzw_bssctx.h"
#include "wdrv_pic32mzw_bssfind.h"
#include "wdrv_pic32mzw_common.h"
#include "app_ctrl.h"
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************

typedef enum 
{
    OPEN = 0,
    WPA2,
    WPAWPA2MIXED,
    WEP,
    NONE
} WIFI_AUTH;

typedef struct wifiConfiguration {
    WDRV_PIC32MZW_AUTH_CONTEXT authCtx;
    WDRV_PIC32MZW_BSS_CONTEXT bssCtx;
} wifiConfig;
wifiConfig g_wifiConfig;

typedef struct wifiCredentials {
    uint8_t ssid[WDRV_PIC32MZW_MAX_SSID_LEN];
    uint8_t auth;
    uint8_t key[WDRV_PIC32MZW_MAX_PSK_PASSWORD_LEN];
} wifiCred;
wifiCred wifi;

// *****************************************************************************

/* Default STA credentials */
#define APP_STA_DEFAULT_SSID        "MCHP.IOT"
#define APP_STA_DEFAULT_PASSPHRASE  "microchip"
#define APP_STA_DEFAULT_AUTH        2

#define MACADDRESS_LENGTH   6

/* Wi-Fi credentials source*/
#define VALID_CREDENTIALS_SRC_AP        0
#define VALID_CREDENTIALS_SRC_USB_MSD   1

/* Wi-Fi credentials status*/
#define CREDENTIALS_UNINITIALIZED       0
#define CREDENTIALS_VALID               1
#define CREDENTIALS_INVALID             2

/* Wi-Fi credentials access APIs */
#define SET_WIFI_CREDENTIALS(src,val)   appData.ValidCrednetials[src] = val
#define CHECK_WIFI_CREDENTIALS(src)     appData.ValidCrednetials[src]

/* Wi-Fi connection, IP address and NTP status */
#define WIFI_CONNECTED      appData.isConnected = true
#define WIFI_DISCONNECTED   appData.isConnected = false
#define WIFI_IS_CONNECTED   (appData.isConnected)
#define IP_ADDR_OBTAINED    appData.isIPObtained = true
#define IP_ADDR_LOST        appData.isIPObtained = false
#define IP_ADDR_IS_OBTAINED (appData.isIPObtained)
#define NTP_DONE            appData.isNTPDone = true
#define NTP_NOT_DONE        appData.isNTPDone = false
#define NTP_IS_DONE         (appData.isNTPDone)

/* AP connection status */
#define AP_CONNECTED        appWifiProvData.apReady = true;
#define AP_DISCONNECTED     appWifiProvData.apReady = false;
#define AP_IS_CONNECTED     (appWifiProvData.apReady)

/* User button status */
#define SW1_PRESSED(status) appData.sw1Pressed = status
#define SW1_IS_PRESSED      (appData.sw1Pressed)
#define SW2_PRESSED(status) appData.sw2Pressed = status
#define SW2_IS_PRESSED      (appData.sw2Pressed)


/* No AP provisioning needed */
#define SECONDARY_TASKS_GO_IDLE \
        if(appWifiProvData.wifiProvTaskState < APP_WIFI_PROV_AP_ENABLE) \
            appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_IDLE; \
        if(appWifiProvData.tcpServerTaskState < APP_TCP_SERVER_OPEN_SOCKET) \
            appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_IDLE; \

/* Terminate AP provisioning */
#define SECONDARY_TASKS_GO_PROV \
        if(appWifiProvData.wifiProvTaskState < APP_WIFI_PROV_AP_ENABLE) \
            appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_AP_ENABLE; \
        else \
            SYS_DEBUG_PRINT(SYS_ERROR_WARNING, "[APP]: WARNING: wifiProvTaskState suspected state %d\r\n", \
                    appWifiProvData.wifiProvTaskState); \

/* Prepare for AP provisioning */
#define SECONDARY_TASKS_PROV_DONE \
        if(appWifiProvData.wifiProvTaskState == APP_WIFI_PROV_AP_ENABLED) \
            appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_AP_DISABLE; \
        else \
            SYS_DEBUG_PRINT(SYS_ERROR_WARNING, "[APP]: WARNING: wifiProvTaskState suspected state %d\r\n", \
                    appWifiProvData.wifiProvTaskState); \

/* WLAN task sync with AP task */
#define SECONDARY_TASKS_WAIT_SYNC \
        !(appWifiProvData.wifiProvTaskState == APP_WIFI_PROV_IDLE)
                    

// *****************************************************************************

bool APP_NVM_EraseWriteWiFiCredentials(uint32_t*, bool);
bool APP_WifiConfig(char *ssid, char *pass, WIFI_AUTH auth, uint8_t channel);

#endif /* _APP_COMMON_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

