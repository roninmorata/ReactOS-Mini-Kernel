/* $Id: class2.h 12852 2005-01-06 13:58:04Z mf $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            services/storage/include/class2.h
 * PURPOSE:         SCSI class driver definitions
 * PROGRAMMER:      Eric Kohl (ekohl@rz-online.de)
 */

#ifndef __INCLUDE_DDK_CLASS2_H
#define __INCLUDE_DDK_CLASS2_H

#include "ntddscsi.h"
#include "srb.h"

#define MAXIMUM_RETRIES    15
#define RETRY_WAIT         2000000 /* 200 ms in units of 100 ns */

struct _CLASS_INIT_DATA;

typedef VOID STDCALL
(*PCLASS_ERROR)(IN PDEVICE_OBJECT DeviceObject,
		IN PSCSI_REQUEST_BLOCK Srb,
		IN OUT NTSTATUS *Status,
		IN OUT BOOLEAN *Retry);

typedef BOOLEAN STDCALL
(*PCLASS_DEVICE_CALLBACK)(IN PINQUIRYDATA);

typedef NTSTATUS STDCALL
(*PCLASS_READ_WRITE)(IN PDEVICE_OBJECT DeviceObject,
		     IN PIRP Irp);

typedef BOOLEAN STDCALL
(*PCLASS_FIND_DEVICES)(IN PDRIVER_OBJECT DriverObject,
		       IN PUNICODE_STRING RegistryPath,
		       IN struct _CLASS_INIT_DATA *InitializationData,
		       IN PDEVICE_OBJECT PortDeviceObject,
		       IN ULONG PortNumber);

typedef NTSTATUS STDCALL
(*PCLASS_DEVICE_CONTROL)(IN PDEVICE_OBJECT DeviceObject,
			 IN PIRP Irp);

typedef NTSTATUS STDCALL
(*PCLASS_SHUTDOWN_FLUSH)(IN PDEVICE_OBJECT DeviceObject,
			 IN PIRP Irp);

typedef NTSTATUS STDCALL
(*PCLASS_CREATE_CLOSE)(IN PDEVICE_OBJECT DeviceObject,
		       IN PIRP Irp);


typedef struct _CLASS_INIT_DATA
{
  ULONG InitializationDataSize;
  ULONG DeviceExtensionSize;
  DEVICE_TYPE DeviceType;
  ULONG DeviceCharacteristics;
  PCLASS_ERROR ClassError;
  PCLASS_READ_WRITE ClassReadWriteVerification;
  PCLASS_DEVICE_CALLBACK ClassFindDeviceCallBack;
  PCLASS_FIND_DEVICES ClassFindDevices;
  PCLASS_DEVICE_CONTROL ClassDeviceControl;
  PCLASS_SHUTDOWN_FLUSH ClassShutdownFlush;
  PCLASS_CREATE_CLOSE ClassCreateClose;
  PDRIVER_STARTIO ClassStartIo;
} CLASS_INIT_DATA, *PCLASS_INIT_DATA;


