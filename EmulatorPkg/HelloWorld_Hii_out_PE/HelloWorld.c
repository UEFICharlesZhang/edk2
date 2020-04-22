/** @file

  Copyright (c) 2013-2014, ARM Ltd. All rights reserved.<BR>
  Copyright (c) 2017, Linaro. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Library/UefiLib.h>
#include <Library/HiiLib.h>

extern unsigned char HelloWorldStrings[];
 
EFI_STATUS
EFIAPI
MainEntry (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  CHAR16 *String;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_GUID mHelloWordGuid = { 0xfdb23409, 0x6693, 0x470c, { 0x91, 0xb1, 0xe7, 0x2d, 0xcc, 0x50, 0xd, 0xa6 } };

  HiiHandle = HiiAddPackages (&mHelloWordGuid,
                              ImageHandle,
                              HelloWorldStrings,
                              NULL);
  String = HiiGetString (HiiHandle, STRING_TOKEN (STR_EDKII_MENU_TITLE), NULL);

  Print(L"Current String: %s\n",String);

  return EFI_SUCCESS;
}
