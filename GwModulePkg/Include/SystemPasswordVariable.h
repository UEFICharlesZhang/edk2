/** @file

Copyright (c) 2006 - 2016, GreatWall Corporation.<BR>
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced,
stored in a retrieval system, or transmitted in any form or by any
means without the express written consent of GreatWall Corporation.

File Name:
  SystemPassword.h

Abstract:
  Implementation of basic setup password function.

Revision History:

**/

#ifndef __SYSTEM_PASSWORD_VARIABLE___H__
#define __SYSTEM_PASSWORD_VARIABLE___H__

#define SYSTEM_PASSWORD_LENGTH    64

#define SYSTEM_PASSWORD_KEY    0x1028

#define SYSTEM_PASSWORD_HASH_LENGTH    12

#define SYSTEM_PASSWORD_GUID \
  { \
     0x7d925a5d, 0x90c2, 0x4c85, { 0x90, 0x6d, 0x1, 0x3f, 0x56, 0x5e, 0x33, 0xee }\
  }

#define SYSTEM_PASSWORD_NAME   L"SysPd"

#pragma pack(1)
typedef struct _SYSTEM_PASSWORD{
  UINT16    Admin[SYSTEM_PASSWORD_LENGTH];  //temporary buffer for VFR, never be saved.
  UINT8     AdminHash[SYSTEM_PASSWORD_HASH_LENGTH];
  UINT16    PowerOn[SYSTEM_PASSWORD_LENGTH];  //temporary buffer for VFR, never be saved.
  UINT8     PowerOnHash[SYSTEM_PASSWORD_HASH_LENGTH];
  UINT8     bHaveAdmin;  //temporary buffer for VFR, never be saved.
  UINT8     bHavePowerOn;  //temporary buffer for VFR, never be saved.
  UINT8     VerifyTimes;
  UINT8     VerifyTimesAdmin;
  UINT8     VerifyTimesPop;
  UINT8     EnteredType;
  UINT8     PopCheckStatus;
  UINT8     AcpiWakeupSrc;
  UINT8    RequirePopOnRestart;
  UINT8    ChangePopByUser;  
} SYSTEM_PASSWORD;
#pragma pack()

#define SYSTEM_PASSWORD_VARSTORE    \
     efivarstore SYSTEM_PASSWORD, attribute = 0x7, name = SysPd, guid = SYSTEM_PASSWORD_GUID;

#endif
