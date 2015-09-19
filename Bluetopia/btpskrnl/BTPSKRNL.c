/*****< btpskrnl.c >***********************************************************/
/*      Copyright 2000 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPSKRNL - Stonestreet One Bluetooth Stack Kernel Implementation.         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/30/01  D. Lange       Initial creation.                               */
/*   12/18/01  R. Sledge      Port to Linux                                   */
/*   02/05/08  D. Wooldridge  Incorporate functions security module requires  */
/******************************************************************************/
#include "../../Hardware/msp430/HRDWCFG.h"              /* SS1 MSP430 Hardware Configuration Consts.*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "BTPSKRNL.h"             /* BTPS Kernel Prototypes/Constants.        */

   /* The following constant represents the number of bytes that are    */
   /* displayed on an individual line when using the DumpData()         */
   /* function.                                                         */
#define MAXIMUM_BYTES_PER_ROW                          (16)

   /* The following MACRO maps Timeouts (specified in Milliseconds) to  */
   /* System Ticks that are required by the Operating System Timeout    */
   /* functions (Waiting only).                                         */
#define MILLISECONDS_TO_TICKS(_x)                      ((_x) / (MSP430_TICK_RATE_MS))

   /* The following MACRO maps System Ticks to Milliseconds.            */
#define TICKS_TO_MILLISECONDS(_x)                      ((_x) *  (MSP430_TICK_RATE_MS))

   /* The following type declaration represents an individual Scheduler */
   /* Function Entry.  This Entry contains all information needed to    */
   /* Schedule and Execute a Function that has been added to the        */
   /* Scheduler.                                                        */
typedef struct _tagSchedulerInformation_t
{
   unsigned long             ScheduleCount;
   unsigned long             ScheduleExpireCount;
   BTPS_SchedulerFunction_t  ScheduleFunction;
   void                     *ScheduleParameter;
} SchedulerInformation_t;

   /* The following type declaration represents the entire state        */
   /* information for a Mailbox.  This structure is used with all of    */
   /* the Mailbox functions contained in this module.                   */
typedef struct _tagMailboxHeader_t
{
   unsigned int  HeadSlot;
   unsigned int  TailSlot;
   unsigned int  OccupiedSlots;
   unsigned int  NumberSlots;
   unsigned int  SlotSize;
   void         *Slots;
} MailboxHeader_t;

   /* The following defines a type that is the size in bytes of the     */
   /* desired alignment of each datra fragment.                         */
typedef unsigned int Alignment_t;

   /* The following defines the byte boundary size that has been        */
   /* specified size if the alignment data.                             */
#define ALIGNMENT_SIZE          sizeof(Alignment_t)

   /* The following structure is used to allign data fragments on a     */
   /* specified memory boundary.                                        */
typedef union _tagAlignmentStruct_t
{
   Alignment_t   AlignmentValue;
   unsigned char ByteValue;
} AlignmentStruct_t;

   /* The following defines the structure that describes a memory       */
   /* fragment.                                                         */
typedef struct _tagHeapInfo_t
{
   Word_t            PrevSize;
   Word_t            Size;
   AlignmentStruct_t Data;
} HeapInfo_t;

#define HEAP_INFO_DATA_SIZE(_x) ((BTPS_STRUCTURE_OFFSET(HeapInfo_t, Data) / ALIGNMENT_SIZE) + (_x))

#define SEGMENT_ALLOCATED_BITMASK (0x8000)
#define SEGMENT_SIZE_BITMASK      (0x7FFF)

   /* The following defines the size in bytes of a data fragment that is*/
   /* considered a large value.  Allocations that are equal to and      */
   /* larger than this value will be allocated from the end of the      */
   /* buffer.                                                           */
#define LARGE_SIZE              (256/ALIGNMENT_SIZE)

   /* The following defines the minimum size (in alignment units) of a  */
   /* fragment that is considered useful.  The value is used when trying*/
   /* to determine if a fragment that is larger than the requested size */
   /* can be broken into 2 framents leaving a fragment that is of the   */
   /* requested size and one that is at least as larger as the          */
   /* MINIMUM_MEMORY_SIZE.                                              */
#define MINIMUM_MEMORY_SIZE     1

   /* Declare a buffer to use for the Heap.  Note that we declare this  */
   /* as an unsigned long buffer so that we can force alignment to be   */
   /* correct.                                                          */
static Alignment_t MemoryBuffer[(BTPS_MEMORY_BUFFER_SIZE/ALIGNMENT_SIZE) + 1];

static HeapInfo_t *HeapHead = NULL;
static HeapInfo_t *HeapTail = NULL;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */
static Boolean_t              SchedulerInitialized; /* Flag which flags       */
                                                /* whether or not the         */
                                                /* Scheduler has been         */
                                                /* successfully Intitalized.  */

static unsigned int           NumberScheduledFunctions; /* Variable which     */
                                                /* holds the total number of  */
                                                /* Functions that have been   */
                                                /* added to the Scheduler.    */

static SchedulerInformation_t SchedulerInformation[MAX_NUMBER_SCHEDULE_FUNCTIONS];
                                                /* Variable which holds ALL   */
                                                /* Information regarding ALL  */
                                                /* Scheduled Functions.       */

static unsigned long          DebugZoneMask = DEBUG_ZONES; /* Variable which  */
                                                /* holds the current Debug    */
                                                /* Zone Mask.                 */

static char                   DebugMsgBuffer[MAX_DEBUG_MSG_LENGTH];

static BTPS_GetTickCountCallback_t  GetTickCountCallback;  /* Variable which  */
                                                /* holds the currently        */
                                                /* registered function that   */
                                                /* is to be called when this  */
                                                /* module needs to know the   */
                                                /* current value of the       */
                                                /* milli-second Tick Count of */
                                                /* the system.                */

static BTPS_MessageOutputCallback_t MessageOutputCallback; /* Variable which  */
                                                /* holds the currently        */
                                                /* registered function that   */
                                                /* is to be called when there */
                                                /* is a character of an output*/
                                                /* message that is to be      */
                                                /* displayed.  This value is  */
                                                /* set via a call to the      */
                                                /* BTPS_Init() function.      */

static unsigned long PreviousTickCount;         /* Variable which holds the   */
                                                /* previous Tick Count that   */
                                                /* was present through the    */
                                                /* last pass through the      */
                                                /* scheduler.                 */
      /* Internal Function Prototypes.                                  */
static Byte_t ConsoleWrite(char *Message, int Length);
static void CalcTotals(unsigned int *Used, unsigned int *Free, unsigned int *MaxFree);
static void  HeapInit(void);
static void *_Malloc(unsigned long Size);
static void _MemFree(void *MemoryPtr);

   /* The following function is used to send a string of characters to  */
   /* the Console or Output device.  The function takes as its first    */
   /* parameter a pointer to a string of characters to be output.  The  */
   /* second parameter indicates the number of bytes in the character   */
   /* string that is to be output.                                      */
