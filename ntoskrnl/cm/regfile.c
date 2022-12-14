/* $Id: regfile.c 13383 2005-02-01 17:53:55Z gdalsnes $
 * 
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/cm/regfile.c
 * PURPOSE:         Registry file manipulation routines
 *
 * PROGRAMMERS:     No programmer listed.
 */

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

#include "cm.h"


/* uncomment to enable hive checks (incomplete and probably buggy) */
// #define HIVE_CHECK

/* LOCAL MACROS *************************************************************/

#define ROUND_DOWN(N, S) ((N) - ((N) % (S)))

#define ABS_VALUE(V) (((V) < 0) ? -(V) : (V))

BOOLEAN CmiDoVerify = FALSE;

static ULONG
CmiCalcChecksum(PULONG Buffer);

/* FUNCTIONS ****************************************************************/

VOID
CmiCreateDefaultHiveHeader(PHIVE_HEADER Header)
{
  ASSERT(Header);
  RtlZeroMemory(Header, sizeof(HIVE_HEADER));
  Header->BlockId = REG_HIVE_ID;
  Header->UpdateCounter1 = 0;
  Header->UpdateCounter2 = 0;
  Header->DateModified.u.LowPart = 0;
  Header->DateModified.u.HighPart = 0;
  Header->Unused3 = 1;
  Header->Unused4 = 3;
  Header->Unused5 = 0;
  Header->Unused6 = 1;
  Header->Unused7 = 1;
  Header->RootKeyOffset = (BLOCK_OFFSET)-1;
  Header->BlockSize = REG_BLOCK_SIZE;
  Header->Unused6 = 1;
  Header->Checksum = 0;
}


VOID
CmiCreateDefaultBinHeader(PHBIN BinHeader)
{
  ASSERT(BinHeader);
  RtlZeroMemory(BinHeader, sizeof(HBIN));
  BinHeader->HeaderId = REG_BIN_ID;
  BinHeader->DateModified.u.LowPart = 0;
  BinHeader->DateModified.u.HighPart = 0;
  BinHeader->BinSize = REG_BLOCK_SIZE;
}


VOID
CmiCreateDefaultRootKeyCell(PKEY_CELL RootKeyCell)
{
  ASSERT(RootKeyCell);
  RtlZeroMemory(RootKeyCell, sizeof(KEY_CELL));
  RootKeyCell->CellSize = -sizeof(KEY_CELL);
  RootKeyCell->Id = REG_KEY_CELL_ID;
  RootKeyCell->Flags = REG_KEY_ROOT_CELL | REG_KEY_NAME_PACKED;
  KeQuerySystemTime(&RootKeyCell->LastWriteTime);
  RootKeyCell->ParentKeyOffset = 0;
  RootKeyCell->NumberOfSubKeys = 0;
  RootKeyCell->HashTableOffset = -1;
  RootKeyCell->NumberOfValues = 0;
  RootKeyCell->ValueListOffset = -1;
  RootKeyCell->SecurityKeyOffset = 0;
  RootKeyCell->ClassNameOffset = -1;
  RootKeyCell->NameSize = 0;
  RootKeyCell->ClassSize = 0;
}


VOID
CmiVerifyBinHeader(PHBIN BinHeader)
{
  if (CmiDoVerify)
    {

  ASSERT(BinHeader);

  if (BinHeader->HeaderId != REG_BIN_ID)
    {
      DbgPrint("Bin header ID is %.08x (should be %.08x)\n",
        BinHeader->HeaderId, REG_BIN_ID);
      ASSERT(BinHeader->HeaderId == REG_BIN_ID);
    }

  //BinHeader->DateModified.dwLowDateTime

  //BinHeader->DateModified.dwHighDateTime

  
  if (BinHeader->BinSize != REG_BLOCK_SIZE)
    {
      DbgPrint("BinSize is %.08x (should be a multiple of %.08x)\n",
        BinHeader->BinSize, REG_BLOCK_SIZE);
      ASSERT(BinHeader->BinSize % REG_BLOCK_SIZE == 0);
    }

    }
}


VOID
CmiVerifyKeyCell(PKEY_CELL KeyCell)
{
  if (CmiDoVerify)
    {

  ASSERT(KeyCell);

  if (KeyCell->CellSize == 0)
    {
      DbgPrint("CellSize is %d (must not be 0)\n",
        KeyCell->CellSize);
      ASSERT(KeyCell->CellSize != 0);
    }

  if (KeyCell->Id != REG_KEY_CELL_ID)
    {
      DbgPrint("Id is %.08x (should be %.08x)\n",
        KeyCell->Id, REG_KEY_CELL_ID);
      ASSERT(KeyCell->Id == REG_KEY_CELL_ID);
    }

  //KeyCell->Flags;

  //KeyCell->LastWriteTime;

  if (KeyCell->ParentKeyOffset < 0)
    {
      DbgPrint("ParentKeyOffset is %d (must not be < 0)\n",
        KeyCell->ParentKeyOffset);
      ASSERT(KeyCell->ParentKeyOffset >= 0);
    }

  if (KeyCell->NumberOfSubKeys < 0)
    {
      DbgPrint("NumberOfSubKeys is %d (must not be < 0)\n",
        KeyCell->NumberOfSubKeys);
      ASSERT(KeyCell->NumberOfSubKeys >= 0);
    }

  //KeyCell->HashTableOffset;

  if (KeyCell->NumberOfValues < 0)
    {
      DbgPrint("NumberOfValues is %d (must not be < 0)\n",
        KeyCell->NumberOfValues);
      ASSERT(KeyCell->NumberOfValues >= 0);
    }

  //KeyCell->ValuesOffset = -1;

  if (KeyCell->SecurityKeyOffset < 0)
    {
      DbgPrint("SecurityKeyOffset is %d (must not be < 0)\n",
        KeyCell->SecurityKeyOffset);
      ASSERT(KeyCell->SecurityKeyOffset >= 0);
    }

  //KeyCell->ClassNameOffset = -1;

  //KeyCell->NameSize

  //KeyCell->ClassSize

    }
}


VOID
CmiVerifyRootKeyCell(PKEY_CELL RootKeyCell)
{
  if (CmiDoVerify)
    {

  CmiVerifyKeyCell(RootKeyCell);

  if (!(RootKeyCell->Flags & REG_KEY_ROOT_CELL))
    {
      DbgPrint("Flags is %.08x (should be %.08x)\n",
        RootKeyCell->Flags, REG_KEY_ROOT_CELL | REG_KEY_NAME_PACKED);
      ASSERT(!(RootKeyCell->Flags & (REG_KEY_ROOT_CELL | REG_KEY_NAME_PACKED)));
    }

    }
}


VOID
CmiVerifyValueCell(PVALUE_CELL ValueCell)
{
  if (CmiDoVerify)
    {

  ASSERT(ValueCell);

  if (ValueCell->CellSize == 0)
    {
      DbgPrint("CellSize is %d (must not be 0)\n",
        ValueCell->CellSize);
      ASSERT(ValueCell->CellSize != 0);
    }

  if (ValueCell->Id != REG_VALUE_CELL_ID)
    {
      DbgPrint("Id is %.08x (should be %.08x)\n",
        ValueCell->Id, REG_VALUE_CELL_ID);
      ASSERT(ValueCell->Id == REG_VALUE_CELL_ID);
    }

  //ValueCell->NameSize;
  //ValueCell->LONG  DataSize;
  //ValueCell->DataOffset;
  //ValueCell->ULONG  DataType;
  //ValueCell->USHORT Flags;
  //ValueCell->USHORT Unused1;
  //ValueCell->UCHAR  Name[0];
    }
}


VOID
CmiVerifyValueListCell(PVALUE_LIST_CELL ValueListCell)
{
  if (CmiDoVerify)
    {

  if (ValueListCell->CellSize == 0)
    {
      DbgPrint("CellSize is %d (must not be 0)\n",
        ValueListCell->CellSize);
      ASSERT(ValueListCell->CellSize != 0);
    }

    }
}


VOID
CmiVerifyKeyObject(PKEY_OBJECT KeyObject)
{
  if (CmiDoVerify)
    {

  if (KeyObject->RegistryHive == NULL)
    {
      DbgPrint("RegistryHive is NULL (must not be NULL)\n",
        KeyObject->RegistryHive);
      ASSERT(KeyObject->RegistryHive != NULL);
    }

  if (KeyObject->KeyCell == NULL)
    {
      DbgPrint("KeyCell is NULL (must not be NULL)\n",
        KeyObject->KeyCell);
      ASSERT(KeyObject->KeyCell != NULL);
    }

  if (KeyObject->ParentKey == NULL)
    {
      DbgPrint("ParentKey is NULL (must not be NULL)\n",
        KeyObject->ParentKey);
      ASSERT(KeyObject->ParentKey != NULL);
    }

    }
}


VOID
CmiVerifyHiveHeader(PHIVE_HEADER Header)
{
  if (CmiDoVerify)
    {

  if (Header->BlockId != REG_HIVE_ID)
    {
      DbgPrint("BlockId is %.08x (must be %.08x)\n",
        Header->BlockId,
        REG_HIVE_ID);
      ASSERT(Header->BlockId == REG_HIVE_ID);
    }

  if (Header->Unused3 != 1)
    {
      DbgPrint("Unused3 is %.08x (must be 1)\n",
        Header->Unused3);
      ASSERT(Header->Unused3 == 1);
    }

  if (Header->Unused4 != 3)
    {
      DbgPrint("Unused4 is %.08x (must be 3)\n",
        Header->Unused4);
      ASSERT(Header->Unused4 == 3);
    }

  if (Header->Unused5 != 0)
    {
      DbgPrint("Unused5 is %.08x (must be 0)\n",
        Header->Unused5);
      ASSERT(Header->Unused5 == 0);
    }

  if (Header->Unused6 != 1)
    {
      DbgPrint("Unused6 is %.08x (must be 1)\n",
        Header->Unused6);
      ASSERT(Header->Unused6 == 1);
    }

  if (Header->Unused7 != 1)
    {
      DbgPrint("Unused7 is %.08x (must be 1)\n",
        Header->Unused7);
      ASSERT(Header->Unused7 == 1);
    }

    }
}


VOID
CmiVerifyRegistryHive(PREGISTRY_HIVE RegistryHive)
{
  if (CmiDoVerify)
    {

      CmiVerifyHiveHeader(RegistryHive->HiveHeader);

    }
}


static NTSTATUS
CmiCreateNewRegFile(HANDLE FileHandle)
{
  IO_STATUS_BLOCK IoStatusBlock;
  PCELL_HEADER FreeCell;
  PHIVE_HEADER HiveHeader;
  PKEY_CELL RootKeyCell;
  NTSTATUS Status;
  PHBIN BinHeader;
  PCHAR Buffer;

  Buffer = (PCHAR) ExAllocatePool(NonPagedPool, 2 * REG_BLOCK_SIZE);
  if (Buffer == NULL)
    return STATUS_INSUFFICIENT_RESOURCES;

  RtlZeroMemory (Buffer,
		 2 * REG_BLOCK_SIZE);

  HiveHeader = (PHIVE_HEADER)Buffer;
  BinHeader = (PHBIN)((ULONG_PTR)Buffer + REG_BLOCK_SIZE);
  RootKeyCell = (PKEY_CELL)((ULONG_PTR)Buffer + REG_BLOCK_SIZE + REG_HBIN_DATA_OFFSET);
  FreeCell = (PCELL_HEADER)((ULONG_PTR)Buffer + REG_BLOCK_SIZE + REG_HBIN_DATA_OFFSET + sizeof(KEY_CELL));

  CmiCreateDefaultHiveHeader(HiveHeader);
  CmiCreateDefaultBinHeader(BinHeader);
  CmiCreateDefaultRootKeyCell(RootKeyCell);

  /* First block */
  BinHeader->BinOffset = 0;

  /* Offset to root key block */
  HiveHeader->RootKeyOffset = REG_HBIN_DATA_OFFSET;

  /* The rest of the block is free */
  FreeCell->CellSize = REG_BLOCK_SIZE - (REG_HBIN_DATA_OFFSET + sizeof(KEY_CELL));

  Status = ZwWriteFile(FileHandle,
		       NULL,
		       NULL,
		       NULL,
		       &IoStatusBlock,
		       Buffer,
		       2 * REG_BLOCK_SIZE,
		       0,
		       NULL);

  ExFreePool(Buffer);

  ASSERTMSG(NT_SUCCESS(Status), ("Status: 0x%X\n", Status));

  if (!NT_SUCCESS(Status))
    {
      return(Status);
    }

  Status = ZwFlushBuffersFile(FileHandle,
			      &IoStatusBlock);

  return(Status);
}


#ifdef HIVE_CHECK
static NTSTATUS
CmiCheckAndFixHive(PREGISTRY_HIVE RegistryHive)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  FILE_STANDARD_INFORMATION fsi;
  IO_STATUS_BLOCK IoStatusBlock;
  HANDLE HiveHandle = INVALID_HANDLE_VALUE;
  HANDLE LogHandle = INVALID_HANDLE_VALUE;
  PHIVE_HEADER HiveHeader = NULL;
  PHIVE_HEADER LogHeader = NULL;
  LARGE_INTEGER FileOffset;
  ULONG FileSize;
  ULONG BufferSize;
  ULONG BitmapSize;
  RTL_BITMAP BlockBitMap;
  NTSTATUS Status;

  DPRINT("CmiCheckAndFixHive() called\n");

  /* Try to open the hive file */
  InitializeObjectAttributes(&ObjectAttributes,
			     &RegistryHive->HiveFileName,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  Status = ZwCreateFile(&HiveHandle,
			FILE_READ_DATA | FILE_READ_ATTRIBUTES,
			&ObjectAttributes,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);
  if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
      return(STATUS_SUCCESS);
    }
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwCreateFile() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Try to open the log file */
  InitializeObjectAttributes(&ObjectAttributes,
			     &RegistryHive->LogFileName,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  Status = ZwCreateFile(&LogHandle,
			FILE_READ_DATA | FILE_READ_ATTRIBUTES,
			&ObjectAttributes,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);
  if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
      LogHandle = INVALID_HANDLE_VALUE;
    }
  else if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwCreateFile() failed (Status %lx)\n", Status);
      ZwClose(HiveHandle);
      return(Status);
    }

  /* Allocate hive header */
  HiveHeader = ExAllocatePool(PagedPool,
			      sizeof(HIVE_HEADER));
  if (HiveHeader == NULL)
    {
      DPRINT("ExAllocatePool() failed\n");
      Status = STATUS_INSUFFICIENT_RESOURCES;
      goto ByeBye;
    }

  /* Read hive base block */
  FileOffset.QuadPart = 0ULL;
  Status = ZwReadFile(HiveHandle,
		      0,
		      0,
		      0,
		      &IoStatusBlock,
		      HiveHeader,
		      sizeof(HIVE_HEADER),
		      &FileOffset,
		      0);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwReadFile() failed (Status %lx)\n", Status);
      goto ByeBye;
    }

  if (LogHandle == INVALID_HANDLE_VALUE)
    {
      if (HiveHeader->Checksum != CmiCalcChecksum((PULONG)HiveHeader) ||
	  HiveHeader->UpdateCounter1 != HiveHeader->UpdateCounter2)
	{
	  /* There is no way to fix the hive without log file - BSOD! */
	  DPRINT("Hive header inconsistent and no log file available!\n");
	  KEBUGCHECK(CONFIG_LIST_FAILED);
	}

      Status = STATUS_SUCCESS;
      goto ByeBye;
    }
  else
    {
      /* Allocate hive header */
      LogHeader = ExAllocatePool(PagedPool,
				 sizeof(HIVE_HEADER));
      if (LogHeader == NULL)
	{
	  DPRINT("ExAllocatePool() failed\n");
	  Status = STATUS_INSUFFICIENT_RESOURCES;
	  goto ByeBye;
	}

      /* Read log file header */
      FileOffset.QuadPart = 0ULL;
      Status = ZwReadFile(LogHandle,
			  0,
			  0,
			  0,
			  &IoStatusBlock,
			  LogHeader,
			  sizeof(HIVE_HEADER),
			  &FileOffset,
			  0);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT("ZwReadFile() failed (Status %lx)\n", Status);
	  goto ByeBye;
	}

      /* Check log file header integrity */
      if (LogHeader->Checksum != CmiCalcChecksum((PULONG)LogHeader) ||
          LogHeader->UpdateCounter1 != LogHeader->UpdateCounter2)
	{
	  if (HiveHeader->Checksum != CmiCalcChecksum((PULONG)HiveHeader) ||
	      HiveHeader->UpdateCounter1 != HiveHeader->UpdateCounter2)
	    {
	      DPRINT("Hive file and log file are inconsistent!\n");
	      KEBUGCHECK(CONFIG_LIST_FAILED);
	    }

	  /* Log file damaged but hive is okay */
	  Status = STATUS_SUCCESS;
	  goto ByeBye;
	}

      if (HiveHeader->UpdateCounter1 == HiveHeader->UpdateCounter2 &&
	  HiveHeader->UpdateCounter1 == LogHeader->UpdateCounter1)
	{
	  /* Hive and log file are up-to-date */
	  Status = STATUS_SUCCESS;
	  goto ByeBye;
	}

      /*
       * Hive needs an update!
       */

      /* Get file size */
      Status = ZwQueryInformationFile(LogHandle,
				      &IoStatusBlock,
				      &fsi,
				      sizeof(fsi),
				      FileStandardInformation);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT("ZwQueryInformationFile() failed (Status %lx)\n", Status);
	  goto ByeBye;
	}
      FileSize = fsi.EndOfFile.u.LowPart;

      /* Calculate bitmap and block size */
      BitmapSize = ROUND_UP((FileSize / REG_BLOCK_SIZE) - 1, sizeof(ULONG) * 8) / 8;
      BufferSize = sizeof(HIVE_HEADER) +
			  sizeof(ULONG) +
			  BitmapSize;
      BufferSize = ROUND_UP(BufferSize, REG_BLOCK_SIZE);

      /* Reallocate log header block */
      ExFreePool(LogHeader);
      LogHeader = ExAllocatePool(PagedPool,
				 BufferSize);
      if (LogHeader == NULL)
	{
	  DPRINT("ExAllocatePool() failed\n");
	  Status = STATUS_INSUFFICIENT_RESOURCES;
	  goto ByeBye;
	}

      /* Read log file header */
      FileOffset.QuadPart = 0ULL;
      Status = ZwReadFile(LogHandle,
			  0,
			  0,
			  0,
			  &IoStatusBlock,
			  LogHeader,
			  BufferSize,
			  &FileOffset,
			  0);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT("ZwReadFile() failed (Status %lx)\n", Status);
	  goto ByeBye;
	}

      /* Initialize bitmap */
      RtlInitializeBitMap(&BlockBitMap,
			  (PVOID)((ULONG_PTR)LogHeader + REG_BLOCK_SIZE + sizeof(ULONG)),
			  BitmapSize * 8);

      /* FIXME: Update dirty blocks */


      /* FIXME: Update hive header */


      Status = STATUS_SUCCESS;
    }


  /* Clean up the mess */
