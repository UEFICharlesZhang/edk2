/*++

Copyright (c) 2010 - 2015, GreatWall Corporation.<BR>
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced,
stored in a retrieval system, or transmitted in any form or by any
means without the express written consent of GreatWall Corporation.

File Name:

Abstract:
  Platform configuration setup.

Revision History:


--*/

#ifndef __SETUP_SAVE_NOTIFY_H__
#define __SETUP_SAVE_NOTIFY_H__


#define SETUP_SAVE_NOTIFY_PROTOCOL_GUID \
  {0xf49499cc, 0x867f, 0x47a8, { 0x82, 0x11, 0x87, 0xb, 0xfb, 0xbe, 0x2d, 0xcd}}


typedef struct _SETUP_SAVE_NOTIFY_PROTOCOL  SETUP_SAVE_NOTIFY_PROTOCOL;

typedef enum {
  SetupSaveNotifyTypeSaveValue,
  SetupSaveNotifyTypeDiscardVaule,
  SetupSaveNotifyTypeLoadDefault,
  SetupSaveNotifyTypeSaveUserDefault,
  SetupSaveNotifyTypeLoadUserDefault,
  SetupSaveNotifyTypeIsDataChanged,
  SetupHookF4
} SETUP_SAVE_NOTIFY_TYPE;


typedef
EFI_STATUS
(EFIAPI *EFI_SETUP_SAVE_SAVE_VALUE)(
 SETUP_SAVE_NOTIFY_PROTOCOL *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SETUP_SAVE_DISCARD_VALUE)(
 SETUP_SAVE_NOTIFY_PROTOCOL *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SETUP_SAVE_LOAD_DEFAULT)(
 SETUP_SAVE_NOTIFY_PROTOCOL *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SETUP_SAVE_USER_DEFAULT)(
 SETUP_SAVE_NOTIFY_PROTOCOL *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SETUP_LOAD_USER_DEFAULT)(
 SETUP_SAVE_NOTIFY_PROTOCOL *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SETUP_IS_DATA_CHANGED)(
 IN  SETUP_SAVE_NOTIFY_PROTOCOL *This,
 OUT BOOLEAN                    *IsDataChanged
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SETUP_HOOK_F4)(
 IN  SETUP_SAVE_NOTIFY_PROTOCOL *This
  );

struct _SETUP_SAVE_NOTIFY_PROTOCOL{
  EFI_SETUP_SAVE_SAVE_VALUE     SaveValue;
  EFI_SETUP_SAVE_DISCARD_VALUE  DiscardValue;
  EFI_SETUP_SAVE_LOAD_DEFAULT   LoadDefault;
  EFI_SETUP_SAVE_USER_DEFAULT   SaveUserDefault;
  EFI_SETUP_LOAD_USER_DEFAULT   LoadUserDefault;
  EFI_SETUP_IS_DATA_CHANGED     IsSetupDataChanged;
  EFI_SETUP_HOOK_F4             HookF4;
};

extern EFI_GUID gSetupSaveNotifyProtocolGuid;

#endif    // end, __SETUP_SAVE_NOTIFY_H__









