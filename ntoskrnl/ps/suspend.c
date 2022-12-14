/* $Id: suspend.c 13714 2005-02-22 19:25:17Z weiden $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ps/suspend.c
 * PURPOSE:         Thread managment
 * 
 * PROGRAMMERS:     David Welch (welch@mcmail.com)
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* NOTES **********************************************************************
 *
 */

/* GLOBALS *******************************************************************/

static FAST_MUTEX SuspendMutex;

/* FUNCTIONS *****************************************************************/

VOID STDCALL
PiSuspendThreadRundownRoutine(PKAPC Apc)
{
}


VOID STDCALL
PiSuspendThreadKernelRoutine(PKAPC Apc,
			     PKNORMAL_ROUTINE* NormalRoutine,
			     PVOID* NormalContext,
			     PVOID* SystemArgument1,
			     PVOID* SystemArguemnt2)
{
}


VOID STDCALL
PiSuspendThreadNormalRoutine(PVOID NormalContext,
			     PVOID SystemArgument1,
			     PVOID SystemArgument2)
{
  PETHREAD CurrentThread = PsGetCurrentThread();
  while (CurrentThread->Tcb.SuspendCount > 0)
    {
      KeWaitForSingleObject(&CurrentThread->Tcb.SuspendSemaphore,
			    0,
			    UserMode,
			    TRUE,
			    NULL);
    }
}


NTSTATUS
PsResumeThread (PETHREAD Thread,
		PULONG SuspendCount)
{
  DPRINT("PsResumeThread (Thread %p  SuspendCount %p) called\n");

  ExAcquireFastMutex (&SuspendMutex);

  if (SuspendCount != NULL)
    {
      *SuspendCount = Thread->Tcb.SuspendCount;
    }

  if (Thread->Tcb.SuspendCount > 0)
    {
      Thread->Tcb.SuspendCount--;
      if (Thread->Tcb.SuspendCount == 0)
	{
	  KeReleaseSemaphore (&Thread->Tcb.SuspendSemaphore,
			      IO_NO_INCREMENT,
			      1,
			      FALSE);
	}
    }

  ExReleaseFastMutex (&SuspendMutex);

  return STATUS_SUCCESS;
}


NTSTATUS
PsSuspendThread(PETHREAD Thread, PULONG PreviousSuspendCount)
{
  ULONG OldValue;

  ExAcquireFastMutex(&SuspendMutex);
  OldValue = Thread->Tcb.SuspendCount;
  Thread->Tcb.SuspendCount++;
  if (!Thread->Tcb.SuspendApc.Inserted)
    {
      if (!KeInsertQueueApc(&Thread->Tcb.SuspendApc,
			    NULL,
			    NULL,
			    IO_NO_INCREMENT))
	{
	  Thread->Tcb.SuspendCount--;
	  ExReleaseFastMutex(&SuspendMutex);
	  return(STATUS_THREAD_IS_TERMINATING);
	}
    }
  ExReleaseFastMutex(&SuspendMutex);
  if (PreviousSuspendCount != NULL)
    {
      *PreviousSuspendCount = OldValue;
    }
  return(STATUS_SUCCESS);
}


NTSTATUS STDCALL
NtResumeThread(IN HANDLE ThreadHandle,
	       IN PULONG SuspendCount  OPTIONAL)
/*
 * FUNCTION: Decrements a thread's resume count
 * ARGUMENTS: 
 *        ThreadHandle = Handle to the thread that should be resumed
 *        ResumeCount =  The resulting resume count.
 * RETURNS: Status
 */
{
  PETHREAD Thread;
  NTSTATUS Status;
  ULONG Count;
  
  PAGED_CODE();

  DPRINT("NtResumeThead(ThreadHandle %lx  SuspendCount %p)\n",
	 ThreadHandle, SuspendCount);

  Status = ObReferenceObjectByHandle (ThreadHandle,
				      THREAD_SUSPEND_RESUME,
				      PsThreadType,
				      UserMode,
				      (PVOID*)&Thread,
				      NULL);
  if (!NT_SUCCESS(Status))
    {
      return Status;
    }

  Status = PsResumeThread (Thread, &Count);
  if (!NT_SUCCESS(Status))
    {
      ObDereferenceObject ((PVOID)Thread);
      return Status;
    }

  if (SuspendCount != NULL)
    {
      *SuspendCount = Count;
    }

  ObDereferenceObject ((PVOID)Thread);

  return STATUS_SUCCESS;
}


NTSTATUS STDCALL
NtSuspendThread(IN HANDLE ThreadHandle,
		IN PULONG PreviousSuspendCount  OPTIONAL)
/*
 * FUNCTION: Increments a thread's suspend count
 * ARGUMENTS: 
 *        ThreadHandle = Handle to the thread that should be resumed
 *        PreviousSuspendCount =  The resulting/previous suspend count.
 * REMARK:
 *        A thread will be suspended if its suspend count is greater than 0. 
 *        This procedure maps to the win32 SuspendThread function. ( 
 *        documentation about the the suspend count can be found here aswell )
 *        The suspend count is not increased if it is greater than 
 *        MAXIMUM_SUSPEND_COUNT.
 * RETURNS: Status
 */
{
  PETHREAD Thread;
  NTSTATUS Status;
  ULONG Count;
  
  PAGED_CODE();

  Status = ObReferenceObjectByHandle(ThreadHandle,
				     THREAD_SUSPEND_RESUME,
				     PsThreadType,
				     UserMode,
				     (PVOID*)&Thread,
				     NULL);
  if (!NT_SUCCESS(Status))
    {
      return(Status);
    }

  Status = PsSuspendThread(Thread, &Count);
  if (!NT_SUCCESS(Status))
    {
      ObDereferenceObject ((PVOID)Thread);
      return Status;
    }

  if (PreviousSuspendCount != NULL)
    {
      *PreviousSuspendCount = Count;
    }

  ObDereferenceObject ((PVOID)Thread);

  return STATUS_SUCCESS;
}

VOID INIT_FUNCTION
PsInitialiseSuspendImplementation(VOID)
{
  ExInitializeFastMutex(&SuspendMutex);
}

/* EOF */
