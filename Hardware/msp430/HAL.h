/*****< HAL.h >****************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HAL - Hardware Abstraction functions for Stellaris LM3S9B96 Board         */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/05/11  Tim Thomas     Initial creation.                               */
/******************************************************************************/
#ifndef __HAL_H__
#define __HAL_H__
#include <msp430.h>

   /* The following define the valid Peripheral values that may be      */
   /* passed into HAL_EnableSMCLK() and HAL_DisableSMCLK().             */
#define HAL_PERIPHERAL_DEBUG_UART                        0x01
#define HAL_PERIPHERAL_BLUETOOTH_UART                    0x02

   /* The following function is used to place the hardware into a known */
   /* state.                                                            */
void HAL_ConfigureHardware(void);

   /* * NOTE * The following are the allowed flags for the flags        */
   /*          argument.                                                */
   /*  1.  UART_CONFIG_WLEN_8, UART_CONFIG_WLEN_7                       */
   /*  2.  UART_CONFIG_PAR_NONE,UART_CONFIG_PAR_ODD,UART_CONFIG_PAR_EVEN*/
   /*  3.  UART_CONFIG_STOP_ONE,UART_CONFIG_STOP_TWO                    */
   /*          The flags is a bitfield which may include one flag from  */
   /*          each of the three rows above                             */
void HAL_CommConfigure(unsigned char *UartBase, unsigned long BaudRate, unsigned char Flags);

   /* The following function is used to retreive data from the UART     */
   /* input queue.  The function receives a pointer to a buffer that    */
   /* will receive the UART characters a the length of the buffer.  The */
   /* function will return the number of characters that were returned  */
   /* in Buffer.                                                        */
int HAL_ConsoleRead(unsigned int Length, char *Buffer);

   /* The following function is used to send data to the UART output    */
   /* queue.  The function receives a pointer to a buffer that will     */
   /* contains the data to send and the length of the data.             */
void HAL_ConsoleWrite(unsigned int Length, char *Buffer);

   /* The following function is used to return the configured system    */
   /* clock speed in MHz.                                               */
unsigned long HAL_GetSystemSpeed(void);

   /* This function is called to get the system Tick Count.             */
unsigned long HAL_GetTickCount(void);

   /* The following function is used to toggle the state of an LED.  The*/
   /* number of LEDs on a board is board specific.  If the LED_ID       */
   /* provided does not exist on the hardware platform then nothing is  */
   /* done.                                                             */
void HAL_LedToggle(int LED_ID);

   /* The following function is used to set an LED to a specified state.*/
void HAL_SetLED(int LED_ID, int State);

   /* The following function is called to enable the SMCLK Peripheral   */
   /* on the MSP430.                                                    */
   /* * NOTE * This function should be called with interrupts disabled. */
void HAL_EnableSMCLK(unsigned char Peripheral);

   /* The following function is called to disable the SMCLK Peripheral  */
   /* on the MSP430.                                                    */
   /* * NOTE * This function should be called with interrupts disabled. */
void HAL_DisableSMCLK(unsigned char Peripheral);

   /* The following function is called to enter LPM3 mode on the MSP    */
   /* with the OS Timer Tick Disabled.                                  */
void HAL_LowPowerMode(unsigned char DisableLED);

#endif

