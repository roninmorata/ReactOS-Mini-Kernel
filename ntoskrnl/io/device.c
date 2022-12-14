/* $Id: device.c 13945 2005-03-12 00:49:18Z navaraf $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/io/device.c
 * PURPOSE:         Manage devices
 * 
 * PROGRAMMERS:     David Welch (welch@cwcom.net)
 */

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* GLOBALS ********************************************************************/

#define TAG_DEVICE_EXTENSION   TAG('D', 'E', 'X', 'T')

static ULONG IopDeviceObjectNumber = 0;

/* PRIVATE FUNCTIONS **********************************************************/

NTSTATUS FASTCALL
IopInitializeDevice(
   PDEVICE_NODE DeviceNode,
   PDRIVER_OBJECT DriverObject)
{
   IO_STATUS_BLOCK IoStatusBlock;
   IO_STACK_LOCATION Stack;
   PDEVICE_OBJECT Fdo;
   NTSTATUS Status;

   if (DriverObject->DriverExtension->AddDevice)
   {
      /* This is a Plug and Play driver */
      DPRINT("Plug and Play driver found\n");

      ASSERT(DeviceNode->PhysicalDeviceObject);

      DPRINT("Calling driver AddDevice entrypoint at %08lx\n",
         DriverObject->DriverExtension->AddDevice);

      Status = DriverObject->DriverExtension->AddDevice(
         DriverObject, DeviceNode->PhysicalDeviceObject);

      if (!NT_SUCCESS(Status))
      {
         return Status;
      }

      Fdo = IoGetAttachedDeviceReference(DeviceNode->PhysicalDeviceObject);

      if (Fdo == DeviceNode->PhysicalDeviceObject)
      {
         /* FIXME: What do we do? Unload the driver or just disable the device? */
         DbgPrint("An FDO was not attached\n");
         IopDeviceNodeSetFlag(DeviceNode, DNF_DISABLED);
         return STATUS_UNSUCCESSFUL;
      }

      IopDeviceNodeSetFlag(DeviceNode, DNF_ADDED);

      DPRINT("Sending IRP_MN_START_DEVICE to driver\n");

      /* FIXME: Should be DeviceNode->ResourceList */
      Stack.Parameters.StartDevice.AllocatedResources = DeviceNode->BootResources;
      /* FIXME: Should be DeviceNode->ResourceListTranslated */
      Stack.Parameters.StartDevice.AllocatedResourcesTranslated = DeviceNode->BootResources;

      Status = IopInitiatePnpIrp(
         Fdo,
         &IoStatusBlock,
         IRP_MN_START_DEVICE,
         &Stack);

      if (!NT_SUCCESS(Status))
      {
          DPRINT("IopInitiatePnpIrp() failed\n");
          ObDereferenceObject(Fdo);
          return Status;
      }

      if (Fdo->DeviceType == FILE_DEVICE_ACPI)
      {
         static BOOLEAN SystemPowerDeviceNodeCreated = FALSE;

         /* There can be only one system power device */
         if (!SystemPowerDeviceNodeCreated)
         {
            PopSystemPowerDeviceNode = DeviceNode;
            SystemPowerDeviceNodeCreated = TRUE;
         }
      }

      if (Fdo->DeviceType == FILE_DEVICE_BUS_EXTENDER ||
          Fdo->DeviceType == FILE_DEVICE_ACPI)
      {
         DPRINT("Bus extender found\n");

         Status = IopInvalidateDeviceRelations(DeviceNode, BusRelations);
         if (!NT_SUCCESS(Status))
         {
            ObDereferenceObject(Fdo);
            return Status;
         }
      }

      ObDereferenceObject(Fdo);
   }

   return STATUS_SUCCESS;
}

NTSTATUS STDCALL
IopCreateDevice(
   PVOID ObjectBody,
   PVOID Parent,
   PWSTR RemainingPath,
   POBJECT_ATTRIBUTES ObjectAttributes)
{
   DPRINT("IopCreateDevice(ObjectBody %x, Parent %x, RemainingPath %S)\n",
      ObjectBody, Parent, RemainingPath);
   
   if (RemainingPath != NULL && wcschr(RemainingPath + 1, '\\') != NULL)
      return STATUS_OBJECT_PATH_NOT_FOUND;
   
   return STATUS_SUCCESS;
}

/* PUBLIC FUNCTIONS ***********************************************************/

/*
 * IoAttachDeviceByPointer
 *
 * Status
 *    @implemented
 */

