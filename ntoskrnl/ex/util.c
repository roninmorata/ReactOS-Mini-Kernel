/* $Id: util.c 13311 2005-01-26 13:58:37Z ion $
 * 
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ex/misc.c
 * PURPOSE:         Executive Utility Functions
 *
 * PROGRAMMERS:     No programmer listed.
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* FUNCTIONS *****************************************************************/

/*
 * @unimplemented
 */
STDCALL
NTSTATUS
ExUuidCreate(
    OUT UUID *Uuid
    )
{
	UNIMPLEMENTED;
	return FALSE;
}

/*
 * @implemented
 */
STDCALL
BOOLEAN
ExVerifySuite(
    SUITE_TYPE SuiteType
    )
{
    if (SuiteType == Personal) return TRUE;
    return FALSE;
}

/* EOF */
