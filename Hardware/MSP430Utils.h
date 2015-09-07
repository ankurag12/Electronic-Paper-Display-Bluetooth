/*****< BTPSMSP430F5438.h >*********************************************************/
/*      Copyright 2000 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPSMSP430F5438 - Bluetopia MSP430 Prototypes, Constants, Macros for TI   */
/*                   MSP430F5438 chip                                         */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/07/10  Tim Cook       Initial creation.                               */
/******************************************************************************/
#ifndef BTPSMSP430F5438_H_
#define BTPSMSP430F5438_H_

#include <msp430.h>

   /* The following define the maximum frequency settings for the       */
   /* MSP430F5438 and MSP430F5438A.                                     */
#define MAXIMUM_MSP430F5438_FREQUENCY   16000000l
#define MAXIMUM_MSP430F5438A_FREQUENCY  25001984l

   /* The following are the defines for the various GPIO Regs           */
#define MSP430F5438_GPIO_INPUT_OFFSET     (0x00)
#define MSP430F5438_GPIO_OUTPUT_OFFSET    (0x02)
#define MSP430F5438_GPIO_DIR_OFFSET       (0x04)
#define MSP430F5438_GPIO_RESEN_OFFSET     (0x06)
#define MSP430F5438_GPIO_DRVSTR_OFFSET    (0x08)
#define MSP430F5438_GPIO_PSEL_OFFSET      (0x0A)
#define MSP430F5438_GPIO_INTEDGE_OFFSET   (0x18)
#define MSP430F5438_GPIO_INTEN_OFFSET     (0x1A)
#define MSP430F5438_GPIO_INTFLAG_OFFSET   (0x1C)

   /* The following are defines relating to the setting up of the UART  */
#define UART_CONFIG_WLEN_bit              (4)
#define UART_CONFIG_PAR_bit               (7)
#define UART_CONFIG_PAR_SEL_bit           (6)
#define UART_CONFIG_STOP_bit              (3)

   /* the following define the registers of the UART the EXP Board uses */

   /* generic control registers                                         */
#define MSP430_UART_CTL0                  (UCA2CTL0)
#define MSP430_UART_CTL0_OFFSET           (0x01)

   /* control register 1                                                */
#define MSP430_UART_CTL1                  (UCA2CTL1)
#define MSP430_UART_CTL1_OFFSET           (0x00)

   /* control register 1 USCI clock source select settings              */
#define MSP430_UART_CTL1_UCSSEL_bit             (6)
#define MSP430_UART_CTL1_UCSSEL_SMCLK_bits      (0x3)
#define MSP430_UART_CTL1_UCSSEL_ACLK_bits       (0x1)
#define MSP430_UART_CTL1_UCSSEL_UCA2CLK_bits    (0x0)
#define MSP430_UART_CTL1_UCSSEL_SMCLK_mask      (MSP430_UART_CTL1_UCSSEL_SMCLK_bits   << MSP430_UART_CTL1_UCSSEL_bit)
#define MSP430_UART_CTL1_UCSSEL_ACLK_mask       (MSP430_UART_CTL1_UCSSEL_ACLK_bits    << MSP430_UART_CTL1_UCSSEL_bit)
#define MSP430_UART_CTL1_UCSSEL_UCA2CLK_mask    (MSP430_UART_CTL1_UCSSEL_UCA2CLK_bits << MSP430_UART_CTL1_UCSSEL_bit)

   /* control register 1 1 bit field definitions                        */
#define MSP430_UART_CTL1_RXIE       (BIT5)
#define MSP430_UART_CTL1_BRKIE      (BIT4)
#define MSP430_UART_CTL1_DORM       (BIT3)
#define MSP430_UART_CTL1_TXADDR     (BIT2)
#define MSP430_UART_CTL1_TXBRK      (BIT1)
#define MSP430_UART_CTL1_SWRST      (BIT0)

   /* clock prescale settings                                           */