NTSTATUS STDCALL
IoAttachDeviceByPointer(
   IN PDEVICE_OBJECT SourceDevice,
   IN PDEVICE_OBJECT TargetDevice)
{
   PDEVICE_OBJECT AttachedDevice;

   DPRINT("IoAttachDeviceByPointer(SourceDevice %x, TargetDevice %x)\n",
      SourceDevice, TargetDevice);

   AttachedDevice = IoAttachDeviceToDeviceStack(SourceDevice, TargetDevice);
   if (AttachedDevice == NULL)
      return STATUS_NO_SUCH_DEVICE;

   return STATUS_SUCCESS;
}

/*
 * @unimplemented
 */
NTSTATUS
STDCALL
IoAttachDeviceToDeviceStackSafe(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    OUT PDEVICE_OBJECT *AttachedToDeviceObject
    )
{
   /* FIXME: IoAttachDeviceToDeviceStackSafe must not call
    * IoAttachDeviceToDeviceStack, but the other way around! */
   DPRINT1("IoAttachDeviceToDeviceStackSafe() badly implemented!\n");
   *AttachedToDeviceObject = IoAttachDeviceToDeviceStack(SourceDevice, TargetDevice);
   return STATUS_SUCCESS;
}

/*
 * IoDeleteDevice
 *
 * Status
 *    @implemented
 */

VOID STDCALL
IoDeleteDevice(PDEVICE_OBJECT DeviceObject)
{
   PDEVICE_OBJECT Previous;

   if (DeviceObject->Flags & DO_SHUTDOWN_REGISTERED)
      IoUnregisterShutdownNotification(DeviceObject);

   /* Remove the timer if it exists */
   if (DeviceObject->Timer)
   {
      IopRemoveTimerFromTimerList(DeviceObject->Timer);
      ExFreePool(DeviceObject->Timer);
   }

   /* Free device extension */
   if (DeviceObject->DeviceObjectExtension)
      ExFreePool(DeviceObject->DeviceObjectExtension);

   /* Remove device from driver device list */
   Previous = DeviceObject->DriverObject->DeviceObject;
   if (Previous == DeviceObject)
   {
      DeviceObject->DriverObject->DeviceObject = DeviceObject->NextDevice;
   }
   else
   {
      while (Previous->NextDevice != DeviceObject)
         Previous = Previous->NextDevice;
      Previous->NextDevice = DeviceObject->NextDevice;
   }

   ObDereferenceObject(DeviceObject);
}


/*
 * @unimplemented
 */
NTSTATUS
STDCALL
IoEnumerateDeviceObjectList(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  *DeviceObjectList,
    IN  ULONG           DeviceObjectListSize,
    OUT PULONG          ActualNumberDeviceObjects
    )
{
	UNIMPLEMENTED;
	return STATUS_NOT_IMPLEMENTED;
}


/*
 * @unimplemented
 */
PDEVICE_OBJECT
STDCALL
IoGetDeviceAttachmentBaseRef(
    IN PDEVICE_OBJECT DeviceObject
    )
{
	UNIMPLEMENTED;
	return 0;
}

/*
 * @unimplemented
 */
NTSTATUS
STDCALL
IoGetDiskDeviceObject(
    IN  PDEVICE_OBJECT  FileSystemDeviceObject,
    OUT PDEVICE_OBJECT  *DiskDeviceObject
    )
{
	UNIMPLEMENTED;
	return STATUS_NOT_IMPLEMENTED;
}

/*
 * @unimplemented
 */
PDEVICE_OBJECT
STDCALL
IoGetLowerDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
	UNIMPLEMENTED;
	return 0;
}

/*
 * IoGetRelatedDeviceObject
 *
 * Remarks
 *    See "Windows NT File System Internals", page 633 - 634.
 *
 * Status
 *    @implemented
 */

PDEVICE_OBJECT STDCALL
IoGetRelatedDeviceObject(
   IN PFILE_OBJECT FileObject)
{
   /*
    * Get logical volume mounted on a physical/virtual/logical device
    */

   if (FileObject->Vpb && FileObject->Vpb->DeviceObject)
   {
      return IoGetAttachedDevice(FileObject->Vpb->DeviceObject);
   }

   /*
    * Check if file object has an associated device object mounted by some
    * other file system.
    */

   if (FileObject->DeviceObject->Vpb &&
       FileObject->DeviceObject->Vpb->DeviceObject)
   {
      return IoGetAttachedDevice(FileObject->DeviceObject->Vpb->DeviceObject);
   }

   return IoGetAttachedDevice(FileObject->DeviceObject);
}

