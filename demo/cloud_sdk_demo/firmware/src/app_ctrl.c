/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ctrl.c

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

#include "app_common.h"
#include "app_ctrl.h"
#include <math.h>

// *****************************************************************************
/* Main timer resolution */
#define TIMER_RESOLUTION_MS         50

/* LEDs */
#define NUM_OF_LEDS                     4
#define LED_SLOW_BLINK_PERIOD_MS        2000
#define LED_F_BLINK_PERIOD_MS           300

/* Sensors */
#define SENSORS_READ_FREQ_MS        2000

/* MCP9808 registers */
#define MCP9808_I2C_ADDRESS         0x18 
#define MCP9808_REG_CONFIG          0x01
#define MCP9808_REG_TAMBIENT		0x05
#define MCP9808_REG_MANUF_ID		0x06
#define MCP9808_REG_DEVICE_ID		0x07
#define MCP9808_REG_RESOLUTION		0x08

/* MCP9808 other settings */
#define MCP9808_CONFIG_DEFAULT		0x00
#define MCP9808_CONFIG_SHUTDOWN		0x0100
#define MCP9808_RES_DEFAULT         62500
#define MCP9808_MANUF_ID            0x54
#define MCP9808_DEVICE_ID           0x0400
#define MCP9808_DEVICE_ID_MASK		0xff00

/* OPT3001 registers */
#define OPT3001_I2C_ADDRESS             0x44
#define OPT3001_REG_RESULT              0x00
#define OPT3001_REG_CONFIG              0x01
#define OPT3001_REG_LOW_LIMIT           0x02
#define OPT3001_REG_HIGH_LIMIT          0x03
#define OPT3001_REG_MANUFACTURER_ID     0x7E
#define OPT3001_REG_DEVICE_ID           0x7F

/* MCP9808 other settings */
#define OPT3001_CONFIG_SHUTDOWN             0x00
#define OPT3001_CONFIG_CONT_CONVERSION		0xCE10        //continuous convesrion
#define OPT3001_MANUF_ID                    0x5449
#define OPT3001_DEVICE_ID                   0x3001

// *****************************************************************************
/* I2C tranfser status */
typedef enum
{
    I2C_TRANSFER_STATUS_IN_PROGRESS,
    I2C_TRANSFER_STATUS_SUCCESS,
    I2C_TRANSFER_STATUS_ERROR,
    I2C_TRANSFER_STATUS_IDLE,
} APP_CTRL_I2C_TRANSFER_STATUS;

/* Application CTRL task state machine. */
typedef enum
{
    APP_CTRL_INIT=0,
    APP_CTRL_CHECK,
    APP_CTRL_TURN_ON_MCP9808,
    APP_CTRL_WAIT_TURN_ON_MCP9808,
    APP_CTRL_TURN_ON_OPT3001,
    APP_CTRL_WAIT_TURN_ON_OPT3001,
    APP_CTRL_SHUTDOWN_MCP9808,
    APP_CTRL_WAIT_SHUTDOWN_MCP9808,
    APP_CTRL_SHUTDOWN_OPT3001,
    APP_CTRL_WAIT_SHUTDOWN_OPT3001,
    APP_CTRL_READ_TEMP,
    APP_CTRL_WAIT_READ_TEMP,
    APP_CTRL_READ_LIGHT,
    APP_CTRL_WAIT_READ_LIGHT,
    APP_CTRL_IDLE,
    APP_CTRL_ERROR
} APP_TASK_CTRL_STATES;

/* Control structure periodic operations */
typedef struct
{
    uint16_t counter;
    uint16_t reload;
    bool periodic;
} APP_CTRL_TIMER_S;

/* I2C */
typedef struct
{
    DRV_HANDLE i2cHandle;
    DRV_I2C_TRANSFER_HANDLE transferHandle;
    APP_CTRL_I2C_TRANSFER_STATUS transferStatus;
    uint8_t txBuffer[4];
    uint16_t rxBuffer;
} APP_CTRL_I2C;

/* MCP9808 structure */
typedef struct
{
    bool IsShutdown;
    int16_t temperature;
} APP_CTRL_MCP9808;