#define MSP430_UART_BRW             (UCA2BRW)
#define MSP430_UART_BRW_OFFSET      (0x06)

   /* Modulation stage selection and oversampling mode enable           */
#define MSP430_UART_MCTL               (UCA2MCTL)
#define MSP430_UART_MCTL_OFFSET        (0x08)
#define MSP430_UART_MCTL_BRF_bit       (4)
#define MSP430_UART_MCTL_BRF_MASK      (0xF << MSP430_UART_MCTL_BRF_bit)
#define MSP430_UART_MCTL_BRS_bit       (1)
#define MSP430_UART_MCTL_BRS_MASK      (0x7 << MSP430_UART_MCTL_BRS_bit)
#define MSP430_UART_MCTL_UCOS16_mask   (BIT0)

   /* Status Register Definitions                                       */
#define MSP430_UART_STAT                        (UCA2STAT)
#define MSP430_UART_STAT_OFFSET                 (0x0A)
#define MSP430_UART_STAT_LOOPBACK_ENABLE_mask   (BIT7)
#define MSP430_UART_STAT_FRAME_ERROR_mask       (BIT6)
#define MSP430_UART_STAT_OVERRUN_ERROR_mask     (BIT5)
#define MSP430_UART_STAT_PAR_ERR_mask           (BIT4)
#define MSP430_UART_STAT_BRK_DETECT_mask        (BIT3)
#define MSP430_UART_STAT_RCV_ERR_mask           (BIT2)
#define MSP430_UART_STAT_ADDR_IDLE_mask         (BIT1)
#define MSP430_UART_STAT_BUSY_mask              (BIT0)

   /* Receive and Transmit Buffer definitions                           */
#define MSP430_UART_RXBUF        (UCA2RXBUF)
#define MSP430_UART_RXBUF_OFFSET (0x0C)
#define MSP430_UART_TXBUF        (UCA2TXBUF)
#define MSP430_UART_TXBUF_OFFSET (0x0E)

   /* Receive/Transmit Interrupt Enable definitions                     */
#define MSP430_UART_IE           (UCA2IE)
#define MSP430_UART_IE_OFFSET    (0x1C)
#define MSP430_UART_TXIE_mask    (BIT1)
#define MSP430_UART_RXIE_mask    (BIT0)

   /* Receive/Transmit Interrupt Flag definitions                       */
#define MSP430_UART_IFG          (UCA2IFG)
#define MSP430_UART_IFG_OFFSET   (0x1D)
#define MSP430_UART_TXIFG_mask   (BIT1)
#define MSP430_UART_RXIFG_mask   (BIT0)

   /* Receive/Transmit Interrupt Vector Register                        */
#define MSP430_UART_IV           (UCA2IV)
#define MSP430_UART_IV_OFFSET    (0x1E)

   /* MSP430F5438 DMA Utilities and Defines                             */
#define MSP430_DMA_CTL              (DMACCTL0)

      /* using ch0                                                      */
#define MSP430_DMA_CTL_TSEL_bit     (0)
#define MSP430_DMA_CTL_TSEL_MASK    (0x1F << MSP430_DMA_CTL_TSEL_bit)

      /* trig.  src 16                                                  */
#define MSP430_DMA_CTL_TSRC         (0x10 << SP430_DMA_CTL_TSEL_bit)

   /* DMA Channel x Control Register Definitions                        */
#define MSP430_DMA_CHN_CTL                      (DMA0CTL)
#define MSP430_DMA_CHN_CTL_DT_bit               (12)
#define MSP430_DMA_CHN_CTL_DT_SINGLE            (0)
#define MSP430_DMA_CHN_CTL_DT_BLOCK             (0x01 << MSP430_DMA_CHN_CTL_DT_bit)
#define MSP430_DMA_CHN_CTL_DT_BURST_BLOCK       (0x02 << MSP430_DMA_CHN_CTL_DT_bit)
#define MSP430_DMA_CHN_CTL_DT_REP_SINGLE        (0x04 << MSP430_DMA_CHN_CTL_DT_bit)
#define MSP430_DMA_CHN_CTL_DT_REP_BLOCK         (0x05 << MSP430_DMA_CHN_CTL_DT_bit)
#define MSP430_DMA_CHN_CTL_DT_REP_BURST_BLOCK   (0x06 << MSP430_DMA_CHN_CTL_DT_bit)

