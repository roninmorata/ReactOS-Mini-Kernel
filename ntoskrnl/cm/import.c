/* $Id: import.c 13383 2005-02-01 17:53:55Z gdalsnes $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/cm/import.c
 * PURPOSE:         Registry-Hive import functions
 *
 * PROGRAMMERS:     Eric Kohl
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

#include "cm.h"

/* GLOBALS ******************************************************************/

static BOOLEAN CmiHardwareHiveImported = FALSE;


/* FUNCTIONS ****************************************************************/

static BOOLEAN
CmImportBinaryHive (PCHAR ChunkBase,
		    ULONG ChunkSize,
		    ULONG Flags,
		    PREGISTRY_HIVE *RegistryHive)
{
  PREGISTRY_HIVE Hive;
  NTSTATUS Status;

  *RegistryHive = NULL;

  if (strncmp (ChunkBase, "regf", 4))
    {
      DPRINT1 ("Found invalid '%*s' magic\n", 4, ChunkBase);
      return FALSE;
    }

  /* Create a new hive */
  Hive = ExAllocatePool (NonPagedPool,
			 sizeof(REGISTRY_HIVE));
  if (Hive == NULL)
    {
      return FALSE;
    }
  RtlZeroMemory (Hive,
		 sizeof(REGISTRY_HIVE));

  /* Set hive flags */
  Hive->Flags = Flags;

  /* Allocate hive header */
  Hive->HiveHeader = (PHIVE_HEADER)ExAllocatePool (NonPagedPool,
						   sizeof(HIVE_HEADER));
  if (Hive->HiveHeader == NULL)
    {
      DPRINT1 ("Allocating hive header failed\n");
      ExFreePool (Hive);
      return FALSE;
    }

  /* Import the hive header */
  RtlCopyMemory (Hive->HiveHeader,
		 ChunkBase,
		 sizeof(HIVE_HEADER));

  /* Read update counter */
  Hive->UpdateCounter = Hive->HiveHeader->UpdateCounter1;

  /* Set the hive's size */
  Hive->FileSize = ChunkSize;

  /* Set the size of the block list */
  Hive->BlockListSize = (Hive->FileSize / 4096) - 1;

  /* Allocate block list */
  DPRINT("Space needed for block list describing hive: 0x%x\n",
	 Hive->BlockListSize * sizeof(BLOCK_LIST_ENTRY));
  Hive->BlockList = ExAllocatePool (NonPagedPool,
				    Hive->BlockListSize * sizeof(BLOCK_LIST_ENTRY));
  if (Hive->BlockList == NULL)
    {
      DPRINT1 ("Allocating block list failed\n");
      ExFreePool (Hive->HiveHeader);
      ExFreePool (Hive);
      return FALSE;
    }
  RtlZeroMemory (Hive->BlockList,
		 Hive->BlockListSize * sizeof(BLOCK_LIST_ENTRY));

  /* Import the bins */
  Status = CmiImportHiveBins(Hive,
			     (PUCHAR)((ULONG_PTR)ChunkBase + 4096));
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("CmiImportHiveBins() failed (Status %lx)\n", Status);
      CmiFreeHiveBins (Hive);
      ExFreePool (Hive->BlockList);
      ExFreePool (Hive->HiveHeader);
      ExFreePool (Hive);
      return (BOOLEAN)Status;
    }

  /* Initialize the free cell list */
  Status = CmiCreateHiveFreeCellList (Hive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("CmiCreateHiveFreeCellList() failed (Status %lx)\n", Status);
      CmiFreeHiveBins (Hive);
      ExFreePool (Hive->BlockList);
      ExFreePool (Hive->HiveHeader);
      ExFreePool (Hive);

      return (BOOLEAN)Status;
    }

  if (!(Hive->Flags & HIVE_NO_FILE))
    {
      /* Create the block bitmap */
      Status = CmiCreateHiveBitmap (Hive);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT1 ("CmiCreateHiveBitmap() failed (Status %lx)\n", Status);
	  CmiFreeHiveFreeCellList (Hive);
	  CmiFreeHiveBins (Hive);
	  ExFreePool (Hive->BlockList);
	  ExFreePool (Hive->HiveHeader);
	  ExFreePool (Hive);

	  return (BOOLEAN)Status;
	}
    }

  /* Acquire hive list lock exclusively */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  /* Add the new hive to the hive list */
  InsertTailList(&CmiHiveListHead, &Hive->HiveList);

  /* Release hive list lock */
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  *RegistryHive = Hive;

  return TRUE;
}


