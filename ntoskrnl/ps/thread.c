/* $Id: thread.c 14058 2005-03-14 13:18:29Z weiden $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ps/thread.c
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

/* TYPES *******************************************************************/

/* GLOBALS ******************************************************************/

extern LIST_ENTRY PsActiveProcessHead;
extern PEPROCESS PsIdleProcess;

POBJECT_TYPE EXPORTED PsThreadType = NULL;

LONG PiNrThreadsAwaitingReaping = 0;

extern PVOID Ki386InitialStackArray[MAXIMUM_PROCESSORS];

/*
 * PURPOSE: List of threads associated with each priority level
 */
static LIST_ENTRY PriorityListHead[MAXIMUM_PRIORITY];
static ULONG PriorityListMask = 0;
static ULONG IdleProcessorMask = 0;
static BOOLEAN DoneInitYet = FALSE;
static KEVENT PiReaperThreadEvent;
static BOOLEAN PiReaperThreadShouldTerminate = FALSE;

static GENERIC_MAPPING PiThreadMapping = {STANDARD_RIGHTS_READ | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
					  STANDARD_RIGHTS_WRITE | THREAD_TERMINATE | THREAD_SUSPEND_RESUME | THREAD_ALERT |
                      THREAD_SET_INFORMATION | THREAD_SET_CONTEXT,
                      STANDARD_RIGHTS_EXECUTE | SYNCHRONIZE,
					  THREAD_ALL_ACCESS};

/* FUNCTIONS ***************************************************************/

/*
 * @implemented
 */
PKTHREAD STDCALL KeGetCurrentThread(VOID)
{
#ifdef CONFIG_SMP
   ULONG Flags;
   PKTHREAD Thread;
   Ke386SaveFlags(Flags);
   Ke386DisableInterrupts();
   Thread = KeGetCurrentPrcb()->CurrentThread;
   Ke386RestoreFlags(Flags);
   return Thread;
#else
   return(KeGetCurrentPrcb()->CurrentThread);
#endif
}

/*
 * @implemented
 */
HANDLE STDCALL PsGetCurrentThreadId(VOID)
{
   return(PsGetCurrentThread()->Cid.UniqueThread);
}

/*
 * @implemented
 */
ULONG
STDCALL
PsGetThreadFreezeCount(
	PETHREAD Thread
	)
{
	return Thread->Tcb.FreezeCount;
}

/*
 * @implemented
 */
BOOLEAN
STDCALL
PsGetThreadHardErrorsAreDisabled(
    PETHREAD	Thread
	)
{
	return Thread->HardErrorsAreDisabled;
}

/*
 * @implemented
 */
HANDLE
STDCALL
PsGetThreadId(
    PETHREAD	Thread
	)
{
	return Thread->Cid.UniqueThread;
}

/*
 * @implemented
 */
PEPROCESS
STDCALL
PsGetThreadProcess(
    PETHREAD	Thread
	)
{
	return Thread->ThreadsProcess;
}

/*
 * @implemented
 */
HANDLE
STDCALL
PsGetThreadProcessId(
    PETHREAD	Thread
	)
{
	return Thread->Cid.UniqueProcess;
}

/*
 * @implemented
 */
HANDLE
STDCALL
PsGetThreadSessionId(
    PETHREAD	Thread
	)
{
	return (HANDLE)Thread->ThreadsProcess->SessionId;
}

/*
 * @implemented
 */
PTEB
STDCALL
PsGetThreadTeb(
    PETHREAD	Thread
	)
{
	return Thread->Tcb.Teb;
}

/*
 * @implemented
 */
PVOID
STDCALL
PsGetThreadWin32Thread(
    PETHREAD	Thread
	)
{
	return Thread->Tcb.Win32Thread;
}

/*
 * @implemented
 */
KPROCESSOR_MODE
STDCALL
PsGetCurrentThreadPreviousMode (
    	VOID
	)
{
	return (KPROCESSOR_MODE)PsGetCurrentThread()->Tcb.PreviousMode;
}

/*
 * @implemented
 */
PVOID
STDCALL
PsGetCurrentThreadStackBase (
    	VOID
	)
{
	return PsGetCurrentThread()->Tcb.StackBase;
}

/*
 * @implemented
 */
