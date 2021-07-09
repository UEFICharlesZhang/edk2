/** @file

Copyright (c) 2006 - 2016, GreatWall Corporation.<BR>
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced,
stored in a retrieval system, or transmitted in any form or by any
means without the express written consent of GreatWall Corporation.

File Name:
  SystemPasswordLib.c

Abstract:
  System password library.

Revision History:

**/
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseCryptLib.h>
#include <Protocol/SystemPasswordProtocol.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/SystemPasswordLib.h>
#include <Library/HiiLib.h>
#include <Library/PrintLib.h>

extern UINT8  SystemPasswordLibStrings[];

typedef struct {
  EFI_KEY EFIKey;
  CHAR16  Unicode;
  CHAR16  ShiftedUnicode;
  CHAR16  ScanCode;
} KEY_DESCRIPTOR;

KEY_DESCRIPTOR  KeyDescriptor[] = {
  {EfiKeyC1,    'a',      'A',       0x1e },
  {EfiKeyB5,    'b',      'B',       0x30 },
  {EfiKeyB3,    'c',      'C',       0x2e },
  {EfiKeyC3,    'd',      'D',       0x20 },
  {EfiKeyD3,    'e',      'E',       0x12 },
  {EfiKeyC4,    'f',      'F',       0x21 },
  {EfiKeyC5,    'g',      'G',       0x22 },
  {EfiKeyC6,    'h',      'H',       0x23 },
  {EfiKeyD8,    'i',      'I',       0x17 },
  {EfiKeyC7,    'j',      'J',       0x24 },
  {EfiKeyC8,    'k',      'K',       0x25 },
  {EfiKeyC9,    'l',      'L',       0x26 },
  {EfiKeyB7,    'm',      'M',       0x32 },
  {EfiKeyB6,    'n',      'N',       0x31 },
  {EfiKeyD9,    'o',      'O',       0x18 },
  {EfiKeyD10,   'p',      'P',       0x19 },
  {EfiKeyD1,    'q',      'Q',       0x10 },
  {EfiKeyD4,    'r',      'R',       0x13 },
  {EfiKeyC2,    's',      'S',       0x1f },
  {EfiKeyD5,    't',      'T',       0x14 },
  {EfiKeyD7,    'u',      'U',       0x16 },
  {EfiKeyB4,    'v',      'V',       0x2f },
  {EfiKeyD2,    'w',      'W',       0x11 },
  {EfiKeyB2,    'x',      'X',       0x2d },
  {EfiKeyD6,    'y',      'Y',       0x15 },
  {EfiKeyB1,    'z',      'Z',       0x2c },
  {EfiKeyE1,    '1',      '!',       0x02 },
  {EfiKeyE2,    '2',      '@',       0x03 },
  {EfiKeyE3,    '3',      '#',       0x04 },
  {EfiKeyE4,    '4',      '$',       0x05 },
  {EfiKeyE5,    '5',      '%',       0x06 },
  {EfiKeyE6,    '6',      '^',       0x07 },
  {EfiKeyE7,    '7',      '&',       0x08 },
  {EfiKeyE8,    '8',      '*',       0x09 },
  {EfiKeyE9,    '9',      '(',       0x0a },
  {EfiKeyE10,   '0',      ')',       0x0b },
  {EfiKeyLShift,     0,     0,       0x2a },
  {EfiKeyCapsLock,  0,     0,       0x3a }
};



EFI_STATUS
EFIAPI
Sha256Hash (
  IN  CONST VOID        *Data,
  IN        UINTN       DataLen,
  OUT       UINT8       *Digest,
  IN        UINTN       DigestSize
  )
{
  VOID     *Sha256Ctx;
  UINTN    CtxSize;

  ASSERT(Data != NULL && DataLen != 0 && Digest != NULL && DigestSize >= 32);

  CtxSize = Sha256GetContextSize();
  Sha256Ctx = AllocatePool(CtxSize);
  ASSERT(Sha256Ctx != NULL);

  Sha256Init (Sha256Ctx);
  Sha256Update (Sha256Ctx, Data, DataLen);
  Sha256Final (Sha256Ctx, Digest);

  FreePool(Sha256Ctx);

  return EFI_SUCCESS;
}


