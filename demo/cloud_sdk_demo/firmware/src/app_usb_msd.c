/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_usb_msd.c

  Summary:
    This file contains the source code for the MPLAB Harmony application code related to USB MSD functionality.

  Description:
 
 *******************************************************************************/

// *****************************************************************************

#include "app.h"
#include "app_common.h"
#include "app_usb_msd.h"
#include "cJSON.h"
#include "wdrv_pic32mzw_client_api.h"
#include "wolfcrypt/asn.h"
#include "atca_basic.h"

#ifdef AZURE_CLOUD_DEMO
    #include "app_azure.h"
#elif defined AWS_CLOUD_DEMO
    #include "app_aws.h"
#endif

// *****************************************************************************

#define APP_CTRL_CLIENTID_SIZE ((2 * KEYID_SIZE) + 1)

// *****************************************************************************

uint8_t CACHE_ALIGN work[SYS_FS_FAT_MAX_SS];
extern char g_Cloud_Endpoint[100];
#ifdef AWS_CLOUD_DEMO
    extern char g_Aws_ClientID[CLIENT_IDENTIFIER_MAX_LENGTH];
#endif

// *****************************************************************************

/* USB event callback */
void USBDeviceEventHandler(USB_DEVICE_EVENT event, void * pEventData, uintptr_t context)
{
    APP_USB_MSD_DATA * appData = (APP_USB_MSD_DATA*) context;
    switch (event) {
    case USB_DEVICE_EVENT_RESET:
    case USB_DEVICE_EVENT_DECONFIGURED:
        break;

    case USB_DEVICE_EVENT_CONFIGURED:
        break;

    case USB_DEVICE_EVENT_SUSPENDED:
        break;

    case USB_DEVICE_EVENT_POWER_DETECTED:
        /* VBUS is detected. Attach the device. */
        USB_DEVICE_Attach(appData->usbDeviceHandle);
        break;

    case USB_DEVICE_EVENT_POWER_REMOVED:
        /* VBUS is not detected. Detach the device */
        USB_DEVICE_Detach(appData->usbDeviceHandle);
        break;

        /* These events are not used in this demo */
    case USB_DEVICE_EVENT_RESUMED:
    case USB_DEVICE_EVENT_ERROR:
    case USB_DEVICE_EVENT_SOF:
    default:
        break;
    }
}

#if SYS_FS_AUTOMOUNT_ENABLE
/* FS event callback*/
void SysFSEventHandler(SYS_FS_EVENT event, void* eventData, uintptr_t context)
{
    switch (event) {
    case SYS_FS_EVENT_MOUNT:
        if (strcmp((const char *) eventData, SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0) == 0) {
            appUSBMSDData.fsMounted = true;
        }
        break;
    case SYS_FS_EVENT_UNMOUNT:
        if (strcmp((const char *) eventData, SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0) == 0) {
            appUSBMSDData.fsMounted = false;
        }
        break;
    case SYS_FS_EVENT_ERROR:
        break;
    }
}
#endif

// *****************************************************************************

/* Parse Wi-Fi configuration file */
/* Format is CMD:SEND_UART=wifi <SSID>,<PASSPHRASE>,<AUTH>*/
static bool parseWifiConfig()
{
    char *p; 
    bool ret = false;
    
    p = strtok((char *)appUSBMSDData.appBuffer, " ");
    if(p != NULL && !strncmp(p, APP_USB_MSD_WIFI_CONFIG_ID, strlen(APP_USB_MSD_WIFI_CONFIG_ID)))
    {
        p = (char *)(&appUSBMSDData.appBuffer[strlen(APP_USB_MSD_WIFI_CONFIG_ID)+1]);
        p = strtok(p, ",");
        if (p == NULL || strlen(p) > WDRV_PIC32MZW_MAX_SSID_LEN) 
            return false;
        strcpy(wifi.ssid, p);
   
        p = strtok(NULL, ",");
        if (p == NULL 
            ||  strlen(p) < WDRV_PIC32MZW_MIN_PSK_PASSWORD_LEN 
            || strlen(p) > WDRV_PIC32MZW_MAX_PSK_PASSWORD_LEN)
            return false;
        strcpy(wifi.key, p);
    
        p = strtok(NULL, ",");
        if (p == NULL)
            return false;
        wifi.auth = atoi(p);
    
        APP_USB_MSD_DBG(SYS_ERROR_DEBUG, "SSID:%s - PASSPHRASE:%s - AUTH:%d\r\n", 
                                        wifi.ssid, 
                                        wifi.key, 
                                        wifi.auth);
        return true;
    }
    return false;
}