BOOLEAN INIT_FUNCTION
CmImportSystemHive(PCHAR ChunkBase,
		   ULONG ChunkSize)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  PREGISTRY_HIVE RegistryHive;
  UNICODE_STRING KeyName;
  NTSTATUS Status;

  DPRINT ("CmImportSystemHive() called\n");

  if (strncmp (ChunkBase, "regf", 4))
    {
      DPRINT1 ("Found invalid '%.*s' magic\n", 4, ChunkBase);
      return FALSE;
    }

  DPRINT ("Found '%.*s' magic\n", 4, ChunkBase);

  /* Import the binary system hive (non-volatile, offset-based, permanent) */
  if (!CmImportBinaryHive (ChunkBase, ChunkSize, 0, &RegistryHive))
    {
      DPRINT1 ("CmiImportBinaryHive() failed\n");
      return FALSE;
    }

  /* Attach it to the machine key */
  RtlInitUnicodeString (&KeyName,
			REG_SYSTEM_KEY_NAME);
  InitializeObjectAttributes (&ObjectAttributes,
			      &KeyName,
			      OBJ_CASE_INSENSITIVE,
			      NULL,
			      NULL);
  Status = CmiConnectHive (&ObjectAttributes,
			   RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("CmiConnectHive(%wZ) failed (Status %lx)\n", &KeyName, Status);
//      CmiRemoveRegistryHive(RegistryHive);
      return FALSE;
    }

  /* Set the hive filename */
  RtlpCreateUnicodeString (&RegistryHive->HiveFileName,
           SYSTEM_REG_FILE, NonPagedPool);

  /* Set the log filename */
  RtlpCreateUnicodeString (&RegistryHive->LogFileName,
           SYSTEM_LOG_FILE, NonPagedPool);

  return TRUE;
}


