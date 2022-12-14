#ifndef _NTOS_H
#define _NTOS_H
/* $Id: ntos.h 13392 2005-02-02 23:07:33Z gvg $ */

#if defined(NTOS_MODE_USER)
/* 
 * Include windows.h before ntddk.h to get user mode prototype 
 * for InterlockedXxx functions.
 */
#include <windows.h>
#include <ddk/ntddk.h>
#include <ddk/ntifs.h>
#include "ntos/types.h"
#include "ntos/cdrom.h"
#include "ntos/console.h"
#include "ntos/disk.h"
#include "ntos/tape.h"
#include "ntos/except.h"
#include "ntos/file.h"
#include "ntos/gditypes.h"
#include "ntos/fstypes.h"   /* AG */
#include "ntos/heap.h"
#include "ntos/keyboard.h"
#include "ntos/minmax.h"
#include "ntos/mm.h"
#include "ntos/ntdef.h"
#include "ntos/port.h"
#include "ntos/ps.h"
#include "ntos/registry.h"
#include "ntos/security.h"
#include "ntos/synch.h"
#include "ntos/time.h"
#include "napi/i386/segment.h"
#include "napi/types.h"
#include "napi/dbg.h"
#include "napi/npipe.h"
#include "napi/shared_data.h"
#include "napi/win32.h"
#include "ntos/rtltypes.h"
#include "ntos/rtl.h"
#include "ntos/zwtypes.h"
#include "ntos/zw.h"
#include "ntos/dbgfuncs.h"
#include "ntos/service.h"
#include "ntdll/csr.h"
#include "ntdll/dbg.h"
#include "ntdll/ldr.h"
#include "ntdll/rtl.h"
#include "ntdll/trace.h"
#include "rosrtl/thread.h"
#else /* Assume kernel mode */
#include <ddk/ntddk.h>
#include <ddk/ntifs.h>
#include "ntos/types.h"
#include "ntos/cdrom.h"
#include "ntos/console.h"
#include "ntos/disk.h"
#include "ntos/tape.h"
#include "ntos/except.h"
#include "ntos/file.h"
#include "ntos/gditypes.h"
#include "ntos/heap.h"
#include "ntos/keyboard.h"
#include "ntos/minmax.h"
#include "ntos/mm.h"
#include "ntos/ntdef.h"
#include "ntos/port.h"
#include "ntos/ps.h"
#include "ntos/registry.h"
#include "ntos/security.h"
#include "ntos/synch.h"
#include "ntos/time.h"
#include "napi/i386/segment.h"
#include "napi/types.h"
#include "napi/dbg.h"
#include "napi/npipe.h"
#include "napi/shared_data.h"
#include "napi/win32.h"
#include "ntos/rtltypes.h"
#include "ntos/rtl.h"
#include "ntos/zwtypes.h"
#include "ntos/zw.h"
#include "ntos/dbgfuncs.h"
#include "ntos/service.h"
#include "ntos/haltypes.h"
#include "ntos/halfuncs.h"
#include "ntos/kdfuncs.h"
#include "ntos/kefuncs.h"   /* AG */
#include "ntos/fstypes.h"   /* AG */
#include "ntos/obtypes.h"
#include "ntos/tss.h"
#include "rosrtl/thread.h"
#endif

#endif /* ndef _NTOS_H */
