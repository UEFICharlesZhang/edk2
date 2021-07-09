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

#include "PlatformSetupDxe.h"
#include "SetupCallback.h"

//
// module global data.
//
EFI_HII_CONFIG_ROUTING_PROTOCOL    *mHiiConfigRouting;

HII_VENDOR_DEVICE_PATH  gHiiVendorDevicePath = {
  {
    {
      {
        HARDWARE_DEVICE_PATH,
        HW_VENDOR_DP,
        {
          (UINT8) (sizeof (HII_VENDOR_DEVICE_PATH_NODE)),
          (UINT8) ((sizeof (HII_VENDOR_DEVICE_PATH_NODE)) >> 8)
        }
      },
      EFI_IFR_TIANO_GUID,
    },
    0,
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      END_DEVICE_PATH_LENGTH
    }
  }
};

SETUP_FORMSET_INFO    gSetupFormSets[] = {
  {FORMSET_MAIN,       FORMSET_GUID_MAIN,       FormsetMainBin,       NULL,     NULL},
  {FORMSET_ADVANCE,    FORMSET_GUID_ADVANCE,    FormsetAdvancedBin,   NULL,     NULL},
  {FORMSET_SECURITY,   FORMSET_GUID_SECURITY,   FormsetSecurityBin,   NULL,     NULL},
  {FORMSET_BOOT,       FORMSET_GUID_BOOT,       FormsetBootBin,       NULL,     NULL},
  {FORMSET_EXIT,       FORMSET_GUID_EXIT,       FormsetExitBin,       NULL,     NULL},
};
UINTN    gSetupFormSetsCount;

FORM_CALLBACK_ITEM	gFormCallback[] = {
  {FORMSET_EXIT,    EXIT_KEY_SAVE,    ExitFormCallback},
  {FORMSET_EXIT,    EXIT_KEY_DISCARD,    ExitFormCallback},
  {FORMSET_EXIT,    EXIT_KEY_DEFAULT,    ExitFormCallback},
  {FORMSET_SECURITY,    SEC_KEY_ADMIN_PD,    PasswordFormCallback},
  {FORMSET_SECURITY,    SEC_KEY_POWER_ON_PD,    PasswordFormCallback},
  {FORMSET_SECURITY,    SEC_KEY_CLEAR_USER_PD,    PasswordFormCallback},
};

FORM_INIT_ITEM gFormInit[] = {
  {FORMSET_MAIN, MainFormInit},
};

STATIC SETUP_SAVE_NOTIFY_PROTOCOL  gSetupSaveNotify;

EFI_STATUS
EFIAPI
HiiExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL    *This,
  IN  CONST EFI_STRING    Request,
  OUT EFI_STRING    *Progress,
  OUT EFI_STRING    *Results
  )
{
  EFI_STATUS    Status;
  FORM_CALLBACK_INFO    *Private;
  EFI_STRING    ConfigRequestHdr;
  EFI_STRING    ConfigRequest;
  BOOLEAN    AllocatedRequest;
  UINTN    Size;
  UINTN    BufferSize;
  VOID    *SystemConfigPtr;

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Request;
  if ((Request != NULL) && !HiiIsConfigHdrMatch (Request, &gPlatformSetupVariableGuid, PLATFORM_SETUP_VARIABLE_NAME)) {
    return EFI_NOT_FOUND;
  }

  ConfigRequestHdr = NULL;
  ConfigRequest    = NULL;
  Size             = 0;
  AllocatedRequest = FALSE;

  Private          = CALLBACK_INFO_FROM_THIS (This);

  ConfigRequest = Request;
  if ((Request == NULL) || (StrStr (Request, L"OFFSET") == NULL)) {
    //
    // Request has no request element, construct full request string.
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&gPlatformSetupVariableGuid, PLATFORM_SETUP_VARIABLE_NAME, Private->DriverHandle);
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    AllocatedRequest = TRUE;
    BufferSize = sizeof (SYSTEM_CONFIGURATION);
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    FreePool (ConfigRequestHdr);
  }
  SystemConfigPtr = GetVariable(PLATFORM_SETUP_VARIABLE_NAME, &gPlatformSetupVariableGuid);

  if (SystemConfigPtr == NULL) {
    ZeroMem(&Private->FakeNvData, sizeof(SYSTEM_CONFIGURATION));
  } else {
    CopyMem(&Private->FakeNvData, SystemConfigPtr, sizeof(SYSTEM_CONFIGURATION));
    FreePool(SystemConfigPtr);
  }
  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  Status = mHiiConfigRouting->BlockToConfig (
                                mHiiConfigRouting,
                                ConfigRequest,
                                (UINT8 *) &Private->FakeNvData,
                                sizeof (SYSTEM_CONFIGURATION),
                                Results,
                                Progress
                                );
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
    ConfigRequest = NULL;
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return Status;
}

