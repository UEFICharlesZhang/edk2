/** @file

Copyright (c) 2006 - 2016, Byosoft Corporation.<BR>
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced,
stored in a retrieval system, or transmitted in any form or by any
means without the express written consent of Byosoft Corporation.

**/

#ifndef _PLATFORM_LANGUAGE_LIB_H_
#define _PLATFORM_LANGUAGE_LIB_H_

#ifndef VFRCOMPILE
#include <Uefi.h>
#include <Guid/MdeModuleHii.h>
#include <Protocol/UnicodeCollation.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>
#endif

#define LABEL_CHANGE_LANGUAGE    0x1234
#define LABEL_END    0xffff
#define KEY_CHANGE_LANGUAGE    0x2341

#define LANGUAGE_VALUE_VARIABLE_GUID \
  { \
     0x538f2cf0, 0x1d9c, 0x44a9, { 0x99, 0x14, 0x4, 0xc6, 0xf6, 0x29, 0xd2, 0xce } \
  }

typedef struct _LANGUAGE_VALUE {
  UINT16  Value;
} LANGUAGE_VALUE;

#define LANGUAGE_VALUE_VARSTORE \
    efivarstore LANGUAGE_VALUE, varid = KEY_CHANGE_LANGUAGE, attribute = 2, name  = LangValue, guid  = LANGUAGE_VALUE_VARIABLE_GUID;

//
// This is the VFR compiler generated header file which defines the
// string identifiers.
//
#define LANGUAGE_NAME_STRING_ID     0x0001
 
#ifndef VFRCOMPILE
/**
  Get next language from language code list (with separator ';').

  If LangCode is NULL, then ASSERT.
  If Lang is NULL, then ASSERT.

  @param  LangCode    On input: point to first language in the list. On
                                 output: point to next language in the list, or
                                 NULL if no more language in the list.
  @param  Lang           The first language in the list.

**/
VOID
EFIAPI
GetNextLanguage (
  IN OUT CHAR8      **LangCode,
  OUT CHAR8         *Lang
  );

/**
  Determine the current language that will be used
  based on language related EFI Variables.

  @param LangCodesSettingRequired If required to set LangCode variable

**/
VOID
InitializePlatformLanguage (
  BOOLEAN LangCodesSettingRequired
  );

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
  );

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
  );

/**
  Get current language and store to gLastLanguage.
  Return TRUE if language is changed .

**/
BOOLEAN
BeLanguageChanged (
  VOID
  );
#endif
    
#endif