#define MSP430_DMA_CHN_CTL_DST_INCR_bit         (10)
#define MSP430_DMA_CHN_CTL_DST_INCR_UNCHANGED   (0)
#define MSP430_DMA_CHN_CTL_DST_INCR_DECRMENT    (0x2 << MSP430_DMA_CHN_CTL_DST_INCR_bit)
#define MSP430_DMA_CHN_CTL_DST_INCR_INCREMENT   (0x3 << MSP430_DMA_CHN_CTL_DST_INCR_bit)

#define MSP430_DMA_CHN_CTL_SRC_INCR_bit         (8)
#define MSP430_DMA_CHN_CTL_SRC_INCR_UNCHANGED   (0)
#define MSP430_DMA_CHN_CTL_SRC_INCR_DECRMENT    (0x2 << MSP430_DMA_CHN_CTL_SRC_INCR_bit)
#define MSP430_DMA_CHN_CTL_SRC_INCR_INCREMENT   (0x3 << MSP430_DMA_CHN_CTL_SRC_INCR_bit)

#define MSP430_DMA_CHN_CTL_DEST_BYTE_bit        (7)
#define MSP430_DMA_CHN_CTL_DEST_IS_BYTE         (1 << MSP430_DMA_CHN_CTL_DEST_BYTE_bit)
#define MSP430_DMA_CHN_CTL_DEST_IS_WORD         (0)

#define MSP430_DMA_CHN_CTL_SRC_BYTE_bit         (6)
#define MSP430_DMA_CHN_CTL_SRC_IS_BYTE          (1 << MSP430_DMA_CHN_CTL_SRC_BYTE_bit)
#define MSP430_DMA_CHN_CTL_SRC_IS_WORD          (0)

#define MSP430_DMA_CHN_CTL_LEVEL_bit            (5)
#define MSP430_DMA_CHN_CTL_LEVEL_EDGE_SENS      (0)
#define MSP430_DMA_CHN_CTL_LEVEL_LEVEL_SENS     (1 << MSP430_DMA_CHN_CTL_LEVEL_bit)

#define MSP430_DMA_CHN_CTL_ENABLE_MASK          (BIT4)
#define MSP430_DMA_CHN_CTL_IFG_MASK             (BIT3)
#define MSP430_DMA_CHN_CTL_IE_MASK              (BIT2)
#define MSP430_DMA_CHN_CTL_ABORT_MASK           (BIT1)
#define MSP430_DMA_CHN_CTL_REQ_MASK             (BIT0)

   /* UART Platform Typedefs.                                            */

   /* Used to determine size of a RAM address.                          */
typedef unsigned char *RAMAddress_t;

   /* Defines to fit into HCITRANS_PLATFORM.h structure                 */
#define HWREG8(_x_)        (*((volatile unsigned char *)((RAMAddress_t)(_x_))))
#define HWREG8ADDR(_x_)    ((volatile unsigned char *)((RAMAddress_t)(_x_)))
#define HWREG16(_x_)       (*((volatile unsigned int *)((RAMAddress_t)(_x_))))
#define HWREG16ADDR(_x_)   ((volatile unsigned int *)((RAMAddress_t)(_x_)))
#define HWREG32(_x_)       (*((volatile unsigned long  *)((RAMAddress_t)(_x_))))
#define HWREG32ADDR(_x_)   ((volatile unsigned long *)((RAMAddress_t)(_x_)))

   /* UART Macros and Defines Interrupt Enable Register                 */
