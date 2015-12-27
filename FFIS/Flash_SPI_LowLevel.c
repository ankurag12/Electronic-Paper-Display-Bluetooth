/*
 * msp430_spi.c
 *
 *  Created on: Oct 4, 2015
 *      Author: Robo
 */
#include "Flash_SPI_LowLevel.h"
#include <msp430.h>
#include "../epd_sys/assert-pl.h"
#include "../epd_sys/utils.h"
#include "../pl/gpio.h"
#include "../pl/platform.h"
#include "../Hardware/msp430/msp430-defs.h"
#include "../Hardware/msp430/msp430-gpio.h"

//SPI for Ext Flash
#define USCI_UNIT	B
#define USCI_CHAN	2
// Pin definitions for this unit
#define	FLASH_SPI_SIMO                MSP430_GPIO(9,1)
#define	FLASH_SPI_SOMI                MSP430_GPIO(9,2)
#define	FLASH_SPI_CLK                 MSP430_GPIO(9,3)
#define	FLASH_SPI_CS                  MSP430_GPIO(9,0)
#define	FLASH_SPI_RST                 MSP430_GPIO(9,7)


struct pl_gpio *SPI_Flash_plat;

void flash_spi_init()
{

	SPI_Flash_plat = &g_plat.gpio;

	static const struct pl_gpio_config gpios[] = {
		{ FLASH_SPI_CS,   PL_GPIO_OUTPUT  | PL_GPIO_INIT_H                  },
		{ FLASH_SPI_SIMO, PL_GPIO_SPECIAL | PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
		{ FLASH_SPI_SOMI, PL_GPIO_SPECIAL | PL_GPIO_INPUT                   },
		{ FLASH_SPI_CLK,  PL_GPIO_SPECIAL | PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
		{ FLASH_SPI_RST,  PL_GPIO_OUTPUT  | PL_GPIO_INIT_L                  },
	};

	assert(SPI_Flash_plat != NULL);

	__disable_interrupt();
	UCxnCTL1 |= UCSWRST;				// Put state machine in reset

	if (pl_gpio_config_list(SPI_Flash_plat, gpios, ARRAY_SIZE(gpios)))
		return;

	UCxnCTL0 |= UCMST+UCSYNC+UCMSB+UCCKPH;		// Master, synchronous, MSB
	UCxnCTL1 |= UCSSEL_2;                     // SMCLK
    UCxnBR0 = 1;				// f_UCxCLK = 20MHz/1 = 20MHz
    UCxnBR1 = 0;		//

	UCxnCTL1 &= ~UCSWRST;                  		// Release state machine from reset
	__enable_interrupt();


}

void flash_spi_send_bytes(uint8_t *bytes, uint16_t len)
{
	//uint16_t gie = __get_SR_register() & GIE;   // Store current GIE state

    //__disable_interrupt();                      // Make this operation atomic

    while(len--) {
    	while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
    	// Write the command
    	UCxnTXBUF = *bytes++;
    }

    while (UCxnSTAT & UCBUSY) ;                 // Wait for all TX/RX to finish
    UCxnRXBUF;                                  // Dummy read to empty RX buffer
                                                // and clear any overrun conditions

   // __bis_SR_register(gie);                     // Restore original GIE state

}

void flash_spi_read_bytes(uint8_t *bytes, uint16_t len)
{
   // uint16_t gie = __get_SR_register() & GIE;   // Store current GIE state

   // __disable_interrupt();                      // Make this operation atomic

    UCxnIFG &= ~UCRXIFG;                        // Ensure RXIFG is clear

    // Clock the actual data transfer and receive the bytes
    while (len--){
        while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
        UCxnTXBUF = 0xff;                       // Write dummy byte
        while (!(UCxnIFG & UCRXIFG)) ;          // Wait for RX buffer (full)
        *bytes++ = UCxnRXBUF;
    }

   //__bis_SR_register(gie);                     // Restore original GIE state
}


void flash_setCSHigh(void)
{
	SPI_Flash_plat->set(FLASH_SPI_CS, 1);
}

void flash_setCSLow(void)
{
	SPI_Flash_plat->set(FLASH_SPI_CS, 0);
}

void flash_CheckBusy(void)
{
	while (UCxnSTAT & UCBUSY);
}
