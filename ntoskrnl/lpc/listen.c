/* $Id: listen.c 13311 2005-01-26 13:58:37Z ion $
 * 
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/lpc/listen.c
 * PURPOSE:         Communication mechanism
 * 
 * PROGRAMMERS:     David Welch (welch@cwcom.net)
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* FUNCTIONS *****************************************************************/

/**********************************************************************
 * NAME							EXPORTED
 *	NtListenPort@8
 *
 * DESCRIPTION
 *	Listen on a named port and wait for a connection attempt.
 *
 * ARGUMENTS
 *	PortHandle	[IN] LPC port to listen on.
 *
 *	ConnectMsg	[IN] User provided storage for a
 *			possible connection request LPC message.
 *
 * RETURN VALUE
 *	STATUS_SUCCESS if a connection request is received
 *	successfully; otherwise an error code.
 *
 *	The buffer ConnectMessage is filled with the connection
 *	request message queued by NtConnectPort() in PortHandle.
 *
 * NOTE
 */
/*EXPORTED*/ NTSTATUS STDCALL
NtListenPort (IN	HANDLE		PortHandle,
	      IN	PLPC_MESSAGE	ConnectMsg)
{
  NTSTATUS	Status;
  
  /*
   * Wait forever for a connection request.
   */
  for (;;)
    {
      Status = NtReplyWaitReceivePort(PortHandle,
				      NULL,
				      NULL,
				      ConnectMsg);
      /*
       * Accept only LPC_CONNECTION_REQUEST requests.
       * Drop any other message.
       */
      if (!NT_SUCCESS(Status) || 
	  LPC_CONNECTION_REQUEST == ConnectMsg->MessageType)
	{
	  DPRINT("Got message (type %x)\n", LPC_CONNECTION_REQUEST);
	  break;
	}
      DPRINT("Got message (type %x)\n", ConnectMsg->MessageType);
    }
  
  return (Status);
}


/* EOF */
