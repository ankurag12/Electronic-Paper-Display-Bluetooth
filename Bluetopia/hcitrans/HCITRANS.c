/*****< hcitrans.c >***********************************************************/
/*      Copyright 2000 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  UART HCITRANS - HCI Transport Layer for use with Bluetopia.               */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/25/01  R. Sledge      Initial creation.                               */
/******************************************************************************/
#include "../../Hardware/msp430/HAL.h"                 /* MSP430 Hardware Abstraction API.          */
#include "../../Hardware/msp430/HRDWCFG.h"             /* MSP430 Exp Board Setup and Utilities      */
#include "BTPSKRNL.h"            /* Bluetooth Kernel Protoypes/Constants.     */
#include "HCITRANS.h"            /* HCI Transport Prototypes/Constants.       */


#define TRANSPORT_ID                                           1

   /* Macro to wake up CPU from LPM.                                    */
#define LPM_EXIT()                                             LPM3_EXIT

   /* The following MACRO returns a Boolean that is TRUE if the UART    */
   /* Transmit is Active or FALSE otherwise.                            */
#define UART_TRANSMIT_ACTIVE()                                 (HWREG8(UartContext.UartBase + MSP430_UART_STAT_OFFSET) & 0x01)

   /* The following MACRO is used to raise RTS so that the Bluetooth    */
   /* chip will not send us any data until we are ready to receive the  */
   /* data.                                                             */
#define FLOW_OFF()                                             (BT_DISABLE_FLOW())

   /* The following MACRO is used to re-enable RTS if necessary.  This  */
   /* MACRO will not lower RTS unless UART_CONTEXT_FLAG_FLOW_ENABLED is */
   /* set in the UartContext Flags AND (UART_CONTEXT_FLAG_RTS_HIGH) are */
   /* both cleared.                                                     */
#define FLOW_ON()                                                                                                    \
{                                                                                                                    \
   if ((UartContext.Flags & (UART_CONTEXT_FLAG_FLOW_ENABLED | UART_CONTEXT_FLAG_RTS_HIGH | UART_CONTEXT_FLAG_UART_SUSPENDED)) == UART_CONTEXT_FLAG_FLOW_ENABLED)  \
      BT_ENABLE_FLOW();                                                                                              \
}

   /* Macros to change the interrupt state.                             */
#define DISABLE_INTERRUPTS() \
{                            \
   __disable_interrupt();    \
}

#define ENABLE_INTERRUPTS()  \
{                            \
   __enable_interrupt();     \
}

   /* TI Chip Defaults.                                                 */

   /* The following defines the TI Bluetooth chips startup baud rate.   */
   /* This value can't be changed and is only here to increase code     */
   /* readability.  That means that regardless what this is defined to  */
   /* be the TI CC256x will always come up at 115200 baud.              */
#define BLUETOOTH_STARTUP_BAUD_RATE                              115200L

#define DEFAULT_INPUT_BUFFER_SIZE                                128
#define DEFAULT_OUTPUT_BUFFER_SIZE                               64
#define XOFF_LIMIT                                               8
#define XON_LIMIT                                                16

#define UART_CONTEXT_FLAG_OPEN_STATE                             0x0001
#define UART_CONTEXT_FLAG_FLOW_ENABLED                           0x0002
#define UART_CONTEXT_FLAG_RX_OVERRUN                             0x0004
#define UART_CONTEXT_FLAG_TRANSMIT_ENABLED                       0x0008
#define UART_CONTEXT_FLAG_TX_FLOW_ENABLED                        0x0010
#define UART_CONTEXT_FLAG_TX_PRIMED                              0x0020
#define UART_CONTEXT_FLAG_RTS_HIGH                               0x0040
#define UART_CONTEXT_FLAG_UART_SUSPENDED                         0x0080

typedef enum
{
   hssNormal,
   hssSuspendWait,
   hssSuspendWaitInterrupted,
   hssSuspended,
   hssSuspendedCTSWakeup
} SuspendState_t;

typedef struct _tagUartContext_t
{
   unsigned char   ID;
   unsigned char  *UartBase;
   SuspendState_t  SuspendState;
   unsigned char   RxBuffer[DEFAULT_INPUT_BUFFER_SIZE];
   int             RxBufferSize;
   volatile int    RxBytesFree;
   int             RxInIndex;
   int             RxOutIndex;
   int             XOffLimit;
   int             XOnLimit;
   unsigned char   TxBuffer[DEFAULT_OUTPUT_BUFFER_SIZE];
   int             TxBufferSize;
   volatile int    TxBytesFree;
   int             TxInIndex;
   int             TxOutIndex;
   unsigned char   Flags;
} UartContext_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static UartContext_t  UartContext;
static unsigned int   HCITransportOpen;

   /* COM Data Callback Function and Callback Parameter information.    */