PVOID
STDCALL
PsGetCurrentThreadStackLimit (
    	VOID
	)
{
	return (PVOID)PsGetCurrentThread()->Tcb.StackLimit;
}

/*
 * @implemented
 */
BOOLEAN STDCALL
PsIsThreadTerminating(IN PETHREAD Thread)
{
  return (Thread->HasTerminated ? TRUE : FALSE);
}

/*
 * @unimplemented
 */
BOOLEAN
STDCALL
PsIsSystemThread(
    PETHREAD Thread
    )
{
	UNIMPLEMENTED;
	return FALSE;	
}

/*
 * @implemented
 */
BOOLEAN
STDCALL
PsIsThreadImpersonating(
    PETHREAD	Thread
	)
{
  return Thread->ActiveImpersonationInfo;
}

static VOID
KiRequestReschedule(CCHAR Processor)
{
   PKPCR Pcr;

   Pcr = (PKPCR)(KPCR_BASE + Processor * PAGE_SIZE);
   Pcr->Prcb->QuantumEnd = TRUE;
   KiIpiSendRequest(1 << Processor, IPI_REQUEST_DPC);
}

static VOID
PsInsertIntoThreadList(KPRIORITY Priority, PETHREAD Thread)
{
   ASSERT(THREAD_STATE_READY == Thread->Tcb.State);
   ASSERT(Thread->Tcb.Priority == Priority);
   if (Priority >= MAXIMUM_PRIORITY || Priority < LOW_PRIORITY)
     {
	DPRINT1("Invalid thread priority (%d)\n", Priority);
	KEBUGCHECK(0);
     }
   InsertTailList(&PriorityListHead[Priority], &Thread->Tcb.QueueListEntry);
   PriorityListMask |= (1 << Priority);
}

static VOID PsRemoveFromThreadList(PETHREAD Thread)
{
   ASSERT(THREAD_STATE_READY == Thread->Tcb.State);
   RemoveEntryList(&Thread->Tcb.QueueListEntry);
   if (IsListEmpty(&PriorityListHead[(ULONG)Thread->Tcb.Priority]))
     {
        PriorityListMask &= ~(1 << Thread->Tcb.Priority);
     }
}


VOID PsDumpThreads(BOOLEAN IncludeSystem)
{
   PLIST_ENTRY AThread, AProcess;
   PEPROCESS Process;
   PETHREAD Thread;
   ULONG nThreads = 0;
   
   AProcess = PsActiveProcessHead.Flink;
   while(AProcess != &PsActiveProcessHead)
   {
     Process = CONTAINING_RECORD(AProcess, EPROCESS, ProcessListEntry);
     /* FIXME - skip suspended, ... processes? */
     if((Process != PsInitialSystemProcess) ||
        (Process == PsInitialSystemProcess && IncludeSystem))
     {
       AThread = Process->ThreadListHead.Flink;
       while(AThread != &Process->ThreadListHead)
       {
         Thread = CONTAINING_RECORD(AThread, ETHREAD, ThreadListEntry);

         nThreads++;
         DbgPrint("Thread->Tcb.State %d Affinity %08x Priority %d PID.TID %d.%d Name %.8s Stack: \n",
                  Thread->Tcb.State,
		  Thread->Tcb.Affinity,
		  Thread->Tcb.Priority,
                  Thread->ThreadsProcess->UniqueProcessId,
                  Thread->Cid.UniqueThread,
                  Thread->ThreadsProcess->ImageFileName);
         if(Thread->Tcb.State == THREAD_STATE_READY ||
            Thread->Tcb.State == THREAD_STATE_SUSPENDED ||
            Thread->Tcb.State == THREAD_STATE_BLOCKED)
         {
           ULONG i = 0;
           PULONG Esp = (PULONG)Thread->Tcb.KernelStack;
           PULONG Ebp = (PULONG)Esp[4];
           DbgPrint("Ebp 0x%.8X\n", Ebp);
           while(Ebp != 0 && Ebp >= (PULONG)Thread->Tcb.StackLimit)
           {
             DbgPrint("%.8X %.8X%s", Ebp[0], Ebp[1], (i % 8) == 7 ? "\n" : "  ");
             Ebp = (PULONG)Ebp[0];
             i++;
           }
           if((i % 8) != 0)
           {
             DbgPrint("\n");
           }
         }
         AThread = AThread->Flink;
       }
     }
     AProcess = AProcess->Flink;
   }
}

