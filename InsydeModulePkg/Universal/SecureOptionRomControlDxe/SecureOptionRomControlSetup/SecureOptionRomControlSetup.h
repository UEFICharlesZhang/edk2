/** @file
  Header file for Secure Option Rom Control Setup page

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SECURE_OPTION_ROM_CONTROL_SETUP_H_
#define _SECURE_OPTION_ROM_CONTROL_SETUP_H_

#include "SetupDataStruc.h"
#include "../CommonDefinition.h"
#include <Guid/SecureOptionRomControl.h>
#include <Library/DebugLib.h>
#include <Library/SetupUtilityLib.h>
#include <Library/VariableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>
#include <Protocol/SetupUtilityApplication.h>

//
// This is the generated IFR binary data for each formset defined in VFR.
// This data array is ready to be used as input of HiiAddPackages() to
// create a packagelist (which contains Form packages, String packages, etc).
//
extern UINT8  SecureOptionRomControlSetupBin[];

//
// This is the generated String package data for all .UNI files.
// This data array is ready to be used as input of HiiAddPackages() to
// create a packagelist (which contains Form packages, String packages, etc).
//
extern UINT8  SecureOptionRomControlSetupStrings[];

#define EFI_LIST_ENTRY                       LIST_ENTRY  

//
// Define the ROM type which contained in ROM image
//
#define HAVE_LEGACY_ROM                      0x01
#define HAVE_UNSIGNED_EFI_ROM                0x02
#define HAVE_SIGNED_EFI_ROM                  0x04 

#define TEXT_INPUT_BOX_HEIGHT                7
#define TEXT_POPUP_DIALOG_WIDTH              23 
#define TEXT_VENDOR_ID_STRING_LENGTHE        4

#define MAX_OPROM_NAME_ID_NUM                16
#define MAX_STRING_BUFFER_NUM                0x100
#define MAX_HELP_BUFFER_NUM                  0x100

typedef struct {
  UINTN                              Signature;

  EFI_HANDLE                         DriverHandle;
  EFI_HII_HANDLE                     HiiHandle;
  SECURE_OPROM_CONTROL_CONFIGURATION Configuration;

  //
  // Consumed protocol
  //
  EFI_HII_DATABASE_PROTOCOL          *HiiDatabase;
  EFI_HII_STRING_PROTOCOL            *HiiString;
  EFI_HII_CONFIG_ROUTING_PROTOCOL    *HiiConfigRouting;
  EFI_FORM_BROWSER2_PROTOCOL         *FormBrowser2;

  //
  // Produced protocol
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL     ConfigAccess;
} SECURE_OPROM_CONTROL_PRIVATE_DATA;

#define SECURE_OPROM_CONTROL_PRIVATE_FROM_THIS(a)  CR (a, SECURE_OPROM_CONTROL_PRIVATE_DATA, ConfigAccess, SECURE_OPROM_CONTROL_PRIVATE_SIGNATURE)

#pragma pack(1)

//
// HII specific Vendor Device Path definition.
//
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;

typedef struct _OPTION_ROM_NAME_LIST {
  UINT16                   VendorId;
  UINT16                   DeviceId;
} OPTION_ROM_DEVICE_LIST;

#pragma pack()

/**
  Initialize Secure Option Rom Control sub-menu for setuputility use.
  
  @param[in] HiiHandle   - Hii hanlde for the call back routine.
  @param[in] OptionList  - Hii hanlde for the call back routine.
    
  @retval EFI_SUCCESS  -  Function has completed successfully.
  @retval Others       -  Error occurred during execution.
  
**/
EFI_STATUS
EFIAPI
UpdateSecureOpRomDynamicItems (
  IN EFI_HII_HANDLE       HiiHandle,
  IN EFI_LIST_ENTRY       *OptionList
  );

/**
  Update Option ROM policy in Option ROM list.
  
  @param[in] OpRomPolicyNum   - Number of Option ROM list.
  @param[in] Type             - Policy of the OpRom entry.
    
  @retval EFI_SUCCESS     -  OpRom entry in Option ROM list.
  @retval EFI_NOT_FOUND   -  Not found the OpRom entry.
  
**/  
EFI_STATUS
EFIAPI
UpdateOptionRomPolicy (
  IN  UINT16                                 OpRomPolicyNum,
  IN  UINT8                                  Type
  );

/**
  Callback function for inserting the Option ROM policy.

  @param[in] HiiHandle  - Hii hanlde for the call back routine.
  @param[in] *H2ODialog - Protocol interface of H2O Dialog protocol.
  
  @retval EFI_SUCCESS   - Insert the Option ROM policy entry success.
  @retval other         - Insert the Option ROM policy entry failure.
  
**/
EFI_STATUS
EFIAPI
InsertOptionRomPolicyCallback (
  IN  EFI_HII_HANDLE           HiiHandle,
  IN  H2O_DIALOG_PROTOCOL      *H2ODialog
  );

/**
  Callback function for deleting the Option ROM policy.

  @param[in] HiiHandle  - Hii hanlde for the call back routine.
  @param[in] *H2ODialog - Protocol interface of H2O Dialog protocol.
    
  @retval EFI_SUCCESS - Delete the Option ROM policy entry success.
  @retval other       - Delete the Option ROM policy entry failure.
  
**/
EFI_STATUS
EFIAPI
DeleteOptionRomPolicyCallback (
  IN  EFI_HII_HANDLE           HiiHandle,
  IN  H2O_DIALOG_PROTOCOL      *H2ODialog
  );

/**
  Remove OpRom Entries from OpRomPolicyList in Setup Utility configuration.

  @param  VOID

  @retval EFI_SUCCESS            Function has completed successfully.
  @return Other                  Error occurred during execution.

**/
EFI_STATUS  
FreeOptionRomPolicyList (
  VOID
  );

/**
  Initialize Secure Option Rom Control sub-menu for setuputility use
  
  @param[in] HiiHandle   - Hii hanlde for the call back routine
    
  @retval EFI_SUCCESS  -  Function has completed successfully.
  @retval Others       -  Error occurred during execution.
  
**/
EFI_STATUS
EFIAPI
InitSecureOpRomSubMenu (
  IN EFI_HII_HANDLE       HiiHandle
  );

/**
  Initialization for the Setup related functions.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
SetupInit (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  );

#endif