ByeBye:
  if (HiveHeader != NULL)
    ExFreePool(HiveHeader);

  if (LogHeader != NULL)
    ExFreePool(LogHeader);

  if (LogHandle != INVALID_HANDLE_VALUE)
    ZwClose(LogHandle);

  ZwClose(HiveHandle);

  return(Status);
}
#endif


NTSTATUS
CmiImportHiveBins(PREGISTRY_HIVE Hive,
		  PUCHAR ChunkPtr)
{
  BLOCK_OFFSET BlockOffset;
  ULONG BlockIndex;
  PHBIN Bin;
  ULONG j;

  BlockIndex = 0;
  BlockOffset = 0;
  while (BlockIndex < Hive->BlockListSize)
    {
      Bin = (PHBIN)((ULONG_PTR)ChunkPtr + BlockOffset);

      if (Bin->HeaderId != REG_BIN_ID)
	{
	  DPRINT1 ("Bad bin header id %x, offset %x\n", Bin->HeaderId, BlockOffset);
	  return STATUS_REGISTRY_CORRUPT;
	}

      ASSERTMSG((Bin->BinSize % REG_BLOCK_SIZE) == 0,
		("Bin size (0x%.08x) must be multiple of 4K\n",
		Bin->BinSize));

      /* Allocate the hive block */
      Hive->BlockList[BlockIndex].Bin = ExAllocatePool (PagedPool,
							Bin->BinSize);
      if (Hive->BlockList[BlockIndex].Bin == NULL)
	{
	  DPRINT1 ("ExAllocatePool() failed\n");
	  return STATUS_INSUFFICIENT_RESOURCES;
	}
      Hive->BlockList[BlockIndex].Block = (PVOID)Hive->BlockList[BlockIndex].Bin;

      /* Import the Bin */
      RtlCopyMemory (Hive->BlockList[BlockIndex].Bin,
		     Bin,
		     Bin->BinSize);

      if (Bin->BinSize > REG_BLOCK_SIZE)
	{
	  for (j = 1; j < Bin->BinSize / REG_BLOCK_SIZE; j++)
	    {
	      Hive->BlockList[BlockIndex + j].Bin = Hive->BlockList[BlockIndex].Bin;
	      Hive->BlockList[BlockIndex + j].Block =
		(PVOID)((ULONG_PTR)Hive->BlockList[BlockIndex].Bin + (j * REG_BLOCK_SIZE));
	    }
	}

      BlockIndex += Bin->BinSize / REG_BLOCK_SIZE;
      BlockOffset += Bin->BinSize;
    }

  return STATUS_SUCCESS;
}


VOID
CmiFreeHiveBins (PREGISTRY_HIVE Hive)
{
  ULONG i;
  PHBIN Bin;

  Bin = NULL;
  for (i = 0; i < Hive->BlockListSize; i++)
    {
      if (Hive->BlockList[i].Bin == NULL)
	continue;

      if (Hive->BlockList[i].Bin != Bin)
	{
	  Bin = Hive->BlockList[i].Bin;
	  ExFreePool (Hive->BlockList[i].Bin);
	}
      Hive->BlockList[i].Bin = NULL;
      Hive->BlockList[i].Block = NULL;
    }
}


NTSTATUS
CmiCreateHiveFreeCellList(PREGISTRY_HIVE Hive)
{
  BLOCK_OFFSET BlockOffset;
  PCELL_HEADER FreeBlock;
  ULONG BlockIndex;
  ULONG FreeOffset;
  PHBIN Bin;
  NTSTATUS Status;

  /* Initialize the free cell list */
  Hive->FreeListSize = 0;
  Hive->FreeListMax = 0;
  Hive->FreeList = NULL;
  Hive->FreeListOffset = NULL;

  BlockOffset = 0;
  BlockIndex = 0;
  while (BlockIndex < Hive->BlockListSize)
    {
      Bin = Hive->BlockList[BlockIndex].Bin;

      /* Search free blocks and add to list */
      FreeOffset = REG_HBIN_DATA_OFFSET;
      while (FreeOffset < Bin->BinSize)
	{
	  FreeBlock = (PCELL_HEADER) ((ULONG_PTR) Bin + FreeOffset);
	  if (FreeBlock->CellSize > 0)
	    {
	      Status = CmiAddFree(Hive,
				  FreeBlock,
				  Bin->BinOffset + FreeOffset,
				  FALSE);

	      if (!NT_SUCCESS(Status))
		{
		  return Status;
		}

	      FreeOffset += FreeBlock->CellSize;
	    }
	  else
	    {
	      FreeOffset -= FreeBlock->CellSize;
	    }
	}

      BlockIndex += Bin->BinSize / REG_BLOCK_SIZE;
      BlockOffset += Bin->BinSize;
    }

  return STATUS_SUCCESS;
}


VOID
CmiFreeHiveFreeCellList(PREGISTRY_HIVE Hive)
{
  ExFreePool (Hive->FreeList);
  ExFreePool (Hive->FreeListOffset);

  Hive->FreeListSize = 0;
  Hive->FreeListMax = 0;
  Hive->FreeList = NULL;
  Hive->FreeListOffset = NULL;
}