static PETHREAD PsScanThreadList(KPRIORITY Priority, ULONG Affinity)
{
   PLIST_ENTRY current_entry;
   PETHREAD current;
   ULONG Mask;

   Mask = (1 << Priority);
   if (PriorityListMask & Mask)
     {
       current_entry = PriorityListHead[Priority].Flink;
       while (current_entry != &PriorityListHead[Priority])
         {
           current = CONTAINING_RECORD(current_entry, ETHREAD,
				       Tcb.QueueListEntry);
	   if (current->Tcb.State != THREAD_STATE_READY)
	     {
	       DPRINT1("%d/%d\n", current->Cid.UniqueThread, current->Tcb.State);
	     }
           ASSERT(current->Tcb.State == THREAD_STATE_READY);
           DPRINT("current->Tcb.Affinity %x Affinity %x PID %d %d\n",
	          current->Tcb.Affinity, Affinity, current->Cid.UniqueThread,
	          Priority);
           if (current->Tcb.Affinity & Affinity)
	     {
	       PsRemoveFromThreadList(current);
	       return(current);
	     }
           current_entry = current_entry->Flink;
	 }
     }
   return(NULL);
}

VOID STDCALL
PiWakeupReaperThread(VOID)
{
  KeSetEvent(&PiReaperThreadEvent, 0, FALSE);
}

VOID STDCALL
PiReaperThreadMain(PVOID Ignored)
{
  for(;;)
  {
    KeWaitForSingleObject(&PiReaperThreadEvent,
			  Executive,
			  KernelMode,
			  FALSE,
			  NULL);
    if (PiReaperThreadShouldTerminate)
	{
	  PsTerminateSystemThread(0);
	}
    PsReapThreads();
  }
}

VOID PsDispatchThreadNoLock (ULONG NewThreadStatus)
{
   KPRIORITY CurrentPriority;
   PETHREAD Candidate;
   ULONG Affinity;
   PKTHREAD KCurrentThread = KeGetCurrentThread();
   PETHREAD CurrentThread = CONTAINING_RECORD(KCurrentThread, ETHREAD, Tcb);

   DPRINT("PsDispatchThread() %d/%d/%d/%d\n", KeGetCurrentProcessorNumber(),
	   CurrentThread->Cid.UniqueThread, NewThreadStatus, CurrentThread->Tcb.State);

   CurrentThread->Tcb.State = (UCHAR)NewThreadStatus;
   switch(NewThreadStatus)
   {
     case THREAD_STATE_READY:
	PsInsertIntoThreadList(CurrentThread->Tcb.Priority,
			       CurrentThread);
	break;
     case THREAD_STATE_TERMINATED_1:
	PsQueueThreadReap(CurrentThread);
	break;
   }

   Affinity = 1 << KeGetCurrentProcessorNumber();
   for (CurrentPriority = HIGH_PRIORITY;
	CurrentPriority >= LOW_PRIORITY;
	CurrentPriority--)
     {
	Candidate = PsScanThreadList(CurrentPriority, Affinity);
	if (Candidate == CurrentThread)
	  {
	     Candidate->Tcb.State = THREAD_STATE_RUNNING;
	     KeReleaseDispatcherDatabaseLockFromDpcLevel();	
	     return;
	  }
	if (Candidate != NULL)
	  {
	    PETHREAD OldThread;
	    PKTHREAD IdleThread;

	    DPRINT("Scheduling %x(%d)\n",Candidate, CurrentPriority);

	    Candidate->Tcb.State = THREAD_STATE_RUNNING;

	    OldThread = CurrentThread;
	    CurrentThread = Candidate;
	    IdleThread = KeGetCurrentPrcb()->IdleThread;

	    if (&OldThread->Tcb == IdleThread)
	    {
	       IdleProcessorMask &= ~Affinity;
	    }
	    else if (&CurrentThread->Tcb == IdleThread)
	    {
	       IdleProcessorMask |= Affinity;
	    }

	    MmUpdatePageDir(PsGetCurrentProcess(),(PVOID)CurrentThread->ThreadsProcess, sizeof(EPROCESS));

	    KiArchContextSwitch(&CurrentThread->Tcb, &OldThread->Tcb);
	    return;
	  }
     }
   CPRINT("CRITICAL: No threads are ready (CPU%d)\n", KeGetCurrentProcessorNumber());
   PsDumpThreads(TRUE);
   KEBUGCHECK(0);
}