EFI_STATUS
EFIAPI
HiiRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL    *This,
  IN  CONST EFI_STRING    Configuration,
  OUT EFI_STRING    *Progress
  )
{
  FORM_CALLBACK_INFO    *Private;
  SYSTEM_CONFIGURATION    *FakeNvData;

  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  *Progress = Configuration;

  if (!HiiIsConfigHdrMatch (Configuration, &gPlatformSetupVariableGuid, PLATFORM_SETUP_VARIABLE_NAME)) {
    return EFI_NOT_FOUND;
  }

  *Progress = Configuration + StrLen (Configuration);
  Private    = CALLBACK_INFO_FROM_THIS (This);
  FakeNvData = &Private->FakeNvData;
  if (!HiiGetBrowserData (&gPlatformSetupVariableGuid, PLATFORM_SETUP_VARIABLE_NAME, sizeof (SYSTEM_CONFIGURATION), (UINT8 *) FakeNvData)) {
    //
    // FakeNvData can't be got from SetupBrowser, which doesn't need to be set.
    //
    return EFI_SUCCESS;
  }

  gRT->SetVariable(
    PLATFORM_SETUP_VARIABLE_NAME,
    &gPlatformSetupVariableGuid,
    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
    sizeof(SYSTEM_CONFIGURATION),
    &Private->FakeNvData
 );

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
FormCallback (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL    *This,
  IN EFI_BROWSER_ACTION    Action,
  IN EFI_QUESTION_ID    KeyValue,
  IN UINT8    Type,
  IN EFI_IFR_TYPE_VALUE    *Value,
  OUT EFI_BROWSER_ACTION_REQUEST    *ActionRequest
)
{
  EFI_STATUS Status;
  UINTN    Index;
  FORM_CALLBACK_INFO    *pCallbackInfo;

  pCallbackInfo = NULL;
  pCallbackInfo =  CALLBACK_INFO_FROM_THIS (This);

  if ((Action != EFI_BROWSER_ACTION_CHANGING && Action != EFI_BROWSER_ACTION_CHANGED) || (pCallbackInfo == NULL)) {
    //
    // Only do callback when data is changed.
    //
    return EFI_UNSUPPORTED;
  }

  Status = EFI_SUCCESS;
  for (Index = 0; Index < (sizeof(gFormCallback) / sizeof(FORM_CALLBACK_ITEM)); Index++) {
    if ((gFormCallback[Index].Class == pCallbackInfo->Class) &&
      ((gFormCallback[Index].Key == KeyValue) || (gFormCallback[Index].Key == KEY_UNASSIGN_GROUP))) {
      Status = gFormCallback[Index].Callback (This, Action, KeyValue, Type, Value, ActionRequest);
    }
  }
  return Status;
}

VOID
RemoveHiiResource ( VOID )
{
  UINTN    Index;

  for (Index = 0; Index < (sizeof(gSetupFormSets) / sizeof(SETUP_FORMSET_INFO)); Index++) {
    if (gSetupFormSets[Index].CallInfo != NULL) {
      if (gSetupFormSets[Index].CallInfo->DriverHandle != NULL) {
        gBS->UninstallMultipleProtocolInterfaces (
               gSetupFormSets[Index].CallInfo->DriverHandle,
               &gEfiDevicePathProtocolGuid,
               &gSetupFormSets[Index].CallInfo->VendorDevicePath,
               &gEfiHiiConfigAccessProtocolGuid,
               &gSetupFormSets[Index].CallInfo->ConfigAccess,
               NULL
               );
      }

      if (gSetupFormSets[Index].CallInfo->VendorDevicePath != NULL) {
        FreePool (gSetupFormSets[Index].CallInfo->VendorDevicePath);
        gSetupFormSets[Index].CallInfo->VendorDevicePath = NULL;
      }

      FreePool (gSetupFormSets[Index].CallInfo);
      gSetupFormSets[Index].CallInfo = NULL;
    }

    if (gSetupFormSets[Index].HiiHandle != NULL) {
      HiiRemovePackages (gSetupFormSets[Index].HiiHandle);
      gSetupFormSets[Index].HiiHandle = NULL;
    }
  }
}

VOID
CheckDefaultSetupVariable (VOID )
{
  EFI_STATUS    Status;
  BOOLEAN    IsSetDefault;
  BOOLEAN    ActionFlag;
  UINTN    Size;
  UINTN    Index;
  SYSTEM_CONFIGURATION    SetupData = {0};
  EFI_STRING    ConfigRequest;

  DEBUG((DEBUG_INFO, "CheckDefaultSetupVariable(),\n"));
  //
  // Initialize the default vaule of setup variable "Setup" if it doesn't exist.
  //
  Size = sizeof (SYSTEM_CONFIGURATION);
  Status = gRT->GetVariable (
                  PLATFORM_SETUP_VARIABLE_NAME,
                  &gPlatformSetupVariableGuid,
                  NULL,
                  &Size,
                  &SetupData
                  );
  if (Status == EFI_NOT_FOUND) {
     gRT->SetVariable (
                  PLATFORM_SETUP_VARIABLE_NAME,
                  &gPlatformSetupVariableGuid,
                  PLATFORM_SETUP_VARIABLE_FLAG,
                  Size,
                  &SetupData
                  );
  }

  for (Index = 0; Index < (sizeof(gSetupFormSets) / sizeof(SETUP_FORMSET_INFO)); Index++) {
    ConfigRequest = HiiConstructConfigHdr (
                               &gPlatformSetupVariableGuid,
                               PLATFORM_SETUP_VARIABLE_NAME,
                               gSetupFormSets[Index].CallInfo->DriverHandle
                               );
    ASSERT (ConfigRequest != NULL);
    if(EFI_ERROR(Status)){
      IsSetDefault = HiiSetToDefaults (ConfigRequest, EFI_HII_DEFAULT_CLASS_STANDARD);
      ASSERT (IsSetDefault);
    }else{
      //
      // EFI variable does exist and Validate Current Setting
      //
      ActionFlag = HiiValidateSettings (ConfigRequest);
      ASSERT (ActionFlag);
    }
      FreePool (ConfigRequest);
  }
}

EFI_STATUS
PlatformLoadDefault (
  IN SETUP_SAVE_NOTIFY_PROTOCOL *This
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;
  SYSTEM_CONFIGURATION    SetupData = {0};

  HiiGetBrowserData(&gPlatformSetupVariableGuid, PLATFORM_SETUP_VARIABLE_NAME, sizeof (SYSTEM_CONFIGURATION), (UINT8*) &SetupData);

  SetupData.Numlock = 32;

  HiiSetBrowserData(&gPlatformSetupVariableGuid, PLATFORM_SETUP_VARIABLE_NAME, sizeof (SYSTEM_CONFIGURATION), (UINT8*) &SetupData, NULL);

  return Status;
}

VOID
SignalEnterSetupEvent (
  VOID
  )
{
  EFI_HANDLE                 Handle;
  EFI_STATUS                 Status;

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiByoSetupEnteringGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
}

EFI_STATUS
InstallPlatformHiiResource (
  VOID )
{
  EFI_STATUS    Status;
  EFI_FORM_BROWSER2_PROTOCOL    *FormBrowser2;
  UINTN    Index, i;
  //
  // There should only be one Form Configuration protocol
  //
  Status = gBS->LocateProtocol (
                 &gEfiFormBrowser2ProtocolGuid,
                 NULL,
                 (VOID **) &FormBrowser2
                 );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **) &mHiiConfigRouting
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  //Install HII resource.
  //
  for (Index = 0; Index < (sizeof(gSetupFormSets) / sizeof(SETUP_FORMSET_INFO)); Index++) {
  DEBUG((DEBUG_INFO,"%a:line:%d\n",__FUNCTION__,__LINE__));

    //
    // Callback Info.
    //
    gSetupFormSets[Index].CallInfo = AllocateZeroPool (sizeof (FORM_CALLBACK_INFO));
    if (gSetupFormSets[Index].CallInfo == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Finish;
    }

    gSetupFormSets[Index].CallInfo->Signature = CALLBACK_INFO_SIGNATURE;
    gSetupFormSets[Index].CallInfo->Class = gSetupFormSets[Index].Class;
    gSetupFormSets[Index].CallInfo->ConfigAccess.ExtractConfig = HiiExtractConfig;
    gSetupFormSets[Index].CallInfo->ConfigAccess.RouteConfig = HiiRouteConfig;
    gSetupFormSets[Index].CallInfo->ConfigAccess.Callback = FormCallback;

    //
    // Device Path.
    //
    gSetupFormSets[Index].CallInfo->VendorDevicePath = AllocateCopyPool (sizeof (HII_VENDOR_DEVICE_PATH), &gHiiVendorDevicePath);
    if (gSetupFormSets[Index].CallInfo->VendorDevicePath == NULL) {
      Status =  EFI_OUT_OF_RESOURCES;
      goto Finish;
    }

    //
    // Use memory address as unique ID to distinguish from different device paths
    //
    gSetupFormSets[Index].CallInfo->VendorDevicePath->UniqueId = (UINT64) ((UINTN) gSetupFormSets[Index].CallInfo->VendorDevicePath);

    //
    // Install Device Path Protocol and Config Access protocol to driver handle
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &gSetupFormSets[Index].CallInfo->DriverHandle,
                    &gEfiDevicePathProtocolGuid,
                    gSetupFormSets[Index].CallInfo->VendorDevicePath,
                    &gEfiHiiConfigAccessProtocolGuid,
                    &gSetupFormSets[Index].CallInfo->ConfigAccess,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      goto Finish;
    }

    //
    // Publish All HII data.
    //
    gSetupFormSets[Index].HiiHandle = HiiAddPackages (
                                       &gSetupFormSets[Index].FormSetGuid,
                                       gSetupFormSets[Index].CallInfo->DriverHandle,
                                       gSetupFormSets[Index].IfrPack,
                                       PlatformSetupDxeStrings,
                                       NULL
                                       );
    if (gSetupFormSets[Index].HiiHandle== NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Finish;
    }
	
    //
    // Initialize Forms.
    // 
    for (i = 0; i < (sizeof(gFormInit) / sizeof(FORM_INIT_ITEM)); i++) {
      if (gSetupFormSets[Index].Class == gFormInit[i].Class) {
        gFormInit[i].FormInit(gSetupFormSets[Index].HiiHandle);
      }
    }
  }

  gSetupFormSetsCount = sizeof(gSetupFormSets) / sizeof(SETUP_FORMSET_INFO);
  //
  // Initialize setup variable.
  //
  CheckDefaultSetupVariable();
  
  return EFI_SUCCESS;