BOOLEAN INIT_FUNCTION
CmImportHardwareHive(PCHAR ChunkBase,
		     ULONG ChunkSize)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  PREGISTRY_HIVE RegistryHive;
  UNICODE_STRING KeyName;
  HANDLE HardwareKey;
  ULONG Disposition;
  NTSTATUS Status;

  DPRINT ("CmImportHardwareHive() called\n");

  if (CmiHardwareHiveImported == TRUE)
    return TRUE;

  if (ChunkBase == NULL &&
      ChunkSize == 0)
    {
      /* Create '\Registry\Machine\HARDWARE' key. */
      RtlInitUnicodeString (&KeyName,
			    REG_HARDWARE_KEY_NAME);
      InitializeObjectAttributes (&ObjectAttributes,
				  &KeyName,
				  OBJ_CASE_INSENSITIVE,
				  NULL,
				  NULL);
      Status = ZwCreateKey (&HardwareKey,
			    KEY_ALL_ACCESS,
			    &ObjectAttributes,
			    0,
			    NULL,
			    REG_OPTION_VOLATILE,
			    &Disposition);
      if (!NT_SUCCESS(Status))
	{
          DPRINT1("NtCreateKey() failed, status: 0x%x\n", Status);
          return FALSE;
	}
      ZwClose (HardwareKey);

      /* Create '\Registry\Machine\HARDWARE\DESCRIPTION' key. */
      RtlInitUnicodeString(&KeyName,
			   REG_DESCRIPTION_KEY_NAME);
      InitializeObjectAttributes (&ObjectAttributes,
				  &KeyName,
				  OBJ_CASE_INSENSITIVE,
				  NULL,
				  NULL);
      Status = ZwCreateKey (&HardwareKey,
			    KEY_ALL_ACCESS,
			    &ObjectAttributes,
			    0,
			    NULL,
			    REG_OPTION_VOLATILE,
			    &Disposition);
      if (!NT_SUCCESS(Status))
	{
          DPRINT1("NtCreateKey() failed, status: 0x%x\n", Status);
          return FALSE;
	}
      ZwClose (HardwareKey);

      /* Create '\Registry\Machine\HARDWARE\DEVICEMAP' key. */
      RtlInitUnicodeString (&KeyName,
			    REG_DEVICEMAP_KEY_NAME);
      InitializeObjectAttributes (&ObjectAttributes,
				  &KeyName,
				  OBJ_CASE_INSENSITIVE,
				  NULL,
				  NULL);
      Status = ZwCreateKey (&HardwareKey,
			    KEY_ALL_ACCESS,
			    &ObjectAttributes,
			    0,
			    NULL,
			    REG_OPTION_VOLATILE,
			    &Disposition);
      if (!NT_SUCCESS(Status))
	{
          DPRINT1("NtCreateKey() failed, status: 0x%x\n", Status);
          return FALSE;
	}
      ZwClose (HardwareKey);

      /* Create '\Registry\Machine\HARDWARE\RESOURCEMAP' key. */
      RtlInitUnicodeString(&KeyName,
			   REG_RESOURCEMAP_KEY_NAME);
      InitializeObjectAttributes (&ObjectAttributes,
				  &KeyName,
				  OBJ_CASE_INSENSITIVE,
				  NULL,
				  NULL);
      Status = ZwCreateKey (&HardwareKey,
			    KEY_ALL_ACCESS,
			    &ObjectAttributes,
			    0,
			    NULL,
			    REG_OPTION_VOLATILE,
			    &Disposition);
      if (!NT_SUCCESS(Status))
	{
          DPRINT1("NtCreateKey() failed, status: 0x%x\n", Status);
          return FALSE;
	}
      ZwClose (HardwareKey);

      return TRUE;
    }

  /* Check the hive magic */
  if (strncmp (ChunkBase, "regf", 4))
    {
      DPRINT1 ("Found invalid '%.*s' magic\n", 4, ChunkBase);
      return FALSE;
    }

  DPRINT ("Found '%.*s' magic\n", 4, ChunkBase);
  DPRINT ("ChunkBase %lx  ChunkSize %lu\n", ChunkBase, ChunkSize);

  /* Import the binary system hive (volatile, offset-based, permanent) */
  if (!CmImportBinaryHive (ChunkBase, ChunkSize, HIVE_NO_FILE, &RegistryHive))
    {
      DPRINT1 ("CmiImportBinaryHive() failed\n");
      return FALSE;
    }

  /* Attach it to the machine key */
  RtlInitUnicodeString (&KeyName,
			REG_HARDWARE_KEY_NAME);
  InitializeObjectAttributes (&ObjectAttributes,
			      &KeyName,
			      OBJ_CASE_INSENSITIVE,
			      NULL,
			      NULL);
  Status = CmiConnectHive (&ObjectAttributes,
			   RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("CmiConnectHive(%wZ) failed (Status %lx)\n", &KeyName, Status);
//      CmiRemoveRegistryHive(RegistryHive);
      return FALSE;
    }

  /* Set the hive filename */
  RtlInitUnicodeString (&RegistryHive->HiveFileName,
			NULL);

  /* Set the log filename */
  RtlInitUnicodeString (&RegistryHive->LogFileName,
			NULL);

  CmiHardwareHiveImported = TRUE;

  return TRUE;
}

/* EOF */
