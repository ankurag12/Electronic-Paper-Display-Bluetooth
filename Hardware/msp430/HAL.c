/*****< HARDWARE.c >**********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                               */
/*      All Rights Reserved.                                                 */
/*                                                                           */
/*  HARDWARE - Hardware API for MSP430 Experimentor Board                    */
/*                                                                           */
/*  Author:  Tim Cook                                                        */
/*                                                                           */
/*** MODIFICATION HISTORY ****************************************************/
/*                                                                           */
/*   mm/dd/yy  F. Lastname    Description of Modification                    */
/*   --------  -----------    -----------------------------------------------*/
/*   07/07/10  Tim Cook       Initial creation.                              */
/*****************************************************************************/
#include "HAL.h"                 /* MSP430 Hardware Abstraction API.         */
#include "HRDWCFG.h"             /* SS1 MSP430 Hardware Configuration Header.*/
#include <msp430.h>
#include <string.h>
#include "BTPSKRNL.h"
#include <stdio.h>
#include "msp430-uart.h"

#define BTPS_MSP430_DEFAULT_BAUD           115200L  /* Default UART Baud Rate*/
                                                    /* used in baud rate     */
                                                    /* given to this module  */
                                                    /* is invalid.           */

   /* The following are some defines that we will define to be 0 if they*/
   /* are not define in the device header.                              */

#ifndef XT1LFOFFG

   #define XT1LFOFFG   0

#endif

#ifndef XT1HFOFFG

   #define XT1HFOFFG   0

#endif

#ifndef XT2OFFG

   #define XT2OFFG     0

#endif

   /* Auxilary clock frequency                                          */
#define ACLK_FREQUENCY_HZ  ((unsigned int)32768)

   /* Macro to stop the OS Scheduler.                                   */
#define STOP_SCHEDULER()   (TA1CTL &= (~(MC_3)))

   /* Instruction to start the Scheduler Tick ISR.                      */
#define START_SCHEDULER()  (TA1CTL |= MC_1)

   /* The following structure represents the data that is stored to     */
   /* allow us to table drive the CPU setup for each of the Clock       */
   /* Frequencies that we allow.                                        */
typedef struct _tagFrequency_Settings_t
{
   unsigned char VCORE_Level;
   unsigned int  DCO_Multiplier;
} Frequency_Settings_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

                              /* The following variable is used to hold */
                              /* a system tick count for the Bluetopia  */
                              /* No-OS stack.                           */
static volatile unsigned long MSP430Ticks;

                              /* The following function is provided to  */
                              /* keep track of the number of peripherals*/
                              /* that have requested that the SMCLK stay*/
                              /* active. When this decrements to ZERO,  */
                              /* the clock will be turned off.          */
static volatile unsigned char ClockRequestedPeripherals;

                              /* The following is used to buffer        */
                              /* characters read from the Debug UART.   */
static unsigned char RecvBuffer[BT_DEBUG_UART_RX_BUFFER_SIZE];

                              /* The following are used to track the    */
                              /* Receive circular buffer.               */
static unsigned int  RxInIndex;
static unsigned int  RxOutIndex;
static unsigned int  RxBytesFree = BT_DEBUG_UART_RX_BUFFER_SIZE;

   /* If no buffer is specified, the this will result in a Blocking     */
   /* Write.                                                            */
#if BT_DEBUG_UART_TX_BUFFER_SIZE > 0

                              /* The following is used to buffer        */
                              /* characters sent to the Debug UART.     */
static unsigned char TransBuffer[BT_DEBUG_UART_TX_BUFFER_SIZE];

                              /* The following are used to track the    */
                              /* Transmir circular buffer.              */
static unsigned int TxInIndex;
static unsigned int TxOutIndex;
static unsigned int TxBytesFree = BT_DEBUG_UART_TX_BUFFER_SIZE;

#endif

   /* The following represents the table that we use to table drive the */
   /* CPU Frequency setup.                                              */
static BTPSCONST Frequency_Settings_t Frequency_Settings[] =
{
   {PMMCOREV_0, 244},  /* cf8MHZ_t.                          */
   {PMMCOREV_1, 488},  /* cf16MHZ_t.                         */
   {PMMCOREV_2, 610},  /* cf20MHZ_t.                         */
   {PMMCOREV_3, 675}   /* cf22.1184MHZ_t.                    */
};

   /* External functions called by this module.  These are neccessary   */
   /* for UART operation and reside in HCITRANS.c                       */

   /* Called upon reception of a CTS Interrupt. Must toggle Interrupt   */
   /* Edge Polarity and flag Tx Flow Enabled State.                     */
extern int CtsInterrupt(void);

   /* Local Function Prototypes.                                        */
static Boolean_t DetermineProcessorType(void);
//static void ConfigureBoardDefaults(void);
static void ConfigureLEDs(void);
static void ToggleLED(int LEDID);
static void SetLED(int LED_ID, int State);
static void ConfigureTimer(void);
static unsigned char IncrementVCORE(unsigned char Level);
static unsigned char DecrementVCORE(unsigned char Level);
static void ConfigureVCore(unsigned char Level);
static void StartCrystalOscillator(void);
static void SetSystemClock(Cpu_Frequency_t CPU_Frequency);

   /* The following function is responsible for determining if we are   */
   /* running on the MSP430F5438 or the MSP430F5438A processor.  This   */
   /* function returns TRUE if we are on the MSP430F5438A or FALSE      */
   /* otherwise.                                                        */
