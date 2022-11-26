/* $Id: tape.h 12852 2005-01-06 13:58:04Z mf $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            include/ntos/tape.h
 * PURPOSE:         Tape drive definitions
 * PROGRAMMER:      Eric Kohl
 */

#ifndef __INCLUDE_TAPE_H
#define __INCLUDE_TAPE_H


#define IOCTL_TAPE_BASE                   FILE_DEVICE_TAPE

#define IOCTL_TAPE_CHECK_VERIFY \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0200, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_TAPE_CREATE_PARTITION \
  CTL_CODE(IOCTL_TAPE_BASE, 0x000a, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_TAPE_ERASE \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0000, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_TAPE_FIND_NEW_DEVICES \
  CTL_CODE(IOCTL_DISK_BASE, 0x0206, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_TAPE_GET_DRIVE_PARAMS \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0005, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_TAPE_GET_MEDIA_PARAMS \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0007, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_TAPE_GET_POSITION \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0003, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_TAPE_GET_STATUS \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0009, METHOD_BUFFERED, FILE_READ_ACCESS)


#define IOCTL_TAPE_PREPARE \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0001, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_TAPE_SET_DRIVE_PARAMS \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0006, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_TAPE_SET_MEDIA_PARAMS \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0008, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_TAPE_SET_POSITION \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0004, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_TAPE_WRITE_MARKS \
  CTL_CODE(IOCTL_TAPE_BASE, 0x0002, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


#endif /* __INCLUDE_TAPE_H */