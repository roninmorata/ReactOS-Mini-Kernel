/* $Id: interlck.c 13311 2005-01-26 13:58:37Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/ex/interlck.c
 * PURPOSE:         Implements interlocked functions
 * 
 * PROGRAMMERS:     David Welch (welch@mcmail.com)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <internal/debug.h>

/* FUNCTIONS *****************************************************************/

#undef ExInterlockedDecrementLong

/*
 * @implemented
 */
INTERLOCKED_RESULT STDCALL
ExInterlockedDecrementLong (PLONG		Addend,
			    PKSPIN_LOCK	Lock)
/*
 * Obsolete, use InterlockedDecrement instead
 */
{
        KIRQL oldlvl;
        LONG  oldval;

        KeAcquireSpinLock (Lock, &oldlvl);

        oldval = *Addend;
        (*Addend)--;

        KeReleaseSpinLock (Lock, oldlvl);

        return oldval;
}

#undef ExInterlockedExchangeUlong

/*
 * @implemented
 */
ULONG STDCALL
ExInterlockedExchangeUlong (PULONG		Target,
			    ULONG		Value,
			    PKSPIN_LOCK	Lock)
/*
 * Obsolete, use InterlockedExchange instead
 */
{
        KIRQL oldlvl;
        LONG  oldval;

        KeAcquireSpinLock (Lock, &oldlvl);

        oldval = *Target;
        *Target = Value;

        KeReleaseSpinLock (Lock, oldlvl);

        return oldval;
}


#undef ExInterlockedAddUlong

/*
 * @implemented
 */
ULONG STDCALL
ExInterlockedAddUlong (PULONG		Addend,
		       ULONG		Increment,
		       PKSPIN_LOCK	Lock)
/*
 * ExInterlockedAddUlong adds an unsigned long value to a given unsigned
 * integer as an atomic operation.
 * 
 * ADDEND = Points to an unsigned long integer whose value is to be adjusted
 * by the Increment value.
 * 
 * INCREMENT = Is an unsigned long integer to be added.
 * 
 * LOCK = Points to a spinlock to be used to synchronize access to ADDEND.
 * 
 * Returns: 
 * 
 * The original value of the unsigned integer pointed to by ADDEND.
 */
{
        KIRQL oldlvl;
        ULONG oldval;

        KeAcquireSpinLock (Lock, &oldlvl);

        oldval = *Addend;
        *Addend += Increment;

        KeReleaseSpinLock (Lock, oldlvl);

        return oldval;
}

/*
 * @implemented
 */
LARGE_INTEGER STDCALL
ExInterlockedAddLargeInteger (PLARGE_INTEGER Addend,
			      LARGE_INTEGER Increment,
			      PKSPIN_LOCK Lock)
/*
 * Adds two large integer values as an atomic operation.
 * 
 * ADDEND = Pointer to a large integer value that will have INCREMENT added.
 * 
 * INCREMENT = Value to be added.
 * 
 * LOCK = Spinlock used to synchronize access to ADDEND.
 * 
 * Returns:
 * 
 * The original value of the large integer pointed to by ADDEND.
 */
{
        KIRQL oldlvl;
        LARGE_INTEGER oldval;


        KeAcquireSpinLock (Lock, &oldlvl);


        oldval.QuadPart = Addend->QuadPart;
        Addend->QuadPart += Increment.QuadPart;

        KeReleaseSpinLock (Lock, oldlvl);

        return oldval;
}

#undef ExInterlockedIncrementLong

/*
 * @implemented
 */
INTERLOCKED_RESULT STDCALL
ExInterlockedIncrementLong (PLONG		Addend,
			    PKSPIN_LOCK	Lock)
/*
 * Obsolete, use InterlockedIncrement instead.
 */
{
        KIRQL oldlvl;
        LONG  oldval;

        KeAcquireSpinLock (Lock, &oldlvl);

        oldval = *Addend;
        (*Addend)++;

        KeReleaseSpinLock (Lock, oldlvl);

        return oldval;
}

/*
 * @implemented
 */
VOID FASTCALL
ExInterlockedAddLargeStatistic (IN	PLARGE_INTEGER	Addend,
				IN	ULONG		Increment)
/*
 * Undocumented in DDK.
 */
{
	Addend->QuadPart += Increment;
}

/*
 * @implemented
 */
LONGLONG FASTCALL
ExInterlockedCompareExchange64 (IN OUT	PLONGLONG	Destination,
				IN	PLONGLONG	Exchange,
				IN	PLONGLONG	Comparand,
				IN	PKSPIN_LOCK	Lock)
/*
 * Undocumented in DDK.
 */
{
	KIRQL oldlvl;
	LONGLONG oldval;

	KeAcquireSpinLock (Lock, &oldlvl);

	oldval = *Destination;
	if (*Destination == *Comparand)
	{
		*Destination = *Exchange;
	}

	KeReleaseSpinLock (Lock, oldlvl);

	return oldval;
}

/*
 * @implemented
 */
ULONG FASTCALL
ExfInterlockedAddUlong(PULONG Addend,
		       ULONG Increment,
		       PKSPIN_LOCK Lock)
/*
 * ExInterlockedAddUlong adds an unsigned long value to a given unsigned
 * integer as an atomic operation.
 * 
 * ADDEND = Points to an unsigned long integer whose value is to be adjusted
 * by the Increment value.
 * 
 * INCREMENT = Is an unsigned long integer to be added.
 * 
 * LOCK = Points to a spinlock to be used to synchronize access to ADDEND.
 * 
 * Returns: 
 * 
 * The original value of the unsigned integer pointed to by ADDEND.
 */
{
  KIRQL oldlvl;
  ULONG oldval;

  KeAcquireSpinLock (Lock, &oldlvl);

  oldval = *Addend;
  *Addend += Increment;

  KeReleaseSpinLock (Lock, oldlvl);

  return oldval;
}

/* EOF */