static HCITR_COMDataCallback_t _COMDataCallback;
static unsigned long           _COMCallbackParameter;

   /* Local Function Prototypes.                                        */
static void FlushRxFIFO(unsigned char *Base);
static void TxTransmit(void);
static void RxProcess(void);
static void DisableTransmitter(void);
static void EnableTransmitter(void);
static void LoadTransmitBuffer(unsigned int Length, unsigned char *Buffer);

   /* The following function is used to unload all of the characters in */
   /* the receive FIFO.                                                 */
static void FlushRxFIFO(unsigned char *Base)
{
   unsigned long Dummy;

   /* WHILE FIFO NOT EMPTY.                                             */
   while(!UARTReceiveBufferEmpty(Base))
   {
      /* Remove the character from the FIFO.                            */
      Dummy = UARTReceiveBufferReg(Base);
      if(Dummy)
         Dummy = 0;
   }
}

   /* The following function, which should be called with interrupts    */
   /* disabled, exists to transmit a character and update the Tx Buffer */
   /* Counts.                                                           */
static void TxTransmit(void)
{
   /* if Tx Flow is Enabled (CTS is low) and the Tx Buffer is not empty */
   /* send a character and update the counts.                           */
   if((UartContext.Flags & UART_CONTEXT_FLAG_TX_FLOW_ENABLED) && (UartContext.TxBytesFree != UartContext.TxBufferSize))
   {
      /* Load the Tx Buffer.                                            */
      UARTTransmitBufferReg(UartContext.UartBase) = UartContext.TxBuffer[UartContext.TxOutIndex];

      /* Update the circular buffer counts.                             */
      UartContext.TxBytesFree++;
      UartContext.TxOutIndex++;

      /* Check if we need to wrap the buffer.                           */
      if(UartContext.TxOutIndex >= UartContext.TxBufferSize)
         UartContext.TxOutIndex = 0;

      /* The UART Tx side is now primed.                                */
      UartContext.Flags |= UART_CONTEXT_FLAG_TX_PRIMED;
   }
   else
   {
      /* Disable the transmit interrupt.                                */
      UARTIntDisableTransmit(UartContext.UartBase);

      /* The Tx Transmitter is no longer primed.                        */
      UartContext.Flags &= ~UART_CONTEXT_FLAG_TX_PRIMED;
   }
}

   /* The following function is the Interrupt Service Routine for the   */
   /* UART RX interrupt.                                                */
#pragma vector=BT_UART_IV
__interrupt void UartInterrupt(void)
{
   Word_t        VectorRegister;
   volatile char Dummy;

   /* Read the Vector register once to determine the cause of the       */
   /* interrupt.                                                        */
   VectorRegister = BT_UART_IVR;

   /* Determine the cause of the interrupt (Rx or Tx).                  */
   if(VectorRegister == USCI_UCRXIFG)
   {
      /* Check to see if there is buffer space to receive the data.     */
      if(UartContext.RxBytesFree)
      {
         /* check to see if an overrun occured.                         */
         if(HWREG8(UartContext.UartBase + MSP430_UART_STAT_OFFSET) & 0xEC)
            HAL_ConsoleWrite(1, "?");

         /* Read the character from the UART Receive Buf.               */
         UartContext.RxBuffer[UartContext.RxInIndex++] = UARTReceiveBufferReg(UartContext.UartBase);

         /* Credit the received character.                              */
         --(UartContext.RxBytesFree);

         /* Check to see if we have reached the end of the Buffer and   */
         /* need to loop back to the beginning.                         */
         if(UartContext.RxInIndex >= UartContext.RxBufferSize)
            UartContext.RxInIndex = 0;

         /* Check to see if we need to Disable Rx Flow                  */
         /* RxThread will re-enable flow control when it is possible    */
         if((UartContext.Flags & UART_CONTEXT_FLAG_FLOW_ENABLED) && (UartContext.RxBytesFree <= UartContext.XOffLimit))
         {
            /* if Flow is Enabled then disable it                       */
            UartContext.Flags &= (~UART_CONTEXT_FLAG_FLOW_ENABLED);
            FLOW_OFF();
         }
      }
      else
      {
         /* We have data in the FIFO, but no place to put the data,     */
         /* so will will have to flush the FIFO and discard the data.   */
         Dummy = UARTReceiveBufferReg(UartContext.UartBase);

         /* Flag that we have encountered an RX Overrun.                */
         /* Also Disable Rx Flow.                                       */
         UartContext.Flags |= UART_CONTEXT_FLAG_RX_OVERRUN;
         UartContext.Flags &= (~UART_CONTEXT_FLAG_FLOW_ENABLED);
         FLOW_OFF();
      }

      /* If the UART is in the process of shutting down flag that this  */
      /* has been interrupted due to the arrival of UART data.          */
      if(UartContext.SuspendState == hssSuspendWait)
      {
         /* Indicate the suspend is interrupted.                        */
         UartContext.SuspendState = hssSuspendWaitInterrupted;
      }
   }
   else
   {
      if(VectorRegister == USCI_UCTXIFG)
      {
         /* Process the Transmit Empty Interrupt.                       */
         TxTransmit();
      }
   }

   /* Exit from LPM if necessary (this statement will have no effect if */
   /* we are not currently in low power mode).                          */
   LPM_EXIT();
}

   /* The following function is the CTS Pin Change Interrupt.           */
   /* It is called when the CTS Line changes. This routine must change  */
   /* the interrupt polarity and flag what state the CTS line is in.    */