VOID STDCALL
PsDispatchThread(ULONG NewThreadStatus)
{
   KIRQL oldIrql;

   if (!DoneInitYet || KeGetCurrentPrcb()->IdleThread == NULL)
     {
	return;
     }
   oldIrql = KeAcquireDispatcherDatabaseLock();
   PsDispatchThreadNoLock(NewThreadStatus);
   KeLowerIrql(oldIrql);
}

VOID
PsUnblockThread(PETHREAD Thread, PNTSTATUS WaitStatus, KPRIORITY Increment)
{
  if (THREAD_STATE_TERMINATED_1 == Thread->Tcb.State ||
      THREAD_STATE_TERMINATED_2 == Thread->Tcb.State)
    {
       DPRINT("Can't unblock thread %d because it's terminating\n",
	       Thread->Cid.UniqueThread);
    }
  else if (THREAD_STATE_READY == Thread->Tcb.State ||
           THREAD_STATE_RUNNING == Thread->Tcb.State)
    {
       DPRINT("Can't unblock thread %d because it's ready or running\n",
	       Thread->Cid.UniqueThread);
    }
  else
    {
      ULONG Processor;
      KAFFINITY Affinity;

      /* FIXME: This propably isn't the right way to do it... */
      if (Thread->Tcb.Priority < LOW_REALTIME_PRIORITY &&
          Thread->Tcb.BasePriority < LOW_REALTIME_PRIORITY - 2)
        {
          if (!Thread->Tcb.PriorityDecrement && !Thread->Tcb.DisableBoost)
            {
              Thread->Tcb.Priority = Thread->Tcb.BasePriority + Increment;
              Thread->Tcb.PriorityDecrement = Increment;
            }
        }
      else
        {
          Thread->Tcb.Quantum = Thread->Tcb.ApcState.Process->ThreadQuantum;
        }
     
      if (WaitStatus != NULL)
	{
	  Thread->Tcb.WaitStatus = *WaitStatus;
	}
      Thread->Tcb.State = THREAD_STATE_READY;
      PsInsertIntoThreadList(Thread->Tcb.Priority, Thread);
      Processor = KeGetCurrentProcessorNumber();
      Affinity = Thread->Tcb.Affinity;
      if (!(IdleProcessorMask & (1 << Processor) & Affinity) &&
          (IdleProcessorMask & ~(1 << Processor) & Affinity))
        {
	  ULONG i;
	  for (i = 0; i < KeNumberProcessors - 1; i++)
	    {
	      Processor++;
	      if (Processor >= KeNumberProcessors)
	        {
	          Processor = 0;
	        }
	      if (IdleProcessorMask & (1 << Processor) & Affinity)
	        {
#if 0	        
                  /* FIXME:
                   *   Reschedule the threads on an other processor 
                   */
		  KeReleaseDispatcherDatabaseLockFromDpcLevel();
                  KiRequestReschedule(Processor);
		  KeAcquireDispatcherDatabaseLockAtDpcLevel();
#endif
	          break;
		}
	    }
	} 
    }
}

VOID
STDCALL
PsBlockThread(PNTSTATUS Status, 
              UCHAR Alertable, 
              ULONG WaitMode,
              UCHAR WaitReason)
{
    PKTHREAD Thread = KeGetCurrentThread();
    PKWAIT_BLOCK WaitBlock;

    if (Thread->ApcState.KernelApcPending) {
    
        DPRINT("Dispatching Thread as ready (APC!)\n");
        
        /* Remove Waits */
        WaitBlock = Thread->WaitBlockList;
        while (WaitBlock) {
            RemoveEntryList (&WaitBlock->WaitListEntry);
            WaitBlock = WaitBlock->NextWaitBlock;
        }
        Thread->WaitBlockList = NULL;
        
        /* Dispatch it and return status */
        PsDispatchThreadNoLock (THREAD_STATE_READY);
        if (Status != NULL) *Status = STATUS_KERNEL_APC;

    } else {

        /* Set the Thread Data as Requested */
        DPRINT("Dispatching Thread as blocked\n");
        Thread->Alertable = Alertable;
        Thread->WaitMode = (UCHAR)WaitMode;
        Thread->WaitReason = WaitReason;
        
        /* Dispatch it and return status */
        PsDispatchThreadNoLock(THREAD_STATE_BLOCKED);
        if (Status != NULL) *Status = Thread->WaitStatus;
    }
    
    DPRINT("Releasing Dispatcher Lock\n");
    KfLowerIrql(Thread->WaitIrql);
}