/* Write data from 'buffer' into 'fileName'*/
static int8_t writeFile(const char* fileName, const void *buffer, size_t nbyte) {
    SYS_FS_HANDLE fd = NULL;
    size_t size;
    APP_USB_MSD_PRNT("Creating %s\r\n", fileName);
    fd = SYS_FS_FileOpen(fileName, SYS_FS_FILE_OPEN_WRITE);
    if (SYS_FS_HANDLE_INVALID != fd) 
    {
        size = SYS_FS_FileWrite(fd, buffer, nbyte);
        SYS_FS_FileSync(fd);
        SYS_FS_FileClose(fd);

        if ((nbyte) != size) {
            APP_USB_MSD_DBG(SYS_ERROR_ERROR, "error writing %s . Size mismatch (got %d on fd %x. FSError = %d) \r\n", fileName, (int) size, (int) fd, SYS_FS_Error());
            return -1;
        }
    } else {
        APP_USB_MSD_DBG(SYS_ERROR_ERROR, "Error creating new %s (fsError=%d)\r\n", fileName, SYS_FS_Error());
        return -2;
    }
    return 0;
}

/* Read certificate subject key ID */
static int8_t getSubjectKeyID(uint8_t* derCert, size_t derCertSz, char* keyID) {
    DecodedCert cert;
    int8_t ret;

    InitDecodedCert(&cert, (const byte*) derCert, derCertSz, 0);
    ret = ParseCert(&cert, CERT_TYPE, NO_VERIFY, 0);
    if (ret) {
        APP_USB_MSD_DBG(SYS_ERROR_ERROR, "ERROR PARSING KEY IDENTIFIER\r\n");
    } else {
        char displayStr[KEYID_SIZE * 3];
        char packedDisplayStr[APP_CTRL_CLIENTID_SIZE];
        size_t displen = sizeof (displayStr);
        size_t packedDispLen = sizeof (packedDisplayStr);

        atcab_bin2hex_(cert.extSubjKeyId, KEYID_SIZE, displayStr, &displen, false, false, false);
        packHex(displayStr, displen, packedDisplayStr, &packedDispLen); /*Check if this needs to be called*/

        APP_USB_MSD_PRNT("Certificate Key ID: %.*s\r\n", packedDispLen, packedDisplayStr);
        strncpy(keyID, packedDisplayStr, packedDispLen);
    }
    FreeDecodedCert(&cert);
    return ret;
}

/*Read Wi-Fi configuration file*/
static int8_t readWifiConfigFile() {
    SYS_FS_RESULT fsResult = SYS_FS_RES_FAILURE;
    SYS_FS_HANDLE fd = NULL;
    bool status;
    
    /* Touch the file */
    fsResult = SYS_FS_FileStat(APP_USB_MSD_WIFI_CONFIG_FILE_NAME, &appUSBMSDData.fileStatus);
    if (SYS_FS_RES_FAILURE != fsResult) {
        APP_USB_MSD_PRNT("config file exists (FS Error %d)\r\n", SYS_FS_Error());
        
        /* Open the file */
        fd = SYS_FS_FileOpen(APP_USB_MSD_WIFI_CONFIG_FILE_NAME, SYS_FS_FILE_OPEN_READ);
        if (SYS_FS_HANDLE_INVALID != fd) {
            APP_USB_MSD_DBG(SYS_ERROR_DEBUG, "File opened\r\n");
            
            /* Read the file */
            fsResult = SYS_FS_FileStringGet(fd, (char *)appUSBMSDData.appBuffer, sizeof(appUSBMSDData.appBuffer));
            SYS_FS_FileClose(fd);
            
            if (fsResult == SYS_FS_RES_SUCCESS) {
                APP_USB_MSD_DBG(SYS_ERROR_DEBUG, ">> %s \r\n", (char *)appUSBMSDData.appBuffer);
                if(strlen((char *)appUSBMSDData.appBuffer) < APP_USB_MSD_WIFI_CONFIG_MIN_LEN)
                {
                    APP_USB_MSD_DBG(SYS_ERROR_ERROR, "Bad config file format\r\n");
                    return -1;
                }
                else
                {
                    /* Parse file contents*/
                    if(parseWifiConfig() == false)
                    {
                        APP_USB_MSD_DBG(SYS_ERROR_ERROR, "Failed parsing Wi-Fi config\r\n");
                        return -1;
                    }
                    return 0;
                }
            }
            else {
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "Error reading config file (fsError=%d)\r\n", SYS_FS_Error());
                return -1;
            }
        }
    }
    return -2;
}