int CtsInterrupt(void)
{
   int ret_val = 0;

   /* Check the current GPIO Interrupt Edge Polarity.                   */
   /* A negative edge polarity indicates that the CTS pin is low, this  */
   /* means that we can re-enable TX Flow and set up the DMA to send    */
   /* more data. Pos Edge Polarity means we should turn off Tx Flow and */
   /* credit and data that was sent before this happened and turn off   */
   /* the DMA Controller.                                               */
   if(BT_CTS_INT_IS_NEG_EDGE())
   {
      /* The CTS Interrupt should now be set to positive edge since CTS */
      /* is low.                                                        */
      BT_CTS_INT_POS_EDGE();

      /* Flag that we are now allowed to continue sending data.         */
      UartContext.Flags |= UART_CONTEXT_FLAG_TX_FLOW_ENABLED;

      /* If the transmitter is enabled go ahead and prime the UART if   */
      /* needed.                                                        */
      if((UartContext.Flags & UART_CONTEXT_FLAG_TRANSMIT_ENABLED) && (UartContext.TxBytesFree != UartContext.TxBufferSize))
      {
         /* Re-enable the transmit interrupt.                           */
         UARTIntEnableTransmit(UartContext.UartBase);

         /* Reprime the transmitter.                                    */
         TxTransmit();
      }

      /* Check to see if this is a CTS Wakeup.  If so we need to        */
      /* de-assert RTS so the controller can send a wakeup byte.        */
      if(UartContext.SuspendState == hssSuspendedCTSWakeup)
      {
         /* Re-set the UART state to Normal.                            */
         UartContext.SuspendState  = hssNormal;
         UartContext.Flags        &= ~UART_CONTEXT_FLAG_UART_SUSPENDED;
         FLOW_ON();
      }
   }
   else
   {
      /* Negative edge active CTS Interrupt (CTS is currently high).    */
      BT_CTS_INT_NEG_EDGE();

      /* Flag that we cannot transmit.                                  */
      UartContext.Flags &= (~UART_CONTEXT_FLAG_TX_FLOW_ENABLED);

      /* Flag that we are no longer primed.                             */
      UartContext.Flags &= (~UART_CONTEXT_FLAG_TX_PRIMED);

      /* Check to see what the current UART State is.  If the UART is   */
      /* currently suspended we will re-start the UART here as this must*/
      /* mean that we are in HCILL sleep mode and the Bluetooth         */
      /* Controller is attempting to wake us.                           */
      if(UartContext.SuspendState == hssSuspended)
      {
#ifdef __DISABLE_SMCLK__

         /* Request that the SMCLK stay active.                         */
         /* * NOTE * Since we are executing code the SMCLK is currently */
         /*          active, however what this function calls does is   */
         /*          enable SMCLK requests, so that when LPM3 is next   */
         /*          entered the UART may request that the clock stays  */
         /*          active.                                            */
         HAL_EnableSMCLK(HAL_PERIPHERAL_BLUETOOTH_UART);

#endif

         /* Flag that the UART clock is being resumed.                  */
         UartContext.SuspendState  = hssSuspendedCTSWakeup;
      }
      else
      {
         /* We are currently in the process of suspending the UART.     */
         /* Since the CTS line is now high we should interrupt the      */
         /* suspension of the UART.                                     */
         if(UartContext.SuspendState == hssSuspendWait)
         {
            /* Indicate the suspend is interrupted.                     */
            UartContext.SuspendState = hssSuspendWaitInterrupted;
         }
      }

      /* Flag that LPM3 should be exited on return.                     */
      ret_val = 1;
   }

   return(ret_val);
}

   /* The following thread is used to process the data that has been    */
   /* received from the UART and placed in the receive buffer.          */
