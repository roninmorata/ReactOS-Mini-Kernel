/* $Id: cntrller.c 13311 2005-01-26 13:58:37Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/io/cntrller.c
 * PURPOSE:         Implements the controller object
 * 
 * PROGRAMMERS:     David Welch (welch@mcmail.com)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#include <internal/debug.h>

/* GLOBALS *******************************************************************/

#define TAG_CQE                    TAG('C', 'Q', 'E', ' ')
#define TAG_CONTROLLER             TAG('C', 'N', 'T', 'R')
#define TAG_CONTROLLER_EXTENSION   TAG('C', 'E', 'X', 'T')

/* TYPES ********************************************************************/

typedef struct
/*
 * PURPOSE: A entry in the queue waiting for a controller object
 */
{
   KDEVICE_QUEUE_ENTRY Entry;
   PDEVICE_OBJECT DeviceObject;
   PDRIVER_CONTROL ExecutionRoutine;
   PVOID Context;
} CONTROLLER_QUEUE_ENTRY, *PCONTROLLER_QUEUE_ENTRY;

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
VOID
STDCALL
IoAllocateController(PCONTROLLER_OBJECT ControllerObject,
			  PDEVICE_OBJECT DeviceObject,
			  PDRIVER_CONTROL ExecutionRoutine,
			  PVOID Context)
/*
 * FUNCTION: Sets up a call to a driver-supplied ControllerControl routine
 * as soon as the device controller, represented by the given controller
 * object, is available to carry out an I/O operation for the target device,
 * represented by the given device object.
 * ARGUMENTS:
 *       ControllerObject = Driver created controller object
 *       DeviceObject = Target device for the current irp
 *       ExecutionRoutine = Routine to be called when the device is available
 *       Context = Driver supplied context to be passed on to the above routine
 * NOTE: Is the below implementation correct. 
 */
{
   PCONTROLLER_QUEUE_ENTRY entry;
   IO_ALLOCATION_ACTION Result;

   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   entry = 
     ExAllocatePoolWithTag(NonPagedPool, sizeof(CONTROLLER_QUEUE_ENTRY),
			   TAG_CQE);
   ASSERT(entry!=NULL);
   
   entry->DeviceObject = DeviceObject;
   entry->ExecutionRoutine = ExecutionRoutine;
   entry->Context = Context;
   
   if (KeInsertDeviceQueue(&ControllerObject->DeviceWaitQueue,&entry->Entry))
     {
	return;
     }   
   Result = ExecutionRoutine(DeviceObject,DeviceObject->CurrentIrp,
			     NULL,Context);
   if (Result == DeallocateObject)
     {
	IoFreeController(ControllerObject);
     }
   ExFreePool(entry);
}

/*
 * @implemented
 */
PCONTROLLER_OBJECT
STDCALL
IoCreateController(ULONG Size)
/*
 * FUNCTION: Allocates memory and initializes a controller object
 * ARGUMENTS:
 *        Size = Size (in bytes) to be allocated for the controller extension
 * RETURNS: A pointer to the created object
 */
{
   PCONTROLLER_OBJECT controller;
   
   ASSERT_IRQL(PASSIVE_LEVEL);
   
   controller = 
     ExAllocatePoolWithTag(NonPagedPool, sizeof(CONTROLLER_OBJECT),
			   TAG_CONTROLLER);
   if (controller==NULL)
     {
	return(NULL);
     }
   
   controller->ControllerExtension = 
     ExAllocatePoolWithTag(NonPagedPool, Size, TAG_CONTROLLER_EXTENSION);
   if (controller->ControllerExtension==NULL)
     {
	ExFreePool(controller);
	return(NULL);
     }

   KeInitializeDeviceQueue(&controller->DeviceWaitQueue);
   return(controller);
}

/*
 * @implemented
 */
VOID
STDCALL
IoDeleteController(PCONTROLLER_OBJECT ControllerObject)
/*
 * FUNCTION: Removes a given controller object from the system
 * ARGUMENTS:
 *        ControllerObject = Controller object to be released
 */
{
   ASSERT_IRQL(PASSIVE_LEVEL);

   ExFreePool(ControllerObject->ControllerExtension);
   ExFreePool(ControllerObject);
}

/*
 * @implemented
 */
VOID
STDCALL
IoFreeController(PCONTROLLER_OBJECT ControllerObject)
/*
 * FUNCTION: Releases a previously allocated controller object when a 
 * device has finished an I/O request
 * ARGUMENTS:
 *       ControllerObject = Controller object to be released
 */
{
   PKDEVICE_QUEUE_ENTRY QEntry;
   CONTROLLER_QUEUE_ENTRY* Entry;
   IO_ALLOCATION_ACTION Result;

   do
     {
	QEntry = KeRemoveDeviceQueue(&ControllerObject->DeviceWaitQueue);
	Entry = CONTAINING_RECORD(QEntry,CONTROLLER_QUEUE_ENTRY,Entry);
	if (QEntry==NULL)
	  {
	     return;
	  }
	Result = Entry->ExecutionRoutine(Entry->DeviceObject,
					 Entry->DeviceObject->CurrentIrp,
					 NULL,
					 Entry->Context);
	ExFreePool(Entry);
     } while (Result == DeallocateObject);
}


/* EOF */