VOID
PsFreezeAllThreads(PEPROCESS Process)
     /*
      * Used by the debugging code to freeze all the process's threads
      * while the debugger is examining their state.
      */
{
  KIRQL oldIrql;
  PLIST_ENTRY current_entry;
  PETHREAD current;

  oldIrql = KeAcquireDispatcherDatabaseLock();
  current_entry = Process->ThreadListHead.Flink;
  while (current_entry != &Process->ThreadListHead)
    {
      current = CONTAINING_RECORD(current_entry, ETHREAD,
				  ThreadListEntry);

      /*
       * We have to be careful here, we can't just set the freeze the
       * thread inside kernel mode since it may be holding a lock.
       */

      current_entry = current_entry->Flink;
    }

    KeReleaseDispatcherDatabaseLock(oldIrql);
}

ULONG
PsEnumThreadsByProcess(PEPROCESS Process)
{
  KIRQL oldIrql;
  PLIST_ENTRY current_entry;
  ULONG Count = 0;

  oldIrql = KeAcquireDispatcherDatabaseLock();

  current_entry = Process->ThreadListHead.Flink;
  while (current_entry != &Process->ThreadListHead)
    {
      Count++;
      current_entry = current_entry->Flink;
    }
  
  KeReleaseDispatcherDatabaseLock(oldIrql);
  return Count;
}

/*
 * @unimplemented
 */
NTSTATUS
STDCALL
PsRemoveCreateThreadNotifyRoutine (
    IN PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    )
{
	UNIMPLEMENTED;
	return STATUS_NOT_IMPLEMENTED;	
}

/*
 * @unimplemented
 */                       
ULONG
STDCALL
PsSetLegoNotifyRoutine(   	
	PVOID LegoNotifyRoutine  	 
	)
{
	UNIMPLEMENTED;
	return 0;
}

/*
 * @implemented
 */                       
VOID
STDCALL
PsSetThreadHardErrorsAreDisabled(
    PETHREAD	Thread,
    BOOLEAN	HardErrorsAreDisabled
	)
{
	Thread->HardErrorsAreDisabled = HardErrorsAreDisabled;
}

/*
 * @implemented
 */                       
VOID
STDCALL
PsSetThreadWin32Thread(
    PETHREAD	Thread,
    PVOID	Win32Thread
	)
{
	Thread->Tcb.Win32Thread = Win32Thread;
}

VOID
PsApplicationProcessorInit(VOID)
{
   KIRQL oldIrql;
   oldIrql = KeAcquireDispatcherDatabaseLock();
   IdleProcessorMask |= (1 << KeGetCurrentProcessorNumber());
   KeReleaseDispatcherDatabaseLock(oldIrql);
}

VOID INIT_FUNCTION
PsPrepareForApplicationProcessorInit(ULONG Id)
{
  PETHREAD IdleThread;
  PKPRCB Prcb = ((PKPCR)((ULONG_PTR)KPCR_BASE + Id * PAGE_SIZE))->Prcb;

  PsInitializeThread(PsIdleProcess,
		     &IdleThread,
		     NULL,
		     KernelMode,
		     FALSE);
  IdleThread->Tcb.State = THREAD_STATE_RUNNING;
  IdleThread->Tcb.FreezeCount = 0;
  IdleThread->Tcb.Affinity = 1 << Id;
  IdleThread->Tcb.UserAffinity = 1 << Id;
  IdleThread->Tcb.Priority = LOW_PRIORITY;
  IdleThread->Tcb.BasePriority = LOW_PRIORITY;
  Prcb->IdleThread = &IdleThread->Tcb;
  Prcb->CurrentThread = &IdleThread->Tcb;

  Ki386InitialStackArray[Id] = (PVOID)IdleThread->Tcb.StackLimit;

  DPRINT("IdleThread for Processor %d has PID %d\n",
	   Id, IdleThread->Cid.UniqueThread);
}