static void RxProcess(void)
{
   unsigned int MaxWrite;
   unsigned int Count;

   /* Determine the number of characters that can be delivered.         */
   Count = (UartContext.RxBufferSize - UartContext.RxBytesFree);
   if(Count)
   {
      /* Determine the maximum number of characters that we can send    */
      /* before we reach the end of the buffer.  We need to process     */
      /* the smaller of the max characters of the number of             */
      /* characters that are in the buffer.                             */
      MaxWrite = (UartContext.RxBufferSize-UartContext.RxOutIndex);
      Count    = (MaxWrite < Count)?MaxWrite:Count;

      /* Call the upper layer back with the data.                       */
      if((Count) && (_COMDataCallback))
         (*_COMDataCallback)(TRANSPORT_ID, Count, &UartContext.RxBuffer[UartContext.RxOutIndex], _COMCallbackParameter);

      /* Adjust the Out Index and handle any looping.                   */
      UartContext.RxOutIndex += Count;
      if(UartContext.RxOutIndex >= UartContext.RxBufferSize)
         UartContext.RxOutIndex = 0;

      /* Enter a critical region to update counts and also create       */
      /* new Rx records if needed.                                      */
      DISABLE_INTERRUPTS();

      /* Credit the amount that was sent to the upper layer.            */
      UartContext.RxBytesFree += Count;

      ENABLE_INTERRUPTS();

      if((!(UartContext.Flags & UART_CONTEXT_FLAG_FLOW_ENABLED)) && (UartContext.RxBytesFree >= UartContext.XOnLimit) && (UartContext.SuspendState == hssNormal))
      {
         DISABLE_INTERRUPTS();

         /* Flag that flow is re-enabled.                               */
         UartContext.Flags |= UART_CONTEXT_FLAG_FLOW_ENABLED;

         ENABLE_INTERRUPTS();

         /* Re-enable flow                                              */
         FLOW_ON();
      }
   }
}

   /* The following function is used to disable Transmit Operation.     */
   /* * NOTE * This function MUST be called with interrupts DISABLED.   */
static void DisableTransmitter(void)
{
   /* Dis-able the transmit functionality.                              */
   UartContext.Flags &= ~UART_CONTEXT_FLAG_TRANSMIT_ENABLED;

   /* Disable the Transmit Interrupt.                                   */
   UARTIntDisableTransmit(UartContext.UartBase);
}

   /* The following function is used to re-enable Transmit Operation.   */
   /* * NOTE * This function MUST be called with interrupts DISABLED.   */
static void EnableTransmitter(void)
{
   /* Re-enable the transmit functionality.                             */
   UartContext.Flags |= UART_CONTEXT_FLAG_TRANSMIT_ENABLED;

   /* Check to see if we should reprime the transmitter.  We will only  */
   /* do this if CTS is low.                                            */
   if(UartContext.Flags & UART_CONTEXT_FLAG_TX_FLOW_ENABLED)
   {
      /* Re-enable the transmit interrupt.                              */
      UARTIntEnableTransmit(UartContext.UartBase);

      /* Reprime the transmitter.                                       */
      TxTransmit();
   }
}

   /* The following function is responsible for loading the Transmit    */
   /* Buffer with characters to transmit on the UART.  This function    */
   /* will spin until a spot is found in the Transmit Buffer for the    */
   /* requested characters and it will return the Index of the first    */
   /* character in the stream into the final parameter to this function.*/