typedef struct _DEVICE_EXTENSION
{
  PDEVICE_OBJECT DeviceObject;
  PDEVICE_OBJECT PortDeviceObject;
  LARGE_INTEGER PartitionLength;
  LARGE_INTEGER StartingOffset;
  ULONG DMByteSkew;
  ULONG DMSkew;
  BOOLEAN DMActive;
  PCLASS_ERROR ClassError;
  PCLASS_READ_WRITE ClassReadWriteVerification;
  PCLASS_FIND_DEVICES ClassFindDevices;
  PCLASS_DEVICE_CONTROL ClassDeviceControl;
  PCLASS_SHUTDOWN_FLUSH ClassShutdownFlush;
  PCLASS_CREATE_CLOSE ClassCreateClose;
  PDRIVER_STARTIO ClassStartIo;
  PIO_SCSI_CAPABILITIES PortCapabilities;
  PDISK_GEOMETRY DiskGeometry;
  PDEVICE_OBJECT PhysicalDevice;
  PSENSE_DATA SenseData;
  ULONG TimeOutValue;
  ULONG DeviceNumber;
  ULONG SrbFlags;
  ULONG ErrorCount;
  KSPIN_LOCK SplitRequestSpinLock;
  NPAGED_LOOKASIDE_LIST SrbLookasideListHead;
  LONG LockCount;
  UCHAR PortNumber;
  UCHAR PathId;
  UCHAR TargetId;
  UCHAR Lun;
  UCHAR SectorShift;
  UCHAR ReservedByte;
  USHORT DeviceFlags;
  PKEVENT MediaChangeEvent;
  HANDLE MediaChangeEventHandle;
  BOOLEAN MediaChangeNoMedia;
  ULONG MediaChangeCount;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _COMPLETION_CONTEXT
{
  PDEVICE_OBJECT DeviceObject;
  SCSI_REQUEST_BLOCK Srb;
} COMPLETION_CONTEXT, *PCOMPLETION_CONTEXT;


/* FUNCTIONS ****************************************************************/

NTSTATUS STDCALL
ScsiClassAsynchronousCompletion(IN PDEVICE_OBJECT DeviceObject,
				IN PIRP Irp,
				IN PVOID Context);

VOID STDCALL
ScsiClassBuildRequest(IN PDEVICE_OBJECT DeviceObject,
		      IN PIRP Irp);

NTSTATUS STDCALL
ScsiClassClaimDevice(IN PDEVICE_OBJECT PortDeviceObject,
		     IN PSCSI_INQUIRY_DATA LunInfo,
		     IN BOOLEAN Release,
		     OUT PDEVICE_OBJECT *NewPortDeviceObject OPTIONAL);

NTSTATUS STDCALL
ScsiClassCreateDeviceObject(IN PDRIVER_OBJECT DriverObject,
			    IN PCCHAR ObjectNameBuffer,
			    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
			    IN OUT PDEVICE_OBJECT *DeviceObject,
			    IN PCLASS_INIT_DATA InitializationData);

NTSTATUS STDCALL
ScsiClassDeviceControl(IN PDEVICE_OBJECT DeviceObject,
		       IN PIRP Irp);

PVOID STDCALL
ScsiClassFindModePage(IN PCHAR ModeSenseBuffer,
		      IN ULONG Length,
		      IN UCHAR PageMode,
		      IN BOOLEAN Use6Byte);

ULONG STDCALL
ScsiClassFindUnclaimedDevices(IN PCLASS_INIT_DATA InitializationData,
			      OUT PSCSI_ADAPTER_BUS_INFO AdapterInformation);

NTSTATUS STDCALL
ScsiClassGetCapabilities(IN PDEVICE_OBJECT PortDeviceObject,
			 OUT PIO_SCSI_CAPABILITIES *PortCapabilities);

NTSTATUS STDCALL
ScsiClassGetInquiryData(IN PDEVICE_OBJECT PortDeviceObject,
			OUT PSCSI_ADAPTER_BUS_INFO *ConfigInfo);

ULONG STDCALL
ScsiClassInitialize(IN PVOID Argument1,
		    IN PVOID Argument2,
		    IN PCLASS_INIT_DATA InitializationData);

VOID STDCALL
ScsiClassInitializeSrbLookasideList(IN PDEVICE_EXTENSION DeviceExtension,
				    IN ULONG NumberElements);

NTSTATUS STDCALL
ScsiClassInternalIoControl(IN PDEVICE_OBJECT DeviceObject,
			   IN PIRP Irp);

BOOLEAN STDCALL
ScsiClassInterpretSenseInfo(IN PDEVICE_OBJECT DeviceObject,
			    IN PSCSI_REQUEST_BLOCK Srb,
			    IN UCHAR MajorFunctionCode,
			    IN ULONG IoDeviceCode,
			    IN ULONG RetryCount,
			    OUT NTSTATUS *Status);

NTSTATUS STDCALL
ScsiClassIoComplete(IN PDEVICE_OBJECT DeviceObject,
		    IN PIRP Irp,
		    IN PVOID Context);

NTSTATUS STDCALL
ScsiClassIoCompleteAssociated(IN PDEVICE_OBJECT DeviceObject,
			      IN PIRP Irp,
			      IN PVOID Context);

ULONG STDCALL
ScsiClassModeSense(IN PDEVICE_OBJECT DeviceObject,
		   IN PCHAR ModeSenseBuffer,
		   IN ULONG Length,
		   IN UCHAR PageMode);

ULONG STDCALL
ScsiClassQueryTimeOutRegistryValue(IN PUNICODE_STRING RegistryPath);

NTSTATUS STDCALL
ScsiClassReadDriveCapacity(IN PDEVICE_OBJECT DeviceObject);

VOID STDCALL
ScsiClassReleaseQueue(IN PDEVICE_OBJECT DeviceObject);

NTSTATUS STDCALL
ScsiClassSendSrbAsynchronous(PDEVICE_OBJECT DeviceObject,
			     PSCSI_REQUEST_BLOCK Srb,
			     PIRP Irp,
			     PVOID BufferAddress,
			     ULONG BufferLength,
			     BOOLEAN WriteToDevice);

NTSTATUS STDCALL
ScsiClassSendSrbSynchronous(PDEVICE_OBJECT DeviceObject,
			    PSCSI_REQUEST_BLOCK Srb,
			    PVOID BufferAddress,
			    ULONG BufferLength,
			    BOOLEAN WriteToDevice);

VOID STDCALL
ScsiClassSplitRequest(IN PDEVICE_OBJECT DeviceObject,
		      IN PIRP Irp,
		      IN ULONG MaximumBytes);

#endif /* __INCLUDE_DDK_CLASS2_H */

/* EOF */