#define UART_O_IER            (MSP430_UART_IE_OFFSET)
#define UART_O_IM             (UART_O_IER)
         /* Interrupt Enable Transmit Interrupt Flag                    */
#define UART_IER_TXIE         (MSP430_UART_TXIE_mask)
#define UART_IM_TXIM          (UART_IER_TXIE) /* alias for above              */
         /* Interrupt Enable Receive Interrupt Flag                     */
#define UART_IER_RXIE         (MSP430_UART_RXIE_mask)
#define UART_INT_RX           (UART_IER_RXIE) /* alias for above              */
#define UART_INT_RT           (0)

         /* Interrupt Flag Register                                     */
#define UART_O_FR             (MSP430_UART_IFG_OFFSET)
         /* Receive Interrupt Flag Mask (indicating data ready)         */
#define UART_FR_RXFG          (MSP430_UART_RXIFG_mask)

         /* Transmit Interrupt Flag Mask (indicating TXBUF Empty)       */
#define UART_FR_TXFG          (MSP430_UART_TXIFG_mask)
         /* Receive Buffer Register (or FIFO)                           */
#define UART_O_DR             (MSP430_UART_RXBUF_OFFSET)

         /* Transmit Buffer Register (or FIFO)                          */
#define UART_O_TR             (MSP430_UART_TXBUF_OFFSET)

         /* Line Control Register                                       */
#define UART_O_LCR            (MSP430_UART_CTL0_OFFSET)
         /* Line Control Flag Fields                                    */
#define UART_CONFIG_WLEN_7                   (1 << UART_CONFIG_WLEN_bit)
#define UART_CONFIG_WLEN_8                   (0)
#define UART_CONFIG_PAR_NONE                 (0)
#define UART_CONFIG_PAR_ODD                  ((1 << UART_CONFIG_PAR_bit) | 0)
#define UART_CONFIG_PAR_EVEN                 ((1 << UART_CONFIG_PAR_bit) | (1 << UART_CONFIG_PAR_SEL_bit))
#define UART_CONFIG_STOP_ONE                 (0)
#define UART_CONFIG_STOP_TWO                 (1 << UART_CONFIG_STOP_bit)

         /* Line Control Macros, use 1 or more ORed flags above         */
#define UARTSetLCR(_x_,_flags_)              (HWREG8(((RAMAddress_t)(_x_)) + UART_O_LCR) |= (_flags_))
#define UARTClearLCR(_x_,_flags_)            (HWREG8(((RAMAddress_t)(_x_)) + UART_O_LCR) &= (_flags_))

         /* Interrupt Flag and Enable Register Access Macros            */
#define UARTIntFlagReg(_x_)                  (HWREG8(((RAMAddress_t)(_x_)) + UART_O_FR))
#define UARTIntEnableReg(_x_)                (HWREG8(((RAMAddress_t)(_x_)) + UART_O_IER))

         /* Macro to enable or disable Receiver or Transmit Interrupts  */
         /* Flags may be: UART_IER_TXIE, UART_IER_RXIE                  */
#define UARTIntEnable(_x_,_flags_)           (UARTIntEnableReg(_x_) |= (_flags_))
#define UARTIntDisable(_x_,_flags_)          (UARTIntEnableReg(_x_) &= (_flags_))

#define UARTIntEnableReceive(_x_)            (UARTIntEnable((_x_),UART_IER_RXIE))
#define UARTIntDisableReceive(_x_)           (UARTIntDisable((_x_),~UART_IER_RXIE))
#define UARTIntReceiveEnabled(_x_)           (UARTIntEnableReg(_x_) & UART_IER_RXIE)

#define UARTIntEnableTransmit(_x_)           (UARTIntEnable((_x_),UART_IER_TXIE))
#define UARTIntDisableTransmit(_x_)          (UARTIntDisable((_x_),~UART_IER_TXIE))
#define UARTIntTransmitEnabled(_x_)          (UARTIntEnableReg(_x_) & UART_IER_TXIE)