/*
 * IoGetDeviceObjectPointer
 *
 * Status
 *    @implemented
 */

NTSTATUS STDCALL
IoGetDeviceObjectPointer(
   IN PUNICODE_STRING ObjectName,
   IN ACCESS_MASK DesiredAccess,
   OUT PFILE_OBJECT *FileObject,
   OUT PDEVICE_OBJECT *DeviceObject)
{
   OBJECT_ATTRIBUTES ObjectAttributes;
   IO_STATUS_BLOCK StatusBlock;
   PFILE_OBJECT LocalFileObject;
   HANDLE FileHandle;
   NTSTATUS Status;

   DPRINT("IoGetDeviceObjectPointer(ObjectName %wZ, DesiredAccess %x, FileObject %p DeviceObject %p)\n",
      ObjectName, DesiredAccess, FileObject, DeviceObject);

   InitializeObjectAttributes(
      &ObjectAttributes,
      ObjectName,
      0,
      NULL,
      NULL);

   Status = ZwOpenFile(
      &FileHandle,
      DesiredAccess,
      &ObjectAttributes,
      &StatusBlock,
      0,
      FILE_NON_DIRECTORY_FILE);

   if (!NT_SUCCESS(Status))
      return Status;

   Status = ObReferenceObjectByHandle(
      FileHandle,
      0,
      IoFileObjectType,
      KernelMode,
      (PVOID*)&LocalFileObject,
      NULL);

   if (NT_SUCCESS(Status))
   {
      *DeviceObject = IoGetRelatedDeviceObject(LocalFileObject);
      *FileObject = LocalFileObject;
   }

   ZwClose(FileHandle);

   return Status;
}

/*
 * IoDetachDevice
 *
 * Status
 *    @unimplemented
 */

VOID STDCALL
IoDetachDevice(PDEVICE_OBJECT TargetDevice)
{
   DPRINT("IoDetachDevice(TargetDevice %x) - UNIMPLEMENTED\n", TargetDevice);
}

/*
 * IoGetAttachedDevice
 *
 * Status
 *    @implemented
 */

PDEVICE_OBJECT STDCALL
IoGetAttachedDevice(PDEVICE_OBJECT DeviceObject)
{
   PDEVICE_OBJECT Current = DeviceObject;
   
   while (Current->AttachedDevice != NULL)
      Current = Current->AttachedDevice;

   return Current;
}

/*
 * IoGetAttachedDeviceReference
 *
 * Status
 *    @implemented
 */

PDEVICE_OBJECT STDCALL
IoGetAttachedDeviceReference(PDEVICE_OBJECT DeviceObject)
{
   PDEVICE_OBJECT Current = IoGetAttachedDevice(DeviceObject);
   ObReferenceObject(Current);
   return Current;
}

/*
 * IoAttachDeviceToDeviceStack
 *
 * Status
 *    @implemented
 */

PDEVICE_OBJECT STDCALL
IoAttachDeviceToDeviceStack(
   PDEVICE_OBJECT SourceDevice,
   PDEVICE_OBJECT TargetDevice)
{
   PDEVICE_OBJECT AttachedDevice;
   
   DPRINT("IoAttachDeviceToDeviceStack(SourceDevice %x, TargetDevice %x)\n",
      SourceDevice, TargetDevice);

   AttachedDevice = IoGetAttachedDevice(TargetDevice);
   AttachedDevice->AttachedDevice = SourceDevice;
   SourceDevice->AttachedDevice = NULL;
   SourceDevice->StackSize = AttachedDevice->StackSize + 1;
   SourceDevice->AlignmentRequirement = AttachedDevice->AlignmentRequirement;
   SourceDevice->SectorSize = AttachedDevice->SectorSize;
   SourceDevice->Vpb = AttachedDevice->Vpb;
   return AttachedDevice;
}

/*
 * IoAttachDevice
 *
 * Layers a device over the highest device in a device stack.
 *
 * Parameters
 *    SourceDevice
 *       Device to be attached.
 *
 *    TargetDevice
 *       Name of the target device.
 *
 *    AttachedDevice
 *       Caller storage for the device attached to.
 *
 * Status
 *    @implemented
 */