/* Read cloud configuration file */
static int8_t readCloudConfigFile() {
    /*Read the MQTT config now*/
    SYS_FS_RESULT fsResult = SYS_FS_RES_FAILURE;
    SYS_FS_HANDLE fd = NULL;
    size_t size, rSize;
    
    /* Touch the file */
    fsResult = SYS_FS_FileStat(APP_USB_MSD_CLOUD_CONFIG_FILE_NAME, &appUSBMSDData.fileStatus);
    if (SYS_FS_RES_FAILURE != fsResult) 
    {
        /* Open the file*/
        fd = SYS_FS_FileOpen(APP_USB_MSD_CLOUD_CONFIG_FILE_NAME, SYS_FS_FILE_OPEN_READ);
        if (SYS_FS_HANDLE_INVALID != fd) 
        {
            /*Get size of file*/
            SYS_FS_FileSeek(fd, 0L, SYS_FS_SEEK_END);
            size = SYS_FS_FileTell(fd);
            SYS_FS_FileSeek(fd, 0L, SYS_FS_SEEK_SET);

            char configString[size + 1];
            memset(configString, 0, size + 1);
            rSize = SYS_FS_FileRead(fd, configString, size);
            SYS_FS_FileClose(fd);

            if (rSize != size) {
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "error reading Cloud config file . Size mismatch (got %d. Expected %d. FSError = %d) \r\n", (int) rSize, (int) size, SYS_FS_Error());
                return -1;
            }
            
            /*Parse the file */
            cJSON *messageJson = cJSON_Parse(configString);
            if (messageJson == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();            
                if (error_ptr != NULL) {
                    APP_USB_MSD_DBG(SYS_ERROR_ERROR, "Message JSON parse Error. Error before: %s \r\n", error_ptr);
            }
                cJSON_Delete(messageJson);
                return -1;
            }
            
            cJSON *endpoint = cJSON_GetObjectItem(messageJson, "Endpoint");
            if (!endpoint || endpoint->type !=cJSON_String ) {
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "JSON endpoint parsing error\r\n");
                cJSON_Delete(messageJson);
                return -1;
            }
            snprintf(g_Cloud_Endpoint, sizeof(g_Cloud_Endpoint), "%s", endpoint->valuestring);
#ifdef AWS_CLOUD_DEMO
            //Get the ClientID
            cJSON *clientID = cJSON_GetObjectItem(messageJson, "ClientID");
            if (!clientID || endpoint->type !=cJSON_String ) {
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "JSON ClientID parsing error\r\n");
                cJSON_Delete(messageJson);
                return -1;
            }
            snprintf(g_Aws_ClientID, sizeof(g_Aws_ClientID), "%s", clientID->valuestring);
#endif
        }
        else
        {
            APP_USB_MSD_DBG(SYS_ERROR_ERROR, "config file handle invlid. Open failed\r\n");
            return -5;
        }
    }
    else 
    {
        APP_USB_MSD_DBG(SYS_ERROR_ERROR, "Error opening Cloud config file (fsError=%d)\r\n", SYS_FS_Error());
        return -6;
    }

    return 0;
}