VOID INIT_FUNCTION
PsInitThreadManagment(VOID)
/*
 * FUNCTION: Initialize thread managment
 */
{
   PETHREAD FirstThread;
   ULONG i;

   for (i=0; i < MAXIMUM_PRIORITY; i++)
     {
	InitializeListHead(&PriorityListHead[i]);
     }

   PsThreadType = ExAllocatePool(NonPagedPool,sizeof(OBJECT_TYPE));

   PsThreadType->Tag = TAG('T', 'H', 'R', 'T');
   PsThreadType->TotalObjects = 0;
   PsThreadType->TotalHandles = 0;
   PsThreadType->PeakObjects = 0;
   PsThreadType->PeakHandles = 0;
   PsThreadType->PagedPoolCharge = 0;
   PsThreadType->NonpagedPoolCharge = sizeof(ETHREAD);
   PsThreadType->Mapping = &PiThreadMapping;
   PsThreadType->Dump = NULL;
   PsThreadType->Open = NULL;
   PsThreadType->Close = NULL;
   PsThreadType->Delete = PiDeleteThread;
   PsThreadType->Parse = NULL;
   PsThreadType->Security = NULL;
   PsThreadType->QueryName = NULL;
   PsThreadType->OkayToClose = NULL;
   PsThreadType->Create = NULL;
   PsThreadType->DuplicationNotify = NULL;

   RtlInitUnicodeString(&PsThreadType->TypeName, L"Thread");

   ObpCreateTypeObject(PsThreadType);

   PsInitializeThread(NULL, &FirstThread, NULL, KernelMode, TRUE);
   FirstThread->Tcb.State = THREAD_STATE_RUNNING;
   FirstThread->Tcb.FreezeCount = 0;
   FirstThread->Tcb.UserAffinity = (1 << 0);   /* Set the affinity of the first thread to the boot processor */
   FirstThread->Tcb.Affinity = (1 << 0);
   KeGetCurrentPrcb()->CurrentThread = (PVOID)FirstThread;

   DPRINT("FirstThread %x\n",FirstThread);

   DoneInitYet = TRUE;
}

VOID
PsInitReaperThread(VOID)
{
   PETHREAD ReaperThread;
   KIRQL oldIrql;
   NTSTATUS Status;
   
   /*
    * Create the reaper thread
    */
   PsInitializeThreadReaper();
   KeInitializeEvent(&PiReaperThreadEvent, SynchronizationEvent, FALSE);
   Status = PsInitializeThread(NULL,
			       &ReaperThread,
			       NULL,
			       KernelMode,
			       FALSE);
   if (!NT_SUCCESS(Status))
     {
       DPRINT1("PS: Failed to create reaper thread.\n");
       KEBUGCHECK(0);
     }

   ReaperThread->StartAddress = PiReaperThreadMain;
   Status = KiArchInitThread(&ReaperThread->Tcb, PiReaperThreadMain, NULL);
   if (!NT_SUCCESS(Status))
     {
       DPRINT1("PS: Failed to initialize reaper thread.\n");
       KEBUGCHECK(0);
     }

   oldIrql = KeAcquireDispatcherDatabaseLock ();
   PsUnblockThread(ReaperThread, NULL, 0);
   KeReleaseDispatcherDatabaseLock(oldIrql);
}

/*
 * @implemented
 */
LONG STDCALL
KeSetBasePriorityThread (PKTHREAD	Thread,
			 LONG		Increment)
/*
 * Sets thread's base priority relative to the process' base priority
 * Should only be passed in THREAD_PRIORITY_ constants in pstypes.h
 */
{
   KPRIORITY Priority;
   if (Increment < -2)
     {
       Increment = -2;
     }
   else if (Increment > 2)
     {
       Increment = 2;
     }
   Priority = ((PETHREAD)Thread)->ThreadsProcess->Pcb.BasePriority + Increment;
   if (Priority < LOW_PRIORITY)
   {
     Priority = LOW_PRIORITY;
   }
   else if (Priority >= MAXIMUM_PRIORITY)
     {
       Thread->BasePriority = HIGH_PRIORITY;
     }
   KeSetPriorityThread(Thread, Priority);
   return 1;
}


/*
 * @implemented
 */
