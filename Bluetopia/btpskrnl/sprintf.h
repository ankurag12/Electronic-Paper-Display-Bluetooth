/*****< sprintf.h >************************************************************/
/*      Copyright 2000 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SPRINTF - Stonestreet One 'sprintf' Implementation Type Definitions,      */
/*            Constants, and Prototypes.                                      */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/03/10  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __SPRINTF_H__
#define __SPRINTF_H__

#include <stdarg.h>

   /* The folllwing function is used to format an output sting based on */
   /* a provided format.  This function should be called from a function*/
   /* that receives a variable argument lsit.  The function takes as its*/
   /* first parameter a pointer to a buffer that will receive the       */
   /* formatted output.  The second parameters is a pointer to a strnig */
   /* that defines the format of the output.  The last parameter is a   */
   /* variable argument list.  The function returns the number if       */
   /* characters that weere placed in the output buffer.                */
int vSprintF(char *buffer, const char *format, va_list ap);

   /* The following function formats a string and places the formatted  */
   /* string into an output buffer.  The function takes as its first    */
   /* parameter a pointer to the output buffer.  The second parameter is*/
   /* a pointer to a string that represents how to format the data.  The*/
   /* remaining data are the parameters specified by the format of the  */
   /* output string.  The function return the number of characters that */
   /* were placed in the output buffer..                                */
int SprintF(char *buffer, const char *format, ...);

#endif
