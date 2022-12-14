/* $Id: gmutex.c 13311 2005-01-26 13:58:37Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ke/gmutex.c
 * PURPOSE:         Implements guarded mutex (w2k3+/64)
 * 
 * PROGRAMMERS:      No programmer listed.
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#include <internal/debug.h>

/* FUNCTIONS *****************************************************************/

/*
KeAcquireGuardedMutex
KeAcquireGuardedMutexUnsafe
KeEnterGuardedRegion
KeInitializeGuardedMutex
KeReleaseGuardedMutexUnsafe
KeTryToAcquireGuardedMutex
KeReleaseGuardedMutex
KeLeaveGuardedRegion
*/

/* EOF */
