/* $Id: locale.c 13714 2005-02-22 19:25:17Z weiden $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ps/locale.c
 * PURPOSE:         Locale support
 *
 * PROGRAMMERS:     David Welch (welch@cwcom.net)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>

#define NDEBUG
#include <internal/debug.h>


/* GLOBALS *******************************************************************/

/*
 * Default setting: LANG_NEUTRAL, SUBLANG_NEUTRAL, SORT_DEFAULT
 */
LCID PsDefaultThreadLocaleId = 0;
LCID PsDefaultSystemLocaleId = 0;
BOOL PsDefaultThreadLocaleInitialized = FALSE;

static LANGID PsInstallUILanguageId = 0;

#define VALUE_BUFFER_SIZE 256

/* FUNCTIONS *****************************************************************/

/*
 * FUNCTION:
 *    Initializes the default locale.
 *    Reads default locale from registry, if available
 * ARGUMENTS:
 *    None.
 * Returns:
 *    None.
 */
VOID INIT_FUNCTION
PiInitDefaultLocale(VOID)
{
   OBJECT_ATTRIBUTES ObjectAttributes;
   UNICODE_STRING KeyName;
   UNICODE_STRING ValueName;
   HANDLE KeyHandle;
   ULONG ValueLength;
   UCHAR ValueBuffer[VALUE_BUFFER_SIZE];
   PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
   UNICODE_STRING ValueString;
   ULONG Value;
   NTSTATUS Status;

   ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;

   RtlRosInitUnicodeStringFromLiteral(&KeyName,
			L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Language");

   InitializeObjectAttributes(&ObjectAttributes,
			      &KeyName,
			      OBJ_CASE_INSENSITIVE,
			      NULL,
			      NULL);
   Status = ZwOpenKey(&KeyHandle,
		      KEY_QUERY_VALUE,
		      &ObjectAttributes);
   if (NT_SUCCESS(Status))
     {
	/* Read system locale */
	RtlRosInitUnicodeStringFromLiteral(&ValueName,
					   L"Default");
	Status = ZwQueryValueKey(KeyHandle,
				 &ValueName,
				 KeyValuePartialInformation,
				 ValueBuffer,
				 VALUE_BUFFER_SIZE,
				 &ValueLength);
	if ((NT_SUCCESS(Status)) && (ValueInfo->Type == REG_SZ))
	  {
	     ValueString.Length = ValueInfo->DataLength;
	     ValueString.MaximumLength = ValueInfo->DataLength;
	     ValueString.Buffer = (PWSTR)ValueInfo->Data;

	     Status = RtlUnicodeStringToInteger(&ValueString,
						16,
						&Value);
	     if (NT_SUCCESS(Status))
	       {
		  DPRINT("System locale: %08lx\n", Value);
		  PsDefaultSystemLocaleId = (LCID)Value;
	       }
	  }

	/* Read install language id */
	RtlRosInitUnicodeStringFromLiteral(&ValueName,
					   L"InstallLanguage");
	Status = ZwQueryValueKey(KeyHandle,
				 &ValueName,
				 KeyValuePartialInformation,
				 ValueBuffer,
				 VALUE_BUFFER_SIZE,
				 &ValueLength);
	if ((NT_SUCCESS(Status)) && (ValueInfo->Type == REG_SZ))
	  {
	     ValueString.Length = ValueInfo->DataLength;
	     ValueString.MaximumLength = ValueInfo->DataLength;
	     ValueString.Buffer = (PWSTR)ValueInfo->Data;

	     Status = RtlUnicodeStringToInteger(&ValueString,
						16,
						&Value);
	     if (NT_SUCCESS(Status))
	       {
		  DPRINT("Install language id: %04lx\n", Value);
		  PsInstallUILanguageId = (LANGID)Value;
	       }
	  }

	ZwClose(KeyHandle);
     }
}


/*
 * FUNCTION:
 *    Initializes the default thread locale.
 *    Reads default locale from registry, if available
 * ARGUMENTS:
 *    None.
 * Returns:
 *    None.
 */
VOID STDCALL
PiInitThreadLocale(VOID)
{
   OBJECT_ATTRIBUTES ObjectAttributes;
   UNICODE_STRING KeyName;
   UNICODE_STRING ValueName;
   HANDLE KeyHandle;
   ULONG ValueLength;
   UCHAR ValueBuffer[VALUE_BUFFER_SIZE];
   PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
   UNICODE_STRING ValueString;
   ULONG LocaleValue;
   NTSTATUS Status;

   ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;

   /* read default thread locale */
   RtlRosInitUnicodeStringFromLiteral(&KeyName,
			L"\\Registry\\User\\.Default\\Control Panel\\International");
   RtlRosInitUnicodeStringFromLiteral(&ValueName,
			L"Locale");

   InitializeObjectAttributes(&ObjectAttributes,
			      &KeyName,
			      OBJ_CASE_INSENSITIVE,
			      NULL,
			      NULL);
   Status = ZwOpenKey(&KeyHandle,
		      KEY_QUERY_VALUE,
		      &ObjectAttributes);
   if (NT_SUCCESS(Status))
     {
	Status = ZwQueryValueKey(KeyHandle,
				 &ValueName,
				 KeyValuePartialInformation,
				 ValueBuffer,
				 VALUE_BUFFER_SIZE,
				 &ValueLength);
	if ((NT_SUCCESS(Status)) && (ValueInfo->Type == REG_SZ))
	  {
	     ValueString.Length = ValueInfo->DataLength;
	     ValueString.MaximumLength = ValueInfo->DataLength;
	     ValueString.Buffer = (PWSTR)ValueInfo->Data;

	     Status = RtlUnicodeStringToInteger(&ValueString,
						16,
						&LocaleValue);
	     if (NT_SUCCESS(Status))
	       {
		  DPRINT("Thread locale: %08lu\n", LocaleValue);
		  PsDefaultThreadLocaleId = (LCID)LocaleValue;
	       }
	  }
	ZwClose(KeyHandle);
     }

   PsDefaultThreadLocaleInitialized = TRUE;
}


/*
 * FUNCTION:
 *    Returns the default locale.
 * ARGUMENTS:
 *    UserProfile = If TRUE then the locale for this thread is returned,
 *                  otherwise the locale for the system is returned.
 *    DefaultLocaleId = Points to a variable that receives the locale id.
 * Returns:
 *    Status.
 */
NTSTATUS STDCALL
NtQueryDefaultLocale(IN BOOLEAN UserProfile,
		     OUT PLCID DefaultLocaleId)
{
  PAGED_CODE();

  if (DefaultLocaleId == NULL)
    return STATUS_UNSUCCESSFUL;

  if (UserProfile)
    {
      if (!PsDefaultThreadLocaleInitialized)
	{
	  PiInitThreadLocale();
	}

      /* set thread locale */
      *DefaultLocaleId = PsDefaultThreadLocaleId;
    }
  else
    {
      /* set system locale */
      *DefaultLocaleId = PsDefaultSystemLocaleId;
    }

  return STATUS_SUCCESS;
}


/*
 * FUNCTION:
 *    Sets the default locale.
 * ARGUMENTS:
 *    ThreadOrSystem = If TRUE then the locale for this thread is set,
 *                     otherwise the locale for the system is set.
 *    DefaultLocaleId = The locale id to be set.
 * Returns:
 *    Status.
 */
NTSTATUS STDCALL
NtSetDefaultLocale(IN BOOLEAN UserProfile,
		   IN LCID DefaultLocaleId)
{
   OBJECT_ATTRIBUTES ObjectAttributes;
   UNICODE_STRING KeyName;
   UNICODE_STRING ValueName;
   HANDLE KeyHandle;
   ULONG ValueLength;
   WCHAR ValueBuffer[20];
   HANDLE UserKey = NULL;
   NTSTATUS Status;
   
   PAGED_CODE();

   if (UserProfile)
     {
	/* thread locale */
	Status = RtlOpenCurrentUser(KEY_WRITE,
				    &UserKey);
	if (!NT_SUCCESS(Status))
	  return(Status);
	RtlRosInitUnicodeStringFromLiteral(&KeyName,
			     L"Control Panel\\International");
	RtlRosInitUnicodeStringFromLiteral(&ValueName,
			     L"Locale");
     }
   else
     {
	/* system locale */
	RtlRosInitUnicodeStringFromLiteral(&KeyName,
			     L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\Language");
	RtlRosInitUnicodeStringFromLiteral(&ValueName,
			     L"Default");
     }

   InitializeObjectAttributes(&ObjectAttributes,
			      &KeyName,
			      OBJ_CASE_INSENSITIVE,
			      UserKey,
			      NULL);
   Status = ZwOpenKey(&KeyHandle,
		      KEY_SET_VALUE,
		      &ObjectAttributes);
   if (!NT_SUCCESS(Status))
     {
	if (UserKey != NULL)
	  {
	     ZwClose(UserKey);
	  }
	return(Status);
     }

   if (UserProfile)
     {
	ValueLength = swprintf(ValueBuffer,
			       L"%08lx",
			       (ULONG)DefaultLocaleId);
     }
   else
     {
	ValueLength = swprintf(ValueBuffer,
			       L"%04lx",
			       (ULONG)DefaultLocaleId & 0xFFFF);
     }
   ValueLength = (ValueLength + 1) * sizeof(WCHAR);

   Status = ZwSetValueKey(KeyHandle,
			  &ValueName,
			  0,
			  REG_SZ,
			  ValueBuffer,
			  ValueLength);

   ZwClose(KeyHandle);
   if (UserKey != NULL)
     {
	ZwClose(UserKey);
     }

   if (!NT_SUCCESS(Status))
     {
	return(Status);
     }

   if (UserProfile)
     {
	/* set thread locale */
	DPRINT("Thread locale: %08lu\n", DefaultLocaleId);
	PsDefaultThreadLocaleId = DefaultLocaleId;
	PsDefaultThreadLocaleInitialized = TRUE;
     }
   else
     {
	/* set system locale */
	DPRINT("System locale: %08lu\n", DefaultLocaleId);
	PsDefaultSystemLocaleId = DefaultLocaleId;
     }

   return STATUS_SUCCESS;
}


/*
 * @implemented
 */
NTSTATUS STDCALL
NtQueryDefaultUILanguage(OUT PLANGID LanguageId)
{
  UCHAR ValueBuffer[VALUE_BUFFER_SIZE];
  PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
  OBJECT_ATTRIBUTES ObjectAttributes;
  UNICODE_STRING KeyName;
  UNICODE_STRING ValueName;
  UNICODE_STRING ValueString;
  ULONG ValueLength;
  ULONG Value;
  HANDLE UserKey;
  HANDLE KeyHandle;
  NTSTATUS Status;
  
  PAGED_CODE();

  Status = RtlOpenCurrentUser(KEY_READ,
			      &UserKey);
  if (!NT_SUCCESS(Status))
    {
      *LanguageId = PsInstallUILanguageId;
      return STATUS_SUCCESS;
    }

  RtlRosInitUnicodeStringFromLiteral(&KeyName,
				     L"Control Panel\\International");
  RtlRosInitUnicodeStringFromLiteral(&ValueName,
				     L"MultiUILanguageId");

  InitializeObjectAttributes(&ObjectAttributes,
			     &KeyName,
			     OBJ_CASE_INSENSITIVE,
			     UserKey,
			     NULL);
  Status = ZwOpenKey(&KeyHandle,
		     KEY_QUERY_VALUE,
		     &ObjectAttributes);
  if (!NT_SUCCESS(Status))
    {
      *LanguageId = PsInstallUILanguageId;
      return STATUS_SUCCESS;
    }

  ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;

  Status = ZwQueryValueKey(KeyHandle,
			   &ValueName,
			   KeyValuePartialInformation,
			   ValueBuffer,
			   VALUE_BUFFER_SIZE,
			   &ValueLength);

  ZwClose(KeyHandle);
  ZwClose(UserKey);

  if (!NT_SUCCESS(Status) || ValueInfo->Type != REG_SZ)
    {
      *LanguageId = PsInstallUILanguageId;
      return STATUS_SUCCESS;
    }

  ValueString.Length = ValueInfo->DataLength;
  ValueString.MaximumLength = ValueInfo->DataLength;
  ValueString.Buffer = (PWSTR)ValueInfo->Data;

  Status = RtlUnicodeStringToInteger(&ValueString,
				     16,
				     &Value);
  if (!NT_SUCCESS(Status))
    {
      *LanguageId = PsInstallUILanguageId;
      return STATUS_SUCCESS;
    }

  DPRINT("Default language id: %04lx\n", Value);

  *LanguageId = Value;

  return STATUS_SUCCESS;
}


/*
 * @implemented
 */
NTSTATUS STDCALL
NtQueryInstallUILanguage(OUT PLANGID LanguageId)
{
  PAGED_CODE();
  
  *LanguageId = PsInstallUILanguageId;

  return STATUS_SUCCESS;
}


/*
 * @implemented
 */
NTSTATUS STDCALL
NtSetDefaultUILanguage(IN LANGID LanguageId)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  UNICODE_STRING KeyName;
  UNICODE_STRING ValueName;
  WCHAR ValueBuffer[8];
  ULONG ValueLength;
  HANDLE UserHandle;
  HANDLE KeyHandle;
  NTSTATUS Status;
  
  PAGED_CODE();

  Status = RtlOpenCurrentUser(KEY_WRITE,
			      &UserHandle);
  if (!NT_SUCCESS(Status))
    {
      return Status;
    }

  RtlRosInitUnicodeStringFromLiteral(&KeyName,
				     L"Control Panel\\Desktop");
  RtlRosInitUnicodeStringFromLiteral(&ValueName,
				     L"MultiUILanguageId");

  InitializeObjectAttributes(&ObjectAttributes,
			     &KeyName,
			     OBJ_CASE_INSENSITIVE,
			     UserHandle,
			     NULL);

  Status = ZwOpenKey(&KeyHandle,
		     KEY_SET_VALUE,
		     &ObjectAttributes);
  if (!NT_SUCCESS(Status))
    {
      ZwClose(UserHandle);
      return Status;
    }

  ValueLength = swprintf(ValueBuffer,
			 L"%04lX",
			 (ULONG)LanguageId);
  ValueLength = (ValueLength + 1) * sizeof(WCHAR);

  Status = ZwSetValueKey(KeyHandle,
			 &ValueName,
			 0,
			 REG_SZ,
			 ValueBuffer,
			 ValueLength);

  ZwClose(KeyHandle);
  ZwClose(UserHandle);

  return Status;
}

/* EOF */