/* OPT3001 structure */
typedef struct
{
    bool IsShutdown;
    uint32_t light;
} APP_CTRL_OPT3001;

/* APP CTRL module config structure */
typedef struct
{
    APP_TASK_CTRL_STATES ctrlTaskState;
    
    SYS_TIME_HANDLE timeHandle;
    APP_CTRL_TIMER_S ledBlinkCtrl[NUM_OF_LEDS];
    APP_CTRL_TIMER_S sensorsReadCtrl;
    bool readSensors;
    bool shutdownSensors;
    bool turnOnSensors;
    
    APP_CTRL_I2C i2c;
    
    APP_CTRL_MCP9808 mcp9808;
    APP_CTRL_OPT3001 opt3001;
} APP_CTRL_DATA;
APP_CTRL_DATA appCtrlData;

// *****************************************************************************

/* LEDs */
static void ledOn(LED_COLOR led);
static void ledOff(LED_COLOR led);
static void ledToggle(LED_COLOR led);

/* I2C */
static bool i2cReadReg(uint8_t, uint16_t, uint8_t);
static bool i2cWriteReg(uint8_t, uint16_t, uint16_t);
static void i2cReadRegComp(uint8_t, uint8_t);
static void i2cWriteRegComp(uint8_t, uint8_t);

// *****************************************************************************

/* Main timer handler */
static void timeCallback(uintptr_t param) {
    uint8_t led;
    
    /* LEDs */
    for(led=0; led<NUM_OF_LEDS; led++){
        if(appCtrlData.ledBlinkCtrl[led].reload == 0)
            continue;
        
        if(appCtrlData.ledBlinkCtrl[led].reload > 0 && 
                ++appCtrlData.ledBlinkCtrl[led].counter == appCtrlData.ledBlinkCtrl[led].reload){
            ledToggle(led);
            if(appCtrlData.ledBlinkCtrl[led].periodic)
                appCtrlData.ledBlinkCtrl[led].counter = 0;
            else
                appCtrlData.ledBlinkCtrl[led].reload = 0;
        }
    }
    
    /* Sensors */
    if(appCtrlData.sensorsReadCtrl.reload > 0 && 
            ++appCtrlData.sensorsReadCtrl.counter == appCtrlData.sensorsReadCtrl.reload){
        appCtrlData.readSensors = true;     
        if(appCtrlData.sensorsReadCtrl.periodic)
            appCtrlData.sensorsReadCtrl.counter = 0;
        else
            appCtrlData.sensorsReadCtrl.reload = 0;
    }
}

/* I2C transfer callback */
static void i2cTransferCallback(DRV_I2C_TRANSFER_EVENT event, 
        DRV_I2C_TRANSFER_HANDLE transferHandle, 
        uintptr_t context){
    switch(event)
    {
        case DRV_I2C_TRANSFER_EVENT_COMPLETE:
            appCtrlData.i2c.transferStatus = I2C_TRANSFER_STATUS_SUCCESS;
            break;
        case DRV_I2C_TRANSFER_EVENT_ERROR:
            appCtrlData.i2c.transferStatus = I2C_TRANSFER_STATUS_ERROR;
            break;
        default:
            break;
    }
}

// *****************************************************************************

/* LED on handler */
static void ledOn(LED_COLOR led){
    if(led == LED_RED)
        LED_RED_On();
    else if(led == LED_GREEN)
        LED_GREEN_On();
    else if(led == LED_YELLOW)
        LED_YELLOW_On();
    else if(led == LED_BLUE)
        LED_BLUE_On();
}

/* LED off handler */
static void ledOff(LED_COLOR led){   
    if(led == LED_RED)
        LED_RED_Off();
    else if(led == LED_GREEN)
        LED_GREEN_Off();
    else if(led == LED_YELLOW)
        LED_YELLOW_Off();
    else if(led == LED_BLUE)
        LED_BLUE_Off();
}

/* LED toggle handler */
static void ledToggle(LED_COLOR led){
    if(led == LED_RED)
        LED_RED_Toggle();
    else if(led == LED_GREEN)
        LED_GREEN_Toggle();
    else if(led == LED_YELLOW)
        LED_YELLOW_Toggle();
    else if(led == LED_BLUE)
        LED_BLUE_Toggle();
}

