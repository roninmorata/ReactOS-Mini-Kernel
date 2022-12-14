/* $Id: ntfunc.c 14017 2005-03-13 17:03:42Z hbirr $
 * 
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/cm/ntfunc.c
 * PURPOSE:         Ntxxx function for registry access
 *
 * PROGRAMMERS:     No programmer listed.
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

#include "cm.h"


/* GLOBALS ******************************************************************/

extern POBJECT_TYPE  CmiKeyType;
extern PREGISTRY_HIVE  CmiVolatileHive;
extern LIST_ENTRY CmiKeyObjectListHead;

static BOOLEAN CmiRegistryInitialized = FALSE;

LIST_ENTRY CmiCallbackHead;
FAST_MUTEX CmiCallbackLock;

/* FUNCTIONS ****************************************************************/

/*
 * @implemented
 */
NTSTATUS STDCALL
CmRegisterCallback(IN PEX_CALLBACK_FUNCTION Function,
                   IN PVOID Context,
                   IN OUT PLARGE_INTEGER Cookie)
{
  PREGISTRY_CALLBACK Callback;
  
  PAGED_CODE();
  
  ASSERT(Function && Cookie);
  
  Callback = ExAllocatePoolWithTag(PagedPool,
                                   sizeof(REGISTRY_CALLBACK),
                                   TAG('C', 'M', 'c', 'b'));
  if(Callback != NULL)
  {
    /* initialize the callback */
    ExInitializeRundownProtection(&Callback->RundownRef);
    Callback->Function = Function;
    Callback->Context = Context;
    Callback->PendingDelete = FALSE;
    
    /* add it to the callback list and receive a cookie for the callback */
    ExAcquireFastMutex(&CmiCallbackLock);
    /* FIXME - to receive a unique cookie we'll just return the pointer to the
       callback object */
    Callback->Cookie.QuadPart = (ULONG_PTR)Callback;
    InsertTailList(&CmiCallbackHead, &Callback->ListEntry);

    ExReleaseFastMutex(&CmiCallbackLock);
    
    *Cookie = Callback->Cookie;
    return STATUS_SUCCESS;
  }

  return STATUS_INSUFFICIENT_RESOURCES;
}


/*
 * @implemented
 */
NTSTATUS STDCALL
CmUnRegisterCallback(IN LARGE_INTEGER Cookie)
{
  PLIST_ENTRY CurrentEntry;
  
  PAGED_CODE();

  ExAcquireFastMutex(&CmiCallbackLock);

  for(CurrentEntry = CmiCallbackHead.Flink;
      CurrentEntry != &CmiCallbackHead;
      CurrentEntry = CurrentEntry->Flink)
  {
    PREGISTRY_CALLBACK CurrentCallback;

    CurrentCallback = CONTAINING_RECORD(CurrentEntry, REGISTRY_CALLBACK, ListEntry);
    if(CurrentCallback->Cookie.QuadPart == Cookie.QuadPart)
    {
      if(!CurrentCallback->PendingDelete)
      {
        /* found the callback, don't unlink it from the list yet so we don't screw
           the calling loop */
        CurrentCallback->PendingDelete = TRUE;
        ExReleaseFastMutex(&CmiCallbackLock);

        /* if the callback is currently executing, wait until it finished */
        ExWaitForRundownProtectionRelease(&CurrentCallback->RundownRef);

        /* time to unlink it. It's now safe because every attempt to acquire a
           runtime protection on this callback will fail */
        ExAcquireFastMutex(&CmiCallbackLock);
        RemoveEntryList(&CurrentCallback->ListEntry);
        ExReleaseFastMutex(&CmiCallbackLock);

        /* free the callback */
        ExFreePool(CurrentCallback);
        return STATUS_SUCCESS;
      }
      else
      {
        /* pending delete, pretend like it already is deleted */
        ExReleaseFastMutex(&CmiCallbackLock);
        return STATUS_UNSUCCESSFUL;
      }
    }
  }
  
  ExReleaseFastMutex(&CmiCallbackLock);

  return STATUS_UNSUCCESSFUL;
}


NTSTATUS
CmiCallRegisteredCallbacks(IN REG_NOTIFY_CLASS Argument1,
                           IN PVOID Argument2)
{
  PLIST_ENTRY CurrentEntry;
  
  PAGED_CODE();
  
  ExAcquireFastMutex(&CmiCallbackLock);

  for(CurrentEntry = CmiCallbackHead.Flink;
      CurrentEntry != &CmiCallbackHead;
      CurrentEntry = CurrentEntry->Flink)
  {
    PREGISTRY_CALLBACK CurrentCallback;

    CurrentCallback = CONTAINING_RECORD(CurrentEntry, REGISTRY_CALLBACK, ListEntry);
    if(!CurrentCallback->PendingDelete &&
       ExAcquireRundownProtectionEx(&CurrentCallback->RundownRef, 1))
    {
      NTSTATUS Status;
      
      /* don't hold locks during the callbacks! */
      ExReleaseFastMutex(&CmiCallbackLock);
      
      Status = CurrentCallback->Function(CurrentCallback->Context,
                                         Argument1,
                                         Argument2);
      if(!NT_SUCCESS(Status))
      {
        /* one callback returned failure, don't call any more callbacks */
        return Status;
      }

      ExAcquireFastMutex(&CmiCallbackLock);
      /* don't release the rundown protection before holding the callback lock
         so the pointer to the next callback isn't cleared in case this callback
         get's deleted */
      ExReleaseRundownProtectionEx(&CurrentCallback->RundownRef, 1);
    }
  }
  
  ExReleaseFastMutex(&CmiCallbackLock);
  
  return STATUS_SUCCESS;
}


NTSTATUS STDCALL
NtCreateKey(OUT PHANDLE KeyHandle,
	    IN ACCESS_MASK DesiredAccess,
	    IN POBJECT_ATTRIBUTES ObjectAttributes,
	    IN ULONG TitleIndex,
	    IN PUNICODE_STRING Class,
	    IN ULONG CreateOptions,
	    OUT PULONG Disposition)
{
  UNICODE_STRING RemainingPath;
  PKEY_OBJECT KeyObject;
  NTSTATUS Status;
  PVOID Object;
  PWSTR Start;
  unsigned i;
  
  PAGED_CODE();

  DPRINT("NtCreateKey (Name %wZ  KeyHandle %x  Root %x)\n",
	 ObjectAttributes->ObjectName,
	 KeyHandle,
	 ObjectAttributes->RootDirectory);

  Status = ObFindObject(ObjectAttributes,
			&Object,
			&RemainingPath,
			CmiKeyType);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ObFindObject failed, Status: 0x%x\n", Status);
      return(Status);
    }

  DPRINT("RemainingPath %wZ\n", &RemainingPath);

  if (RemainingPath.Length == 0)
    {
      /* Fail if the key has been deleted */
      if (((PKEY_OBJECT) Object)->Flags & KO_MARKED_FOR_DELETE)
	{
	  ObDereferenceObject(Object);
	  RtlFreeUnicodeString(&RemainingPath);
	  DPRINT("Object marked for delete!\n");
	  return(STATUS_UNSUCCESSFUL);
	}

      if (Disposition)
	*Disposition = REG_OPENED_EXISTING_KEY;

      Status = ObCreateHandle(PsGetCurrentProcess(),
			      Object,
			      DesiredAccess,
			      TRUE,
			      KeyHandle);

      DPRINT("ObCreateHandle failed Status 0x%x\n", Status);
      ObDereferenceObject(Object);
      RtlFreeUnicodeString(&RemainingPath);
      return Status;
    }

  /* If RemainingPath contains \ we must return error
     because NtCreateKey doesn't create trees */
  Start = RemainingPath.Buffer;
  if (*Start == L'\\')
    Start++;

  for (i = 1; i < RemainingPath.Length / sizeof(WCHAR); i++)
    {
      if (L'\\' == RemainingPath.Buffer[i])
        {
          ObDereferenceObject(Object);
          DPRINT1("NtCreateKey() doesn't create trees! (found \'\\\' in remaining path: \"%wZ\"!)\n", &RemainingPath);
          RtlFreeUnicodeString(&RemainingPath);
          return STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

  DPRINT("RemainingPath %S  ParentObject %x\n", RemainingPath.Buffer, Object);

  Status = ObCreateObject(ExGetPreviousMode(),
			  CmiKeyType,
			  NULL,
			  ExGetPreviousMode(),
			  NULL,
			  sizeof(KEY_OBJECT),
			  0,
			  0,
			  (PVOID*)&KeyObject);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1("ObCreateObject() failed!\n");
      return(Status);
    }

  Status = ObInsertObject((PVOID)KeyObject,
			  NULL,
			  DesiredAccess,
			  0,
			  NULL,
			  KeyHandle);
  if (!NT_SUCCESS(Status))
    {
      ObDereferenceObject(KeyObject);
      RtlFreeUnicodeString(&RemainingPath);
      DPRINT1("ObInsertObject() failed!\n");
      return(Status);
    }

  KeyObject->ParentKey = Object;

  if (CreateOptions & REG_OPTION_VOLATILE)
    KeyObject->RegistryHive = CmiVolatileHive;
  else
    KeyObject->RegistryHive = KeyObject->ParentKey->RegistryHive;

  KeyObject->Flags = 0;
  KeyObject->NumberOfSubKeys = 0;
  KeyObject->SizeOfSubKeys = 0;
  KeyObject->SubKeys = NULL;

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  InsertTailList(&CmiKeyObjectListHead, &KeyObject->ListEntry);

  /* add key to subkeys of parent if needed */
  Status = CmiAddSubKey(KeyObject->RegistryHive,
			KeyObject->ParentKey,
			KeyObject,
			&RemainingPath,
			TitleIndex,
			Class,
			CreateOptions);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("CmiAddSubKey() failed (Status %lx)\n", Status);
      /* Release hive lock */
      ExReleaseResourceLite(&CmiRegistryLock);
      KeLeaveCriticalRegion();
      ObDereferenceObject(KeyObject);
      ObDereferenceObject(Object);
      RtlFreeUnicodeString(&RemainingPath);
      return STATUS_UNSUCCESSFUL;
    }

  if (Start == RemainingPath.Buffer)
    {
      KeyObject->Name = RemainingPath;
    }
  else
    {
      RtlpCreateUnicodeString(&KeyObject->Name, Start, NonPagedPool);
      RtlFreeUnicodeString(&RemainingPath);
    }

  if (KeyObject->RegistryHive == KeyObject->ParentKey->RegistryHive)
    {
      KeyObject->KeyCell->ParentKeyOffset = KeyObject->ParentKey->KeyCellOffset;
      KeyObject->KeyCell->SecurityKeyOffset = KeyObject->ParentKey->KeyCell->SecurityKeyOffset;
    }
  else
    {
      KeyObject->KeyCell->ParentKeyOffset = -1;
      KeyObject->KeyCell->SecurityKeyOffset = -1;
      /* This key must remain in memory unless it is deleted
	 or file is unloaded */
      ObReferenceObjectByPointer(KeyObject,
				 STANDARD_RIGHTS_REQUIRED,
				 NULL,
				 UserMode);
    }

  CmiAddKeyToList(KeyObject->ParentKey, KeyObject);

  VERIFY_KEY_OBJECT(KeyObject);

  /* Release hive lock */
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  
  ObDereferenceObject(Object);

  if (Disposition)
    *Disposition = REG_CREATED_NEW_KEY;

  CmiSyncHives();

  return Status;
}


