/* $Id: object.c 14014 2005-03-13 16:44:15Z hbirr $
 * 
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ob/object.c
 * PURPOSE:         Implements generic object managment functions
 * 
 * PROGRAMMERS:     David Welch (welch@cwcom.net)
 *                  Skywing (skywing@valhallalegends.com)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>


typedef struct _RETENTION_CHECK_PARAMS
{
  WORK_QUEUE_ITEM WorkItem;
  POBJECT_HEADER ObjectHeader;
} RETENTION_CHECK_PARAMS, *PRETENTION_CHECK_PARAMS;


/* FUNCTIONS ************************************************************/

NTSTATUS
ObpCaptureObjectAttributes(IN POBJECT_ATTRIBUTES ObjectAttributes  OPTIONAL,
                           IN KPROCESSOR_MODE AccessMode,
                           IN POOL_TYPE PoolType,
                           IN BOOLEAN CaptureIfKernel,
                           OUT PCAPTURED_OBJECT_ATTRIBUTES CapturedObjectAttributes  OPTIONAL,
                           OUT PUNICODE_STRING ObjectName  OPTIONAL)
{
  OBJECT_ATTRIBUTES AttributesCopy;
  NTSTATUS Status = STATUS_SUCCESS;

  /* at least one output parameter must be != NULL! */
  ASSERT(CapturedObjectAttributes != NULL || ObjectName != NULL);

  if(ObjectAttributes == NULL)
  {
    /* we're going to return STATUS_SUCCESS! */
    goto failbasiccleanup;
  }

  if(AccessMode != KernelMode)
  {
    _SEH_TRY
    {
      ProbeForRead(ObjectAttributes,
                   sizeof(ObjectAttributes),
                   sizeof(ULONG));
      /* make a copy on the stack */
      AttributesCopy = *ObjectAttributes;
    }
    _SEH_HANDLE
    {
      Status = _SEH_GetExceptionCode();
    }
    _SEH_END;

    if(!NT_SUCCESS(Status))
    {
      DPRINT1("ObpCaptureObjectAttributes failed to probe object attributes\n");
      goto failbasiccleanup;
    }
  }
  else if(!CaptureIfKernel)
  {
    if(ObjectAttributes->Length == sizeof(OBJECT_ATTRIBUTES))
    {
      if(ObjectName != NULL)
      {
        /* we don't have to capture any memory, the caller considers the passed data
           as valid */
        if(ObjectAttributes->ObjectName != NULL)
        {
          *ObjectName = *ObjectAttributes->ObjectName;
        }
        else
        {
          ObjectName->Length = ObjectName->MaximumLength = 0;
          ObjectName->Buffer = NULL;
        }
      }
      if(CapturedObjectAttributes != NULL)
      {
        CapturedObjectAttributes->RootDirectory = ObjectAttributes->RootDirectory;
        CapturedObjectAttributes->Attributes = ObjectAttributes->Attributes;
        CapturedObjectAttributes->SecurityDescriptor = ObjectAttributes->SecurityDescriptor;
        CapturedObjectAttributes->SecurityQualityOfService = ObjectAttributes->SecurityQualityOfService;
      }

      return STATUS_SUCCESS;
    }
    else
    {
      Status = STATUS_INVALID_PARAMETER;
      goto failbasiccleanup;
    }
  }
  else
  {
    AttributesCopy = *ObjectAttributes;
  }

  /* if Length isn't as expected, bail with an invalid parameter status code so
     the caller knows he passed garbage... */
  if(AttributesCopy.Length != sizeof(OBJECT_ATTRIBUTES))
  {
    Status = STATUS_INVALID_PARAMETER;
    goto failbasiccleanup;
  }

  if(CapturedObjectAttributes != NULL)
  {
    CapturedObjectAttributes->RootDirectory = AttributesCopy.RootDirectory;
    CapturedObjectAttributes->Attributes = AttributesCopy.Attributes;

    if(AttributesCopy.SecurityDescriptor != NULL)
    {
      Status = SeCaptureSecurityDescriptor(AttributesCopy.SecurityDescriptor,
                                           AccessMode,
                                           PoolType,
                                           TRUE,
                                           &CapturedObjectAttributes->SecurityDescriptor);
      if(!NT_SUCCESS(Status))
      {
        DPRINT1("Unable to capture the security descriptor!!!\n");
        goto failbasiccleanup;
      }
    }
    else
    {
      CapturedObjectAttributes->SecurityDescriptor = NULL;
    }

    if(AttributesCopy.SecurityQualityOfService != NULL)
    {
      SECURITY_QUALITY_OF_SERVICE SafeQoS;

      _SEH_TRY
      {
        ProbeForRead(AttributesCopy.SecurityQualityOfService,
                     sizeof(SECURITY_QUALITY_OF_SERVICE),
                     sizeof(ULONG));
        SafeQoS = *(PSECURITY_QUALITY_OF_SERVICE)AttributesCopy.SecurityQualityOfService;
      }
      _SEH_HANDLE
      {
        Status = _SEH_GetExceptionCode();
      }
      _SEH_END;

      if(!NT_SUCCESS(Status))
      {
        DPRINT1("Unable to capture QoS!!!\n");
        goto failcleanupsdescriptor;
      }

      if(SafeQoS.Length != sizeof(SECURITY_QUALITY_OF_SERVICE))
      {
        DPRINT1("Unable to capture QoS, wrong size!!!\n");
        Status = STATUS_INVALID_PARAMETER;
        goto failcleanupsdescriptor;
      }

      CapturedObjectAttributes->SecurityQualityOfService = ExAllocatePool(PoolType,
                                                                          sizeof(SECURITY_QUALITY_OF_SERVICE));
      if(CapturedObjectAttributes->SecurityQualityOfService != NULL)
      {
        *CapturedObjectAttributes->SecurityQualityOfService = SafeQoS;
      }
      else
      {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto failcleanupsdescriptor;
      }
    }
    else
    {
      CapturedObjectAttributes->SecurityQualityOfService = NULL;
    }
  }

  if(ObjectName != NULL)
  {
    if(AttributesCopy.ObjectName != NULL)
    {
      UNICODE_STRING OriginalCopy;

      if(AccessMode != KernelMode)
      {
        _SEH_TRY
        {
          /* probe the ObjectName structure and make a local stack copy of it */
          ProbeForRead(AttributesCopy.ObjectName,
                       sizeof(UNICODE_STRING),
                       sizeof(ULONG));
          OriginalCopy = *AttributesCopy.ObjectName;
          if(OriginalCopy.Length > 0)
          {
            ProbeForRead(OriginalCopy.Buffer,
                         OriginalCopy.Length,
                         sizeof(WCHAR));
          }
        }
        _SEH_HANDLE
        {
          Status = _SEH_GetExceptionCode();
        }
        _SEH_END;

        if(NT_SUCCESS(Status))
        {
          if(OriginalCopy.Length > 0)
          {
            ObjectName->MaximumLength = OriginalCopy.Length + sizeof(WCHAR);
            ObjectName->Buffer = ExAllocatePool(PoolType,
                                                ObjectName->MaximumLength);
            if(ObjectName->Buffer != NULL)
            {
              _SEH_TRY
              {
                /* no need to probe OriginalCopy.Buffer again, we already did that
                   when capturing the UNICODE_STRING structure itself */
                RtlCopyMemory(ObjectName->Buffer, OriginalCopy.Buffer, OriginalCopy.Length);
                ObjectName->Buffer[OriginalCopy.Length / sizeof(WCHAR)] = L'\0';
              }
              _SEH_HANDLE
              {
                Status = _SEH_GetExceptionCode();
              }
              _SEH_END;

              if(!NT_SUCCESS(Status))
              {
                DPRINT1("ObpCaptureObjectAttributes failed to copy the unicode string!\n");
              }
            }
            else
            {
              Status = STATUS_INSUFFICIENT_RESOURCES;
            }
          }
          else if(AttributesCopy.RootDirectory != NULL /* && OriginalCopy.Length == 0 */)
          {
            /* if the caller specified a root directory, there must be an object name! */
            Status = STATUS_OBJECT_NAME_INVALID;
          }
        }
        else
        {
          DPRINT1("ObpCaptureObjectAttributes failed to probe the object name UNICODE_STRING structure!\n");
        }
      }
      else /* AccessMode == KernelMode */
      {
        OriginalCopy = *AttributesCopy.ObjectName;

        if(OriginalCopy.Length > 0)
        {
          ObjectName->MaximumLength = OriginalCopy.Length + sizeof(WCHAR);
          ObjectName->Buffer = ExAllocatePool(PoolType,
                                              ObjectName->MaximumLength);
          if(ObjectName->Buffer != NULL)
          {
            RtlCopyMemory(ObjectName->Buffer, OriginalCopy.Buffer, OriginalCopy.Length);
            ObjectName->Buffer[OriginalCopy.Length / sizeof(WCHAR)] = L'\0';
          }
          else
          {
            Status = STATUS_INSUFFICIENT_RESOURCES;
          }
        }
        else if(AttributesCopy.RootDirectory != NULL /* && OriginalCopy.Length == 0 */)
        {
          /* if the caller specified a root directory, there must be an object name! */
          Status = STATUS_OBJECT_NAME_INVALID;
        }
      }
    }
    else
    {
      ObjectName->Length = ObjectName->MaximumLength = 0;
      ObjectName->Buffer = NULL;
    }
  }

  if(!NT_SUCCESS(Status))
  {
    if(ObjectName->Buffer)
    {
      ExFreePool(ObjectName->Buffer);
    }

failcleanupsdescriptor:
    if(CapturedObjectAttributes != NULL)
    {
      /* cleanup allocated resources */
      SeReleaseSecurityDescriptor(CapturedObjectAttributes->SecurityDescriptor,
                                  AccessMode,
                                  TRUE);
    }

failbasiccleanup:
    if(ObjectName != NULL)
    {
      ObjectName->Length = ObjectName->MaximumLength = 0;
      ObjectName->Buffer = NULL;
    }
    if(CapturedObjectAttributes != NULL)
    {
      RtlZeroMemory(CapturedObjectAttributes, sizeof(CAPTURED_OBJECT_ATTRIBUTES));
    }
  }

  return Status;
}


