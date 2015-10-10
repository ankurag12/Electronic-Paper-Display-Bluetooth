/*****< main.c >***************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAIN - Main application implementation.                                   */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "Main.h"                /* Main application header.                  */

#include "Hardware/msp430/HAL.h"                 /* Function for Hardware Abstraction.        */
#include "Hardware/msp430/msp430-epd.h"
#include "HCITRANS.h"            /* HCI Transport Prototypes/Constants.       */
#include "epd_app/app.h"
#include <stdio.h>
#include "SPPDemo.h"


#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define LED_TOGGLE_RATE_SUCCESS                    (500) /* The LED Toggle    */
                                                         /* rate when the demo*/
                                                         /* successfully      */
                                                         /* starts up.        */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int BluetoothStackID;
static unsigned int InputIndex;
static Boolean_t    SleepEnabled;
static char         Input[MAX_COMMAND_LENGTH];


   /* Internal function prototypes.                                     */
static int GetInput(void);

   /* HCI Sleep Mode Callback.                                          */
static void BTPSAPI HCI_Sleep_Callback(Boolean_t SleepAllowed, unsigned long CallbackParameter);

   /* Application Tasks.                                                */
static void DisplayCallback(char Character);
static unsigned long GetTickCallback(void);
static void ProcessCharacters(void);
static void IdleFunction(void *UserParameter);
static void MainThread(void);

   /* The following function is responsible for retrieving the Commands */
   /* from the Serial Input routines and copying this Command into the  */
   /* specified Buffer.  This function blocks until a Command (defined  */
   /* to be a NULL terminated ASCII string).  The Serial Data Callback  */
   /* is responsible for building the Command and dispatching the Signal*/
   /* that this function waits for.                                     */
/*static int GetInput(void)
{
   char Char;
   int  Done;

    Initialize the Flag indicating a complete line has been parsed.
   Done = 0;

    Attempt to read data from the Console.
   while((!Done) && (HAL_ConsoleRead(1, &Char)))
   {
      switch(Char)
      {
         case '\r':
         case '\n':
             This character is a new-line or a line-feed character
             NULL terminate the Input Buffer.
            Input[InputIndex] = '\0';

             Set Done to the number of bytes that are to be returned.
             ** NOTE ** In the current implementation any data after a
                        new-line or line-feed character will be lost.
                        This is fine for how this function is used is
                        no more data will be entered after a new-line
                        or line-feed.
            Done       = (InputIndex-1);
            InputIndex = 0;
            break;
         case 0x08:
             Backspace has been pressed, so now decrement the number
             of bytes in the buffer (if there are bytes in the
             buffer).
            if(InputIndex)
            {
               InputIndex--;
               HAL_ConsoleWrite(3, "\b \b");
            }
            break;
         default:
             Accept any other printable characters.
            if((Char >= ' ') && (Char <= '~'))
            {
                Add the Data Byte to the Input Buffer, and make sure
                that we do not overwrite the Input Buffer.
               Input[InputIndex++] = Char;
               HAL_ConsoleWrite(1, &Char);

                Check to see if we have reached the end of the buffer.
               if(InputIndex == (MAX_COMMAND_LENGTH-1))
               {
                  Input[InputIndex] = 0;
                  Done              = (InputIndex-1);
                  InputIndex        = 0;
               }
            }
            break;
      }
   }

   return(Done);
}*/

   /* The following is the HCI Sleep Callback.  This is registered with */
   /* the stack to note when the Host processor may enter into a sleep  */
   /* mode.                                                             */
static void BTPSAPI HCI_Sleep_Callback(Boolean_t SleepAllowed, unsigned long CallbackParameter)
{
   /* Simply store the state internally.                                */
   SleepEnabled = SleepAllowed;
}

   /* The following function is registered with the application so that */
   /* it can display strings to the debug UART.                         */
static void DisplayCallback(char Character)
{
   HAL_ConsoleWrite(1, &Character);
}

   /* The following function is registered with the application so that */
   /* it can get the current System Tick Count.                         */
static unsigned long GetTickCallback(void)
{
   return(HAL_GetTickCount());
}

   /* The following function processes terminal input.                  */
/*
static void ProcessCharacters(void)
{
    Check to see if we have a command to process.
   if(GetInput() > 0)
   {
       Attempt to process a character.
      ProcessCommandLine(Input);
   }
}
*/

   /* The following function is responsible for checking the idle state */
   /* and possibly entering LPM3 mode.                                  */