static void LoadTransmitBuffer(unsigned int Length, unsigned char *Buffer)
{
   unsigned int Count;

   /* If the UART is suspended, resume it.                              */
   if(UartContext.SuspendState == hssSuspended)
   {
      DISABLE_INTERRUPTS();

#ifdef __DISABLE_SMCLK__

      /* Request that the SMCLK stay active.                            */
      /* * NOTE * Since we are executing code the SMCLK is currently    */
      /*          active, however what this function calls does is      */
      /*          enable SMCLK requests, so that when LPM3 is next      */
      /*          entered the UART may request that the clock stays     */
      /*          active.                                               */
      HAL_EnableSMCLK(HAL_PERIPHERAL_BLUETOOTH_UART);

#endif

      /* Flag that the UART clock has been resumed.                     */
      UartContext.SuspendState  = hssNormal;
      UartContext.Flags        &= ~UART_CONTEXT_FLAG_UART_SUSPENDED;
      FLOW_ON();

      ENABLE_INTERRUPTS();

      /* Add a delay while the SMCLK stabilizes.                        */
      __delay_cycles(2500);
   }

   /* Process all of the data.                                          */
   while(Length)
   {
      /* Loop until space becomes available in the Tx Buffer            */
      while (UartContext.TxBytesFree <= 0)
         ;

      /* The data may have to be copied in 2 phases.  Calculate the     */
      /* number of character that can be placed in the buffer before the*/
      /* buffer must be wrapped.                                        */
      Count = UartContext.TxBufferSize-UartContext.TxInIndex;

      /* Make sure we dont copy over data waiting to be sent.           */
      Count = (UartContext.TxBytesFree < Count)?(UartContext.TxBytesFree):(Count);

      /* Next make sure we arent trying to copy greater than what we are*/
      /* given.                                                         */
      Count = (Count > Length)?Length:Count;
      BTPS_MemCopy(&(UartContext.TxBuffer[UartContext.TxInIndex]), Buffer, Count);

      /* Update the number of free bytes in the buffer.  Since this     */
      /* count can also be updated in the interrupt routine, we will    */
      /* have have to update this with interrupts disabled.             */
      DISABLE_INTERRUPTS();
      UartContext.TxBytesFree -= Count;
      ENABLE_INTERRUPTS();

      /* Adjust the count and index values.                             */
      Buffer                += Count;
      Length                -= Count;
      UartContext.TxInIndex += Count;
      if(UartContext.TxInIndex >= UartContext.TxBufferSize)
         UartContext.TxInIndex = 0;

      /* Check to see if we need to prime the transmitter.  The Tx      */
      /* Interrupt will flag that that it is disabled if thinks that    */
      /* TxBytesFree == TxBufferSize, re-enable if it is necessary.     */
      if((UartContext.Flags & (UART_CONTEXT_FLAG_TRANSMIT_ENABLED | UART_CONTEXT_FLAG_TX_FLOW_ENABLED | UART_CONTEXT_FLAG_TX_PRIMED)) == (UART_CONTEXT_FLAG_TRANSMIT_ENABLED | UART_CONTEXT_FLAG_TX_FLOW_ENABLED))
      {
         /* Start sending data to the Uart Transmit FIFO.               */
         DISABLE_INTERRUPTS();

         /* Enable the transmit interrupt.                              */
         UARTIntEnableTransmit(UartContext.UartBase);

         /* Prime the transmitter.                                      */
         TxTransmit();

         ENABLE_INTERRUPTS();
      }
   }
}

   /* The following function is responsible for opening the HCI         */
   /* Transport layer that will be used by Bluetopia to send and receive*/
   /* COM (Serial) data.  This function must be successfully issued in  */
   /* order for Bluetopia to function.  This function accepts as its    */
   /* parameter the HCI COM Transport COM Information that is to be used*/
   /* to open the port.  The final two parameters specify the HCI       */
   /* Transport Data Callback and Callback Parameter (respectively) that*/
   /* is to be called when data is received from the UART.  A successful*/
   /* call to this function will return a non-zero, positive value which*/
   /* specifies the HCITransportID that is used with the remaining      */
   /* transport functions in this module.  This function returns a      */
   /* negative return value to signify an error.                        */