NTSTATUS STDCALL
NtDeleteKey(IN HANDLE KeyHandle)
{
  KPROCESSOR_MODE PreviousMode;
  PKEY_OBJECT KeyObject;
  NTSTATUS Status;
  
  PAGED_CODE();

  DPRINT1("NtDeleteKey(KeyHandle %x) called\n", KeyHandle);
  
  PreviousMode = ExGetPreviousMode();

  /* Verify that the handle is valid and is a registry key */
  Status = ObReferenceObjectByHandle(KeyHandle,
				     DELETE,
				     CmiKeyType,
				     PreviousMode,
				     (PVOID *)&KeyObject,
				     NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1("ObReferenceObjectByHandle() failed (Status %lx)\n", Status);
      return Status;
    }

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  VERIFY_KEY_OBJECT(KeyObject);

  /* Check for subkeys */
  if (KeyObject->NumberOfSubKeys != 0)
    {
      Status = STATUS_CANNOT_DELETE;
    }
  else
    {
      /* Set the marked for delete bit in the key object */
      KeyObject->Flags |= KO_MARKED_FOR_DELETE;
      Status = STATUS_SUCCESS;
    }

  /* Release hive lock */
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  DPRINT1("PointerCount %lu\n", ObGetObjectPointerCount((PVOID)KeyObject));

  /* Dereference the object */
  ObDereferenceObject(KeyObject);
  /* Remove the keep-alive reference */
  ObDereferenceObject(KeyObject);

  if (KeyObject->RegistryHive != KeyObject->ParentKey->RegistryHive)
    ObDereferenceObject(KeyObject);

  DPRINT("PointerCount %lu\n", ObGetObjectPointerCount((PVOID)KeyObject));
  DPRINT("HandleCount %lu\n", ObGetObjectHandleCount((PVOID)KeyObject));

  /*
   * Note:
   * Hive-Synchronization will not be triggered here. This is done in
   * CmiObjectDelete() (in regobj.c) after all key-related structures
   * have been released.
   */

  return Status;
}