/*Pull device, signer and root certificates from the ECC608 device and write them to the MSD */
/* Also re-create the cloud config file */
static int8_t writeCloudFiles(void) 
{
    ATCA_STATUS status;
    SYS_FS_RESULT fsResultCloudConfig = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultDeviceCert = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultClickMe = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultVoice = SYS_FS_RES_FAILURE;
    SYS_FS_RESULT fsResultKitInfo = SYS_FS_RES_FAILURE;
    
    fsResultCloudConfig = SYS_FS_FileStat(APP_USB_MSD_CLOUD_CONFIG_FILE_NAME, &appUSBMSDData.fileStatus);
    fsResultClickMe = SYS_FS_FileStat(APP_USB_MSD_CLICKME_FILE_NAME, &appUSBMSDData.fileStatus);
    fsResultVoice = SYS_FS_FileStat(APP_USB_MSD_VOICE_CLICKME_FILE_NAME, &appUSBMSDData.fileStatus);
    fsResultKitInfo = SYS_FS_FileStat(APP_USB_MSD_KIT_INFO_FILE_NAME, &appUSBMSDData.fileStatus);

    /* Uncomment below code to read and print ECC608's serial number */
#if 1
    uint8_t sernum[9];
    char displayStr[ATCA_SERIAL_NUM_SIZE * 3];
    size_t displen = sizeof (displayStr);
    atcab_read_serial_number(sernum);
    atcab_bin2hex_(sernum, ATCA_SERIAL_NUM_SIZE, displayStr, &displen, false, false, true);
    APP_USB_MSD_PRNT("Serial Number of the Device: %s\r\n\n", displayStr);
#endif

    /*Read device cert signer by the signer above*/
    size_t deviceCertSize = 0;
    status = tng_atcacert_max_device_cert_size(&deviceCertSize);
    if (ATCA_SUCCESS != status) {
        APP_USB_MSD_DBG(SYS_ERROR_ERROR, "tng_atcacert_max_signer_cert_size Failed \r\n");
        return status;
    }

    uint8_t deviceCert[deviceCertSize];
    status = tng_atcacert_read_device_cert((uint8_t*) & deviceCert, &deviceCertSize, NULL);
    if (ATCA_SUCCESS != status) {
        APP_USB_MSD_DBG(SYS_ERROR_ERROR, "tng_atcacert_read_device_cert Failed (%x) \r\n", status);
        return status;
    }
    
#ifdef AZURE_CLOUD_DEMO
    if (SYS_FS_RES_FAILURE == fsResultCloudConfig)
    {
        uint8_t digest[32];
        status =  atcab_hw_sha2_256((const uint8_t *) deviceCert, deviceCertSize, digest);
        if (ATCA_SUCCESS != status) 
        {
            APP_USB_MSD_DBG(SYS_ERROR_ERROR, "atcab_hw_sha2_256 Failed \r\n");
        }
        else
        {
            char displayStr[32 * 3];
            size_t displen = sizeof (displayStr);

            atcab_bin2hex_(digest, 32, displayStr, &displen, false, false, true);            
            printf("SHA265 = %s\r\n", displayStr);
            /*Write cloud config to file*/
            char cloudConfigString[strlen(APP_USB_MSD_AZURE_CLOUD_CONFIG_DATA_TEMPLATE) + strlen(DEVICE_CONNECTION_STRING) + displen + 1];
            sprintf(cloudConfigString, APP_USB_MSD_AZURE_CLOUD_CONFIG_DATA_TEMPLATE, DEVICE_CONNECTION_STRING, displayStr);
            if (0 != writeFile(APP_USB_MSD_CLOUD_CONFIG_FILE_NAME, cloudConfigString, strlen(cloudConfigString))) 
            {
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "cloud config write Failed\r\n");
                return -8;
            }
        }
    }