#define UARTIntEnableTransmitReceive(_x_)    (UARTIntEnable((_x_), (UART_IER_TXIE | UART_IER_RXIE)))
#define UARTIntDisableTransmitReceive(_x_)   (UARTIntDisable((_x_),(UART_IER_TXIE | UART_IER_RXIE)))

         /* Receive & Transmit Buffer Register Access Macros            */
#define UARTTransmitBufferReg(_x_)           (HWREG8(((RAMAddress_t)(_x_)) + UART_O_TR))
#define UARTTransmitBufferAddr(_x_)          (HWREG8ADDR(((RAMAddress_t)(_x_)) + UART_O_TR))
#define UARTTransmitBufferEmpty(_x_)         (UARTIntFlagReg((_x_)) & UART_FR_TXFG)

#define UARTReceiveBufferReg(_x_)            (HWREG8(((RAMAddress_t)(_x_)) + UART_O_DR))
#define UARTReceiveBufferAddr(_x_)           (HWREG8ADDR(((RAMAddress_t)(_x_)) + UART_O_DR))
#define UARTReceiveBufferEmpty(_x_)          (!(UARTIntFlagReg((_x_)) & UART_FR_RXFG))
#define UARTReceiveBufferOverrun(_x_)        (HWREG8(((RAMAddress_t)(_x_)) + MSP430_UART_STAT_OFFSET) & MSP430_UART_STAT_OVERRUN_ERROR_mask)

         /* UART GPIO Pin Configuration                                 */
#define GPIOPinTypeUART(_x_,_txd_,_rxd_)                                                  \
{                                                                                         \
   (HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_PSEL_OFFSET) |= (_txd_ | _rxd_));   \
   (HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_DIR_OFFSET)  |= (_txd_));           \
   (HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_DIR_OFFSET)  &= (~(_rxd_)));        \
}
      /* The following MACROs are used to configure Peripheral Pins.    */
#define GPIOPinTypePeripheralOutput(_base,_pin)                                     \
{                                                                                   \
   HWREG8(((RAMAddress_t)(_base)) + MSP430F5438_GPIO_DIR_OFFSET)  |= (_pin);      \
   HWREG8(((RAMAddress_t)(_base)) + MSP430F5438_GPIO_PSEL_OFFSET) |= (_pin);      \
}

   /* Perform any special config for Flow RTS should be output.  CTS    */
   /* should be input with interrupts.  Both should be configured to be */
   /* I/O.                                                              */
#define GPIOPinTypeCtsFlow(_x_,_cts_)                                                    \
{                                                                                        \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_PSEL_OFFSET)     &= (~(_cts_));   \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_DIR_OFFSET)      &= (~(_cts_));   \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTFLAG_OFFSET)  &= (~(_cts_));   \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTEDGE_OFFSET)  |= (_cts_);      \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTEN_OFFSET)    |= (_cts_);      \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_OUTPUT_OFFSET)   &= (~(_cts_));   \
}

#define GPIOPinTypeRtsFlow(_x_,_rts_)                                                    \
{                                                                                        \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_PSEL_OFFSET)     &= (~(_rts_));   \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_DIR_OFFSET)      |= (_rts_);      \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_DRVSTR_OFFSET)   |= (_rts_);      \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTFLAG_OFFSET)  &= (~(_rts_));   \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_OUTPUT_OFFSET)   &= (~(_rts_));   \
}

#define GPIOPinTypeFlow(_x_,_rts_,_cts_)                                                           \
{                                                                                                  \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_PSEL_OFFSET)     &= (~((_rts_)|(_cts_)));   \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_DIR_OFFSET)      &= (~(_cts_));             \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_DIR_OFFSET)      |= (_rts_);                \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_DRVSTR_OFFSET)   |= (_rts_);                \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTFLAG_OFFSET)  &= (~((_rts_) | (_cts_))); \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTEDGE_OFFSET)  |= (_cts_);                \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTEN_OFFSET)    |= (_cts_);                \
     HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_OUTPUT_OFFSET)   &= (~((_rts_) | (_cts_))); \
}