int BTPSAPI HCITR_COMOpen(HCI_COMMDriverInformation_t *COMMDriverInformation, HCITR_COMDataCallback_t COMDataCallback, unsigned long CallbackParameter)
{
   int ret_val;
   volatile char dummy = 0;

   /* First, make sure that the port is not already open and make sure  */
   /* that valid COMM Driver Information was specified.                 */
   if((!HCITransportOpen) && (COMMDriverInformation) && (COMDataCallback))
   {
      /* Initialize the return value for success.                       */
      ret_val               = TRANSPORT_ID;

      /* Note the COM Callback information.                             */
      _COMDataCallback      = COMDataCallback;
      _COMCallbackParameter = CallbackParameter;

      /* Try to Open the port for Reading/Writing.                      */
      BTPS_MemInitialize(&UartContext, 0, sizeof(UartContext_t));

      UartContext.UartBase     = BT_UART_MODULE_BASE;
      UartContext.ID           = 1;
      UartContext.RxBufferSize = DEFAULT_INPUT_BUFFER_SIZE;
      UartContext.RxBytesFree  = DEFAULT_INPUT_BUFFER_SIZE;
      UartContext.XOffLimit    = XOFF_LIMIT;
      UartContext.XOnLimit     = XON_LIMIT;
      UartContext.TxBufferSize = DEFAULT_OUTPUT_BUFFER_SIZE;
      UartContext.TxBytesFree  = DEFAULT_OUTPUT_BUFFER_SIZE;
      UartContext.SuspendState = hssNormal;

      /* Check to see if this is the first time that the port has been  */
      /* opened.                                                        */
      if(!HCITransportOpen)
      {
         /* Configure the Bluetooth Slow Clock.                         */
         BT_CONFIG_SLOW_CLOCK();

         /* Configure the TXD and RXD pins as UART peripheral pins.     */
         BT_CONFIG_UART_PINS();

         /* configures the RTS and CTS lines.                           */
         BT_CONFIG_RTS_PIN();
         BT_CONFIG_CTS_PIN();

         /* disable Flow through Local RTS line.                        */
         FLOW_OFF();

         /* configure the Device Reset line                             */
         BT_CONFIG_RESET();

         /* Set the Baud rate up.                                       */
         HAL_CommConfigure(UartContext.UartBase, BLUETOOTH_STARTUP_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

         /* Disable Tx Flow, later we will check RTS and see if we      */
         /* should enable it, but enable our receive flow.              */
         DISABLE_INTERRUPTS();
         UartContext.Flags &= (~UART_CONTEXT_FLAG_TX_FLOW_ENABLED);
         UartContext.Flags |= UART_CONTEXT_FLAG_FLOW_ENABLED;
         ENABLE_INTERRUPTS();

         /* Bring the Bluetooth Device out of Reset.                    */
         BT_DEVICE_RESET();
         BTPS_Delay(10);
         BT_DEVICE_UNRESET();

         /* Bring CTS Line Low to Indicate that we are ready to receive.*/
         FLOW_ON();

         /* Check to see if we need to enable Tx Flow.                  */
         if(BT_CTS_READ())
         {
            /* CTS is High so we cannot send data at this time. We will */
            /* configure the CTS Interrupt to be Negative Edge Active.  */
            DISABLE_INTERRUPTS();
            UartContext.Flags &= (~UART_CONTEXT_FLAG_TX_FLOW_ENABLED);
            BT_CTS_INT_NEG_EDGE();
            ENABLE_INTERRUPTS();
         }
         else
         {
            /* CTS is low and ergo we may send data to the controller.  */
            /* The CTS interrupt will be set to fire on the Positive    */
            /* Edge.                                                    */
            DISABLE_INTERRUPTS();
            UartContext.Flags |= (UART_CONTEXT_FLAG_TX_FLOW_ENABLED);
            BT_CTS_INT_POS_EDGE();
            ENABLE_INTERRUPTS();
         }

         /* Clear any data that is in the Buffer.                       */
         FlushRxFIFO(UartContext.UartBase);

         /* Enable Receive interrupt.                                   */
         UARTIntEnableReceive(UartContext.UartBase);

         /* Disable Transmit Interrupt.                                  */
         UARTIntDisableTransmit(UartContext.UartBase);

         DISABLE_INTERRUPTS();

         /* Flag that the UART Tx Buffer will need to be primed.        */
         UartContext.Flags &= (~UART_CONTEXT_FLAG_TX_PRIMED);

         /* Enable the transmit functionality.                          */
         UartContext.Flags |= UART_CONTEXT_FLAG_TRANSMIT_ENABLED;

         ENABLE_INTERRUPTS();

         /* Check to see if we need to delay after opening the COM Port.*/
         if(COMMDriverInformation->InitializationDelay)
            BTPS_Delay(COMMDriverInformation->InitializationDelay);

         /* Flag that the HCI Transport is open.                        */
         HCITransportOpen = 1;
      }
   }
   else
      ret_val = HCITR_ERROR_UNABLE_TO_OPEN_TRANSPORT;

   return(ret_val);
}

   /* The following function is responsible for closing the the specific*/
   /* HCI Transport layer that was opened via a successful call to the  */
   /* HCITR_COMOpen() function (specified by the first parameter).      */
   /* Bluetopia makes a call to this function whenever an either        */
   /* Bluetopia is closed, or an error occurs during initialization and */
   /* the driver has been opened (and ONLY in this case).  Once this    */
   /* function completes, the transport layer that was closed will no   */
   /* longer process received data until the transport layer is         */
   /* Re-Opened by calling the HCITR_COMOpen() function.                */
   /* * NOTE * This function *MUST* close the specified COM Port.       */
   /*          This module will then call the registered COM Data       */
   /*          Callback function with zero as the data length and NULL  */
   /*          as the data pointer.  This will signify to the HCI       */
   /*          Driver that this module is completely finished with the  */
   /*          port and information and (more importantly) that NO      */
   /*          further data callbacks will be issued.  In other words   */
   /*          the very last data callback that is issued from this     */
   /*          module *MUST* be a data callback specifying zero and NULL*/
   /*          for the data length and data buffer (respectively).      */
void BTPSAPI HCITR_COMClose(unsigned int HCITransportID)
{
   HCITR_COMDataCallback_t COMDataCallback;
   unsigned long           CallbackParameter;

   /* Check to make sure that the specified Transport ID is valid.      */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen))
   {
      /* Appears to be valid, go ahead and close the port.              */
      UARTIntDisableReceive(UartContext.UartBase);
      UARTIntDisableTransmit(UartContext.UartBase);

      /* Clear the UartContext Flags.                                   */
      DISABLE_INTERRUPTS();
      UartContext.Flags = 0;
      ENABLE_INTERRUPTS();

      /* Place the Bluetooth Device in Reset.                           */
      BT_DEVICE_RESET();

      /* Note the Callback information.                                 */
      COMDataCallback   = _COMDataCallback;
      CallbackParameter = _COMCallbackParameter;

      /* Flag that the HCI Transport is no longer open.                 */
      HCITransportOpen = 0;

      /* Flag that there is no callback information present.            */
      _COMDataCallback      = NULL;
      _COMCallbackParameter = 0;

      /* All finished, perform the callback to let the upper layer know */
      /* that this module will no longer issue data callbacks and is    */
      /* completely cleaned up.                                         */
      if(COMDataCallback)
         (*COMDataCallback)(HCITransportID, 0, NULL, CallbackParameter);
   }
}

   /* The following function is responsible for instructing the         */
   /* specified HCI Transport layer (first parameter) that was opened   */
   /* via a successful call to the HCITR_COMOpen() function to          */
   /* reconfigure itself with the specified information.  This          */
   /* information is completely opaque to the upper layers and is passed*/
   /* through the HCI Driver layer to the transport untouched.  It is   */
   /* the responsibility of the HCI Transport driver writer to define   */
   /* the contents of this member (or completely ignore it).            */
   /* * NOTE * This function does not close the HCI Transport specified */
   /*          by HCI Transport ID, it merely reconfigures the          */
   /*          transport.  This means that the HCI Transport specified  */
   /*          by HCI Transport ID is still valid until it is closed    */
   /*          via the HCI_COMClose() function.                         */