KPRIORITY STDCALL
KeSetPriorityThread (PKTHREAD Thread, KPRIORITY Priority)
{
   KPRIORITY OldPriority;
   KIRQL oldIrql;
   PKTHREAD CurrentThread;
   ULONG Mask;
   int i;
   PKPCR Pcr;

   if (Priority < LOW_PRIORITY || Priority >= MAXIMUM_PRIORITY)
     {
	KEBUGCHECK(0);
     }

   oldIrql = KeAcquireDispatcherDatabaseLock();

   OldPriority = Thread->Priority;

   if (OldPriority != Priority)
     {
       CurrentThread = KeGetCurrentThread();
       if (Thread->State == THREAD_STATE_READY)
         {
	   PsRemoveFromThreadList((PETHREAD)Thread);
           Thread->BasePriority = Thread->Priority = (CHAR)Priority;
	   PsInsertIntoThreadList(Priority, (PETHREAD)Thread);
	   if (CurrentThread->Priority < Priority)
	     {
               PsDispatchThreadNoLock(THREAD_STATE_READY);
               KeLowerIrql(oldIrql);
	       return (OldPriority);
	     }
	 }
       else if (Thread->State == THREAD_STATE_RUNNING)
         {
           Thread->BasePriority = Thread->Priority = (CHAR)Priority;
	   if (Priority < OldPriority)
	     {
	       /* Check for threads with a higher priority */
	       Mask = ~((1 << (Priority + 1)) - 1);
	       if (PriorityListMask & Mask)
	         {
		   if (Thread == CurrentThread)
		     {
                       PsDispatchThreadNoLock(THREAD_STATE_READY);
                       KeLowerIrql(oldIrql);
	               return (OldPriority);
		     }
		   else
		     {
		       for (i = 0; i < KeNumberProcessors; i++)
		       {
			  Pcr = (PKPCR)(KPCR_BASE + i * PAGE_SIZE);
			  if (Pcr->Prcb->CurrentThread == Thread)
			  {
			    KeReleaseDispatcherDatabaseLockFromDpcLevel();
                            KiRequestReschedule(i);
                            KeLowerIrql(oldIrql);
	                    return (OldPriority);
			  }
		       }
		     }
		 }
	     }
	 }
       else
         {
            Thread->BasePriority = Thread->Priority = (CHAR)Priority;
         }
     }
   KeReleaseDispatcherDatabaseLock(oldIrql);
   return(OldPriority);
}

/*
 * @unimplemented
 */
NTSTATUS STDCALL
KeSetAffinityThread(PKTHREAD	Thread,
		    KAFFINITY	Affinity)
/*
 * Sets thread's affinity
 */
{
    KIRQL oldIrql;
    ULONG i;
    PKPCR Pcr;
    KAFFINITY ProcessorMask;

    DPRINT("KeSetAffinityThread(Thread %x, Affinity %x)\n", Thread, Affinity);

    ASSERT(Affinity & ((1 << KeNumberProcessors) - 1));

    oldIrql = KeAcquireDispatcherDatabaseLock();

    Thread->UserAffinity = Affinity;
    if (Thread->SystemAffinityActive == FALSE)
    {
       Thread->Affinity = Affinity;
       if (Thread->State == THREAD_STATE_RUNNING)
       {
          ProcessorMask = 1 << KeGetCurrentKPCR()->ProcessorNumber;
          if (Thread == KeGetCurrentThread())
	  {
	     if (!(Affinity & ProcessorMask))
	     {
                PsDispatchThreadNoLock(THREAD_STATE_READY);
                KeLowerIrql(oldIrql);
		return STATUS_SUCCESS;
	     }
	  }
	  else
	  {
	     for (i = 0; i < KeNumberProcessors; i++)
	     {
		Pcr = (PKPCR)(KPCR_BASE + i * PAGE_SIZE);
		if (Pcr->Prcb->CurrentThread == Thread)
		{
		   if (!(Affinity & ProcessorMask))
		   {
		      KeReleaseDispatcherDatabaseLockFromDpcLevel();
                      KiRequestReschedule(i);
                      KeLowerIrql(oldIrql);
		      return STATUS_SUCCESS;
		   }
		   break;
		}
	     }
	     ASSERT (i < KeNumberProcessors);
	  }
       }
    }
    KeReleaseDispatcherDatabaseLock(oldIrql);
    return STATUS_SUCCESS;
}


/**********************************************************************
 *	NtOpenThread/4
 *
 *	@implemented
 */
