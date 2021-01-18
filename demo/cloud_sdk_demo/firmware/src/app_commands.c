/*******************************************************************************
  Sample Application

  File Name:
    app_commands.c

  Summary:
    commands for the tcp client demo app.

  Description:
    
 *******************************************************************************/


// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

#include "tcpip/tcpip.h"
#include "app.h"
#include "app_common.h"
#include "app_commands.h"
#include "config.h"
#include <wolfssl/ssl.h>
#include "task.h"
#include "wolfcrypt/error-crypt.h"
#include "cryptoauthlib.h"
#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_assoc.h"
#include "system/debug/sys_debug.h"

//******************************************************************************

#if defined(TCPIP_STACK_COMMAND_ENABLE)

static void _APP_Commands_GetUnixTime(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_GetRSSI(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_GetRTCC(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_SetDebugLevel(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);

//******************************************************************************

static const SYS_CMD_DESCRIPTOR appCmdTbl[] = {
    {"unixtime", _APP_Commands_GetUnixTime, ": Unix Time"},
    {"rssi", _APP_Commands_GetRSSI, ": Get current RSSI"},
    {"rtcc", _APP_Commands_GetRTCC, ": Get uptime"},
    {"debug", _APP_Commands_SetDebugLevel, ": Set debug level"},
};

//******************************************************************************

static void rssiCallback(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE assocHandle, int8_t rssi) {
    APP_CMD_PRNT("Connected RSSI: %d \r\n", rssi);
}

//******************************************************************************

bool APP_Commands_Init() {
    if (!SYS_CMD_ADDGRP(appCmdTbl, sizeof (appCmdTbl) / sizeof (*appCmdTbl), "app", ": app commands")) {
        SYS_ERROR(SYS_ERROR_ERROR, "Failed to create TCPIP Commands\r\n", 0);
        return false;
    }
    return true;
}

void _APP_Commands_SetDebugLevel(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    SYS_ERROR_LEVEL debugLevel = atoi(argv[1]);
    if (argc == 2 && (0 <= argv[1] <= 4)) {
        SYS_DEBUG_ErrorLevelSet(debugLevel);
        APP_CMD_PRNT("Debug level set to %d \r\n", debugLevel);
    }
    else{
        APP_CMD_PRNT("Usage: debug <debug level 0:4>\r\n");
    }
}

void _APP_Commands_GetRSSI(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    if (WIFI_IS_CONNECTED) {
        WDRV_PIC32MZW_STATUS ret;
        ret = WDRV_PIC32MZW_AssocRSSIGet(appData.assocHandle, NULL, rssiCallback);
        if (ret != WDRV_PIC32MZW_STATUS_RETRY_REQUEST) {
            APP_CMD_PRNT("Failed getting RSSI. Driver error (%d)\r\n", ret);
        }
    } else {
        APP_CMD_PRNT("RSSI: WI-Fi not connected.\r\n");
    }
}

void _APP_Commands_GetRTCC(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    //struct tm *sys_time = &app_controlData.rtccData.sys_time;
    //APP_CMD_PRNT("RTCC: %d-%d-%d %d:%d:%d\r\n", sys_time->tm_mday, sys_time->tm_mon, sys_time->tm_year, sys_time->tm_hour, sys_time->tm_min, sys_time->tm_sec);
}

void _APP_Commands_GetUnixTime(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    uint32_t sec = TCPIP_SNTP_UTCSecondsGet();
    APP_CMD_PRNT("Time from SNTP: %d\r\n", sec);
    APP_CMD_PRNT("Low Rez Timer: %d\r\n", SYS_TIME_CounterGet() /
            SYS_TIME_FrequencyGet());
}

#endif