/* Fast blink handler */
static void ledFastBlink(LED_COLOR led, LED_BLINK_MODE mode) {
    /* If older blinking is active, just return */
    if(appCtrlData.ledBlinkCtrl[led].reload > 0)
        return;
    
    appCtrlData.ledBlinkCtrl[led].counter = 0;
    appCtrlData.ledBlinkCtrl[led].reload = LED_F_BLINK_PERIOD_MS/TIMER_RESOLUTION_MS;
    if(mode == BLINK_MODE_PERIODIC)
        appCtrlData.ledBlinkCtrl[led].periodic = true;
    else
        appCtrlData.ledBlinkCtrl[led].periodic = false;
        
    /* Turn on the LED */
    ledOn(led); 
}

/* Slow blink handler */
static void ledSlowBlink(LED_COLOR led, LED_BLINK_MODE mode, LED_MODE val) {
    appCtrlData.ledBlinkCtrl[led].counter = 0;
    appCtrlData.ledBlinkCtrl[led].reload = LED_SLOW_BLINK_PERIOD_MS/TIMER_RESOLUTION_MS;
    if(mode == BLINK_MODE_PERIODIC)
        appCtrlData.ledBlinkCtrl[led].periodic = true;
    else
        appCtrlData.ledBlinkCtrl[led].periodic = false;
    
    /* Turn on/off the LED */
    if(val == LED_ON)
        ledOn(led);
    else
        ledOff(led);
}

/* Reset LED's timer configuration */
static void ledBlinkStop(LED_COLOR led){  
    appCtrlData.ledBlinkCtrl[led].counter = 0;
    appCtrlData.ledBlinkCtrl[led].reload = 0;
    appCtrlData.ledBlinkCtrl[led].periodic = false;
}

/* LED sub-module init */
static void ledInit(){
    uint8_t led;
    for(led=0; led<NUM_OF_LEDS; led++){
        appCtrlData.ledBlinkCtrl[led].counter = 0;
        appCtrlData.ledBlinkCtrl[led].reload = 0;
        appCtrlData.ledBlinkCtrl[led].periodic = false;
        ledOff(led);
    }
}

/* I2C read */
static bool i2cReadReg(uint8_t addr, uint16_t reg, uint8_t size){
    bool ret = false;
    appCtrlData.i2c.transferHandle == DRV_I2C_TRANSFER_HANDLE_INVALID;
    appCtrlData.i2c.txBuffer[0] = (uint8_t)reg;
    
    DRV_I2C_WriteReadTransferAdd(appCtrlData.i2c.i2cHandle, 
            addr, 
            (void*)appCtrlData.i2c.txBuffer, 1, 
            (void*)&appCtrlData.i2c.rxBuffer, size, 
            &appCtrlData.i2c.transferHandle);
    if(appCtrlData.i2c.transferHandle == DRV_I2C_TRANSFER_HANDLE_INVALID)
    {
        APP_CTRL_DBG(SYS_ERROR_ERROR, "I2C read reg %x error \r\n", reg);
        ret = false;
    }
    else
        ret = true;
    return ret;
}