#elif defined AWS_CLOUD_DEMO
    /*Write PEM format device certificate to root folder*/
    char fileName[strlen(APP_USB_MSD_DEVCERT_FILE_NAME) + strlen(displayStr)];
    sprintf(fileName, APP_USB_MSD_DEVCERT_FILE_NAME, displayStr);
    fsResultDeviceCert = SYS_FS_FileStat(fileName, &appUSBMSDData.fileStatus);
    if (SYS_FS_RES_FAILURE == fsResultDeviceCert) 
    {
        /*Generate a PEM device certificate.*/
        byte devPem[1024];
        XMEMSET(devPem, 0, 1024);
        int devPemSz;

        devPemSz = wc_DerToPem(deviceCert, deviceCertSize, devPem, sizeof (devPem), CERT_TYPE);
        if ((devPemSz <= 0)) {
            APP_USB_MSD_DBG(SYS_ERROR_ERROR, "Failed converting device Cert to PEM (%d)\r\n", devPemSz);
            return devPemSz;
        }

        if (0 != writeFile(fileName, devPem, devPemSz)) 
        {
            return -1;
        }
    }
    
    /* Compute key ID only if Cloud config, Voice registration or 'CLICK-ME' file is missing */
    /* and write those files */
    if ((SYS_FS_RES_FAILURE == fsResultClickMe) 
            || (SYS_FS_RES_FAILURE == fsResultCloudConfig)
            || (SYS_FS_RES_FAILURE == fsResultVoice)) 
    {
        char keyID[APP_CTRL_CLIENTID_SIZE];
        memset(keyID, '\0', APP_CTRL_CLIENTID_SIZE);
        if (0 != getSubjectKeyID(deviceCert, deviceCertSize, keyID)) 
        {
            return -3;
        } else 
        {
            /* Demo web interface */
            if (SYS_FS_RES_FAILURE == fsResultClickMe) 
            {
                char clickmeString[strlen(APP_USB_MSD_CLICKME_DATA_TEMPLATE) + strlen(keyID)];
                sprintf(clickmeString, APP_USB_MSD_CLICKME_DATA_TEMPLATE, keyID);
                if (0 != writeFile(APP_USB_MSD_CLICKME_FILE_NAME, clickmeString, strlen(clickmeString))) 
                {
                    return -2;
                }
            }
            /* Voice control registration web page */
            if (SYS_FS_RES_FAILURE == fsResultVoice) 
            {
                char voiceClickmeString[strlen(APP_USB_MSD_VOICE_CLICKME_DATA_TEMPLATE) + strlen(keyID)];
                sprintf(voiceClickmeString, APP_USB_MSD_VOICE_CLICKME_DATA_TEMPLATE, keyID);
                if (0 != writeFile(APP_USB_MSD_VOICE_CLICKME_FILE_NAME, voiceClickmeString, strlen(voiceClickmeString))) {
                    return -7;
                }
            }
            /* Product/Kit web page */
            if (SYS_FS_RES_FAILURE == fsResultKitInfo) 
            {
                char KitInfoString[strlen(APP_USB_MSD_KIT_INFO_DATA_TEMPLATE)];
                sprintf(KitInfoString, APP_USB_MSD_KIT_INFO_DATA_TEMPLATE, keyID);
                if (0 != writeFile(APP_USB_MSD_KIT_INFO_FILE_NAME, KitInfoString, strlen(KitInfoString))) {
                    return -7;
                }
            }
            /* Cloud configuration JSON file */
            if (SYS_FS_RES_FAILURE == fsResultCloudConfig) 
            {
                char cloudConfigString[strlen(APP_USB_MSD_AWS_CLOUD_CONFIG_DATA_TEMPLATE) + strlen(keyID) + strlen(IOT_DEMO_SERVER)];
                sprintf(cloudConfigString, APP_USB_MSD_AWS_CLOUD_CONFIG_DATA_TEMPLATE, IOT_DEMO_SERVER, keyID);
                if (0 != writeFile(APP_USB_MSD_CLOUD_CONFIG_FILE_NAME, cloudConfigString, strlen(cloudConfigString))) 
                {
                    return -8;
                }
            }
        }
    }
#endif
    return 0;
}

static bool checkFSMount(){
#if SYS_FS_AUTOMOUNT_ENABLE
    return appUSBMSDData.fsMounted;
#else
    if(SYS_FS_Mount(SYS_FS_MEDIA_IDX0_DEVICE_NAME_VOLUME_IDX0, SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0, FAT, 0, NULL) != SYS_FS_RES_SUCCESS){
        return false;
    }
    else{
        return true;
    }
#endif            
}

