/* $Id: query.c 13311 2005-01-26 13:58:37Z ion $
 * 
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/lpc/query.c
 * PURPOSE:         Communication mechanism
 * 
 * PROGRAMMERS:     David Welch (welch@cwcom.net)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* FUNCTIONS *****************************************************************/

/**********************************************************************
 * NAME							EXPORTED
 *	NtQueryInformationPort@20
 *	
 * DESCRIPTION
 *
 * ARGUMENTS
 *	PortHandle		[IN]
 *	PortInformationClass	[IN]
 *	PortInformation		[OUT]
 *	PortInformationLength	[IN]
 *	ReturnLength		[OUT]
 *
 * RETURN VALUE
 *	STATUS_SUCCESS if the call succedeed. An error code
 *	otherwise.
 *
 * NOTES
 * 	P. Dabak reports that this system service seems to return
 * 	no information.
 */
/*EXPORTED*/ NTSTATUS STDCALL
NtQueryInformationPort (IN	HANDLE	PortHandle,
			IN	CINT	PortInformationClass,	
			OUT	PVOID	PortInformation,    
			IN	ULONG	PortInformationLength,
			OUT	PULONG	ReturnLength)
{
  NTSTATUS	Status;
  PEPORT		Port;
  
  Status = ObReferenceObjectByHandle (PortHandle,
				      PORT_ALL_ACCESS,   /* AccessRequired */
				      LpcPortObjectType,
				      UserMode,
				      (PVOID *) & Port,
				      NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("NtQueryInformationPort() = %x\n", Status);
      return (Status);
    }
  /*
   * FIXME: NT does nothing here!
   */
  ObDereferenceObject (Port);
  return STATUS_SUCCESS;
}


/* EOF */