NTSTATUS STDCALL
NtEnumerateKey(IN HANDLE KeyHandle,
	       IN ULONG Index,
	       IN KEY_INFORMATION_CLASS KeyInformationClass,
	       OUT PVOID KeyInformation,
	       IN ULONG Length,
	       OUT PULONG ResultLength)
{
  PKEY_OBJECT KeyObject;
  PKEY_OBJECT SubKeyObject;
  PREGISTRY_HIVE  RegistryHive;
  PKEY_CELL  KeyCell, SubKeyCell;
  PHASH_TABLE_CELL  HashTableBlock;
  PKEY_BASIC_INFORMATION  BasicInformation;
  PKEY_NODE_INFORMATION  NodeInformation;
  PKEY_FULL_INFORMATION  FullInformation;
  PDATA_CELL ClassCell;
  ULONG NameSize, ClassSize;
  NTSTATUS Status;
  
  PAGED_CODE();

  DPRINT("KH %x  I %d  KIC %x KI %x  L %d  RL %x\n",
	 KeyHandle,
	 Index,
	 KeyInformationClass,
	 KeyInformation,
	 Length,
	 ResultLength);

  /* Verify that the handle is valid and is a registry key */
  Status = ObReferenceObjectByHandle(KeyHandle,
		KEY_ENUMERATE_SUB_KEYS,
		CmiKeyType,
		UserMode,
		(PVOID *) &KeyObject,
		NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ObReferenceObjectByHandle() failed with status %x\n", Status);
      return(Status);
    }

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceSharedLite(&CmiRegistryLock, TRUE);

  VERIFY_KEY_OBJECT(KeyObject);

  /* Get pointer to KeyCell */
  KeyCell = KeyObject->KeyCell;
  RegistryHive = KeyObject->RegistryHive;

  SubKeyObject = NULL;

  /* Check for hightest possible sub key index */
  if (Index >= KeyCell->NumberOfSubKeys + KeyObject->NumberOfSubKeys)
    {
      ExReleaseResourceLite(&CmiRegistryLock);
      KeLeaveCriticalRegion();
      ObDereferenceObject(KeyObject);
      DPRINT("No more volatile entries\n");
      return STATUS_NO_MORE_ENTRIES;
    }

  /* Get pointer to SubKey */
  if (Index >= KeyCell->NumberOfSubKeys)
    {
      PKEY_OBJECT CurKey = NULL;
      ULONG i;
      ULONG j;

      /* Search for volatile or 'foreign' keys */
      j = KeyCell->NumberOfSubKeys;
      for (i = 0; i < KeyObject->NumberOfSubKeys; i++)
	{
	  CurKey = KeyObject->SubKeys[i];
	  if (CurKey->RegistryHive != RegistryHive)
	    {
	      if (j == Index)
		break;
	      j++;
	    }
	}

      if (i >= KeyObject->NumberOfSubKeys)
	{
	  ExReleaseResourceLite(&CmiRegistryLock);
	  KeLeaveCriticalRegion();
	  ObDereferenceObject(KeyObject);
	  DPRINT("No more non-volatile entries\n");
	  return STATUS_NO_MORE_ENTRIES;
	}

      SubKeyObject = CurKey;
      SubKeyCell = CurKey->KeyCell;
    }
  else
    {
      if (KeyCell->HashTableOffset == (BLOCK_OFFSET)-1)
	{
	  ExReleaseResourceLite(&CmiRegistryLock);
	  KeLeaveCriticalRegion();
	  ObDereferenceObject(KeyObject);
	  return STATUS_NO_MORE_ENTRIES;
	}

      HashTableBlock = CmiGetCell (RegistryHive, KeyCell->HashTableOffset, NULL);
      if (HashTableBlock == NULL)
	{
	  DPRINT("CmiGetBlock() failed\n");
	  ExReleaseResourceLite(&CmiRegistryLock);
	  KeLeaveCriticalRegion();
	  ObDereferenceObject(KeyObject);
	  return STATUS_UNSUCCESSFUL;
	}

      SubKeyCell = CmiGetKeyFromHashByIndex(RegistryHive,
					    HashTableBlock,
					    Index);
    }

  if (SubKeyCell == NULL)
    {
      ExReleaseResourceLite(&CmiRegistryLock);
      KeLeaveCriticalRegion();
      ObDereferenceObject(KeyObject);
      DPRINT("No more entries\n");
      return STATUS_NO_MORE_ENTRIES;
    }

  Status = STATUS_SUCCESS;
  switch (KeyInformationClass)
    {
      case KeyBasicInformation:
	/* Check size of buffer */
	if (SubKeyObject != NULL)
	  {
	    NameSize = SubKeyObject->Name.Length;
	  }
	else
	  {
	    NameSize = SubKeyCell->NameSize;
	    if (SubKeyCell->Flags & REG_KEY_NAME_PACKED)
	      {
		NameSize *= sizeof(WCHAR);
	      }
	  }

	*ResultLength = FIELD_OFFSET(KEY_BASIC_INFORMATION, Name[0]) + NameSize;

	/*
	 * NOTE: It's perfetly valid to call NtEnumerateKey to get
         * all the information but name. Actually the NT4 sound
         * framework does that while querying parameters from registry.
         * -- Filip Navara, 19/07/2004
         */
	if (Length < FIELD_OFFSET(KEY_BASIC_INFORMATION, Name[0]))
	  {
	    Status = STATUS_BUFFER_TOO_SMALL;
	  }
	else
	  {
	    /* Fill buffer with requested info */
	    BasicInformation = (PKEY_BASIC_INFORMATION) KeyInformation;
	    BasicInformation->LastWriteTime.u.LowPart = SubKeyCell->LastWriteTime.u.LowPart;
	    BasicInformation->LastWriteTime.u.HighPart = SubKeyCell->LastWriteTime.u.HighPart;
	    BasicInformation->TitleIndex = Index;
	    BasicInformation->NameLength = NameSize;

	    if (Length - FIELD_OFFSET(KEY_BASIC_INFORMATION, Name[0]) < NameSize)
	      {
	        NameSize = Length - FIELD_OFFSET(KEY_BASIC_INFORMATION, Name[0]);
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }

	    if (SubKeyObject != NULL)
	      {
		RtlCopyMemory(BasicInformation->Name,
			      SubKeyObject->Name.Buffer,
			      NameSize);
	      }
	    else
	      {
		if (SubKeyCell->Flags & REG_KEY_NAME_PACKED)
		  {
		    CmiCopyPackedName(BasicInformation->Name,
				      SubKeyCell->Name,
				      NameSize / sizeof(WCHAR));
		  }
		else
		  {
		    RtlCopyMemory(BasicInformation->Name,
				  SubKeyCell->Name,
				  NameSize);
		  }
	      }
	  }
	break;

      case KeyNodeInformation:
	/* Check size of buffer */
	if (SubKeyObject != NULL)
	  {
	    NameSize = SubKeyObject->Name.Length;
	  }
	else
	  {
	    NameSize = SubKeyCell->NameSize;
	    if (SubKeyCell->Flags & REG_KEY_NAME_PACKED)
	      {
		NameSize *= sizeof(WCHAR);
	      }
	  }
	ClassSize = SubKeyCell->ClassSize;

	*ResultLength = FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]) +
	  NameSize + ClassSize;

	if (Length < FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]))
	  {
	    Status = STATUS_BUFFER_TOO_SMALL;
	  }
	else
	  {
	    /* Fill buffer with requested info */
	    NodeInformation = (PKEY_NODE_INFORMATION) KeyInformation;
	    NodeInformation->LastWriteTime.u.LowPart = SubKeyCell->LastWriteTime.u.LowPart;
	    NodeInformation->LastWriteTime.u.HighPart = SubKeyCell->LastWriteTime.u.HighPart;
	    NodeInformation->TitleIndex = Index;
	    NodeInformation->ClassOffset = sizeof(KEY_NODE_INFORMATION) + NameSize;
	    NodeInformation->ClassLength = SubKeyCell->ClassSize;
	    NodeInformation->NameLength = NameSize;

	    if (Length - FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]) < NameSize)
	      {
	        NameSize = Length - FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]);
	        ClassSize = 0;
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }
	    else if (Length - FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]) - 
	             NameSize < ClassSize)
	      {
	        ClassSize = Length - FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]) - 
	                    NameSize;
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }

	    if (SubKeyObject != NULL)
	      {
		RtlCopyMemory(NodeInformation->Name,
			      SubKeyObject->Name.Buffer,
			      NameSize);
	      }
	    else
	      {
		if (SubKeyCell->Flags & REG_KEY_NAME_PACKED)
		  {
		    CmiCopyPackedName(NodeInformation->Name,
				      SubKeyCell->Name,
				      NameSize / sizeof(WCHAR));
		  }
		else
		  {
		    RtlCopyMemory(NodeInformation->Name,
				  SubKeyCell->Name,
				  NameSize);
		  }
	      }

	    if (ClassSize != 0)
	      {
		ClassCell = CmiGetCell (KeyObject->RegistryHive,
					SubKeyCell->ClassNameOffset,
					NULL);
		RtlCopyMemory (NodeInformation->Name + SubKeyCell->NameSize,
			       ClassCell->Data,
			       ClassSize);
	      }
	  }
	break;

      case KeyFullInformation:
	ClassSize = SubKeyCell->ClassSize;

	*ResultLength = FIELD_OFFSET(KEY_FULL_INFORMATION, Class[0]) +
	  ClassSize;

	/* Check size of buffer */
	if (Length < FIELD_OFFSET(KEY_FULL_INFORMATION, Class[0]))
	  {
	    Status = STATUS_BUFFER_TOO_SMALL;
	  }
	else
	  {
	    /* Fill buffer with requested info */
	    FullInformation = (PKEY_FULL_INFORMATION) KeyInformation;
	    FullInformation->LastWriteTime.u.LowPart = SubKeyCell->LastWriteTime.u.LowPart;
	    FullInformation->LastWriteTime.u.HighPart = SubKeyCell->LastWriteTime.u.HighPart;
	    FullInformation->TitleIndex = Index;
	    FullInformation->ClassOffset = sizeof(KEY_FULL_INFORMATION) -
	      sizeof(WCHAR);
	    FullInformation->ClassLength = SubKeyCell->ClassSize;
	    FullInformation->SubKeys = CmiGetNumberOfSubKeys(KeyObject); //SubKeyCell->NumberOfSubKeys;
	    FullInformation->MaxNameLen = CmiGetMaxNameLength(KeyObject);
	    FullInformation->MaxClassLen = CmiGetMaxClassLength(KeyObject);
	    FullInformation->Values = SubKeyCell->NumberOfValues;
	    FullInformation->MaxValueNameLen =
	      CmiGetMaxValueNameLength(RegistryHive, SubKeyCell);
	    FullInformation->MaxValueDataLen =
	      CmiGetMaxValueDataLength(RegistryHive, SubKeyCell);

	    if (Length - FIELD_OFFSET(KEY_FULL_INFORMATION, Class[0]) < ClassSize)
	      {
	        ClassSize = Length - FIELD_OFFSET(KEY_FULL_INFORMATION, Class[0]);
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }

	    if (ClassSize != 0)
	      {
		ClassCell = CmiGetCell (KeyObject->RegistryHive,
					SubKeyCell->ClassNameOffset,
					NULL);
		RtlCopyMemory (FullInformation->Class,
			       ClassCell->Data,
			       ClassSize);
	      }
	  }
	break;

      default:
	DPRINT1("Not handling 0x%x\n", KeyInformationClass);
	break;
    }

  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();
  ObDereferenceObject(KeyObject);

  DPRINT("Returning status %x\n", Status);

  return(Status);
}


