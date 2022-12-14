/* $Id: ntddscsi.h 12852 2005-01-06 13:58:04Z mf $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            services/storage/include/ntddscsi.h
 * PURPOSE:         Basic SCSI definitions
 * PROGRAMMER:      Eric Kohl (ekohl@rz-online.de)
 */

#ifndef __NTDDSCSI_H
#define __NTDDSCSI_H

#if __GNUC__ >=3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,4)

#include "ntddk.h"


#define DD_SCSI_DEVICE_NAME               "\\Device\\ScsiPort"
#define DD_SCSI_DEVICE_NAME_U             L"\\Device\\ScsiPort"

#define IOCTL_SCSI_BASE                   FILE_DEVICE_CONTROLLER

#define IOCTL_SCSI_GET_INQUIRY_DATA \
  CTL_CODE(IOCTL_SCSI_BASE, 0x0403, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SCSI_GET_CAPABILITIES \
  CTL_CODE(IOCTL_SCSI_BASE, 0x0404, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SCSI_GET_ADDRESS \
  CTL_CODE(IOCTL_SCSI_BASE, 0x0406, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SCSI_MINIPORT \
  CTL_CODE(IOCTL_SCSI_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_SCSI_PASS_THROUGH \
  CTL_CODE(IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_SCSI_PASS_THROUGH_DIRECT \
  CTL_CODE(IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_SCSI_RESCAN_BUS \
  CTL_CODE(IOCTL_SCSI_BASE, 0x0407, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SCSI_GET_DUMP_POINTERS \
	CTL_CODE(FILE_DEVICE_CONTROLLER, 0x0408, METHOD_BUFFERED, FILE_ANY_ACCESS)






/* Used by IOCTL_SCSI_PASS_THROUGH */

typedef struct _SCSI_PASS_THROUGH {
  USHORT Length;
  UCHAR ScsiStatus;
  UCHAR PathId;
  UCHAR TargetId;
  UCHAR Lun;
  UCHAR CdbLength;
  UCHAR SenseInfoLength;
  UCHAR DataIn;
  ULONG DataTransferLength;
  ULONG TimeOutValue;
  ULONG DataBufferOffset;
  ULONG SenseInfoOffset;
  UCHAR Cdb[16];
} SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;


/* Used by IOCTL_SCSI_PASS_THROUGH_DIRECT */

typedef struct _SCSI_PASS_THROUGH_DIRECT {
  USHORT Length;
  UCHAR ScsiStatus;
  UCHAR PathId;
  UCHAR TargetId;
  UCHAR Lun;
  UCHAR CdbLength;
  UCHAR SenseInfoLength;
  UCHAR DataIn;
  ULONG DataTransferLength;
  ULONG TimeOutValue;
  ULONG DataBufferOffset;
  ULONG SenseInfoOffset;
  UCHAR Cdb[16];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

typedef struct _SRB_IO_CONTROL { 
  ULONG  HeaderLength; 
  UCHAR  Signature[8]; 
  ULONG  Timeout; 
  ULONG  ControlCode; 
  ULONG  ReturnCode; 
  ULONG  Length; 
} SRB_IO_CONTROL, *PSRB_IO_CONTROL; 

/* Used by IOCTL_SCSI_GET_ADDRESS */

typedef struct _SCSI_ADDRESS {
  ULONG Length;
  UCHAR PortNumber;
  UCHAR PathId;
  UCHAR TargetId;
  UCHAR Lun;
} SCSI_ADDRESS, *PSCSI_ADDRESS;

typedef struct _SCSI_BUS_DATA {
  UCHAR NumberOfLogicalUnits;
  UCHAR InitiatorBusId;
  ULONG InquiryDataOffset;
}SCSI_BUS_DATA, *PSCSI_BUS_DATA;

typedef struct _SCSI_ADAPTER_BUS_INFO {
  UCHAR NumberOfBuses;
  SCSI_BUS_DATA BusData[1];
} SCSI_ADAPTER_BUS_INFO, *PSCSI_ADAPTER_BUS_INFO;

/* Used by IOCTL_SCSI_GET_CAPABILITIES */

typedef struct _IO_SCSI_CAPABILITIES {
  ULONG Length;
  ULONG MaximumTransferLength;
  ULONG MaximumPhysicalPages;
  ULONG SupportedAsynchronousEvents;
  ULONG AlignmentMask;
  BOOLEAN TaggedQueuing;
  BOOLEAN AdapterScansDown;
  BOOLEAN AdapterUsesPio;
} IO_SCSI_CAPABILITIES, *PIO_SCSI_CAPABILITIES;

/* Used by IOCTL_SCSI_GET_INQUIRY_DATA */

typedef struct _SCSI_INQUIRY_DATA {
  UCHAR PathId;
  UCHAR TargetId;
  UCHAR Lun;
  BOOLEAN DeviceClaimed;
  ULONG InquiryDataLength;
  ULONG NextInquiryDataOffset;
  UCHAR InquiryData[1];
}SCSI_INQUIRY_DATA, *PSCSI_INQUIRY_DATA;

/* Pass through DataIn */

#define SCSI_IOCTL_DATA_OUT		0
#define SCSI_IOCTL_DATA_IN		1
#define SCSI_IOCTL_DATA_UNSPECIFIED	2

typedef struct _DUMP_POINTERS {

	PADAPTER_OBJECT  AdapterObject;
	PVOID  MappedRegisterBase;
	PVOID  DumpData;
	PVOID  CommonBufferVa;
	LARGE_INTEGER  CommonBufferPa;
	ULONG  CommonBufferSize;
	BOOLEAN  AllocateCommonBuffers;
	BOOLEAN  UseDiskDump;
	UCHAR  Spare1[2];

  PVOID DeviceObject;
} DUMP_POINTERS, *PDUMP_POINTERS;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* __NTDDSCSI_H */
