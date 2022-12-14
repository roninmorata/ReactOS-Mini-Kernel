/* $Id: wdm.c 13311 2005-01-26 13:58:37Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/io/wdm.c
 * PURPOSE:         Various Windows Driver Model routines
 * 
 * PROGRAMMERS:     Filip Navara (xnavara@volny.cz)
 */

#include <ntoskrnl.h>

/*
 * @implemented
 */
BOOLEAN STDCALL
IoIsWdmVersionAvailable(
	IN UCHAR MajorVersion,
	IN UCHAR MinorVersion
	)
{
   if (MajorVersion <= 1 && MinorVersion <= 10)
      return TRUE;
   return FALSE;
}
