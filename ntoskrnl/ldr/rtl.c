/* $Id: rtl.c 13312 2005-01-26 14:00:41Z ekohl $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ldr/rtl.c
 * PURPOSE:         Loader utilities
 *
 * PROGRAMMERS:     Jean Michault
 *                  Rex Jolliff (rex@lvcablemodem.com)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* FUNCTIONS ****************************************************************/

#define RVA(m, b) ((ULONG)b + m)

NTSTATUS STDCALL
LdrGetProcedureAddress (IN PVOID BaseAddress,
                        IN PANSI_STRING Name,
                        IN ULONG Ordinal,
                        OUT PVOID *ProcedureAddress)
{
   PIMAGE_EXPORT_DIRECTORY ExportDir;
   PUSHORT OrdinalPtr;
   PULONG NamePtr;
   PULONG AddressPtr;
   ULONG i = 0;

   /* get the pointer to the export directory */
   ExportDir = (PIMAGE_EXPORT_DIRECTORY)
	RtlImageDirectoryEntryToData (BaseAddress, TRUE, 
				      IMAGE_DIRECTORY_ENTRY_EXPORT, &i);

   if (!ExportDir || !i || !ProcedureAddress)
     {
	return(STATUS_INVALID_PARAMETER);
     }
   
   AddressPtr = (PULONG)RVA((char*)BaseAddress, ExportDir->AddressOfFunctions);
   if (Name && Name->Length)
     {
       LONG minn, maxn;

	/* by name */
       OrdinalPtr = 
	 (PUSHORT)RVA((char*)BaseAddress, ExportDir->AddressOfNameOrdinals);
       NamePtr = (PULONG)RVA((char*)BaseAddress, ExportDir->AddressOfNames);

	minn = 0; maxn = ExportDir->NumberOfNames;
	while (minn <= maxn)
	  {
	    LONG mid;
	    LONG res;

	    mid = (minn + maxn) / 2;
	    res = _strnicmp(Name->Buffer, (PCH)RVA((char*)BaseAddress, NamePtr[mid]),
			    Name->Length);
	    if (res == 0)
	      {
		*ProcedureAddress = 
		  (PVOID)RVA((char*)BaseAddress, AddressPtr[OrdinalPtr[mid]]);
		return(STATUS_SUCCESS);
	      }
	    else if (res > 0)
	      {
		maxn = mid - 1;
	      }
	    else
	      {
		minn = mid + 1;
	      }
	  }

	for (i = 0; i < ExportDir->NumberOfNames; i++, NamePtr++, OrdinalPtr++)
	  {
	     if (!_strnicmp(Name->Buffer, 
			    (char*)((char*)BaseAddress + *NamePtr), Name->Length))
	       {
		  *ProcedureAddress = 
		    (PVOID)((ULONG)BaseAddress + 
			    (ULONG)AddressPtr[*OrdinalPtr]);
		  return STATUS_SUCCESS;
	       }
	  }
	CPRINT("LdrGetProcedureAddress: Can't resolve symbol '%Z'\n", Name);
     }
   else
     {
	/* by ordinal */
	Ordinal &= 0x0000FFFF;
	if (Ordinal - ExportDir->Base < ExportDir->NumberOfFunctions)
	  {
	     *ProcedureAddress = 
	       (PVOID)((ULONG)BaseAddress + 
		       (ULONG)AddressPtr[Ordinal - ExportDir->Base]);
	     return STATUS_SUCCESS;
	  }
	CPRINT("LdrGetProcedureAddress: Can't resolve symbol @%d\n",
		 Ordinal);
  }

   return STATUS_PROCEDURE_NOT_FOUND;
}

/* EOF */
