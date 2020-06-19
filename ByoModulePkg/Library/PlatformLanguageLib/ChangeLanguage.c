/** @file

Copyright (c) 2006 - 2016, Byosoft Corporation.<BR>
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced,
stored in a retrieval system, or transmitted in any form or by any
means without the express written consent of Byosoft Corporation.

**/

#include "PlatformLanguage.h"


EFI_STRING_ID    *gLanguageToken = NULL;
UINTN    gDefaultLanguageOption = 0;
EFI_HII_HANDLE    gCurrentFormsetHiiHandle = NULL;
CHAR8    *gLastLanguage = NULL;

/**
  Create language select item on a Formset.

  @param HiiHandle         The Hii Handle of Formset
  @param FormSetGuid    The Guid of Formset
  
**/
EFI_STATUS
UpdateLanguageSettingItem (
  EFI_HII_HANDLE    HiiHandle,
  EFI_GUID    *FormSetGuid,
  EFI_FORM_ID     FormId,
  IN EFI_STRING_ID    ItemPrompt,
  IN EFI_STRING_ID    ItemHelp
  )
{
  EFI_STATUS                  Status;
  CHAR8                       *LanguageString;
  CHAR8                       *LangCode;
  CHAR8                       *Lang;
  CHAR8                       *CurrentLang;
  CHAR8                       *BestLanguage;
  UINTN                       OptionCount;
  CHAR16                      *StringBuffer;
  VOID                        *OptionsOpCodeHandle;
  VOID                        *StartOpCodeHandle;
  VOID                        *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL          *StartLabel;
  EFI_IFR_GUID_LABEL          *EndLabel;
  BOOLEAN                     FirstFlag;  
  EFI_GUID    LangValueVariableGuid = LANGUAGE_VALUE_VARIABLE_GUID;
  
  if (NULL == HiiHandle || NULL == FormSetGuid) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Init OpCode Handle and Allocate space for creation of UpdateData Buffer
  //
  gCurrentFormsetHiiHandle = HiiHandle;

  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (OptionsOpCodeHandle != NULL);
  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LABEL_CHANGE_LANGUAGE;

  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = LABEL_END;

  //
  // Collect the languages from what our current Language support is based on our VFR
  //
  LanguageString = HiiGetSupportedLanguages (HiiHandle);
  ASSERT (LanguageString != NULL);
  //
  // Allocate working buffer for RFC 4646 language in supported LanguageString.
  //
  Lang = AllocatePool (AsciiStrSize (LanguageString));
  ASSERT (Lang != NULL);

  CurrentLang = GetEfiGlobalVariable (L"PlatformLang");
  //
  // Select the best language in LanguageString as the default one.
  //
  BestLanguage = GetBestLanguage (
                   LanguageString,
                   FALSE,
                   (CurrentLang != NULL) ? CurrentLang : "",
                   (CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang),
                   LanguageString,
                   NULL
                   );
  //
  // BestLanguage must be selected as it is the first language in LanguageString by default
  //
  ASSERT (BestLanguage != NULL);
  OptionCount = 0;
  LangCode    = LanguageString;
  FirstFlag   = FALSE;

 if (NULL == gLanguageToken) {
    while (*LangCode != 0) {
      GetNextLanguage (&LangCode, Lang);
      OptionCount ++;
    }
    gLanguageToken = AllocatePool (OptionCount * sizeof (EFI_STRING_ID));
    ASSERT (gLanguageToken != NULL);
    FirstFlag = TRUE; 
  }

  //
  //Greate Best Language Option.
  //
  OptionCount = 0;
  LangCode = LanguageString;
  while (*LangCode != 0) {
    GetNextLanguage (&LangCode, Lang);	
    if (AsciiStrCmp (Lang, "uqi") == 0) {
      continue;
    }
    if (FirstFlag) {
      StringBuffer = HiiGetString (HiiHandle, LANGUAGE_NAME_STRING_ID, Lang);
      ASSERT (StringBuffer != NULL);
      //
      // Save the string Id for each language
      //
      gLanguageToken[OptionCount] = HiiSetString (HiiHandle, 0, StringBuffer, NULL);
      FreePool (StringBuffer);
    }

    if (AsciiStrCmp (Lang, BestLanguage) == 0) {
      HiiCreateOneOfOptionOpCode (
        OptionsOpCodeHandle,
        gLanguageToken[OptionCount],
        EFI_IFR_OPTION_DEFAULT,
        EFI_IFR_NUMERIC_SIZE_1,
        (UINT8) OptionCount
        );
        gDefaultLanguageOption = OptionCount;

  gRT->SetVariable(
    L"LangValue",
    &LangValueVariableGuid,
    EFI_VARIABLE_BOOTSERVICE_ACCESS,
    sizeof(LANGUAGE_VALUE),
    &OptionCount
     );

    } else {
      HiiCreateOneOfOptionOpCode (
        OptionsOpCodeHandle,
        gLanguageToken[OptionCount],
        0,
        EFI_IFR_NUMERIC_SIZE_1,
        (UINT8) OptionCount
        );
    }
    OptionCount++;
  }

  if (CurrentLang != NULL) {
    FreePool (CurrentLang);
  }
  FreePool (BestLanguage);
  FreePool (Lang);
  FreePool (LanguageString);

  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    KEY_CHANGE_LANGUAGE,
    KEY_CHANGE_LANGUAGE,
    0,
    ItemPrompt,
    ItemHelp,
    EFI_IFR_FLAG_CALLBACK,
    EFI_IFR_NUMERIC_SIZE_1,
    OptionsOpCodeHandle,
    NULL
    );

  Status = HiiUpdateForm (
             HiiHandle,
             FormSetGuid,
             FormId,
             StartOpCodeHandle,
             EndOpCodeHandle
             );
  ASSERT (Status == EFI_SUCCESS);

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);
  HiiFreeOpCodeHandle (OptionsOpCodeHandle);

  return Status;
}