static Byte_t ConsoleWrite(char *Message, int Length)
{
   char ch = '\0';

   /* Check to make sure that the Debug String appears to be semi-valid.*/
   if((Message) && (Length) && (MessageOutputCallback))
   {
      while(*Message)
      {
         ch = *Message++;
         MessageOutputCallback(ch);
      }
   }

   return(0);
}

   /* The following is a utility function that can calculate the current*/
   /* memory usage.  The function takes as its first parameter a pointer*/
   /* to receive the number of bytes currently allocated and in use.    */
   /* The second parameter is a pointer to a value that will receive the*/
   /* number of unallocated byte that are free for use.  The third      */
   /* parameter is a pointer to receive the largest fragment that is    */
   /* free to be used.                                                  */
static void CalcTotals(unsigned int *Used, unsigned int *Free, unsigned int *MaxFree)
{
   HeapInfo_t *HeapInfoPtr;
   Word_t      FreeSegments;
   Word_t      AllocSegments;

   /* Verify that the heap has been initialized.                        */
   if(!HeapHead)
      HeapInit();

   /* Verify that the parameters that were passed in appear valid.      */
   if((Used) && (Free) && (MaxFree) && (HeapHead))
   {
      /* Initialize the return values.                                  */
      *Used         = 0;
      *Free         = 0;
      *MaxFree      = 0;
      FreeSegments  = 0;
      AllocSegments = 0;
      HeapInfoPtr   = HeapHead;

      do
      {
         /* Check to see if the current fragment is marked as Free.     */
         if(HeapInfoPtr->Size & SEGMENT_ALLOCATED_BITMASK)
         {
            AllocSegments++;

            *Used += (HeapInfoPtr->Size & SEGMENT_SIZE_BITMASK) * ALIGNMENT_SIZE;
         }
         else
         {
            FreeSegments++;

            /* Accumulate the total number of Free Bytes.               */
            *Free += HeapInfoPtr->Size * ALIGNMENT_SIZE;

            /* Check to see if the current fragment is larger that any  */
            /* we have seen and update the Max Value if it is larger.   */
            if(HeapInfoPtr->Size > *MaxFree)
               *MaxFree = HeapInfoPtr->Size * ALIGNMENT_SIZE;
         }

         BTPS_OutputMessage("(%u %u)%c \r\n", HeapInfoPtr->PrevSize, (HeapInfoPtr->Size & SEGMENT_SIZE_BITMASK), (HeapInfoPtr->Size & SEGMENT_ALLOCATED_BITMASK)?'U':'F');

         /* Adjuse the pointer to the next entry.                       */
         HeapInfoPtr = (HeapInfo_t *)(((Alignment_t *)HeapInfoPtr) + (HeapInfoPtr->Size & SEGMENT_SIZE_BITMASK));

      } while(HeapInfoPtr != HeapTail);

      BTPS_OutputMessage("\r\nSegments (Alloc, Free) = (%u, %u)\r\n", AllocSegments, FreeSegments);
   }
}

   /* The following function is used to initialize the heap structure.  */
   /* The function takes no parameters and returns no status.           */
static void HeapInit(void)
{
   DWord_t HeapSize;

   /* Verify that the heap info structure is properly aligned.          */
   if((BTPS_STRUCTURE_OFFSET(HeapInfo_t, Data) % ALIGNMENT_SIZE) == 0)
   {
      /* Calculate the size of the heap in alignment units.             */
      HeapSize = sizeof(MemoryBuffer) / ALIGNMENT_SIZE;

      /* Verify that the heap is not bigger than the maximum segment    */
      /* length.                                                        */
      if(HeapSize <= SEGMENT_SIZE_BITMASK)
      {
         /* Initialize the heap.                                        */
         HeapHead           = (HeapInfo_t *)MemoryBuffer;
         HeapHead->PrevSize = HeapSize;
         HeapHead->Size     = HeapSize;

         HeapTail           = (HeapInfo_t *)(MemoryBuffer + HeapSize);
      }
   }
}

   /* The following function is used to allocate a fragment of memory   */
   /* from a large buffer.  The function takes as its parameter the size*/
   /* in bytes of the fragment to be allocated.  The function tries to  */
   /* avoid fragmentation by obtaining memory requests larger than      */
   /* LARGE_SIZE from the end of the buffer, while small fragments are  */
   /* taken from the start of the buffer.                               */