/* I2C read complete */
static void i2cReadRegComp(uint8_t addr, uint8_t reg){
    appCtrlData.i2c.rxBuffer = (appCtrlData.i2c.rxBuffer << 8) | (appCtrlData.i2c.rxBuffer >> 8);
    APP_CTRL_DBG(SYS_ERROR_DEBUG, "I2C read complete - periph addr %x val %x\r\n", addr, appCtrlData.i2c.rxBuffer);
    switch(addr)
    {   
        /* MCP9808 */
        case MCP9808_I2C_ADDRESS:
            if (reg == MCP9808_REG_TAMBIENT){
                uint8_t upperByte = (uint8_t)(appCtrlData.i2c.rxBuffer >> 8);
                uint8_t lowerByte = ((uint8_t)(appCtrlData.i2c.rxBuffer & 0x00FF));
                upperByte = upperByte & 0x1F;
                if ((upperByte & 0x10) == 0x10){         // Ta < 0 degC
                    upperByte = upperByte & 0x0F;       // Clear sign bit
                    appCtrlData.mcp9808.temperature = 256 - ((upperByte * 16) + lowerByte/16);
                }
                else{
                    appCtrlData.mcp9808.temperature = ((upperByte * 16) + lowerByte/16);
                }
                APP_CTRL_DBG(SYS_ERROR_INFO, "MCP9808 Temperature %d (C)\r\n", appCtrlData.mcp9808.temperature);                
            }
            break;

        /* OPT3001 */
        case OPT3001_I2C_ADDRESS:
            if (reg == OPT3001_REG_RESULT){
                uint16_t m = appCtrlData.i2c.rxBuffer & 0x0FFF;
                uint16_t e = (appCtrlData.i2c.rxBuffer & 0xF000) >> 12;
                appCtrlData.opt3001.light = (m*pow(2,e))/100;
                APP_CTRL_DBG(SYS_ERROR_INFO, "OPT3001 Light %d (lux)\r\n", appCtrlData.opt3001.light); 
            }
            break;

        default:
            break;
    }
}

/* I2C write */
static bool i2cWriteReg(uint8_t addr, uint16_t reg, uint16_t val){
    bool ret = false;
    appCtrlData.i2c.transferHandle == DRV_I2C_TRANSFER_HANDLE_INVALID;
    appCtrlData.i2c.txBuffer[0] = (uint8_t)reg;
    appCtrlData.i2c.txBuffer[1] = (uint8_t)(val >> 8);
    appCtrlData.i2c.txBuffer[2] = (uint8_t)(val & 0x00FF);
    
    DRV_I2C_WriteTransferAdd(appCtrlData.i2c.i2cHandle, 
            addr, 
            (void*)appCtrlData.i2c.txBuffer, 3, 
            &appCtrlData.i2c.transferHandle);
    if(appCtrlData.i2c.transferHandle == DRV_I2C_TRANSFER_HANDLE_INVALID)
    {
        APP_CTRL_DBG(SYS_ERROR_ERROR, "I2C write reg %x error \r\n", reg);
        ret = false;
    }
    else
        ret = true;
    
    return ret;
}

/* I2C write complete */
static void i2cWriteRegComp(uint8_t addr, uint8_t reg){
    APP_CTRL_DBG(SYS_ERROR_DEBUG, "I2C write complete - periph addr %x\r\n", addr);
}

/* Sensors sub-module init */
static void sensorsInit(){
    /* Issue I2C read operation to get sensors readings*/
    appCtrlData.readSensors = false;
      
    /* Default is sensors on Vs sensors shutdown */
    appCtrlData.turnOnSensors = true;
    appCtrlData.shutdownSensors = false;
    
    /*sensors periodic behavior configuration*/
    appCtrlData.sensorsReadCtrl.counter = 0;
    appCtrlData.sensorsReadCtrl.reload = SENSORS_READ_FREQ_MS/TIMER_RESOLUTION_MS;
    appCtrlData.sensorsReadCtrl.periodic = true;
    
    /*sensors structures*/
    memset(&appCtrlData.mcp9808, 0, sizeof(appCtrlData.mcp9808));
    memset(&appCtrlData.opt3001, 0, sizeof(appCtrlData.opt3001));
    appCtrlData.mcp9808.IsShutdown = true;
    appCtrlData.opt3001.IsShutdown = true;
    
    /*I2C structure*/
    memset(&appCtrlData.i2c, 0, sizeof(appCtrlData.i2c));
    appCtrlData.i2c.i2cHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    appCtrlData.i2c.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
}

