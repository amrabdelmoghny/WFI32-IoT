/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ctrl.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_CTRL_H
#define _APP_CTRL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "definitions.h"
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************

#define APP_CTRL_DBG(level,fmt,...) SYS_DEBUG_PRINT(level,"[APP_CTRL] "fmt, ##__VA_ARGS__)
#define APP_CTRL_PRNT(fmt,...) SYS_CONSOLE_PRINT("[APP_CTRL] "fmt, ##__VA_ARGS__)

// *****************************************************************************
    
// *****************************************************************************

/* LEDs */
typedef enum 
{
    LED_RED = 0,
    LED_GREEN,
    LED_YELLOW,
    LED_BLUE,
    LED_COLOR_IVALID,
} LED_COLOR;

/* LED mode */
typedef enum 
{
    LED_ON = 0,
    LED_OFF,
    LED_TOGGLE,
    LED_F_BLINK,                  /* A fast blink (always started in 'on' state) */
    LED_S_BLINK_STARTING_ON,      /* A slow blink started in 'on' state */
    LED_S_BLINK_STARTING_OFF,     /* A slow blink started in 'off' state */
    LED_MODE_INVALID,
} LED_MODE;

/* Blinking LED mode */
typedef enum 
{
    BLINK_MODE_SINGLE = 0,
    BLINK_MODE_PERIODIC,
    BLINK_MODE_INVALID,
} LED_BLINK_MODE;

// *****************************************************************************

/* Sensors Turn on */
void APP_sensorsOn(void);
void APP_sensorsOff(void);
int16_t APP_readTemp(void);
uint32_t APP_readLight(void);
void APP_manageLed(LED_COLOR, LED_MODE, LED_BLINK_MODE);
void APP_InitializeCtrl (void);
void APP_TaskCtrl ( void );

#endif /* _APP_CTRL_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