NTSTATUS STDCALL
IoAttachDevice(
   PDEVICE_OBJECT SourceDevice,
   PUNICODE_STRING TargetDeviceName,
   PDEVICE_OBJECT *AttachedDevice)
{
   NTSTATUS Status;
   PFILE_OBJECT FileObject;
   PDEVICE_OBJECT TargetDevice;
     
   Status = IoGetDeviceObjectPointer(
      TargetDeviceName,
      FILE_READ_ATTRIBUTES,
      &FileObject,
      &TargetDevice);
   
   if (!NT_SUCCESS(Status))
   {
      return Status;
   }

   *AttachedDevice = IoAttachDeviceToDeviceStack(
      SourceDevice,
      TargetDevice);

   ObDereferenceObject(FileObject);

   return STATUS_SUCCESS;
}

/*
 * IoCreateDevice
 *
 * Allocates memory for and intializes a device object for use for
 * a driver.
 *
 * Parameters
 *    DriverObject
 *       Driver object passed by IO Manager when the driver was loaded.
 *
 *    DeviceExtensionSize
 *       Number of bytes for the device extension.
 *
 *    DeviceName
 *       Unicode name of device.
 *
 *    DeviceType
 *       Device type of the new device.
 *
 *    DeviceCharacteristics
 *       Bit mask of device characteristics.
 *
 *    Exclusive
 *       TRUE if only one thread can access the device at a time.
 *
 *    DeviceObject
 *       On successful return this parameter is filled by pointer to
 *       allocated device object.
 *
 * Status
 *    @implemented
 */