NTSTATUS STDCALL
NtEnumerateValueKey(IN HANDLE KeyHandle,
	IN ULONG Index,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	OUT PVOID KeyValueInformation,
	IN ULONG Length,
	OUT PULONG ResultLength)
{
  NTSTATUS  Status;
  PKEY_OBJECT  KeyObject;
  PREGISTRY_HIVE  RegistryHive;
  PKEY_CELL  KeyCell;
  PVALUE_CELL  ValueCell;
  PDATA_CELL  DataCell;
  ULONG NameSize, DataSize;
  PKEY_VALUE_BASIC_INFORMATION  ValueBasicInformation;
  PKEY_VALUE_PARTIAL_INFORMATION  ValuePartialInformation;
  PKEY_VALUE_FULL_INFORMATION  ValueFullInformation;
  
  PAGED_CODE();

  DPRINT("KH %x  I %d  KVIC %x  KVI %x  L %d  RL %x\n",
	 KeyHandle,
	 Index,
	 KeyValueInformationClass,
	 KeyValueInformation,
	 Length,
	 ResultLength);

  /*  Verify that the handle is valid and is a registry key  */
  Status = ObReferenceObjectByHandle(KeyHandle,
		KEY_QUERY_VALUE,
		CmiKeyType,
		UserMode,
		(PVOID *) &KeyObject,
		NULL);

  if (!NT_SUCCESS(Status))
    {
      return Status;
    }

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceSharedLite(&CmiRegistryLock, TRUE);

  VERIFY_KEY_OBJECT(KeyObject);

  /* Get pointer to KeyCell */
  KeyCell = KeyObject->KeyCell;
  RegistryHive = KeyObject->RegistryHive;

  /* Get Value block of interest */
  Status = CmiGetValueFromKeyByIndex(RegistryHive,
		KeyCell,
		Index,
		&ValueCell);

  if (!NT_SUCCESS(Status))
    {
      ExReleaseResourceLite(&CmiRegistryLock);
      KeLeaveCriticalRegion();
      ObDereferenceObject(KeyObject);
      return Status;
    }

  if (ValueCell != NULL)
    {
      switch (KeyValueInformationClass)
        {
        case KeyValueBasicInformation:
	  NameSize = ValueCell->NameSize;
	  if (ValueCell->Flags & REG_VALUE_NAME_PACKED)
	    {
	      NameSize *= sizeof(WCHAR);
	    }

          *ResultLength = FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name[0]) + NameSize;

          if (Length < FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name[0]))
            {
              Status = STATUS_BUFFER_TOO_SMALL;
            }
          else
            {
              ValueBasicInformation = (PKEY_VALUE_BASIC_INFORMATION) 
                KeyValueInformation;
              ValueBasicInformation->TitleIndex = 0;
              ValueBasicInformation->Type = ValueCell->DataType;
	      ValueBasicInformation->NameLength = NameSize;

	      if (Length - FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name[0]) <
	          NameSize)
	        {
	          NameSize = Length - FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name[0]);
	          Status = STATUS_BUFFER_OVERFLOW;
	          CHECKPOINT;
	        }

              if (ValueCell->Flags & REG_VALUE_NAME_PACKED)
                {
                  CmiCopyPackedName(ValueBasicInformation->Name,
                                    ValueCell->Name,
                                    NameSize / sizeof(WCHAR));
                }
              else
                {
                  RtlCopyMemory(ValueBasicInformation->Name,
                                ValueCell->Name,
                                NameSize);
                }
            }
          break;

        case KeyValuePartialInformation:
          DataSize = ValueCell->DataSize & REG_DATA_SIZE_MASK;

          *ResultLength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data[0]) + 
            DataSize;

          if (Length < FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data[0]))
            {
              Status = STATUS_BUFFER_TOO_SMALL;
            }
          else
            {
              ValuePartialInformation = (PKEY_VALUE_PARTIAL_INFORMATION)
                KeyValueInformation;
              ValuePartialInformation->TitleIndex = 0;
              ValuePartialInformation->Type = ValueCell->DataType;
              ValuePartialInformation->DataLength = ValueCell->DataSize & REG_DATA_SIZE_MASK;

              if (Length - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data[0]) <
                  DataSize)
                {
                  DataSize = Length - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data[0]);
                  Status = STATUS_BUFFER_OVERFLOW;
                  CHECKPOINT;
                }
              
              if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET))
              {
                DataCell = CmiGetCell (RegistryHive, ValueCell->DataOffset, NULL);
                RtlCopyMemory(ValuePartialInformation->Data, 
                  DataCell->Data,
                  DataSize);
              }
              else
              {
                RtlCopyMemory(ValuePartialInformation->Data, 
                  &ValueCell->DataOffset, 
                  DataSize);
              }
            }
          break;

        case KeyValueFullInformation:
	  NameSize = ValueCell->NameSize;
          if (ValueCell->Flags & REG_VALUE_NAME_PACKED)
            {
	      NameSize *= sizeof(WCHAR);
	    }
	  DataSize = ValueCell->DataSize & REG_DATA_SIZE_MASK;

          *ResultLength = ROUND_UP(FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION,
                          Name[0]) + NameSize, sizeof(PVOID)) + DataSize;

          if (Length < FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name[0]))
            {
              Status = STATUS_BUFFER_TOO_SMALL;
            }
          else
            {
              ValueFullInformation = (PKEY_VALUE_FULL_INFORMATION) 
                KeyValueInformation;
              ValueFullInformation->TitleIndex = 0;
              ValueFullInformation->Type = ValueCell->DataType;
              ValueFullInformation->NameLength = NameSize;
              ValueFullInformation->DataOffset = 
                (ULONG_PTR)ValueFullInformation->Name -
                (ULONG_PTR)ValueFullInformation +
                ValueFullInformation->NameLength;
              ValueFullInformation->DataOffset =
                  ROUND_UP(ValueFullInformation->DataOffset, sizeof(PVOID));
              ValueFullInformation->DataLength = ValueCell->DataSize & REG_DATA_SIZE_MASK;
              
	      if (Length - FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name[0]) <
	          NameSize)
	        {
	          NameSize = Length - FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name[0]);
	          DataSize = 0;
	          Status = STATUS_BUFFER_OVERFLOW;
	          CHECKPOINT;
	        }
              else if (ROUND_UP(Length - FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION,
                       Name[0]) - NameSize, sizeof(PVOID)) < DataSize)
	        {
	          DataSize = ROUND_UP(Length - FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name[0]) - NameSize, sizeof(PVOID));
	          Status = STATUS_BUFFER_OVERFLOW;
	          CHECKPOINT;
	        }

              if (ValueCell->Flags & REG_VALUE_NAME_PACKED)
                {
                  CmiCopyPackedName(ValueFullInformation->Name,
				    ValueCell->Name,
				    NameSize / sizeof(WCHAR));
                }
              else
                {
                  RtlCopyMemory(ValueFullInformation->Name,
				ValueCell->Name,
				NameSize);
                }

              if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET))
                {
                  DataCell = CmiGetCell (RegistryHive, ValueCell->DataOffset, NULL);
                  RtlCopyMemory((PCHAR) ValueFullInformation
                    + ValueFullInformation->DataOffset,
                    DataCell->Data, DataSize);
                }
              else
                {
                  RtlCopyMemory((PCHAR) ValueFullInformation
                    + ValueFullInformation->DataOffset,
                    &ValueCell->DataOffset, DataSize);
                }
            }
          break;

          default:
            DPRINT1("Not handling 0x%x\n", KeyValueInformationClass);
        	break;
        }
    }
  else
    {
      Status = STATUS_UNSUCCESSFUL;
    }

  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();
  ObDereferenceObject(KeyObject);

  return Status;
}


NTSTATUS STDCALL
NtFlushKey(IN HANDLE KeyHandle)
{
  NTSTATUS Status;
  PKEY_OBJECT  KeyObject;
  PREGISTRY_HIVE  RegistryHive;
  KPROCESSOR_MODE  PreviousMode;
  
  PAGED_CODE();

  DPRINT("NtFlushKey (KeyHandle %lx) called\n", KeyHandle);
  
  PreviousMode = ExGetPreviousMode();

  /* Verify that the handle is valid and is a registry key */
  Status = ObReferenceObjectByHandle(KeyHandle,
				     KEY_QUERY_VALUE,
				     CmiKeyType,
				     PreviousMode,
				     (PVOID *)&KeyObject,
				     NULL);
  if (!NT_SUCCESS(Status))
    {
      return(Status);
    }

  VERIFY_KEY_OBJECT(KeyObject);

  RegistryHive = KeyObject->RegistryHive;

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  if (IsNoFileHive(RegistryHive))
    {
      Status = STATUS_SUCCESS;
    }
  else
    {
      /* Flush non-volatile hive */
      Status = CmiFlushRegistryHive(RegistryHive);
    }

  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  ObDereferenceObject(KeyObject);

  return STATUS_SUCCESS;
}