static void *_Malloc(unsigned long Size)
{
   HeapInfo_t *HeapInfoPtr;
   HeapInfo_t *TmpInfoPtr;
   Word_t      TmpSize;
   void       *ret_val;

   /* Convert the requested memory allocation in bytes to alignment     */
   /* size.                                                             */
   if(Size % ALIGNMENT_SIZE)
      Size = (Size / ALIGNMENT_SIZE) + 1;
   else
      Size /= ALIGNMENT_SIZE;

   /* Add the header size to the requested size.                        */
   Size += HEAP_INFO_DATA_SIZE(0);

   /* Verify that the requested size is valid                           */
   if((Size >= HEAP_INFO_DATA_SIZE(1)) && (!(Size & SEGMENT_ALLOCATED_BITMASK)))
   {
      /* Verify that the heap has been initialized.                     */
      if(!HeapHead)
         HeapInit();

      /* Verify that the heap is valid.                                 */
      if(HeapHead)
      {
         /* If we are allocating a large segment, then start at the end */
         /* of the heap.  Otherwise, start at the beginning of the heap.*/
         /* If there is only one segment in the heap, then HeapHead will*/
         /* be used either way.                                         */
         if(Size >= LARGE_SIZE)
            HeapInfoPtr = (HeapInfo_t *)(((Alignment_t *)HeapTail) - HeapHead->PrevSize);
         else
            HeapInfoPtr = HeapHead;

         /* Loop until we have walked the entire list.                  */
         while(((Size < LARGE_SIZE) || (HeapInfoPtr != HeapHead)) && (HeapInfoPtr != HeapTail))
         {
            /* Check to see if the current entry is free and is large   */
            /* enough to hold the data being requested.                 */
            if((HeapInfoPtr->Size & SEGMENT_ALLOCATED_BITMASK) || (HeapInfoPtr->Size < Size))
            {
               /* If the requested size is larger than the limit then   */
               /* search backwards for an available buffer, else go     */
               /* forward.  This will hopefully help to reduce          */
               /* fragmentataion problems.                              */
               if(Size >= LARGE_SIZE)
                  HeapInfoPtr = (HeapInfo_t *)(((Alignment_t *)HeapInfoPtr) - (HeapInfoPtr->PrevSize));
               else
                  HeapInfoPtr = (HeapInfo_t *)(((Alignment_t *)HeapInfoPtr) + (HeapInfoPtr->Size & SEGMENT_SIZE_BITMASK));
            }
            else
               break;
         }

         /* Check to see if we found a segment large enough for the     */
         /* request.                                                    */
         if((HeapInfoPtr != HeapTail) && (!(HeapInfoPtr->Size & SEGMENT_ALLOCATED_BITMASK)) && (HeapInfoPtr->Size >= Size))
         {
            /* Check to see if we need to split this into two entries.  */
            /* * NOTE * If there is not enough room to make another     */
            /*          entry then we will not adjust the size of this  */
            /*          entry to match the amount requested.            */
            if(HeapInfoPtr->Size >= (Size + HEAP_INFO_DATA_SIZE(MINIMUM_MEMORY_SIZE)))
            {
               /* Calculate the size for the free segment.              */
               TmpSize = HeapInfoPtr->Size - Size;

               /* If this is a large segment allocation, then split the */
               /* segment so that the free segment is at the beginning. */
               if(Size >= LARGE_SIZE)
               {
                  /* Calculate the pointer to the allocated segment.    */
                  TmpInfoPtr = (HeapInfo_t *)(((Alignment_t *)HeapInfoPtr) + TmpSize);

                  /* Set the previous size and size values for the      */
                  /* allocated segment.                                 */
                  TmpInfoPtr->PrevSize = TmpSize;
                  TmpInfoPtr->Size     = Size | SEGMENT_ALLOCATED_BITMASK;

                  /* Set the new size of the free segment.              */
                  HeapInfoPtr->Size = TmpSize;

                  /* Set the pointer to the beginning of the newly      */
                  /* allocated segment                                  */
                  HeapInfoPtr = TmpInfoPtr;

                  /* Save the size of the allocated segment so that the */
                  /* next segment can be updated.                       */
                  TmpSize = Size;
               }
               else
               {
                  /* Calculate the pointer to the free segment.         */
                  TmpInfoPtr = (HeapInfo_t *)(((Alignment_t *)HeapInfoPtr) + Size);

                  /* Set the previous size and size values for the free */
                  /* segment.                                           */
                  TmpInfoPtr->PrevSize = Size;
                  TmpInfoPtr->Size     = TmpSize;

                  /* Set the new size of the allocated segment and      */
                  /* indicate that it is no longer free.                */
                  HeapInfoPtr->Size = Size | SEGMENT_ALLOCATED_BITMASK;
               }

               /* Calculate the pointer to the next segment.            */
               TmpInfoPtr = (HeapInfo_t *)(((Alignment_t *)TmpInfoPtr) + TmpSize);

               /* Check for a wrap condition and update the next        */
               /* segment's PrevSize field.                             */
               if(TmpInfoPtr == HeapTail)
               {
                  HeapHead->PrevSize = TmpSize;
               }
               else
               {
                  TmpInfoPtr->PrevSize = TmpSize;
               }
            }
            else
            {
               /* Mark the buffer as In Use.                            */
               HeapInfoPtr->Size |= SEGMENT_ALLOCATED_BITMASK;
            }

            /* Get the address of the start of RAM.                     */
            ret_val = (void *)&HeapInfoPtr->Data;
         }
         else
         {
            DBG_MSG(DBG_ZONE_BTPSKRNL, ("NO SEGMENT FOUND.\r\n"));

            /* There is not a segment large enough to satisfy this      */
            /* request.                                                 */
            ret_val = NULL;
         }
      }
      else
      {
         /* The heap could not be initialized.                          */
         DBG_MSG(DBG_ZONE_BTPSKRNL, ("HEAP HEAD NULL.\r\n"));

         ret_val = NULL;
      }
   }
   else
   {
      DBG_MSG(DBG_ZONE_BTPSKRNL, ("INVALID SIZE %04x.\r\n", Size));

      /* The requested size is invalid.                                 */
      ret_val = NULL;
   }

   return(ret_val);
}

   /* The following function is used to free memory that was previously */
   /* allocated with _Malloc.  The function takes as its parameter a    */
   /* pointer to the memory that was allocated.  The pointer is used to */
   /* locate the structure of information that describes the allocated  */
   /* fragment.  The function tries to a verify that the structure is a */
   /* valid fragment structure before the memory is freed.  When a      */
   /* fragment is freed, it may be combined with adjacent fragments to  */
   /* produce a larger free fragment.                                   */
static void _MemFree(void *MemoryPtr)
{
   HeapInfo_t *HeapInfoPtr;
   HeapInfo_t *TmpInfoPtr;

   /* Verify that the parameter passed in appears valid.                */
   if((MemoryPtr) && (HeapHead) && (((HeapInfo_t *)MemoryPtr) >= HeapHead) && (((HeapInfo_t *)MemoryPtr) < HeapTail))
   {
      /* Get a pointer to the Heap Info.                                */
      HeapInfoPtr = (HeapInfo_t *)(((Alignment_t *)MemoryPtr) - HEAP_INFO_DATA_SIZE(0));

      /* Verify that this segment is allocated.                         */
      if(HeapInfoPtr->Size & SEGMENT_ALLOCATED_BITMASK)
      {
         /* Mask out the allocation bit of the segment to be freed.     */
         /* This will make calculations in this block easier.           */
         HeapInfoPtr->Size &= SEGMENT_SIZE_BITMASK;

         /* If the segment to be freed is at the head of the heap, then */
         /* we do not have to merge or update any sizes of the previous */
         /* segment.  This will also handle the case where the entire   */
         /* heap has been allocated to one segment.                     */
         if(HeapInfoPtr != HeapHead)
         {
            /* Calculate the pointer to the previous segment.           */
            TmpInfoPtr = (HeapInfo_t *)(((Alignment_t *)HeapInfoPtr) - HeapInfoPtr->PrevSize);

            /* Check to see if the previous segment can be combined.    */
            if(!(TmpInfoPtr->Size & SEGMENT_ALLOCATED_BITMASK))
            {
               /* Add the segment to be freed to the new header.        */
               TmpInfoPtr->Size += HeapInfoPtr->Size;

               /* Set the pointer to the beginning of the previous free */
               /* segment.                                              */
               HeapInfoPtr = TmpInfoPtr;
            }
         }

         /* Calculate the pointer to the next segment.                  */
         TmpInfoPtr = (HeapInfo_t *)(((Alignment_t *)HeapInfoPtr) + HeapInfoPtr->Size);

         /* If we are pointing at the end of the heap, then use the head*/
         /* as the next segment.                                        */
         if(TmpInfoPtr == HeapTail)
         {
            /* We can't combine the head with the tail, so just update  */
            /* the PrevSize field.                                      */
            HeapHead->PrevSize = HeapInfoPtr->Size;
         }
         else
         {
            /* We are not pointing to the end of the heap, so if the    */
            /* next segment is allocated then update the PrevSize field.*/
            if(TmpInfoPtr->Size & SEGMENT_ALLOCATED_BITMASK)
            {
               TmpInfoPtr->PrevSize = HeapInfoPtr->Size;
            }
            else
            {
               /* The next segment is free, so merge it with the current*/
               /* segment.                                              */
               HeapInfoPtr->Size += TmpInfoPtr->Size;

               /* Since we merged the next segment, we have to update   */
               /* the next next segment's PrevSize field.               */
               TmpInfoPtr =  (HeapInfo_t *)(((Alignment_t *)HeapInfoPtr) + HeapInfoPtr->Size);

               /* If we are pointing at the end of the heap, then use   */
               /* the head as the next next segment.                    */
               if(TmpInfoPtr == HeapTail)
               {
                  HeapHead->PrevSize = HeapInfoPtr->Size;
               }
               else
               {
                  TmpInfoPtr->PrevSize = HeapInfoPtr->Size;
               }
            }
         }
      }
      else
      {
         DBG_MSG(DBG_ZONE_BTPSKRNL, ("SEGMENT ALREADY FREE.\r\n"));
      }
   }
   else
   {
      DBG_MSG(DBG_ZONE_BTPSKRNL, ("INVALID POINTER %p %p %p\r\n", HeapHead, MemoryPtr, HeapTail));
   }
}

   /* The following function is responsible for the Memory Usage        */
   /* Information.  This function accepts as input the Memory Pool Usage*/
   /* Length and a pointer to an Buffer of Memory Pool Usage structures.*/
   /* The third parameter to this function is a pointer to a unsigned   */
   /* int which will passed the Total Buffer Size.  The fourth parameter*/
   /* to this function is a pointer to a unsigned int which will be     */
   /* passed the Memory Buffer Usage.                                   */
