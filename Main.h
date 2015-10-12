/*****< main.c >***************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAIN - Stonestreet One main sample application header.                    */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __MAIN_H__
#define __MAIN_H__

#include "SS1BTPS.h"             /* Main SS1 Bluetooth Stack Header.          */
#include "SS1BTVS.h"             /* Vendor Specific Prototypes/Constants.     */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */
#include "pl/platform.h"
#include "FFIS/FlashFileIndexSystem.h"

   /* The following is used as a printf replacement.                    */
#define Display(_x)                 do { BTPS_OutputMessage _x; } while(0)

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define APPLICATION_ERROR_INVALID_PARAMETERS             (-1000)
#define APPLICATION_ERROR_UNABLE_TO_OPEN_STACK           (-1001)

   /* The following function is used to initialize the application      */
   /* instance.  This function should open the stack and prepare to     */
   /* execute commands based on user input.  The first parameter passed */
   /* to this function is the HCI Driver Information that will be used  */
   /* when opening the stack and the second parameter is used to pass   */
   /* parameters to BTPS_Init.  This function returns the               */
   /* BluetoothStackID returned from BSC_Initialize on success or a     */
   /* negative error code (of the form APPLICATION_ERROR_XXX).          */
int InitializeApplication(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization);

   /* The following function is used to process a command line string.  */
   /* This function takes as it's only parameter the command line string*/
   /* to be parsed and returns TRUE if a command was parsed and executed*/
   /* or FALSE otherwise.                                               */
Boolean_t ProcessCommandLine(char *String);



#endif