Finish:
  DEBUG ((EFI_D_ERROR, "InstallPlatformHiiResource(), Finish :%r.\n", Status));
  RemoveHiiResource();
  return Status;
}


VOID
EFIAPI
SetupLoadEventCallback (
  IN EFI_EVENT Event,
  IN VOID     *Context
  )
{

  InstallPlatformHiiResource();
  
  return;
}
/**
  The driver Entry Point. The funciton will export a disk device class formset and
  its callback function to hii database.

  @param  ImageHandle    The firmware allocated handle for the EFI image.
  @param  SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS    The entry point is executed successfully.
  @retval other          Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
PlatformSetupDxeInit (
  IN EFI_HANDLE    ImageHandle,
  IN EFI_SYSTEM_TABLE    *SystemTable
  )
{
  EFI_STATUS    Status;
  EFI_EVENT   Event;	
  VOID    *pSetupRegistration;
  EFI_HANDLE    DriverHandle;

  //
  // Create event to trigger enter setup.
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  SetupLoadEventCallback,
                  NULL,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for protocol notifications on this event
  //
  pSetupRegistration = NULL;
  Status = gBS->RegisterProtocolNotify (
                  &gEfiByoSetupEnteringGuid,
                  Event,
                  &pSetupRegistration
                  );

  ASSERT_EFI_ERROR (Status);

  //
  //Install setup notigy function.
  //
  DriverHandle = NULL;
  ZeroMem(&gSetupSaveNotify, sizeof(SETUP_SAVE_NOTIFY_PROTOCOL));
  gSetupSaveNotify.LoadDefault = (EFI_SETUP_SAVE_LOAD_DEFAULT)PlatformLoadDefault;
  Status = gBS->InstallProtocolInterface (
                  &DriverHandle,
                  &gSetupSaveNotifyProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &gSetupSaveNotify
                  );

  return Status;
}

/**
  Unload its installed protocol.

  @param[in]  ImageHandle       Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
**/
EFI_STATUS
EFIAPI
PlatformSetupDxeUnload (
  IN EFI_HANDLE    ImageHandle
  )
{
  RemoveHiiResource ();
  return EFI_SUCCESS;
}