// *****************************************************************************

int8_t APP_WriteWifiConfigFile(char* ssid, char* passphrase, uint8_t auth , bool force) 
{
    SYS_FS_RESULT fsResultWifiConfig = SYS_FS_RES_FAILURE;   
    fsResultWifiConfig = SYS_FS_FileStat(APP_USB_MSD_WIFI_CONFIG_FILE_NAME, &appUSBMSDData.fileStatus);

    /* Wi-Fi config file */
    if (SYS_FS_RES_FAILURE == fsResultWifiConfig || force) 
    {
        char configString[strlen(APP_USB_MSD_WIFI_CONFIG_DATA_TEMPLATE)
                            + strlen(ssid)
                            + strlen(passphrase)
                            + sizeof(WIFI_AUTH)];
        sprintf(configString, APP_USB_MSD_WIFI_CONFIG_DATA_TEMPLATE, ssid, passphrase, auth);
        if (0 != writeFile(APP_USB_MSD_WIFI_CONFIG_FILE_NAME, configString, strlen(configString))) 
        {
            return -2;
        }
    }
            
    return 0;
}

void APP_InitializeUSBMSD ( void )
{
    appUSBMSDData.USBMSDTaskState = APP_USB_MSD_INIT;
    appUSBMSDData.fsMounted = false;
    appUSBMSDData.usbDeviceHandle = USB_DEVICE_HANDLE_INVALID;
#if SYS_FS_AUTOMOUNT_ENABLE
    SYS_FS_EventHandlerSet(SysFSEventHandler, (uintptr_t) NULL);
#endif
}