void BTPSAPI HCITR_COMReconfigure(unsigned int HCITransportID, HCI_Driver_Reconfigure_Data_t *DriverReconfigureData)
{
   unsigned long                 BaudRate;
   HCI_Driver_Reconfigure_Data_t DisableRxTxData;

   /* Check to make sure that the specified Transport ID is valid.      */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen) && (DriverReconfigureData) && (DriverReconfigureData->ReconfigureCommand == HCI_COMM_DRIVER_RECONFIGURE_DATA_COMMAND_CHANGE_PARAMETERS))
   {
      /* Disable Transmit and Receive while we change the baud rate.    */
      DisableRxTxData.ReconfigureCommand = HCI_COMM_DRIVER_DISABLE_UART_TX_RX;
      DisableRxTxData.ReconfigureData    = NULL;
      HCITR_COMReconfigure(0, &DisableRxTxData);

      /* Configure the requested baud rate.                             */
      BaudRate = *((unsigned long *)DriverReconfigureData->ReconfigureData);

      UARTIntDisableReceive(UartContext.UartBase);
      HAL_CommConfigure(UartContext.UartBase, BaudRate, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
      UARTIntEnableReceive(UartContext.UartBase);

      /* Small delay to let the COM Port reconfigure itself.            */
      BTPS_Delay(1);

      /* Re-enable Transmit and Receive operation.                      */
      DisableRxTxData.ReconfigureCommand = HCI_COMM_DRIVER_DISABLE_UART_TX_RX;
      DisableRxTxData.ReconfigureData    = (void *)DWORD_SIZE;
      HCITR_COMReconfigure(0, &DisableRxTxData);
   }

   /* Check to see if there is a global reconfigure parameter.          */
   if((DriverReconfigureData) && (!HCITransportID) && (HCITransportOpen))
   {
      /* Check to see if we are being instructed to disable RX/TX.      */
      if(DriverReconfigureData->ReconfigureCommand == HCI_COMM_DRIVER_DISABLE_UART_TX_RX)
      {
         if(DriverReconfigureData->ReconfigureData)
         {
            /* Flow on.                                                 */
            DISABLE_INTERRUPTS();

            /* Clear the RTS High Flag.                                 */
            UartContext.Flags &= ~UART_CONTEXT_FLAG_RTS_HIGH;

            /* Re-enable the Transmitter.                               */
            EnableTransmitter();

            ENABLE_INTERRUPTS();

            /* Flow the Bluetooth chip on if necessary.                 */
            FLOW_ON();
         }
         else
         {
            /* We are being asked to flow off all Bluetooth UART        */
            /* Transactions.                                            */
            DISABLE_INTERRUPTS();

            /* Disable the Receiver.                                    */
            FLOW_OFF();

            /* Flag that the RTS Line should stay high.                 */
            UartContext.Flags |= UART_CONTEXT_FLAG_RTS_HIGH;

            /* Disable the Transmit Operation.                          */
            DisableTransmitter();

            ENABLE_INTERRUPTS();

            /* Wait until we have finished transmitting any bytes in the*/
            /* UART Transmit Buffer.                                    */
            while(UART_TRANSMIT_ACTIVE())
               ;
         }
      }
   }
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to force the processing of incoming COM Data.             */
   /* * NOTE * This function is only applicable in device stacks that   */
   /*          are non-threaded.  This function has no effect for device*/
   /*          stacks that are operating in threaded environments.      */
void BTPSAPI HCITR_COMProcess(unsigned int HCITransportID)
{
   /* Check to make sure that the specified Transport ID is valid.      */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen))
      RxProcess();
}

   /* The following function is responsible for actually sending data   */
   /* through the opened HCI Transport layer (specified by the first    */
   /* parameter).  Bluetopia uses this function to send formatted HCI   */
   /* packets to the attached Bluetooth Device.  The second parameter to*/
   /* this function specifies the number of bytes pointed to by the     */
   /* third parameter that are to be sent to the Bluetooth Device.  This*/
   /* function returns a zero if the all data was transfered sucessfully*/
   /* or a negetive value if an error occurred.  This function MUST NOT */
   /* return until all of the data is sent (or an error condition       */
   /* occurs).  Bluetopia WILL NOT attempt to call this function        */
   /* repeatedly if data fails to be delivered.  This function will     */
   /* block until it has either buffered the specified data or sent all */
   /* of the specified data to the Bluetooth Device.                    */
   /* * NOTE * The type of data (Command, ACL, SCO, etc.) is NOT passed */
   /*          to this function because it is assumed that this         */
   /*          information is contained in the Data Stream being passed */
   /*          to this function.                                        */