NTSTATUS STDCALL
NtOpenKey(OUT PHANDLE KeyHandle,
	  IN ACCESS_MASK DesiredAccess,
	  IN POBJECT_ATTRIBUTES ObjectAttributes)
{
  UNICODE_STRING RemainingPath;
  KPROCESSOR_MODE PreviousMode;
  PVOID Object;
  HANDLE hKey;
  NTSTATUS Status = STATUS_SUCCESS;
  
  PAGED_CODE();

  DPRINT("NtOpenKey(KH %x  DA %x  OA %x  OA->ON '%wZ'\n",
	 KeyHandle,
	 DesiredAccess,
	 ObjectAttributes,
	 ObjectAttributes ? ObjectAttributes->ObjectName : NULL);

  PreviousMode = ExGetPreviousMode();

  if(PreviousMode != KernelMode)
  {
    _SEH_TRY
    {
      ProbeForWrite(KeyHandle,
                    sizeof(HANDLE),
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

  RemainingPath.Buffer = NULL;
  Status = ObFindObject(ObjectAttributes,
			&Object,
			&RemainingPath,
			CmiKeyType);
  if (!NT_SUCCESS(Status))
    {
      return(Status);
    }

  VERIFY_KEY_OBJECT((PKEY_OBJECT) Object);

  DPRINT("RemainingPath '%wZ'\n", &RemainingPath);

  if ((RemainingPath.Buffer != NULL) && (RemainingPath.Buffer[0] != 0))
    {
      ObDereferenceObject(Object);
      RtlFreeUnicodeString(&RemainingPath);
      return STATUS_OBJECT_NAME_NOT_FOUND;
    }

  RtlFreeUnicodeString(&RemainingPath);

  /* Fail if the key has been deleted */
  if (((PKEY_OBJECT)Object)->Flags & KO_MARKED_FOR_DELETE)
    {
      ObDereferenceObject(Object);
      return(STATUS_UNSUCCESSFUL);
    }

  Status = ObCreateHandle(PsGetCurrentProcess(),
			  Object,
			  DesiredAccess,
			  TRUE,
			  &hKey);
  ObDereferenceObject(Object);

  if (!NT_SUCCESS(Status))
    {
      return(Status);
    }

  _SEH_TRY
  {
    *KeyHandle = hKey;
  }
  _SEH_HANDLE
  {
    Status = _SEH_GetExceptionCode();
  }
  _SEH_END;

  return Status;
}


NTSTATUS STDCALL
NtQueryKey(IN HANDLE KeyHandle,
	   IN KEY_INFORMATION_CLASS KeyInformationClass,
	   OUT PVOID KeyInformation,
	   IN ULONG Length,
	   OUT PULONG ResultLength)
{
  PKEY_BASIC_INFORMATION BasicInformation;
  PKEY_NODE_INFORMATION NodeInformation;
  PKEY_FULL_INFORMATION FullInformation;
  PREGISTRY_HIVE RegistryHive;
  PDATA_CELL ClassCell;
  PKEY_OBJECT KeyObject;
  PKEY_CELL KeyCell;
  ULONG NameSize, ClassSize;
  NTSTATUS Status;
  
  PAGED_CODE();

  DPRINT("NtQueryKey(KH %x  KIC %x  KI %x  L %d  RL %x)\n",
	 KeyHandle,
	 KeyInformationClass,
	 KeyInformation,
	 Length,
	 ResultLength);

  /* Verify that the handle is valid and is a registry key */
  Status = ObReferenceObjectByHandle(KeyHandle,
		KEY_READ,
		CmiKeyType,
		UserMode,
		(PVOID *) &KeyObject,
		NULL);
  if (!NT_SUCCESS(Status))
    {
      return Status;
    }

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceSharedLite(&CmiRegistryLock, TRUE);

  VERIFY_KEY_OBJECT(KeyObject);

  /* Get pointer to KeyCell */
  KeyCell = KeyObject->KeyCell;
  RegistryHive = KeyObject->RegistryHive;

  Status = STATUS_SUCCESS;
  switch (KeyInformationClass)
    {
      case KeyBasicInformation:
        NameSize = KeyObject->Name.Length;

	*ResultLength = FIELD_OFFSET(KEY_BASIC_INFORMATION, Name[0]);

	/* Check size of buffer */
	if (Length < FIELD_OFFSET(KEY_BASIC_INFORMATION, Name[0]))
	  {
	    Status = STATUS_BUFFER_TOO_SMALL;
	  }
	else
	  {
	    /* Fill buffer with requested info */
	    BasicInformation = (PKEY_BASIC_INFORMATION) KeyInformation;
	    BasicInformation->LastWriteTime.u.LowPart = KeyCell->LastWriteTime.u.LowPart;
	    BasicInformation->LastWriteTime.u.HighPart = KeyCell->LastWriteTime.u.HighPart;
	    BasicInformation->TitleIndex = 0;
	    BasicInformation->NameLength = KeyObject->Name.Length;

	    if (Length - FIELD_OFFSET(KEY_BASIC_INFORMATION, Name[0]) <
	        NameSize)
	      {
	        NameSize = Length - FIELD_OFFSET(KEY_BASIC_INFORMATION, Name[0]);
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }

	    RtlCopyMemory(BasicInformation->Name,
			  KeyObject->Name.Buffer,
			  NameSize);
	  }
	break;

      case KeyNodeInformation:
        NameSize = KeyObject->Name.Length;
        ClassSize = KeyCell->ClassSize;

	*ResultLength = FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]) +
	  NameSize + ClassSize;

	/* Check size of buffer */
	if (Length < *ResultLength)
	  {
	    Status = STATUS_BUFFER_TOO_SMALL;
	  }
	else
	  {
	    /* Fill buffer with requested info */
	    NodeInformation = (PKEY_NODE_INFORMATION) KeyInformation;
	    NodeInformation->LastWriteTime.u.LowPart = KeyCell->LastWriteTime.u.LowPart;
	    NodeInformation->LastWriteTime.u.HighPart = KeyCell->LastWriteTime.u.HighPart;
	    NodeInformation->TitleIndex = 0;
	    NodeInformation->ClassOffset = sizeof(KEY_NODE_INFORMATION) +
	      KeyObject->Name.Length;
	    NodeInformation->ClassLength = KeyCell->ClassSize;
	    NodeInformation->NameLength = KeyObject->Name.Length;

	    if (Length - FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]) < NameSize)
	      {
	        NameSize = Length - FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]);
	        ClassSize = 0;
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }
	    else if (Length - FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]) - 
	             NameSize < ClassSize)
	      {
	        ClassSize = Length - FIELD_OFFSET(KEY_NODE_INFORMATION, Name[0]) - 
	                    NameSize;
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }

	    RtlCopyMemory(NodeInformation->Name,
			  KeyObject->Name.Buffer,
			  NameSize);

	    if (ClassSize != 0)
	      {
		ClassCell = CmiGetCell (KeyObject->RegistryHive,
					KeyCell->ClassNameOffset,
					NULL);
		RtlCopyMemory (NodeInformation->Name + KeyObject->Name.Length,
			       ClassCell->Data,
			       ClassSize);
	      }
	  }
	break;

      case KeyFullInformation:
        ClassSize = KeyCell->ClassSize;

	*ResultLength = FIELD_OFFSET(KEY_FULL_INFORMATION, Class) +
	  ClassSize;

	/* Check size of buffer */
	if (Length < FIELD_OFFSET(KEY_FULL_INFORMATION, Class))
	  {
	    Status = STATUS_BUFFER_TOO_SMALL;
	  }
	else
	  {
	    /* Fill buffer with requested info */
	    FullInformation = (PKEY_FULL_INFORMATION) KeyInformation;
	    FullInformation->LastWriteTime.u.LowPart = KeyCell->LastWriteTime.u.LowPart;
	    FullInformation->LastWriteTime.u.HighPart = KeyCell->LastWriteTime.u.HighPart;
	    FullInformation->TitleIndex = 0;
	    FullInformation->ClassOffset = sizeof(KEY_FULL_INFORMATION) - sizeof(WCHAR);
	    FullInformation->ClassLength = KeyCell->ClassSize;
	    FullInformation->SubKeys = CmiGetNumberOfSubKeys(KeyObject); //KeyCell->NumberOfSubKeys;
	    FullInformation->MaxNameLen = CmiGetMaxNameLength(KeyObject);
	    FullInformation->MaxClassLen = CmiGetMaxClassLength(KeyObject);
	    FullInformation->Values = KeyCell->NumberOfValues;
	    FullInformation->MaxValueNameLen =
	      CmiGetMaxValueNameLength(RegistryHive, KeyCell);
	    FullInformation->MaxValueDataLen =
	      CmiGetMaxValueDataLength(RegistryHive, KeyCell);

	    if (Length - FIELD_OFFSET(KEY_FULL_INFORMATION, Class[0]) < ClassSize)
	      {
	        ClassSize = Length - FIELD_OFFSET(KEY_FULL_INFORMATION, Class[0]);
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }
	      
	    if (ClassSize)
	      {
		ClassCell = CmiGetCell (KeyObject->RegistryHive,
					KeyCell->ClassNameOffset,
					NULL);
		RtlCopyMemory (FullInformation->Class,
			       ClassCell->Data, ClassSize);
	      }
	  }
	break;

      default:
	DPRINT1("Not handling 0x%x\n", KeyInformationClass);
	Status = STATUS_INVALID_INFO_CLASS;
	break;
    }

  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();
  ObDereferenceObject(KeyObject);

  return(Status);
}


