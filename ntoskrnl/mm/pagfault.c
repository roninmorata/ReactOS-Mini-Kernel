/* $Id: pagfault.c 13311 2005-01-26 13:58:37Z ion $
 * 
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/mm/pagfault.c
 * PURPOSE:         No purpose listed.
 *
 * PROGRAMMERS:     No programmer listed.
 */


#include <ntoskrnl.h>

/*
 * @implemented
 */
BOOLEAN
STDCALL
MmIsRecursiveIoFault (
   VOID
)
{
   PETHREAD Thread = PsGetCurrentThread ();

   return ( Thread->DisablePageFaultClustering
            | Thread->ForwardClusterOnly
          );
}


/* EOF */
