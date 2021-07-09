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

  
EFI_STRING_ID 
SetHiiString (
  EFI_HII_HANDLE Handle, 
  STRING_REF     StrRef, 
  CHAR16         *sFormat, ...
  )
{
  STATIC CHAR16 s[1024];
  VA_LIST  Marker;
  EFI_STRING_ID    StringId;

  VA_START (Marker, sFormat);
  UnicodeVSPrint (s, sizeof (s),  sFormat, Marker);
  VA_END (Marker);
    
  StringId = HiiSetString (Handle, StrRef, s, NULL);
  
  return StringId;
}

CHAR16 *
GetToken (
  IN  EFI_STRING_ID                Token,
  IN  EFI_HII_HANDLE               HiiHandle
  )
{
  EFI_STRING  String;

  if (HiiHandle == NULL) {
    return NULL;
  }

  String = HiiGetString (HiiHandle, Token, NULL);
  if (String == NULL) {
    String = AllocateCopyPool (sizeof (L"!"), L"!");
    ASSERT (String != NULL);
  }
  return (CHAR16 *) String;
}

/**
  Main Form Init.

**/
EFI_STATUS
EFIAPI
MainFormInit (
  IN EFI_HII_HANDLE    Handle
  )
{
  EFI_STATUS Status = EFI_SUCCESS;

  SetHiiString (
      Handle,
      STRING_TOKEN(STR_BIOS_VENDOR_VALUE), 
      // L"%s", 
      L"Byosoft"
    );
 
  return Status;
}
