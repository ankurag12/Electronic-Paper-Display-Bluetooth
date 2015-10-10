/**********************  DRIVER FOR SPI CONTROLLER ON MSP430**********************


*******************************************************************************/
#include "Serialize.h"
#include <msp430.h>
#include <stdio.h>
#include "../FFIS/Flash_SPI_LowLevel.h"

//#define ENABLE_PRINT_DEBUG

#define EXT_MOD

/*******************************************************************************
Function:     ConfigureSpi(SpiConfigOptions opt)
Arguments:    opt configuration options, all acceptable values are enumerated in
              SpiMasterConfigOptions, which is a typedefed enum.
Return Values:There is no return value for this function.
Description:  This function can be used to properly configure the SPI master
              before and after the transfer/receive operation
Pseudo Code:
   Step 1  : perform or skip select/deselect slave
   Step 2  : perform or skip enable/disable transfer
   Step 3  : perform or skip enable/disable receive
*******************************************************************************/

void ConfigureSpi(SpiConfigOptions opt)
{
	switch (opt)
	{
	case OpsWakeUp:
		flash_CheckBusy();
		flash_setCSLow();
		break;
	case OpsInitTransfer:
		break;
	case OpsEndTransfer:
		flash_setCSHigh();
		break;
	default:
		break;
	}
}



/*******************************************************************************
Function:     Serialize(const CharStream* char_stream_send,
					CharStream* char_stream_recv,
					SpiMasterConfigOptions optBefore,
					SpiMasterConfigOptions optAfter
				)
Arguments:    char_stream_send, the char stream to be sent from the SPI master to
              the Flash memory, usually contains instruction, address, and data to be
              programmed.
              char_stream_recv, the char stream to be received from the Flash memory
              to the SPI master, usually contains data to be read from the memory.
              optBefore, configurations of the SPI master before any transfer/receive
              optAfter, configurations of the SPI after any transfer/receive
Return Values:TRUE
Description:  This function can be used to encapsulate a complete transfer/receive
              operation
Pseudo Code:
   Step 1  : perform pre-transfer configuration
   Step 2  : perform transfer/ receive
   Step 3  : perform post-transfer configuration
*******************************************************************************/
SPI_STATUS Serialize_SPI(const CharStream* char_stream_send,
                         CharStream* char_stream_recv,
                         SpiConfigOptions optBefore,
                         SpiConfigOptions optAfter
                        )
{

	uint8 *char_send, *char_recv;
	uint16 rx_len = 0, tx_len = 0;

#ifdef ENABLE_PRINT_DEBUG
	int i;
	printf("\nSEND: ");
	for(i=0; i<char_stream_send->length; i++)
		printf(" 0x%x ", char_stream_send->pChar[i]);
	printf("\n");
#endif

	tx_len = char_stream_send->length;
	char_send = char_stream_send->pChar;

	if (NULL_PTR != char_stream_recv)
	{
		rx_len = char_stream_recv->length;
		char_recv = char_stream_recv->pChar;
	}



	ConfigureSpi(optBefore);


	flash_spi_send_bytes(char_send, tx_len);

    // RX

	flash_spi_send_bytes(char_recv, rx_len);


#ifdef ENABLE_PRINT_DEBUG
	printf("\nRECV: ");
	for(i=0; i<char_stream_recv->length; i++)
		printf(" 0x%x ", char_stream_recv->pChar[i]);
	printf("\n");
#endif

	ConfigureSpi(optAfter);


	return RetSpiSuccess;
}

/*void four_byte_addr_ctl(int enable)
{
	if(enable)
		FOUR_BYTE_ENABLE;

	if(!enable)
		FOUR_BYTE_DISABLE;
}*/
