#ifndef _FMIFS_H
#define _FMIFS_H
/* $Id: fmifs.h 12852 2005-01-06 13:58:04Z mf $
 *
 * fmifs.h
 *
 * Copyright (c) 1998 Mark Russinovich
 * Systems Internals
 * http://www.sysinternals.com
 *
 * Typedefs and definitions for using chkdsk and formatex
 * functions exported by the fmifs.dll library.
 *
 * ---
 *
 * 1999-02-18 (Emanuele Aliberti)
 * 	Normalized function names.
 *
 */

/* Output command */
typedef
struct
{
	ULONG Lines;
	PCHAR Output;
	
} TEXTOUTPUT, *PTEXTOUTPUT;


/* Callback command types */
typedef
enum
{
	PROGRESS,
	DONEWITHSTRUCTURE,
	UNKNOWN2,
	UNKNOWN3,
	UNKNOWN4,
	UNKNOWN5,
	INSUFFICIENTRIGHTS,
	UNKNOWN7,
	UNKNOWN8,
	UNKNOWN9,
	UNKNOWNA,
	DONE,
	UNKNOWNC,
	UNKNOWND,
	OUTPUT,
	STRUCTUREPROGRESS

} CALLBACKCOMMAND;


/* FMIFS callback definition */
typedef
BOOLEAN
(STDCALL * PFMIFSCALLBACK) (
	CALLBACKCOMMAND	Command,
	ULONG		SubAction,
	PVOID		ActionInfo
	);

/* Chkdsk command in FMIFS */
VOID
STDCALL
Chkdsk(
	PWCHAR		DriveRoot,
	PWCHAR		Format,
	BOOLEAN		CorrectErrors,
	BOOLEAN		Verbose,
	BOOLEAN		CheckOnlyIfDirty,
	BOOLEAN		ScanDrive,
	PVOID		Unused2,
	PVOID		Unused3,
	PFMIFSCALLBACK	Callback
	);

/* ChkdskEx command in FMIFS (not in the original) */
VOID
STDCALL
ChkDskEx(
	PWCHAR		DriveRoot,
	PWCHAR		Format,
	BOOLEAN		CorrectErrors,
	BOOLEAN		Verbose,
	BOOLEAN		CheckOnlyIfDirty,
	BOOLEAN		ScanDrive,
	PVOID		Unused2,
	PVOID		Unused3,
	PFMIFSCALLBACK	Callback
	);

/* DiskCopy command in FMIFS */

VOID
STDCALL
DiskCopy(VOID);

/* Enable/Disable volume compression */
BOOL
STDCALL
EnableVolumeCompression(
	PWCHAR	DriveRoot,
	USHORT Compression
	);

/* Format command in FMIFS */

/* media flags */
#define FMIFS_HARDDISK 0xC
#define FMIFS_FLOPPY   0x8

VOID
STDCALL
FormatEx(
	PWCHAR		DriveRoot,
	ULONG		MediaFlag,
	PWCHAR		Format,
	PWCHAR		Label,
	BOOLEAN		QuickFormat,
	ULONG		ClusterSize,
	PFMIFSCALLBACK	Callback
	);

#endif /* ndef _FMIFS_H */
