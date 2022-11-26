/* FILE:            System Call Table for Native API
 * COPYRIGHT:       See COPYING in the top level directory
 * PURPOSE:         ../../include/ntdll/napi.h
 * PROGRAMMER:      Computer Generated File. See tools/nci/ncitool.c
 * REMARK:          DO NOT EDIT OR COMMIT MODIFICATIONS TO THIS FILE
 */


#include <napi/shared_data.h>




SSDT MainSSDT[] = {
		(PVOID (NTAPI *)(VOID))NtAcceptConnectPort,
		(PVOID (NTAPI *)(VOID))NtAccessCheck,
		(PVOID (NTAPI *)(VOID))NtAccessCheckAndAuditAlarm,
		(PVOID (NTAPI *)(VOID))NtAddAtom,
		(PVOID (NTAPI *)(VOID))NtAddBootEntry,
		(PVOID (NTAPI *)(VOID))NtAdjustGroupsToken,
		(PVOID (NTAPI *)(VOID))NtAdjustPrivilegesToken,
		(PVOID (NTAPI *)(VOID))NtAlertResumeThread,
		(PVOID (NTAPI *)(VOID))NtAlertThread,
		(PVOID (NTAPI *)(VOID))NtAllocateLocallyUniqueId,
		(PVOID (NTAPI *)(VOID))NtAllocateUuids,
		(PVOID (NTAPI *)(VOID))NtAllocateVirtualMemory,
		(PVOID (NTAPI *)(VOID))NtAssignProcessToJobObject,
		(PVOID (NTAPI *)(VOID))NtCallbackReturn,
		(PVOID (NTAPI *)(VOID))NtCancelIoFile,
		(PVOID (NTAPI *)(VOID))NtCancelTimer,
		(PVOID (NTAPI *)(VOID))NtClearEvent,
		(PVOID (NTAPI *)(VOID))NtClose,
		(PVOID (NTAPI *)(VOID))NtCloseObjectAuditAlarm,
		(PVOID (NTAPI *)(VOID))NtCompleteConnectPort,
		(PVOID (NTAPI *)(VOID))NtConnectPort,
		(PVOID (NTAPI *)(VOID))NtContinue,
		(PVOID (NTAPI *)(VOID))NtCreateDirectoryObject,
		(PVOID (NTAPI *)(VOID))NtCreateEvent,
		(PVOID (NTAPI *)(VOID))NtCreateEventPair,
		(PVOID (NTAPI *)(VOID))NtCreateFile,
		(PVOID (NTAPI *)(VOID))NtCreateIoCompletion,
		(PVOID (NTAPI *)(VOID))NtCreateJobObject,
		(PVOID (NTAPI *)(VOID))NtCreateKey,
		(PVOID (NTAPI *)(VOID))NtCreateMailslotFile,
		(PVOID (NTAPI *)(VOID))NtCreateMutant,
		(PVOID (NTAPI *)(VOID))NtCreateNamedPipeFile,
		(PVOID (NTAPI *)(VOID))NtCreatePagingFile,
		(PVOID (NTAPI *)(VOID))NtCreatePort,
		(PVOID (NTAPI *)(VOID))NtCreateProcess,
		(PVOID (NTAPI *)(VOID))NtCreateProfile,
		(PVOID (NTAPI *)(VOID))NtCreateSection,
		(PVOID (NTAPI *)(VOID))NtCreateSemaphore,
		(PVOID (NTAPI *)(VOID))NtCreateSymbolicLinkObject,
		(PVOID (NTAPI *)(VOID))NtCreateThread,
		(PVOID (NTAPI *)(VOID))NtCreateTimer,
		(PVOID (NTAPI *)(VOID))NtCreateToken,
		(PVOID (NTAPI *)(VOID))NtCreateWaitablePort,
		(PVOID (NTAPI *)(VOID))NtDelayExecution,
		(PVOID (NTAPI *)(VOID))NtDeleteAtom,
		(PVOID (NTAPI *)(VOID))NtDeleteBootEntry,
		(PVOID (NTAPI *)(VOID))NtDeleteFile,
		(PVOID (NTAPI *)(VOID))NtDeleteKey,
		(PVOID (NTAPI *)(VOID))NtDeleteObjectAuditAlarm,
		(PVOID (NTAPI *)(VOID))NtDeleteValueKey,
		(PVOID (NTAPI *)(VOID))NtDeviceIoControlFile,
		(PVOID (NTAPI *)(VOID))NtDisplayString,
		(PVOID (NTAPI *)(VOID))NtDuplicateObject,
		(PVOID (NTAPI *)(VOID))NtDuplicateToken,
		(PVOID (NTAPI *)(VOID))NtEnumerateBootEntries,
		(PVOID (NTAPI *)(VOID))NtEnumerateKey,
		(PVOID (NTAPI *)(VOID))NtEnumerateValueKey,
		(PVOID (NTAPI *)(VOID))NtExtendSection,
		(PVOID (NTAPI *)(VOID))NtFindAtom,
		(PVOID (NTAPI *)(VOID))NtFlushBuffersFile,
		(PVOID (NTAPI *)(VOID))NtFlushInstructionCache,
		(PVOID (NTAPI *)(VOID))NtFlushKey,
		(PVOID (NTAPI *)(VOID))NtFlushVirtualMemory,
		(PVOID (NTAPI *)(VOID))NtFlushWriteBuffer,
		(PVOID (NTAPI *)(VOID))NtFreeVirtualMemory,
		(PVOID (NTAPI *)(VOID))NtFsControlFile,
		(PVOID (NTAPI *)(VOID))NtGetContextThread,
		(PVOID (NTAPI *)(VOID))NtGetPlugPlayEvent,
		(PVOID (NTAPI *)(VOID))NtGetTickCount,
		(PVOID (NTAPI *)(VOID))NtImpersonateClientOfPort,
		(PVOID (NTAPI *)(VOID))NtImpersonateThread,
		(PVOID (NTAPI *)(VOID))NtInitializeRegistry,
		(PVOID (NTAPI *)(VOID))NtInitiatePowerAction,
		(PVOID (NTAPI *)(VOID))NtIsProcessInJob,
		(PVOID (NTAPI *)(VOID))NtListenPort,
		(PVOID (NTAPI *)(VOID))NtLoadDriver,
		(PVOID (NTAPI *)(VOID))NtLoadKey,
		(PVOID (NTAPI *)(VOID))NtLoadKey2,
		(PVOID (NTAPI *)(VOID))NtLockFile,
		(PVOID (NTAPI *)(VOID))NtLockVirtualMemory,
		(PVOID (NTAPI *)(VOID))NtMakePermanentObject,
		(PVOID (NTAPI *)(VOID))NtMakeTemporaryObject,
		(PVOID (NTAPI *)(VOID))NtMapViewOfSection,
		(PVOID (NTAPI *)(VOID))NtNotifyChangeDirectoryFile,
		(PVOID (NTAPI *)(VOID))NtNotifyChangeKey,
		(PVOID (NTAPI *)(VOID))NtOpenDirectoryObject,
		(PVOID (NTAPI *)(VOID))NtOpenEvent,
		(PVOID (NTAPI *)(VOID))NtOpenEventPair,
		(PVOID (NTAPI *)(VOID))NtOpenFile,
		(PVOID (NTAPI *)(VOID))NtOpenIoCompletion,
		(PVOID (NTAPI *)(VOID))NtOpenJobObject,
		(PVOID (NTAPI *)(VOID))NtOpenKey,
		(PVOID (NTAPI *)(VOID))NtOpenMutant,
		(PVOID (NTAPI *)(VOID))NtOpenObjectAuditAlarm,
		(PVOID (NTAPI *)(VOID))NtOpenProcess,
		(PVOID (NTAPI *)(VOID))NtOpenProcessToken,
		(PVOID (NTAPI *)(VOID))NtOpenProcessTokenEx,
		(PVOID (NTAPI *)(VOID))NtOpenSection,
		(PVOID (NTAPI *)(VOID))NtOpenSemaphore,
		(PVOID (NTAPI *)(VOID))NtOpenSymbolicLinkObject,
		(PVOID (NTAPI *)(VOID))NtOpenThread,
		(PVOID (NTAPI *)(VOID))NtOpenThreadToken,
		(PVOID (NTAPI *)(VOID))NtOpenThreadTokenEx,
		(PVOID (NTAPI *)(VOID))NtOpenTimer,
		(PVOID (NTAPI *)(VOID))NtPlugPlayControl,
		(PVOID (NTAPI *)(VOID))NtPowerInformation,
		(PVOID (NTAPI *)(VOID))NtPrivilegeCheck,
		(PVOID (NTAPI *)(VOID))NtPrivilegedServiceAuditAlarm,
		(PVOID (NTAPI *)(VOID))NtPrivilegeObjectAuditAlarm,
		(PVOID (NTAPI *)(VOID))NtProtectVirtualMemory,
		(PVOID (NTAPI *)(VOID))NtPulseEvent,
		(PVOID (NTAPI *)(VOID))NtQueryInformationAtom,
		(PVOID (NTAPI *)(VOID))NtQueryAttributesFile,
		(PVOID (NTAPI *)(VOID))NtQueryBootEntryOrder,
		(PVOID (NTAPI *)(VOID))NtQueryBootOptions,
		(PVOID (NTAPI *)(VOID))NtQueryDefaultLocale,
		(PVOID (NTAPI *)(VOID))NtQueryDefaultUILanguage,
		(PVOID (NTAPI *)(VOID))NtQueryDirectoryFile,
		(PVOID (NTAPI *)(VOID))NtQueryDirectoryObject,
		(PVOID (NTAPI *)(VOID))NtQueryEaFile,
		(PVOID (NTAPI *)(VOID))NtQueryEvent,
		(PVOID (NTAPI *)(VOID))NtQueryFullAttributesFile,
		(PVOID (NTAPI *)(VOID))NtQueryInformationFile,
		(PVOID (NTAPI *)(VOID))NtQueryInformationJobObject,
		(PVOID (NTAPI *)(VOID))NtQueryInformationPort,
		(PVOID (NTAPI *)(VOID))NtQueryInformationProcess,
		(PVOID (NTAPI *)(VOID))NtQueryInformationThread,
		(PVOID (NTAPI *)(VOID))NtQueryInformationToken,
		(PVOID (NTAPI *)(VOID))NtQueryInstallUILanguage,
		(PVOID (NTAPI *)(VOID))NtQueryIntervalProfile,
		(PVOID (NTAPI *)(VOID))NtQueryIoCompletion,
		(PVOID (NTAPI *)(VOID))NtQueryKey,
		(PVOID (NTAPI *)(VOID))NtQueryMultipleValueKey,
		(PVOID (NTAPI *)(VOID))NtQueryMutant,
		(PVOID (NTAPI *)(VOID))NtQueryObject,
		(PVOID (NTAPI *)(VOID))NtQueryPerformanceCounter,
		(PVOID (NTAPI *)(VOID))NtQueryQuotaInformationFile,
		(PVOID (NTAPI *)(VOID))NtQuerySection,
		(PVOID (NTAPI *)(VOID))NtQuerySecurityObject,
		(PVOID (NTAPI *)(VOID))NtQuerySemaphore,
		(PVOID (NTAPI *)(VOID))NtQuerySymbolicLinkObject,
		(PVOID (NTAPI *)(VOID))NtQuerySystemEnvironmentValue,
		(PVOID (NTAPI *)(VOID))NtQuerySystemInformation,
		(PVOID (NTAPI *)(VOID))NtQuerySystemTime,
		(PVOID (NTAPI *)(VOID))NtQueryTimer,
		(PVOID (NTAPI *)(VOID))NtQueryTimerResolution,
		(PVOID (NTAPI *)(VOID))NtQueryValueKey,
		(PVOID (NTAPI *)(VOID))NtQueryVirtualMemory,
		(PVOID (NTAPI *)(VOID))NtQueryVolumeInformationFile,
		(PVOID (NTAPI *)(VOID))NtQueueApcThread,
		(PVOID (NTAPI *)(VOID))NtRaiseException,
		(PVOID (NTAPI *)(VOID))NtRaiseHardError,
		(PVOID (NTAPI *)(VOID))NtReadFile,
		(PVOID (NTAPI *)(VOID))NtReadFileScatter,
		(PVOID (NTAPI *)(VOID))NtReadRequestData,
		(PVOID (NTAPI *)(VOID))NtReadVirtualMemory,
		(PVOID (NTAPI *)(VOID))NtRegisterThreadTerminatePort,
		(PVOID (NTAPI *)(VOID))NtReleaseMutant,
		(PVOID (NTAPI *)(VOID))NtReleaseSemaphore,
		(PVOID (NTAPI *)(VOID))NtRemoveIoCompletion,
		(PVOID (NTAPI *)(VOID))NtReplaceKey,
		(PVOID (NTAPI *)(VOID))NtReplyPort,
		(PVOID (NTAPI *)(VOID))NtReplyWaitReceivePort,
		(PVOID (NTAPI *)(VOID))NtReplyWaitReplyPort,
		(PVOID (NTAPI *)(VOID))NtRequestPort,
		(PVOID (NTAPI *)(VOID))NtRequestWaitReplyPort,
		(PVOID (NTAPI *)(VOID))NtResetEvent,
		(PVOID (NTAPI *)(VOID))NtRestoreKey,
		(PVOID (NTAPI *)(VOID))NtResumeThread,
		(PVOID (NTAPI *)(VOID))NtSaveKey,
		(PVOID (NTAPI *)(VOID))NtSaveKeyEx,
		(PVOID (NTAPI *)(VOID))NtSetBootEntryOrder,
		(PVOID (NTAPI *)(VOID))NtSetBootOptions,
		(PVOID (NTAPI *)(VOID))NtSetIoCompletion,
		(PVOID (NTAPI *)(VOID))NtSetContextThread,
		(PVOID (NTAPI *)(VOID))NtSetDefaultHardErrorPort,
		(PVOID (NTAPI *)(VOID))NtSetDefaultLocale,
		(PVOID (NTAPI *)(VOID))NtSetDefaultUILanguage,
		(PVOID (NTAPI *)(VOID))NtSetEaFile,
		(PVOID (NTAPI *)(VOID))NtSetEvent,
		(PVOID (NTAPI *)(VOID))NtSetHighEventPair,
		(PVOID (NTAPI *)(VOID))NtSetHighWaitLowEventPair,
		(PVOID (NTAPI *)(VOID))NtSetInformationFile,
		(PVOID (NTAPI *)(VOID))NtSetInformationKey,
		(PVOID (NTAPI *)(VOID))NtSetInformationJobObject,
		(PVOID (NTAPI *)(VOID))NtSetInformationObject,
		(PVOID (NTAPI *)(VOID))NtSetInformationProcess,
		(PVOID (NTAPI *)(VOID))NtSetInformationThread,
		(PVOID (NTAPI *)(VOID))NtSetInformationToken,
		(PVOID (NTAPI *)(VOID))NtSetIntervalProfile,
		(PVOID (NTAPI *)(VOID))NtSetLdtEntries,
		(PVOID (NTAPI *)(VOID))NtSetLowEventPair,
		(PVOID (NTAPI *)(VOID))NtSetLowWaitHighEventPair,
		(PVOID (NTAPI *)(VOID))NtSetQuotaInformationFile,
		(PVOID (NTAPI *)(VOID))NtSetSecurityObject,
		(PVOID (NTAPI *)(VOID))NtSetSystemEnvironmentValue,
		(PVOID (NTAPI *)(VOID))NtSetSystemInformation,
		(PVOID (NTAPI *)(VOID))NtSetSystemPowerState,
		(PVOID (NTAPI *)(VOID))NtSetSystemTime,
		(PVOID (NTAPI *)(VOID))NtSetTimer,
		(PVOID (NTAPI *)(VOID))NtSetTimerResolution,
		(PVOID (NTAPI *)(VOID))NtSetUuidSeed,
		(PVOID (NTAPI *)(VOID))NtSetValueKey,
		(PVOID (NTAPI *)(VOID))NtSetVolumeInformationFile,
		(PVOID (NTAPI *)(VOID))NtShutdownSystem,
		(PVOID (NTAPI *)(VOID))NtSignalAndWaitForSingleObject,
		(PVOID (NTAPI *)(VOID))NtStartProfile,
		(PVOID (NTAPI *)(VOID))NtStopProfile,
		(PVOID (NTAPI *)(VOID))NtSuspendThread,
		(PVOID (NTAPI *)(VOID))NtSystemDebugControl,
		(PVOID (NTAPI *)(VOID))NtTerminateJobObject,
		(PVOID (NTAPI *)(VOID))NtTerminateProcess,
		(PVOID (NTAPI *)(VOID))NtTerminateThread,
		(PVOID (NTAPI *)(VOID))NtTestAlert,
		(PVOID (NTAPI *)(VOID))NtTraceEvent,
		(PVOID (NTAPI *)(VOID))NtTranslateFilePath,
		(PVOID (NTAPI *)(VOID))NtUnloadDriver,
		(PVOID (NTAPI *)(VOID))NtUnloadKey,
		(PVOID (NTAPI *)(VOID))NtUnlockFile,
		(PVOID (NTAPI *)(VOID))NtUnlockVirtualMemory,
		(PVOID (NTAPI *)(VOID))NtUnmapViewOfSection,
		(PVOID (NTAPI *)(VOID))NtVdmControl,
		(PVOID (NTAPI *)(VOID))NtWaitForMultipleObjects,
		(PVOID (NTAPI *)(VOID))NtWaitForSingleObject,
		(PVOID (NTAPI *)(VOID))NtWaitHighEventPair,
		(PVOID (NTAPI *)(VOID))NtWaitLowEventPair,
		(PVOID (NTAPI *)(VOID))NtWriteFile,
		(PVOID (NTAPI *)(VOID))NtWriteFileGather,
		(PVOID (NTAPI *)(VOID))NtWriteRequestData,
		(PVOID (NTAPI *)(VOID))NtWriteVirtualMemory,
		(PVOID (NTAPI *)(VOID))NtW32Call,
		(PVOID (NTAPI *)(VOID))NtYieldExecution
};



