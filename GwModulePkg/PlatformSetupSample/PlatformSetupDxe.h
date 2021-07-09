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

#ifndef _PLAT_OVER_MNGR_H_
#define _PLAT_OVER_MNGR_H_

#include <FrameworkDxe.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/FormBrowserEx2.h>
#include <Protocol/DisplayProtocol.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/PciIo.h>
#include <Protocol/BusSpecificDriverOverride.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/ComponentName.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/DevicePath.h>
#include <Protocol/PlatformDriverOverride.h>
#include <Protocol/DataHub.h>
#include <Guid/MdeModuleHii.h>
#include <Guid/VariableFormat.h>
#include <Guid/DataHubRecords.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/FrameworkFormBrowser.h>
#include <Protocol/SetupSaveNotify.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/HobLib.h>
#include <IndustryStandard/Pci22.h>

#include <SetupVariableSample.h>
#include <Guid/ByoSetupFormsetGuid.h>
//#include <Protocol/LogSetupVariableProtocol.h>
#include <Protocol/ByoFormSetManager.h>
#include "FormsetConfiguration.h"
#include "SetupFormInit.h"


typedef enum {
  FORMSET_MAIN = 0,
  FORMSET_ADVANCE,
  FORMSET_SECURITY,
  FORMSET_EXIT,
  FORMSET_BOOT,
  FORMSET_MAX
} FORM_CLASS;

#pragma pack(1)
typedef struct {
  VENDOR_DEVICE_PATH    VendorDevicePath;
  UINT32    Reserved;
  UINT64    UniqueId;
} HII_VENDOR_DEVICE_PATH_NODE;

typedef struct {
  HII_VENDOR_DEVICE_PATH_NODE    VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL    End;
} HII_VENDOR_DEVICE_PATH;
#pragma pack()

typedef struct {
  UINTN    Signature;
  FORM_CLASS    Class;
  EFI_HANDLE    DriverHandle;
  EFI_HII_HANDLE    RegisteredHandle;
  HII_VENDOR_DEVICE_PATH_NODE    *VendorDevicePath;
  SYSTEM_CONFIGURATION    FakeNvData;
  EFI_HII_CONFIG_ACCESS_PROTOCOL    ConfigAccess;
} FORM_CALLBACK_INFO;

#define CALLBACK_INFO_SIGNATURE SIGNATURE_32 ('C', 'l', 'b', 'k')
#define CALLBACK_INFO_FROM_THIS(a)  CR (a, FORM_CALLBACK_INFO, ConfigAccess, CALLBACK_INFO_SIGNATURE)

typedef struct _SETUP_FORMSET_INFO {
  FORM_CLASS    Class;
  EFI_GUID    FormSetGuid;
  UINT8    *IfrPack;
  EFI_HII_HANDLE    HiiHandle;
  FORM_CALLBACK_INFO    *CallInfo;
} SETUP_FORMSET_INFO;

typedef struct _FORM_CALLBACK_ITEM {
  FORM_CLASS    Class;
  UINT16    Key;
  EFI_HII_ACCESS_FORM_CALLBACK    Callback;
} FORM_CALLBACK_ITEM;

typedef
EFI_STATUS
(EFIAPI *EFI_FORM_INIT)(
  IN EFI_HII_HANDLE    Handle
 );

typedef struct _FORM_INIT_ITEM {
  FORM_CLASS    Class;
  EFI_FORM_INIT    FormInit;
} FORM_INIT_ITEM;

//
// uni string and Vfr Binary data.
//
extern UINT8  PlatformSetupDxeStrings[];
extern UINT8  FormsetMainBin[];
extern UINT8  FormsetAdvancedBin[];
extern UINT8  FormsetSecurityBin[];
extern UINT8  FormsetExitBin[];
extern UINT8  FormsetBootBin[];

extern SETUP_FORMSET_INFO    gSetupFormSets[];
extern UINTN    gSetupFormSetsCount;

#endif