int BTPSAPI HCITR_COMWrite(unsigned int HCITransportID, unsigned int Length, unsigned char *Buffer)
{
   int ret_val = 0;

   /* Check to make sure that the specified Transport ID is valid and   */
   /* the output buffer appears to be valid as well.                    */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen) && (Length) && (Buffer))
   {
      /* Load the Transmit Buffer with the selected characters.         */
      LoadTransmitBuffer(Length, Buffer);

      /* Return success to the caller.                                  */
      ret_val = 0;
   }

   return(ret_val);
}

   /* The following function is responsible for suspending the HCI COM  */
   /* transport.  It will block until the transmit buffers are empty and*/
   /* all data has been sent then put the transport in a suspended      */
   /* state.  This function will return a value of 0 if the suspend was */
   /* successful or a negative value if there is an error.              */
   /* * NOTE * An error will occur if the suspend operation was         */
   /*          interrupted by another event, such as data being received*/
   /*          before the transmit buffer was empty.                    */
   /* * NOTE * The calling function must lock the Bluetooth stack with a*/
   /*          call to BSC_LockBluetoothStack() before this function is */
   /*          called.                                                  */
   /* * NOTE * This function should only be called when the baseband    */
   /*          low-power protocol in use has indicated that it is safe  */
   /*          to sleep.  Also, this function must be called            */
   /*          successfully before any clocks necessary for the         */
   /*          transport to operate are disabled.                       */
int BTPSAPI HCITR_COMSuspend(unsigned int HCITransportID)
{
   int ret_val;

   /* Verify that the Transport ID is somewhat valid.                   */
   if((!HCITransportID) || (HCITransportID == TRANSPORT_ID))
   {
      /* Verify that the UART is not currently suspended.               */
      if(UartContext.SuspendState == hssNormal)
      {
         /* Disable interrupts while we change state.                   */
         DISABLE_INTERRUPTS();

         /* Signal that we are waiting for a suspend operation to       */
         /* complete.                                                   */
         UartContext.SuspendState = hssSuspendWait;

         /* Flag that the RTS is asserted due to the fact that the UART */
         /* is suspended.                                               */
         UartContext.Flags |= UART_CONTEXT_FLAG_UART_SUSPENDED;
         FLOW_OFF();

         /* Re-enable interrupts.                                       */
         ENABLE_INTERRUPTS();

         /* Add a small delay to allow controller to process the RTS    */
         /* change.                                                     */
         __delay_cycles(2500);

         /* Wait for the UART transmit buffer and FIFO to be empty.     */
         while(((UartContext.TxBytesFree != UartContext.TxBufferSize) || (UART_TRANSMIT_ACTIVE())) && (UartContext.SuspendState == hssSuspendWait))
            ;

         /* Confirm that no data was received in this time and suspend  */
         /* the UART.                                                   */
         DISABLE_INTERRUPTS();

         if(UartContext.SuspendState == hssSuspendWait)
         {
            /* Flag that the UART is current suspended.                 */
            UartContext.SuspendState = hssSuspended;

#ifdef __DISABLE_SMCLK__

            /* Disable the SMCLK request bit.                           */
            HAL_DisableSMCLK(HAL_PERIPHERAL_BLUETOOTH_UART);

#endif

            /* Re-enable interrupts.                                    */
            ENABLE_INTERRUPTS();

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Re-set the UART state to Normal.                         */
            UartContext.SuspendState  = hssNormal;
            UartContext.Flags        &= ~UART_CONTEXT_FLAG_UART_SUSPENDED;
            FLOW_ON();

            /* Re-enable interrupts.                                    */
            ENABLE_INTERRUPTS();

            /* Data was received, abort suspending the UART.            */
            ret_val                  = HCITR_ERROR_SUSPEND_ABORTED;
         }
      }
      else
      {
         /* Check to see what the state currently is.                   */
         if(UartContext.SuspendState == hssSuspended)
         {
            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Data was received, abort suspending the UART.            */
            ret_val                  = HCITR_ERROR_SUSPEND_ABORTED;
         }
      }
   }
   else
      ret_val = HCITR_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is used to determine if the UART is        */
   /* currently suspended.  This function returns TRUE if the UART is   */
   /* suspended or FALSE otherwise.                                     */
Boolean_t BTPSAPI HCITR_UartSuspended(unsigned int HCITransportID)
{
   return((Boolean_t)(UartContext.SuspendState == hssSuspended));
}
