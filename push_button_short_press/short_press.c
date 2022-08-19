
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

/* Define the duration of short press in 500 ms */
const uint32_t ui32_SHORT_PRESS_TIME = 500;

/* Store the number of milliseconds since system boot up. This variable will overflow in 49 days. */
uint32_t ui32Milliseconds = 0;

/* System clock rate in Hz */
uint32_t ui32SysClock;

/* Interrupt handler for TIMER1 */
void Timer1IntHandler(void){
    /* Clear the timer interrupt */
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    /* Increase the number of milliseconds */
    ui32Milliseconds++;
}

/**
 * main.c
 */
int main(void)
{
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
            SYSCTL_OSC_MAIN |
            SYSCTL_USE_PLL |
            SYSCTL_CFG_VCO_480), 120000000); /* Set system clock to run at 120 MHz */


    /* Enable the peripheral TIMER1 */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    /* Enable processor interrupts */
    IntMasterEnable();

    /* Configure the 32 bit periodic timer */
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, ui32SysClock / 1000); /* Trigger interrupt each milisecond */

    /* Setup the interrupts for the timer interrupts */
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    /* Enable the timer */
    TimerEnable(TIMER1_BASE, TIMER_A);

    /* Enable the GPIOJ peripheral for Switch 1 PJ0 */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);

    /* Setup GPIO J0 as input*/
    GPIODirModeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_DIR_MODE_IN);

    /* Set GPIO Pin 0 as a pull-up resistor */
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


    /* Enable the GPION peripheral for LED 1 PN1 */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    /* Set GPIO N as Output */
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00);

    uint32_t lastState = 0x01;
    uint32_t currentState = 0x01;
    uint32_t pressedTime = 0x00;
    uint32_t releasedTime = 0x00;
    uint32_t pressDuration = 0x00;

    while(1){

        if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == 0) {
            currentState = 0;
        } else {
            currentState = 1;
        }
        if (lastState == 1 && currentState == 0) { /* button is pressed */
            pressedTime = ui32Milliseconds;
        }else {
            if(lastState == 0 && currentState == 1){ /* button is released */
                releasedTime = ui32Milliseconds;
            }
        }
        pressDuration = releasedTime - pressedTime;

        if(pressDuration > 1 && pressDuration < ui32_SHORT_PRESS_TIME)
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x02);

        /* Save the last state */
        lastState = currentState;

    }

	return 0;
}