int BTPSAPI BTPS_QueryMemoryUsage(unsigned int *Used, unsigned int *Free, unsigned int *MaxFree)
{
   CalcTotals(Used, Free, MaxFree);

   return(0);
}

   /* The following function is responsible for delaying the current    */
   /* task for the specified duration (specified in Milliseconds).      */
   /* * NOTE * Very small timeouts might be smaller in granularity than */
   /*          the system can support !!!!                              */
void BTPSAPI BTPS_Delay(unsigned long MilliSeconds)
{
   unsigned long DelayCount;
   unsigned long msTickCount;

   msTickCount = BTPS_GetTickCount();

   DelayCount = msTickCount + MILLISECONDS_TO_TICKS(MilliSeconds);

   while(msTickCount < DelayCount)
      msTickCount = BTPS_GetTickCount();
}

   /* The following function is responsible for retrieving the current  */
   /* Tick Count of system.  This function returns the System Tick      */
   /* Count in System Tick Count resolution.                            */
   /* * NOTE * The System Tick Count is defined in Milliseconds.  The   */
   /*          constant BTPS_TICK_COUNT_INTERVAL can be used to         */
   /*          to programmatically determine elapsed time.              */
unsigned long BTPSAPI BTPS_GetTickCount(void)
{
   /* Simply wrap the OS supplied GetTickCount function.                */
   if(GetTickCountCallback)
      return((*GetTickCountCallback)());
   else
      return(0);
}

   /* The following function is provided to allow a mechanism for       */
   /* adding Scheduler Functions to the Scheduler.  These functions are */
   /* called periodically by the Scheduler (based upon the requested    */
   /* Schedule Period).  This function accepts as input the Scheduler   */
   /* Function to add to the Scheduler, the Scheduler parameter that is */
   /* passed to the Scheduled function, and the Scheduler Period.  The  */
   /* Scheduler Period is specified in Milliseconds.  This function     */
   /* returns TRUE if the function was added successfully or FALSE if   */
   /* there was an error.                                               */
   /* * NOTE * Once a function is added to the Scheduler, it can only   */
   /*          be removed by calling the                                */
   /*          BTPS_DeleteFunctionFromScheduler() function.             */
   /* * NOTE * The BTPS_ExecuteScheduler() function *MUST* be called    */
   /*          ONCE (AND ONLY ONCE) to begin the Scheduler Executing    */
   /*          periodic Scheduled functions (or calling the             */
   /*          BTPS_ProcessScheduler() function repeatedly.             */