UINTN
Unicode2Ascii (
  IN  CHAR16    *String16,
  OUT  UINT8    **String8
  )
{
  UINTN    Length;
  UINTN    Index;
  UINT8    *Buffer;

  Length = StrLen(String16);

  if (0 < Length) {
    Buffer = NULL;
    Buffer = AllocateZeroPool(Length + 1);
    ASSERT (Buffer != NULL);

    for (Index = 0; Index < Length; Index++) {
      Buffer[Index] = (UINT8) (String16[Index]);
    }
  } else {
    Buffer = NULL;
  }

  *String8 = Buffer;
  return Length;
}


UINTN
GetUniqueNumber (
  UINT8    **Number
  )
{
  UINT8         *Buffer = NULL;

  Buffer = AllocateZeroPool(UNIQUE_SYSTEM_NUMBER_LENGTH + 1);
  ASSERT (Buffer != NULL);  

  //
  // todo: Should be unique number.
  //
  CopyMem(Buffer, "87654321", UNIQUE_SYSTEM_NUMBER_LENGTH);

  *Number = Buffer;
  return UNIQUE_SYSTEM_NUMBER_LENGTH;
}


UINTN
EncodePassword (
  IN  UINT8    *Password,
  IN  UINTN    Length,
  OUT  UINT8    **Hash,
  IN  UINTN    Type    //Type :Ascii or Scancode
  )
{
  UINTN    Len;
  UINTN    i,j;
  UINTN    KeyIndex;
  UINTN    Count;
  UINT8   *Buf = NULL;
  UINT8    PasswordArray[64];
  UINT8    HashArray[32];
  BOOLEAN  ShiftPress;
  BOOLEAN  CaplockPress;
  
  Count        = 0;
  ShiftPress   = FALSE;
  CaplockPress = FALSE;
  Len = GetUniqueNumber(&Buf);
  ZeroMem(PasswordArray, sizeof(PasswordArray));
  CopyMem(PasswordArray, Buf, Len);
  FreePool(Buf);


  if(Type){
    //
    //convert Scancode to Ascii
    //
    for( i=0; i<Length; i++ ) 
    { 
      j=i*2;
      if((Password[j] == 0x00)&&(Password[j+1] == 0x00)) break; 
      if('0'<=Password[i]&&Password[i]<='9')Password[i]=(UINT16)(Password[j]-'0');
      else if('a'<=Password[j]&&Password[j]<='f') Password[i]=(UINT16)(Password[j]-'a')+10;       
      else if('A'<=Password[j]&&Password[j]<='F') Password[i]=(UINT16)(Password[j]-'A')+10;         
      else return 0;
      
      if('0'<=Password[j+1]&&Password[j+1]<='9') Password[i]=Password[i]*0x10+(UINT16)(Password[j+1]-'0');         
        else if('a'<=Password[j+1]&&Password[j+1]<='f')Password[i]=Password[i]*0x10+(UINT16)(Password[j+1]-'a')+10;    
        else if('A'<=Password[j+1]&&Password[j+1]<='F') Password[i]=Password[i]*0x10+(UINT16)(Password[j+1]-'A')+10;       
        else return 0;      
    }

    for(i=0;i<(Length/2);i++){
      if(Password[i] == 0x00) break;
        switch(Password[i])
        {
          case 0x2a: 
            //
            //Lshift pressed
            //
            ShiftPress=ShiftPress?FALSE:TRUE;
            break;
          case 0x3a:
            //
            //Capslock pressed
            //
            CaplockPress=CaplockPress?FALSE:TRUE;
            break;
          default:
            for(KeyIndex =0 ;KeyIndex < (sizeof(KeyDescriptor)/sizeof(KEY_DESCRIPTOR));KeyIndex++ )
            {
              if(Password[i] == KeyDescriptor[KeyIndex].ScanCode){
                if(ShiftPress){
                  Password[i-1] = (UINT8)KeyDescriptor[KeyIndex].ShiftedUnicode;
                  ShiftPress=ShiftPress?FALSE:TRUE;
                }else if(CaplockPress){
                       Password[i-1] = (UINT8)KeyDescriptor[KeyIndex].ShiftedUnicode;
                      }else{
                       Password[i] = (UINT8)KeyDescriptor[KeyIndex].Unicode;                    
                      }
                Count++;
                break;
               }
            }
            break;
        }
      }
       Length = Count;
    }

   //
   // Uppercase of input.
   //

    for (i = 0; i < Length; i++) {
      if (Password[i] > 96 && Password[i] <123 ) {
        Password[i] = Password[i] - 32;
      }
    }

  
  ZeroMem(HashArray, sizeof(HashArray));
  Sha256Hash(Password, Length, HashArray, sizeof(HashArray));
  CopyMem(PasswordArray + Len, HashArray, sizeof(HashArray));

  ZeroMem(HashArray, sizeof(HashArray));
  Sha256Hash(PasswordArray, Len + sizeof(HashArray), HashArray, sizeof(HashArray));

  Buf = NULL;
  Buf = AllocateZeroPool(SYSTEM_PASSWORD_HASH_LENGTH + 1);
  ASSERT (Buf != NULL);
  CopyMem(Buf, HashArray, SYSTEM_PASSWORD_HASH_LENGTH);

  *Hash = Buf;
  return SYSTEM_PASSWORD_HASH_LENGTH;
}