NTSTATUS STDCALL
NtQueryValueKey(IN HANDLE KeyHandle,
	IN PUNICODE_STRING ValueName,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	OUT PVOID KeyValueInformation,
	IN ULONG Length,
	OUT PULONG ResultLength)
{
  NTSTATUS  Status;
  ULONG NameSize, DataSize;
  PKEY_OBJECT  KeyObject;
  PREGISTRY_HIVE  RegistryHive;
  PKEY_CELL  KeyCell;
  PVALUE_CELL  ValueCell;
  PDATA_CELL  DataCell;
  PKEY_VALUE_BASIC_INFORMATION  ValueBasicInformation;
  PKEY_VALUE_PARTIAL_INFORMATION  ValuePartialInformation;
  PKEY_VALUE_FULL_INFORMATION  ValueFullInformation;
  
  PAGED_CODE();

  DPRINT("NtQueryValueKey(KeyHandle %x  ValueName %S  Length %x)\n",
    KeyHandle, ValueName->Buffer, Length);

  /* Verify that the handle is valid and is a registry key */
  Status = ObReferenceObjectByHandle(KeyHandle,
		KEY_QUERY_VALUE,
		CmiKeyType,
		UserMode,
		(PVOID *)&KeyObject,
		NULL);

  if (!NT_SUCCESS(Status))
    {
      DPRINT1("ObReferenceObjectByHandle() failed with status %x\n", Status);
      return Status;
    }

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceSharedLite(&CmiRegistryLock, TRUE);

  VERIFY_KEY_OBJECT(KeyObject);

  /* Get pointer to KeyCell */
  KeyCell = KeyObject->KeyCell;
  RegistryHive = KeyObject->RegistryHive;

  /* Get value cell by name */
  Status = CmiScanKeyForValue(RegistryHive,
			      KeyCell,
			      ValueName,
			      &ValueCell,
			      NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("CmiScanKeyForValue() failed with status %x\n", Status);
      goto ByeBye;
    }

  Status = STATUS_SUCCESS;
  switch (KeyValueInformationClass)
    {
      case KeyValueBasicInformation:
	NameSize = ValueCell->NameSize;
	if (ValueCell->Flags & REG_VALUE_NAME_PACKED)
	  {
	    NameSize *= sizeof(WCHAR);
	  }

	*ResultLength = FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name[0]) +
	                NameSize;

	if (Length < FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name[0]))
	  {
	    Status = STATUS_BUFFER_TOO_SMALL;
	  }
	else
	  {
	    ValueBasicInformation = (PKEY_VALUE_BASIC_INFORMATION) 
	      KeyValueInformation;
	    ValueBasicInformation->TitleIndex = 0;
	    ValueBasicInformation->Type = ValueCell->DataType;
	    ValueBasicInformation->NameLength = NameSize;

	    if (Length - FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name[0]) <
	        NameSize)
	      {
	        NameSize = Length - FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name[0]);
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }

	    if (ValueCell->Flags & REG_VALUE_NAME_PACKED)
	      {
		CmiCopyPackedName(ValueBasicInformation->Name,
				  ValueCell->Name,
				  NameSize / sizeof(WCHAR));
	      }
	    else
	      {
		RtlCopyMemory(ValueBasicInformation->Name,
			      ValueCell->Name,
			      NameSize);
	      }
	  }
	break;

      case KeyValuePartialInformation:
	DataSize = ValueCell->DataSize & REG_DATA_SIZE_MASK;

	*ResultLength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data[0]) +
	                DataSize;

	if (Length < FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data[0]))
	  {
	    Status = STATUS_BUFFER_TOO_SMALL;
	  }
	else
	  {
	    ValuePartialInformation = (PKEY_VALUE_PARTIAL_INFORMATION)
	      KeyValueInformation;
	    ValuePartialInformation->TitleIndex = 0;
	    ValuePartialInformation->Type = ValueCell->DataType;
	    ValuePartialInformation->DataLength = DataSize;

	    if (Length - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data[0]) <
	        DataSize)
	      {
		DataSize = Length - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data[0]);
		Status = STATUS_BUFFER_OVERFLOW;
		CHECKPOINT;
	      }

	    if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET))
	      {
		DataCell = CmiGetCell (RegistryHive, ValueCell->DataOffset, NULL);
		RtlCopyMemory(ValuePartialInformation->Data,
			      DataCell->Data,
			      DataSize);
	      }
	    else
	      {
		RtlCopyMemory(ValuePartialInformation->Data,
			      &ValueCell->DataOffset,
			      DataSize);
	      }
	  }
	break;

      case KeyValueFullInformation:
	NameSize = ValueCell->NameSize;
	if (ValueCell->Flags & REG_VALUE_NAME_PACKED)
	  {
	    NameSize *= sizeof(WCHAR);
	  }
	DataSize = ValueCell->DataSize & REG_DATA_SIZE_MASK;

	*ResultLength = ROUND_UP(FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION,
	                Name[0]) + NameSize, sizeof(PVOID)) + DataSize;

	if (Length < FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name[0]))
	  {
	    Status = STATUS_BUFFER_TOO_SMALL;
	  }
	else
	  {
	    ValueFullInformation = (PKEY_VALUE_FULL_INFORMATION)
	      KeyValueInformation;
	    ValueFullInformation->TitleIndex = 0;
	    ValueFullInformation->Type = ValueCell->DataType;
	    ValueFullInformation->NameLength = NameSize;
	    ValueFullInformation->DataOffset = 
	      (ULONG_PTR)ValueFullInformation->Name -
	      (ULONG_PTR)ValueFullInformation +
	      ValueFullInformation->NameLength;
	    ValueFullInformation->DataOffset =
	      ROUND_UP(ValueFullInformation->DataOffset, sizeof(PVOID));
	    ValueFullInformation->DataLength = ValueCell->DataSize & REG_DATA_SIZE_MASK;

	    if (Length - FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name[0]) <
	        NameSize)
	      {
	        NameSize = Length - FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name[0]);
	        DataSize = 0;
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }
            else if (ROUND_UP(Length - FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION,
                     Name[0]) - NameSize, sizeof(PVOID)) < DataSize)
	      {
	        DataSize = ROUND_UP(Length - FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION,
	                            Name[0]) - NameSize, sizeof(PVOID));
	        Status = STATUS_BUFFER_OVERFLOW;
	        CHECKPOINT;
	      }

	    if (ValueCell->Flags & REG_VALUE_NAME_PACKED)
	      {
		CmiCopyPackedName(ValueFullInformation->Name,
				  ValueCell->Name,
				  NameSize / sizeof(WCHAR));
	      }
	    else
	      {
		RtlCopyMemory(ValueFullInformation->Name,
			      ValueCell->Name,
			      NameSize);
	      }
	    if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET))
	      {
		DataCell = CmiGetCell (RegistryHive, ValueCell->DataOffset, NULL);
		RtlCopyMemory((PCHAR) ValueFullInformation
			      + ValueFullInformation->DataOffset,
			      DataCell->Data,
			      DataSize);
	      }
	    else
	      {
		RtlCopyMemory((PCHAR) ValueFullInformation
			      + ValueFullInformation->DataOffset,
			      &ValueCell->DataOffset,
			      DataSize);
	      }
	  }
	break;

      default:
	DPRINT1("Not handling 0x%x\n", KeyValueInformationClass);
	Status = STATUS_INVALID_INFO_CLASS;
	break;
    }

ByeBye:;
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();
  ObDereferenceObject(KeyObject);

  return Status;
}


NTSTATUS STDCALL
NtSetValueKey(IN HANDLE KeyHandle,
	      IN PUNICODE_STRING ValueName,
	      IN ULONG TitleIndex,
	      IN ULONG Type,
	      IN PVOID Data,
	      IN ULONG DataSize)
{
  NTSTATUS  Status;
  PKEY_OBJECT  KeyObject;
  PREGISTRY_HIVE  RegistryHive;
  PKEY_CELL  KeyCell;
  PVALUE_CELL  ValueCell;
  BLOCK_OFFSET ValueCellOffset;
  PDATA_CELL DataCell;
  PDATA_CELL NewDataCell;
  PHBIN pBin;
  ULONG DesiredAccess;
  
  PAGED_CODE();

  DPRINT("NtSetValueKey(KeyHandle %x  ValueName '%wZ'  Type %d)\n",
	 KeyHandle, ValueName, Type);

  DesiredAccess = KEY_SET_VALUE;
  if (Type == REG_LINK)
    DesiredAccess |= KEY_CREATE_LINK;

  /* Verify that the handle is valid and is a registry key */
  Status = ObReferenceObjectByHandle(KeyHandle,
				     DesiredAccess,
				     CmiKeyType,
				     UserMode,
				     (PVOID *)&KeyObject,
				     NULL);
  if (!NT_SUCCESS(Status))
    return(Status);

  /* Acquire hive lock exclucively */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  VERIFY_KEY_OBJECT(KeyObject);

  /* Get pointer to key cell */
  KeyCell = KeyObject->KeyCell;
  RegistryHive = KeyObject->RegistryHive;
  Status = CmiScanKeyForValue(RegistryHive,
			      KeyCell,
			      ValueName,
			      &ValueCell,
			      &ValueCellOffset);
  if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
      DPRINT("Allocate new value cell\n");
      Status = CmiAddValueToKey(RegistryHive,
				KeyCell,
				KeyObject->KeyCellOffset,
				ValueName,
				&ValueCell,
				&ValueCellOffset);
    }

  if (!NT_SUCCESS(Status))
    {
      DPRINT("Cannot add value. Status 0x%X\n", Status);

      ExReleaseResourceLite(&CmiRegistryLock);
      KeLeaveCriticalRegion();
      ObDereferenceObject(KeyObject);
      return Status;
    }

  DPRINT("DataSize %lu\n", DataSize);
  DPRINT("ValueCell %p\n", ValueCell);
  DPRINT("ValueCell->DataSize %lu\n", ValueCell->DataSize);

  if (DataSize <= sizeof(BLOCK_OFFSET))
    {
      /* If data size <= sizeof(BLOCK_OFFSET) then store data in the data offset */
      DPRINT("ValueCell->DataSize %lu\n", ValueCell->DataSize);
      if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET) &&
	  (ValueCell->DataSize & REG_DATA_SIZE_MASK) != 0)
	{
	  DataCell = CmiGetCell (RegistryHive, ValueCell->DataOffset, NULL);
	  CmiDestroyCell(RegistryHive, DataCell, ValueCell->DataOffset);
	}

      RtlCopyMemory(&ValueCell->DataOffset, Data, DataSize);
      ValueCell->DataSize = DataSize | REG_DATA_IN_OFFSET;
      ValueCell->DataType = Type;
      RtlMoveMemory(&ValueCell->DataOffset, Data, DataSize);
      CmiMarkBlockDirty(RegistryHive, ValueCellOffset);
    }
  else if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET) &&
	   (DataSize <= (ValueCell->DataSize & REG_DATA_SIZE_MASK)))
    {
      /* If new data size is <= current then overwrite current data */
      DataCell = CmiGetCell (RegistryHive, ValueCell->DataOffset,&pBin);
      RtlZeroMemory(DataCell->Data, ValueCell->DataSize);
      RtlCopyMemory(DataCell->Data, Data, DataSize);
      ValueCell->DataSize = DataSize;
      ValueCell->DataType = Type;
    }
  else
    {
      /*
       * New data size is larger than the current, destroy current
       * data block and allocate a new one.
       */
      BLOCK_OFFSET NewOffset;

      DPRINT("ValueCell->DataSize %lu\n", ValueCell->DataSize);

      if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET) &&
	  (ValueCell->DataSize & REG_DATA_SIZE_MASK) != 0)
	{
	  DataCell = CmiGetCell (RegistryHive, ValueCell->DataOffset, NULL);
	  CmiDestroyCell(RegistryHive, DataCell, ValueCell->DataOffset);
	  ValueCell->DataSize = 0;
	  ValueCell->DataType = 0;
	  ValueCell->DataOffset = (BLOCK_OFFSET)-1;
	}

      Status = CmiAllocateCell (RegistryHive,
				sizeof(CELL_HEADER) + DataSize,
				(PVOID *)&NewDataCell,
				&NewOffset);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT("CmiAllocateBlock() failed (Status %lx)\n", Status);

	  ExReleaseResourceLite(&CmiRegistryLock);
	  KeLeaveCriticalRegion();
	  ObDereferenceObject(KeyObject);

	  return Status;
	}

      RtlCopyMemory(&NewDataCell->Data[0], Data, DataSize);
      ValueCell->DataSize = DataSize & REG_DATA_SIZE_MASK;
      ValueCell->DataType = Type;
      ValueCell->DataOffset = NewOffset;
      CmiMarkBlockDirty(RegistryHive, ValueCell->DataOffset);
      CmiMarkBlockDirty(RegistryHive, ValueCellOffset);
    }

  /* Mark link key */
  if ((Type == REG_LINK) &&
      (_wcsicmp(ValueName->Buffer, L"SymbolicLinkValue") == 0))
    {
      KeyCell->Flags |= REG_KEY_LINK_CELL;
    }

  KeQuerySystemTime (&KeyCell->LastWriteTime);
  CmiMarkBlockDirty (RegistryHive, KeyObject->KeyCellOffset);

  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();
  ObDereferenceObject(KeyObject);

  CmiSyncHives();

  DPRINT("Return Status 0x%X\n", Status);

  return Status;
}


