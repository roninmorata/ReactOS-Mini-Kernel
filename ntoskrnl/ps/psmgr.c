/* $Id: psmgr.c 14044 2005-03-14 01:47:18Z weiden $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ps/psmgr.c
 * PURPOSE:         Process management
 * 
 * PROGRAMMERS:     David Welch (welch@mcmail.com)
 */

/* INCLUDES **************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* FUNCTIONS ***************************************************************/

VOID PiShutdownProcessManager(VOID)
{
   DPRINT("PiShutdownProcessManager()\n");
   
   PiKillMostProcesses();
}

VOID INIT_FUNCTION
PiInitProcessManager(VOID)
{
   PsInitJobManagment();
   PsInitProcessManagment();
   PsInitThreadManagment();
   PsInitIdleThread();
   PsInitReaperThread();
   PsInitialiseSuspendImplementation();
   PsInitialiseW32Call();
}


/**********************************************************************
 * NAME							EXPORTED
 *	PsGetVersion
 *
 * DESCRIPTION
 *	Retrieves the current OS version.
 *
 * ARGUMENTS
 *	MajorVersion	Pointer to a variable that will be set to the
 *			major version of the OS. Can be NULL.
 *
 *	MinorVersion	Pointer to a variable that will be set to the
 *			minor version of the OS. Can be NULL.
 *
 *	BuildNumber	Pointer to a variable that will be set to the
 *			build number of the OS. Can be NULL.
 *
 *	CSDVersion	Pointer to a variable that will be set to the
 *			CSD string of the OS. Can be NULL.
 *
 * RETURN VALUE
 *	TRUE	OS is a checked build.
 *	FALSE	OS is a free build.
 *
 * NOTES
 *	The DDK docs say something about a 'CmCSDVersionString'.
 *	How do we determine in the build is checked or free??
 *
 * @unimplemented
 */

BOOLEAN
STDCALL
PsGetVersion (
	PULONG		MajorVersion	OPTIONAL,
	PULONG		MinorVersion	OPTIONAL,
	PULONG		BuildNumber	OPTIONAL,
	PUNICODE_STRING	CSDVersion	OPTIONAL
	)
{
	if (MajorVersion)
		*MajorVersion = 4;

	if (MinorVersion)
		*MinorVersion = 0;

	if (BuildNumber)
		*BuildNumber = 1381;

	if (CSDVersion)
	{
		CSDVersion->Length = 0;
		CSDVersion->MaximumLength = 0;
		CSDVersion->Buffer = NULL;
#if 0
		CSDVersion->Length = CmCSDVersionString.Length;
		CSDVersion->MaximumLength = CmCSDVersionString.Maximum;
		CSDVersion->Buffer = CmCSDVersionString.Buffer;
#endif
	}

	/* FIXME: How do we determine if build is checked or free? */
	return FALSE;
}

/* EOF */