#define GPIOIntPosEdge(_x_,_rts_)      (HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTEDGE_OFFSET)  &= (~(_rts_)))
#define GPIOIntNegEdge(_x_,_rts_)      (HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTEDGE_OFFSET)  |= (_rts_))
#define GPIOIntEdgeIsNeg(_x_,_rts_)    (HWREG8(((RAMAddress_t)(_x_)) + MSP430F5438_GPIO_INTEDGE_OFFSET)  &  (_rts_))

         /* Enable/Disable Flow Control                                 */
#define GPIORead(_base)             (HWREG8((_base) + MSP430F5438_GPIO_INPUT_OFFSET))

         /* RTS/CTS Flow Control Utilities                              */
#define GPIOIntEdgeEnableFlow(_base,_rts)          (GPIOIntEdgeIsNeg((_base),(_rts)))
#define GPIOIntEdgeDisableFlow(_base,_rts)         (!GPIOIntEdgeIsNeg((_base),(_rts)))
#define GPIOIntEdgeEnableStateChange(_base,_rts)   (GPIOIntPosEdge((_base),(_rts)))
#define GPIOIntEdgeDisableStateChange(_base,_rts)  (GPIOIntNegEdge((_base),(_rts)))
#define GPIOCtsDisabled(_base,_rts)                (GPIORead((_base)) & (_rts))

         /* Chip Reset Control                                          */
#define GPIOPinTypeReset(_base,_pin)                                                      \
{                                                                                         \
   HWREG8(((RAMAddress_t)(_base)) + MSP430F5438_GPIO_PSEL_OFFSET)       &= (~(_pin));   \
   HWREG8(((RAMAddress_t)(_base)) + MSP430F5438_GPIO_DIR_OFFSET)        |= (_pin);      \
   HWREG8(((RAMAddress_t)(_base)) + MSP430F5438_GPIO_OUTPUT_OFFSET)     &= (~(_pin));   \
}

   /* DMA Defines and Macros for UART Platform DMA Driver               */
#define DMAControlRegBase              ((RAMAddress_t)&DMACTL0)

   /* DMA Register Offsets                                              */
#define DMA_CHANNEL_CONTROL_OFFSET     (0x00)
#define DMA_SOURCE_ADDRESS_OFFSET      (0x02)
#define DMA_DEST_ADDRESS_OFFSET        (0x06)
#define DMA_TRANSFER_SIZE_OFFSET       (0x0A)

   /* Macros to Access DMA Registers                                    */
#define DMAControlReg(_base)           (HWREG16(((RAMAddress_t)(_base)) + DMA_CHANNEL_CONTROL_OFFSET))
#define DMASourceAddressReg(_base)     (HWREG32(((RAMAddress_t)(_base)) + DMA_SOURCE_ADDRESS_OFFSET))
#define DMADestAddressReg(_base)       (HWREG32(((RAMAddress_t)(_base)) + DMA_DEST_ADDRESS_OFFSET))
#define DMATransferSizeReg(_base)      (HWREG16(((RAMAddress_t)(_base)) + DMA_TRANSFER_SIZE_OFFSET))

   /* DMA Trigger Setup                                                 */
#define DMAGetTransmitTriggerForUart(_UartNum_) (17 + ((_UartNum_) * 4))
#define DMAGetReceiveTriggerForUart(_UartNum_)  (16 + ((_UartNum_) * 4))
#define DMASetTrigger(_base,_channel,_trigger)  (HWREG16(DMAControlRegBase + ((_channel) - ((_channel) & 1))) |= (_trigger << ((_channel & 1)?8:0)))

   /* DMA Transfer Setup Transfer Setup Flags Specifies the Hardware    */
   /* specific configuration flags We will specifiy the platform flags  */
   /* to be for single transfer                                         */