void ledStartupSeq(){
    uint8_t i=0;
    for(i=0; i<2; i++){
        APP_manageLed(LED_BLUE, LED_F_BLINK, BLINK_MODE_SINGLE);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        APP_manageLed(LED_GREEN, LED_F_BLINK, BLINK_MODE_SINGLE);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        APP_manageLed(LED_YELLOW, LED_F_BLINK, BLINK_MODE_SINGLE);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        APP_manageLed(LED_RED, LED_F_BLINK, BLINK_MODE_SINGLE);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
}

// *****************************************************************************

/* Sensors Turn on & enable periodic I2C reading */
void APP_sensorsOn(void)
{
    appCtrlData.turnOnSensors = true;
    appCtrlData.sensorsReadCtrl.counter = 0;
    appCtrlData.sensorsReadCtrl.reload = SENSORS_READ_FREQ_MS/TIMER_RESOLUTION_MS;
    appCtrlData.sensorsReadCtrl.periodic = true;
}

/* Sensors Shutdown (to save power) & disable periodic I2C reading */
void APP_sensorsOff(void)
{
    appCtrlData.shutdownSensors = true;
    appCtrlData.sensorsReadCtrl.counter = 0;
    appCtrlData.sensorsReadCtrl.reload = 0;
    appCtrlData.sensorsReadCtrl.periodic = false;
}

/* Read MCP9808 Temperature */
int16_t APP_readTemp(void)
{
    return appCtrlData.mcp9808.temperature;
}

/* Read OPT3001 Light */
uint32_t APP_readLight(void)
{
    return appCtrlData.opt3001.light;
}

/* LED Manager */
void APP_manageLed(LED_COLOR led, LED_MODE mode, LED_BLINK_MODE blinkMode)
{
    if(led < LED_RED || led > LED_BLUE
        || blinkMode < BLINK_MODE_SINGLE 
        || blinkMode > BLINK_MODE_INVALID){
        APP_CTRL_DBG(SYS_ERROR_ERROR, "Invalid parameters to LED manager \r\n");
        return;
    }
    
    /* Check required LED mode */
    switch (mode) 
    {
        case LED_ON:
            ledBlinkStop(led);
            ledOn(led);
            break;
        case LED_OFF:
            ledBlinkStop(led);
            ledOff(led);
            break;
        case LED_TOGGLE:
            ledToggle(led);
            break;        
        case LED_F_BLINK:
            ledFastBlink(led, blinkMode);
            break;
        case LED_S_BLINK_STARTING_ON:
            ledSlowBlink(led, blinkMode, LED_ON);
            break;
        case LED_S_BLINK_STARTING_OFF:
            ledSlowBlink(led, blinkMode, LED_OFF);
            break;
        default:
            APP_CTRL_DBG(SYS_ERROR_ERROR, "Invalid LED mode\r\n");
            break;
    }
}

void APP_InitializeCtrl ()
{
    appCtrlData.ctrlTaskState = APP_CTRL_INIT;
    appCtrlData.timeHandle == SYS_TIME_HANDLE_INVALID;
    ledInit();
    sensorsInit();
    
    /* Start a periodic timer to handle periodic events*/
    appCtrlData.timeHandle = SYS_TIME_CallbackRegisterMS(timeCallback, 
                                                            (uintptr_t) 0, 
                                                            TIMER_RESOLUTION_MS, 
                                                            SYS_TIME_PERIODIC);
    if (appCtrlData.timeHandle == SYS_TIME_HANDLE_INVALID) {
        APP_CTRL_DBG(SYS_ERROR_ERROR, "Failed creating a periodic timer \r\n");
        return;
    }
}

/* Application control main task */
void APP_TaskCtrl ( void )
{
    switch (appCtrlData.ctrlTaskState) 
    {
        /* Init state */
        case APP_CTRL_INIT:
        {
            ledStartupSeq();
            
            /* Open I2C driver client */
            appCtrlData.i2c.i2cHandle = DRV_I2C_Open( DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE );
            if (appCtrlData.i2c.i2cHandle == DRV_HANDLE_INVALID)
            {
                APP_CTRL_DBG(SYS_ERROR_ERROR, "Failed to open I2C driver for sensors reading\r\n");
                appCtrlData.ctrlTaskState = APP_CTRL_ERROR;
            }
            else{
                DRV_I2C_TransferEventHandlerSet(appCtrlData.i2c.i2cHandle, i2cTransferCallback, 0);
                appCtrlData.ctrlTaskState = APP_CTRL_CHECK;
            }
        }
        
        /* CTRL task checkpoint */
        case APP_CTRL_CHECK:
        {
            /* User request to turn on sensors */
            if(appCtrlData.turnOnSensors){
                appCtrlData.turnOnSensors = false;
                appCtrlData.ctrlTaskState = APP_CTRL_TURN_ON_MCP9808;
            }
            /* User request to shutdown sensors (to save power) */
            else if(appCtrlData.shutdownSensors){
                appCtrlData.shutdownSensors = false;
                appCtrlData.ctrlTaskState = APP_CTRL_SHUTDOWN_MCP9808;
            }
            else if(appCtrlData.readSensors && 
                    appCtrlData.mcp9808.IsShutdown == false && 
                    appCtrlData.opt3001.IsShutdown == false){
                appCtrlData.readSensors = false;
                appCtrlData.ctrlTaskState = APP_CTRL_READ_TEMP;
            }
            break;
        }

        /* MCP9808 turn on */
        case APP_CTRL_TURN_ON_MCP9808:
        {
            appCtrlData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            if(i2cWriteReg(MCP9808_I2C_ADDRESS, MCP9808_REG_CONFIG, MCP9808_CONFIG_DEFAULT))
                appCtrlData.ctrlTaskState = APP_CTRL_WAIT_TURN_ON_MCP9808;
            else
                appCtrlData.ctrlTaskState = APP_CTRL_TURN_ON_OPT3001;
            break;
        }
        
        /* MCP9808 wait for turn on */
        case APP_CTRL_WAIT_TURN_ON_MCP9808:
        {
            if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS){
                appCtrlData.mcp9808.IsShutdown = false;
                i2cWriteRegComp(MCP9808_I2C_ADDRESS, MCP9808_REG_CONFIG);
                appCtrlData.ctrlTaskState = APP_CTRL_TURN_ON_OPT3001;
            }
            else if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR){
                APP_CTRL_DBG(SYS_ERROR_ERROR, "I2C write MCP9808_REG_CONFIG error \r\n");
                appCtrlData.ctrlTaskState = APP_CTRL_TURN_ON_OPT3001;
            }
            break;
        }
        
        /* OPT3001 turn on */
        case APP_CTRL_TURN_ON_OPT3001:
        {
            appCtrlData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            appCtrlData.turnOnSensors = false;
            if(i2cWriteReg(OPT3001_I2C_ADDRESS, OPT3001_REG_CONFIG, OPT3001_CONFIG_CONT_CONVERSION))
                appCtrlData.ctrlTaskState = APP_CTRL_WAIT_TURN_ON_OPT3001;
            else
                appCtrlData.ctrlTaskState = APP_CTRL_READ_TEMP;
            break;
        }
        
        /* OPT3001 wait for turn on */
        case APP_CTRL_WAIT_TURN_ON_OPT3001:
        {
            if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS){
                appCtrlData.opt3001.IsShutdown = false;
                i2cWriteRegComp(OPT3001_I2C_ADDRESS, OPT3001_REG_CONFIG);
                appCtrlData.ctrlTaskState = APP_CTRL_READ_TEMP;
            }
            else if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR){
                APP_CTRL_DBG(SYS_ERROR_ERROR, "I2C write OPT3001_REG_CONFIG error \r\n");
                appCtrlData.ctrlTaskState = APP_CTRL_READ_TEMP;
            }
            break;
        }
        
        /* MCP9808 shutdown (to save power)*/
        case APP_CTRL_SHUTDOWN_MCP9808:
        {
            appCtrlData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            if(i2cWriteReg(MCP9808_I2C_ADDRESS, MCP9808_REG_CONFIG, MCP9808_CONFIG_SHUTDOWN))
                appCtrlData.ctrlTaskState = APP_CTRL_WAIT_SHUTDOWN_MCP9808;
            else
                appCtrlData.ctrlTaskState = APP_CTRL_SHUTDOWN_OPT3001;
            break;
        }
        
        /* MCP9808 wait for shutdown */
        case APP_CTRL_WAIT_SHUTDOWN_MCP9808:
        {
            if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS){
                appCtrlData.mcp9808.IsShutdown = true;
                i2cWriteRegComp(MCP9808_I2C_ADDRESS, MCP9808_REG_CONFIG);
                appCtrlData.ctrlTaskState = APP_CTRL_SHUTDOWN_OPT3001;
            }
            else if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR){
                APP_CTRL_DBG(SYS_ERROR_ERROR, "I2C write MCP9808_REG_CONFIG error \r\n");
                appCtrlData.ctrlTaskState = APP_CTRL_SHUTDOWN_OPT3001;
            }
            break;
        }
        
        /* OPT3001 shutdown (to save power)*/
        case APP_CTRL_SHUTDOWN_OPT3001:
        {
            appCtrlData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
            appCtrlData.turnOnSensors = false;
            if(i2cWriteReg(OPT3001_I2C_ADDRESS, OPT3001_REG_CONFIG, OPT3001_CONFIG_SHUTDOWN))
                appCtrlData.ctrlTaskState = APP_CTRL_WAIT_SHUTDOWN_OPT3001;
            else
                appCtrlData.ctrlTaskState = APP_CTRL_READ_TEMP;
            break;
        }
        
        /* OPT3001 wait for shutdown */
        case APP_CTRL_WAIT_SHUTDOWN_OPT3001:
        {
            if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS){
                appCtrlData.opt3001.IsShutdown = true;
                i2cWriteRegComp(OPT3001_I2C_ADDRESS, OPT3001_REG_CONFIG);
                appCtrlData.ctrlTaskState = APP_CTRL_READ_TEMP;
            }
            else if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR){
                APP_CTRL_DBG(SYS_ERROR_ERROR, "I2C write OPT3001_REG_CONFIG error \r\n");
                appCtrlData.ctrlTaskState = APP_CTRL_READ_TEMP;
            }
            break;
        }
        
        /* MCP9808 read ambient temperature */
        case APP_CTRL_READ_TEMP:
        {
            /* Schedule MCP9808 temperature reading */
            appCtrlData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
                if(i2cReadReg(MCP9808_I2C_ADDRESS, MCP9808_REG_TAMBIENT, 2))
                    appCtrlData.ctrlTaskState = APP_CTRL_WAIT_READ_TEMP;
                else
                    appCtrlData.ctrlTaskState = APP_CTRL_READ_LIGHT;
            break;
        }
        
        /* MCP9808 wait for read ambient temperature */
        case APP_CTRL_WAIT_READ_TEMP:
        {
            /* MCP9808 Temperature reading operation done */
            if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS){
                i2cReadRegComp(MCP9808_I2C_ADDRESS, MCP9808_REG_TAMBIENT);
                appCtrlData.ctrlTaskState = APP_CTRL_READ_LIGHT;
            }
            else if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR){
                APP_CTRL_DBG(SYS_ERROR_ERROR, "I2C read temperature error \r\n");
                appCtrlData.ctrlTaskState = APP_CTRL_READ_LIGHT;
            }
            break;
        }
        
        /* OPT3001 read ambient light */
        case APP_CTRL_READ_LIGHT:
        {
            /* Schedule OPT3001 light reading */
            appCtrlData.i2c.transferStatus = I2C_TRANSFER_STATUS_IN_PROGRESS;
                if(i2cReadReg(OPT3001_I2C_ADDRESS, OPT3001_REG_RESULT, 2))
                    appCtrlData.ctrlTaskState = APP_CTRL_WAIT_READ_LIGHT;
                else
                    appCtrlData.ctrlTaskState = APP_CTRL_CHECK;
            break;
        }
        
        /* OPT3001 wait for read ambient light */
        case APP_CTRL_WAIT_READ_LIGHT:
        {
            /* OPT3001 Light reading operation done */
            if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_SUCCESS){
                i2cReadRegComp(OPT3001_I2C_ADDRESS, OPT3001_REG_RESULT);
                appCtrlData.ctrlTaskState = APP_CTRL_CHECK;
            }
            else if(appCtrlData.i2c.transferStatus == I2C_TRANSFER_STATUS_ERROR){
                APP_CTRL_DBG(SYS_ERROR_ERROR, "I2C read light error \r\n");
                appCtrlData.ctrlTaskState = APP_CTRL_CHECK;
            }
            break;
        }
        
        /* Idle */
        case APP_CTRL_IDLE:
        {
            break;
        }
        
        /* Error */
        case APP_CTRL_ERROR:
        {
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
