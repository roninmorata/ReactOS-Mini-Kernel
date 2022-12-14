/* $Id: npipe.c 13311 2005-01-26 13:58:37Z ion $
 * 
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/io/npipe.c
 * PURPOSE:         Named pipe helper function
 * 
 * PROGRAMMERS:     David Welch (welch@mcmail.com)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* FUNCTIONS *****************************************************************/

NTSTATUS STDCALL
NtCreateNamedPipeFile(PHANDLE FileHandle,
		      ACCESS_MASK DesiredAccess,
		      POBJECT_ATTRIBUTES ObjectAttributes,
		      PIO_STATUS_BLOCK IoStatusBlock,
		      ULONG ShareAccess,
		      ULONG CreateDisposition,
		      ULONG CreateOptions,
		      ULONG NamedPipeType,
		      ULONG ReadMode,
		      ULONG CompletionMode,
		      ULONG MaximumInstances,
		      ULONG InboundQuota,
		      ULONG OutboundQuota,
		      PLARGE_INTEGER DefaultTimeout)
{
  NAMED_PIPE_CREATE_PARAMETERS Buffer;

  DPRINT("NtCreateNamedPipeFile(FileHandle %x, DesiredAccess %x, "
	 "ObjectAttributes %x ObjectAttributes->ObjectName->Buffer %S)\n",
	 FileHandle,DesiredAccess,ObjectAttributes,
	 ObjectAttributes->ObjectName->Buffer);

  ASSERT_IRQL(PASSIVE_LEVEL);

  if (DefaultTimeout != NULL)
    {
      Buffer.DefaultTimeout.QuadPart = DefaultTimeout->QuadPart;
      Buffer.TimeoutSpecified = TRUE;
    }
  else
    {
      Buffer.TimeoutSpecified = FALSE;
    }
  Buffer.NamedPipeType = NamedPipeType;
  Buffer.ReadMode = ReadMode;
  Buffer.CompletionMode = CompletionMode;
  Buffer.MaximumInstances = MaximumInstances;
  Buffer.InboundQuota = InboundQuota;
  Buffer.OutboundQuota = OutboundQuota;

  return IoCreateFile(FileHandle,
		      DesiredAccess,
		      ObjectAttributes,
		      IoStatusBlock,
		      NULL,
		      FILE_ATTRIBUTE_NORMAL,
		      ShareAccess,
		      CreateDisposition,
		      CreateOptions,
		      NULL,
		      0,
		      CreateFileTypeNamedPipe,
		      (PVOID)&Buffer,
		      0);
}

/* EOF */
