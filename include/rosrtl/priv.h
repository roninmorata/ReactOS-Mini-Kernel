/* $Id: priv.h 12852 2005-01-06 13:58:04Z mf $
 */

#ifndef ROSRTL_SEC_H__
#define ROSRTL_SEC_H__

#ifdef __cplusplus
extern "C"
{
#endif

BOOL
RosEnableThreadPrivileges(HANDLE *hToken, LUID *Privileges, DWORD PrivilegeCount);
BOOL
RosResetThreadPrivileges(HANDLE hToken);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