VOID
ObpReleaseObjectAttributes(IN PCAPTURED_OBJECT_ATTRIBUTES CapturedObjectAttributes  OPTIONAL,
                           IN PUNICODE_STRING ObjectName  OPTIONAL,
                           IN KPROCESSOR_MODE AccessMode,
                           IN BOOLEAN CaptureIfKernel)
{
  /* WARNING - You need to pass the same parameters to this function as you passed
               to ObpCaptureObjectAttributes() to avoid memory leaks */
  if(AccessMode != KernelMode || CaptureIfKernel)
  {
    if(CapturedObjectAttributes != NULL)
    {
      if(CapturedObjectAttributes->SecurityDescriptor != NULL)
      {
        ExFreePool(CapturedObjectAttributes->SecurityDescriptor);
        CapturedObjectAttributes->SecurityDescriptor = NULL;
      }
      if(CapturedObjectAttributes->SecurityQualityOfService != NULL)
      {
        ExFreePool(CapturedObjectAttributes->SecurityQualityOfService);
        CapturedObjectAttributes->SecurityQualityOfService = NULL;
      }
    }
    if(ObjectName != NULL &&
       ObjectName->Length > 0)
    {
      ExFreePool(ObjectName->Buffer);
    }
  }
}


/**********************************************************************
 * NAME							PRIVATE
 * 	ObFindObject@16
 *
 * DESCRIPTION
 *
 * ARGUMENTS
 *	ObjectAttributes
 *
 *	ReturnedObject
 *
 *	RemainigPath
 *		Pointer to a unicode string that will contain the
 *		remaining path if the function returns successfully.
 *		The caller must free the buffer after use by calling
 *		RtlFreeUnicodeString ().
 *
 *	ObjectType
 *		Optional pointer to an object type. This is used to
 *		descide if a symbolic link object will be parsed or not.
 *
 * RETURN VALUE
 */
