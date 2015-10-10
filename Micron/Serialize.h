/**********************  DRIVER FOR SPI CONTROLLER ON MSP430**********************

   Filename:     Serialize.h
   Description:  Header file of Serialize.c
   Version:      0.1
   Date:         Sept 2015
   Authors:      Ankur Agrawal

********************************************************************************/

#ifndef _SERIALIZE_H_
#define _SERIALIZE_H_

typedef unsigned char	uint8;
typedef signed char		sint8;
typedef unsigned int	uint16;
typedef int				sint16;
typedef unsigned long	uint32;
typedef long			sint32;

#define NULL_PTR 0x0   // a null pointer


/*Return Type*/

typedef enum
{
	RetSpiError,
	RetSpiSuccess
} SPI_STATUS;

typedef unsigned char Bool;

// Acceptable values for SPI master side configuration
typedef enum _SpiConfigOptions
{
	OpsNull,  			// do nothing
	OpsWakeUp,			// enable transfer
	OpsInitTransfer,
	OpsEndTransfer,

} SpiConfigOptions;


// char stream definition for
typedef struct _structCharStream
{
	uint8* pChar;                                // buffer address that holds the streams
	uint32 length;                               // length of the stream in bytes
} CharStream;

SPI_STATUS Serialize_SPI(const CharStream* char_stream_send,
                         CharStream* char_stream_recv,
                         SpiConfigOptions optBefore, SpiConfigOptions optAfter) ;

void ConfigureSpi(SpiConfigOptions opt);
//void four_byte_addr_ctl(int enable) ;


#endif //end of file