NTSTATUS STDCALL
NtDeleteValueKey (IN HANDLE KeyHandle,
		  IN PUNICODE_STRING ValueName)
{
  PKEY_OBJECT KeyObject;
  NTSTATUS Status;
  
  PAGED_CODE();

  /* Verify that the handle is valid and is a registry key */
  Status = ObReferenceObjectByHandle(KeyHandle,
		KEY_QUERY_VALUE,
		CmiKeyType,
		UserMode,
		(PVOID *)&KeyObject,
		NULL);
  if (!NT_SUCCESS(Status))
    {
      return Status;
    }

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  VERIFY_KEY_OBJECT(KeyObject);

  Status = CmiDeleteValueFromKey(KeyObject->RegistryHive,
				 KeyObject->KeyCell,
				 KeyObject->KeyCellOffset,
				 ValueName);

  KeQuerySystemTime (&KeyObject->KeyCell->LastWriteTime);
  CmiMarkBlockDirty (KeyObject->RegistryHive, KeyObject->KeyCellOffset);

  /* Release hive lock */
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  ObDereferenceObject (KeyObject);

  CmiSyncHives ();

  return Status;
}


/*
 * NOTE:
 * KeyObjectAttributes->RootDirectory specifies the handle to the parent key and
 * KeyObjectAttributes->Name specifies the name of the key to load.
 */
NTSTATUS STDCALL
NtLoadKey (IN POBJECT_ATTRIBUTES KeyObjectAttributes,
	   IN POBJECT_ATTRIBUTES FileObjectAttributes)
{
  return NtLoadKey2 (KeyObjectAttributes,
		     FileObjectAttributes,
		     0);
}


/*
 * NOTE:
 * KeyObjectAttributes->RootDirectory specifies the handle to the parent key and
 * KeyObjectAttributes->Name specifies the name of the key to load.
 * Flags can be 0 or REG_NO_LAZY_FLUSH.
 */
NTSTATUS STDCALL
NtLoadKey2 (IN POBJECT_ATTRIBUTES KeyObjectAttributes,
	    IN POBJECT_ATTRIBUTES FileObjectAttributes,
	    IN ULONG Flags)
{
  POBJECT_NAME_INFORMATION NameInfo;
  PUNICODE_STRING NamePointer;
  PUCHAR Buffer;
  ULONG BufferSize;
  ULONG Length;
  NTSTATUS Status;
  
  PAGED_CODE();

  DPRINT ("NtLoadKey2() called\n");

#if 0
  if (!SeSinglePrivilegeCheck (SeRestorePrivilege, KeGetPreviousMode ()))
    return STATUS_PRIVILEGE_NOT_HELD;
#endif

  if (FileObjectAttributes->RootDirectory != NULL)
    {
      BufferSize =
	sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR);
      Buffer = ExAllocatePool (NonPagedPool,
			       BufferSize);
      if (Buffer == NULL)
	return STATUS_INSUFFICIENT_RESOURCES;

      Status = ZwQueryObject (FileObjectAttributes->RootDirectory,
			      ObjectNameInformation,
			      Buffer,
			      BufferSize,
			      &Length);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT1 ("NtQueryObject() failed (Status %lx)\n", Status);
	  ExFreePool (Buffer);
	  return Status;
	}

      NameInfo = (POBJECT_NAME_INFORMATION)Buffer;
      DPRINT ("ObjectPath: '%wZ'  Length %hu\n",
	      &NameInfo->Name, NameInfo->Name.Length);

      NameInfo->Name.MaximumLength = MAX_PATH * sizeof(WCHAR);
      if (FileObjectAttributes->ObjectName->Buffer[0] != L'\\')
	{
	  RtlAppendUnicodeToString (&NameInfo->Name,
				    L"\\");
	  DPRINT ("ObjectPath: '%wZ'  Length %hu\n",
		  &NameInfo->Name, NameInfo->Name.Length);
	}
      RtlAppendUnicodeStringToString (&NameInfo->Name,
				      FileObjectAttributes->ObjectName);

      DPRINT ("ObjectPath: '%wZ'  Length %hu\n",
	      &NameInfo->Name, NameInfo->Name.Length);
      NamePointer = &NameInfo->Name;
    }
  else
    {
      if (FileObjectAttributes->ObjectName->Buffer[0] == L'\\')
	{
	  Buffer = NULL;
	  NamePointer = FileObjectAttributes->ObjectName;
	}
      else
	{
	  BufferSize =
	    sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR);
	  Buffer = ExAllocatePool (NonPagedPool,
				   BufferSize);
	  if (Buffer == NULL)
	    return STATUS_INSUFFICIENT_RESOURCES;

	  NameInfo = (POBJECT_NAME_INFORMATION)Buffer;
	  NameInfo->Name.MaximumLength = MAX_PATH * sizeof(WCHAR);
	  NameInfo->Name.Length = 0;
	  NameInfo->Name.Buffer = (PWSTR)((ULONG_PTR)Buffer + sizeof(OBJECT_NAME_INFORMATION));
	  NameInfo->Name.Buffer[0] = 0;

	  RtlAppendUnicodeToString (&NameInfo->Name,
				    L"\\");
	  RtlAppendUnicodeStringToString (&NameInfo->Name,
					  FileObjectAttributes->ObjectName);

	  NamePointer = &NameInfo->Name;
	}
    }

  DPRINT ("Full name: '%wZ'\n", NamePointer);

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  Status = CmiLoadHive (KeyObjectAttributes,
			NamePointer,
			Flags);
  if (!NT_SUCCESS (Status))
    {
      DPRINT1 ("CmiLoadHive() failed (Status %lx)\n", Status);
    }

  /* Release hive lock */
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  if (Buffer != NULL)
    ExFreePool (Buffer);

  return Status;
}


NTSTATUS STDCALL
NtNotifyChangeKey (IN HANDLE KeyHandle,
		   IN HANDLE Event,
		   IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		   IN PVOID ApcContext OPTIONAL,
		   OUT PIO_STATUS_BLOCK IoStatusBlock,
		   IN ULONG CompletionFilter,
		   IN BOOLEAN WatchSubtree,
		   OUT PVOID Buffer,
		   IN ULONG Length,
		   IN BOOLEAN Asynchronous)
{
	UNIMPLEMENTED;
	return(STATUS_NOT_IMPLEMENTED);
}