/* Application USB MSD main task */
void APP_TaskUSBMSD ( void )
{
    SYS_FS_RESULT fsResult = SYS_FS_RES_FAILURE;
    bool status;
    int8_t ret = 0;
    SYS_FS_FORMAT_PARAM opt;

    switch (appUSBMSDData.USBMSDTaskState) {
        /* Application's initial state. */
        case APP_USB_MSD_INIT:
        {
            appUSBMSDData.usbDeviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);
            if (appUSBMSDData.usbDeviceHandle != USB_DEVICE_HANDLE_INVALID) {
                /* Set the Event Handler. We will start receiving events after
                 * the handler is set */
                USB_DEVICE_EventHandlerSet(appUSBMSDData.usbDeviceHandle, USBDeviceEventHandler, (uintptr_t) & appUSBMSDData);

                APP_USB_MSD_DBG(SYS_ERROR_INFO, "Waiting for FS mount\r\n");
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_WAIT_FS_MOUNT;
            } else {
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
            }
            break;
        }
        
        /* Pending state */
        case APP_USB_MSD_PENDING:
        {
            break;
        }
        
        /* Waiting for FS mount */
        case APP_USB_MSD_WAIT_FS_MOUNT:
        {
            if (checkFSMount()) {
                APP_USB_MSD_PRNT("FS Mounted \r\n");
                if (SW1_IS_PRESSED && SW2_IS_PRESSED) 
                {
                    APP_USB_MSD_PRNT("Factory config reset requested\r\n");
                    SW1_PRESSED(false);
                    SW2_PRESSED(false);
                    appUSBMSDData.USBMSDTaskState = MSD_APP_STATE_CLEAR_DRIVE;
                } 
                else if (SYS_FS_ERROR_NO_FILESYSTEM==SYS_FS_Error())
                {
                    APP_USB_MSD_PRNT("No Filesystem. Doing a format\r\n");
                    appUSBMSDData.USBMSDTaskState = MSD_APP_STATE_CLEAR_DRIVE;
                }
                else 
                {
                    appUSBMSDData.USBMSDTaskState = APP_USB_MSD_TOUCH_CLOUD_FILES;
                }
                
                SYS_FS_FileDirectoryRemove("FILE.txt");
            }
            break;
        }
        
        case MSD_APP_STATE_CLEAR_DRIVE:
        {
            opt.fmt = SYS_FS_FORMAT_FAT;
            opt.au_size = 0;
            if (SYS_FS_DriveFormat (SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0, &opt, (void *)work, SYS_FS_FAT_MAX_SS) != SYS_FS_RES_SUCCESS)
            {
                /* Format of the disk failed. */
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "Media Format failed\r\n");
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
            }
            else
            {
                /* Format succeeded. Open a file. */
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_TOUCH_CLOUD_FILES;
            }
            break;
        }
        
        /* Write cloud config file and web pages' files if not already existing*/
        case APP_USB_MSD_TOUCH_CLOUD_FILES:
        {
            SYS_FS_DriveLabelSet(SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0,APP_USB_MSD_DRIVE_NAME);
            SYS_FS_CurrentDriveSet(SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0);

            extern ATCAIfaceCfg atecc608_0_init_data;
            ATCA_STATUS atcaStat;
            atcaStat = atcab_init(&atecc608_0_init_data);
            if (ATCA_SUCCESS == atcaStat) 
            {
                /* Write relevant files if not already existing*/
                if (0 != writeCloudFiles())
                {
                    APP_USB_MSD_DBG(SYS_ERROR_ERROR, "error writing cloud config File\r\n");
                    appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
                    break;
                }
                
                /*Read data from active config*/
                if (0 != readCloudConfigFile()) {
                    APP_USB_MSD_DBG(SYS_ERROR_ERROR, "invalid cloud config File\r\n");
                    appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
                    break;
                }
            }
            else
            {
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "atcab_init failed\r\n");
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
                break;
            }
            
            atcab_release();
            
            appUSBMSDData.USBMSDTaskState = APP_USB_MSD_TOUCH_WIFI_CONFIG_FILE;          
            break;
        }

        /* Write and Read Wi-Fi configuration file */
        case APP_USB_MSD_TOUCH_WIFI_CONFIG_FILE:
        {   
            /* Write Wi-Fi config file */
            if (0 != APP_WriteWifiConfigFile(APP_STA_DEFAULT_SSID, 
                                            APP_STA_DEFAULT_PASSPHRASE,
                                            APP_STA_DEFAULT_AUTH,
                                            false)){
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "error writing Wi-Fi config File\r\n");
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
                break;
            }
                    
            /*Read data from active config*/
            if (0 == readWifiConfigFile())
                SET_WIFI_CREDENTIALS(VALID_CREDENTIALS_SRC_USB_MSD,CREDENTIALS_VALID);
            else
                SET_WIFI_CREDENTIALS(VALID_CREDENTIALS_SRC_USB_MSD,CREDENTIALS_INVALID);
            appUSBMSDData.USBMSDTaskState = APP_USB_MSD_IDLE;
            break;
        }

        /* Unmount FS */
        case APP_USB_MSD_FS_UNMOUNT:
        {
            if(SYS_FS_Unmount(SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0) != SYS_FS_RES_SUCCESS)
            {
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "FS unmount failed\r\n");
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
            }
            else
            {
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_DEINIT;
            }
            break;
        }
        
        /* USB de-init */
        case APP_USB_MSD_DEINIT:
        {
            USB_DEVICE_Detach(appUSBMSDData.usbDeviceHandle);
            USB_DEVICE_Close(appUSBMSDData.usbDeviceHandle);
            APP_USB_MSD_DBG(SYS_ERROR_INFO, "USB device closed\r\n");
            appUSBMSDData.USBMSDTaskState = APP_USB_MSD_IDLE;
            break;
        }
        
        /* Idle */
        case APP_USB_MSD_IDLE:
        {
            break;
        }
        
        /* Error */
        case APP_USB_MSD_ERROR:
        {
            APP_USB_MSD_DBG(SYS_ERROR_ERROR, "APP_USB_MSD_ERROR\r\n");
            /* Always inform APP_Task() about credentials validity to avoid code blocking*/
            SET_WIFI_CREDENTIALS(VALID_CREDENTIALS_SRC_USB_MSD,CREDENTIALS_INVALID);
            appUSBMSDData.USBMSDTaskState = APP_USB_MSD_IDLE;
            break;
        }
        
        default:
        {
            break;
        }
    }
}

/*******************************************************************************
 End of File
 */

