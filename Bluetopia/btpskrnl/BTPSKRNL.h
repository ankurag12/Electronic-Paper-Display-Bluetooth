/*****< btpskrnl.h >***********************************************************/
/*      Copyright 2000 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPSKRNL - Stonestreet One Bluetooth Stack Kernel Implementation Type     */
/*             Definitions, Constants, and Prototypes.                        */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/30/01  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPSKRNLH__
#define __BTPSKRNLH__

#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */
#include "BTTypes.h"            /* Bluetooth basic type definitions           */

   /* The following constant represents the Interval Time (in           */
   /* Milliseconds) of a single System Tick.  This constant can be      */
   /* used with the BTPS_GetTickCount() function to measure time        */
   /* intervals in Real-Time Units.                                     */
#define BTPS_TICK_COUNT_INTERVAL                         (1)

   /* The following constant represents the Minimum Amount of Time      */
   /* that can be scheduled with the BTPS Scheduler.  Attempting to     */
   /* Schedule a Scheduled Function less than this time will result in  */
   /* the function being scheduled for the specified Amount.  This      */
   /* value is in Milliseconds.                                         */
#define BTPS_MINIMUM_SCHEDULER_RESOLUTION                (0)

   /* Defines the maximum number of bytes that will be allocated by the */
   /* kernel abstraction module to support allocations.                 */
   /* * NOTE * This module declares a memory array of this size (in     */
   /*          bytes) that will be used by this module for memory       */
   /*          allocation.                                              */
#ifndef BTPS_MEMORY_BUFFER_SIZE

   #define BTPS_MEMORY_BUFFER_SIZE                       (2900)

#endif

   /* The following constant represents the maximum number of functions */
   /* that can be added to the scheduler.                               */
#ifndef MAX_NUMBER_SCHEDULE_FUNCTIONS
   
   #define MAX_NUMBER_SCHEDULE_FUNCTIONS                 (5)

#endif

   /* The following declared type represents the Prototype Function for */
   /* a function that should be registered with the BTPSKRNL module to  */
   /* retrieve the current Millisecond Tick Count.  This function will  */
   /* be called whenever BTPS_TickCount() is called and allows the main */
   /* application to control all timers within the application.         */
   /* * NOTE * This function can be registered by passing the address   */
   /*          of the implementation function in the                    */
   /*          GetTickCountCallback member of the                       */
   /*          BTPS_Initialization_t structure which is passed to the   */
   /*          BTPS_Init() function.  If no function is registered then */
   /*          the the system will not function as the scheduler cannot */
   /*          be executed because the tick count will never change.    */
   /* * NOTE * This function *MUST* be specified otherwise the system   */
   /*          will NOT function.                                       */
typedef unsigned long (BTPSAPI *BTPS_GetTickCountCallback_t)(void);

   /* The following declared type represents the Prototype Function for */
   /* a function that can be registered with the BTPSKRNL module to     */
   /* receive output characters.  This function will be called whenever */
   /* BTPS_OutputMessage() or BTPS_DumpData() is called (or if debug is */
   /* is enabled - DEBUG preprocessor symbol, whenever the DBG_MSG() or */
   /* DBG_DUMP() MACRO is used and there is debug data to output.       */
   /* * NOTE * This function can be registered by passing the address   */
   /*          of the implementation function in the                    */
   /*          MessageOutputCallback member of the                      */
   /*          BTPS_Initialization_t structure which is passed to the   */
   /*          BTPS_Init() function.  If no function is registered then */
   /*          there will be no output (i.e. it will simply be ignored).*/
typedef void (BTPSAPI *BTPS_MessageOutputCallback_t)(char DebugCharacter);

   /* The following structure represents the structure that is passed   */
   /* to the BTPS_Init() function to notify the Bluetooth abstraction   */
   /* layer of the function(s) that are required for proper device      */
   /* functionality.                                                    */
   /* * NOTE * This structure *MUST* be passed to the BTPS_Init()       */
   /*          function AND THE GetTickCountCallback MEMBER MUST BE     */
   /*          SPECIFIED.  Failure to provide this function will cause  */
   /*          the Bluetooth sub-system to not function because the     */
   /*          scheduler will not function (as the Tick Count will      */
   /*          never change).                                           */
typedef struct _tagBTPS_Initialization_t
{
   BTPS_GetTickCountCallback_t  GetTickCountCallback;
   BTPS_MessageOutputCallback_t MessageOutputCallback;
} BTPS_Initialization_t;

#define BTPS_INITIALIZATION_SIZE                         (sizeof(BTPS_Initialization_t))

#include "BKRNLAPI.h"           /* Bluetooth Kernel Prototypes/Constants.     */

#endif