NTSTATUS
CmiCreateHiveBitmap(PREGISTRY_HIVE Hive)
{
  ULONG BitmapSize;

  /* Calculate bitmap size in bytes (always a multiple of 32 bits) */
  BitmapSize = ROUND_UP(Hive->BlockListSize, sizeof(ULONG) * 8) / 8;
  DPRINT("Hive->BlockListSize: %lu\n", Hive->BlockListSize);
  DPRINT("BitmapSize:  %lu Bytes  %lu Bits\n", BitmapSize, BitmapSize * 8);

  /* Allocate bitmap */
  Hive->BitmapBuffer = (PULONG)ExAllocatePool(PagedPool,
					      BitmapSize);
  if (Hive->BitmapBuffer == NULL)
    {
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlInitializeBitMap(&Hive->DirtyBitMap,
		      Hive->BitmapBuffer,
		      BitmapSize * 8);

  /* Initialize bitmap */
  RtlClearAllBits(&Hive->DirtyBitMap);
  Hive->HiveDirty = FALSE;

  return STATUS_SUCCESS;
}


static NTSTATUS
CmiInitNonVolatileRegistryHive (PREGISTRY_HIVE RegistryHive,
				PWSTR Filename)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  ULONG CreateDisposition;
  IO_STATUS_BLOCK IoSB;
  HANDLE FileHandle;
  PSECTION_OBJECT SectionObject;
  PUCHAR ViewBase;
  ULONG ViewSize;
  NTSTATUS Status;

  DPRINT("CmiInitNonVolatileRegistryHive(%p, %S) called\n",
	 RegistryHive, Filename);

  /* Duplicate Filename */
  Status = RtlpCreateUnicodeString(&RegistryHive->HiveFileName,
              Filename, NonPagedPool);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("RtlpCreateUnicodeString() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Create log file name */
  RegistryHive->LogFileName.Length = (wcslen(Filename) + 4) * sizeof(WCHAR);
  RegistryHive->LogFileName.MaximumLength = RegistryHive->LogFileName.Length + sizeof(WCHAR);
  RegistryHive->LogFileName.Buffer = ExAllocatePool(NonPagedPool,
						    RegistryHive->LogFileName.MaximumLength);
  if (RegistryHive->LogFileName.Buffer == NULL)
    {
      RtlFreeUnicodeString(&RegistryHive->HiveFileName);
      DPRINT("ExAllocatePool() failed\n");
      return(STATUS_INSUFFICIENT_RESOURCES);
    }
  wcscpy(RegistryHive->LogFileName.Buffer,
	 Filename);
  wcscat(RegistryHive->LogFileName.Buffer,
	 L".log");

#ifdef HIVE_CHECK
  /* Check and eventually fix a hive */
  Status = CmiCheckAndFixHive(RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      RtlFreeUnicodeString(&RegistryHive->HiveFileName);
      RtlFreeUnicodeString(&RegistryHive->LogFileName);
      DPRINT1("CmiCheckAndFixHive() failed (Status %lx)\n", Status);
      return(Status);
    }
#endif

  InitializeObjectAttributes(&ObjectAttributes,
			     &RegistryHive->HiveFileName,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  CreateDisposition = FILE_OPEN_IF;
  Status = ZwCreateFile(&FileHandle,
			FILE_ALL_ACCESS,
			&ObjectAttributes,
			&IoSB,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			0,
			CreateDisposition,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);
  if (!NT_SUCCESS(Status))
    {
      RtlFreeUnicodeString(&RegistryHive->HiveFileName);
      RtlFreeUnicodeString(&RegistryHive->LogFileName);
      DPRINT("ZwCreateFile() failed (Status %lx)\n", Status);
      return(Status);
    }

  if (IoSB.Information != FILE_OPENED)
    {
      Status = CmiCreateNewRegFile(FileHandle);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT("CmiCreateNewRegFile() failed (Status %lx)\n", Status);
	  ZwClose(FileHandle);
	  RtlFreeUnicodeString(&RegistryHive->HiveFileName);
	  RtlFreeUnicodeString(&RegistryHive->LogFileName);
	  return(Status);
	}
    }

  /* Create the hive section */
  Status = MmCreateSection(&SectionObject,
			   SECTION_ALL_ACCESS,
			   NULL,
			   NULL,
			   PAGE_READWRITE,
			   SEC_COMMIT,
			   FileHandle,
			   NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1("MmCreateSection() failed (Status %lx)\n", Status);
      RtlFreeUnicodeString(&RegistryHive->HiveFileName);
      RtlFreeUnicodeString(&RegistryHive->LogFileName);
      return(Status);
    }

  /* Map the hive file */
  ViewBase = NULL;
  ViewSize = 0;
  Status = MmMapViewOfSection(SectionObject,
			      PsGetCurrentProcess(),
			      (PVOID*)&ViewBase,
			      0,
			      ViewSize,
			      NULL,
			      &ViewSize,
			      0,
			      MEM_COMMIT,
			      PAGE_READWRITE);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1("MmMapViewOfSection() failed (Status %lx)\n", Status);
      ObDereferenceObject(SectionObject);
      RtlFreeUnicodeString(&RegistryHive->HiveFileName);
      RtlFreeUnicodeString(&RegistryHive->LogFileName);
      ZwClose(FileHandle);
      return(Status);
    }
  DPRINT("ViewBase %p  ViewSize %lx\n", ViewBase, ViewSize);

  /* Copy hive header and initalize hive */
  RtlCopyMemory (RegistryHive->HiveHeader,
		 ViewBase,
		 sizeof(HIVE_HEADER));
  RegistryHive->FileSize = ViewSize;
  RegistryHive->BlockListSize = (RegistryHive->FileSize / REG_BLOCK_SIZE) - 1;
  RegistryHive->UpdateCounter = RegistryHive->HiveHeader->UpdateCounter1;

  /* Allocate hive block list */
  RegistryHive->BlockList = ExAllocatePool(NonPagedPool,
					   RegistryHive->BlockListSize * sizeof(BLOCK_LIST_ENTRY));
  if (RegistryHive->BlockList == NULL)
    {
      DPRINT1("Failed to allocate the hive block list\n");
      MmUnmapViewOfSection(PsGetCurrentProcess(),
			   ViewBase);
      ObDereferenceObject(SectionObject);
      RtlFreeUnicodeString(&RegistryHive->HiveFileName);
      RtlFreeUnicodeString(&RegistryHive->LogFileName);
      ZwClose(FileHandle);
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  RtlZeroMemory (RegistryHive->BlockList,
		 RegistryHive->BlockListSize * sizeof(BLOCK_LIST_ENTRY));

  /* Import the hive bins */
  Status = CmiImportHiveBins (RegistryHive,
			      ViewBase + REG_BLOCK_SIZE);
  if (!NT_SUCCESS(Status))
    {
      ExFreePool(RegistryHive->BlockList);
      MmUnmapViewOfSection(PsGetCurrentProcess(),
			   ViewBase);
      ObDereferenceObject(SectionObject);
      RtlFreeUnicodeString(&RegistryHive->HiveFileName);
      RtlFreeUnicodeString(&RegistryHive->LogFileName);
      ZwClose(FileHandle);
      return Status;
    }

  /* Unmap and dereference the hive section */
  MmUnmapViewOfSection(PsGetCurrentProcess(),
                       ViewBase);
  ObDereferenceObject(SectionObject);

  /* Close the hive file */
  ZwClose(FileHandle);

  /* Initialize the free cell list */
  Status = CmiCreateHiveFreeCellList (RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      CmiFreeHiveBins(RegistryHive);
      ExFreePool(RegistryHive->BlockList);
      RtlFreeUnicodeString(&RegistryHive->HiveFileName);
      RtlFreeUnicodeString(&RegistryHive->LogFileName);
      return Status;
    }

  /* Create the block bitmap */
  Status = CmiCreateHiveBitmap (RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      CmiFreeHiveFreeCellList(RegistryHive);
      CmiFreeHiveBins(RegistryHive);
      ExFreePool(RegistryHive->BlockList);
      RtlFreeUnicodeString(&RegistryHive->HiveFileName);
      RtlFreeUnicodeString(&RegistryHive->LogFileName);
      return Status;
    }

  DPRINT("CmiInitNonVolatileRegistryHive(%p, %S) - Finished.\n",
	 RegistryHive, Filename);

  return STATUS_SUCCESS;
}


NTSTATUS
CmiCreateVolatileHive(PREGISTRY_HIVE *RegistryHive)
{
  PKEY_CELL RootKeyCell;
  PREGISTRY_HIVE Hive;

  *RegistryHive = NULL;

  Hive = ExAllocatePool (NonPagedPool,
			 sizeof(REGISTRY_HIVE));
  if (Hive == NULL)
    return STATUS_INSUFFICIENT_RESOURCES;

  RtlZeroMemory (Hive,
		 sizeof(REGISTRY_HIVE));

  DPRINT("Hive %x\n", Hive);

  Hive->HiveHeader = (PHIVE_HEADER)ExAllocatePool (NonPagedPool,
						   sizeof(HIVE_HEADER));
  if (Hive->HiveHeader == NULL)
    {
      ExFreePool (Hive);
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  RtlZeroMemory (Hive->HiveHeader,
		 sizeof(HIVE_HEADER));

  Hive->Flags = (HIVE_NO_FILE | HIVE_POINTER);

  CmiCreateDefaultHiveHeader (Hive->HiveHeader);

  RootKeyCell = (PKEY_CELL)ExAllocatePool (NonPagedPool,
					   sizeof(KEY_CELL));
  if (RootKeyCell == NULL)
    {
      ExFreePool(Hive->HiveHeader);
      ExFreePool(Hive);
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  CmiCreateDefaultRootKeyCell (RootKeyCell);
  Hive->HiveHeader->RootKeyOffset = (BLOCK_OFFSET)RootKeyCell;

  /* Acquire hive list lock exclusively */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite (&CmiRegistryLock, TRUE);

  /* Add the new hive to the hive list */
  InsertTailList (&CmiHiveListHead,
		  &Hive->HiveList);

  /* Release hive list lock */
  ExReleaseResourceLite (&CmiRegistryLock);
  KeLeaveCriticalRegion();

  VERIFY_REGISTRY_HIVE (Hive);

  *RegistryHive = Hive;

  return STATUS_SUCCESS;
}


NTSTATUS
CmiCreateTempHive(PREGISTRY_HIVE *RegistryHive)
{
  PHBIN BinHeader;
  PCELL_HEADER FreeCell;
  PREGISTRY_HIVE Hive;
  NTSTATUS Status;

  DPRINT ("CmiCreateTempHive() called\n");

  *RegistryHive = NULL;

  Hive = ExAllocatePool (NonPagedPool,
			 sizeof(REGISTRY_HIVE));
  if (Hive == NULL)
    {
      DPRINT1 ("Failed to allocate registry hive block\n");
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  RtlZeroMemory (Hive,
		 sizeof(REGISTRY_HIVE));

  DPRINT ("Hive %x\n", Hive);

  Hive->HiveHeader = (PHIVE_HEADER)ExAllocatePool (NonPagedPool,
						   REG_BLOCK_SIZE);
  if (Hive->HiveHeader == NULL)
    {
      DPRINT1 ("Failed to allocate hive header block\n");
      ExFreePool (Hive);
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  RtlZeroMemory (Hive->HiveHeader,
		 REG_BLOCK_SIZE);

  DPRINT ("HiveHeader %x\n", Hive->HiveHeader);

  Hive->Flags = HIVE_NO_FILE;

  RtlInitUnicodeString (&Hive->HiveFileName,
			NULL);
  RtlInitUnicodeString (&Hive->LogFileName,
			NULL);

  CmiCreateDefaultHiveHeader (Hive->HiveHeader);

  /* Allocate hive block list */
  Hive->BlockList = ExAllocatePool (NonPagedPool,
				    sizeof(PBLOCK_LIST_ENTRY));
  if (Hive->BlockList == NULL)
    {
      DPRINT1 ("Failed to allocate hive block list\n");
      ExFreePool(Hive->HiveHeader);
      ExFreePool(Hive);
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  /* Allocate first Bin */
  Hive->BlockList[0].Bin = ExAllocatePool (NonPagedPool,
					   REG_BLOCK_SIZE);
  if (Hive->BlockList[0].Bin == NULL)
    {
      DPRINT1 ("Failed to allocate first bin\n");
      ExFreePool(Hive->BlockList);
      ExFreePool(Hive->HiveHeader);
      ExFreePool(Hive);
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  Hive->BlockList[0].Block = (PVOID)Hive->BlockList[0].Bin;

  Hive->FileSize = 2* REG_BLOCK_SIZE;
  Hive->BlockListSize = 1;
  Hive->UpdateCounter = Hive->HiveHeader->UpdateCounter1;


  BinHeader = Hive->BlockList[0].Bin;
  FreeCell = (PCELL_HEADER)((ULONG_PTR)BinHeader + REG_HBIN_DATA_OFFSET);

  CmiCreateDefaultBinHeader (BinHeader);

  /* First block */
  BinHeader->BinOffset = 0;

  /* Offset to root key block */
  Hive->HiveHeader->RootKeyOffset = (BLOCK_OFFSET)-1;

  /* The rest of the block is free */
  FreeCell->CellSize = REG_BLOCK_SIZE - REG_HBIN_DATA_OFFSET;

  /* Create the free cell list */
  Status = CmiCreateHiveFreeCellList (Hive);
  if (Hive->BlockList[0].Bin == NULL)
    {
      DPRINT1 ("CmiCreateHiveFreeCellList() failed (Status %lx)\n", Status);
      ExFreePool(Hive->BlockList[0].Bin);
      ExFreePool(Hive->BlockList);
      ExFreePool(Hive->HiveHeader);
      ExFreePool(Hive);
      return Status;
    }

  /* Acquire hive list lock exclusively */
  KeEnterCriticalRegion();
  ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);

  /* Add the new hive to the hive list */
  InsertTailList (&CmiHiveListHead,
		  &Hive->HiveList);

  /* Release hive list lock */
  ExReleaseResourceLite(&CmiRegistryLock);
  KeLeaveCriticalRegion();

  VERIFY_REGISTRY_HIVE (Hive);

  *RegistryHive = Hive;

  return STATUS_SUCCESS;
}


NTSTATUS
CmiLoadHive(IN POBJECT_ATTRIBUTES KeyObjectAttributes,
	    IN PUNICODE_STRING FileName,
	    IN ULONG Flags)
{
  PREGISTRY_HIVE Hive;
  NTSTATUS Status;

  DPRINT ("CmiLoadHive(Filename %wZ)\n", FileName);

  if (Flags & ~REG_NO_LAZY_FLUSH)
    return STATUS_INVALID_PARAMETER;

  Hive = ExAllocatePool (NonPagedPool,
			 sizeof(REGISTRY_HIVE));
  if (Hive == NULL)
    {
      DPRINT1 ("Failed to allocate hive header.\n");
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  RtlZeroMemory (Hive,
		 sizeof(REGISTRY_HIVE));

  DPRINT ("Hive %x\n", Hive);
  Hive->Flags = (Flags & REG_NO_LAZY_FLUSH) ? HIVE_NO_SYNCH : 0;

  Hive->HiveHeader = (PHIVE_HEADER)ExAllocatePool(NonPagedPool,
						  sizeof(HIVE_HEADER));
  if (Hive->HiveHeader == NULL)
    {
      DPRINT1 ("Failed to allocate hive header.\n");
      ExFreePool (Hive);
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  RtlZeroMemory (Hive->HiveHeader,
		 sizeof(HIVE_HEADER));

  Status = CmiInitNonVolatileRegistryHive (Hive,
					   FileName->Buffer);
  if (!NT_SUCCESS (Status))
    {
      DPRINT1 ("CmiInitNonVolatileRegistryHive() failed (Status %lx)\n", Status);
      ExFreePool (Hive->HiveHeader);
      ExFreePool (Hive);
      return Status;
    }

  /* Add the new hive to the hive list */
  InsertTailList (&CmiHiveListHead,
		  &Hive->HiveList);

  VERIFY_REGISTRY_HIVE(Hive);

  Status = CmiConnectHive (KeyObjectAttributes,
			   Hive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("CmiConnectHive() failed (Status %lx)\n", Status);
//      CmiRemoveRegistryHive (Hive);
    }

  DPRINT ("CmiLoadHive() done\n");

  return Status;
}


NTSTATUS
CmiRemoveRegistryHive(PREGISTRY_HIVE RegistryHive)
{
  if (RegistryHive->Flags & HIVE_POINTER)
    return STATUS_UNSUCCESSFUL;

  /* Remove hive from hive list */
  RemoveEntryList (&RegistryHive->HiveList);

  /* Release file names */
  RtlFreeUnicodeString (&RegistryHive->HiveFileName);
  RtlFreeUnicodeString (&RegistryHive->LogFileName);

  /* Release hive bitmap */
  ExFreePool (RegistryHive->BitmapBuffer);

  /* Release free cell list */
  ExFreePool (RegistryHive->FreeList);
  ExFreePool (RegistryHive->FreeListOffset);

  /* Release bins and bin list */
  CmiFreeHiveBins (RegistryHive);
  ExFreePool (RegistryHive->BlockList);

  /* Release hive header */
  ExFreePool (RegistryHive->HiveHeader);

  /* Release hive */
  ExFreePool (RegistryHive);

  return STATUS_SUCCESS;
}


static ULONG
CmiCalcChecksum(PULONG Buffer)
{
  ULONG Sum = 0;
  ULONG i;

  for (i = 0; i < 127; i++)
    Sum += Buffer[i];

  return(Sum);
}


static NTSTATUS
CmiStartLogUpdate(PREGISTRY_HIVE RegistryHive)
{
  FILE_END_OF_FILE_INFORMATION EndOfFileInfo;
  FILE_ALLOCATION_INFORMATION FileAllocationInfo;
  OBJECT_ATTRIBUTES ObjectAttributes;
  IO_STATUS_BLOCK IoStatusBlock;
  HANDLE FileHandle;
  LARGE_INTEGER FileOffset;
  ULONG BufferSize;
  ULONG BitmapSize;
  PUCHAR Buffer;
  PUCHAR Ptr;
  ULONG BlockIndex;
  ULONG LastIndex;
  PVOID BlockPtr;
  NTSTATUS Status;

  DPRINT("CmiStartLogUpdate() called\n");

  BitmapSize = ROUND_UP(RegistryHive->BlockListSize, sizeof(ULONG) * 8) / 8;
  BufferSize = sizeof(HIVE_HEADER) +
	       sizeof(ULONG) +
	       BitmapSize;
  BufferSize = ROUND_UP(BufferSize, REG_BLOCK_SIZE);

  DPRINT("Bitmap size %lu  buffer size: %lu\n", BitmapSize, BufferSize);

  Buffer = (PUCHAR)ExAllocatePool(NonPagedPool,
				  BufferSize);
  if (Buffer == NULL)
    {
      DPRINT("ExAllocatePool() failed\n");
      return(STATUS_INSUFFICIENT_RESOURCES);
    }
  RtlZeroMemory (Buffer,
		 BufferSize);

  /* Open log file for writing */
  InitializeObjectAttributes(&ObjectAttributes,
			     &RegistryHive->LogFileName,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  Status = ZwCreateFile(&FileHandle,
			FILE_ALL_ACCESS,
			&ObjectAttributes,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_SUPERSEDE,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwCreateFile() failed (Status %lx)\n", Status);
      ExFreePool(Buffer);
      return(Status);
    }

  /* Update firt update counter and checksum */
  RegistryHive->HiveHeader->UpdateCounter1 = RegistryHive->UpdateCounter + 1;
  RegistryHive->HiveHeader->Checksum = CmiCalcChecksum((PULONG)RegistryHive->HiveHeader);

  /* Copy hive header */
  RtlCopyMemory(Buffer,
		RegistryHive->HiveHeader,
		sizeof(HIVE_HEADER));
  Ptr = Buffer + sizeof(HIVE_HEADER);

  RtlCopyMemory(Ptr,
		"DIRT",
		4);
  Ptr += 4;
  RtlCopyMemory(Ptr,
		RegistryHive->DirtyBitMap.Buffer,
		BitmapSize);

  /* Write hive block and block bitmap */
  FileOffset.QuadPart = (ULONGLONG)0;
  Status = ZwWriteFile(FileHandle,
		       NULL,
		       NULL,
		       NULL,
		       &IoStatusBlock,
		       Buffer,
		       BufferSize,
		       &FileOffset,
		       NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwWriteFile() failed (Status %lx)\n", Status);
      ZwClose(FileHandle);
      ExFreePool(Buffer);
      return(Status);
    }
  ExFreePool(Buffer);

  /* Write dirty blocks */
  FileOffset.QuadPart = (ULONGLONG)BufferSize;
  BlockIndex = 0;
  while (BlockIndex < RegistryHive->BlockListSize)
    {
      LastIndex = BlockIndex;
      BlockIndex = RtlFindSetBits(&RegistryHive->DirtyBitMap,
				  1,
				  BlockIndex);
      if (BlockIndex == (ULONG)-1 || BlockIndex < LastIndex)
	{
	  DPRINT("No more set bits\n");
	  Status = STATUS_SUCCESS;
	  break;
	}

      DPRINT("Block %lu is dirty\n", BlockIndex);

      BlockPtr = RegistryHive->BlockList[BlockIndex].Block;
      DPRINT("BlockPtr %p\n", BlockPtr);
      DPRINT("File offset %I64x\n", FileOffset.QuadPart);

      /* Write hive block */
      Status = ZwWriteFile(FileHandle,
			   NULL,
			   NULL,
			   NULL,
			   &IoStatusBlock,
			   BlockPtr,
			   REG_BLOCK_SIZE,
			   &FileOffset,
			   NULL);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT1("ZwWriteFile() failed (Status %lx)\n", Status);
	  ZwClose(FileHandle);
	  return(Status);
	}

      BlockIndex++;
      FileOffset.QuadPart += (ULONGLONG)REG_BLOCK_SIZE;
    }

  /* Truncate log file */
  EndOfFileInfo.EndOfFile.QuadPart = FileOffset.QuadPart;
  Status = ZwSetInformationFile(FileHandle,
				&IoStatusBlock,
				&EndOfFileInfo,
				sizeof(FILE_END_OF_FILE_INFORMATION),
				FileEndOfFileInformation);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwSetInformationFile() failed (Status %lx)\n", Status);
      ZwClose(FileHandle);
      return(Status);
    }

  FileAllocationInfo.AllocationSize.QuadPart = FileOffset.QuadPart;
  Status = ZwSetInformationFile(FileHandle,
				&IoStatusBlock,
				&FileAllocationInfo,
				sizeof(FILE_ALLOCATION_INFORMATION),
				FileAllocationInformation);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwSetInformationFile() failed (Status %lx)\n", Status);
      ZwClose(FileHandle);
      return(Status);
    }

  /* Flush the log file */
  Status = ZwFlushBuffersFile(FileHandle,
			      &IoStatusBlock);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwFlushBuffersFile() failed (Status %lx)\n", Status);
    }

  ZwClose(FileHandle);

  return(Status);
}


static NTSTATUS
CmiFinishLogUpdate(PREGISTRY_HIVE RegistryHive)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  IO_STATUS_BLOCK IoStatusBlock;
  HANDLE FileHandle;
  LARGE_INTEGER FileOffset;
  ULONG BufferSize;
  ULONG BitmapSize;
  PUCHAR Buffer;
  PUCHAR Ptr;
  NTSTATUS Status;

  DPRINT("CmiFinishLogUpdate() called\n");

  BitmapSize = ROUND_UP(RegistryHive->BlockListSize, sizeof(ULONG) * 8) / 8;
  BufferSize = sizeof(HIVE_HEADER) +
	       sizeof(ULONG) +
	       BitmapSize;
  BufferSize = ROUND_UP(BufferSize, REG_BLOCK_SIZE);

  DPRINT("Bitmap size %lu  buffer size: %lu\n", BitmapSize, BufferSize);

  Buffer = (PUCHAR)ExAllocatePool(NonPagedPool, BufferSize);
  if (Buffer == NULL)
    {
      DPRINT("ExAllocatePool() failed\n");
      return(STATUS_INSUFFICIENT_RESOURCES);
    }

  /* Open log file for writing */
  InitializeObjectAttributes(&ObjectAttributes,
			     &RegistryHive->LogFileName,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  Status = ZwCreateFile(&FileHandle,
			FILE_ALL_ACCESS,
			&ObjectAttributes,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwCreateFile() failed (Status %lx)\n", Status);
      ExFreePool(Buffer);
      return(Status);
    }

  /* Update first and second update counter and checksum */
  RegistryHive->HiveHeader->UpdateCounter1 = RegistryHive->UpdateCounter + 1;
  RegistryHive->HiveHeader->UpdateCounter2 = RegistryHive->UpdateCounter + 1;
  RegistryHive->HiveHeader->Checksum = CmiCalcChecksum((PULONG)RegistryHive->HiveHeader);

  /* Copy hive header */
  RtlCopyMemory(Buffer,
		RegistryHive->HiveHeader,
		sizeof(HIVE_HEADER));
  Ptr = Buffer + sizeof(HIVE_HEADER);

  /* Write empty block bitmap */
  RtlCopyMemory(Ptr,
		"DIRT",
		4);
  Ptr += 4;
  RtlZeroMemory(Ptr,
		BitmapSize);

  /* Write hive block and block bitmap */
  FileOffset.QuadPart = (ULONGLONG)0;
  Status = ZwWriteFile(FileHandle,
		       NULL,
		       NULL,
		       NULL,
		       &IoStatusBlock,
		       Buffer,
		       BufferSize,
		       &FileOffset,
		       NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwWriteFile() failed (Status %lx)\n", Status);
      ZwClose(FileHandle);
      ExFreePool(Buffer);
      return(Status);
    }

  ExFreePool(Buffer);

  /* Flush the log file */
  Status = ZwFlushBuffersFile(FileHandle,
			      &IoStatusBlock);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwFlushBuffersFile() failed (Status %lx)\n", Status);
    }

  ZwClose(FileHandle);

  return(Status);
}


static NTSTATUS
CmiCleanupLogUpdate(PREGISTRY_HIVE RegistryHive)
{
  FILE_END_OF_FILE_INFORMATION EndOfFileInfo;
  FILE_ALLOCATION_INFORMATION FileAllocationInfo;
  OBJECT_ATTRIBUTES ObjectAttributes;
  IO_STATUS_BLOCK IoStatusBlock;
  HANDLE FileHandle;
  ULONG BufferSize;
  ULONG BitmapSize;
  NTSTATUS Status;

  DPRINT("CmiCleanupLogUpdate() called\n");

  BitmapSize = ROUND_UP(RegistryHive->BlockListSize, sizeof(ULONG) * 8) / 8;
  BufferSize = sizeof(HIVE_HEADER) +
	       sizeof(ULONG) +
	       BitmapSize;
  BufferSize = ROUND_UP(BufferSize, REG_BLOCK_SIZE);

  DPRINT("Bitmap size %lu  buffer size: %lu\n", BitmapSize, BufferSize);

  /* Open log file for writing */
  InitializeObjectAttributes(&ObjectAttributes,
			     &RegistryHive->LogFileName,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  Status = ZwCreateFile(&FileHandle,
			FILE_ALL_ACCESS,
			&ObjectAttributes,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwCreateFile() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Truncate log file */
  EndOfFileInfo.EndOfFile.QuadPart = (ULONGLONG)BufferSize;
  Status = ZwSetInformationFile(FileHandle,
				&IoStatusBlock,
				&EndOfFileInfo,
				sizeof(FILE_END_OF_FILE_INFORMATION),
				FileEndOfFileInformation);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwSetInformationFile() failed (Status %lx)\n", Status);
      ZwClose(FileHandle);
      return(Status);
    }

  FileAllocationInfo.AllocationSize.QuadPart = (ULONGLONG)BufferSize;
  Status = ZwSetInformationFile(FileHandle,
				&IoStatusBlock,
				&FileAllocationInfo,
				sizeof(FILE_ALLOCATION_INFORMATION),
				FileAllocationInformation);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("NtSetInformationFile() failed (Status %lx)\n", Status);
      ZwClose(FileHandle);
      return(Status);
    }

  /* Flush the log file */
  Status = ZwFlushBuffersFile(FileHandle,
			      &IoStatusBlock);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwFlushBuffersFile() failed (Status %lx)\n", Status);
    }

  ZwClose(FileHandle);

  return(Status);
}


static NTSTATUS
CmiStartHiveUpdate(PREGISTRY_HIVE RegistryHive)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  IO_STATUS_BLOCK IoStatusBlock;
  HANDLE FileHandle;
  LARGE_INTEGER FileOffset;
  ULONG BlockIndex;
  ULONG LastIndex;
  PVOID BlockPtr;
  NTSTATUS Status;

  DPRINT("CmiStartHiveUpdate() called\n");

  /* Open hive for writing */
  InitializeObjectAttributes(&ObjectAttributes,
			     &RegistryHive->HiveFileName,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  Status = ZwCreateFile(&FileHandle,
			FILE_ALL_ACCESS,
			&ObjectAttributes,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwCreateFile() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Update firt update counter and checksum */
  RegistryHive->HiveHeader->UpdateCounter1 = RegistryHive->UpdateCounter + 1;
  RegistryHive->HiveHeader->Checksum = CmiCalcChecksum((PULONG)RegistryHive->HiveHeader);

  /* Write hive block */
  FileOffset.QuadPart = (ULONGLONG)0;
  Status = ZwWriteFile(FileHandle,
		       NULL,
		       NULL,
		       NULL,
		       &IoStatusBlock,
		       RegistryHive->HiveHeader,
		       sizeof(HIVE_HEADER),
		       &FileOffset,
		       NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwWriteFile() failed (Status %lx)\n", Status);
      ZwClose(FileHandle);
      return(Status);
    }

  BlockIndex = 0;
  while (BlockIndex < RegistryHive->BlockListSize)
    {
      LastIndex = BlockIndex;
      BlockIndex = RtlFindSetBits(&RegistryHive->DirtyBitMap,
				  1,
				  BlockIndex);
      if (BlockIndex == (ULONG)-1 || BlockIndex < LastIndex)
	{
	  DPRINT("No more set bits\n");
	  Status = STATUS_SUCCESS;
	  break;
	}

      DPRINT("Block %lu is dirty\n", BlockIndex);

      BlockPtr = RegistryHive->BlockList[BlockIndex].Block;
      DPRINT("  BlockPtr %p\n", BlockPtr);

      FileOffset.QuadPart = (ULONGLONG)(BlockIndex + 1) * (ULONGLONG)REG_BLOCK_SIZE;
      DPRINT("  File offset %I64x\n", FileOffset.QuadPart);

      /* Write hive block */
      Status = ZwWriteFile(FileHandle,
			   NULL,
			   NULL,
			   NULL,
			   &IoStatusBlock,
			   BlockPtr,
			   REG_BLOCK_SIZE,
			   &FileOffset,
			   NULL);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT("ZwWriteFile() failed (Status %lx)\n", Status);
	  ZwClose(FileHandle);
	  return(Status);
	}

      BlockIndex++;
    }

  Status = ZwFlushBuffersFile(FileHandle,
			      &IoStatusBlock);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwFlushBuffersFile() failed (Status %lx)\n", Status);
    }

  ZwClose(FileHandle);

  return(Status);
}


static NTSTATUS
CmiFinishHiveUpdate(PREGISTRY_HIVE RegistryHive)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  IO_STATUS_BLOCK IoStatusBlock;
  LARGE_INTEGER FileOffset;
  HANDLE FileHandle;
  NTSTATUS Status;

  DPRINT("CmiFinishHiveUpdate() called\n");

  InitializeObjectAttributes(&ObjectAttributes,
			     &RegistryHive->HiveFileName,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  Status = ZwCreateFile(&FileHandle,
			FILE_ALL_ACCESS,
			&ObjectAttributes,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			0,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwCreateFile() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Update second update counter and checksum */
  RegistryHive->HiveHeader->UpdateCounter1 = RegistryHive->UpdateCounter + 1;
  RegistryHive->HiveHeader->UpdateCounter2 = RegistryHive->UpdateCounter + 1;
  RegistryHive->HiveHeader->Checksum = CmiCalcChecksum((PULONG)RegistryHive->HiveHeader);

  /* Write hive block */
  FileOffset.QuadPart = (ULONGLONG)0;
  Status = ZwWriteFile(FileHandle,
		       NULL,
		       NULL,
		       NULL,
		       &IoStatusBlock,
		       RegistryHive->HiveHeader,
		       sizeof(HIVE_HEADER),
		       &FileOffset,
		       NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwWriteFile() failed (Status %lx)\n", Status);
      ZwClose(FileHandle);
      return(Status);
    }

  Status = ZwFlushBuffersFile(FileHandle,
			      &IoStatusBlock);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("ZwFlushBuffersFile() failed (Status %lx)\n", Status);
    }

  ZwClose(FileHandle);

  return(Status);
}


NTSTATUS
CmiFlushRegistryHive(PREGISTRY_HIVE RegistryHive)
{
  NTSTATUS Status;

  DPRINT("CmiFlushRegistryHive() called\n");

  if (RegistryHive->HiveDirty == FALSE)
    {
      return(STATUS_SUCCESS);
    }

  DPRINT("Hive '%wZ' is dirty\n",
	 &RegistryHive->HiveFileName);
  DPRINT("Log file: '%wZ'\n",
	 &RegistryHive->LogFileName);

  /* Update hive header modification time */
  KeQuerySystemTime(&RegistryHive->HiveHeader->DateModified);

  /* Start log update */
  Status = CmiStartLogUpdate(RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("CmiStartLogUpdate() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Finish log update */
  Status = CmiFinishLogUpdate(RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("CmiFinishLogUpdate() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Start hive update */
  Status = CmiStartHiveUpdate(RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("CmiStartHiveUpdate() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Finish the hive update */
  Status = CmiFinishHiveUpdate(RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("CmiFinishHiveUpdate() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Cleanup log update */
  Status = CmiCleanupLogUpdate(RegistryHive);
  if (!NT_SUCCESS(Status))
    {
      DPRINT("CmiFinishLogUpdate() failed (Status %lx)\n", Status);
      return(Status);
    }

  /* Increment hive update counter */
  RegistryHive->UpdateCounter++;

  /* Clear dirty bitmap and dirty flag */
  RtlClearAllBits(&RegistryHive->DirtyBitMap);
  RegistryHive->HiveDirty = FALSE;

  DPRINT("CmiFlushRegistryHive() done\n");

  return(STATUS_SUCCESS);
}


ULONG
CmiGetNumberOfSubKeys(PKEY_OBJECT KeyObject)
{
  PKEY_OBJECT CurKey;
  PKEY_CELL KeyCell;
  ULONG SubKeyCount;
  ULONG i;

  VERIFY_KEY_OBJECT(KeyObject);

  KeyCell = KeyObject->KeyCell;
  VERIFY_KEY_CELL(KeyCell);

  SubKeyCount = (KeyCell == NULL) ? 0 : KeyCell->NumberOfSubKeys;

  /* Search for volatile or 'foreign' keys */
  for (i = 0; i < KeyObject->NumberOfSubKeys; i++)
    {
      CurKey = KeyObject->SubKeys[i];
      if (CurKey->RegistryHive == CmiVolatileHive ||
	  CurKey->RegistryHive != KeyObject->RegistryHive)
	{
	  SubKeyCount++;
	}
    }

  return SubKeyCount;
}


ULONG
CmiGetMaxNameLength(PKEY_OBJECT KeyObject)
{
  PHASH_TABLE_CELL HashBlock;
  PKEY_OBJECT CurKey;
  PKEY_CELL CurSubKeyCell;
  PKEY_CELL KeyCell;
  ULONG MaxName;
  ULONG NameSize;
  ULONG i;

  VERIFY_KEY_OBJECT(KeyObject);

  KeyCell = KeyObject->KeyCell;
  VERIFY_KEY_CELL(KeyCell);

  MaxName = 0;
  HashBlock = CmiGetCell (KeyObject->RegistryHive,
			  KeyCell->HashTableOffset,
			  NULL);
  if (HashBlock == NULL)
    {
      DPRINT("CmiGetBlock() failed\n");
    }
  else
    {
      for (i = 0; i < HashBlock->HashTableSize; i++)
	{
	  if (HashBlock->Table[i].KeyOffset != 0)
	    {
	      CurSubKeyCell = CmiGetCell (KeyObject->RegistryHive,
					  HashBlock->Table[i].KeyOffset,
					   NULL);
	      if (CurSubKeyCell == NULL)
	        {
	          DPRINT("CmiGetBlock() failed\n");
	          continue;
	        }

	      NameSize = CurSubKeyCell->NameSize;
	      if (CurSubKeyCell->Flags & REG_KEY_NAME_PACKED)
	        {
	          NameSize *= sizeof(WCHAR);
	        }

	      if (NameSize > MaxName)
	        {
	          MaxName = NameSize;
	        }
	    }
	}
    }

  DPRINT ("KeyObject->NumberOfSubKeys %d\n", KeyObject->NumberOfSubKeys);
  for (i = 0; i < KeyObject->NumberOfSubKeys; i++)
    {
      CurKey = KeyObject->SubKeys[i];
      if (CurKey->RegistryHive == CmiVolatileHive ||
	  CurKey->RegistryHive != KeyObject->RegistryHive)
	{
	  CurSubKeyCell = CurKey->KeyCell;
	  if (CurSubKeyCell == NULL)
	    {
	      DPRINT("CmiGetBlock() failed\n");
	      continue;
	    }

	  if ((CurSubKeyCell->Flags & REG_KEY_ROOT_CELL) == REG_KEY_ROOT_CELL)
	    {
	      /* Use name of the key object */
	      NameSize = CurKey->Name.Length;
	    }
	  else
	    {
	      /* Use name of the key cell */
	      NameSize = CurSubKeyCell->NameSize;
	      if (CurSubKeyCell->Flags & REG_KEY_NAME_PACKED)
		{
		  NameSize *= sizeof(WCHAR);
		}
	    }
	  DPRINT ("NameSize %lu\n", NameSize);

	  if (NameSize > MaxName)
	    {
	      MaxName = NameSize;
	    }
	}
    }

  DPRINT ("MaxName %lu\n", MaxName);

  return MaxName;
}


ULONG
CmiGetMaxClassLength(PKEY_OBJECT  KeyObject)
{
  PHASH_TABLE_CELL HashBlock;
  PKEY_OBJECT CurKey;
  PKEY_CELL CurSubKeyCell;
  PKEY_CELL KeyCell;
  ULONG MaxClass;
  ULONG i;

  VERIFY_KEY_OBJECT(KeyObject);

  KeyCell = KeyObject->KeyCell;
  VERIFY_KEY_CELL(KeyCell);

  MaxClass = 0;
  HashBlock = CmiGetCell (KeyObject->RegistryHive,
			  KeyCell->HashTableOffset,
			  NULL);
  if (HashBlock == NULL)
    {
      DPRINT("CmiGetBlock() failed\n");
    }
  else
    {
      for (i = 0; i < HashBlock->HashTableSize; i++)
	{
	  if (HashBlock->Table[i].KeyOffset != 0)
	    {
	      CurSubKeyCell = CmiGetCell (KeyObject->RegistryHive,
					  HashBlock->Table[i].KeyOffset,
					  NULL);
	      if (CurSubKeyCell == NULL)
	        {
	          DPRINT("CmiGetBlock() failed\n");
	          continue;
	        }

	      if (MaxClass < CurSubKeyCell->ClassSize)
	        {
	          MaxClass = CurSubKeyCell->ClassSize;
	        }
	    }
	}
    }

  DPRINT("KeyObject->NumberOfSubKeys %d\n", KeyObject->NumberOfSubKeys);
  for (i = 0; i < KeyObject->NumberOfSubKeys; i++)
    {
      CurKey = KeyObject->SubKeys[i];
      if (CurKey->RegistryHive == CmiVolatileHive ||
	  CurKey->RegistryHive != KeyObject->RegistryHive)
	{
	  CurSubKeyCell = CurKey->KeyCell;
	  if (CurSubKeyCell == NULL)
	    {
	      DPRINT("CmiGetBlock() failed\n");
	      continue;
	    }

	  if (MaxClass < CurSubKeyCell->ClassSize)
	    {
	      MaxClass = CurSubKeyCell->ClassSize;
	    }
	}
    }

  return MaxClass;
}


ULONG
CmiGetMaxValueNameLength(PREGISTRY_HIVE RegistryHive,
			 PKEY_CELL KeyCell)
{
  PVALUE_LIST_CELL ValueListCell;
  PVALUE_CELL CurValueCell;
  ULONG MaxValueName;
  ULONG Size;
  ULONG i;

  VERIFY_KEY_CELL(KeyCell);

  MaxValueName = 0;
  ValueListCell = CmiGetCell (RegistryHive,
			      KeyCell->ValueListOffset,
			      NULL);
  if (ValueListCell == NULL)
    {
      DPRINT("CmiGetBlock() failed\n");
      return 0;
    }

  for (i = 0; i < KeyCell->NumberOfValues; i++)
    {
      CurValueCell = CmiGetCell (RegistryHive,
				 ValueListCell->ValueOffset[i],
				 NULL);
      if (CurValueCell == NULL)
	{
	  DPRINT("CmiGetBlock() failed\n");
	}

      if (CurValueCell != NULL)
	{
	  Size = CurValueCell->NameSize;
	  if (CurValueCell->Flags & REG_VALUE_NAME_PACKED)
	    {
	      Size *= sizeof(WCHAR);
	    }
	  if (MaxValueName < Size)
	    {
	      MaxValueName = Size;
	    }
        }
    }

  return MaxValueName;
}


ULONG
CmiGetMaxValueDataLength(PREGISTRY_HIVE RegistryHive,
			 PKEY_CELL KeyCell)
{
  PVALUE_LIST_CELL ValueListCell;
  PVALUE_CELL CurValueCell;
  LONG MaxValueData;
  ULONG i;

  VERIFY_KEY_CELL(KeyCell);

  MaxValueData = 0;
  ValueListCell = CmiGetCell (RegistryHive, KeyCell->ValueListOffset, NULL);
  if (ValueListCell == NULL)
    {
      return 0;
    }

  for (i = 0; i < KeyCell->NumberOfValues; i++)
    {
      CurValueCell = CmiGetCell (RegistryHive,
                                 ValueListCell->ValueOffset[i],NULL);
      if ((CurValueCell != NULL) &&
          (MaxValueData < (LONG)(CurValueCell->DataSize & REG_DATA_SIZE_MASK)))
        {
          MaxValueData = CurValueCell->DataSize & REG_DATA_SIZE_MASK;
        }
    }

  return MaxValueData;
}


NTSTATUS
CmiScanForSubKey(IN PREGISTRY_HIVE RegistryHive,
		 IN PKEY_CELL KeyCell,
		 OUT PKEY_CELL *SubKeyCell,
		 OUT BLOCK_OFFSET *BlockOffset,
		 IN PUNICODE_STRING KeyName,
		 IN ACCESS_MASK DesiredAccess,
		 IN ULONG Attributes)
{
  PHASH_TABLE_CELL HashBlock;
  PKEY_CELL CurSubKeyCell;
  ULONG i;

  VERIFY_KEY_CELL(KeyCell);

  DPRINT("Scanning for sub key %wZ\n", KeyName);

  ASSERT(RegistryHive);

  *SubKeyCell = NULL;

  /* The key does not have any subkeys */
  if (KeyCell->HashTableOffset == (BLOCK_OFFSET)-1)
    {
      return STATUS_SUCCESS;
    }

  /* Get hash table */
  HashBlock = CmiGetCell (RegistryHive, KeyCell->HashTableOffset, NULL);
  if (HashBlock == NULL)
    {
      DPRINT("CmiGetBlock() failed\n");
      return STATUS_UNSUCCESSFUL;
    }

  for (i = 0; (i < KeyCell->NumberOfSubKeys) && (i < HashBlock->HashTableSize); i++)
    {
      if (Attributes & OBJ_CASE_INSENSITIVE)
	{
	  if (HashBlock->Table[i].KeyOffset != 0 &&
	      HashBlock->Table[i].KeyOffset != (ULONG_PTR)-1 &&
	      (HashBlock->Table[i].HashValue == 0 ||
	       CmiCompareHashI(KeyName, (PCHAR)&HashBlock->Table[i].HashValue)))
	    {
	      CurSubKeyCell = CmiGetCell (RegistryHive,
					  HashBlock->Table[i].KeyOffset,
					  NULL);
	      if (CurSubKeyCell == NULL)
		{
		  DPRINT("CmiGetBlock() failed\n");
		  return STATUS_UNSUCCESSFUL;
		}

	      if (CmiCompareKeyNamesI(KeyName, CurSubKeyCell))
		{
		  *SubKeyCell = CurSubKeyCell;
		  *BlockOffset = HashBlock->Table[i].KeyOffset;
		  break;
		}
	    }
	}
      else
	{
	  if (HashBlock->Table[i].KeyOffset != 0 &&
	      HashBlock->Table[i].KeyOffset != (ULONG_PTR) -1 &&
	      (HashBlock->Table[i].HashValue == 0 ||
	       CmiCompareHash(KeyName, (PCHAR)&HashBlock->Table[i].HashValue)))
	    {
	      CurSubKeyCell = CmiGetCell (RegistryHive,
					  HashBlock->Table[i].KeyOffset,
					  NULL);
	      if (CurSubKeyCell == NULL)
		{
		  DPRINT("CmiGetBlock() failed\n");
		  return STATUS_UNSUCCESSFUL;
		}

	      if (CmiCompareKeyNames(KeyName, CurSubKeyCell))
		{
		  *SubKeyCell = CurSubKeyCell;
		  *BlockOffset = HashBlock->Table[i].KeyOffset;
		  break;
		}
	    }
	}
    }

  return STATUS_SUCCESS;
}


NTSTATUS
CmiAddSubKey(PREGISTRY_HIVE RegistryHive,
	     PKEY_OBJECT ParentKey,
	     PKEY_OBJECT SubKey,
	     PUNICODE_STRING SubKeyName,
	     ULONG TitleIndex,
	     PUNICODE_STRING Class,
	     ULONG CreateOptions)
{
  PHASH_TABLE_CELL HashBlock;
  BLOCK_OFFSET NKBOffset;
  PKEY_CELL NewKeyCell;
  ULONG NewBlockSize;
  PKEY_CELL ParentKeyCell;
  PDATA_CELL ClassCell;
  NTSTATUS Status;
  USHORT NameSize;
  PWSTR NamePtr;
  BOOLEAN Packable;
  ULONG i;

  ParentKeyCell = ParentKey->KeyCell;

  VERIFY_KEY_CELL(ParentKeyCell);

  /* Skip leading backslash */
  if (SubKeyName->Buffer[0] == L'\\')
    {
      NamePtr = &SubKeyName->Buffer[1];
      NameSize = SubKeyName->Length - sizeof(WCHAR);
    }
  else
    {
      NamePtr = SubKeyName->Buffer;
      NameSize = SubKeyName->Length;
    }

  /* Check whether key name can be packed */
  Packable = TRUE;
  for (i = 0; i < NameSize / sizeof(WCHAR); i++)
    {
      if (NamePtr[i] & 0xFF00)
	{
	  Packable = FALSE;
	  break;
	}
    }

  /* Adjust name size */
  if (Packable)
    {
      NameSize = NameSize / sizeof(WCHAR);
    }

  DPRINT("Key %S  Length %lu  %s\n", NamePtr, NameSize, (Packable)?"True":"False");

  Status = STATUS_SUCCESS;

  NewBlockSize = sizeof(KEY_CELL) + NameSize;
  Status = CmiAllocateCell (RegistryHive,
			    NewBlockSize,
			    (PVOID) &NewKeyCell,
			    &NKBOffset);
  if (NewKeyCell == NULL)
    {
      Status = STATUS_INSUFFICIENT_RESOURCES;
    }
  else
    {
      NewKeyCell->Id = REG_KEY_CELL_ID;
      NewKeyCell->Flags = 0;
      KeQuerySystemTime(&NewKeyCell->LastWriteTime);
      NewKeyCell->ParentKeyOffset = -1;
      NewKeyCell->NumberOfSubKeys = 0;
      NewKeyCell->HashTableOffset = -1;
      NewKeyCell->NumberOfValues = 0;
      NewKeyCell->ValueListOffset = -1;
      NewKeyCell->SecurityKeyOffset = -1;
      NewKeyCell->ClassNameOffset = -1;

      /* Pack the key name */
      NewKeyCell->NameSize = NameSize;
      if (Packable)
	{
	  NewKeyCell->Flags |= REG_KEY_NAME_PACKED;
	  for (i = 0; i < NameSize; i++)
	    {
	      NewKeyCell->Name[i] = (CHAR)(NamePtr[i] & 0x00FF);
	    }
	}
      else
	{
	  RtlCopyMemory(NewKeyCell->Name,
			NamePtr,
			NameSize);
	}

      VERIFY_KEY_CELL(NewKeyCell);

      if (Class != NULL)
	{
	  NewKeyCell->ClassSize = Class->Length;
	  Status = CmiAllocateCell (RegistryHive,
				    sizeof(CELL_HEADER) + NewKeyCell->ClassSize,
				    (PVOID)&ClassCell,
				    &NewKeyCell->ClassNameOffset);
	  RtlCopyMemory (ClassCell->Data,
			 Class->Buffer,
			 Class->Length);
	}
    }

  if (!NT_SUCCESS(Status))
    {
      return Status;
    }

  SubKey->KeyCell = NewKeyCell;
  SubKey->KeyCellOffset = NKBOffset;

  /* Don't modify hash table if key is located in a pointer-based hive and parent key is not */
  if (IsPointerHive(RegistryHive) && (!IsPointerHive(ParentKey->RegistryHive)))
    {
      return(Status);
    }

  if (ParentKeyCell->HashTableOffset == (ULONG_PTR) -1)
    {
      Status = CmiAllocateHashTableCell (RegistryHive,
					 &HashBlock,
					 &ParentKeyCell->HashTableOffset,
					 REG_INIT_HASH_TABLE_SIZE);
      if (!NT_SUCCESS(Status))
	{
	  return(Status);
	}
    }
  else
    {
      HashBlock = CmiGetCell (RegistryHive,
			      ParentKeyCell->HashTableOffset,
			      NULL);
      if (HashBlock == NULL)
	{
	  DPRINT("CmiGetCell() failed\n");
	  return STATUS_UNSUCCESSFUL;
	}

      if (((ParentKeyCell->NumberOfSubKeys + 1) >= HashBlock->HashTableSize))
	{
	  PHASH_TABLE_CELL NewHashBlock;
	  BLOCK_OFFSET HTOffset;

	  /* Reallocate the hash table cell */
	  Status = CmiAllocateHashTableCell (RegistryHive,
					     &NewHashBlock,
					     &HTOffset,
					     HashBlock->HashTableSize +
					       REG_EXTEND_HASH_TABLE_SIZE);
	  if (!NT_SUCCESS(Status))
	    {
	      return Status;
	    }

	  RtlZeroMemory(&NewHashBlock->Table[0],
			sizeof(NewHashBlock->Table[0]) * NewHashBlock->HashTableSize);
	  RtlCopyMemory(&NewHashBlock->Table[0],
			&HashBlock->Table[0],
			sizeof(NewHashBlock->Table[0]) * HashBlock->HashTableSize);
	  CmiDestroyCell (RegistryHive,
			  HashBlock,
			  ParentKeyCell->HashTableOffset);
	  ParentKeyCell->HashTableOffset = HTOffset;
	  HashBlock = NewHashBlock;
	}
    }

  Status = CmiAddKeyToHashTable(RegistryHive,
				HashBlock,
				ParentKeyCell->HashTableOffset,
				NewKeyCell,
				NKBOffset);
  if (NT_SUCCESS(Status))
    {
      ParentKeyCell->NumberOfSubKeys++;
    }

  KeQuerySystemTime (&ParentKeyCell->LastWriteTime);
  CmiMarkBlockDirty (RegistryHive, ParentKey->KeyCellOffset);

  return(Status);
}


NTSTATUS
CmiRemoveSubKey(PREGISTRY_HIVE RegistryHive,
		PKEY_OBJECT ParentKey,
		PKEY_OBJECT SubKey)
{
  PHASH_TABLE_CELL HashBlock;
  PVALUE_LIST_CELL ValueList;
  PVALUE_CELL ValueCell;
  PDATA_CELL DataCell;
  ULONG i;

  DPRINT("CmiRemoveSubKey() called\n");

  /* Remove all values */
  if (SubKey->KeyCell->NumberOfValues != 0)
    {
      /* Get pointer to the value list cell */
      ValueList = CmiGetCell (RegistryHive,
			      SubKey->KeyCell->ValueListOffset,
			      NULL);
      if (ValueList == NULL)
	{
	  DPRINT("CmiGetCell() failed\n");
	  return STATUS_UNSUCCESSFUL;
	}

      /* Enumerate all values */
      for (i = 0; i < SubKey->KeyCell->NumberOfValues; i++)
	{
	  /* Get pointer to value cell */
	  ValueCell = CmiGetCell(RegistryHive,
				 ValueList->ValueOffset[i],
				 NULL);
	  if (ValueCell == NULL)
	    {
	      DPRINT("CmiGetCell() failed\n");
	      return STATUS_UNSUCCESSFUL;
	    }

	  if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET)
              && ValueCell->DataSize > sizeof(BLOCK_OFFSET))
	    {
	      DataCell = CmiGetCell (RegistryHive,
				     ValueCell->DataOffset,
				     NULL);
	      if (DataCell == NULL)
		{
		  DPRINT("CmiGetCell() failed\n");
		  return STATUS_UNSUCCESSFUL;
		}

	      if (DataCell != NULL)
		{
		  /* Destroy data cell */
		  CmiDestroyCell (RegistryHive,
				  DataCell,
				  ValueCell->DataOffset);
		}
	    }

	  /* Destroy value cell */
	  CmiDestroyCell (RegistryHive,
			  ValueCell,
			  ValueList->ValueOffset[i]);
	}

      /* Destroy value list cell */
      CmiDestroyCell (RegistryHive,
		      ValueList,
		      SubKey->KeyCell->ValueListOffset);

      SubKey->KeyCell->NumberOfValues = 0;
      SubKey->KeyCell->ValueListOffset = (BLOCK_OFFSET)-1;

      CmiMarkBlockDirty(RegistryHive,
			SubKey->KeyCellOffset);
    }

  /* Remove the key from the parent key's hash block */
  if (ParentKey->KeyCell->HashTableOffset != (BLOCK_OFFSET) -1)
    {
      DPRINT("ParentKey HashTableOffset %lx\n", ParentKey->KeyCell->HashTableOffset);
      HashBlock = CmiGetCell (ParentKey->RegistryHive,
			      ParentKey->KeyCell->HashTableOffset,
			      NULL);
      if (HashBlock == NULL)
	{
	  DPRINT("CmiGetCell() failed\n");
	  return STATUS_UNSUCCESSFUL;
	}
      DPRINT("ParentKey HashBlock %p\n", HashBlock);
      if (HashBlock != NULL)
	{
	  CmiRemoveKeyFromHashTable(ParentKey->RegistryHive,
				    HashBlock,
				    SubKey->KeyCellOffset);
	  CmiMarkBlockDirty(ParentKey->RegistryHive,
			    ParentKey->KeyCell->HashTableOffset);
	}
    }

  /* Remove the key's hash block */
  if (SubKey->KeyCell->HashTableOffset != (BLOCK_OFFSET) -1)
    {
      DPRINT("SubKey HashTableOffset %lx\n", SubKey->KeyCell->HashTableOffset);
      HashBlock = CmiGetCell (RegistryHive,
			      SubKey->KeyCell->HashTableOffset,
			      NULL);
      if (HashBlock == NULL)
	{
	  DPRINT("CmiGetCell() failed\n");
	  return STATUS_UNSUCCESSFUL;
	}
      DPRINT("SubKey HashBlock %p\n", HashBlock);
      if (HashBlock != NULL)
	{
	  CmiDestroyCell (RegistryHive,
			  HashBlock,
			  SubKey->KeyCell->HashTableOffset);
	  SubKey->KeyCell->HashTableOffset = -1;
	}
    }

  /* Decrement the number of the parent key's sub keys */
  if (ParentKey != NULL)
    {
      DPRINT("ParentKey %p\n", ParentKey);
      ParentKey->KeyCell->NumberOfSubKeys--;

      /* Remove the parent key's hash table */
      if (ParentKey->KeyCell->NumberOfSubKeys == 0)
	{
	  DPRINT("ParentKey HashTableOffset %lx\n", ParentKey->KeyCell->HashTableOffset);
	  HashBlock = CmiGetCell (ParentKey->RegistryHive,
				  ParentKey->KeyCell->HashTableOffset,
				  NULL);
	  if (HashBlock == NULL)
	    {
	      DPRINT("CmiGetCell() failed\n");
	      return STATUS_UNSUCCESSFUL;
	    }
	  DPRINT("ParentKey HashBlock %p\n", HashBlock);
	  if (HashBlock != NULL)
	    {
	      CmiDestroyCell (ParentKey->RegistryHive,
			      HashBlock,
			      ParentKey->KeyCell->HashTableOffset);
	      ParentKey->KeyCell->HashTableOffset = (BLOCK_OFFSET)-1;
	    }
	}

      KeQuerySystemTime(&ParentKey->KeyCell->LastWriteTime);
      CmiMarkBlockDirty(ParentKey->RegistryHive,
			ParentKey->KeyCellOffset);
    }

  /* Destroy key cell */
  CmiDestroyCell (RegistryHive,
		  SubKey->KeyCell,
		  SubKey->KeyCellOffset);
  SubKey->KeyCell = NULL;
  SubKey->KeyCellOffset = (BLOCK_OFFSET)-1;

  DPRINT("CmiRemoveSubKey() done\n");

  return STATUS_SUCCESS;
}


NTSTATUS
CmiScanKeyForValue(IN PREGISTRY_HIVE RegistryHive,
		   IN PKEY_CELL KeyCell,
		   IN PUNICODE_STRING ValueName,
		   OUT PVALUE_CELL *ValueCell,
		   OUT BLOCK_OFFSET *ValueCellOffset)
{
  PVALUE_LIST_CELL ValueListCell;
  PVALUE_CELL CurValueCell;
  ULONG i;

  *ValueCell = NULL;
  if (ValueCellOffset != NULL)
    *ValueCellOffset = (BLOCK_OFFSET)-1;

  /* The key does not have any values */
  if (KeyCell->ValueListOffset == (BLOCK_OFFSET)-1)
    {
      return STATUS_OBJECT_NAME_NOT_FOUND;
    }

  ValueListCell = CmiGetCell (RegistryHive, KeyCell->ValueListOffset, NULL);
  if (ValueListCell == NULL)
    {
      DPRINT("ValueListCell is NULL\n");
      return STATUS_UNSUCCESSFUL;
    }

  VERIFY_VALUE_LIST_CELL(ValueListCell);

  for (i = 0; i < KeyCell->NumberOfValues; i++)
    {
      CurValueCell = CmiGetCell (RegistryHive,
				 ValueListCell->ValueOffset[i],
				 NULL);
      if (CurValueCell == NULL)
	{
	  DPRINT("CmiGetBlock() failed\n");
	  return STATUS_UNSUCCESSFUL;
	}

      if ((CurValueCell != NULL) &&
	  CmiComparePackedNames(ValueName,
				CurValueCell->Name,
				CurValueCell->NameSize,
				(BOOLEAN)((CurValueCell->Flags & REG_VALUE_NAME_PACKED) ? TRUE : FALSE)))
	{
	  *ValueCell = CurValueCell;
	  if (ValueCellOffset != NULL)
	    *ValueCellOffset = ValueListCell->ValueOffset[i];
	  //DPRINT("Found value %s\n", ValueName);
	  return STATUS_SUCCESS;
	}
    }

  return STATUS_OBJECT_NAME_NOT_FOUND;
}


NTSTATUS
CmiGetValueFromKeyByIndex(IN PREGISTRY_HIVE RegistryHive,
			  IN PKEY_CELL KeyCell,
			  IN ULONG Index,
			  OUT PVALUE_CELL *ValueCell)
{
  PVALUE_LIST_CELL ValueListCell;
  PVALUE_CELL CurValueCell;

  *ValueCell = NULL;

  if (KeyCell->ValueListOffset == (BLOCK_OFFSET)-1)
    {
      return STATUS_NO_MORE_ENTRIES;
    }

  if (Index >= KeyCell->NumberOfValues)
    {
      return STATUS_NO_MORE_ENTRIES;
    }


  ValueListCell = CmiGetCell (RegistryHive, KeyCell->ValueListOffset, NULL);
  if (ValueListCell == NULL)
    {
      DPRINT("CmiGetBlock() failed\n");
      return STATUS_UNSUCCESSFUL;
    }

  VERIFY_VALUE_LIST_CELL(ValueListCell);


  CurValueCell = CmiGetCell (RegistryHive,
			     ValueListCell->ValueOffset[Index],
			     NULL);
  if (CurValueCell == NULL)
    {
      DPRINT("CmiGetBlock() failed\n");
      return STATUS_UNSUCCESSFUL;
    }

  *ValueCell = CurValueCell;

  return STATUS_SUCCESS;
}


NTSTATUS
CmiAddValueToKey(IN PREGISTRY_HIVE RegistryHive,
		 IN PKEY_CELL KeyCell,
		 IN BLOCK_OFFSET KeyCellOffset,
		 IN PUNICODE_STRING ValueName,
		 OUT PVALUE_CELL *pValueCell,
		 OUT BLOCK_OFFSET *pValueCellOffset)
{
  PVALUE_LIST_CELL NewValueListCell;
  PVALUE_LIST_CELL ValueListCell;
  PVALUE_CELL NewValueCell;
  BLOCK_OFFSET NewValueListCellOffset;
  BLOCK_OFFSET ValueListCellOffset;
  BLOCK_OFFSET NewValueCellOffset;
  ULONG CellSize;
  NTSTATUS Status;

  DPRINT("KeyCell->ValuesOffset %lu\n", (ULONG)KeyCell->ValueListOffset);

  ValueListCell = CmiGetCell (RegistryHive, KeyCell->ValueListOffset, NULL);
  if (ValueListCell == NULL)
    {
      CellSize = sizeof(VALUE_LIST_CELL) +
		 (3 * sizeof(BLOCK_OFFSET));
      Status = CmiAllocateCell (RegistryHive,
				CellSize,
				(PVOID) &ValueListCell,
				&ValueListCellOffset);
      if (!NT_SUCCESS(Status))
	{
	  return Status;
	}

      KeyCell->ValueListOffset = ValueListCellOffset;
      CmiMarkBlockDirty(RegistryHive, KeyCellOffset);
      CmiMarkBlockDirty(RegistryHive, ValueListCellOffset);
    }
  else if (KeyCell->NumberOfValues >= 
	   (((ULONG)ABS_VALUE(ValueListCell->CellSize) - sizeof(VALUE_LIST_CELL)) / sizeof(BLOCK_OFFSET)))
    {
#if 0
      CellSize = sizeof(VALUE_LIST_CELL) +
		 ((KeyCell->NumberOfValues + REG_VALUE_LIST_CELL_MULTIPLE) * sizeof(BLOCK_OFFSET));
#endif
      CellSize = 2 * (ULONG)ABS_VALUE(ValueListCell->CellSize);
      Status = CmiAllocateCell (RegistryHive,
				CellSize,
				(PVOID) &NewValueListCell,
				&NewValueListCellOffset);
      if (!NT_SUCCESS(Status))
	{
	  return Status;
	}

      RtlCopyMemory(&NewValueListCell->ValueOffset[0],
		    &ValueListCell->ValueOffset[0],
		    sizeof(BLOCK_OFFSET) * KeyCell->NumberOfValues);
      CmiDestroyCell (RegistryHive, ValueListCell, KeyCell->ValueListOffset);
      CmiMarkBlockDirty (RegistryHive, KeyCell->ValueListOffset);

      KeyCell->ValueListOffset = NewValueListCellOffset;
      ValueListCell = NewValueListCell;
      CmiMarkBlockDirty (RegistryHive, KeyCellOffset);
      CmiMarkBlockDirty (RegistryHive, NewValueListCellOffset);
    }

  DPRINT("KeyCell->NumberOfValues %lu, ValueListCell->CellSize %lu (%lu %lx)\n",
	 KeyCell->NumberOfValues,
	 (ULONG)ABS_VALUE(ValueListCell->CellSize),
	 ((ULONG)ABS_VALUE(ValueListCell->CellSize) - sizeof(VALUE_LIST_CELL)) / sizeof(BLOCK_OFFSET),
	 ((ULONG)ABS_VALUE(ValueListCell->CellSize) - sizeof(VALUE_LIST_CELL)) / sizeof(BLOCK_OFFSET));

  Status = CmiAllocateValueCell(RegistryHive,
				&NewValueCell,
				&NewValueCellOffset,
				ValueName);
  if (!NT_SUCCESS(Status))
    {
      return Status;
    }

  ValueListCell->ValueOffset[KeyCell->NumberOfValues] = NewValueCellOffset;
  KeyCell->NumberOfValues++;

  CmiMarkBlockDirty(RegistryHive, KeyCellOffset);
  CmiMarkBlockDirty(RegistryHive, KeyCell->ValueListOffset);
  CmiMarkBlockDirty(RegistryHive, NewValueCellOffset);

  *pValueCell = NewValueCell;
  *pValueCellOffset = NewValueCellOffset;

  return STATUS_SUCCESS;
}


NTSTATUS
CmiDeleteValueFromKey(IN PREGISTRY_HIVE RegistryHive,
		      IN PKEY_CELL KeyCell,
		      IN BLOCK_OFFSET KeyCellOffset,
		      IN PUNICODE_STRING ValueName)
{
  PVALUE_LIST_CELL ValueListCell;
  PVALUE_CELL CurValueCell;
  ULONG i;
  NTSTATUS Status;

  ValueListCell = CmiGetCell (RegistryHive, KeyCell->ValueListOffset, NULL);
  if (ValueListCell == NULL)
    {
      DPRINT1("CmiGetBlock() failed\n");
      return STATUS_SUCCESS;
    }

  VERIFY_VALUE_LIST_CELL(ValueListCell);

  for (i = 0; i < KeyCell->NumberOfValues; i++)
    {
      CurValueCell = CmiGetCell (RegistryHive, ValueListCell->ValueOffset[i], NULL);
      if (CurValueCell == NULL)
	{
	  DPRINT1("CmiGetBlock() failed\n");
	  return STATUS_UNSUCCESSFUL;
	}

      if (CmiComparePackedNames(ValueName,
				CurValueCell->Name,
				CurValueCell->NameSize,
				(BOOLEAN)((CurValueCell->Flags & REG_VALUE_NAME_PACKED) ? TRUE : FALSE)))
	{
	  Status = CmiDestroyValueCell(RegistryHive,
				       CurValueCell,
				       ValueListCell->ValueOffset[i]);
	  if (CurValueCell == NULL)
	    {
	      DPRINT1("CmiDestroyValueCell() failed\n");
	      return Status;
	    }

	  if (i < (KeyCell->NumberOfValues - 1))
	    {
	      RtlMoveMemory(&ValueListCell->ValueOffset[i],
			    &ValueListCell->ValueOffset[i + 1],
			    sizeof(BLOCK_OFFSET) * (KeyCell->NumberOfValues - 1 - i));
	    }
	  ValueListCell->ValueOffset[KeyCell->NumberOfValues - 1] = 0;


	  KeyCell->NumberOfValues--;

	  if (KeyCell->NumberOfValues == 0)
	    {
	      CmiDestroyCell(RegistryHive,
			     ValueListCell,
			     KeyCell->ValueListOffset);
	      KeyCell->ValueListOffset = -1;
	    }
	  else
	    {
	      CmiMarkBlockDirty(RegistryHive,
				KeyCell->ValueListOffset);
	    }

	  CmiMarkBlockDirty(RegistryHive,
			    KeyCellOffset);

	  return STATUS_SUCCESS;
	}
    }

  DPRINT("Couldn't find the desired value\n");

  return STATUS_OBJECT_NAME_NOT_FOUND;
}


NTSTATUS
CmiAllocateHashTableCell (IN PREGISTRY_HIVE RegistryHive,
	OUT PHASH_TABLE_CELL *HashBlock,
	OUT BLOCK_OFFSET *HBOffset,
	IN ULONG SubKeyCount)
{
  PHASH_TABLE_CELL NewHashBlock;
  ULONG NewHashSize;
  NTSTATUS Status;

  Status = STATUS_SUCCESS;
  *HashBlock = NULL;
  NewHashSize = sizeof(HASH_TABLE_CELL) + 
		(SubKeyCount * sizeof(HASH_RECORD));
  Status = CmiAllocateCell (RegistryHive,
			    NewHashSize,
			    (PVOID*) &NewHashBlock,
			    HBOffset);

  if ((NewHashBlock == NULL) || (!NT_SUCCESS(Status)))
    {
      Status = STATUS_INSUFFICIENT_RESOURCES;
    }
  else
    {
      ASSERT(SubKeyCount <= 0xffff); /* should really be USHORT_MAX or similar */
      NewHashBlock->Id = REG_HASH_TABLE_CELL_ID;
      NewHashBlock->HashTableSize = (USHORT)SubKeyCount;
      *HashBlock = NewHashBlock;
    }

  return Status;
}


PKEY_CELL
CmiGetKeyFromHashByIndex(PREGISTRY_HIVE RegistryHive,
			 PHASH_TABLE_CELL HashBlock,
			 ULONG Index)
{
  BLOCK_OFFSET KeyOffset;
  PKEY_CELL KeyCell;

  if (HashBlock == NULL)
    return NULL;

  if (IsPointerHive(RegistryHive))
    {
      KeyCell = (PKEY_CELL) HashBlock->Table[Index].KeyOffset;
    }
  else
    {
      KeyOffset =  HashBlock->Table[Index].KeyOffset;
      KeyCell = CmiGetCell (RegistryHive, KeyOffset, NULL);
    }

  return KeyCell;
}


NTSTATUS
CmiAddKeyToHashTable(PREGISTRY_HIVE RegistryHive,
		     PHASH_TABLE_CELL HashCell,
		     BLOCK_OFFSET HashCellOffset,
		     PKEY_CELL NewKeyCell,
		     BLOCK_OFFSET NKBOffset)
{
  ULONG i;

  for (i = 0; i < HashCell->HashTableSize; i++)
    {
      if (HashCell->Table[i].KeyOffset == 0)
	{
	  HashCell->Table[i].KeyOffset = NKBOffset;
	  HashCell->Table[i].HashValue = 0;
	  if (NewKeyCell->Flags & REG_KEY_NAME_PACKED)
	    {
	      RtlCopyMemory(&HashCell->Table[i].HashValue,
			    NewKeyCell->Name,
			    min(NewKeyCell->NameSize, sizeof(ULONG)));
	    }
	  CmiMarkBlockDirty(RegistryHive, HashCellOffset);
	  return STATUS_SUCCESS;
	}
    }

  return STATUS_UNSUCCESSFUL;
}


NTSTATUS
CmiRemoveKeyFromHashTable(PREGISTRY_HIVE RegistryHive,
			  PHASH_TABLE_CELL HashBlock,
			  BLOCK_OFFSET NKBOffset)
{
  ULONG i;

  for (i = 0; i < HashBlock->HashTableSize; i++)
    {
      if (HashBlock->Table[i].KeyOffset == NKBOffset)
	{
	  HashBlock->Table[i].KeyOffset = 0;
	  HashBlock->Table[i].HashValue = 0;
	  return STATUS_SUCCESS;
	}
    }

  return STATUS_UNSUCCESSFUL;
}


NTSTATUS
CmiAllocateValueCell(PREGISTRY_HIVE RegistryHive,
		     PVALUE_CELL *ValueCell,
		     BLOCK_OFFSET *VBOffset,
		     IN PUNICODE_STRING ValueName)
{
  PVALUE_CELL NewValueCell;
  NTSTATUS Status;
  BOOLEAN Packable;
  ULONG NameSize;
  ULONG i;

  Status = STATUS_SUCCESS;

  NameSize = CmiGetPackedNameLength(ValueName,
				    &Packable);

  DPRINT("ValueName->Length %lu  NameSize %lu\n", ValueName->Length, NameSize);

  Status = CmiAllocateCell (RegistryHive,
			    sizeof(VALUE_CELL) + NameSize,
			    (PVOID*) &NewValueCell,
			    VBOffset);
  if ((NewValueCell == NULL) || (!NT_SUCCESS(Status)))
    {
      Status = STATUS_INSUFFICIENT_RESOURCES;
    }
  else
    {
      ASSERT(NameSize <= 0xffff); /* should really be USHORT_MAX or similar */
      NewValueCell->Id = REG_VALUE_CELL_ID;
      NewValueCell->NameSize = (USHORT)NameSize;
      if (Packable)
	{
	  /* Pack the value name */
	  for (i = 0; i < NameSize; i++)
	    NewValueCell->Name[i] = (CHAR)ValueName->Buffer[i];
	  NewValueCell->Flags |= REG_VALUE_NAME_PACKED;
	}
      else
	{
	  /* Copy the value name */
	  RtlCopyMemory(NewValueCell->Name,
			ValueName->Buffer,
			NameSize);
	  NewValueCell->Flags = 0;
	}
      NewValueCell->DataType = 0;
      NewValueCell->DataSize = 0;
      NewValueCell->DataOffset = (BLOCK_OFFSET)-1;
      *ValueCell = NewValueCell;
    }

  return Status;
}


NTSTATUS
CmiDestroyValueCell(PREGISTRY_HIVE RegistryHive,
		    PVALUE_CELL ValueCell,
		    BLOCK_OFFSET ValueCellOffset)
{
  NTSTATUS Status;
  PVOID DataCell;
  PHBIN Bin;

  DPRINT("CmiDestroyValueCell(Cell %p  Offset %lx)\n",
	 ValueCell, ValueCellOffset);

  VERIFY_VALUE_CELL(ValueCell);

  /* Destroy the data cell */
  if (!(ValueCell->DataSize & REG_DATA_IN_OFFSET)
      && ValueCell->DataSize > sizeof(BLOCK_OFFSET))
    {
      DataCell = CmiGetCell (RegistryHive, ValueCell->DataOffset, &Bin);
      if (DataCell == NULL)
	{
	  DPRINT("CmiGetCell() failed\n");
	  return STATUS_UNSUCCESSFUL;
	}

      Status = CmiDestroyCell (RegistryHive, DataCell, ValueCell->DataOffset);
      if (!NT_SUCCESS(Status))
	{
	  return Status;
	}

      /* Update time of heap */
      if (!IsNoFileHive(RegistryHive))
	KeQuerySystemTime(&Bin->DateModified);
    }

  /* Destroy the value cell */
  Status = CmiDestroyCell (RegistryHive, ValueCell, ValueCellOffset);

  /* Update time of heap */
  if (!IsNoFileHive(RegistryHive) && CmiGetCell (RegistryHive, ValueCellOffset, &Bin))
    {
      KeQuerySystemTime(&Bin->DateModified);
    }

  return Status;
}


NTSTATUS
CmiAddBin(PREGISTRY_HIVE RegistryHive,
	  ULONG BlockCount,
	  PVOID *NewBlock,
	  BLOCK_OFFSET *NewBlockOffset)
{
  PBLOCK_LIST_ENTRY BlockList;
  PCELL_HEADER tmpBlock;
  PHBIN tmpBin;
  ULONG BinSize;
  ULONG i;

  DPRINT ("CmiAddBin (BlockCount %lu)\n", BlockCount);

  BinSize = BlockCount * REG_BLOCK_SIZE;
  tmpBin = ExAllocatePool(PagedPool, BinSize);
  if (tmpBin == NULL)
    {
      return STATUS_INSUFFICIENT_RESOURCES;
    }
  RtlZeroMemory (tmpBin,
		 BinSize);

  tmpBin->HeaderId = REG_BIN_ID;
  tmpBin->BinOffset = RegistryHive->FileSize - REG_BLOCK_SIZE;
  RegistryHive->FileSize += BinSize;
  tmpBin->BinSize = BinSize;
  tmpBin->Unused1 = 0;
  KeQuerySystemTime(&tmpBin->DateModified);
  tmpBin->Unused2 = 0;

  DPRINT ("  BinOffset %lx  BinSize %lx\n", tmpBin->BinOffset,tmpBin->BinSize);

  /* Allocate new block list */
  BlockList = ExAllocatePool(NonPagedPool,
			     sizeof(BLOCK_LIST_ENTRY) * (RegistryHive->BlockListSize + BlockCount));
  if (BlockList == NULL)
    {
      ExFreePool(tmpBin);
      return STATUS_INSUFFICIENT_RESOURCES;
    }

  if (RegistryHive->BlockListSize > 0)
    {
      RtlCopyMemory (BlockList,
		     RegistryHive->BlockList,
		     sizeof(BLOCK_LIST_ENTRY) * RegistryHive->BlockListSize);
      ExFreePool(RegistryHive->BlockList);
    }

  RegistryHive->BlockList = BlockList;
  for (i = 0; i < BlockCount; i++)
    {
      RegistryHive->BlockList[RegistryHive->BlockListSize + i].Block =
	(PVOID)((ULONG_PTR)tmpBin + (i * REG_BLOCK_SIZE));
      RegistryHive->BlockList[RegistryHive->BlockListSize + i].Bin = tmpBin;
    }
  RegistryHive->BlockListSize += BlockCount;

  /* Initialize a free block in this heap : */
  tmpBlock = (PCELL_HEADER)((ULONG_PTR) tmpBin + REG_HBIN_DATA_OFFSET);
  tmpBlock->CellSize = (BinSize - REG_HBIN_DATA_OFFSET);

  /* Grow bitmap if necessary */
  if (IsNoFileHive(RegistryHive) &&
      (RegistryHive->BlockListSize % (sizeof(ULONG) * 8) == 0))
    {
      PULONG BitmapBuffer;
      ULONG BitmapSize;

      DPRINT("Grow hive bitmap\n");

      /* Calculate bitmap size in bytes (always a multiple of 32 bits) */
      BitmapSize = ROUND_UP(RegistryHive->BlockListSize, sizeof(ULONG) * 8) / 8;
      DPRINT("RegistryHive->BlockListSize: %lu\n", RegistryHive->BlockListSize);
      DPRINT("BitmapSize:  %lu Bytes  %lu Bits\n", BitmapSize, BitmapSize * 8);
      BitmapBuffer = (PULONG)ExAllocatePool(PagedPool,
					    BitmapSize);
      RtlZeroMemory(BitmapBuffer, BitmapSize);
      RtlCopyMemory(BitmapBuffer,
		    RegistryHive->DirtyBitMap.Buffer,
		    RegistryHive->DirtyBitMap.SizeOfBitMap);
      ExFreePool(RegistryHive->BitmapBuffer);
      RegistryHive->BitmapBuffer = BitmapBuffer;
      RtlInitializeBitMap(&RegistryHive->DirtyBitMap,
			  RegistryHive->BitmapBuffer,
			  BitmapSize * 8);
    }

  *NewBlock = (PVOID) tmpBlock;

  if (NewBlockOffset)
    *NewBlockOffset = tmpBin->BinOffset + REG_HBIN_DATA_OFFSET;

  /* Mark new bin dirty */
  CmiMarkBinDirty(RegistryHive,
		  tmpBin->BinOffset);

  return STATUS_SUCCESS;
}


NTSTATUS
CmiAllocateCell (PREGISTRY_HIVE RegistryHive,
		 LONG CellSize,
		 PVOID *Cell,
		 BLOCK_OFFSET *CellOffset)
{
  PCELL_HEADER NewCell;
  PHBIN Bin;
  ULONG i;
  PVOID Temp;
  NTSTATUS Status;

  /* Round to 16 bytes multiple */
  CellSize = ROUND_UP(CellSize, 16);

  /* Handle volatile hives first */
  if (IsPointerHive(RegistryHive))
    {
      NewCell = ExAllocatePool(NonPagedPool, CellSize);
      if (NewCell == NULL)
	{
	  return STATUS_INSUFFICIENT_RESOURCES;
	}

      RtlZeroMemory (NewCell,
		     CellSize);
      NewCell->CellSize = -CellSize;

      *Cell = NewCell;
      if (CellOffset != NULL)
	*CellOffset = (BLOCK_OFFSET) NewCell;
    }
  else
    {
      /* first search in free blocks */
      NewCell = NULL;
      for (i = 0; i < RegistryHive->FreeListSize; i++)
	{
	  if (RegistryHive->FreeList[i]->CellSize >= CellSize)
	    {
	      NewCell = RegistryHive->FreeList[i];
	      if (CellOffset != NULL)
		*CellOffset = RegistryHive->FreeListOffset[i];

	      /* Update time of heap */
	      Temp = CmiGetCell (RegistryHive,
				 RegistryHive->FreeListOffset[i],
				 &Bin);
	      if (Temp == NULL)
		{
		  DPRINT("CmiGetBlock() failed\n");
		  return STATUS_UNSUCCESSFUL;
		}

	      KeQuerySystemTime(&Bin->DateModified);
	      CmiMarkBlockDirty(RegistryHive, RegistryHive->FreeListOffset[i]);

	      if ((i + 1) < RegistryHive->FreeListSize)
		{
		  RtlMoveMemory(&RegistryHive->FreeList[i],
				&RegistryHive->FreeList[i + 1],
				sizeof(RegistryHive->FreeList[0])
				  * (RegistryHive->FreeListSize - i - 1));
		  RtlMoveMemory(&RegistryHive->FreeListOffset[i],
				&RegistryHive->FreeListOffset[i + 1],
				sizeof(RegistryHive->FreeListOffset[0])
				  * (RegistryHive->FreeListSize - i - 1));
		}
	      RegistryHive->FreeListSize--;
	      break;
	    }
	}

      /* Need to extend hive file : */
      if (NewCell == NULL)
	{
	  /* Add a new bin */
	  Status = CmiAddBin(RegistryHive,
			     ((CellSize + sizeof(HBIN) - 1) / REG_BLOCK_SIZE) + 1,
			     (PVOID *)&NewCell,
			     CellOffset);
	  if (!NT_SUCCESS(Status))
	    return Status;
	}

      *Cell = NewCell;

      /* Split the block in two parts */
      if (NewCell->CellSize > CellSize)
	{
	  NewCell = (PCELL_HEADER) ((ULONG_PTR) NewCell + CellSize);
	  NewCell->CellSize = ((PCELL_HEADER) (*Cell))->CellSize - CellSize;
	  CmiAddFree(RegistryHive,
		     NewCell,
		     *CellOffset + CellSize,
		     TRUE);
	  CmiMarkBlockDirty(RegistryHive,
			    *CellOffset + CellSize);
	}
      else if (NewCell->CellSize < CellSize)
	{
	  return STATUS_UNSUCCESSFUL;
	}

      RtlZeroMemory(*Cell,
		    CellSize);
      ((PCELL_HEADER) (*Cell))->CellSize = -CellSize;
    }

  return STATUS_SUCCESS;
}


NTSTATUS
CmiDestroyCell (PREGISTRY_HIVE RegistryHive,
		PVOID Cell,
		BLOCK_OFFSET CellOffset)
{
  NTSTATUS Status;
  PHBIN pBin;

  Status = STATUS_SUCCESS;

  if (IsPointerHive(RegistryHive))
    {
      ExFreePool(Cell);
    }
  else
    {
      PCELL_HEADER pFree = Cell;

      if (pFree->CellSize < 0)
        pFree->CellSize = -pFree->CellSize;

      /* Clear block (except the block size) */
      RtlZeroMemory(((char*)pFree) + sizeof(ULONG),
		    pFree->CellSize - sizeof(ULONG));

      /* Add block to the list of free blocks */
      CmiAddFree(RegistryHive, Cell, CellOffset, TRUE);

      /* Update time of heap */
      if (!IsNoFileHive(RegistryHive) && CmiGetCell (RegistryHive, CellOffset,&pBin))
	KeQuerySystemTime(&pBin->DateModified);

      CmiMarkBlockDirty(RegistryHive, CellOffset);
    }

  return Status;
}


PVOID
CmiGetCell (PREGISTRY_HIVE RegistryHive,
	    BLOCK_OFFSET CellOffset,
	    PHBIN *Bin)
{
  PHBIN pBin;

  if (Bin != NULL)
    {
      *Bin = NULL;
    }

  if (CellOffset == (BLOCK_OFFSET)-1)
    {
      return NULL;
    }

  if (IsPointerHive (RegistryHive))
    {
      return (PVOID)CellOffset;
    }

  if (CellOffset > RegistryHive->BlockListSize * REG_BLOCK_SIZE)
    {
      DPRINT1("CellOffset exceeds valid range (%lu > %lu)\n",
	      CellOffset, RegistryHive->BlockListSize * REG_BLOCK_SIZE);
      return NULL;
    }

  pBin = RegistryHive->BlockList[CellOffset / REG_BLOCK_SIZE].Bin;
  if (pBin == NULL)
    {
      return NULL;
    }

  if (Bin != NULL)
    {
      *Bin = pBin;
    }

  return((PVOID)((ULONG_PTR)pBin + (CellOffset - pBin->BinOffset)));
}


static BOOLEAN
CmiMergeFree(PREGISTRY_HIVE RegistryHive,
	     PCELL_HEADER FreeBlock,
	     BLOCK_OFFSET FreeOffset)
{
  BLOCK_OFFSET BlockOffset;
  BLOCK_OFFSET BinOffset;
  ULONG BlockSize;
  ULONG BinSize;
  PHBIN Bin;
  ULONG i;

  DPRINT("CmiMergeFree(Block %lx  Offset %lx  Size %lx) called\n",
	 FreeBlock, FreeOffset, FreeBlock->CellSize);

  CmiGetCell (RegistryHive,
	      FreeOffset,
	      &Bin);
  DPRINT("Bin %p\n", Bin);
  if (Bin == NULL)
    return(FALSE);

  BinOffset = Bin->BinOffset;
  BinSize = Bin->BinSize;
  DPRINT("Bin %p  Offset %lx  Size %lx\n", Bin, BinOffset, BinSize);

  for (i = 0; i < RegistryHive->FreeListSize; i++)
    {
      BlockOffset = RegistryHive->FreeListOffset[i];
      BlockSize = RegistryHive->FreeList[i]->CellSize;
      if (BlockOffset > BinOffset &&
	  BlockOffset < BinOffset + BinSize)
	{
	  DPRINT("Free block: Offset %lx  Size %lx\n",
		  BlockOffset, BlockSize);

	  if ((i < (RegistryHive->FreeListSize - 1)) &&
	      (BlockOffset + BlockSize == FreeOffset) &&
	      (FreeOffset + FreeBlock->CellSize == RegistryHive->FreeListOffset[i + 1]))
	    {
	      DPRINT("Merge current block with previous and next block\n");

	      RegistryHive->FreeList[i]->CellSize +=
		(FreeBlock->CellSize + RegistryHive->FreeList[i + 1]->CellSize);

	      FreeBlock->CellSize = 0;
	      RegistryHive->FreeList[i + 1]->CellSize = 0;


	      if ((i + 2) < RegistryHive->FreeListSize)
		{
		  RtlMoveMemory(&RegistryHive->FreeList[i + 1],
				&RegistryHive->FreeList[i + 2],
				sizeof(RegistryHive->FreeList[0])
				  * (RegistryHive->FreeListSize - i - 2));
		  RtlMoveMemory(&RegistryHive->FreeListOffset[i + 1],
				&RegistryHive->FreeListOffset[i + 2],
				sizeof(RegistryHive->FreeListOffset[0])
				  * (RegistryHive->FreeListSize - i - 2));
		}
	      RegistryHive->FreeListSize--;

	      CmiMarkBlockDirty(RegistryHive, BlockOffset);

	      return(TRUE);
	    }
	  else if (BlockOffset + BlockSize == FreeOffset)
	    {
	      DPRINT("Merge current block with previous block\n");

	      RegistryHive->FreeList[i]->CellSize += FreeBlock->CellSize;
	      FreeBlock->CellSize = 0;

	      CmiMarkBlockDirty(RegistryHive, BlockOffset);

	      return(TRUE);
	    }
	  else if (FreeOffset + FreeBlock->CellSize == BlockOffset)
	    {
	      DPRINT("Merge current block with next block\n");

	      FreeBlock->CellSize += RegistryHive->FreeList[i]->CellSize;
	      RegistryHive->FreeList[i]->CellSize = 0;
	      RegistryHive->FreeList[i] = FreeBlock;
	      RegistryHive->FreeListOffset[i] = FreeOffset;

	      CmiMarkBlockDirty(RegistryHive, FreeOffset);

	      return(TRUE);
	    }
	}
    }

  return(FALSE);
}


NTSTATUS
CmiAddFree(PREGISTRY_HIVE RegistryHive,
	   PCELL_HEADER FreeBlock,
	   BLOCK_OFFSET FreeOffset,
	   BOOLEAN MergeFreeBlocks)
{
  PCELL_HEADER *tmpList;
  BLOCK_OFFSET *tmpListOffset;
  LONG minInd;
  LONG maxInd;
  LONG medInd;

  ASSERT(RegistryHive);
  ASSERT(FreeBlock);

  DPRINT("FreeBlock %.08lx  FreeOffset %.08lx\n",
	 FreeBlock, FreeOffset);

  /* Merge free blocks */
  if (MergeFreeBlocks == TRUE)
    {
      if (CmiMergeFree(RegistryHive, FreeBlock, FreeOffset))
	return(STATUS_SUCCESS);
    }

  if ((RegistryHive->FreeListSize + 1) > RegistryHive->FreeListMax)
    {
      tmpList = ExAllocatePool(PagedPool,
			  sizeof(PCELL_HEADER) * (RegistryHive->FreeListMax + 32));
      if (tmpList == NULL)
	return STATUS_INSUFFICIENT_RESOURCES;

      tmpListOffset = ExAllocatePool(PagedPool,
			  sizeof(BLOCK_OFFSET) * (RegistryHive->FreeListMax + 32));

      if (tmpListOffset == NULL)
	{
	  ExFreePool(tmpList);
	  return STATUS_INSUFFICIENT_RESOURCES;
	}

      if (RegistryHive->FreeListMax)
	{
	  RtlMoveMemory(tmpList,
			RegistryHive->FreeList,
			sizeof(PCELL_HEADER) * (RegistryHive->FreeListMax));
	  RtlMoveMemory(tmpListOffset,
			RegistryHive->FreeListOffset,
			sizeof(BLOCK_OFFSET) * (RegistryHive->FreeListMax));
	  ExFreePool(RegistryHive->FreeList);
	  ExFreePool(RegistryHive->FreeListOffset);
	}
      RegistryHive->FreeList = tmpList;
      RegistryHive->FreeListOffset = tmpListOffset;
      RegistryHive->FreeListMax += 32;
    }

  /* Add new offset to free list, maintaining list in ascending order */
  if ((RegistryHive->FreeListSize == 0)
     || (RegistryHive->FreeListOffset[RegistryHive->FreeListSize-1] < FreeOffset))
    {
      /* Add to end of list */
      RegistryHive->FreeList[RegistryHive->FreeListSize] = FreeBlock;
      RegistryHive->FreeListOffset[RegistryHive->FreeListSize++] = FreeOffset;
    }
  else if (RegistryHive->FreeListOffset[0] > FreeOffset)
    {
      /* Add to begin of list */
      RtlMoveMemory(&RegistryHive->FreeList[1],
		    &RegistryHive->FreeList[0],
		    sizeof(RegistryHive->FreeList[0]) * RegistryHive->FreeListSize);
      RtlMoveMemory(&RegistryHive->FreeListOffset[1],
		    &RegistryHive->FreeListOffset[0],
		    sizeof(RegistryHive->FreeListOffset[0]) * RegistryHive->FreeListSize);
      RegistryHive->FreeList[0] = FreeBlock;
      RegistryHive->FreeListOffset[0] = FreeOffset;
      RegistryHive->FreeListSize++;
    }
  else
    {
      /* Search where to insert */
      minInd = 0;
      maxInd = RegistryHive->FreeListSize - 1;
      while ((maxInd - minInd) > 1)
	{
	  medInd = (minInd + maxInd) / 2;
	  if (RegistryHive->FreeListOffset[medInd] > FreeOffset)
	    maxInd = medInd;
	  else
	    minInd = medInd;
	}

      /* Insert before maxInd */
      RtlMoveMemory(&RegistryHive->FreeList[maxInd+1],
		    &RegistryHive->FreeList[maxInd],
		    sizeof(RegistryHive->FreeList[0]) * (RegistryHive->FreeListSize - minInd));
      RtlMoveMemory(&RegistryHive->FreeListOffset[maxInd + 1],
		    &RegistryHive->FreeListOffset[maxInd],
		    sizeof(RegistryHive->FreeListOffset[0]) * (RegistryHive->FreeListSize-minInd));
      RegistryHive->FreeList[maxInd] = FreeBlock;
      RegistryHive->FreeListOffset[maxInd] = FreeOffset;
      RegistryHive->FreeListSize++;
    }

  return STATUS_SUCCESS;
}


VOID
CmiMarkBlockDirty(PREGISTRY_HIVE RegistryHive,
		  BLOCK_OFFSET BlockOffset)
{
  PDATA_CELL Cell;
  LONG CellSize;
  ULONG BlockNumber;
  ULONG BlockCount;

  if (IsNoFileHive(RegistryHive))
    return;

  DPRINT("CmiMarkBlockDirty(Offset 0x%lx)\n", (ULONG)BlockOffset);

  BlockNumber = (ULONG)BlockOffset / REG_BLOCK_SIZE;

  Cell = CmiGetCell (RegistryHive,
		     BlockOffset,
		     NULL);

  CellSize = Cell->CellSize;
  if (CellSize < 0)
    CellSize = -CellSize;

  BlockCount = (ROUND_UP(BlockOffset + CellSize, REG_BLOCK_SIZE) -
		ROUND_DOWN(BlockOffset, REG_BLOCK_SIZE)) / REG_BLOCK_SIZE;

  DPRINT("  BlockNumber %lu  Size %lu (%s)  BlockCount %lu\n",
	 BlockNumber,
	 CellSize,
	 (Cell->CellSize < 0) ? "used" : "free",
	 BlockCount);

  RegistryHive->HiveDirty = TRUE;
  RtlSetBits(&RegistryHive->DirtyBitMap,
	     BlockNumber,
	     BlockCount);
}


VOID
CmiMarkBinDirty(PREGISTRY_HIVE RegistryHive,
		BLOCK_OFFSET BinOffset)
{
  ULONG BlockNumber;
  ULONG BlockCount;
  PHBIN Bin;

  if (IsNoFileHive(RegistryHive))
    return;

  DPRINT("CmiMarkBinDirty(Offset 0x%lx)\n", (ULONG)BinOffset);

  BlockNumber = (ULONG)BinOffset / REG_BLOCK_SIZE;

  Bin = RegistryHive->BlockList[BlockNumber].Bin;

  BlockCount = Bin->BinSize / REG_BLOCK_SIZE;

  DPRINT("  BlockNumber %lu  BinSize %lu  BlockCount %lu\n",
	 BlockNumber,
	 Bin->BinSize,
	 BlockCount);

  RegistryHive->HiveDirty = TRUE;
  RtlSetBits(&RegistryHive->DirtyBitMap,
	     BlockNumber,
	     BlockCount);
}


ULONG
CmiGetPackedNameLength(IN PUNICODE_STRING Name,
		       OUT PBOOLEAN Packable)
{
  ULONG i;

  if (Packable != NULL)
    *Packable = TRUE;

  for (i = 0; i < Name->Length / sizeof(WCHAR); i++)
    {
      if (Name->Buffer[i] & 0xFF00)
	{
	  if (Packable != NULL)
	    *Packable = FALSE;
	  return Name->Length;
	}
    }

  return (Name->Length / sizeof(WCHAR));
}


BOOLEAN
CmiComparePackedNames(IN PUNICODE_STRING Name,
		      IN PUCHAR NameBuffer,
		      IN USHORT NameBufferSize,
		      IN BOOLEAN NamePacked)
{
  PWCHAR UNameBuffer;
  ULONG i;

  if (NamePacked == TRUE)
    {
      if (Name->Length != NameBufferSize * sizeof(WCHAR))
	return(FALSE);

      for (i = 0; i < Name->Length / sizeof(WCHAR); i++)
	{
	  if (RtlUpcaseUnicodeChar(Name->Buffer[i]) != RtlUpcaseUnicodeChar((WCHAR)NameBuffer[i]))
	    return(FALSE);
	}
    }
  else
    {
      if (Name->Length != NameBufferSize)
	return(FALSE);

      UNameBuffer = (PWCHAR)NameBuffer;

      for (i = 0; i < Name->Length / sizeof(WCHAR); i++)
	{
	  if (RtlUpcaseUnicodeChar(Name->Buffer[i]) != RtlUpcaseUnicodeChar(UNameBuffer[i]))
	    return(FALSE);
	}
    }

  return(TRUE);
}


VOID
CmiCopyPackedName(PWCHAR NameBuffer,
		  PUCHAR PackedNameBuffer,
		  ULONG PackedNameSize)
{
  ULONG i;

  for (i = 0; i < PackedNameSize; i++)
    NameBuffer[i] = (WCHAR)PackedNameBuffer[i];
}


BOOLEAN
CmiCompareHash(PUNICODE_STRING KeyName,
	       PCHAR HashString)
{
  CHAR Buffer[4];

  Buffer[0] = (KeyName->Length >= 2) ? (CHAR)KeyName->Buffer[0] : 0;
  Buffer[1] = (KeyName->Length >= 4) ? (CHAR)KeyName->Buffer[1] : 0;
  Buffer[2] = (KeyName->Length >= 6) ? (CHAR)KeyName->Buffer[2] : 0;
  Buffer[3] = (KeyName->Length >= 8) ? (CHAR)KeyName->Buffer[3] : 0;

  return (strncmp(Buffer, HashString, 4) == 0);
}


BOOLEAN
CmiCompareHashI(PUNICODE_STRING KeyName,
	        PCHAR HashString)
{
  CHAR Buffer[4];

  Buffer[0] = (KeyName->Length >= 2) ? (CHAR)KeyName->Buffer[0] : 0;
  Buffer[1] = (KeyName->Length >= 4) ? (CHAR)KeyName->Buffer[1] : 0;
  Buffer[2] = (KeyName->Length >= 6) ? (CHAR)KeyName->Buffer[2] : 0;
  Buffer[3] = (KeyName->Length >= 8) ? (CHAR)KeyName->Buffer[3] : 0;

  return (_strnicmp(Buffer, HashString, 4) == 0);
}


BOOLEAN
CmiCompareKeyNames(PUNICODE_STRING KeyName,
		   PKEY_CELL KeyCell)
{
  PWCHAR UnicodeName;
  USHORT i;

  DPRINT("Flags: %hx\n", KeyCell->Flags);

  if (KeyCell->Flags & REG_KEY_NAME_PACKED)
    {
      if (KeyName->Length != KeyCell->NameSize * sizeof(WCHAR))
	return FALSE;

      for (i = 0; i < KeyCell->NameSize; i++)
	{
	  if (KeyName->Buffer[i] != (WCHAR)KeyCell->Name[i])
	    return FALSE;
	}
    }
  else
    {
      if (KeyName->Length != KeyCell->NameSize)
	return FALSE;

      UnicodeName = (PWCHAR)KeyCell->Name;
      for (i = 0; i < KeyCell->NameSize / sizeof(WCHAR); i++)
	{
	  if (KeyName->Buffer[i] != UnicodeName[i])
	    return FALSE;
	}
    }

  return TRUE;
}


BOOLEAN
CmiCompareKeyNamesI(PUNICODE_STRING KeyName,
		    PKEY_CELL KeyCell)
{
  PWCHAR UnicodeName;
  USHORT i;

  DPRINT("Flags: %hx\n", KeyCell->Flags);

  if (KeyCell->Flags & REG_KEY_NAME_PACKED)
    {
      if (KeyName->Length != KeyCell->NameSize * sizeof(WCHAR))
	return FALSE;

      for (i = 0; i < KeyCell->NameSize; i++)
	{
	  if (RtlUpcaseUnicodeChar(KeyName->Buffer[i]) !=
	      RtlUpcaseUnicodeChar((WCHAR)KeyCell->Name[i]))
	    return FALSE;
	}
    }
  else
    {
      if (KeyName->Length != KeyCell->NameSize)
	return FALSE;

      UnicodeName = (PWCHAR)KeyCell->Name;
      for (i = 0; i < KeyCell->NameSize / sizeof(WCHAR); i++)
	{
	  if (RtlUpcaseUnicodeChar(KeyName->Buffer[i]) !=
	      RtlUpcaseUnicodeChar(UnicodeName[i]))
	    return FALSE;
	}
    }

  return TRUE;
}


NTSTATUS
CmiCopyKey (PREGISTRY_HIVE DstHive,
	    PKEY_CELL DstKeyCell,
	    PREGISTRY_HIVE SrcHive,
	    PKEY_CELL SrcKeyCell)
{
  PKEY_CELL NewKeyCell;
  ULONG NewKeyCellSize;
  BLOCK_OFFSET NewKeyCellOffset;
  PHASH_TABLE_CELL NewHashTableCell;
  ULONG NewHashTableSize;
  BLOCK_OFFSET NewHashTableOffset;
  ULONG i;
  NTSTATUS Status;

  DPRINT ("CmiCopyKey() called\n");

  if (DstKeyCell == NULL)
    {
      /* Allocate and copy key cell */
      NewKeyCellSize = sizeof(KEY_CELL) + SrcKeyCell->NameSize;
      Status = CmiAllocateCell (DstHive,
				NewKeyCellSize,
				(PVOID) &NewKeyCell,
				&NewKeyCellOffset);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT1 ("CmiAllocateBlock() failed (Status %lx)\n", Status);
	  return Status;
	}
      if (NewKeyCell == NULL)
	{
	  DPRINT1 ("Failed to allocate a key cell\n");
	  return STATUS_INSUFFICIENT_RESOURCES;
	}

      RtlCopyMemory (NewKeyCell,
		     SrcKeyCell,
		     NewKeyCellSize);

      DstHive->HiveHeader->RootKeyOffset = NewKeyCellOffset;

      /* Copy class name */
      if (SrcKeyCell->ClassNameOffset != (BLOCK_OFFSET) -1)
	{
	  PDATA_CELL SrcClassNameCell;
	  PDATA_CELL NewClassNameCell;
	  BLOCK_OFFSET NewClassNameOffset;

	  SrcClassNameCell = CmiGetCell (SrcHive, SrcKeyCell->ClassNameOffset, NULL),

	  NewKeyCell->ClassSize = SrcKeyCell->ClassSize;
	  Status = CmiAllocateCell (DstHive,
				    sizeof(CELL_HEADER) + NewKeyCell->ClassSize,
				    (PVOID)&NewClassNameCell,
				    &NewClassNameOffset);
	  if (!NT_SUCCESS(Status))
	    {
	      DPRINT1 ("CmiAllocateBlock() failed (Status %lx)\n", Status);
	      return Status;
	    }

	  RtlCopyMemory (NewClassNameCell,
			 SrcClassNameCell,
			 NewKeyCell->ClassSize);
	  NewKeyCell->ClassNameOffset = NewClassNameOffset;
	}
    }
  else
    {
      NewKeyCell = DstKeyCell;
    }

  /* Allocate hash table */
  if (SrcKeyCell->NumberOfSubKeys > 0)
    {
      NewHashTableSize = ROUND_UP(SrcKeyCell->NumberOfSubKeys + 1, 4) - 1;
      Status = CmiAllocateHashTableCell (DstHive,
					 &NewHashTableCell,
					 &NewHashTableOffset,
					 NewHashTableSize);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT1 ("CmiAllocateHashTableBlock() failed (Status %lx)\n", Status);
	  return Status;
	}
      NewKeyCell->HashTableOffset = NewHashTableOffset;
    }

  /* Allocate and copy value list and values */
  if (SrcKeyCell->NumberOfValues != 0)
    {
      PVALUE_LIST_CELL NewValueListCell;
      PVALUE_LIST_CELL SrcValueListCell;
      PVALUE_CELL NewValueCell;
      PVALUE_CELL SrcValueCell;
      PDATA_CELL SrcValueDataCell;
      PDATA_CELL NewValueDataCell;
      BLOCK_OFFSET ValueCellOffset;
      BLOCK_OFFSET ValueDataCellOffset;
      ULONG NewValueListCellSize;
      ULONG NewValueCellSize;


      NewValueListCellSize =
	ROUND_UP(SrcKeyCell->NumberOfValues, 4) * sizeof(BLOCK_OFFSET);
      Status = CmiAllocateCell (DstHive,
				NewValueListCellSize,
				(PVOID)&NewValueListCell,
				&NewKeyCell->ValueListOffset);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT1 ("CmiAllocateBlock() failed (Status %lx)\n", Status);
	  return Status;
	}

      RtlZeroMemory (NewValueListCell,
		     NewValueListCellSize);

      /* Copy values */
      SrcValueListCell = CmiGetCell (SrcHive, SrcKeyCell->ValueListOffset, NULL);
      for (i = 0; i < SrcKeyCell->NumberOfValues; i++)
	{
	  /* Copy value cell */
	  SrcValueCell = CmiGetCell (SrcHive, SrcValueListCell->ValueOffset[i], NULL);

	  NewValueCellSize = sizeof(VALUE_CELL) + SrcValueCell->NameSize;
	  Status = CmiAllocateCell (DstHive,
				    NewValueCellSize,
				    (PVOID*) &NewValueCell,
				    &ValueCellOffset);
	  if (!NT_SUCCESS(Status))
	    {
	      DPRINT1 ("CmiAllocateBlock() failed (Status %lx)\n", Status);
	      return Status;
	    }

	  NewValueListCell->ValueOffset[i] = ValueCellOffset;
	  RtlCopyMemory (NewValueCell,
			 SrcValueCell,
			 NewValueCellSize);

	  /* Copy value data cell */
	  if (SrcValueCell->DataSize > (LONG) sizeof(PVOID))
	    {
	      SrcValueDataCell = CmiGetCell (SrcHive, SrcValueCell->DataOffset, NULL);

	      Status = CmiAllocateCell (DstHive,
					sizeof(CELL_HEADER) + SrcValueCell->DataSize,
					(PVOID*) &NewValueDataCell,
					&ValueDataCellOffset);
	      if (!NT_SUCCESS(Status))
		{
		  DPRINT1 ("CmiAllocateBlock() failed (Status %lx)\n", Status);
		  return Status;
		}
	      RtlCopyMemory (NewValueDataCell,
			     SrcValueDataCell,
			     SrcValueCell->DataSize);
	      NewValueCell->DataOffset = ValueDataCellOffset;
	    }
	}
    }

  /* Copy subkeys */
  if (SrcKeyCell->NumberOfSubKeys > 0)
    {
      PHASH_TABLE_CELL SrcHashTableCell;
      PKEY_CELL SrcSubKeyCell;
      PKEY_CELL NewSubKeyCell;
      ULONG NewSubKeyCellSize;
      BLOCK_OFFSET NewSubKeyCellOffset;
      PHASH_RECORD SrcHashRecord;

      SrcHashTableCell = CmiGetCell (SrcHive,
				     SrcKeyCell->HashTableOffset,
				     NULL);

      for (i = 0; i < SrcKeyCell->NumberOfSubKeys; i++)
	{
	  SrcHashRecord = &SrcHashTableCell->Table[i];
	  SrcSubKeyCell = CmiGetCell (SrcHive, SrcHashRecord->KeyOffset, NULL);

	  /* Allocate and copy key cell */
	  NewSubKeyCellSize = sizeof(KEY_CELL) + SrcSubKeyCell->NameSize;
	  Status = CmiAllocateCell (DstHive,
				    NewSubKeyCellSize,
				    (PVOID)&NewSubKeyCell,
				    &NewSubKeyCellOffset);
	  if (!NT_SUCCESS(Status))
	    {
	      DPRINT1 ("CmiAllocateBlock() failed (Status %lx)\n", Status);
	      return Status;
	    }
	  if (NewKeyCell == NULL)
	    {
	      DPRINT1 ("Failed to allocate a sub key cell\n");
	      return STATUS_INSUFFICIENT_RESOURCES;
	    }

	  NewHashTableCell->Table[i].KeyOffset = NewSubKeyCellOffset;
	  NewHashTableCell->Table[i].HashValue = SrcHashRecord->HashValue;

	  RtlCopyMemory (NewSubKeyCell,
			 SrcSubKeyCell,
			 NewSubKeyCellSize);

	  /* Copy class name */
	  if (SrcSubKeyCell->ClassNameOffset != (BLOCK_OFFSET) -1)
	    {
	      PDATA_CELL SrcClassNameCell;
	      PDATA_CELL NewClassNameCell;
	      BLOCK_OFFSET NewClassNameOffset;

	      SrcClassNameCell = CmiGetCell (SrcHive,
					     SrcSubKeyCell->ClassNameOffset,
					     NULL),

	      NewSubKeyCell->ClassSize = SrcSubKeyCell->ClassSize;
	      Status = CmiAllocateCell (DstHive,
					sizeof(CELL_HEADER) + NewSubKeyCell->ClassSize,
					(PVOID)&NewClassNameCell,
					&NewClassNameOffset);
	      if (!NT_SUCCESS(Status))
		{
		  DPRINT1 ("CmiAllocateBlock() failed (Status %lx)\n", Status);
		  return Status;
		}

	      NewSubKeyCell->ClassNameOffset = NewClassNameOffset;
	      RtlCopyMemory (NewClassNameCell,
			     SrcClassNameCell,
			     NewSubKeyCell->ClassSize);
	    }

	  /* Copy subkey data and subkeys */
	  Status = CmiCopyKey (DstHive,
			       NewSubKeyCell,
			       SrcHive,
			       SrcSubKeyCell);
	  if (!NT_SUCCESS(Status))
	    {
	      DPRINT1 ("CmiAllocateBlock() failed (Status %lx)\n", Status);
	      return Status;
	    }
	}
    }

  return STATUS_SUCCESS;
}


NTSTATUS
CmiSaveTempHive (PREGISTRY_HIVE Hive,
		 HANDLE FileHandle)
{
  IO_STATUS_BLOCK IoStatusBlock;
  LARGE_INTEGER FileOffset;
  ULONG BlockIndex;
  PVOID BlockPtr;
  NTSTATUS Status;

  DPRINT ("CmiSaveTempHive() called\n");

  Hive->HiveHeader->Checksum = CmiCalcChecksum ((PULONG)Hive->HiveHeader);

  /* Write hive block */
  FileOffset.QuadPart = (ULONGLONG)0;
  Status = ZwWriteFile (FileHandle,
			NULL,
			NULL,
			NULL,
			&IoStatusBlock,
			Hive->HiveHeader,
			sizeof(HIVE_HEADER),
			&FileOffset,
			NULL);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("ZwWriteFile() failed (Status %lx)\n", Status);
      return Status;
    }

  DPRINT ("Saving %lu blocks\n", Hive->BlockListSize);
  for (BlockIndex = 0; BlockIndex < Hive->BlockListSize; BlockIndex++)
    {
      BlockPtr = Hive->BlockList[BlockIndex].Block;
      DPRINT ("BlockPtr %p\n", BlockPtr);

      FileOffset.QuadPart = (ULONGLONG)(BlockIndex + 1) * (ULONGLONG)REG_BLOCK_SIZE;
      DPRINT ("File offset %I64x\n", FileOffset.QuadPart);

      /* Write hive block */
      Status = ZwWriteFile (FileHandle,
			    NULL,
			    NULL,
			    NULL,
			    &IoStatusBlock,
			    BlockPtr,
			    REG_BLOCK_SIZE,
			    &FileOffset,
			    NULL);
      if (!NT_SUCCESS(Status))
	{
	  DPRINT1 ("ZwWriteFile() failed (Status %lx)\n", Status);
	  return Status;
	}
    }

  Status = ZwFlushBuffersFile (FileHandle,
			       &IoStatusBlock);
  if (!NT_SUCCESS(Status))
    {
      DPRINT1 ("ZwFlushBuffersFile() failed (Status %lx)\n", Status);
    }

  DPRINT ("CmiSaveTempHive() done\n");

  return Status;
}

/* EOF */
