/*****< sppdemo.c >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SPPDEMO - Simple embedded application using SPP Profile.                  */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/24/11  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __SPPDEMO_H__
#define __SPPDEMO_H__

#include "FFIS/FlashFileIndexSystem.h"
#include "pl/platform.h"

extern struct _FlashHW flashObj;		// Defined in msp430-epd.c
extern struct pl_platform g_plat;		// Defined in msp430-epd.c

int ReadCmdFromPhoneApp();

#endif