static Boolean_t DetermineProcessorType(void)
{
   Boolean_t ret_val = FALSE;

  /* Read the TLV descriptors to determine the device type.             */
  if ((*((char *)0x1A04) == 0x05) && (*((char *)0x1A05) == 0x80))
     ret_val = TRUE;

  return(ret_val);
}

   /* The following function is used to configure all unused pins to    */
   /* their board default values.                                       */
/*
static void ConfigureBoardDefaults(void)
{
    Tie unused pins as inputs.
   PAOUT  = 0;
   PADIR  = 0xFFFF;
   PASEL  = 0;
   PBOUT  = 0;
   PBDIR  = 0xFFFF;
   PBSEL  = 0;
   PCOUT  = 0;
   PCDIR  = 0xFFFF;
   PCSEL  = 0;
   PDOUT  = 0;
   PDDIR  = 0xFFFF;
   PDSEL  = 0;

    P10.0 is tied to USB RST pin.
   PEOUT  = 0;
   PEDIR  = 0xFEFF;

   PESEL  = 0;
   P11OUT = 0;
   P11DIR = 0xFF;
   PJOUT  = 0;
   PJDIR  = 0xFF;
   P11SEL = 0;

    Shutdown the Microphone Amplifier (P6.4 = 0) and shutdown the
    Audio Out Amplifier (P6.6 = 1 to raise SHTDWN).
   P6DIR  = (BIT6 | BIT4);
   P6OUT  = BIT6;

    The MSP430 Experimenter board needs to configure the "bridge" pins
    on the RF3 jumper block to be inputs (with pull-up/pull-down
    resistors disabled) as these will be driven by other pins.

    PORT1, bits 2,5,6 are jumpered on RF3 to other pins, the other
    pins control them, so these should be input, with pullup/pulldown
    resistors disabled and the corresponding interrupt sources for
    PORT1 interrupts should also be disabled, since these are
    controlled by other pins care should be taken to mask them out
    when reading the status of PORT1.  The same is true for PORT8 PIN1
   P1REN &= ~(BIT2 | BIT5 | BIT6);
   P1DIR &= ~(BIT2 | BIT5 | BIT6);
   P1SEL &= ~(BIT2 | BIT5 | BIT6);
   P8REN &= ~(BIT1);
   P8DIR &= ~(BIT1);
   P8SEL &= ~(BIT1);
   P1IE  &= ~(BIT2 | BIT5 | BIT6);
}
*/


   /* The following function is used to configure the board LEDs.       */
static void ConfigureLEDs(void)
{
   P8SEL &= ~(BIT0 | BIT1);
   P8DIR |= (BIT0 | BIT1);
}

   /* The following function is a utility function used to toggle an    */
   /* LED.                                                              */
static void ToggleLED(int LEDID)
{
   if(P1OUT & BIT1)
   {
      P1OUT &= ~BIT1;
      P1OUT |= BIT0;
   }
   else
   {
      P1OUT |= BIT1;
      P1OUT &= ~BIT0;
   }
}

   /* The following function is a utility function that is used to set  */
   /* the state of a specified LED.                                     */
static void SetLED(int LED_ID, int State)
{
   Byte_t Mask;

  if(LED_ID)
     Mask = BIT1;
  else
     Mask = BIT0;

  if(State)
     P1OUT |= Mask;
  else
     P1OUT &= ~Mask;
}

   /* This function is called to configure the System Timer, i.e TA1.   */
   /* This timer is used for all system time scheduling.                */
static void ConfigureTimer(void)
{
   /* Ensure the timer is stopped.                                      */
   TA1CTL = 0;

   /* Run the timer off of the ACLK.                                    */
   TA1CTL = TASSEL_1 | ID_0;

   /* Clear everything to start with.                                   */
   TA1CTL |= TACLR;

   /* Set the compare match value according to the tick rate we want.   */
   TA1CCR0 = ( ACLK_FREQUENCY_HZ / MSP430_TICK_RATE_HZ ) + 1;

   /* Enable the interrupts.                                            */
   TA1CCTL0 = CCIE;

   /* Start up clean.                                                   */
   TA1CTL |= TACLR;

   /* Up mode.                                                          */
   TA1CTL |= TASSEL_1 | MC_1 | ID_0;
}

   /* The following function is a utility function the is used to       */
   /* increment the VCore setting to the specified value.               */