NTSTATUS
ObFindObject(POBJECT_ATTRIBUTES ObjectAttributes,
	     PVOID* ReturnedObject,
	     PUNICODE_STRING RemainingPath,
	     POBJECT_TYPE ObjectType)
{
  PVOID NextObject;
  PVOID CurrentObject;
  PVOID RootObject;
  POBJECT_HEADER CurrentHeader;
  NTSTATUS Status;
  PWSTR current;
  UNICODE_STRING PathString;
  ULONG Attributes;
  PUNICODE_STRING ObjectName;
  
  PAGED_CODE();

  DPRINT("ObFindObject(ObjectAttributes %x, ReturnedObject %x, "
	 "RemainingPath %x)\n",ObjectAttributes,ReturnedObject,RemainingPath);
  DPRINT("ObjectAttributes->ObjectName %wZ\n",
	 ObjectAttributes->ObjectName);

  RtlInitUnicodeString (RemainingPath, NULL);

  if (ObjectAttributes->RootDirectory == NULL)
    {
      ObReferenceObjectByPointer(NameSpaceRoot,
				 DIRECTORY_TRAVERSE,
				 NULL,
				 UserMode);
      CurrentObject = NameSpaceRoot;
    }
  else
    {
      Status = ObReferenceObjectByHandle(ObjectAttributes->RootDirectory,
					 DIRECTORY_TRAVERSE,
					 NULL,
					 UserMode,
					 &CurrentObject,
					 NULL);
      if (!NT_SUCCESS(Status))
	{
	  return Status;
	}
    }

  ObjectName = ObjectAttributes->ObjectName;
  if (ObjectName->Length == 0 ||
      ObjectName->Buffer[0] == UNICODE_NULL)
    {
      *ReturnedObject = CurrentObject;
      return STATUS_SUCCESS;
    }

  if (ObjectAttributes->RootDirectory == NULL &&
      ObjectName->Buffer[0] != L'\\')
    {
      ObDereferenceObject (CurrentObject);
      return STATUS_UNSUCCESSFUL;
    }

  /* Create a zero-terminated copy of the object name */
  PathString.Length = ObjectName->Length;
  PathString.MaximumLength = ObjectName->Length + sizeof(WCHAR);
  PathString.Buffer = ExAllocatePool (NonPagedPool,
				      PathString.MaximumLength);
  if (PathString.Buffer == NULL)
    {
      ObDereferenceObject (CurrentObject);
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlCopyMemory (PathString.Buffer,
		 ObjectName->Buffer,
		 ObjectName->Length);
  PathString.Buffer[PathString.Length / sizeof(WCHAR)] = UNICODE_NULL;

  current = PathString.Buffer;

  RootObject = CurrentObject;
  Attributes = ObjectAttributes->Attributes;
  if (ObjectType == ObSymbolicLinkType)
    Attributes |= OBJ_OPENLINK;

  while (TRUE)
    {
	DPRINT("current %S\n",current);
	CurrentHeader = BODY_TO_HEADER(CurrentObject);

	DPRINT("Current ObjectType %wZ\n",
	       &CurrentHeader->ObjectType->TypeName);

	if (CurrentHeader->ObjectType->Parse == NULL)
	  {
	     DPRINT("Current object can't parse\n");
	     break;
	  }
	Status = CurrentHeader->ObjectType->Parse(CurrentObject,
						  &NextObject,
						  &PathString,
						  &current,
						  Attributes);
	if (Status == STATUS_REPARSE)
	  {
	     /* reparse the object path */
	     NextObject = NameSpaceRoot;
	     current = PathString.Buffer;
	     
	     ObReferenceObjectByPointer(NextObject,
					DIRECTORY_TRAVERSE,
					NULL,
					UserMode);
	  }

	if (NextObject == NULL)
	  {
	     break;
	  }
	ObDereferenceObject(CurrentObject);
	CurrentObject = NextObject;
    }

  if (current)
     RtlpCreateUnicodeString (RemainingPath, current, NonPagedPool);
  RtlFreeUnicodeString (&PathString);
  *ReturnedObject = CurrentObject;

  return STATUS_SUCCESS;
}


/**********************************************************************
 * NAME							EXPORTED
 * 	ObQueryNameString@16
 *
 * DESCRIPTION
 *
 * ARGUMENTS
 *
 * RETURN VALUE
 *
 * @implemented
 */
NTSTATUS STDCALL
ObQueryNameString (IN PVOID Object,
		   OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
		   IN ULONG Length,
		   OUT PULONG ReturnLength)
{
  POBJECT_NAME_INFORMATION LocalInfo;
  POBJECT_HEADER ObjectHeader;
  ULONG LocalReturnLength;
  NTSTATUS Status;
  
  PAGED_CODE();

  *ReturnLength = 0;

  if (Length < sizeof(OBJECT_NAME_INFORMATION) + sizeof(WCHAR))
    return STATUS_INVALID_BUFFER_SIZE;

  ObjectNameInfo->Name.MaximumLength = (USHORT)(Length - sizeof(OBJECT_NAME_INFORMATION));
  ObjectNameInfo->Name.Length = 0;
  ObjectNameInfo->Name.Buffer =
    (PWCHAR)((ULONG_PTR)ObjectNameInfo + sizeof(OBJECT_NAME_INFORMATION));
  ObjectNameInfo->Name.Buffer[0] = 0;

  ObjectHeader = BODY_TO_HEADER(Object);

  if (ObjectHeader->ObjectType != NULL &&
      ObjectHeader->ObjectType->QueryName != NULL)
    {
      DPRINT ("Calling %x\n", ObjectHeader->ObjectType->QueryName);
      Status = ObjectHeader->ObjectType->QueryName (Object,
						    ObjectNameInfo,
						    Length,
						    ReturnLength);
    }
  else if (ObjectHeader->Name.Length > 0 && ObjectHeader->Name.Buffer != NULL)
    {
      DPRINT ("Object does not have a 'QueryName' function\n");

      if (ObjectHeader->Parent == NameSpaceRoot)
	{
	  DPRINT ("Reached the root directory\n");
	  ObjectNameInfo->Name.Length = 0;
	  ObjectNameInfo->Name.Buffer[0] = 0;
	  Status = STATUS_SUCCESS;
	}
      else if (ObjectHeader->Parent != NULL)
	{
	  LocalInfo = ExAllocatePool (NonPagedPool,
				      sizeof(OBJECT_NAME_INFORMATION) +
				      MAX_PATH * sizeof(WCHAR));
	  if (LocalInfo == NULL)
	    return STATUS_INSUFFICIENT_RESOURCES;

	  Status = ObQueryNameString (ObjectHeader->Parent,
				      LocalInfo,
				      MAX_PATH * sizeof(WCHAR),
				      &LocalReturnLength);
	  if (!NT_SUCCESS (Status))
	    {
	      ExFreePool (LocalInfo);
	      return Status;
	    }

	  Status = RtlAppendUnicodeStringToString (&ObjectNameInfo->Name,
						   &LocalInfo->Name);

	  ExFreePool (LocalInfo);

	  if (!NT_SUCCESS (Status))
	    return Status;
	}

      DPRINT ("Object path %wZ\n", &ObjectHeader->Name);
      Status = RtlAppendUnicodeToString (&ObjectNameInfo->Name,
					 L"\\");
      if (!NT_SUCCESS (Status))
	return Status;

      Status = RtlAppendUnicodeStringToString (&ObjectNameInfo->Name,
					       &ObjectHeader->Name);
    }
  else
    {
      DPRINT ("Object is unnamed\n");

      ObjectNameInfo->Name.MaximumLength = 0;
      ObjectNameInfo->Name.Length = 0;
      ObjectNameInfo->Name.Buffer = NULL;

      Status = STATUS_SUCCESS;
    }

  if (NT_SUCCESS (Status))
    {
      ObjectNameInfo->Name.MaximumLength =
	(ObjectNameInfo->Name.Length) ? ObjectNameInfo->Name.Length + sizeof(WCHAR) : 0;
      *ReturnLength =
	sizeof(OBJECT_NAME_INFORMATION) + ObjectNameInfo->Name.MaximumLength;
      DPRINT ("Returned object path: %wZ\n", &ObjectNameInfo->Name);
    }

  return Status;
}


/**********************************************************************
 * NAME							EXPORTED
 * 	ObCreateObject@36
 *
 * DESCRIPTION
 *
 * ARGUMENTS
 *
 * RETURN VALUE
 *	Status
 *
 * @implemented
 */
NTSTATUS STDCALL
ObCreateObject (IN KPROCESSOR_MODE ObjectAttributesAccessMode OPTIONAL,
		IN POBJECT_TYPE Type,
		IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
		IN KPROCESSOR_MODE AccessMode,
		IN OUT PVOID ParseContext OPTIONAL,
		IN ULONG ObjectSize,
		IN ULONG PagedPoolCharge OPTIONAL,
		IN ULONG NonPagedPoolCharge OPTIONAL,
		OUT PVOID *Object)
{
  PVOID Parent = NULL;
  UNICODE_STRING RemainingPath;
  POBJECT_HEADER Header;
  POBJECT_HEADER ParentHeader = NULL;
  NTSTATUS Status;
  BOOLEAN ObjectAttached = FALSE;
  PWCHAR NamePtr;
  PSECURITY_DESCRIPTOR NewSecurityDescriptor = NULL;
  SECURITY_SUBJECT_CONTEXT SubjectContext;

  PAGED_CODE();
  
  if(ObjectAttributesAccessMode == UserMode && ObjectAttributes != NULL)
  {
    Status = STATUS_SUCCESS;
    _SEH_TRY
    {
      ProbeForRead(ObjectAttributes,
                   sizeof(OBJECT_ATTRIBUTES),
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

  DPRINT("ObCreateObject(Type %p ObjectAttributes %p, Object %p)\n",
	 Type, ObjectAttributes, Object);

  if (Type == NULL)
    {
      DPRINT1("Invalid object type!\n");
      return STATUS_INVALID_PARAMETER;
    }

  if (ObjectAttributes != NULL &&
      ObjectAttributes->ObjectName != NULL &&
      ObjectAttributes->ObjectName->Buffer != NULL)
    {
      Status = ObFindObject(ObjectAttributes,
			    &Parent,
			    &RemainingPath,
			    NULL);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT1("ObFindObject() failed! (Status 0x%x)\n", Status);
	  return Status;
	}
    }
  else
    {
      RtlInitUnicodeString(&RemainingPath, NULL);
    }

  Header = (POBJECT_HEADER)ExAllocatePoolWithTag(NonPagedPool,
						 OBJECT_ALLOC_SIZE(ObjectSize),
						 Type->Tag);
  if (Header == NULL) {
	DPRINT1("Not enough memory!\n");
	return STATUS_INSUFFICIENT_RESOURCES;
  }

  RtlZeroMemory(Header, OBJECT_ALLOC_SIZE(ObjectSize));

  /* Initialize the object header */
  DPRINT("Initalizing header 0x%x (%wZ)\n", Header, &Type->TypeName);
  Header->HandleCount = 0;
  Header->RefCount = 1;
  Header->ObjectType = Type;
  if (ObjectAttributes != NULL &&
      ObjectAttributes->Attributes & OBJ_PERMANENT)
    {
      Header->Permanent = TRUE;
    }
  else
    {
      Header->Permanent = FALSE;
    }

  if (ObjectAttributes != NULL &&
      ObjectAttributes->Attributes & OBJ_INHERIT)
    {
      Header->Inherit = TRUE;
    }
  else
    {
      Header->Inherit = FALSE;
    }

  RtlInitUnicodeString(&(Header->Name),NULL);

  DPRINT("Getting Parent and adding entry\n");
  if (Parent != NULL)
    {
      ParentHeader = BODY_TO_HEADER(Parent);
    }

  if (ParentHeader != NULL &&
      ParentHeader->ObjectType == ObDirectoryType &&
      RemainingPath.Buffer != NULL)
    {
      NamePtr = RemainingPath.Buffer;
      if (*NamePtr == L'\\')
	NamePtr++;

      ObpAddEntryDirectory(Parent,
			   Header,
			   NamePtr);

      ObjectAttached = TRUE;
    }

  DPRINT("About to call Create Routine\n");
  if (Header->ObjectType->Create != NULL)
    {
      DPRINT("Calling %x\n", Header->ObjectType->Create);
      Status = Header->ObjectType->Create(HEADER_TO_BODY(Header),
					  Parent,
					  RemainingPath.Buffer,
					  ObjectAttributes);
      if (!NT_SUCCESS(Status))
	{
	  if (ObjectAttached == TRUE)
	    {
	      ObpRemoveEntryDirectory(Header);
	    }
	  if (Parent)
	    {
	      ObDereferenceObject(Parent);
	    }
	  RtlFreeUnicodeString(&Header->Name);
	  RtlFreeUnicodeString(&RemainingPath);
	  ExFreePool(Header);
	  DPRINT("Create Failed\n");
	  return Status;
	}
    }
  RtlFreeUnicodeString(&RemainingPath);

  SeCaptureSubjectContext(&SubjectContext);

  DPRINT("Security Assignment in progress\n");
  /* Build the new security descriptor */
  Status = SeAssignSecurity((ParentHeader != NULL) ? ParentHeader->SecurityDescriptor : NULL,
			    (ObjectAttributes != NULL) ? ObjectAttributes->SecurityDescriptor : NULL,
			    &NewSecurityDescriptor,
			    (Header->ObjectType == ObDirectoryType),
			    &SubjectContext,
			    Header->ObjectType->Mapping,
			    PagedPool);
  if (NT_SUCCESS(Status))
    {
      DPRINT("NewSecurityDescriptor %p\n", NewSecurityDescriptor);

      if (Header->ObjectType->Security != NULL)
	{
	  /* Call the security method */
	  Status = Header->ObjectType->Security(HEADER_TO_BODY(Header),
						AssignSecurityDescriptor,
						0,
						NewSecurityDescriptor,
						NULL);
	}
      else
	{
	  /* Assign the security descriptor to the object header */
	  Status = ObpAddSecurityDescriptor(NewSecurityDescriptor,
					    &Header->SecurityDescriptor);
	  DPRINT("Object security descriptor %p\n", Header->SecurityDescriptor);
	}

      /* Release the new security descriptor */
      SeDeassignSecurity(&NewSecurityDescriptor);
    }

  DPRINT("Security Complete\n");
  SeReleaseSubjectContext(&SubjectContext);

  if (Object != NULL)
    {
      *Object = HEADER_TO_BODY(Header);
    }

  DPRINT("Sucess!\n");
  return STATUS_SUCCESS;
}


/*
 * FUNCTION: Increments the pointer reference count for a given object
 * ARGUMENTS:
 *         ObjectBody = Object's body
 *         DesiredAccess = Desired access to the object
 *         ObjectType = Points to the object type structure
 *         AccessMode = Type of access check to perform
 * RETURNS: Status
 *
 * @implemented
 */
NTSTATUS STDCALL
ObReferenceObjectByPointer(IN PVOID Object,
			   IN ACCESS_MASK DesiredAccess,
			   IN POBJECT_TYPE ObjectType,
			   IN KPROCESSOR_MODE AccessMode)
{
   POBJECT_HEADER Header;
   
   /* NOTE: should be possible to reference an object above APC_LEVEL! */

   DPRINT("ObReferenceObjectByPointer(Object %x, ObjectType %x)\n",
	  Object,ObjectType);
   
   Header = BODY_TO_HEADER(Object);
   
   if (ObjectType != NULL && Header->ObjectType != ObjectType)
     {
	DPRINT("Failed %x (type was %x %S) should be %x %S\n",
		Header,
		Header->ObjectType,
		Header->ObjectType->TypeName.Buffer,
		ObjectType,
		ObjectType->TypeName.Buffer);
	return(STATUS_UNSUCCESSFUL);
     }
   if (Header->ObjectType == PsProcessType)
     {
	DPRINT("Ref p 0x%x refcount %d type %x ",
		Object, Header->RefCount, PsProcessType);
	DPRINT("eip %x\n", ((PULONG)&Object)[-1]);
     }
   if (Header->ObjectType == PsThreadType)
     {
	DPRINT("Deref t 0x%x with refcount %d type %x ",
		Object, Header->RefCount, PsThreadType);
	DPRINT("eip %x\n", ((PULONG)&Object)[-1]);
     }
 
   if (Header->RefCount == 0 && !Header->Permanent)
   {
      if (Header->ObjectType == PsProcessType)
        {
	  return STATUS_PROCESS_IS_TERMINATING;
	}
      if (Header->ObjectType == PsThreadType)
        {
	  return STATUS_THREAD_IS_TERMINATING;
	}
      return(STATUS_UNSUCCESSFUL);
   }

   if (1 == InterlockedIncrement(&Header->RefCount) && !Header->Permanent)
   {
      KEBUGCHECK(0);
   }
   
   return(STATUS_SUCCESS);
}


/*
 * @implemented
 */
NTSTATUS STDCALL
ObOpenObjectByPointer(IN POBJECT Object,
		      IN ULONG HandleAttributes,
		      IN PACCESS_STATE PassedAccessState,
		      IN ACCESS_MASK DesiredAccess,
		      IN POBJECT_TYPE ObjectType,
		      IN KPROCESSOR_MODE AccessMode,
		      OUT PHANDLE Handle)
{
   NTSTATUS Status;
   
   PAGED_CODE();
   
   DPRINT("ObOpenObjectByPointer()\n");
   
   Status = ObReferenceObjectByPointer(Object,
				       0,
				       ObjectType,
				       AccessMode);
   if (!NT_SUCCESS(Status))
     {
	return Status;
     }
   
   Status = ObCreateHandle(PsGetCurrentProcess(),
			   Object,
			   DesiredAccess,
			   (BOOLEAN)(HandleAttributes & OBJ_INHERIT),
			   Handle);
   
   ObDereferenceObject(Object);
   
   return STATUS_SUCCESS;
}


static NTSTATUS
ObpDeleteObject(POBJECT_HEADER Header)
{
  DPRINT("ObpDeleteObject(Header %p)\n", Header);
  if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
      DPRINT("ObpDeleteObject called at an unsupported IRQL.  Use ObpDeleteObjectDpcLevel instead.\n");
      KEBUGCHECK(0);
    }

  if (Header->SecurityDescriptor != NULL)
    {
      ObpRemoveSecurityDescriptor(Header->SecurityDescriptor);
    }

  if (Header->ObjectType != NULL &&
      Header->ObjectType->Delete != NULL)
    {
      Header->ObjectType->Delete(HEADER_TO_BODY(Header));
    }

  if (Header->Name.Buffer != NULL)
    {
      ObpRemoveEntryDirectory(Header);
      RtlFreeUnicodeString(&Header->Name);
    }

  DPRINT("ObPerformRetentionChecks() = Freeing object\n");
  ExFreePool(Header);

  return(STATUS_SUCCESS);
}


VOID STDCALL
ObpDeleteObjectWorkRoutine (IN PVOID Parameter)
{
  PRETENTION_CHECK_PARAMS Params = (PRETENTION_CHECK_PARAMS)Parameter;
  /* ULONG Tag; */ /* See below */

  ASSERT(Params);
  ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL); /* We need PAGED_CODE somewhere... */

  /* Turn this on when we have ExFreePoolWithTag
  Tag = Params->ObjectHeader->ObjectType->Tag; */
  ObpDeleteObject(Params->ObjectHeader);
  ExFreePool(Params);
  /* ExFreePoolWithTag(Params, Tag); */
}


STATIC NTSTATUS
ObpDeleteObjectDpcLevel(IN POBJECT_HEADER ObjectHeader,
			IN LONG OldRefCount)
{
  if (ObjectHeader->RefCount < 0)
    {
      CPRINT("Object %p/%p has invalid reference count (%d)\n",
	     ObjectHeader, HEADER_TO_BODY(ObjectHeader), 
	     ObjectHeader->RefCount);
      KEBUGCHECK(0);
    }

  if (ObjectHeader->HandleCount < 0)
    {
      CPRINT("Object %p/%p has invalid handle count (%d)\n",
	     ObjectHeader, HEADER_TO_BODY(ObjectHeader), 
	     ObjectHeader->HandleCount);
      KEBUGCHECK(0);
    }

  
  switch (KeGetCurrentIrql ())
    {
    case PASSIVE_LEVEL:
      return ObpDeleteObject (ObjectHeader);
      
    case APC_LEVEL:
    case DISPATCH_LEVEL:
      {
	PRETENTION_CHECK_PARAMS Params;
	
	/*
	  We use must succeed pool here because if the allocation fails
	  then we leak memory.
	*/
	Params = (PRETENTION_CHECK_PARAMS)
	  ExAllocatePoolWithTag(NonPagedPoolMustSucceed,
				sizeof(RETENTION_CHECK_PARAMS),
				ObjectHeader->ObjectType->Tag);
	Params->ObjectHeader = ObjectHeader;
	ExInitializeWorkItem(&Params->WorkItem,
			     ObpDeleteObjectWorkRoutine,
			     (PVOID)Params);
	ExQueueWorkItem(&Params->WorkItem,
			CriticalWorkQueue);
      }
      return STATUS_PENDING;
      
    default:
      DPRINT("ObpDeleteObjectDpcLevel called at unsupported "
	     "IRQL %u!\n", KeGetCurrentIrql());
      KEBUGCHECK(0);
      return STATUS_UNSUCCESSFUL;
    }

  return STATUS_SUCCESS;
}


/**********************************************************************
 * NAME							EXPORTED
 * 	ObfReferenceObject@4
 *
 * DESCRIPTION
 *	Increments a given object's reference count and performs
 *	retention checks.
 *
 * ARGUMENTS
 *  ObjectBody = Body of the object.
 *
 * RETURN VALUE
 * 	None.
 *
 * @implemented
 */
VOID FASTCALL
ObfReferenceObject(IN PVOID Object)
{
  POBJECT_HEADER Header;

  ASSERT(Object);

  Header = BODY_TO_HEADER(Object);

  /* No one should be referencing an object once we are deleting it. */
  if (InterlockedIncrement(&Header->RefCount) == 1 && !Header->Permanent)
  {
     KEBUGCHECK(0);
  }

}


/**********************************************************************
 * NAME							EXPORTED
 *	ObfDereferenceObject@4
 *
 * DESCRIPTION
 *	Decrements a given object's reference count and performs
 *	retention checks.
 *
 * ARGUMENTS
 *	ObjectBody = Body of the object.
 *
 * RETURN VALUE
 * 	None.
 *
 * @implemented
 */
VOID FASTCALL
ObfDereferenceObject(IN PVOID Object)
{
  POBJECT_HEADER Header;
  LONG NewRefCount;
  BOOL Permanent;

  ASSERT(Object);

  /* Extract the object header. */
  Header = BODY_TO_HEADER(Object);
  Permanent = Header->Permanent;

  /* 
     Drop our reference and get the new count so we can tell if this was the
     last reference.
  */
  NewRefCount = InterlockedDecrement(&Header->RefCount);
  DPRINT("ObfDereferenceObject(0x%x)==%d (%wZ)\n", Object, NewRefCount, &Header->ObjectType->TypeName);
  ASSERT(NewRefCount >= 0);

  /* Check whether the object can now be deleted. */
  if (NewRefCount == 0 &&
      !Permanent)
    {
      ObpDeleteObjectDpcLevel(Header, NewRefCount);
    }
}


/**********************************************************************
 * NAME							EXPORTED
 *	ObGetObjectPointerCount@4
 *
 * DESCRIPTION
 *	Retrieves the pointer(reference) count of the given object.
 *
 * ARGUMENTS
 *	ObjectBody = Body of the object.
 *
 * RETURN VALUE
 * 	Reference count.
 *
 * @implemented
 */
ULONG STDCALL
ObGetObjectPointerCount(PVOID Object)
{
  POBJECT_HEADER Header;
  
  PAGED_CODE();

  ASSERT(Object);
  Header = BODY_TO_HEADER(Object);

  return Header->RefCount;
}


/**********************************************************************
 * NAME							INTERNAL
 *	ObGetObjectHandleCount@4
 *
 * DESCRIPTION
 *	Retrieves the handle count of the given object.
 *
 * ARGUMENTS
 *	ObjectBody = Body of the object.
 *
 * RETURN VALUE
 * 	Reference count.
 */
ULONG
ObGetObjectHandleCount(PVOID Object)
{
  POBJECT_HEADER Header;
  
  PAGED_CODE();

  ASSERT(Object);
  Header = BODY_TO_HEADER(Object);

  return Header->HandleCount;
}


/**********************************************************************
 * NAME							EXPORTED
 *	ObDereferenceObject@4
 *
 * DESCRIPTION
 *	Decrements a given object's reference count and performs
 *	retention checks.
 *
 * ARGUMENTS
 *	ObjectBody = Body of the object.
 *
 * RETURN VALUE
 * 	None.
 *
 * @implemented
 */

#ifdef ObDereferenceObject
#undef ObDereferenceObject
#endif

VOID STDCALL
ObDereferenceObject(IN PVOID Object)
{
  ObfDereferenceObject(Object);
}

/* EOF */
