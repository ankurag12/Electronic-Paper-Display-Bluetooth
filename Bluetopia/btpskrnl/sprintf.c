/*****< sprintf.c >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  sprintf - Reduced memory requirment implimentation for use in embeded     */
/*            systems.                                                        */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/15/10  T. Thomas      Initial Creation                                */
/******************************************************************************/

#include "sprintf.h"

   /* Flag bit patterns                                                 */
#define FFORMAT   0x0100 /* Saw a Format Flag                           */
#define FZERO     0x0200 /* PAD WITH '0'                                */
#define FPADLEFT  0x0400 /* LEFT PADDING POSSIBILY REQUIRED             */
#define FUNSIGNED 0x0800 /* if writing an unsigned num                  */
#define FLONG     0x1000 /* if writing a long varible                   */
#define FUPPER    0x2000 /* Upper case Hex values                       */

#define PF_8         signed char
#define PF_U8        unsigned char
#define PF_16        int
#define PF_U16       unsigned int
#define PF_32        long
#define PF_U32       unsigned long

static char HexTable[]  = "0123456789abcdef";
static char UHexTable[] = "0123456789ABCDEF";


static int NumDigits(unsigned long val, int Radix);
static unsigned int LtoA(PF_U32 val, PF_8 *buffer, PF_U16 flags, char LongValue);
static unsigned int LtoH(PF_U32 val, PF_8 *buffer, PF_U16 flags);


   /* The following function is used to determine the number of digits  */
   /* that will need to be printed to print specified value.            */
static int NumDigits(unsigned long val, int Radix)
{
   int ret_val= 1;

   while(val /= Radix)
      ret_val++;

   return(ret_val);
}

   /* The following function converts a 32 bit value to a string        */
   /* representation of the value.  The function receives the value to  */
   /* be processed as its first parameter.  The second parameter is a   */
   /* pointer to a buffer to receive the formatted string.  The last    */
   /* parameter contains flags that control to format of the string.    */
   /* The function returns the number of characters that were placed in */
   /* the output buffer.                                                */
static unsigned int LtoA(PF_U32 val, PF_8 *bufptr, PF_U16 flags, char LongValue)
{
   unsigned int ret_val;
   int          negative;
   char         fill_char;

   /* set a local pointer to the buffer pointer.                        */
   negative = 0;
   /* Check to see if the value is negative                             */
   if(!(flags & FUNSIGNED))
   {
      if(LongValue)
      {
         if((PF_32)val < 0)
         {
            negative = 1;
            val      = (PF_U32)(0-((PF_32)val));
         }
      }
      else
      {
         if((int)val < 0)
         {
            negative = 1;
            val      = (PF_U32)(0-((int)val));
         }
      }
   }

   /* If the number of digits required is greater than or equal to the  */
   /* padding, then just adjust the pointer.                            */
   ret_val = NumDigits(val, 10);
   if((PF_U8)ret_val >= (PF_U8)flags)
   {
      bufptr += ret_val;
      if(negative)
      {
         ret_val++;
         bufptr++;
      }
   }
   else
   {
      ret_val = (PF_U8)flags;

      fill_char = (flags & FZERO)?'0':' ';
      while((PF_U8)flags)
      {
         *(bufptr++) = fill_char;
         flags--;
      }
   }

   /* Make sure string is NULL terminated                               */
   *bufptr = 0;

   /* Build string 'backwards'                                          */
   do
   {
      *(--bufptr) = (val % 10) + '0';
   }
   while(val /= 10);
   if(negative)
      *(--bufptr) = '-';

   return(ret_val);
}

   /* The following function converts a 32 bit value to a Hex string    */
   /* representation of the value.  The function receives the value to  */
   /* be processed as its first parameter.  The second parameter is a   */
   /* pointer to a buffer to receive the formatted string.  The last    */
   /* parameter contains flags that control to format of the string.    */
   /* The function returns the number of characters that were placed in */
   /* the output buffer.                                                */
