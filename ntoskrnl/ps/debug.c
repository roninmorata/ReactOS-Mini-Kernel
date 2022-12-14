/* $Id: debug.c 13714 2005-02-22 19:25:17Z weiden $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ps/debug.c
 * PURPOSE:         Thread managment
 * 
 * PROGRAMMERS:     David Welch (welch@mcmail.com)
 *                  Phillip Susi
 */

/*
 * NOTE:
 * 
 * All of the routines that manipulate the thread queue synchronize on
 * a single spinlock
 * 
 */

/* INCLUDES ****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* FUNCTIONS ***************************************************************/

VOID
KeContextToTrapFrame(PCONTEXT Context,
		     PKTRAP_FRAME TrapFrame)
{
   if ((Context->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL)
     {
	TrapFrame->Esp = Context->Esp;
	TrapFrame->Ss = Context->SegSs;
	TrapFrame->Cs = Context->SegCs;
	TrapFrame->Eip = Context->Eip;
	TrapFrame->Eflags = Context->EFlags;	
	TrapFrame->Ebp = Context->Ebp;
     }
   if ((Context->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER)
     {
	TrapFrame->Eax = Context->Eax;
	TrapFrame->Ebx = Context->Ebx;
	TrapFrame->Ecx = Context->Ecx;
	TrapFrame->Edx = Context->Edx;
	TrapFrame->Esi = Context->Esi;
	TrapFrame->Edi = Context->Edi;
     }
   if ((Context->ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS)
     {
	TrapFrame->Ds = Context->SegDs;
	TrapFrame->Es = Context->SegEs;
	TrapFrame->Fs = Context->SegFs;
	TrapFrame->Gs = Context->SegGs;
     }
   if ((Context->ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT)
     {
	/*
	 * Not handled
	 *
	 * This should be handled separately I think.
	 *  - blight
	 */
     }
   if ((Context->ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS)
     {
	/*
	 * Not handled
	 */
     }
}

VOID
KeTrapFrameToContext(PKTRAP_FRAME TrapFrame,
		     PCONTEXT Context)
{
   if ((Context->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL)
     {
	Context->SegSs = TrapFrame->Ss;
	Context->Esp = TrapFrame->Esp;
	Context->SegCs = TrapFrame->Cs;
	Context->Eip = TrapFrame->Eip;
	Context->EFlags = TrapFrame->Eflags;
	Context->Ebp = TrapFrame->Ebp;
     }
   if ((Context->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER)
     {
	Context->Eax = TrapFrame->Eax;
	Context->Ebx = TrapFrame->Ebx;
	Context->Ecx = TrapFrame->Ecx;
	/*
	 * NOTE: In the trap frame which is built on entry to a system
	 * call TrapFrame->Edx will actually hold the address of the
	 * previous TrapFrame. I don't believe leaking this information
	 * has security implications. Also EDX holds the address of the
	 * arguments to the system call in progress so it isn't of much
	 * interest to the debugger.
	 */
	Context->Edx = TrapFrame->Edx;
	Context->Esi = TrapFrame->Esi;
	Context->Edi = TrapFrame->Edi;
     }
   if ((Context->ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS)
     {
	Context->SegDs = TrapFrame->Ds;
	Context->SegEs = TrapFrame->Es;
	Context->SegFs = TrapFrame->Fs;
	Context->SegGs = TrapFrame->Gs;
     }
   if ((Context->ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS)
     {
	/*
	 * FIXME: Implement this case
	 */	
	Context->ContextFlags &= (~CONTEXT_DEBUG_REGISTERS) | CONTEXT_i386;
     }
   if ((Context->ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT)
     {
	/*
	 * FIXME: Implement this case
	 *
	 * I think this should only be filled for FPU exceptions, otherwise I
         * would not know where to get it from as it can be the current state
	 * of the FPU or already saved in the thread's FPU save area.
	 *  -blight
	 */
	Context->ContextFlags &= (~CONTEXT_FLOATING_POINT) | CONTEXT_i386;
     }
#if 0
   if ((Context->ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS)
     {
	/*
	 * FIXME: Investigate this
	 *
	 * This is the XMM state (first 512 bytes of FXSAVE_FORMAT/FX_SAVE_AREA)
	 * This should only be filled in case of a SIMD exception I think, so
	 * this is not the right place (like for FPU the state could already be
	 * saved in the thread's FX_SAVE_AREA or still be in the CPU)
	 *  -blight
	 */
        Context->ContextFlags &= ~CONTEXT_EXTENDED_REGISTERS;
     }
#endif
}

VOID STDCALL
KeGetSetContextRundownRoutine(PKAPC Apc)
{
  PKEVENT Event;
  PNTSTATUS Status;

  Event = (PKEVENT)Apc->SystemArgument1;   
  Status = (PNTSTATUS)Apc->SystemArgument2;
  (*Status) = STATUS_THREAD_IS_TERMINATING;
  KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
}

VOID STDCALL
KeGetContextKernelRoutine(PKAPC Apc,
			  PKNORMAL_ROUTINE* NormalRoutine,
			  PVOID* NormalContext,
			  PVOID* SystemArgument1,
			  PVOID* SystemArgument2)
/*
 * FUNCTION: This routine is called by an APC sent by NtGetContextThread to
 * copy the context of a thread into a buffer.
 */
{
  PKEVENT Event;
  PCONTEXT Context;
  PNTSTATUS Status;
   
  Context = (PCONTEXT)(*NormalContext);
  Event = (PKEVENT)(*SystemArgument1);
  Status = (PNTSTATUS)(*SystemArgument2);
   
  KeTrapFrameToContext(KeGetCurrentThread()->TrapFrame, Context);
   
  *Status = STATUS_SUCCESS;
  KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
}

NTSTATUS STDCALL
NtGetContextThread(IN HANDLE ThreadHandle,
		   OUT PCONTEXT ThreadContext)
{
  PETHREAD Thread;
  CONTEXT Context;
  KAPC Apc;
  KEVENT Event;
  KPROCESSOR_MODE PreviousMode;
  NTSTATUS Status = STATUS_SUCCESS;
  
  PAGED_CODE();
  
  PreviousMode = ExGetPreviousMode();

  if(PreviousMode != KernelMode)
  {
    _SEH_TRY
    {
      ProbeForWrite(ThreadContext,
                    sizeof(CONTEXT),
                    sizeof(ULONG));
    }
    _SEH_HANDLE
    {
      Status = _SEH_GetExceptionCode();
    }
    _SEH_END;

    if(!NT_SUCCESS(Status))
    {
      return Status;
    }
  }
  
  Status = ObReferenceObjectByHandle(ThreadHandle,
                                     THREAD_GET_CONTEXT,
                                     PsThreadType,
                                     PreviousMode,
                                     (PVOID*)&Thread,
                                     NULL);
  if(NT_SUCCESS(Status))
  {
    if(Thread == PsGetCurrentThread())
    {
      /*
       * I don't know if trying to get your own context makes much
       * sense but we can handle it more efficently.
       */
	
      KeTrapFrameToContext(Thread->Tcb.TrapFrame, &Context);
    }
    else
    {
      KeInitializeEvent(&Event,
                        NotificationEvent,
                        FALSE);

      KeInitializeApc(&Apc,
                      &Thread->Tcb,
                      OriginalApcEnvironment,
                      KeGetContextKernelRoutine,
                      KeGetSetContextRundownRoutine,
                      NULL,
                      KernelMode,
                      (PVOID)&Context);
      if (!KeInsertQueueApc(&Apc,
			    (PVOID)&Event,
			    (PVOID)&Status,
			    IO_NO_INCREMENT))
	{
	  Status = STATUS_THREAD_IS_TERMINATING;
	}
      else
	{
	  Status = KeWaitForSingleObject(&Event,
					 0,
					 KernelMode,
					 FALSE,
					 NULL);
	}
    }
    ObDereferenceObject(Thread);
    
    if(NT_SUCCESS(Status))
    {
      _SEH_TRY
      {
        *ThreadContext = Context;
      }
      _SEH_HANDLE
      {
        Status = _SEH_GetExceptionCode();
      }
      _SEH_END;
    }
  }
  
  return Status;
}

VOID STDCALL
KeSetContextKernelRoutine(PKAPC Apc,
			  PKNORMAL_ROUTINE* NormalRoutine,
			  PVOID* NormalContext,
			  PVOID* SystemArgument1,
			  PVOID* SystemArgument2)
/*
 * FUNCTION: This routine is called by an APC sent by NtSetContextThread to
 * set the context of a thread from a buffer.
 */
{
  PKEVENT Event;
  PCONTEXT Context;
  PNTSTATUS Status;
   
  Context = (PCONTEXT)(*NormalContext);
  Event = (PKEVENT)(*SystemArgument1);
  Status = (PNTSTATUS)(*SystemArgument2);
   
  KeContextToTrapFrame(Context, KeGetCurrentThread()->TrapFrame);
   
  *Status = STATUS_SUCCESS;
  KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
}

NTSTATUS STDCALL
NtSetContextThread(IN HANDLE ThreadHandle,
		   IN PCONTEXT ThreadContext)
{
  PETHREAD Thread;
  KAPC Apc;
  KEVENT Event;
  CONTEXT Context;
  KPROCESSOR_MODE PreviousMode;
  NTSTATUS Status = STATUS_SUCCESS;
  
  PAGED_CODE();
  
  PreviousMode = ExGetPreviousMode();
  
  if(PreviousMode != KernelMode)
  {
    _SEH_TRY
    {
      ProbeForRead(ThreadContext,
                   sizeof(CONTEXT),
                   sizeof(ULONG));
      Context = *ThreadContext;
      ThreadContext = &Context;
    }
    _SEH_HANDLE
    {
      Status = _SEH_GetExceptionCode();
    }
    _SEH_END;
    
    if(!NT_SUCCESS(Status))
    {
      return Status;
    }
  }

  Status = ObReferenceObjectByHandle(ThreadHandle,
                                     THREAD_SET_CONTEXT,
                                     PsThreadType,
                                     PreviousMode,
                                     (PVOID*)&Thread,
                                     NULL);
  if(NT_SUCCESS(Status))
  {
    if (Thread == PsGetCurrentThread())
    {
      /*
       * I don't know if trying to set your own context makes much
       * sense but we can handle it more efficently.
       */
	
      KeContextToTrapFrame(&Context, Thread->Tcb.TrapFrame);
    }
    else
    {
      KeInitializeEvent(&Event,
                        NotificationEvent,
                        FALSE);	
	
      KeInitializeApc(&Apc,
                      &Thread->Tcb,
                      OriginalApcEnvironment,
                      KeSetContextKernelRoutine,
                      KeGetSetContextRundownRoutine,
                      NULL,
                      KernelMode,
                      (PVOID)&Context);
      if (!KeInsertQueueApc(&Apc,
			    (PVOID)&Event,
			    (PVOID)&Status,
			    IO_NO_INCREMENT))
	{
	  Status = STATUS_THREAD_IS_TERMINATING;
	}
      else
	{
	  Status = KeWaitForSingleObject(&Event,
					 0,
					 KernelMode,
					 FALSE,
                                         NULL);
	}
    }
    ObDereferenceObject(Thread);
  }
  
  return Status;
}

/* EOF */