static unsigned char IncrementVCORE(unsigned char Level)
{
   unsigned char Result;
   unsigned char PMMRIE_backup;
   unsigned char SVSMHCTL_backup;
   unsigned char SVSMLCTL_backup;

   /* The code flow for increasing the Vcore has been altered to work   */
   /* around the erratum FLASH37.  Please refer to the Errata sheet to  */
   /* know if a specific device is affected DO NOT ALTER THIS FUNCTION  */

   /* Open PMM registers for write access.                              */
   PMMCTL0_H     = 0xA5;

   /* Disable dedicated Interrupts and backup all registers.            */
   PMMRIE_backup    = PMMRIE;
   PMMRIE          &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE | SVSLPE | SVMHVLRIE | SVMHIE | SVSMHDLYIE | SVMLVLRIE | SVMLIE | SVSMLDLYIE );
   SVSMHCTL_backup  = SVSMHCTL;
   SVSMLCTL_backup  = SVSMLCTL;

   /* Clear flags.                                                      */
   PMMIFG           = 0;

   /* Set SVM highside to new level and check if a VCore increase is    */
   /* possible.                                                         */
   SVSMHCTL = SVMHE | SVSHE | (SVSMHRRL0 * Level);

   /* Wait until SVM highside is settled.                               */
   while ((PMMIFG & SVSMHDLYIFG) == 0);

   /* Clear flag.                                                       */
   PMMIFG &= ~SVSMHDLYIFG;

   /* Check if a VCore increase is possible.                            */
   if((PMMIFG & SVMHIFG) == SVMHIFG)
   {
      /* Vcc is too low for a Vcore increase so we will recover the     */
      /* previous settings                                              */
      PMMIFG &= ~SVSMHDLYIFG;
      SVSMHCTL = SVSMHCTL_backup;

      /* Wait until SVM highside is settled.                            */
      while ((PMMIFG & SVSMHDLYIFG) == 0)
         ;

      /* Return that the value was not set.                             */
      Result = 1;
   }
   else
   {
      /* Set also SVS highside to new level Vcc is high enough for a    */
      /* Vcore increase                                                 */
      SVSMHCTL |= (SVSHRVL0 * Level);

      /* Wait until SVM highside is settled.                            */
      while ((PMMIFG & SVSMHDLYIFG) == 0)
         ;

      /* Clear flags.                                                   */
      PMMIFG &= ~SVSMHDLYIFG;

      /* Set VCore to new level.                                        */
      PMMCTL0_L = PMMCOREV0 * Level;

      /* Set SVM, SVS low side to new level.                            */
      SVSMLCTL = SVMLE | (SVSMLRRL0 * Level) | SVSLE | (SVSLRVL0 * Level);

      /* Wait until SVM, SVS low side is settled.                       */
      while ((PMMIFG & SVSMLDLYIFG) == 0)
         ;

      /* Clear flag.                                                    */
      PMMIFG &= ~SVSMLDLYIFG;

      /* SVS, SVM core and high side are now set to protect for the new */
      /* core level.  Restore Low side settings Clear all other bits    */
      /* _except_ level settings                                        */
      SVSMLCTL &= (SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

      /* Clear level settings in the backup register,keep all other     */
      /* bits.                                                          */
      SVSMLCTL_backup &= ~(SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

      /* Restore low-side SVS monitor settings.                         */
      SVSMLCTL |= SVSMLCTL_backup;

      /* Restore High side settings.  Clear all other bits except level */
      /* settings                                                       */
      SVSMHCTL &= (SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

      /* Clear level settings in the backup register,keep all other     */
      /* bits.                                                          */
      SVSMHCTL_backup &= ~(SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

      /* Restore backup.                                                */
      SVSMHCTL |= SVSMHCTL_backup;

      /* Wait until high side, low side settled.                        */
      while(((PMMIFG & SVSMLDLYIFG) == 0) && ((PMMIFG & SVSMHDLYIFG) == 0))
         ;

      /* Return that the value was set.                                 */
      Result = 0;
   }

   /* Clear all Flags.                                                  */
   PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG | SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG);

   /* Restore PMM interrupt enable register.                            */
   PMMRIE = PMMRIE_backup;

   /* Lock PMM registers for write access.                              */
   PMMCTL0_H = 0x00;

   return(Result);
}

   /* The following function is a utility function the is used to       */
   /* decrement the VCore setting to the specified value.               */
static unsigned char DecrementVCORE(unsigned char Level)
{
   unsigned char Result;
   unsigned char PMMRIE_backup;
   unsigned char SVSMHCTL_backup;
   unsigned char SVSMLCTL_backup;

   /* The code flow for decreasing the Vcore has been altered to work   */
   /* around the erratum FLASH37.  Please refer to the Errata sheet to  */
   /* know if a specific device is affected DO NOT ALTER THIS FUNCTION  */

   /* Open PMM registers for write access.                              */
   PMMCTL0_H        = 0xA5;

   /* Disable dedicated Interrupts Backup all registers                 */
   PMMRIE_backup    = PMMRIE;
   PMMRIE          &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE | SVSLPE | SVMHVLRIE | SVMHIE | SVSMHDLYIE | SVMLVLRIE | SVMLIE | SVSMLDLYIE );
   SVSMHCTL_backup  = SVSMHCTL;
   SVSMLCTL_backup  = SVSMLCTL;

   /* Clear flags.                                                      */
   PMMIFG &= ~(SVMHIFG | SVSMHDLYIFG | SVMLIFG | SVSMLDLYIFG);

   /* Set SVM, SVS high & low side to new settings in normal mode.      */
   SVSMHCTL = SVMHE | (SVSMHRRL0 * Level) | SVSHE | (SVSHRVL0 * Level);
   SVSMLCTL = SVMLE | (SVSMLRRL0 * Level) | SVSLE | (SVSLRVL0 * Level);

   /* Wait until SVM high side and SVM low side is settled.             */
   while (((PMMIFG & SVSMHDLYIFG) == 0) || ((PMMIFG & SVSMLDLYIFG) == 0))
      ;

   /* Clear flags.                                                      */
   PMMIFG &= ~(SVSMHDLYIFG + SVSMLDLYIFG);

   /* SVS, SVM core and high side are now set to protect for the new    */
   /* core level.                                                       */

   /* Set VCore to new level.                                           */
   PMMCTL0_L = PMMCOREV0 * Level;

   /* Restore Low side settings Clear all other bits _except_ level     */
   /* settings                                                          */
   SVSMLCTL &= (SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

   /* Clear level settings in the backup register,keep all other bits.  */
   SVSMLCTL_backup &= ~(SVSLRVL0+SVSLRVL1+SVSMLRRL0+SVSMLRRL1+SVSMLRRL2);

   /* Restore low-side SVS monitor settings.                            */
   SVSMLCTL |= SVSMLCTL_backup;

   /* Restore High side settings Clear all other bits except level      */
   /* settings                                                          */
   SVSMHCTL &= (SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

   /* Clear level settings in the backup register, keep all other bits. */
   SVSMHCTL_backup &= ~(SVSHRVL0+SVSHRVL1+SVSMHRRL0+SVSMHRRL1+SVSMHRRL2);

   /* Restore backup.                                                   */
   SVSMHCTL |= SVSMHCTL_backup;

   /* Wait until high side, low side settled.                           */
   while (((PMMIFG & SVSMLDLYIFG) == 0) && ((PMMIFG & SVSMHDLYIFG) == 0))
      ;

   /* Clear all Flags.                                                  */
   PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG | SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG);

   /* Restore PMM interrupt enable register.                            */
   PMMRIE = PMMRIE_backup;

   /* Lock PMM registers for write access.                              */
   PMMCTL0_H = 0x00;

   /* Return success to the caller.                                     */
   Result    = 0;

   return(Result);
}

   /* The following function is responsible for setting the PMM core    */
   /* voltage to the specified level.                                   */
static void ConfigureVCore(unsigned char Level)
{
   unsigned int ActualLevel;
   unsigned int Status;

   /* Set Mask for Max.  level.                                         */
   Level       &= PMMCOREV_3;

   /* Get actual VCore.                                                 */
   ActualLevel  = (PMMCTL0 & PMMCOREV_3);

   /* Step by step increase or decrease the VCore setting.              */
   Status = 0;
   while (((Level != ActualLevel) && (Status == 0)) || (Level < ActualLevel))
   {
     if (Level > ActualLevel)
       Status = IncrementVCORE(++ActualLevel);
     else
       Status = DecrementVCORE(--ActualLevel);
   }
}

   /* The following function is responsible for starting XT1 in the     */
   /* MSP430 that is used to source the internal FLL that drives the    */
   /* MCLK and SMCLK.                                                   */
static void StartCrystalOscillator(void)
{
   /* Set up XT1 Pins to analog function, and to lowest drive           */
   P7SEL   |= (BIT1 | BIT0);

   /* Set internal cap values.  (Hardware cap present)                                        */
   // UCSCTL6 |= XCAP_3;

   /* Loop while the Oscillator Fault bit is set.                       */
   while(SFRIFG1 & OFIFG)
   {
     while (SFRIFG1 & OFIFG)
     {
        /* Clear OSC fault flags.                                       */
       UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT1HFOFFG + XT2OFFG);
       SFRIFG1 &= ~OFIFG;
     }

     /* Reduce the drive strength.
      * NOT SURE ABOUT THIS. THE USER GUIDE SAYS OPERATING RANGE FOR THIS IS 4-8 MHz
      * I think it should be UCSCTL6 &= ~(XT1DRIVE0);                                    */
     UCSCTL6 &= ~(XT1DRIVE1_L + XT1DRIVE0);
   }
}

   /* The following function is responsible for setting up the system   */
   /* clock at a specified frequency.                                   */
static void SetSystemClock(Cpu_Frequency_t CPU_Frequency)
{
   Boolean_t                       UseDCO;
   unsigned int                    Ratio;
   unsigned int                    DCODivBits;
   unsigned long                   SystemFrequency;
   volatile unsigned int           Counter;
   BTPSCONST Frequency_Settings_t *CPU_Settings;

   /* Verify that the CPU Frequency enumerated type is valid, if it is  */
   /* not then we will force it to a default.                           */
   if((CPU_Frequency != cf8MHZ_t) && (CPU_Frequency != cf16MHZ_t) && (CPU_Frequency != cf20MHZ_t) && (CPU_Frequency != cf25MHZ_t))
      CPU_Frequency = cf16MHZ_t;

   /* Do not allow improper settings (MSP430F5438 cannot run at 20MHz or*/
   /* 25 MHz).                                                          */
   if((!DetermineProcessorType()) && ((CPU_Frequency == cf20MHZ_t) || (CPU_Frequency == cf25MHZ_t)))
      CPU_Frequency = cf16MHZ_t;

   /* Get the CPU settings for the specified frequency.                 */
   CPU_Settings = &Frequency_Settings[CPU_Frequency - cf8MHZ_t];

   /* Configure the PMM core voltage.                                   */
   ConfigureVCore(CPU_Settings->VCORE_Level);

   /* Get the ratio of the system frequency to the source clock.        */
   Ratio           = CPU_Settings->DCO_Multiplier;

   /* Use a divider of at least 2 in the FLL control loop.              */
   DCODivBits      = FLLD__2;

   /* Get the system frequency that is configured.                      */
   SystemFrequency  = HAL_GetSystemSpeed();
   SystemFrequency /= 1000;

   /* If the requested frequency is above 16MHz we will use DCO as the  */
   /* source of the system clocks, otherwise we will use DCOCLKDIV.     */
   if(SystemFrequency > 16000)
   {
       Ratio  >>= 1;
       UseDCO   = TRUE;
   }
   else
   {
       SystemFrequency <<= 1;
       UseDCO            = FALSE;
   }

   /* While needed set next higher div level.                           */
   while (Ratio > 512)
   {
       DCODivBits   = DCODivBits + FLLD0;
       Ratio      >>= 1;
   }

   /* Disable the FLL.                                                  */
   __bis_SR_register(SCG0);

   /* Set DCO to lowest Tap.                                            */
   UCSCTL0 = 0x0000;

   /* Reset FN bits.                                                    */
   UCSCTL2 &= ~(0x03FF);
   UCSCTL2  = (DCODivBits | (Ratio - 1));

   /* Set the DCO Range.                                                */
   if(SystemFrequency <= 630)
   {
      /* Fsystem < 630KHz.                                              */
      UCSCTL1 = DCORSEL_0;
   }
   else if(SystemFrequency <  1250)
   {
      /* 0.63MHz < fsystem < 1.25MHz.                                   */
      UCSCTL1 = DCORSEL_1;
   }
   else if(SystemFrequency <  2500)
   {
      /* 1.25MHz < fsystem < 2.5MHz.                                    */
      UCSCTL1 = DCORSEL_2;
   }
   else if(SystemFrequency <  5000)
   {
      /* 2.5MHz < fsystem < 5MHz.                                       */
      UCSCTL1 = DCORSEL_3;
   }
   else if(SystemFrequency <  10000)
   {
      /* 5MHz < fsystem < 10MHz.                                        */
      UCSCTL1 = DCORSEL_4;
   }
   else if(SystemFrequency <  20000)
   {
      /* 10MHz < fsystem < 20MHz.                                       */
      UCSCTL1 = DCORSEL_5;
   }
   else if(SystemFrequency <  40000)
   {
      /* 20MHz < fsystem < 40MHz.                                       */
      UCSCTL1 = DCORSEL_6;
   }
   else
      UCSCTL1 = DCORSEL_7;

   /* Re-enable the FLL.                                                */
   __bic_SR_register(SCG0);

   /* Loop until the DCO is stabilized.                                 */
   while(UCSCTL7 & DCOFFG)
   {
       /* Clear DCO Fault Flag.                                         */
       UCSCTL7 &= ~DCOFFG;

       /* Clear OFIFG fault flag.                                       */
       SFRIFG1 &= ~OFIFG;
   }

   /* Enable the FLL control loop.                                      */
   __bic_SR_register(SCG0);

   /* Based on the frequency we will use either DCO or DCOCLKDIV as the */
   /* source of MCLK and SMCLK.                                         */
   if (UseDCO)
   {
      /* Select DCOCLK for MCLK and SMCLK.                              */
      UCSCTL4 &=  ~(SELM_7 | SELS_7);
      UCSCTL4 |= (SELM__DCOCLK | SELS__DCOCLK);
   }
   else
   {
      /* Select DCOCLKDIV for MCLK and SMCLK.                           */
       UCSCTL4 &=  ~(SELM_7 | SELS_7);
       UCSCTL4 |= (SELM__DCOCLKDIV | SELS__DCOCLKDIV);
   }

   /* Delay the appropriate amount of cycles for the clock to settle.   */
   Counter = Ratio * 32;
   while (Counter--)
       __delay_cycles(30);
}

   /* The following function is provided to allow a mechanism of        */
   /* configuring the MSP430 pins to their default state for the sample.*/
void HAL_ConfigureHardware(void)
{
   /* Configure the default board setup.                                */
   // ConfigureBoardDefaults();

   /* Configure the LEDs for outputs                                    */
   ConfigureLEDs();

   /* Call the MSP430F5438 Experimentor Board Hardware Abstraction Layer*/
   /* to setup the system clock.                                        */
   StartCrystalOscillator();
   SetSystemClock(BT_CPU_FREQ);

   /* Configure the UART-USB Port for its default configuration         */
   HAL_CommConfigure(BT_DEBUG_UART_BASE, BT_DEBUG_UART_BAUDRATE, 0);
   GPIOPinTypeUART(BT_DEBUG_UART_PIN_BASE, BT_DEBUG_UART_PIN_TX_MASK, BT_DEBUG_UART_PIN_RX_MASK);

   /* Enable Debug UART Receive Interrupt.                              */
   UARTIntEnableReceive(BT_DEBUG_UART_BASE);

   /* Configure the scheduler timer.                                    */
   ConfigureTimer();


}

   /* * NOTE * The following are the allowed flags for the flags        */
   /*          argument.                                                */
   /*  1.  UART_CONFIG_WLEN_8, UART_CONFIG_WLEN_7                       */
   /*  2.  UART_CONFIG_PAR_NONE,UART_CONFIG_PAR_ODD,UART_CONFIG_PAR_EVEN*/
   /*  3.  UART_CONFIG_STOP_ONE,UART_CONFIG_STOP_TWO                    */
   /*          The flags is a bitfield which may include one flag from  */
   /*          each of the three rows above                             */
void HAL_CommConfigure(unsigned char *UartBase, unsigned long BaudRate, unsigned char Flags)
{
   unsigned long Frequency;
   unsigned int  Divider;

   /* Since we allow access to register clear any invalid flags.        */
   Flags &= (UART_CONFIG_PAR_EVEN | UART_CONFIG_WLEN_7 | UART_CONFIG_STOP_TWO);

   /* set UCSWRST bit to hold UART module in reset while we configure   */
   /* it.                                                               */
   HWREG8(UartBase + MSP430_UART_CTL1_OFFSET) = MSP430_UART_CTL1_SWRST;

   /* Configure control register 0 by clearing and then setting the     */
   /* allowed user options we also ensure that UCSYNC = Asynchronous    */
   /* Mode, UCMODE = UART, UCMSB = LSB first and also ensure that the   */
   /* default 8N1 configuration is used if the flags argument is 0.     */
   HWREG8(UartBase + MSP430_UART_CTL0_OFFSET) = Flags;

   /* UART peripheral erroneous characters cause interrupts break       */
   /* characters cause interrupts on reception                          */
   HWREG8(UartBase + MSP430_UART_CTL1_OFFSET) |= (MSP430_UART_CTL1_RXIE | MSP430_UART_CTL1_BRKIE);

   /* clear UCA status register                                         */
   HWREG8(UartBase + MSP430_UART_STAT_OFFSET)  = 0x00;

   /* clear interrupt flags                                             */
   HWREG8(UartBase + MSP430_UART_IFG_OFFSET)  &= ~(MSP430_UART_TXIFG_mask | MSP430_UART_RXIFG_mask);

   /* If the baud rate is not valid, use the default.                   */
   if(!BaudRate)
      BaudRate = BTPS_MSP430_DEFAULT_BAUD;

   /* Use ACLK for Baud rates less than 9600 to allow us to still       */
   /* receive characters while in LPM3.                                 */
   if(BaudRate <= 9600)
   {
      HWREG8(UartBase + MSP430_UART_CTL1_OFFSET) |= MSP430_UART_CTL1_UCSSEL_ACLK_mask;
      Frequency = ACLK_FREQUENCY_HZ;
   }
   else
   {
      HWREG8(UartBase + MSP430_UART_CTL1_OFFSET) |= MSP430_UART_CTL1_UCSSEL_SMCLK_mask;
      Frequency = HAL_GetSystemSpeed();
   }

   /* Calculate the initial baud rate divisor. This value can be used   */
   /* for correct rounding for either the low frequency or oversampling */
   /* calculations.                                                     */
   Divider = (unsigned int)((Frequency * 16) / BaudRate);

   /* If the divider will be less than 32, use the low frequency        */
   /* calculation, otherwise use oversampling. Note that the current    */
   /* divider value is 16 times the actual divider.                     */
   if(Divider < (32 * 16))
   {
      /* For low frequency baud rates, first calculate the divider * 8, */
      /* rounded to the nearest integer. The lower 3 bits of the result */
      /* will be the fraction portion of the divider and the rest will  */
      /* be the integer portion of the divider.                         */
      Divider     = (Divider + 1) / 2;
      HWREG16(UartBase + MSP430_UART_BRW_OFFSET) = Divider / 8;
      HWREG8(UartBase + MSP430_UART_MCTL_OFFSET) = (Divider & 0x07) << MSP430_UART_MCTL_BRS_bit;
   }
   else
   {
      /* For oversamping mode, first calculate the divider * 16, rounded*/
      /* to the nearest integer. The lower 4 bits of the result will be */
      /* the fraction portion of the divider and the rest will be the   */
      /* integer portion of the divider.                                */
      Divider   = (Divider + 8) / 16;
      HWREG16(UartBase + MSP430_UART_BRW_OFFSET) = Divider / 16;
      HWREG8(UartBase + MSP430_UART_MCTL_OFFSET) = ((Divider & 0x0F) << MSP430_UART_MCTL_BRF_bit) | MSP430_UART_MCTL_UCOS16_mask;
   }

   /* now clear the UCA2 Software Reset bit                             */
   HWREG8(UartBase + MSP430_UART_CTL1_OFFSET) &= (~(MSP430_UART_CTL1_SWRST));

   // Added from Plastic Logic code to use "LOG" function
   msp430_uart_register_files();
}




   /* Called to read from a UART port and returns the number of bytes   */
   /* read.                                                             */
int HAL_ConsoleRead(unsigned int Length, char *Buffer)
{
   int Processed = 0;
   int CopyLength;
   int MaxRead;
   int Count;

   /* Make sure the passed in parameters seem valid.                    */
   if((Buffer) && (Length))
   {
      /* Read the characters if neccessary.                             */
      Processed = 0;
      while((Length) && (RxBytesFree != BT_DEBUG_UART_RX_BUFFER_SIZE))
      {
         /* Determine the number of characters until the buffer wraps.  */
         Count      = BT_DEBUG_UART_RX_BUFFER_SIZE - RxBytesFree;
         MaxRead    = BT_DEBUG_UART_RX_BUFFER_SIZE - RxOutIndex;
         CopyLength = (MaxRead < Count)?MaxRead:Count;

         /* Process the number of characters till the buffer wraps or   */
         /* maximum number that we can store in the passed in buffer.   */
         CopyLength = (Length < CopyLength)?Length:CopyLength;

         /* Copy the characters over.                                   */
         BTPS_MemCopy(&Buffer[Processed], &RecvBuffer[RxOutIndex], CopyLength);

         /* Update the counts.                                          */
         Processed      += CopyLength;
         Length         -= CopyLength;
         RxOutIndex     += CopyLength;

         /* Handle the case where the out index wraps.                  */
         if(RxOutIndex >= BT_DEBUG_UART_RX_BUFFER_SIZE)
            RxOutIndex = 0;

         /* This is changed in an interrupt so we must protect this     */
         /* section.                                                    */
         __disable_interrupt();

         RxBytesFree += CopyLength;

         __enable_interrupt();
      }
   }

   return(Processed);
}

   /* This function writes a fixed size string to the UART port         */
   /* specified by UartBase.                                            */
void HAL_ConsoleWrite(unsigned int Length, char *String)
{
#if BT_DEBUG_UART_TX_BUFFER_SIZE

   unsigned int Count;

#endif

   volatile int Flags;

   /* First make sure the parameters seem semi valid.                   */
   if((Length) && (String))
   {
      /* Loop and transmit all characters to the Debug UART.            */
      while(Length)
      {
#if BT_DEBUG_UART_TX_BUFFER_SIZE

         if(TxBytesFree)
         {
            /* Get the number of bytes till we reach the end of the     */
            /* buffer.                                                  */
            Count = (BT_DEBUG_UART_TX_BUFFER_SIZE-TxInIndex);

            /* Limit this by the number of Byte that are available.     */
            if(Count > TxBytesFree)
               Count = TxBytesFree;

            if(Count > Length)
               Count = Length;

            /* Copy as much data as we can.                             */
            BTPS_MemCopy(&TransBuffer[TxInIndex], String, Count);

            /* Adjust the number of Free Bytes.                         */
            Flags = (__get_interrupt_state() & GIE);
            __disable_interrupt();

            TxBytesFree -= Count;

            if(Flags)
               __enable_interrupt();

            /* Adjust the Index and Counts.                             */
            TxInIndex += Count;
            String    += Count;
            Length    -= Count;
            if(TxInIndex == BT_DEBUG_UART_TX_BUFFER_SIZE)
               TxInIndex = 0;

            /* Check to see if we need to prime the transmitter.        */
            if(!UARTIntTransmitEnabled(BT_DEBUG_UART_BASE))
            {
               /* Send the next character out.                          */
               UARTTransmitBufferReg(BT_DEBUG_UART_BASE) = TransBuffer[TxOutIndex++];

               /* Decrement the number of characters that are in the    */
               /* transmit buffer and adjust the out index.             */
               TxBytesFree++;
               if(TxOutIndex == BT_DEBUG_UART_TX_BUFFER_SIZE)
                  TxOutIndex = 0;

               UARTIntEnableTransmit(BT_DEBUG_UART_BASE);
            }
         }

#else

         /* Loop until the TXBUF is empty.                              */
         while(!UARTTransmitBufferEmpty(BT_DEBUG_UART_BASE));

         /* Now that the TXBUF is empty send the character.             */
         UARTTransmitBufferReg(BT_DEBUG_UART_BASE) = *String;

         String++;
         Length--;

#endif

      }
   }
}

   /* The following function is used to return the configured system    */
   /* clock speed in Hz.                                               */
unsigned long HAL_GetSystemSpeed(void)
{
   Cpu_Frequency_t Frequency;

   /* Verify that the CPU Frequency enumerated type is valid, if it is  */
   /* not then we will force it to a default.                           */
   if((BT_CPU_FREQ != cf8MHZ_t) && (BT_CPU_FREQ != cf16MHZ_t) && (BT_CPU_FREQ != cf20MHZ_t) && (BT_CPU_FREQ != cf25MHZ_t))
      Frequency = cf16MHZ_t;
   else
      Frequency = BT_CPU_FREQ;

   if((!DetermineProcessorType()) && ((Frequency == cf20MHZ_t) || (Frequency == cf25MHZ_t)))
      return(((unsigned long)Frequency_Settings[cf16MHZ_t - cf8MHZ_t].DCO_Multiplier) * 32768L);
   else
      return(((unsigned long)Frequency_Settings[Frequency - cf8MHZ_t].DCO_Multiplier) * 32768L);
}

   /* This function is called to get the system Tick Count.             */
unsigned long HAL_GetTickCount(void)
{
   return(MSP430Ticks);
}

   /* The following Toggles an LED at a passed in blink rate.           */
void HAL_LedToggle(int LED_ID)
{
   ToggleLED(0);
}

   /* The following function is used to set an LED to a specified state.*/
void HAL_SetLED(int LED_ID, int State)
{
   SetLED(LED_ID, State);
}

   /* The following function is called to enter LPM3 mode on the MSP    */
   /* with the OS Timer Tick Disabled.                                  */
void HAL_LowPowerMode(unsigned char DisableLED)
{
   /* Turn off Timer 1, which is used for the FreeRTOS and NoRTOS       */
   /* timers. The timer runs off of the Auxilary Clock (ACLK) thus      */
   /* without this the timer would continue to run, consuming power     */
   /* and waking up the processor every 1 ms. Enter a critical section  */
   /* to do so that we do get switched out by the OS in the middle of   */
   /* stopping the OS Scheduler.                                        */
   __disable_interrupt();

   STOP_SCHEDULER();

   /* Clear the count register.                                         */
   TA1R = 0;

   __enable_interrupt();

   /* Turn off the LEDs if requested.                                   */
   if(DisableLED)
   {
      SetLED(0, 0);
      SetLED(1, 0);
   }

   /* Enter LPM3.                                                       */
   LPM3;

   /* Re-start the OS scheduler.                                        */
   START_SCHEDULER();

   /* Set the interrupt trigger bit to trigger an interrupt.            */
   TA1CCTL0 |= 0x01;
}

   /* The following function is called to enable the SMCLK Peripheral   */
   /* on the MSP430.                                                    */
   /* * NOTE * This function should be called with interrupts disabled. */
void HAL_EnableSMCLK(unsigned char Peripheral)
{
   /* Note, we will only disable SMCLK Request if the Baud Rate for the */
   /* Debug Console is less than or equal to 9600 BAUD.                 */
#if BT_DEBUG_UART_BAUDRATE <= 9600

   UCSCTL8                   |= SMCLKREQEN;
   ClockRequestedPeripherals |= Peripheral;

#endif
}

   /* The following function is called to disable the SMCLK Peripheral  */
   /* on the MSP430.                                                    */
   /* * NOTE * This function should be called with interrupts disabled. */
void HAL_DisableSMCLK(unsigned char Peripheral)
{
   /* Note, we will only disable SMCLK Request if the Baud Rate for the */
   /* Debug Console is less than or equal to 9600 BAUD.                 */
#if BT_DEBUG_UART_BAUDRATE <= 9600

   ClockRequestedPeripherals &= ~Peripheral;

   if(!ClockRequestedPeripherals)
      UCSCTL8 &= ~SMCLKREQEN;

#endif
}

   /* Timer A Get Tick Count Function for BTPSKRNL Timer A Interrupt.   */
   /* Included for Non-OS builds                                        */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER_INTERRUPT(void)
{
   ++MSP430Ticks;

   /* Exit from LPM if necessary (this statement will have no effect if */
   /* we are not currently in low power mode).                          */
   LPM3_EXIT;
}

   /* Debug UART Receive Interrupt Handler.                             */
#pragma vector=BT_DEBUG_UART_IV
__interrupt void DEBUG_UART_INTERRUPT(void)
{
   unsigned char ch;

   if(BT_DEBUG_UART_IVR == USCI_UCRXIFG)
   {
      /* Read the received character.                                   */
      ch = UARTReceiveBufferReg(BT_DEBUG_UART_BASE);

      /* Place characters in receive buffer if there is any space.      */
      if(RxBytesFree)
      {
         /* Save the character in the Receive Buffer.                   */
         RecvBuffer[RxInIndex++] = ch;
         RxBytesFree--;

         /* Wrap the buffer if neccessary.                              */
         if(RxInIndex >= BT_DEBUG_UART_RX_BUFFER_SIZE)
            RxInIndex = 0;
      }

      /* Exit from LPM if necessary (this statement will have no effect */
      /* if we are not currently in low power mode).                    */
      LPM3_EXIT;
   }

#if BT_DEBUG_UART_TX_BUFFER_SIZE

   else
   {
      if(TxBytesFree != BT_DEBUG_UART_TX_BUFFER_SIZE)
      {
         /* Send the next character out.                                */
         UARTTransmitBufferReg(BT_DEBUG_UART_BASE) = TransBuffer[TxOutIndex++];

         /* Decrement the number of characters that are in the transmit */
         /* buffer and adjust the out index.                            */
         TxBytesFree++;
         if(TxOutIndex == BT_DEBUG_UART_TX_BUFFER_SIZE)
            TxOutIndex = 0;
      }
      else
      {
         /* There is no more data, so disable the TX Interrupt.         */
         UARTIntDisableTransmit(BT_DEBUG_UART_BASE);
      }
   }

#endif
}

   /* CTS Pin Interrupt. CtsInterrupt routine must change the polarity  */
   /* of the Cts Interrupt.                                             */
#pragma vector=BT_UART_CTS_IV
__interrupt void CTS_ISR(void)
{
   switch(BT_UART_CTS_IVR)
   {
      case BT_UART_CTS_INT_NUM:
         if(CtsInterrupt())
         {
            /* Exit LPM3 on interrupt exit (RETI).                      */
            LPM3_EXIT;
         }
         break;
   }
}