static unsigned int LtoH(PF_U32 val, PF_8 *bufptr, PF_U16 flags)
{
   unsigned int  ret_val;
   char         *hextable;
   char          fill_char;

   ret_val = NumDigits(val, 16);

   /* If the number of digits required is greater than or equal to the  */
   /* padding, then just adjust the pointer.                            */
   if((PF_U8)ret_val >= (PF_U8)flags)
      bufptr += ret_val;
   else
   {
      ret_val   = (PF_U8)flags;
      fill_char = (flags & FZERO)?'0':' ';
      while((PF_U8)flags)
      {
         *(bufptr++) = fill_char;
         flags--;
      }
   }

   /* Make sure string is NULL terminated                               */
   *bufptr = 0;

   hextable = (flags & FUPPER)?UHexTable:HexTable;

   /* Build string 'backwards'                                          */
   do
   {
      *(--bufptr) = hextable[val & 0x0F];
   }
   while(val >>= 4);

   return(ret_val);
}

   /* The following function is used to format an output sting based on */
   /* a provided format.  This function should be called from a function*/
   /* that receives a variable argument list.  The function takes as its*/
   /* first parameter a pointer to a buffer that will receive the       */
   /* formatted output.  The second parameters is a pointer to a string */
   /* that defines the format of the output.  The last parameter is a   */
   /* variable argument list.  The function returns the number if       */
   /* characters that weere placed in the output buffer.                */
int vSprintF(char *buffer, const char *format, va_list ap)
{
   unsigned int  flags;                /* flags that control conversion */
   unsigned int  value;                /* value read for %d             */
   unsigned long lvalue;               /* value read for %l             */
   char          *p;                   /* temp char pointer             */
   char          *bufptr;
   char          ch;

   /* go through format until the end                                   */
   flags   = 0;
   bufptr  = buffer;

   /* Verify that the pointer to the output buffer is valid.            */
   if(buffer)
   {
      while((ch = *(format++)))
      {
         /* Check to see if we are working on a format.                 */
         if(!flags)
         {
            /* copy non '%' chars into result string                    */
            if(ch != '%')
            {
               *(bufptr++) = ch;
            }
            else
               flags = FFORMAT;

            /* Continue with the next character.                        */
            continue;
         }

         /* Check to see if the last character was a '%'.               */
         if(flags == FFORMAT)
         {
            /* Clear the format flag.                                   */
            flags = 0;

            /* Must have hit a '%', handle possible '%%'                */
            if(ch == '%')
            {
               /* Hit 2 '%'s, print one out                             */
               *(bufptr++) = ch;
               continue;
            }
         }

         /* process flags                                               */
         if(ch == '0')
         {
            flags     |= FZERO;
            continue;
         }

         /* check for possible field width (no support for '*')         */
         if((ch > '1') && (ch <= '9'))
         {
            flags     |= (FPADLEFT + (ch & 0x0F));
            continue;
         }

         switch(ch)
         {
            case 'l':
               /* Small cheat to see if this is an unsigned sprintf().  */
               if(*format == 'u')
               {
                  flags |= FUNSIGNED;

                  format++;
               }
               else
               {
                  if(*format == 'd')
                     format++;
               }

               lvalue  = va_arg(ap, long);
               bufptr += LtoA((PF_32)lvalue, (PF_8 *)bufptr, flags, 1);
               break;
            case 'u':
               flags |= FUNSIGNED;
            case 'd':
               value = va_arg(ap, int);
               bufptr += LtoA((PF_32)value, (PF_8 *)bufptr, flags, 0);
               break;
            case 'X':
               flags |= FUPPER;
            case 'x':
               value = va_arg(ap, int);
               bufptr += LtoH((PF_U32)value, (PF_8 *)bufptr, flags);
               break;
            case 'c':
               value       = va_arg(ap, int);
               *(bufptr++) = (char)value;
               break;
            case 's':
               p = va_arg(ap, char *);
               if(p)
               {
                  while(*p)
                     *(bufptr++) = *(p++);
               }
               break;
         }
         flags = 0;
      }

      /* Null Terminate the string.                                     */
      *bufptr = 0x00;
   }

   return(int)(bufptr-buffer);
}

   /* The following function formats a string and places the formatted  */
   /* string into an output buffer.  The function takes as its first    */
   /* parameter a pointer to the output buffer.  The second parameter is*/
   /* a pointer to a string that represents how to format the data.  The*/
   /* remaining data are the parameters specified by the format of the  */
   /* output string.  The function return the number of characters that */
   /* were placed in the output buffer..                                */
int SprintF(char *buffer, const char *format, ...)
{
   int     ret_val;
   va_list ap;                         /* argument pointer              */

   /* init the variable number argument pointer                         */
   va_start(ap, format);

   ret_val = vSprintF(buffer, format ,ap);

   va_end(ap);

   return(ret_val);
}