NTSTATUS STDCALL
NtQueryMultipleValueKey (IN HANDLE KeyHandle,
			 IN OUT PKEY_VALUE_ENTRY ValueList,
			 IN ULONG NumberOfValues,
			 OUT PVOID Buffer,
			 IN OUT PULONG Length,
			 OUT PULONG ReturnLength)
{
  PREGISTRY_HIVE RegistryHive;
  PVALUE_CELL ValueCell;
  PKEY_OBJECT KeyObject;
  PDATA_CELL DataCell;
  ULONG BufferLength = 0;
  PKEY_CELL KeyCell;
  NTSTATUS Status;
  PUCHAR DataPtr;
  ULONG i;
  
  PAGED_CODE();

  /* Verify that the handle is valid and is a registry key */
  Status = ObReferenceObjectByHandle(KeyHandle,
				     KEY_QUERY_VALUE,
				     CmiKeyType,
				     UserMode,
				     (PVOID *) &KeyObject,
				     NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ObReferenceObjectByHandle() failed with status %x\n", Status);
      return(Status);
    }

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceSharedLite(&CmiRegistryLock, TRUE);

  VERIFY_KEY_OBJECT(KeyObject);

  /* Get pointer to KeyCell */
  KeyCell = KeyObject->KeyCell;
  RegistryHive = KeyObject->RegistryHive;

  DataPtr = (PUCHAR) Buffer;

  for (i = 0; i < NumberOfValues; i++)
    {
      DPRINT("ValueName: '%wZ'\n", ValueList[i].ValueName);

      /* Get Value block of interest */
      Status = CmiScanKeyForValue(RegistryHive,
			  KeyCell,
			  ValueList[i].ValueName,
			  &ValueCell,
			  NULL);

      if (!NT_SUCCESS(Status))
	{
	  DPRINT("CmiScanKeyForValue() failed with status %x\n", Status);
	  break;
	}
      else if (ValueCell == NULL)
	{
	  Status = STATUS_OBJECT_NAME_NOT_FOUND;
	  break;
	}

      BufferLength = ROUND_UP(BufferLength, sizeof(PVOID));

      if (BufferLength + (ValueCell->DataSize & REG_DATA_SIZE_MASK) <= *Length)
	{
	  DataPtr = (PUCHAR)ROUND_UP((ULONG_PTR)DataPtr, sizeof(PVOID));

	  ValueList[i].Type = ValueCell->DataType;
	  ValueList[i].DataLength = ValueCell->DataSize & REG_DATA_SIZE_MASK;
	  ValueList[i].DataOffset = (ULONG_PTR)DataPtr - (ULONG_PTR)Buffer;

	  if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET))
	    {
	      DataCell = CmiGetCell (RegistryHive,
				     ValueCell->DataOffset,
				     NULL);
	      RtlCopyMemory(DataPtr,
			    DataCell->Data,
			    ValueCell->DataSize & REG_DATA_SIZE_MASK);
	    }
	  else
	    {
	      RtlCopyMemory(DataPtr,
			    &ValueCell->DataOffset,
			    ValueCell->DataSize & REG_DATA_SIZE_MASK);
	    }

	  DataPtr += ValueCell->DataSize & REG_DATA_SIZE_MASK;
	}
      else
	{
	  Status = STATUS_BUFFER_TOO_SMALL;
	}

      BufferLength +=  ValueCell->DataSize & REG_DATA_SIZE_MASK;
    }

  if (NT_SUCCESS(Status))
    *Length = BufferLength;

  *ReturnLength = BufferLength;

  /* Release hive lock */
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  ObDereferenceObject(KeyObject);

  DPRINT("Return Status 0x%X\n", Status);

  return Status;
}


NTSTATUS STDCALL
NtReplaceKey (IN POBJECT_ATTRIBUTES ObjectAttributes,
	      IN HANDLE Key,
	      IN POBJECT_ATTRIBUTES ReplacedObjectAttributes)
{
	UNIMPLEMENTED;
	return(STATUS_NOT_IMPLEMENTED);
}


NTSTATUS STDCALL
NtRestoreKey (IN HANDLE KeyHandle,
	      IN HANDLE FileHandle,
	      IN ULONG RestoreFlags)
{
	UNIMPLEMENTED;
	return(STATUS_NOT_IMPLEMENTED);
}


NTSTATUS STDCALL
NtSaveKey (IN HANDLE KeyHandle,
	   IN HANDLE FileHandle)
{
  PREGISTRY_HIVE TempHive;
  PKEY_OBJECT KeyObject;
  NTSTATUS Status;
  
  PAGED_CODE();

  DPRINT ("NtSaveKey() called\n");

#if 0
  if (!SeSinglePrivilegeCheck (SeBackupPrivilege, KeGetPreviousMode ()))
    return STATUS_PRIVILEGE_NOT_HELD;
#endif

  Status = ObReferenceObjectByHandle (KeyHandle,
				      0,
				      CmiKeyType,
				      KeGetPreviousMode(),
				      (PVOID *)&KeyObject,
				      NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("ObReferenceObjectByHandle() failed (Status %lx)\n", Status);
      return Status;
    }

  /* Acquire hive lock exclucively */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  /* Refuse to save a volatile key */
  if (KeyObject->RegistryHive == CmiVolatileHive)
    {
      DPRINT1 ("Cannot save a volatile key\n");
      ExReleaseResourceLite(&CmiRegistryLock);
      KeLeaveCriticalRegion();
      ObDereferenceObject (KeyObject);
      return STATUS_ACCESS_DENIED;
    }

  Status = CmiCreateTempHive(&TempHive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("CmiCreateTempHive() failed (Status %lx)\n", Status);
      ExReleaseResourceLite(&CmiRegistryLock);
      KeLeaveCriticalRegion();
      ObDereferenceObject (KeyObject);
      return(Status);
    }

  Status = CmiCopyKey (TempHive,
		       NULL,
		       KeyObject->RegistryHive,
		       KeyObject->KeyCell);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("CmiCopyKey() failed (Status %lx)\n", Status);
      CmiRemoveRegistryHive (TempHive);
      ExReleaseResourceLite(&CmiRegistryLock);
      KeLeaveCriticalRegion();
      ObDereferenceObject (KeyObject);
      return(Status);
    }

  Status = CmiSaveTempHive (TempHive,
			    FileHandle);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("CmiSaveTempHive() failed (Status %lx)\n", Status);
    }

  CmiRemoveRegistryHive (TempHive);

  /* Release hive lock */
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  ObDereferenceObject (KeyObject);

  DPRINT ("NtSaveKey() done\n");

  return STATUS_SUCCESS;
}

/*
 * @unimplemented
 */
NTSTATUS
STDCALL
NtSaveKeyEx(
	IN HANDLE KeyHandle,
	IN HANDLE FileHandle,
	IN ULONG Flags // REG_STANDARD_FORMAT, etc..
	)
{
	UNIMPLEMENTED;
	return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS STDCALL
NtSetInformationKey (IN HANDLE KeyHandle,
		     IN KEY_SET_INFORMATION_CLASS KeyInformationClass,
		     IN PVOID KeyInformation,
		     IN ULONG KeyInformationLength)
{
  PKEY_OBJECT KeyObject;
  NTSTATUS Status;
  
  PAGED_CODE();

  if (KeyInformationClass != KeyWriteTimeInformation)
    return STATUS_INVALID_INFO_CLASS;

  if (KeyInformationLength != sizeof (KEY_WRITE_TIME_INFORMATION))
    return STATUS_INFO_LENGTH_MISMATCH;

  /* Verify that the handle is valid and is a registry key */
  Status = ObReferenceObjectByHandle (KeyHandle,
				      KEY_SET_VALUE,
				      CmiKeyType,
				      UserMode,
				      (PVOID *)&KeyObject,
				      NULL);
  if (!NT_SUCCESS (Status))
    {
      DPRINT ("ObReferenceObjectByHandle() failed with status %x\n", Status);
      return Status;
    }

  /* Acquire hive lock */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  VERIFY_KEY_OBJECT(KeyObject);

  KeyObject->KeyCell->LastWriteTime.QuadPart =
    ((PKEY_WRITE_TIME_INFORMATION)KeyInformation)->LastWriteTime.QuadPart;

  CmiMarkBlockDirty (KeyObject->RegistryHive,
		     KeyObject->KeyCellOffset);

  /* Release hive lock */
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  ObDereferenceObject (KeyObject);

  CmiSyncHives ();

  DPRINT ("NtSaveKey() done\n");

  return STATUS_SUCCESS;
}


/*
 * NOTE:
 * KeyObjectAttributes->RootDirectory specifies the handle to the parent key and
 * KeyObjectAttributes->Name specifies the name of the key to unload.
 */
NTSTATUS STDCALL
NtUnloadKey (IN POBJECT_ATTRIBUTES KeyObjectAttributes)
{
  PREGISTRY_HIVE RegistryHive;
  NTSTATUS Status;
  
  PAGED_CODE();

  DPRINT ("NtUnloadKey() called\n");

#if 0
  if (!SeSinglePrivilegeCheck (SeRestorePrivilege, KeGetPreviousMode ()))
    return STATUS_PRIVILEGE_NOT_HELD;
#endif

  /* Acquire registry lock exclusively */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  Status = CmiDisconnectHive (KeyObjectAttributes,
			      &RegistryHive);
  if (!NT_SUCCESS (Status))
    {
      DPRINT1 ("CmiDisconnectHive() failed (Status %lx)\n", Status);
      ExReleaseResourceLite (&CmiRegistryLock);
      KeLeaveCriticalRegion();
      return Status;
    }

  DPRINT ("RegistryHive %p\n", RegistryHive);

#if 0
  /* Flush hive */
  if (!IsNoFileHive (RegistryHive))
    CmiFlushRegistryHive (RegistryHive);
#endif

  CmiRemoveRegistryHive (RegistryHive);

  /* Release registry lock */
  ExReleaseResourceLite (&CmiRegistryLock);
  KeLeaveCriticalRegion();

  DPRINT ("NtUnloadKey() done\n");

  return STATUS_SUCCESS;
}


NTSTATUS STDCALL
NtInitializeRegistry (IN BOOLEAN SetUpBoot)
{
  NTSTATUS Status;
  
  PAGED_CODE();

  if (CmiRegistryInitialized == TRUE)
    return STATUS_ACCESS_DENIED;

  /* Save boot log file */
  IopSaveBootLogToFile();

  Status = CmiInitHives (SetUpBoot);

  CmiRegistryInitialized = TRUE;

  return Status;
}

/* EOF */