NTSTATUS STDCALL
IoCreateDevice(
   PDRIVER_OBJECT DriverObject,
   ULONG DeviceExtensionSize,
   PUNICODE_STRING DeviceName,
   DEVICE_TYPE DeviceType,
   ULONG DeviceCharacteristics,
   BOOLEAN Exclusive,
   PDEVICE_OBJECT *DeviceObject)
{
   WCHAR AutoNameBuffer[20];
   UNICODE_STRING AutoName;
   PDEVICE_OBJECT CreatedDeviceObject;
   PDEVOBJ_EXTENSION DeviceObjectExtension;
   OBJECT_ATTRIBUTES ObjectAttributes;
   NTSTATUS Status;
   
   ASSERT_IRQL(PASSIVE_LEVEL);
   
   if (DeviceName != NULL)
   {
      DPRINT("IoCreateDevice(DriverObject %x, DeviceName %S)\n",
         DriverObject, DeviceName->Buffer);
   }
   else
   {
      DPRINT("IoCreateDevice(DriverObject %x)\n",DriverObject);
   }
   
   if (DeviceCharacteristics & FILE_AUTOGENERATED_DEVICE_NAME)
   {
      swprintf(AutoNameBuffer,
               L"\\Device\\%08lx",
               InterlockedIncrementUL(&IopDeviceObjectNumber));
      RtlInitUnicodeString(&AutoName,
                           AutoNameBuffer);
      DeviceName = &AutoName;
   }
   
   if (DeviceName != NULL)
   {
      InitializeObjectAttributes(&ObjectAttributes, DeviceName, 0, NULL, NULL);
      Status = ObCreateObject(
         KernelMode,
         IoDeviceObjectType,
         &ObjectAttributes,
         KernelMode,
         NULL,
         sizeof(DEVICE_OBJECT),
         0,
         0,
         (PVOID*)&CreatedDeviceObject);
   }
   else
   {
      Status = ObCreateObject(
         KernelMode,
         IoDeviceObjectType,
         NULL,
         KernelMode,
         NULL,
         sizeof(DEVICE_OBJECT),
         0,
         0,
         (PVOID*)&CreatedDeviceObject);
   }
   
   *DeviceObject = NULL;
   
   if (!NT_SUCCESS(Status))
   {
      DPRINT("IoCreateDevice() ObCreateObject failed, status: 0x%08X\n", Status);
      return Status;
   }
  
   if (DriverObject->DeviceObject == NULL)
   {
      DriverObject->DeviceObject = CreatedDeviceObject;
      CreatedDeviceObject->NextDevice = NULL;
   }
   else
   {
      CreatedDeviceObject->NextDevice = DriverObject->DeviceObject;
      DriverObject->DeviceObject = CreatedDeviceObject;
   }
  
   CreatedDeviceObject->Type = DeviceType;
   CreatedDeviceObject->DriverObject = DriverObject;
   CreatedDeviceObject->CurrentIrp = NULL;
   CreatedDeviceObject->Flags = 0;

   CreatedDeviceObject->DeviceExtension = 
      ExAllocatePoolWithTag(
         NonPagedPool,
         DeviceExtensionSize,
         TAG_DEVICE_EXTENSION);

   if (DeviceExtensionSize > 0 && CreatedDeviceObject->DeviceExtension == NULL)
   {
      ExFreePool(CreatedDeviceObject);
      DPRINT("IoCreateDevice() ExAllocatePoolWithTag failed, returning: 0x%08X\n", STATUS_INSUFFICIENT_RESOURCES);
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   if (DeviceExtensionSize > 0)
   {
      RtlZeroMemory(CreatedDeviceObject->DeviceExtension, DeviceExtensionSize);
   }

   CreatedDeviceObject->Size = sizeof(DEVICE_OBJECT) + DeviceExtensionSize;
   CreatedDeviceObject->ReferenceCount = 1;
   CreatedDeviceObject->AttachedDevice = NULL;
   CreatedDeviceObject->DeviceType = DeviceType;
   CreatedDeviceObject->StackSize = 1;
   CreatedDeviceObject->AlignmentRequirement = 1;
   CreatedDeviceObject->Characteristics = DeviceCharacteristics;
   CreatedDeviceObject->Timer = NULL;
   CreatedDeviceObject->Vpb = NULL;
   KeInitializeDeviceQueue(&CreatedDeviceObject->DeviceQueue);
  
   KeInitializeEvent(
      &CreatedDeviceObject->DeviceLock,
      SynchronizationEvent,
      TRUE);
  
   /* FIXME: Do we need to add network drives too?! */
   if (CreatedDeviceObject->DeviceType == FILE_DEVICE_DISK ||
       CreatedDeviceObject->DeviceType == FILE_DEVICE_CD_ROM ||
       CreatedDeviceObject->DeviceType == FILE_DEVICE_TAPE)
   {
      IoAttachVpb(CreatedDeviceObject);
   }
   CreatedDeviceObject->SectorSize = 512; /* FIXME */
  
   DeviceObjectExtension =
      ExAllocatePoolWithTag(
         NonPagedPool,
         sizeof(DEVOBJ_EXTENSION),
         TAG_DEVICE_EXTENSION);

   DeviceObjectExtension->Type = 0 /* ?? */;
   DeviceObjectExtension->Size = sizeof(DEVOBJ_EXTENSION);
   DeviceObjectExtension->DeviceObject = CreatedDeviceObject;
   DeviceObjectExtension->DeviceNode = NULL;

   CreatedDeviceObject->DeviceObjectExtension = DeviceObjectExtension;

   *DeviceObject = CreatedDeviceObject;
  
   return STATUS_SUCCESS;
}

/*
 * IoOpenDeviceInstanceKey
 *
 * Status
 *    @unimplemented
 */

NTSTATUS STDCALL
IoOpenDeviceInstanceKey(
   DWORD Unknown0,
   DWORD Unknown1,
   DWORD Unknown2,
   DWORD Unknown3,
   DWORD Unknown4)
{
   UNIMPLEMENTED;
   return STATUS_NOT_IMPLEMENTED;
}

/*
 * @unimplemented
 */
VOID
STDCALL
IoRegisterBootDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    )
{
	UNIMPLEMENTED;
}


/*
 * @unimplemented
 */
NTSTATUS
STDCALL
IoRegisterLastChanceShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    )
{
	UNIMPLEMENTED;
	return STATUS_NOT_IMPLEMENTED;
}

/*
 * IoQueryDeviceEnumInfo
 *
 * Status
 *    @unimplemented
 */

DWORD STDCALL
IoQueryDeviceEnumInfo(
   DWORD Unknown0,
   DWORD Unknown1)
{
   UNIMPLEMENTED;
   return 0;
}

/*
 * @unimplemented
 */
VOID
STDCALL
IoSetStartIoAttributes(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeferredStartIo,
    IN BOOLEAN NonCancelable
    )
{
	UNIMPLEMENTED;
}

/*
 * @unimplemented
 */
VOID
STDCALL
IoSynchronousInvalidateDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN DEVICE_RELATION_TYPE Type
    )
{
	UNIMPLEMENTED;
}


/*
 * @unimplemented
 */
NTSTATUS
STDCALL
IoValidateDeviceIoControlAccess(
    IN  PIRP    Irp,
    IN  ULONG   RequiredAccess
    )
{
	UNIMPLEMENTED;
	return STATUS_NOT_IMPLEMENTED;
}

/* EOF */