NTSTATUS STDCALL
NtOpenThread(OUT PHANDLE ThreadHandle,
	     IN	ACCESS_MASK DesiredAccess,
	     IN	POBJECT_ATTRIBUTES ObjectAttributes  OPTIONAL,
	     IN	PCLIENT_ID ClientId  OPTIONAL)
{
   KPROCESSOR_MODE PreviousMode;
   CLIENT_ID SafeClientId;
   HANDLE hThread;
   NTSTATUS Status = STATUS_SUCCESS;

   PAGED_CODE();

   PreviousMode = ExGetPreviousMode();

   if(PreviousMode != KernelMode)
   {
     _SEH_TRY
     {
       ProbeForWrite(ThreadHandle,
                     sizeof(HANDLE),
                     sizeof(ULONG));
       if(ClientId != NULL)
       {
         ProbeForRead(ClientId,
                      sizeof(CLIENT_ID),
                      sizeof(ULONG));
         SafeClientId = *ClientId;
         ClientId = &SafeClientId;
       }
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

   if(!((ObjectAttributes == NULL) ^ (ClientId == NULL)))
   {
     DPRINT("NtOpenThread should be called with either ObjectAttributes or ClientId!\n");
     return STATUS_INVALID_PARAMETER;
   }

   if(ClientId != NULL)
   {
     PETHREAD Thread;

     Status = PsLookupThreadByThreadId(ClientId->UniqueThread,
                                       &Thread);
     if(NT_SUCCESS(Status))
     {
       Status = ObInsertObject(Thread,
                               NULL,
                               DesiredAccess,
                               0,
                               NULL,
                               &hThread);

       ObDereferenceObject(Thread);
     }
   }
   else
   {
     Status = ObOpenObjectByName(ObjectAttributes,
                                 PsThreadType,
                                 NULL,
                                 PreviousMode,
                                 DesiredAccess,
                                 NULL,
                                 &hThread);
   }

   if(NT_SUCCESS(Status))
   {
     _SEH_TRY
     {
       *ThreadHandle = hThread;
     }
     _SEH_HANDLE
     {
       Status = _SEH_GetExceptionCode();
     }
     _SEH_END;
   }

   return Status;
}

NTSTATUS STDCALL
NtYieldExecution(VOID)
{
  PsDispatchThread(THREAD_STATE_READY);
  return(STATUS_SUCCESS);
}


/*
 * NOT EXPORTED
 */
NTSTATUS STDCALL
NtTestAlert(VOID)
{
  /* Check and Alert Thread if needed */
  return KeTestAlertThread(ExGetPreviousMode()) ? STATUS_ALERTED : STATUS_SUCCESS;
}


/*
 * @implemented
 */
NTSTATUS STDCALL
PsLookupProcessThreadByCid(IN PCLIENT_ID Cid,
			   OUT PEPROCESS *Process OPTIONAL,
			   OUT PETHREAD *Thread)
{
  PHANDLE_TABLE_ENTRY CidEntry;
  PETHREAD FoundThread;

  PAGED_CODE();

  ASSERT(Thread);
  ASSERT(Cid);

  CidEntry = PsLookupCidHandle(Cid->UniqueThread, PsThreadType, (PVOID*)&FoundThread);
  if(CidEntry != NULL)
  {
    ObReferenceObject(FoundThread);

    PsUnlockCidHandle(CidEntry);

    if(Process != NULL)
    {
      *Process = FoundThread->ThreadsProcess;
    }
    *Thread = FoundThread;
    return STATUS_SUCCESS;
  }

  return STATUS_INVALID_PARAMETER;
}


/*
 * @implemented
 */
NTSTATUS STDCALL
PsLookupThreadByThreadId(IN HANDLE ThreadId,
			 OUT PETHREAD *Thread)
{
  PHANDLE_TABLE_ENTRY CidEntry;
  PETHREAD FoundThread;

  PAGED_CODE();

  ASSERT(Thread);

  CidEntry = PsLookupCidHandle(ThreadId, PsThreadType, (PVOID*)&FoundThread);
  if(CidEntry != NULL)
  {
    ObReferenceObject(FoundThread);

    PsUnlockCidHandle(CidEntry);

    *Thread = FoundThread;
    return STATUS_SUCCESS;
  }

  return STATUS_INVALID_PARAMETER;
}

/* EOF */