Boolean_t BTPSAPI BTPS_AddFunctionToScheduler(BTPS_SchedulerFunction_t SchedulerFunction, void *SchedulerParameter, unsigned int Period)
{
   Boolean_t ret_val;

   /* First, let's make sure that the Scheduler has been initialized    */
   /* successfully AND that the Scheduler is NOT full.                  */
   if((SchedulerInitialized) && (NumberScheduledFunctions != MAX_NUMBER_SCHEDULE_FUNCTIONS))
   {
      /* Next, let's make sure that the Scheduled Function specified    */
      /* appears to be semi-valid.                                      */
      if(SchedulerFunction)
      {
         /* Simply add the Scheduled Function into the Next available   */
         /* Schedule Slot.                                              */
         SchedulerInformation[NumberScheduledFunctions].ScheduleCount     = 0;
         SchedulerInformation[NumberScheduledFunctions].ScheduleFunction  = SchedulerFunction;
         SchedulerInformation[NumberScheduledFunctions].ScheduleParameter = SchedulerParameter;

#if BTPS_MINIMUM_SCHEDULER_RESOLUTION

         if((SchedulerInformation[NumberScheduledFunctions].ScheduleExpireCount = Period) < BTPS_MINIMUM_SCHEDULER_RESOLUTION)
            SchedulerInformation[NumberScheduledFunctions].ScheduleExpireCount = BTPS_MINIMUM_SCHEDULER_RESOLUTION;

#else

         SchedulerInformation[NumberScheduledFunctions].ScheduleExpireCount = Period;

#endif

         /* Update the total number of Functions that have been added to*/
         /* the Scheduler.                                              */
         NumberScheduledFunctions++;

         /* Finally return success to the caller.                       */
         ret_val = TRUE;
      }
      else
         ret_val = FALSE;
   }
   else
      ret_val = FALSE;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* deleting a Function that has previously been registered with the  */
   /* Scheduler via a successful call to the                            */
   /* BTPS_AddFunctionToScheduler() function.  This function accepts as */
   /* input the Scheduler Function to that was added to the Scheduler,  */
   /* as well as the Scheduler Parameter that was registered.  Both of  */
   /* these values *must* match to remove a specific Scheduler Entry.   */
void BTPSAPI BTPS_DeleteFunctionFromScheduler(BTPS_SchedulerFunction_t SchedulerFunction, void *SchedulerParameter)
{
   unsigned int Index;

   /* First, let's make sure that the Scheduler has been initialized    */
   /* successfully AND that the Scheduler is NOT full.                  */
   if(SchedulerInitialized)
   {
      /* Next, let's make sure that the Scheduled Function specified    */
      /* appears to be semi-valid.                                      */
      if(SchedulerFunction)
      {
         /* Loop through the scheduler and remove the function (if we   */
         /* find it).                                                   */
         for(Index=0;Index<NumberScheduledFunctions;Index++)
         {
            if((SchedulerInformation[Index].ScheduleFunction == SchedulerFunction) && (SchedulerInformation[Index].ScheduleParameter == SchedulerParameter))
               break;
         }

         /* Check to see if we have found the scheduled function.  Also,*/
         /* if we have we need to copy all the functions in the         */
         /* scheduler that exist AFTER the current index into the       */
         /* Scheduler array starting at the index that was found above. */
         if(Index < NumberScheduledFunctions)
         {
            /* Simply start copying from the next location back one in  */
            /* the array.                                               */
            for(Index++;Index<NumberScheduledFunctions;Index++)
            {
               SchedulerInformation[Index - 1].ScheduleCount       = SchedulerInformation[Index].ScheduleCount;
               SchedulerInformation[Index - 1].ScheduleFunction    = SchedulerInformation[Index].ScheduleFunction;
               SchedulerInformation[Index - 1].ScheduleParameter   = SchedulerInformation[Index].ScheduleParameter;
               SchedulerInformation[Index - 1].ScheduleExpireCount = SchedulerInformation[Index].ScheduleExpireCount;
            }

            /* Update the total number of Functions that have been added*/
            /* to the Scheduler.                                        */
            NumberScheduledFunctions--;
         }
      }
   }
}

   /* The following function begins execution of the actual Scheduler.  */
   /* Once this function is called, it NEVER returns.  This function is */
   /* responsible for executing all functions that have been added to   */
   /* the Scheduler with the BTPS_AddFunctionToScheduler() function.    */
void BTPSAPI BTPS_ExecuteScheduler(void)
{
   Boolean_t Done;

   /* Initialize the Scheduler state information.                       */
   Done = FALSE;

   /* Simply loop until Done (Forever), calling each scheduled          */
   /* function when it expires.                                         */
   while(!Done)
   {
      /* Simply process the scheduler.                                  */
      BTPS_ProcessScheduler();
   }
}

   /* The following function is provided to allow a mechanism to process*/
   /* the scheduled functions in the scheduler.  This function performs */
   /* the same functionality as the BTPS_ExecuteScheduler() function    */
   /* except that it returns as soon as it has made an iteration through*/
   /* the scheduled functions.  This function is provided for platforms */
   /* that would like to implement their own processing loop and/or     */
   /* scheduler and not rely on the Bluetopia implementation.           */
   /* * NOTE * This function should NEVER be called if the              */
   /*          BTPS_ExecuteScheduler() schema is used.                  */
   /* * NOTE * This function *MUST* not be called from any scheduled    */
   /*          function that was added to the scheduler via the         */
   /*          BTPS_AddFunctionToScheduler() function or an infinite    */
   /*          loop will occur.                                         */
void BTPSAPI BTPS_ProcessScheduler(void)
{
   unsigned int  SchedulerPointer;
   unsigned long ElapsedTicks;
   unsigned long CurrentTickCount;

   /* First, let's calculate the Elapsed Number of Ticks that have      */
   /* occurred since the last time through the scheduler (taking into   */
   /* account the possibility that the Tick Counter could have wrapped).*/
   CurrentTickCount = BTPS_GetTickCount();

   ElapsedTicks = CurrentTickCount - PreviousTickCount;

   if(ElapsedTicks & 0x80000000)
      ElapsedTicks = CurrentTickCount + (0xFFFFFFFF - PreviousTickCount) + 1;

#if BTPS_MINIMUM_SCHEDULER_RESOLUTION

   if(ElapsedTicks)

#else

   if(1)

#endif
   {
      /* Now that we have calculated the Elapsed Time, let's increment  */
      /* the the Scheduler Count for each Scheduled Function by the     */
      /* Elapsed Tick Amount.                                           */
      SchedulerPointer = 0;
      while(SchedulerPointer < NumberScheduledFunctions)
      {
         SchedulerInformation[SchedulerPointer].ScheduleCount += ElapsedTicks;

         if(SchedulerInformation[SchedulerPointer].ScheduleCount >= SchedulerInformation[SchedulerPointer].ScheduleExpireCount)
         {
            /* Simply call the Scheduled function.                      */
            (*(BTPS_SchedulerFunction_t)(SchedulerInformation[SchedulerPointer].ScheduleFunction))(SchedulerInformation[SchedulerPointer].ScheduleParameter);

            /* Reset the current Schedule Count back to zero.           */
            SchedulerInformation[SchedulerPointer].ScheduleCount = 0;
         }

         SchedulerPointer++;
      }

      /* Note the last time that we processed the scheduler.            */
      PreviousTickCount = CurrentTickCount;
   }
}

   /* The following function is provided to allow a mechanism to        */
   /* actually allocate a Block of Memory (of at least the specified    */
   /* size).  This function accepts as input the size (in Bytes) of the */
   /* Block of Memory to be allocated.  This function returns a NON-NULL*/
   /* pointer to this Memory Buffer if the Memory was successfully      */
   /* allocated, or a NULL value if the memory could not be allocated.  */
void BTPSAPI *BTPS_AllocateMemory(unsigned long MemorySize)
{
   void  *ret_val;

   /* Next make sure that the caller is actually requesting memory to be*/
   /* allocated.                                                        */
   if(MemorySize)
   {
      ret_val = _Malloc(MemorySize);
      if(!ret_val)
         BTPS_OutputMessage("Alloc Failed: %d\r\n", MemorySize);
     // else
    //	 BTPS_OutputMessage("Memory Allocated: %d\r\n", MemorySize);
   }
   else
   {
      DBG_MSG(DBG_ZONE_BTPSKRNL, ("Invalid size\r\n"));
      ret_val = NULL;
   }

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for de-allocating a Block   */
   /* of Memory that was successfully allocated with the                */
   /* BTPS_AllocateMemory() function.  This function accepts a NON-NULL */
   /* Memory Pointer which was returned from the BTPS_AllocateMemory()  */
   /* function.  After this function completes the caller CANNOT use    */
   /* ANY of the Memory pointed to by the Memory Pointer.               */
void BTPSAPI BTPS_FreeMemory(void *MemoryPointer)
{
   /* First make sure that the memory being returned is semi-valid.     */
   if(MemoryPointer)
      _MemFree(MemoryPointer);
   else
      DBG_MSG(DBG_ZONE_BTPSKRNL,("Invalid Pointer\r\n"));
}

   /* The following function is responsible for copying a block of      */
   /* memory of the specified size from the specified source pointer    */
   /* to the specified destination memory pointer.  This function       */
   /* accepts as input a pointer to the memory block that is to be      */
   /* Destination Buffer (first parameter), a pointer to memory block   */
   /* that points to the data to be copied into the destination buffer, */
   /* and the size (in bytes) of the Data to copy.  The Source and      */
   /* Destination Memory Buffers must contain AT LEAST as many bytes    */
   /* as specified by the Size parameter.                               */
   /* * NOTE * This function does not allow the overlapping of the      */
   /*          Source and Destination Buffers !!!!                      */
void BTPSAPI BTPS_MemCopy(void *Destination, BTPSCONST void *Source, unsigned long Size)
{
   /* Simply wrap the C Run-Time memcpy() function.                     */
   memcpy(Destination, Source, Size);
}

   /* The following function is responsible for moving a block of       */
   /* memory of the specified size from the specified source pointer    */
   /* to the specified destination memory pointer.  This function       */
   /* accepts as input a pointer to the memory block that is to be      */
   /* Destination Buffer (first parameter), a pointer to memory block   */
   /* that points to the data to be copied into the destination buffer, */
   /* and the size (in bytes) of the Data to copy.  The Source and      */
   /* Destination Memory Buffers must contain AT LEAST as many bytes    */
   /* as specified by the Size parameter.                               */
   /* * NOTE * This function DOES allow the overlapping of the          */
   /*          Source and Destination Buffers.                          */
void BTPSAPI BTPS_MemMove(void *Destination, BTPSCONST void *Source, unsigned long Size)
{
   /* Simply wrap the C Run-Time memmove() function.                    */
   memmove(Destination, Source, Size);
}

   /* The following function is provided to allow a mechanism to fill   */
   /* a block of memory with the specified value.  This function accepts*/
   /* as input a pointer to the Data Buffer (first parameter) that is   */
   /* to filled with the specified value (second parameter).  The       */
   /* final parameter to this function specifies the number of bytes    */
   /* that are to be filled in the Data Buffer.  The Destination        */
   /* Buffer must point to a Buffer that is AT LEAST the size of        */
   /* the Size parameter.                                               */
void BTPSAPI BTPS_MemInitialize(void *Destination, unsigned char Value, unsigned long Size)
{
   /* Simply wrap the C Run-Time memset() function.                     */
   memset(Destination, Value, Size);
}

   /* The following function is provided to allow a mechanism to        */
   /* Compare two blocks of memory to see if the two memory blocks      */
   /* (each of size Size (in bytes)) are equal (each and every byte up  */
   /* to Size bytes).  This function returns a negative number if       */
   /* Source1 is less than Source2, zero if Source1 equals Source2, and */
   /* a positive value if Source1 is greater than Source2.              */
int BTPSAPI BTPS_MemCompare(BTPSCONST void *Source1, BTPSCONST void *Source2, unsigned long Size)
{
   /* Simply wrap the C Run-Time memcmp() function.                     */
   return(memcmp(Source1, Source2, Size));
}

   /* The following function is provided to allow a mechanism to Compare*/
   /* two blocks of memory to see if the two memory blocks (each of size*/
   /* Size (in bytes)) are equal (each and every byte up to Size bytes) */
   /* using a Case-Insensitive Compare.  This function returns a        */
   /* negative number if Source1 is less than Source2, zero if Source1  */
   /* equals Source2, and a positive value if Source1 is greater than   */
   /* Source2.                                                          */
int BTPSAPI BTPS_MemCompareI(BTPSCONST void *Source1, BTPSCONST void *Source2, unsigned long Size)
{
   int           ret_val = 0;
   unsigned char Byte1;
   unsigned char Byte2;
   unsigned int  Index;

   /* Simply loop through each byte pointed to by each pointer and check*/
   /* to see if they are equal.                                         */
   for (Index=0;((Index<Size) && (!ret_val));Index++)
   {
      /* Note each Byte that we are going to compare.                   */
      Byte1 = ((unsigned char *)Source1)[Index];
      Byte2 = ((unsigned char *)Source2)[Index];

      /* If the Byte in the first array is lower case, go ahead and make*/
      /* it upper case (for comparisons below).                         */
      if ((Byte1 >= 'a') && (Byte1 <= 'z'))
         Byte1 = Byte1 - ('a' - 'A');

      /* If the Byte in the second array is lower case, go ahead and    */
      /* make it upper case (for comparisons below).                    */
      if ((Byte2 >= 'a') && (Byte2 <= 'z'))
         Byte2 = Byte2 - ('a' - 'A');

      /* If the two Bytes are equal then there is nothing to do.        */
      if (Byte1 != Byte2)
      {
         /* Bytes are not equal, so set the return value accordingly.   */
         if (Byte1 < Byte2)
            ret_val = -1;
         else
            ret_val = 1;
      }
   }

   /* Simply return the result of the above comparison(s).              */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* copy a source NULL Terminated ASCII (character) String to the     */
   /* specified Destination String Buffer.  This function accepts as    */
   /* input a pointer to a buffer (Destination) that is to receive the  */
   /* NULL Terminated ASCII String pointed to by the Source parameter.  */
   /* This function copies the string byte by byte from the Source      */
   /* to the Destination (including the NULL terminator).               */
void BTPSAPI BTPS_StringCopy(char *Destination, BTPSCONST char *Source)
{
   /* Simply wrap the C Run-Time strcpy() function.                     */
   strcpy(Destination, Source);
}

   /* The following function is provided to allow a mechanism to        */
   /* determine the Length (in characters) of the specified NULL        */
   /* Terminated ASCII (character) String.  This function accepts as    */
   /* input a pointer to a NULL Terminated ASCII String and returns     */
   /* the number of characters present in the string (NOT including     */
   /* the terminating NULL character).                                  */
unsigned int BTPSAPI BTPS_StringLength(BTPSCONST char *Source)
{
   /* Simply wrap the C Run-Time strlen() function.                     */
   return(strlen(Source));
}

   /* The following function is provided to allow a mechanism to create */
   /* a Mailbox.  A Mailbox is a Data Store that contains slots (all    */
   /* of the same size) that can have data placed into (and retrieved   */
   /* from).  Once Data is placed into a Mailbox (via the               */
   /* BTPS_AddMailbox() function, it can be retreived by using the      */
   /* BTPS_WaitMailbox() function.  Data placed into the Mailbox is     */
   /* retrieved in a FIFO method.  This function accepts as input the   */
   /* Maximum Number of Slots that will be present in the Mailbox and   */
   /* the Size of each of the Slots.  This function returns a NON-NULL  */
   /* Mailbox Handle if the Mailbox is successfully created, or a       */
   /* NULL Mailbox Handle if the Mailbox was unable to be created.      */
Mailbox_t BTPSAPI BTPS_CreateMailbox(unsigned int NumberSlots, unsigned int SlotSize)
{
   Mailbox_t        ret_val;
   MailboxHeader_t *MailboxHeader;

   /* Before proceeding any further we need to make sure that the       */
   /* parameters that were passed to us appear semi-valid.              */
   if((NumberSlots) && (SlotSize))
   {
      /* Parameters appear semi-valid, so now let's allocate enough     */
      /* Memory to hold the Mailbox Header AND enough space to hold     */
      /* all requested Mailbox Slots.                                   */
      if((MailboxHeader = (MailboxHeader_t *)BTPS_AllocateMemory(sizeof(MailboxHeader_t)+(NumberSlots*SlotSize))) != NULL)
      {
         /* Memory allocated, now let's initialize the state of the     */
         /* Mailbox such that it contains NO Data.                      */
         MailboxHeader->NumberSlots   = NumberSlots;
         MailboxHeader->SlotSize      = SlotSize;
         MailboxHeader->HeadSlot      = 0;
         MailboxHeader->TailSlot      = 0;
         MailboxHeader->OccupiedSlots = 0;
         MailboxHeader->Slots         = ((unsigned char *)MailboxHeader) + sizeof(MailboxHeader_t);

         /* All finished, return success to the caller (the Mailbox     */
         /* Header).                                                    */
         ret_val                      = (Mailbox_t)MailboxHeader;
      }
      else
         ret_val = NULL;
   }
   else
      ret_val = NULL;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a means to Add data   */
   /* to the Mailbox (where it can be retrieved via the                 */
   /* BTPS_WaitMailbox() function.  This function accepts as input the  */
   /* Mailbox Handle of the Mailbox to place the data into and a        */
   /* pointer to a buffer that contains the data to be added.  This     */
   /* pointer *MUST* point to a data buffer that is AT LEAST the Size   */
   /* of the Slots in the Mailbox (specified when the Mailbox was       */
   /* created) and this pointer CANNOT be NULL.  The data that the      */
   /* MailboxData pointer points to is placed into the Mailbox where it */
   /* can be retrieved via the BTPS_WaitMailbox() function.             */
   /* * NOTE * This function copies from the MailboxData Pointer the    */
   /*          first SlotSize Bytes.  The SlotSize was specified when   */
   /*          the Mailbox was created via a successful call to the     */
   /*          BTPS_CreateMailbox() function.                           */
Boolean_t BTPSAPI BTPS_AddMailbox(Mailbox_t Mailbox, void *MailboxData)
{
   Boolean_t ret_val;

   /* Before proceeding any further make sure that the Mailbox Handle   */
   /* and the MailboxData pointer that was specified appears semi-valid.*/
   if((Mailbox) && (MailboxData))
   {
      /* Before adding the data to the Mailbox, make sure that the      */
      /* Mailbox is not already full.                                   */
      if(((MailboxHeader_t *)Mailbox)->OccupiedSlots < ((MailboxHeader_t *)Mailbox)->NumberSlots)
      {
         /* Mailbox is NOT full, so add the specified User Data to the  */
         /* next available free Mailbox Slot.                           */
         BTPS_MemCopy(&(((unsigned char *)(((MailboxHeader_t *)Mailbox)->Slots))[((MailboxHeader_t *)Mailbox)->HeadSlot*((MailboxHeader_t *)Mailbox)->SlotSize]), MailboxData, ((MailboxHeader_t *)Mailbox)->SlotSize);

         /* Update the Next available Free Mailbox Slot (taking into    */
         /* account wrapping the pointer).                              */
         if(++(((MailboxHeader_t *)Mailbox)->HeadSlot) == ((MailboxHeader_t *)Mailbox)->NumberSlots)
            ((MailboxHeader_t *)Mailbox)->HeadSlot = 0;

         /* Update the Number of occupied slots to signify that there   */
         /* was additional Mailbox Data added to the Mailbox.           */
         ((MailboxHeader_t *)Mailbox)->OccupiedSlots++;

         /* Finally, return success to the caller.                      */
         ret_val = TRUE;
      }
      else
         ret_val = FALSE;
   }
   else
      ret_val = FALSE;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is provided to allow a means to retrieve   */
   /* data from the specified Mailbox.  This function will block until  */
   /* either Data is placed in the Mailbox or an error with the Mailbox */
   /* was detected.  This function accepts as its first parameter a     */
   /* Mailbox Handle that represents the Mailbox to wait for the data   */
   /* with.  This function accepts as its second parameter, a pointer   */
   /* to a data buffer that is AT LEAST the size of a single Slot of    */
   /* the Mailbox (specified when the BTPS_CreateMailbox() function     */
   /* was called).  The MailboxData parameter CANNOT be NULL.  This     */
   /* function will return TRUE if data was successfully retrieved      */
   /* from the Mailbox or FALSE if there was an error retrieving data   */
   /* from the Mailbox.  If this function returns TRUE then the first   */
   /* SlotSize bytes of the MailboxData pointer will contain the data   */
   /* that was retrieved from the Mailbox.                              */
   /* * NOTE * This function copies to the MailboxData Pointer the      */
   /*          data that is present in the Mailbox Slot (of size        */
   /*          SlotSize).  The SlotSize was specified when the Mailbox  */
   /*          was created via a successful call to the                 */
   /*          BTPS_CreateMailbox() function.                           */
Boolean_t BTPSAPI BTPS_WaitMailbox(Mailbox_t Mailbox, void *MailboxData)
{
   Boolean_t ret_val;

   /* Before proceeding any further make sure that the Mailbox Handle   */
   /* and the MailboxData pointer that was specified appears semi-valid.*/
   if((Mailbox) && (MailboxData))
   {
      /* Let's check to see if there exists at least one slot with      */
      /* Mailbox Data present in it.                                    */
      if(((MailboxHeader_t *)Mailbox)->OccupiedSlots)
      {
         /* Flag success to the caller.                                 */
         ret_val = TRUE;

         /* Now copy the Data into the Memory Buffer specified by the   */
         /* caller.                                                     */
         BTPS_MemCopy(MailboxData, &((((unsigned char *)((MailboxHeader_t *)Mailbox)->Slots))[((MailboxHeader_t *)Mailbox)->TailSlot*((MailboxHeader_t *)Mailbox)->SlotSize]), ((MailboxHeader_t *)Mailbox)->SlotSize);

         /* Now that we've copied the data into the Memory Buffer       */
         /* specified by the caller we need to mark the Mailbox Slot as */
         /* free.                                                       */
         if(++(((MailboxHeader_t *)Mailbox)->TailSlot) == ((MailboxHeader_t *)Mailbox)->NumberSlots)
            ((MailboxHeader_t *)Mailbox)->TailSlot = 0;

         ((MailboxHeader_t *)Mailbox)->OccupiedSlots--;
      }
      else
         ret_val = FALSE;
   }
   else
      ret_val = FALSE;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* determine if there is anything queued in the specified Mailbox.   */
   /* This function returns TRUE if there is something queued in the    */
   /* Mailbox, or FALSE if there is nothing queued in the specified     */
   /* Mailbox.                                                          */
Boolean_t BTPSAPI BTPS_QueryMailbox(Mailbox_t Mailbox)
{
   Boolean_t ret_val;

   /* Before proceeding any further make sure that the Mailbox Handle   */
   /* and the MailboxData pointer that was specified appears semi-valid.*/
   if(Mailbox)
   {
      /* Let's check to see if there exists at least one slot with      */
      /* Mailbox Data present in it.                                    */
      if(((MailboxHeader_t *)Mailbox)->OccupiedSlots)
      {
         /* Flag success to the caller.                                 */
         ret_val = TRUE;
      }
      else
         ret_val = FALSE;
   }
   else
      ret_val = FALSE;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for destroying a Mailbox    */
   /* that was created successfully via a successful call to the        */
   /* BTPS_CreateMailbox() function.  This function accepts as input    */
   /* the Mailbox Handle of the Mailbox to destroy.  Once this function */
   /* is completed the Mailbox Handle is NO longer valid and CANNOT be  */
   /* used.  Calling this function will cause all outstanding           */
   /* BTPS_WaitMailbox() functions to fail with an error.  The final    */
   /* parameter specifies an (optional) callback function that is called*/
   /* for each queued Mailbox entry.  This allows a mechanism to free   */
   /* any resources that might be associated with each individual       */
   /* Mailbox item.                                                     */
void BTPSAPI BTPS_DeleteMailbox(Mailbox_t Mailbox, BTPS_MailboxDeleteCallback_t MailboxDeleteCallback)
{
   /* Before proceeding any further make sure that the Mailbox Handle   */
   /* that was specified appears semi-valid.                            */
   if(Mailbox)
   {
      /* Check to see if a Mailbox Delete Item Callback was specified.  */
      if(MailboxDeleteCallback)
      {
         /* Now loop though all of the occupied slots and call the      */
         /* callback with the slot data.                                */
         while(((MailboxHeader_t *)Mailbox)->OccupiedSlots)
         {
            __BTPSTRY
            {
               (*MailboxDeleteCallback)(&((((unsigned char *)((MailboxHeader_t *)Mailbox)->Slots))[((MailboxHeader_t *)Mailbox)->TailSlot*((MailboxHeader_t *)Mailbox)->SlotSize]));
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Now that we've called back with the data, we need to     */
            /* advance to the next slot.                                */
            if(++(((MailboxHeader_t *)Mailbox)->TailSlot) == ((MailboxHeader_t *)Mailbox)->NumberSlots)
               ((MailboxHeader_t *)Mailbox)->TailSlot = 0;

            /* Flag that there is one less occupied slot.               */
            ((MailboxHeader_t *)Mailbox)->OccupiedSlots--;
         }
      }

      /* Finally free all memory that was allocated for the Mailbox.    */
      BTPS_FreeMemory(Mailbox);
   }
}

   /* The following function is used to initialize the Platform module. */
   /* The Platform module relies on some static variables that are used */
   /* to coordinate the abstraction.  When the module is initially      */
   /* started from a cold boot, all variables are set to the proper     */
   /* state.  If the Warm Boot is required, then these variables need to*/
   /* be reset to their default values.  This function sets all static  */
   /* parameters to their default values.                               */
   /* * NOTE * The implementation is free to pass whatever information  */
   /*          required in this parameter.                              */
void BTPSAPI BTPS_Init(void *UserParam)
{
   /* Input parameter represents the Debug Message Output Callback      */
   /* function.                                                         */
   GetTickCountCallback  = NULL;
   MessageOutputCallback = NULL;

   if(UserParam)
   {
      if(((BTPS_Initialization_t *)UserParam)->GetTickCountCallback)
         GetTickCountCallback  = ((BTPS_Initialization_t *)UserParam)->GetTickCountCallback;

      if(((BTPS_Initialization_t *)UserParam)->MessageOutputCallback)
         MessageOutputCallback  = ((BTPS_Initialization_t *)UserParam)->MessageOutputCallback;
   }

   /* Initialize Scheduler parameters.                                  */
   NumberScheduledFunctions = 0;
   PreviousTickCount        = 0;

   /* Finally flag that the Scheduler has been initialized successfully.*/
   SchedulerInitialized     = TRUE;

   /* Initialize the static variables for this module.                  */
   HeapHead                 = NULL;
}

   /* The following function is used to cleanup the Platform module.    */
void BTPSAPI BTPS_DeInit(void)
{
   MessageOutputCallback = NULL;

   SchedulerInitialized  = FALSE;
}

   /* Write out the specified NULL terminated Debugging String to the   */
   /* Debug output.                                                     */
void BTPSAPI BTPS_OutputMessage(BTPSCONST char *DebugString, ...)
{
   int     ret_val;
   va_list args;

   /* Write out the Data.                                               */
   va_start(args, DebugString);
   ret_val = vSprintF(DebugMsgBuffer, DebugString, args);
   va_end(args);

   ConsoleWrite(DebugMsgBuffer, ret_val);
}

   /* The following function is used to set the Debug Mask that controls*/
   /* which debug zone messages get displayed.  The function takes as   */
   /* its only parameter the Debug Mask value that is to be used.  Each */
   /* bit in the mask corresponds to a debug zone.  When a bit is set,  */
   /* the printing of that debug zone is enabled.                       */
void BTPSAPI BTPS_SetDebugMask(unsigned long DebugMask)
{
   DebugZoneMask = DebugMask;
}

   /* The following function is a utility function that can be used to  */
   /* determine if a specified Zone is currently enabled for debugging. */
int BTPSAPI BTPS_TestDebugZone(unsigned long Zone)
{
   return(DebugZoneMask & Zone);
}

   /* The following function is responsible for displaying binary debug */
   /* data.  The first parameter to this function is the length of data */
   /* pointed to by the next parameter.  The final parameter is a       */
   /* pointer to the binary data to be  displayed.                      */
int BTPSAPI BTPS_DumpData(unsigned int DataLength, BTPSCONST unsigned char *DataPtr)
{
   int                    ret_val;
   char                   Buffer[80];
   char                  *BufPtr;
   char                  *HexBufPtr;
   Byte_t                 DataByte;
   unsigned int           Index;
   static BTPSCONST char  HexTable[] = "0123456789ABCDEF\r\n";
   static BTPSCONST char  Header1[]  = "       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  ";
   static BTPSCONST char  Header2[]  = " ------------------------------------------------------\r\n";

   /* Before proceeding any further, lets make sure that the parameters */
   /* passed to us appear semi-valid.                                   */
   if ((DataLength > 0) && (DataPtr != NULL))
   {
      /* The parameters which were passed in appear to be at least      */
      /* semi-valid, next write the header out to the file.             */
      BTPS_OutputMessage((char *)Header1);
      BTPS_OutputMessage((char *)HexTable);
      BTPS_OutputMessage((char *)Header2);

      /* Limit the number of bytes that will be displayed in the dump.  */
      if (DataLength > MAX_DBG_DUMP_BYTES)
      {
         DataLength = MAX_DBG_DUMP_BYTES;
      }

      /* Now that the Header is written out, let's output the Debug     */
      /* Data.                                                          */
      BTPS_MemInitialize(Buffer, ' ', sizeof(Buffer));
      BufPtr    = Buffer + BTPS_SprintF(Buffer," %05X ", 0);
      HexBufPtr = Buffer + sizeof(Header1)-1;
      for (Index=0; Index < DataLength;)
      {
         Index++;
         DataByte     = *DataPtr++;
         *BufPtr++    = HexTable[(Byte_t)(DataByte >> 4)];
         *BufPtr      = HexTable[(Byte_t)(DataByte & 0x0F)];
         BufPtr      += 2;
         /* Check to see if this is a printable character and not a     */
         /* special reserved character.  Replace the non-printable      */
         /* characters with a period.                                   */
         *HexBufPtr++ = (char)(((DataByte >= ' ') && (DataByte <= '~') && (DataByte != '\\') && (DataByte != '%'))?DataByte:'.');
         if (((Index % MAXIMUM_BYTES_PER_ROW) == 0) || (Index == DataLength))
         {
            /* We are at the end of this line, so terminate the data    */
            /* compiled in the buffer and send it to the output device. */
            *HexBufPtr++ = '\r';
            *HexBufPtr++ = '\n';
            *HexBufPtr   = 0;
            BTPS_OutputMessage(Buffer);
            if (Index != DataLength)
            {
               /* We have more to process, so prepare for the next line.*/
               BTPS_MemInitialize(Buffer, ' ', sizeof(Buffer));
               BufPtr    = Buffer + BTPS_SprintF(Buffer," %05X ", Index);
               HexBufPtr = Buffer + sizeof(Header1)-1;
            } else
            {
               /* Flag that there is no more data.                      */
               HexBufPtr = NULL;
            }
         }
      }
      /* Check to see if there is partial data in the buffer.           */
      if ((long)HexBufPtr > 0)
      {
         /* Terminate the buffer and output the line.                   */
         *HexBufPtr++ = '\r';
         *HexBufPtr++ = '\n';
         *HexBufPtr   = 0;
         BTPS_OutputMessage(Buffer);
      }
      BTPS_OutputMessage("\r\n");

      /* Finally, set the return value to indicate success.             */
      ret_val = 0;
   } else
   {
      /* An invalid parameter was enterer.                              */
      ret_val = -1;
   }

   return(ret_val);
}