#define DMA_PLATFORM_RX_FLAGS          (MSP430_DMA_CHN_CTL_DT_SINGLE)//(MSP430_DMA_CHN_CTL_DT_REP_SINGLE)
#define DMA_PLATFORM_TX_FLAGS          (MSP430_DMA_CHN_CTL_DT_SINGLE)//(MSP430_DMA_CHN_CTL_DT_REP_SINGLE)

   /* Specifies whether Source,Dest.  Addr Incr/Decr Lack of flag for   */
   /* Source or Dest means leave unchanged Default for both is unchanged*/
#define DMA_SRC_ADDR_INCR              (MSP430_DMA_CHN_CTL_SRC_INCR_INCREMENT)
#define DMA_SRC_ADDR_DECR              (MSP430_DMA_CHN_CTL_SRC_INCR_DECRMENT)
#define DMA_SRC_ADDR_UNCH              (0)
#define DMA_DEST_ADDR_INCR             (MSP430_DMA_CHN_CTL_DST_INCR_INCREMENT)
#define DMA_DEST_ADDR_DECR             (MSP430_DMA_CHN_CTL_DST_INCR_DECRMENT)
#define DMA_DEST_ADDR_UNCH             (0)

         /* Specify if Destination,Source Byte or Word Default is byte  */
#define DMA_SRC_BYTE                   (MSP430_DMA_CHN_CTL_SRC_IS_BYTE)
#define DMA_SRC_WORD                   (MSP430_DMA_CHN_CTL_SRC_IS_WORD)
#define DMA_DEST_BYTE                  (MSP430_DMA_CHN_CTL_DEST_IS_BYTE)
#define DMA_DEST_WORD                  (MSP430_DMA_CHN_CTL_DEST_IS_WORD)

         /* Specifiy if Trigger is Edge or Level Sensitive              */
#define DMA_TRIGGER_EDGE               (MSP430_DMA_CHN_CTL_LEVEL_EDGE_SENS)
#define DMA_TRIGGER_LEVEL              (MSP430_DMA_CHN_CTL_LEVEL_LEVEL_SENS)

         /* Specifiy whether to enable DMA Interrupts or not            */
#define DMA_INT_ENABLE                 (MSP430_DMA_CHN_CTL_IE_MASK)

      /* Transfer Setup Macro                                           */
#define DMAConfigure(_base,_flags_)    (DMAControlReg((_base)) |= (_flags_))

   /* Utility Macros                                                    */
#define DMAEnable(_base)               (DMAControlReg((_base)) |= (MSP430_DMA_CHN_CTL_ENABLE_MASK))
#define DMADisable(_base)              (DMAControlReg((_base)) &= (~(MSP430_DMA_CHN_CTL_ENABLE_MASK)))
#define DMAEnabled(_base)              (DMAControlReg((_base)) &  (MSP430_DMA_CHN_CTL_ENABLE_MASK))
#define DMAEnableInt(_base)            (DMAControlReg((_base)) |= DMA_INT_ENABLE)
#define DMADisableInt(_base)           (DMAControlReg((_base)) &= (~(DMA_INT_ENABLE)))
#define DMASoftTrig(_base)             (DMAControlReg((_base)) |= (MSP430_DMA_CHN_CTL_REQ_MASK))
#define DMADevSpecific(_base)          (DMACTL4                |= BIT2)

   /* DMA Source,Destinaton,Size setup                                  */
#define DMASetSource(_base,_source)    (DMASourceAddressReg(_base) = (unsigned long)(_source))
#define DMASetDest(_base,_dest)        (DMADestAddressReg(_base)   = (unsigned long)(_dest))
#define DMASetSize(_base,_size)        (DMATransferSizeReg(_base)  = (unsigned int)(_size))

#endif


