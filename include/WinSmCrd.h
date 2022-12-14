/*
 * WinSmCrd.h
 *
 * SmartCard IOCTLs
 *
 * THIS SOFTWARE IS NOT COPYRIGHTED
 *
 * This source code is offered for use in the public domain. You may
 * use, modify or distribute it freely.
 *
 * This code is distributed in the hope that it will be useful but
 * WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 * DISCLAIMED. This includes but is not limited to warranties of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef __WINSMCRD_H
#define __WINSMCRD_H
#if __GNUC__ >=3
#pragma GCC system_header
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SCARD_IO_REQUEST
{
  DWORD dwProtocol;
  DWORD cbPciLength;
} SCARD_IO_REQUEST, *PSCARD_IO_REQUEST, *LPSCARD_IO_REQUEST;
typedef const SCARD_IO_REQUEST *LPCSCARD_IO_REQUEST;

typedef struct _SCARD_T0_COMMAND
{
  BYTE bCla;
  BYTE bIns;
  BYTE P1;
  BYTE P2;
  BYTE P3
} SCARD_T0_COMMAND, *PSCARD_T0_COMMAND, *LPSCARD_T0_COMMAND;

typedef struct _SCARD_T0_REQUEST
{
  SCARD_IO_REQUEST ioRequest;
  BYTE bSw1;
  BYTE bSw2;
  union
  {
    SCARD_T0_COMMAND CmdBytes;
    BYTE rgbHeader[5];
  } u;
} SCARD_T0_REQUEST, *PSCARD_T0_REQUEST, *LPSCARD_T0_REQUEST;

typedef struct _SCARD_T1_REQUEST
{
  SCARD_IO_REQUEST ioRequest;
} SCARD_T1_REQUEST, *PSCARD_T1_REQUEST, *LPSCARD_T1_REQUEST;

#define FILE_DEVICE_SMARTCARD (0x00000031)

#define SCARD_ATR_LENGTH (0x21)

#define SCARD_PROTOCOL_UNDEFINED (0x00000)
#define SCARD_PROTOCOL_T0        (0x00001)
#define SCARD_PROTOCOL_T1        (0x00002)
#define SCARD_PROTOCOL_RAW       (0x10000)

#define SCARD_PROTOCOL_Tx (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1)

#define SCARD_PROTOCOL_DEFAULT (0x80000000)
#define SCARD_PROTOCOL_OPTIMAL (0x00000000)

#define SCARD_T0_HEADER_LENGTH (0x7)
#define SCARD_T0_CMD_LENGTH    (0x5)

#define SCARD_T1_PROLOGUE_LENGTH (0x03)
#define SCARD_T1_EPILOGUE_LENGTH (0x02)
#define SCARD_T1_MAX_IFS         (0xFE)

#define SCARD_POWER_DOWN (0x0)
#define SCARD_COLD_RESET (0x1)
#define SCARD_WARM_RESET (0x2)

#define SCARD_UNKNOWN    (0x0)
#define SCARD_ABSENT     (0x1)
#define SCARD_PRESENT    (0x2)
#define SCARD_SWALLOWED  (0x3)
#define SCARD_POWERED    (0x4)
#define SCARD_NEGOTIABLE (0x5)
#define SCARD_SPECIFIC   (0x6)

#define IOCTL_SMARTCARD_POWER          CTL_CODE(FILE_DEVICE_SMARTCARD, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_GET_ATTRIBUTE  CTL_CODE(FILE_DEVICE_SMARTCARD, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_SET_ATTRIBUTE  CTL_CODE(FILE_DEVICE_SMARTCARD, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_CONFISCATE     CTL_CODE(FILE_DEVICE_SMARTCARD, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_TRANSMIT       CTL_CODE(FILE_DEVICE_SMARTCARD, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_EJECT          CTL_CODE(FILE_DEVICE_SMARTCARD, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_SWALLOW        CTL_CODE(FILE_DEVICE_SMARTCARD, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_READ           CTL_CODE(FILE_DEVICE_SMARTCARD, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_WRITE          CTL_CODE(FILE_DEVICE_SMARTCARD, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_IS_PRESENT     CTL_CODE(FILE_DEVICE_SMARTCARD, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_IS_ABSENT      CTL_CODE(FILE_DEVICE_SMARTCARD, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_SET_PROTOCOL   CTL_CODE(FILE_DEVICE_SMARTCARD, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_GET_STATE      CTL_CODE(FILE_DEVICE_SMARTCARD, 14, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_GET_LAST_ERROR CTL_CODE(FILE_DEVICE_SMARTCARD, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMARTCARD_GET_PERF_CNTR  CTL_CODE(FILE_DEVICE_SMARTCARD, 16, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define MAXIMUM_ATTR_STRING_LENGTH (0x10)
#define MAXIMUM_SMARTCARD_READERS  (0x0A)

#define SCARD_CLASS_VENDOR_INFO    (0x0001)
#define SCARD_CLASS_COMMUNICATIONS (0x0002)
#define SCARD_CLASS_PROTOCOL       (0x0003)
#define SCARD_CLASS_POWER_MGMT     (0x0004)
#define SCARD_CLASS_SECURITY       (0x0005)
#define SCARD_CLASS_MECHANICAL     (0x0006)
#define SCARD_CLASS_VENDOR_DEFINED (0x0007)
#define SCARD_CLASS_IFD_PROTOCOL   (0x0008)
#define SCARD_CLASS_ICC_STATE      (0x0009)
#define SCARD_CLASS_PERF           (0x7FFE)
#define SCARD_CLASS_SYSTEM         (0x7FFF)

#define SCARD_READER_TYPE_VENDOR   (0xF0)

#define SCARD_READER_TYPE_SERIAL   (0x01)
#define SCARD_READER_TYPE_PARALELL (0x02)
#define SCARD_READER_TYPE_KEYBOARD (0x04)
#define SCARD_READER_TYPE_SCSI     (0x08)
#define SCARD_READER_TYPE_IDE      (0x10)
#define SCARD_READER_TYPE_USB      (0x20)
#define SCARD_READER_TYPE_PCMCIA   (0x40)

#define SCARD_READER_SWALLOWS    (0x1)
#define SCARD_READER_EJECTS      (0x2)
#define SCARD_READER_CONFISCATES (0x4)

#define SCARD_ATTR_VALUE(Class, Tag)                                           \
  (((ULONG)(Class) << 16) | (ULONG)(Tag))

#define SCARD_ATTR_VENDOR_NAME              SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0100)
#define SCARD_ATTR_VENDOR_IFD_TYPE          SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0101)
#define SCARD_ATTR_VENDOR_IFD_VERSION       SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0102)
#define SCARD_ATTR_VENDOR_IFD_SERIAL_NO     SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0103)

#define SCARD_ATTR_CHANNEL_ID               SCARD_ATTR_VALUE(SCARD_CLASS_COMMUNICATIONS, 0x0110)

#define SCARD_ATTR_PROTOCOL_TYPES           SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0120)
#define SCARD_ATTR_ASYNC_PROTOCOL_TYPES     SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0120)
#define SCARD_ATTR_DEFAULT_CLK              SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0121)
#define SCARD_ATTR_MAX_CLK                  SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0122)
#define SCARD_ATTR_DEFAULT_DATA_RATE        SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0123)
#define SCARD_ATTR_MAX_DATA_RATE            SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0124)
#define SCARD_ATTR_MAX_IFSD                 SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0125)
#define SCARD_ATTR_SYNC_PROTOCOL_TYPES      SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0126)

#define SCARD_ATTR_POWER_MGMT_SUPPORT       SCARD_ATTR_VALUE(SCARD_CLASS_POWER_MGMT, 0x0131)

#define SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE SCARD_ATTR_VALUE(SCARD_CLASS_SECURITY, 0x0140)
#define SCARD_ATTR_USER_AUTH_INPUT_DEVICE   SCARD_ATTR_VALUE(SCARD_CLASS_SECURITY, 0x0142)

#define SCARD_ATTR_CHARACTERISTICS          SCARD_ATTR_VALUE(SCARD_CLASS_MECHANICAL, 0x0150)

#define SCARD_ATTR_CURRENT_PROTOCOL_TYPE    SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0201)
#define SCARD_ATTR_CURRENT_CLK              SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0202)
#define SCARD_ATTR_CURRENT_F                SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0203)
#define SCARD_ATTR_CURRENT_D                SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0204)
#define SCARD_ATTR_CURRENT_N                SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0205)
#define SCARD_ATTR_CURRENT_W                SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0206)
#define SCARD_ATTR_CURRENT_IFSC             SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0207)
#define SCARD_ATTR_CURRENT_IFSD             SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0208)
#define SCARD_ATTR_CURRENT_BWT              SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0209)
#define SCARD_ATTR_CURRENT_CWT              SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x020A)
#define SCARD_ATTR_CURRENT_EBC_ENCODING     SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x020B)
#define SCARD_ATTR_EXTENDED_BWT             SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x020C)

#define SCARD_ATTR_ICC_PRESENCE             SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0300)
#define SCARD_ATTR_ICC_INTERFACE_STATUS     SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0301)
#define SCARD_ATTR_CURRENT_IO_STATE         SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0302)
#define SCARD_ATTR_ATR_STRING               SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0303)
#define SCARD_ATTR_ICC_TYPE_PER_ATR         SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0304)

#define SCARD_ATTR_ESC_RESET                SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA000)
#define SCARD_ATTR_ESC_CANCEL               SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA003)
#define SCARD_ATTR_ESC_AUTHREQUEST          SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA005)
#define SCARD_ATTR_MAXINPUT                 SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA007)

#define SCARD_ATTR_DEVICE_UNIT              SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0001)
#define SCARD_ATTR_DEVICE_IN_USE            SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0002)
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME_A   SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0003)
#define SCARD_ATTR_DEVICE_SYSTEM_NAME_A     SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0004)
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME_W   SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0005)
#define SCARD_ATTR_DEVICE_SYSTEM_NAME_W     SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0006)
#define SCARD_ATTR_SUPRESS_T1_IFS_REQUEST   SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0007)

#define SCARD_PERF_NUM_TRANSMISSIONS        SCARD_ATTR_VALUE(SCARD_CLASS_PERF, 0x0001)
#define SCARD_PERF_BYTES_TRANSMITTED        SCARD_ATTR_VALUE(SCARD_CLASS_PERF, 0x0002)
#define SCARD_PERF_TRANSMISSION_TIME        SCARD_ATTR_VALUE(SCARD_CLASS_PERF, 0x0003)

#ifdef UNICODE
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME SCARD_ATTR_DEVICE_FRIENDLY_NAME_W
#define SCARD_ATTR_DEVICE_SYSTEM_NAME SCARD_ATTR_DEVICE_SYSTEM_NAME_W
#else /* !UNICODE */
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME SCARD_ATTR_DEVICE_FRIENDLY_NAME_A
#define SCARD_ATTR_DEVICE_SYSTEM_NAME SCARD_ATTR_DEVICE_SYSTEM_NAME_A
#endif /* UNICODE */

#ifdef __cplusplus
}
#endif
#endif /* __WINSMCRD_H */

/* EOF */
