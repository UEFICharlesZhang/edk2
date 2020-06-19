/*++

Copyright (c) 2010 - 2015, Byosoft Corporation.<BR>
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced,
stored in a retrieval system, or transmitted in any form or by any
means without the express written consent of Byosoft Corporation.

File Name:

Abstract:
  Platform configuration setup.

Revision History:


--*/

#include "PlatformSetupDxe.h"


/**
  Exit Formset callback.

**/
EFI_STATUS
EFIAPI
ExitFormCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        KeyValue,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  EDKII_FORM_DISPLAY_ENGINE_PROTOCOL    *FormDisplay = NULL;
  EDKII_FORM_BROWSER_EXTENSION2_PROTOCOL    *BrowserExtension = NULL;
  UINTN    SaveFlag;
  LOG_SETUP_VARIABLE_PROTOCOL    *LogVariable = NULL;
  VARIABLE_LOG_INFO    LogInfo;
  VARIABLE_OPERATION_INFO    OperationInfo;
  CHAR16    *Title;
  
  if (Action != EFI_BROWSER_ACTION_CHANGED) {
    //
    // Only do call back when data is changed.
    //
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gEdkiiFormDisplayEngineProtocolGuid,
                  NULL,
                  (void **) &FormDisplay
                  );
  if ( EFI_ERROR(Status) ) {
    return EFI_SUCCESS;
  }
  
  Status = gBS->LocateProtocol (
                  &gEdkiiFormBrowserEx2ProtocolGuid,
                  NULL,
                  (void **) &BrowserExtension
                  );
  if ( ! EFI_ERROR(Status) ) {
		
    switch (KeyValue) {
      case EXIT_KEY_SAVE:	  	
	  	Title= GetToken(STRING_TOKEN (SAVE_CHANGE_QUESTION), gSetupFormSets[0].HiiHandle);
        SaveFlag = FormDisplay->ConfirmDataChange(Title);
        if (!(SaveFlag & BROWSER_ACTION_SUBMIT)) {
          return EFI_SUCCESS;
        }

        //Status = BrowserExtension->ExecuteAction(BROWSER_ACTION_SUBMIT | BROWSER_ACTION_EXIT, 0);        
        Status = BrowserExtension->ExecuteAction(BROWSER_ACTION_SUBMIT, 0);   
		FreePool(Title);

  if(0){
    Status = gBS->LocateProtocol (
                  &gEfiLogSetupVariableProtocolGuid,
                  NULL,
                  &LogVariable
                  );
    if (!EFI_ERROR(Status)) {

		OperationInfo.VarType= LOG_VAR_NUM;
		OperationInfo.Prompt = L"MemoryMode";		
		OperationInfo.VarInfo.VarStoreName = L"Setup";
		OperationInfo.VarInfo.VarName = "MemoryMode";		
		//OperationInfo.VarInfo.VarOffset= SETUP_OFFSET(MemoryMode);
		
		LogVariable->AppendOpInfo(LogVariable, &OperationInfo);

	do {
			
          Status = LogVariable->ExportLogInfo(LogVariable, &LogInfo);
	} while (!EFI_ERROR(Status));
	LogVariable->CleanLogInfo(LogVariable);
    }
   }

        break;

      case EXIT_KEY_DEFAULT:      
	  	Title= GetToken(STRING_TOKEN (LOAD_DEFAULT_QUESTION), gSetupFormSets[0].HiiHandle);
        SaveFlag = FormDisplay->ConfirmDataChange(Title);
        if (!(SaveFlag & BROWSER_ACTION_SUBMIT)) {
          return EFI_SUCCESS;
        }
		FreePool(Title);

        Status = BrowserExtension->ExecuteAction(BROWSER_ACTION_DEFAULT, 0);
        break;

      case EXIT_KEY_DISCARD:
	  	Title= GetToken(STRING_TOKEN (DISCARD_CHANGE_QUESTION), gSetupFormSets[0].HiiHandle);
        SaveFlag = FormDisplay->ConfirmDataChange(Title);
        if (!(SaveFlag & BROWSER_ACTION_SUBMIT)) {
          return EFI_SUCCESS;
        }
		FreePool(Title);

        Status = BrowserExtension->ExecuteAction(BROWSER_ACTION_DISCARD | BROWSER_ACTION_EXIT, 0);
       *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
        break;

      default:
        break;
    }
  }


  return Status;
}