BOOLEAN
CompareInputPassword (
  IN  UINT8    *PasswordHash,
  IN  CHAR16    *InputPassword
  )
{
  UINTN    Index;
  UINTN    i;
  UINTN    Len;
  UINT8    *Str;
  UINT8    *InputHash;

  Len = Unicode2Ascii(InputPassword, &Str);
  if (0 < Len) {
    //
    // Uppercase of input.
    //
    for (i = 0; i < Len; i++) {
      if (Str[i] > 96 && Str[i] <123 ) {
        Str[i] = Str[i] - 32;
      }
    }
	
    EncodePassword(Str, Len, &InputHash,0);
    FreePool (Str);

    for (Index = 0; Index < SYSTEM_PASSWORD_HASH_LENGTH; Index++) {
      if (InputHash[Index] != PasswordHash[Index] ) {
        FreePool (InputHash);
        return FALSE;
      }
    }

    FreePool (InputHash);
  } else {

    for (Index = 0; Index < SYSTEM_PASSWORD_HASH_LENGTH; Index++) {
      if (0 != PasswordHash[Index] ) {
        return FALSE;
      }
    }
  }
  return TRUE;
}



/**
  Check Power-On Passwrod.

**/
EFI_STATUS
CheckPopPassword (VOID )
{
  EFI_STATUS                      Status;
  UINTN                           VarSize;
  CHAR16                          *Password = NULL;
  EFI_SYSTEM_PASSWORD_PROTOCOL    *SystemPassword = NULL;
  BOOLEAN                         bPassed;
  BOOLEAN                         bFirstChecked;
  SYSTEM_PASSWORD         PasswordVariable;
  CHAR16                             *HiiFormatString1 = NULL;
  CHAR16                             *HiiFormatString2 = NULL;
  CHAR16                             *HiiFormatString3 = NULL;
  EFI_HII_HANDLE                     HiiHandle = NULL;
  
  VarSize = sizeof (SYSTEM_PASSWORD);
  Status = gRT->GetVariable (
                  SYSTEM_PASSWORD_NAME,
                  &gEfiSystemPasswordVariableGuid,
                  NULL,
                  &VarSize,
                  &PasswordVariable
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (!PasswordVariable.RequirePopOnRestart) {
    return EFI_UNSUPPORTED;
  }
  
  Status = gBS->LocateProtocol (
                  &gEfiSystemPasswordProtocolGuid,
                  NULL,
                  (VOID**)&SystemPassword
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (SystemPassword->GetChecked()) {
    return EFI_SUCCESS;
  }

  if (!SystemPassword->BeHave(PD_POWER_ON)) {
    return EFI_UNSUPPORTED;
  }

  HiiHandle = HiiAddPackages (
                 &gEfiCallerIdGuid,
                 NULL,
                 SystemPasswordLibStrings,
                 NULL
                 );
  ASSERT (HiiHandle != NULL);
  HiiFormatString1 = HiiGetString(HiiHandle, STRING_TOKEN(STR_INFORMATION4), NULL);
  HiiFormatString2 = HiiGetString(HiiHandle, STRING_TOKEN(STR_INFORMATION5), NULL);
  HiiFormatString3 = HiiGetString(HiiHandle, STRING_TOKEN(STR_INFORMATION3), NULL);
  
  HiiRemovePackages (HiiHandle);
  SystemPassword->SetTimes (PD_MAX, VERIFY_ACTION_RESET);
  SystemPassword->SetMode(VERIFY_MODE_INTO_POP);
  bPassed = FALSE;
  bFirstChecked = TRUE;
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  while(!bPassed){
    gST->ConOut->ClearScreen (gST->ConOut);
    if (bFirstChecked) {
      bFirstChecked = FALSE;
      gST->ConOut->SetCursorPosition(gST->ConOut, 0, 0);
      gST->ConOut->OutputString(gST->ConOut, HiiFormatString1);//L"System Security: Power-On Password Required.");

      gST->ConOut->SetCursorPosition(gST->ConOut, 0, 1);
      gST->ConOut->OutputString(gST->ConOut, HiiFormatString2);//L"Enter System Power-On Password and Press <Enter>:");

    } else {

      gST->ConOut->SetCursorPosition(gST->ConOut, 0, 0);
      gST->ConOut->OutputString(gST->ConOut, HiiFormatString3);//L"Invalid Password.");

      gST->ConOut->SetCursorPosition(gST->ConOut, 0, 1);
      gST->ConOut->OutputString(gST->ConOut, HiiFormatString2);//L"Enter System Power-On Password and Press <Enter>:");
    }

    Password = AllocateZeroPool ((MAX_PASSWORD_LENGTH + 1)* sizeof (CHAR16));
    ASSERT (Password);

    Status = SystemPassword->Input (0,2, Password);
    if (SystemPassword->BeHave(PD_ADMIN)) {
      bPassed = SystemPassword->Verify(PD_ADMIN, Password);
      if (!bPassed) {
        if (SystemPassword->BeHave(PD_POWER_ON)) {
          SystemPassword->SetTimes (PD_POWER_ON, VERIFY_ACTION_SUB);
          bPassed = SystemPassword->Verify(PD_POWER_ON, Password);          
          if (bPassed) {		  
            FreePool (Password);
            break;        
          }
        }
      }
    } else {
      bPassed = SystemPassword->Verify(PD_POWER_ON, Password);
    }
    FreePool (Password);
  }

  if (bPassed) {
    VarSize = sizeof (SYSTEM_PASSWORD);
    Status = gRT->GetVariable (
                  SYSTEM_PASSWORD_NAME,
                  &gEfiSystemPasswordVariableGuid,
                  NULL,
                  &VarSize,
                  &PasswordVariable
                  );
    if (EFI_ERROR(Status)) {
      return Status;
    }
    PasswordVariable.PopCheckStatus = 1;
    Status = gRT->SetVariable (
                    SYSTEM_PASSWORD_NAME,
                    &gEfiSystemPasswordVariableGuid,
                     EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    VarSize,
                    &PasswordVariable
                    );
  }
  SystemPassword->SetTimes (PD_MAX, VERIFY_ACTION_RESET);

  gST->ConOut->ClearScreen (gST->ConOut);
  FreePool (HiiFormatString1);
  FreePool (HiiFormatString2);
  FreePool (HiiFormatString3);
  return EFI_SUCCESS;
}



/**
  Check Setup Passwrod.

**/
EFI_STATUS
CheckSetupPassword (VOID ) 
{
  EFI_STATUS    Status;
  EFI_SYSTEM_PASSWORD_PROTOCOL    *SystemPassword = NULL;
  CHAR16      *Password;
  BOOLEAN    bPassed;
  BOOLEAN    bFirstChecked;
  CHAR16                             *HiiFormatString1 = NULL;
  CHAR16                             *HiiFormatString2 = NULL;
  CHAR16                             *HiiFormatString3 = NULL;
  EFI_HII_HANDLE                      HiiHandle = NULL;
  
  Status = gBS->LocateProtocol (
                  &gEfiSystemPasswordProtocolGuid,
                  NULL,
                  (VOID**)&SystemPassword
                  );
  if (EFI_ERROR(Status)) {    
    return Status;
  }
  if (!SystemPassword->BeHave(PD_ADMIN)) {
    return EFI_NOT_READY;
  }
  
  HiiHandle = HiiAddPackages (
                &gEfiCallerIdGuid,
                NULL,
                SystemPasswordLibStrings,
                NULL
                );
  
  ASSERT (HiiHandle != NULL);
  HiiFormatString1 = HiiGetString(HiiHandle, STRING_TOKEN(STR_INFORMATION1), NULL);
  HiiFormatString2 = HiiGetString(HiiHandle, STRING_TOKEN(STR_INFORMATION2), NULL);
  HiiFormatString3 = HiiGetString(HiiHandle, STRING_TOKEN(STR_INFORMATION3), NULL);
  HiiRemovePackages (HiiHandle);
  //
  // Check password.
  //
  bPassed = FALSE;
  bFirstChecked = TRUE;
  SystemPassword->SetMode(VERIFY_MODE_INTO_SETUP);
  SystemPassword->SetTimes (PD_MAX, VERIFY_ACTION_RESET);
  while (!bPassed) {
    gST->ConOut->ClearScreen (gST->ConOut);
    if (bFirstChecked) {
      bFirstChecked = FALSE;

      gST->ConOut->SetCursorPosition(gST->ConOut, 0, 0);
      gST->ConOut->OutputString(gST->ConOut, HiiFormatString1);// L"System Security: Setup Password Required.");

      gST->ConOut->SetCursorPosition(gST->ConOut, 0, 1);
      gST->ConOut->OutputString(gST->ConOut, HiiFormatString2);//L"Enter Setup Password and Press <Enter>:");
    } else {

      gST->ConOut->SetCursorPosition(gST->ConOut, 0, 0);
      gST->ConOut->OutputString(gST->ConOut, HiiFormatString3);//L"Invalid Password.");

      gST->ConOut->SetCursorPosition(gST->ConOut, 0, 1);
      gST->ConOut->OutputString(gST->ConOut, HiiFormatString2);//L"Enter Setup Password and Press <Enter>:");

    }

    Password = AllocateZeroPool ((MAX_PASSWORD_LENGTH + 1)* sizeof (CHAR16));
    ASSERT (Password);
    Status = SystemPassword->Input (0,2, Password);
    if (SystemPassword->BeHave(PD_ADMIN)) {
      bPassed = SystemPassword->Verify(PD_ADMIN, Password);
    }
    FreePool (Password);
  }  	
  SystemPassword->SetTimes (PD_MAX, VERIFY_ACTION_RESET);
  
  gST->ConOut->ClearScreen (gST->ConOut);
  FreePool (HiiFormatString1);
  FreePool (HiiFormatString2);
  FreePool (HiiFormatString3);
  return EFI_SUCCESS;
}

EFI_STATUS
ClearAllPassword ()
{
  EFI_STATUS                    Status = EFI_SUCCESS;  
  EFI_SYSTEM_PASSWORD_PROTOCOL  *SystemPassword = NULL;
  SYSTEM_PASSWORD               PasswordVar;
  UINTN                         VarSize;

  //
  // Get system password protocol.
  //
  Status = gBS->LocateProtocol (
                  &gEfiSystemPasswordProtocolGuid,
                  NULL,
                  (VOID**)&SystemPassword
                  );
  if (EFI_ERROR(Status)) {    
    return Status;
  } 

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

  if (PasswordVar.bHaveAdmin == 1) {
    SystemPassword->Clear(PD_ADMIN);
  }

  if (PasswordVar.bHavePowerOn == 1) {
    SystemPassword->Clear(PD_POWER_ON);
  }
  
  return EFI_SUCCESS;
}