static void IdleFunction(void *UserParameter)
{
   unsigned long        CurrentTickCount;
   unsigned long        ElapsedTicks;
   static unsigned long PreviousTickCount = 0;

   /* If the stack is Idle and we are in HCILL Sleep, then we may enter */
   /* LPM3 mode (with Timer Interrupts disabled).                       */
   if((BSC_QueryStackIdle(BluetoothStackID)) && (SleepEnabled))
   {
      /* Attempt to suspend the UART.                                   */
      if(!HCITR_COMSuspend(0))
      {
         /* Enter MSP430 LPM3 with Timer Interrupts disabled (we will   */
         /* require an interrupt to wake us up from this state).        */
         HAL_LowPowerMode((unsigned char)TRUE);

         /* Check to see if a wakeup is in progress (by the Controller).*/
         /* If so we will disable sleep mode so that we complete the    */
         /* process.                                                    */
         if(!HCITR_UartSuspended(0))
            SleepEnabled = FALSE;

         /* Go ahead and process any characters we may have received on */
         /* the console UART.                                           */
         //ProcessCharacters();
      }
      else
      {
         /* Failed to suspend the UART which must mean that the         */
         /* controller is attempting to do a wakeup.  Therefore we will */
         /* flag that sleep mode is disabled.                           */
         SleepEnabled = FALSE;
      }
   }
   else
   {
      /* Calculate the time that we last toggled the LED to determine if*/
      /* we need to do so again.                                        */
      CurrentTickCount = BTPS_GetTickCount();
      ElapsedTicks     = CurrentTickCount - PreviousTickCount;
      if(ElapsedTicks & 0x80000000)
         ElapsedTicks = CurrentTickCount + (0xFFFFFFFF - PreviousTickCount) + 1;

      if(ElapsedTicks >= LED_TOGGLE_RATE_SUCCESS)
      {
         /* Toggle the LEDs.                                            */
         HAL_LedToggle(0);

         /* Set the tick count for the next toggle.                     */
         PreviousTickCount = CurrentTickCount;
      }

      /* Process any console characters that we may have.               */
      //ProcessCharacters();

      /* Enter LPM0 while we wait for something to happen.              */
      LPM0;
   }
}


static void TestFunction(void *UserParameter){

	Display(("Hello"));
	BTPS_Delay(5000);
}

static void CheckForRcvdCmd(void *UserParameter){

	int ret_val;
	ret_val = ReadCmdFromPhoneApp();
	//Display(("ret_val = %d ",ret_val));

}

   /* The following function is the main user interface thread.  It     */
   /* opens the Bluetooth Stack and then drives the main user interface.*/
static void MainThread(void)
{
   int                           Result;
   BTPS_Initialization_t         BTPS_Initialization;
   HCI_DriverInformation_t       HCI_DriverInformation;
   HCI_HCILLConfiguration_t      HCILLConfig;
   HCI_Driver_Reconfigure_Data_t DriverReconfigureData;

   /* Flag that sleep is not currently enabled.                         */
   SleepEnabled = FALSE;

   /* Configure the UART Parameters.                                    */
   HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 1, 115200, cpHCILL_RTS_CTS);
   HCI_DriverInformation.DriverInformation.COMMDriverInformation.InitializationDelay = 100;

   /* Set up the application callbacks.                                 */
   BTPS_Initialization.GetTickCountCallback  = GetTickCallback;
   BTPS_Initialization.MessageOutputCallback = DisplayCallback;

   /* Initialize the application.                                       */
   if((Result = InitializeApplication(&HCI_DriverInformation, &BTPS_Initialization)) > 0)
   {
      /* Save the Bluetooth Stack ID.                                   */
      BluetoothStackID = (unsigned int)Result;

      /* Register a sleep mode callback if we are using HCILL Mode.     */
      if((HCI_DriverInformation.DriverInformation.COMMDriverInformation.Protocol == cpHCILL) || (HCI_DriverInformation.DriverInformation.COMMDriverInformation.Protocol == cpHCILL_RTS_CTS))
      {
         HCILLConfig.SleepCallbackFunction        = HCI_Sleep_Callback;
         HCILLConfig.SleepCallbackParameter       = 0;
         DriverReconfigureData.ReconfigureCommand = HCI_COMM_DRIVER_RECONFIGURE_DATA_COMMAND_CHANGE_HCILL_PARAMETERS;
         DriverReconfigureData.ReconfigureData    = (void *)&HCILLConfig;

         /* Register the sleep mode callback.  Note that if this        */
         /* function returns greater than 0 then sleep is currently     */
         /* enabled.                                                    */
         Result = HCI_Reconfigure_Driver(BluetoothStackID, FALSE, &DriverReconfigureData);
         if(Result > 0)
         {
            /* Flag that sleep mode is enabled.                         */
            SleepEnabled = TRUE;
         }
      }



      /* Add the idle function (which determines if LPM3 may be entered)*/
      /* to the scheduler.                                              */
      BTPS_AddFunctionToScheduler(CheckForRcvdCmd, NULL, 1);
      //if(BTPS_AddFunctionToScheduler(IdleFunction, NULL, 1))

      BTPS_ExecuteScheduler();

   }
}

   /* The following is the Main application entry point.  This function */
   /* will configure the hardware and initialize the OS Abstraction     */
   /* layer, create the Main application thread and start the scheduler.*/
int main(void)
{
   /* Turn off the watchdog timer                                       */
   WDTCTL = WDTPW | WDTHOLD;

   /* Configure the hardware for its intended use.                      */
   HAL_ConfigureHardware();

   /* Enable interrupts  				   					           */
   __enable_interrupt();

   /* Initialize EPD sub systems										*/
   epd_sys_init();

   int ret_val;

   /* call the main application thread.									*/
   MainThread();

   /* MainThread should run continously, if it exits an error occured.  */

   {
      HAL_LedToggle(0);
      BTPS_Delay(100);
   }

   return 0;
}