SSPT MainSSPT[] = {
		6 * sizeof(void *),
		8 * sizeof(void *),
		11 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		6 * sizeof(void *),
		6 * sizeof(void *),
		2 * sizeof(void *),
		1 * sizeof(void *),
		1 * sizeof(void *),
		4 * sizeof(void *),
		6 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		1 * sizeof(void *),
		1 * sizeof(void *),
		3 * sizeof(void *),
		1 * sizeof(void *),
		8 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		5 * sizeof(void *),
		3 * sizeof(void *),
		11 * sizeof(void *),
		4 * sizeof(void *),
		3 * sizeof(void *),
		7 * sizeof(void *),
		8 * sizeof(void *),
		4 * sizeof(void *),
		14 * sizeof(void *),
		4 * sizeof(void *),
		5 * sizeof(void *),
		8 * sizeof(void *),
		9 * sizeof(void *),
		7 * sizeof(void *),
		5 * sizeof(void *),
		4 * sizeof(void *),
		8 * sizeof(void *),
		4 * sizeof(void *),
		13 * sizeof(void *),
		5 * sizeof(void *),
		2 * sizeof(void *),
		1 * sizeof(void *),
		2 * sizeof(void *),
		1 * sizeof(void *),
		1 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		10 * sizeof(void *),
		1 * sizeof(void *),
		7 * sizeof(void *),
		6 * sizeof(void *),
		2 * sizeof(void *),
		6 * sizeof(void *),
		6 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		1 * sizeof(void *),
		4 * sizeof(void *),
		0 * sizeof(void *),
		4 * sizeof(void *),
		10 * sizeof(void *),
		2 * sizeof(void *),
		4 * sizeof(void *),
		0 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		1 * sizeof(void *),
		4 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		1 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		10 * sizeof(void *),
		4 * sizeof(void *),
		1 * sizeof(void *),
		1 * sizeof(void *),
		10 * sizeof(void *),
		9 * sizeof(void *),
		10 * sizeof(void *),
		3 * sizeof(void *),
		3 * sizeof(void *),
		3 * sizeof(void *),
		6 * sizeof(void *),
		3 * sizeof(void *),
		3 * sizeof(void *),
		3 * sizeof(void *),
		3 * sizeof(void *),
		12 * sizeof(void *),
		4 * sizeof(void *),
		3 * sizeof(void *),
		4 * sizeof(void *),
		3 * sizeof(void *),
		3 * sizeof(void *),
		3 * sizeof(void *),
		4 * sizeof(void *),
		4 * sizeof(void *),
		5 * sizeof(void *),
		3 * sizeof(void *),
		3 * sizeof(void *),
		5 * sizeof(void *),
		3 * sizeof(void *),
		5 * sizeof(void *),
		6 * sizeof(void *),
		5 * sizeof(void *),
		2 * sizeof(void *),
		5 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		1 * sizeof(void *),
		11 * sizeof(void *),
		7 * sizeof(void *),
		9 * sizeof(void *),
		5 * sizeof(void *),
		2 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		1 * sizeof(void *),
		2 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		6 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		2 * sizeof(void *),
		9 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		3 * sizeof(void *),
		4 * sizeof(void *),
		4 * sizeof(void *),
		1 * sizeof(void *),
		5 * sizeof(void *),
		3 * sizeof(void *),
		6 * sizeof(void *),
		6 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		3 * sizeof(void *),
		6 * sizeof(void *),
		9 * sizeof(void *),
		9 * sizeof(void *),
		6 * sizeof(void *),
		5 * sizeof(void *),
		1 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		5 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		4 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		5 * sizeof(void *),
		2 * sizeof(void *),
		1 * sizeof(void *),
		2 * sizeof(void *),
		1 * sizeof(void *),
		4 * sizeof(void *),
		2 * sizeof(void *),
		1 * sizeof(void *),
		1 * sizeof(void *),
		5 * sizeof(void *),
		4 * sizeof(void *),
		4 * sizeof(void *),
		4 * sizeof(void *),
		4 * sizeof(void *),
		4 * sizeof(void *),
		4 * sizeof(void *),
		2 * sizeof(void *),
		6 * sizeof(void *),
		1 * sizeof(void *),
		1 * sizeof(void *),
		4 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		3 * sizeof(void *),
		3 * sizeof(void *),
		2 * sizeof(void *),
		7 * sizeof(void *),
		3 * sizeof(void *),
		1 * sizeof(void *),
		6 * sizeof(void *),
		5 * sizeof(void *),
		1 * sizeof(void *),
		4 * sizeof(void *),
		1 * sizeof(void *),
		1 * sizeof(void *),
		2 * sizeof(void *),
		6 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		0 * sizeof(void *),
		4 * sizeof(void *),
		3 * sizeof(void *),
		1 * sizeof(void *),
		1 * sizeof(void *),
		5 * sizeof(void *),
		4 * sizeof(void *),
		2 * sizeof(void *),
		2 * sizeof(void *),
		5 * sizeof(void *),
		3 * sizeof(void *),
		1 * sizeof(void *),
		1 * sizeof(void *),
		9 * sizeof(void *),
		9 * sizeof(void *),
		6 * sizeof(void *),
		5 * sizeof(void *),
		5 * sizeof(void *),
		0 * sizeof(void *)
};


#define MIN_SYSCALL_NUMBER    0
#define MAX_SYSCALL_NUMBER    231
#define NUMBER_OF_SYSCALLS    232
ULONG MainNumberOfSysCalls = 232;