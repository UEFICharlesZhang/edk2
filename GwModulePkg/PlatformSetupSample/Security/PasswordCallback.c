/** @file

Copyright (c) 2006 - 2016, GreatWall Corporation.<BR>
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced,
stored in a retrieval system, or transmitted in any form or by any
means without the express written consent of GreatWall Corporation.

File Name:
  PasswordFormCallback.c

Abstract:
  PasswordFormCallback Setup Rountines

Revision History:


**/

#include "PlatformSetupDxe.h"
#include <Protocol/SystemPasswordProtocol.h>


EFI_STATUS
EFIAPI
PasswordFormCallback (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This,
  IN EFI_BROWSER_ACTION                   Action,
  IN EFI_QUESTION_ID                      KeyValue,
  IN UINT8                                Type,
  IN EFI_IFR_TYPE_VALUE                   *Value,
  OUT EFI_BROWSER_ACTION_REQUEST          *ActionRequest
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;  
  EFI_SYSTEM_PASSWORD_PROTOCOL    *SystemPassword = NULL;
  SYSTEM_PASSWORD    PasswordVar;
  UINTN    VarSize;
  static UINTN PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
  UINTN    Index;  
  CHAR16    *Password = NULL;  
  EFI_HII_HANDLE    HiiHandle = NULL;    

  if (Action != EFI_BROWSER_ACTION_CHANGING) {
    return EFI_SUCCESS;
  }
  //
  // Get system password protocol.
  //
  Status = gBS->LocateProtocol (
                  &gEfiSystemPasswordProtocolGuid,
                  NULL,
                  (void *)&SystemPassword
                  );
  if (EFI_ERROR(Status)) {    
    return Status;
  }    	 
  SystemPassword->SetMode(VERIFY_MODE_SETUP);

  //
  // Get password status.
  //  
  VarSize = sizeof (SYSTEM_PASSWORD);
  Status = gRT->GetVariable (
             SYSTEM_PASSWORD_NAME,
             &gEfiSystemPasswordVariableGuid,
             NULL,
             &VarSize,
             &PasswordVar
             );
  if (EFI_ERROR(Status)) {    
    return Status;
  }
  //
  // Get Security form hii databas handle.
  //
  HiiHandle = NULL;
  for (Index = 0; Index < gSetupFormSetsCount; Index++) {
    if (FORMSET_SECURITY == gSetupFormSets[Index].Class) {
      HiiHandle = gSetupFormSets[Index].HiiHandle;
      break;
    }
  }  
  if (NULL == HiiHandle) {
    return EFI_SUCCESS;
  }
  if ((Type == EFI_IFR_TYPE_STRING) && (Value->string != 0) ) {
    Password = HiiGetString(HiiHandle, Value->string, NULL);
  }
  if (NULL == Password && PasswordState == BROWSER_STATE_SET_PASSWORD) {
    PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
    return EFI_SUCCESS;
  }

  switch (KeyValue) {
  case SEC_KEY_ADMIN_PD:
    switch (PasswordState) {
      case BROWSER_STATE_VALIDATE_PASSWORD:
        if (!SystemPassword->BeHave(PD_ADMIN)) {
	   PasswordState = BROWSER_STATE_SET_PASSWORD;		
          return EFI_SUCCESS;
        }
        if (NULL ==Password) {
          PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
          return EFI_NOT_READY;
        }
        if (SystemPassword->Verify(PD_ADMIN, Password)) {
	   PasswordState = BROWSER_STATE_SET_PASSWORD;		
          Status = EFI_SUCCESS;
        } else {
          Status = EFI_NOT_READY;
        }
        break;
		
      case BROWSER_STATE_SET_PASSWORD:
        if (*Password != CHAR_NULL) {
          SystemPassword->Write(PD_ADMIN, Password);
          PasswordVar.bHaveAdmin = 1;
        } else  {
          SystemPassword->Clear(PD_ADMIN);
          PasswordVar.bHaveAdmin = 0;
        }
        PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
        break;

      default:
        PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
        break;
    }
    break;
    
  case SEC_KEY_POWER_ON_PD:
    switch (PasswordState) {
      case BROWSER_STATE_VALIDATE_PASSWORD:
        if (!SystemPassword->BeHave(PD_POWER_ON)) {
	   PasswordState = BROWSER_STATE_SET_PASSWORD;		
          return EFI_SUCCESS;
        }
        if (NULL ==Password) {
          PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
          return EFI_NOT_READY;
        }
        if (SystemPassword->Verify(PD_POWER_ON, Password)) {
	   PasswordState = BROWSER_STATE_SET_PASSWORD;		
          Status = EFI_SUCCESS;
        } else {
          Status = EFI_NOT_READY;
        }
        break;
		
      case BROWSER_STATE_SET_PASSWORD:
        if (*Password != CHAR_NULL) {
          SystemPassword->Write(PD_POWER_ON, Password);
          PasswordVar.bHavePowerOn = 1;
        } else  {
          SystemPassword->Clear(PD_POWER_ON);
          PasswordVar.bHavePowerOn = 0;
        }
        PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
        break;

      default:
        PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
        break;
    }
    break;

  case SEC_KEY_CLEAR_USER_PD:
    if (SystemPassword->BeHave(PD_POWER_ON)) {
      SystemPassword->Clear (PD_POWER_ON);
      PasswordVar.bHavePowerOn = 0;
    }
    break;
    
  default:
    break;    
  }

  //
  // Restore password to browser.
  //
  VarSize = sizeof (SYSTEM_PASSWORD);
  HiiSetBrowserData (
            &gEfiSystemPasswordVariableGuid, 
            SYSTEM_PASSWORD_NAME, 
            VarSize, 
            (UINT8 *)&PasswordVar, 
            NULL
            );

  return Status;
}