/**
  The Callback Function of Language Select.
  
**/
EFI_STATUS
SetLanguageCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  CHAR8                         *LanguageString;
  CHAR8                         *LangCode;
  CHAR8                         *Lang;
  UINTN                         Index;
  EFI_STATUS                    Status;
  CHAR8                         *PlatformSupportedLanguages;
  CHAR8                         *BestLanguage;

  if (NULL == gCurrentFormsetHiiHandle) {
    return EFI_NOT_READY;
  }
  
  if (Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD) {
    if (QuestionId == KEY_CHANGE_LANGUAGE) {
      Value->u8 = (UINT8) gDefaultLanguageOption;		
      return EFI_SUCCESS;
    }
  }

  if (Action != EFI_BROWSER_ACTION_CHANGING) {
    //
    // Do nothing for UEFI OPEN/CLOSE Action
    //
    return EFI_SUCCESS;
  }

  switch (QuestionId) {
    case KEY_CHANGE_LANGUAGE:
      //
      // Collect the languages from what our current Language support is based on our VFR
      //
      LanguageString = HiiGetSupportedLanguages (gCurrentFormsetHiiHandle);
      ASSERT (LanguageString != NULL);
      //
      // Allocate working buffer for RFC 4646 language in supported LanguageString.
      //	  
      Lang = AllocatePool (AsciiStrSize (LanguageString));
      ASSERT (Lang != NULL);
      Index = 0;
      LangCode = LanguageString;
      while (*LangCode != 0) {
        GetNextLanguage (&LangCode, Lang);
        if (AsciiStrCmp (Lang, "uqi") == 0) {
         continue;
       }
        if (Index == Value->u8) {
          gDefaultLanguageOption = Index;
          break;
        }
        Index++;
      }

      PlatformSupportedLanguages = GetEfiGlobalVariable (L"PlatformLangCodes");
      if (PlatformSupportedLanguages == NULL) {
        PlatformSupportedLanguages = AllocateCopyPool (
                                       AsciiStrSize ((CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLangCodes)),
                                       (CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLangCodes)
                                       );
        ASSERT (PlatformSupportedLanguages != NULL);
      }
      //
      // Select the best language in platform supported Language.
      //
      BestLanguage = GetBestLanguage (
                       PlatformSupportedLanguages,
                       FALSE,
                       Lang,
                       NULL
                       );
      if (BestLanguage != NULL) {
        Status = gRT->SetVariable (
                        L"PlatformLang",
                        &gEfiGlobalVariableGuid,
                        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                        AsciiStrSize (BestLanguage),
                        BestLanguage
                        );
        ASSERT_EFI_ERROR(Status);
        FreePool (BestLanguage);
      } else {
        ASSERT (FALSE);
      }

      FreePool (PlatformSupportedLanguages);
      FreePool (Lang);
      FreePool (LanguageString);
      break;
	  
    default:
      break;
  }

  return EFI_SUCCESS;
}


/**
  Get current language and store to gLastLanguage.
  Return TRUE if language is changed .

**/
BOOLEAN
BeLanguageChanged (void)
{
  EFI_STATUS    Status;
  UINTN    DataSize = 0;
  CHAR8    *Language = NULL;

  Status = gRT->GetVariable (EFI_PLATFORM_LANG_VARIABLE_NAME, &gEfiGlobalVariableGuid, NULL, &DataSize, Language);
  if (!EFI_ERROR(Status)) {
    return FALSE;
  }
  
  if (Status == EFI_BUFFER_TOO_SMALL) {
    Language = AllocatePool(DataSize + 1);
    if (Language == NULL) {
      return FALSE;
    }
    Status = gRT->GetVariable (EFI_PLATFORM_LANG_VARIABLE_NAME, &gEfiGlobalVariableGuid, NULL, &DataSize, Language);
    if (!EFI_ERROR(Status)) {
      if (NULL != gLastLanguage) {	  
        if (AsciiStriCmp(Language, gLastLanguage)) {	  	        
          gBS->FreePool(gLastLanguage);          
          gLastLanguage = Language;
          return TRUE;
        }
      } else {
        gLastLanguage = Language;
        return FALSE;
      }
    }
    gBS->FreePool(Language); 
  }
    
  return FALSE;
}
