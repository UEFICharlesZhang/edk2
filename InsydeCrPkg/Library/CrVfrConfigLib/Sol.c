/** @file
  This file for CR SOL reference
  
;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "Sol.h"
#include <CrSetupConfig.h>

#include <Protocol/SimpleNetwork.h>

#include <Guid/MdeModuleHii.h>

#include <Library/PrintLib.h>
#include <Library/CrVfrConfigLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/VariableLib.h>
#include <Library/CrBdsLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/HiiExLib.h>


#define SOL_CONFIG_VAR_ATTR (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE)
#define IPv4_MAC_LENGTH     6
#define IPv4_IP_LENGTH      4
#define MAX_SCU_TEXT_SIZE   50

#define IsDigital(c)     ((c)>= '0' && (c) <= '9')
#define IsAlphabet(c)    (((c)>= 'A' && (c) <= 'Z') || ((c)>= 'a' && (c) <= 'z'))
#define IsAfAlphabet(c)  (((c)>= 'A' && (c) <= 'F') || ((c)>= 'a' && (c) <= 'f'))
#define IsText(c)        ((c)>= 0x20 && (c) <= 0x7E)
#define NET_IS_DIGIT(Ch) (('0' <= (Ch)) && ((Ch) <= '9'))

typedef BOOLEAN (*VALIDATE_FUNC)(CHAR16 code);

EFI_HII_HANDLE                    mHiiHandle;
CHAR16                            gMacString[30] = {0};
CR_SOL_CONFIG                     gSolConfigNvData;
H2O_DIALOG_PROTOCOL               *gH2oDialog = NULL;

GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR8  SolHexString[] = "0123456789ABCDEFabcdef";

CHAR16 *gDialogTitle [] = {
                            L"Set IP Address",
                            L"Invalid IP address !",
                            L"Set Subnet mask",
                            L"Invalid Subnet Mask !",
                            L"Set Gateway IP address",
                            L"Invalid Gateway IP address !"          
                          };

EFI_STATUS
TextInputDialog (
  IN  CHAR16          *Title,
  IN  VALIDATE_FUNC   IsValidChar,
  IN  UINTN           FieldLen,
  IN  UINTN           BufferSize,
  OUT CHAR16          *Buffer
  );

VOID
ShowErrorMessage (
  CHAR16    *Str
  )
{
  EFI_STATUS          Status;
  EFI_INPUT_KEY       Key;

  if (gH2oDialog == NULL) {
    Status = gBS->LocateProtocol (&gH2ODialogProtocolGuid, NULL, (VOID **)&gH2oDialog);
    if (EFI_ERROR(Status)) {
      return ;
    }
  }
  
  gH2oDialog->ConfirmDialog (
                  2,         // NumberOfLines
                  FALSE,     // HotKey
                  0,         // MaximumStringSize
                  NULL,      // StringBuffer
                  &Key,      // KeyValue
                  Str        // String
                  );

}

BOOLEAN
VisibleCharCheckFun (
  CHAR16    c
  )
{
  if (c >= 0x20 && c < 0x7f) {
    return TRUE;
  }

  return FALSE;
}

BOOLEAN
Ipv4CharCheckFunc (
  CHAR16    c
  )
{
  if (IsDigital(c)) {
    return TRUE;
  }

  if (c == '.') {
    return TRUE;
  }

  return FALSE;
}

BOOLEAN
MacCharCheckFunc(
  CHAR16    c
  )
{
  if (IsDigital(c)) {
    return TRUE;
  }

  if (c == '.') {
    return TRUE;
  }

  if (IsAfAlphabet (c)) {
    return TRUE;
  }

  return FALSE;
}


EFI_STATUS
UnicodeStrToIp (
  IN  CHAR16            *Str,
  OUT EFI_IPv4_ADDRESS  *Ip
  )
{
  UINTN Index;
  UINTN Number;

  Index = 0;

  while (*Str) {

    if (Index > 3) {
      return EFI_INVALID_PARAMETER;
    }

    Number = 0;
    while (NET_IS_DIGIT (*Str)) {
      Number = Number * 10 + (*Str - '0');
      Str++;
    }

    if (Number > 0xFF) {
      return EFI_INVALID_PARAMETER;
    }

    Ip->Addr[Index] = (UINT8) Number;

    if ((*Str != '\0') && (*Str != '.')) {
      //
      // The current character should be either the NULL terminator or
      // the dot delimiter.
      //
      return EFI_INVALID_PARAMETER;
    }

    if (*Str == '.') {
      //
      // Skip the delimiter.
      //
      Str++;
    }

    Index++;
  }

  if (Index != 4) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
UnicodeStrToMac (
  IN  CHAR16            *Str,
  OUT EFI_MAC_ADDRESS   *Mac
  )
{
  UINTN Index;
  UINTN Number;

  Index = 0;

  while (*Str) {

    if (Index > 5) {
      return EFI_INVALID_PARAMETER;
    }
    
    Number = 0;
    while (NET_IS_DIGIT (*Str) || IsAfAlphabet(*Str)) {

      if(NET_IS_DIGIT (*Str)) {
        Number = Number * 16 + (*Str - '0');
      } else {
        if ((*Str >= 'A') && (*Str <= 'F')) {
          Number = Number * 16 + (*Str - 'A') + 10;
        } else {
          Number = Number * 16 + (*Str - 'a') + 10;
        }
      }
      Str++;
    }

    if (Number > 0xFF) {
      return EFI_INVALID_PARAMETER;
    }

    Mac->Addr[Index] = (UINT8) Number;

    if ((*Str != '\0') && (*Str != '.')) {
      //
      // The current character should be either the NULL terminator or
      // the dot delimiter.
      //
      return EFI_INVALID_PARAMETER;
    }

    if (*Str == '.') {
      //
      // Skip the delimiter.
      //
      Str++;
    }

    Index++;
  }

  if (Index != 6) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}


VOID
SolMacAddrToStr (
  IN  EFI_MAC_ADDRESS  *Mac,
  IN  UINT32           Len,
  OUT CHAR16           *Str
  )
{
  UINT32  Index;
  CHAR16  *String;

  for (Index = 0; Index < Len; Index++) {
    Str[3 * Index]      = (CHAR16) SolHexString[(Mac->Addr[Index] >> 4) & 0x0F];
    Str[3 * Index + 1]  = (CHAR16) SolHexString[Mac->Addr[Index] & 0x0F];
    Str[3 * Index + 2]  = L'-';
  }

  String = &Str[3 * Index - 1] ;

  *String = L'\0';
}

CHAR16 *
ByteToStr (
  IN  UINT8     Byte,
  OUT CHAR16    *Str
  )
{
  UINT16     *DecimalStr;
  BOOLEAN    SkipMiddleZero;


  SkipMiddleZero = TRUE;
  DecimalStr = Str;
  
  *Str = L'0' + Byte / 100;
  
  if (*Str != L'0') {
    Byte = Byte % 100;
    Str++;
    SkipMiddleZero = FALSE;
  }

  *Str = L'0' + Byte / 10;
  
  if (*Str != L'0' || !SkipMiddleZero) {
    Byte = Byte % 10;
    Str++;
  }

  *Str = L'0' + Byte;
  
  Str++;

  *Str = 0;

  return Str;
}

VOID
IpAddrToStr (
  IN  EFI_IPv4_ADDRESS  *Ip,
  OUT CHAR16            *Str
  )
{
  UINTN       Index;
  
  for (Index = 0; Index < IPv4_IP_LENGTH; Index++) {

    Str = ByteToStr (Ip->Addr[Index], Str);
    
    if (Index < IPv4_IP_LENGTH - 1)  {
      *Str++ = L'.';
	}
  }
}

VOID
MacAddrToStr (
  IN  EFI_MAC_ADDRESS   *Mac,
  OUT CHAR16            *Str
  )
{
  UINTN       Index;
  
  for (Index = 0; Index < IPv4_MAC_LENGTH; Index++) {

    Str = ByteToStr (Mac->Addr[Index], Str);
    
    if (Index < IPv4_MAC_LENGTH - 1) {
      *Str++ = L'.';
	}
  }
}

VOID
UpdateFormSolAddr (
  EFI_IPv4_ADDRESS  *Address,
  STRING_REF        StringRef
  )
{
  CHAR16            UniStr[MAX_SCU_TEXT_SIZE];

  IpAddrToStr (Address, UniStr);
  HiiSetString (mHiiHandle, StringRef, UniStr, NULL);
}

VOID
UpdateFormSolMacAddr (
  EFI_MAC_ADDRESS   *MacAddr,
  STRING_REF        StringRef
  )
{
  CHAR16            UniStr[MAX_SCU_TEXT_SIZE];

  SolMacAddrToStr (MacAddr, IPv4_MAC_LENGTH, UniStr);
  HiiSetString (mHiiHandle, StringRef, UniStr, NULL);
}


VOID
UpdateFormSolStr (
  CHAR16            *Str,
  STRING_REF        StringRef
  )
{
  HiiSetString (mHiiHandle, StringRef, Str, NULL);
}

VOID
SolUpdateEntrys (
  CR_SERIAL_DEV_INFO       *CrIfrNvdata
  )
{
  CR_SOL_CONFIG            *SolConfigNvdata;

  SolConfigNvdata = &CrIfrNvdata->CrSolConfig;

  UpdateFormSolAddr (&SolConfigNvdata->LocalIp, CrIfrNvdata->CrSolStrId[SOL_IP_ADDR_STR_INDEX]);

  UpdateFormSolAddr (&SolConfigNvdata->SubnetMask, CrIfrNvdata->CrSolStrId[SOL_MASK_STR_INDEX]);

  UpdateFormSolAddr (&SolConfigNvdata->Gateway, CrIfrNvdata->CrSolStrId[SOL_GATEWAY_STR_INDEX]);

  UpdateFormSolAddr (&SolConfigNvdata->AdminIp, CrIfrNvdata->CrSolStrId[SOL_ADMIN_IP_STR_INDEX]);

  UpdateFormSolStr (SolConfigNvdata->UserName, CrIfrNvdata->CrSolStrId[SOL_LOGIN_ACCOUNT_STR_INDEX]);
 
  UpdateFormSolStr (SolConfigNvdata->Password, CrIfrNvdata->CrSolStrId[SOL_LOGIN_PASSWORD_STR_INDEX]);
  
  UpdateFormSolMacAddr (&SolConfigNvdata->AdminMac, CrIfrNvdata->CrSolStrId[SOL_ADMIN_MAC_STR_INDEX]);
}


EFI_STATUS
SolConfigCallback (
  EFI_QUESTION_ID          QuestionId,
  CR_SERIAL_DEV_INFO       *CrIfrNvdata
  )
{
  EFI_STATUS            Status;
  CHAR16                StringBuffer[50];
  EFI_IPv4_ADDRESS      Ip;
  EFI_MAC_ADDRESS       Mac;
  CR_SOL_CONFIG         *SolConfigNvdata;

  SolConfigNvdata = &CrIfrNvdata->CrSolConfig;
  
  switch (QuestionId) {
  case KEY_SOL_PORT_IP :
    Status = TextInputDialog(L"Set IP Address", Ipv4CharCheckFunc, 15, 16, StringBuffer);
    if (Status == EFI_SUCCESS) {
      Status = UnicodeStrToIp (StringBuffer, &Ip);
      if (Status == EFI_SUCCESS) {
        SolConfigNvdata->LocalIp = Ip;
        UpdateFormSolAddr (&Ip, CrIfrNvdata->CrSolStrId[SOL_IP_ADDR_STR_INDEX]);
        Status = EFI_SUCCESS;
      }
      else {
        ShowErrorMessage (L"Invalid IP address !");
        Status = EFI_UNSUPPORTED;
      }
    }
    break;
    
  case KEY_SOL_PORT_MASK :
    Status = TextInputDialog(L"Set Subnet mask", Ipv4CharCheckFunc, 15, 16, StringBuffer);
    if (Status == EFI_SUCCESS) {
      Status = UnicodeStrToIp (StringBuffer, &Ip);
      if (Status == EFI_SUCCESS) {
        SolConfigNvdata->SubnetMask = Ip;
        UpdateFormSolAddr (&Ip, CrIfrNvdata->CrSolStrId[SOL_MASK_STR_INDEX]);
        Status = EFI_SUCCESS;
      }
      else {
        ShowErrorMessage (L"Invalid Subnet Mask !");
        Status = EFI_UNSUPPORTED;
      }
    }
    break;
    
  case KEY_SOL_PORT_GATEWAY :
    Status = TextInputDialog(L"Set Gateway IP address", Ipv4CharCheckFunc, 15, 16, StringBuffer);
    if (Status == EFI_SUCCESS) {
      Status = UnicodeStrToIp (StringBuffer, &Ip);
      if (Status == EFI_SUCCESS) {
        SolConfigNvdata->Gateway = Ip;
        UpdateFormSolAddr (&Ip, CrIfrNvdata->CrSolStrId[SOL_GATEWAY_STR_INDEX]);
        Status = EFI_SUCCESS;
      }
      else {
        ShowErrorMessage (L"Invalid Gateway IP address !");
        Status = EFI_UNSUPPORTED;
      }
    }
    break;

  case KEY_SOL_ADMIN_IP :
    Status = TextInputDialog(L"Set Adminstrator IP address", Ipv4CharCheckFunc, 15, 16, StringBuffer);
    if (Status == EFI_SUCCESS) {
      Status = UnicodeStrToIp (StringBuffer, &Ip);
      if (Status == EFI_SUCCESS) {
        SolConfigNvdata->AdminIp = Ip;
        UpdateFormSolAddr (&Ip, CrIfrNvdata->CrSolStrId[SOL_ADMIN_IP_STR_INDEX]);
        Status = EFI_SUCCESS;
      }
      else {
        ShowErrorMessage (L"Invalid Adminstrator IP address !");
        Status = EFI_UNSUPPORTED;
      }
    }
    break;  

  case KEY_SOL_ADMIN_MAC :
    Status = TextInputDialog(L"Set Adminstrator MAC address", MacCharCheckFunc, 17, 18, StringBuffer);
    if (Status == EFI_SUCCESS) {
      ZeroMem (&Mac, sizeof(EFI_MAC_ADDRESS));
      Status = UnicodeStrToMac (StringBuffer, &Mac);
      if (Status == EFI_SUCCESS) {
        SolConfigNvdata->AdminMac = Mac;
        UpdateFormSolMacAddr (&Mac, CrIfrNvdata->CrSolStrId[SOL_ADMIN_MAC_STR_INDEX]);
        Status = EFI_SUCCESS;
      }
      else {
        ShowErrorMessage (L"Invalid Adminstrator MAC address !");
        Status = EFI_UNSUPPORTED;
      }
    }
    break;  


  case KEY_SOL_LOGIN_NAME :
    Status = TextInputDialog(L"Set Login Name", VisibleCharCheckFun, 12, 13, StringBuffer);
    if (Status == EFI_SUCCESS) {
      StrCpy(SolConfigNvdata->UserName, StringBuffer);
      UpdateFormSolStr (StringBuffer, CrIfrNvdata->CrSolStrId[SOL_LOGIN_ACCOUNT_STR_INDEX]);
      Status = EFI_SUCCESS;
    }
    else {
      ShowErrorMessage (L"Invalid user name !");
      Status = EFI_UNSUPPORTED;
    }
    break;

  case KEY_SOL_LOGIN_PASSWORD :
    Status = TextInputDialog(L"Set Password", VisibleCharCheckFun, 12, 13, StringBuffer);
    if (Status == EFI_SUCCESS) {
      StrCpy(SolConfigNvdata->Password, StringBuffer);
      UpdateFormSolStr (StringBuffer, CrIfrNvdata->CrSolStrId[SOL_LOGIN_PASSWORD_STR_INDEX]);
      Status = EFI_SUCCESS;
    }
    else {
      ShowErrorMessage (L"Invalid password !");
      Status = EFI_UNSUPPORTED;
    }
    break;
    
  default:
    break;
  }
  return EFI_SUCCESS;
}


VOID
SolConfigSetToDefault (
  CR_SOL_CONFIG       *SolConfigNvdata
  )
{
  EFI_CR_POLICY_PROTOCOL      *CRPolicy;
  EFI_STATUS                  Status;  

  Status = gBS->LocateProtocol (&gEfiCRPolicyProtocolGuid, NULL, (VOID **)&CRPolicy);
  if (EFI_ERROR(Status)) {
    return;
  }

  CopyMem ( SolConfigNvdata, &CRPolicy->CrSolDefaultConfig, sizeof (CR_SOL_CONFIG));
    
}

EFI_STATUS
InitCrSolFormLabel (
  CR_SERIAL_DEV_INFO        *CrIfrNvdata
  )
{
  EFI_STATUS                 Status;
  EFI_IFR_GUID_LABEL         *CrSolFormLabel;
  VOID						 *StartOpCodeHandle;
  EFI_STRING_ID              StrTokenID;
  EFI_STRING_ID              PromptStrID;
  EFI_STRING_ID              PromptStrID2;
  CHAR16                     *BlankStr;
  CHAR16                     *PromptStr;
  CHAR16                     *PromptStr2;
  UINTN                      PromptStrSize;
  CR_SOL_CONFIG              *SolConfigNvdata;

  SolConfigNvdata = &CrIfrNvdata->CrSolConfig;
  
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  
  CrSolFormLabel = (EFI_IFR_GUID_LABEL*) HiiCreateGuidOpCode (
                                              StartOpCodeHandle,
                                              &gEfiIfrTianoGuid,
                                              NULL,
                                              sizeof (EFI_IFR_GUID_LABEL)
                                              );

  CrSolFormLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  CrSolFormLabel->Number       = CR_SOL_FORM_LABEL;

  BlankStr = L" ";
  StrTokenID = HiiSetString (mHiiHandle, 0, BlankStr, NULL);
  PromptStrSize = 0x100;
  PromptStr = AllocatePool (PromptStrSize);
  PromptStr2 = AllocatePool (PromptStrSize);
  
  //
  // Admin IP option
  //
  ZeroMem (PromptStr, PromptStrSize);
  UnicodeSPrint (
      PromptStr,
      PromptStrSize, 
      L"  Administrator IP"
      );
  
  ZeroMem (PromptStr2, PromptStrSize);
  IpAddrToStr (&SolConfigNvdata->AdminIp, PromptStr2); 
  PromptStrID = HiiSetString (mHiiHandle, 0, PromptStr, NULL);
  PromptStrID2 = HiiSetString (mHiiHandle, 0, PromptStr2, NULL);
  CrIfrNvdata->CrSolStrId[SOL_ADMIN_IP_STR_INDEX] = PromptStrID2;
  HiiCreateActionOpCodeEx (
    StartOpCodeHandle,
    (EFI_QUESTION_ID) KEY_SOL_ADMIN_IP,
    PromptStrID,
    StrTokenID,
    PromptStrID2,
    EFI_IFR_FLAG_CALLBACK,
    0
    );

  //
  // Admin MAC option
  //
  ZeroMem (PromptStr, PromptStrSize);
  UnicodeSPrint (
      PromptStr,
      PromptStrSize, 
      L"  Administrator MAC"
      );

  ZeroMem (PromptStr2, PromptStrSize);
  MacAddrToStr (&SolConfigNvdata->AdminMac, PromptStr2); 
  PromptStrID = HiiSetString (mHiiHandle, 0, PromptStr, NULL);  
  PromptStrID2 = HiiSetString (mHiiHandle, 0, PromptStr2, NULL);
  CrIfrNvdata->CrSolStrId[SOL_ADMIN_MAC_STR_INDEX] = PromptStrID2;
  HiiCreateActionOpCodeEx (
    StartOpCodeHandle,
    (EFI_QUESTION_ID) KEY_SOL_ADMIN_MAC,
    PromptStrID,
    StrTokenID,
    PromptStrID2,
    EFI_IFR_FLAG_CALLBACK,
    0
    );

  //
  // Login Name option
  //
  ZeroMem (PromptStr, PromptStrSize);
  UnicodeSPrint (
      PromptStr,
      PromptStrSize, 
      L"  Login Name"
      );
    
  PromptStrID = HiiSetString (mHiiHandle, 0, PromptStr, NULL);
  PromptStrID2 = HiiSetString (mHiiHandle, 0, SolConfigNvdata->UserName, NULL);
  CrIfrNvdata->CrSolStrId[SOL_LOGIN_ACCOUNT_STR_INDEX] = PromptStrID2;
  HiiCreateActionOpCodeEx (
    StartOpCodeHandle,
    (EFI_QUESTION_ID) KEY_SOL_LOGIN_NAME,
    PromptStrID,
    StrTokenID,
    PromptStrID2,
    EFI_IFR_FLAG_CALLBACK,
    0
    );

  //
  // Login Password option
  //
  ZeroMem (PromptStr, PromptStrSize);
  UnicodeSPrint (
      PromptStr,
      PromptStrSize, 
      L"  Password"
      );
    
  PromptStrID = HiiSetString (mHiiHandle, 0, PromptStr, NULL);
  PromptStrID2 = HiiSetString (mHiiHandle, 0, SolConfigNvdata->Password, NULL);
  CrIfrNvdata->CrSolStrId[SOL_LOGIN_PASSWORD_STR_INDEX] = PromptStrID2;
  HiiCreateActionOpCodeEx (
    StartOpCodeHandle,
    (EFI_QUESTION_ID) KEY_SOL_LOGIN_PASSWORD,
    PromptStrID,
    StrTokenID,
    PromptStrID2,
    EFI_IFR_FLAG_CALLBACK,
    0
    );
  
  Status = HiiUpdateForm (
             mHiiHandle,
             NULL,
             CONSOLE_REDIRECTION_NET_FORM_ID,
             StartOpCodeHandle,
             NULL
             );

  HiiFreeOpCodeHandle (StartOpCodeHandle);

  return Status;
}

EFI_STATUS
InitCrSolStaticIpLabel (
  CR_SERIAL_DEV_INFO        *CrIfrNvdata
  )
{
  EFI_IFR_GUID_LABEL         *CrSolFormLabel;
  VOID						 *StartOpCodeHandle;
  EFI_STRING_ID              StrTokenID;
  EFI_STRING_ID              PromptStrID;
  EFI_STRING_ID              PromptStrID2;
  CHAR16                     *BlankStr;
  CHAR16                     *PromptStr;
  CHAR16                     *PromptStr2;
  UINTN                      PromptStrSize;
  CR_SOL_CONFIG              *SolConfigNvdata;

  SolConfigNvdata = &CrIfrNvdata->CrSolConfig;
  
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  
  CrSolFormLabel = (EFI_IFR_GUID_LABEL*) HiiCreateGuidOpCode (
                                              StartOpCodeHandle,
                                              &gEfiIfrTianoGuid,
                                              NULL,
                                              sizeof (EFI_IFR_GUID_LABEL)
                                              );

  CrSolFormLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  CrSolFormLabel->Number       = CR_SOL_STATIC_IP_LABEL;

  BlankStr = L" ";
  StrTokenID = HiiSetString (mHiiHandle, 0, BlankStr, NULL);
  PromptStrSize = 0x100;
  PromptStr = AllocatePool (PromptStrSize);
  PromptStr2 = AllocatePool (PromptStrSize);
  
  //
  //  IP Address option
  //
  ZeroMem (PromptStr, PromptStrSize);
  UnicodeSPrint (
      PromptStr,
      PromptStrSize, 
      L"    IP Address"
      );

  ZeroMem (PromptStr2, PromptStrSize); 
  IpAddrToStr (&SolConfigNvdata->LocalIp, PromptStr2);
  PromptStrID = HiiSetString (mHiiHandle, 0, PromptStr, NULL);
  PromptStrID2 = HiiSetString (mHiiHandle, 0, PromptStr2, NULL);
  CrIfrNvdata->CrSolStrId[SOL_IP_ADDR_STR_INDEX] = PromptStrID2;
  HiiCreateActionOpCodeEx (
    StartOpCodeHandle,
    (EFI_QUESTION_ID) KEY_SOL_PORT_IP,
    PromptStrID,
    StrTokenID,
    PromptStrID2,
    EFI_IFR_FLAG_CALLBACK,
    0
    );

  //
  //  Subnet Mask option
  //
  ZeroMem (PromptStr, PromptStrSize);
  UnicodeSPrint (
      PromptStr,
      PromptStrSize, 
      L"    Subnet Mask"
      );

  ZeroMem (PromptStr2, PromptStrSize);
  IpAddrToStr (&SolConfigNvdata->SubnetMask, PromptStr2);  
  PromptStrID = HiiSetString (mHiiHandle, 0, PromptStr, NULL);
  PromptStrID2 = HiiSetString (mHiiHandle, 0, PromptStr2, NULL);
  CrIfrNvdata->CrSolStrId[SOL_MASK_STR_INDEX] = PromptStrID2;
  HiiCreateActionOpCodeEx (
    StartOpCodeHandle,
    (EFI_QUESTION_ID) KEY_SOL_PORT_MASK,
    PromptStrID,
    StrTokenID,
    PromptStrID2,
    EFI_IFR_FLAG_CALLBACK,
    0
    );

  //
  //  Gateway option
  //
  ZeroMem (PromptStr, PromptStrSize);
  UnicodeSPrint (
      PromptStr,
      PromptStrSize, 
      L"    Gateway"
      );

  ZeroMem (PromptStr2, PromptStrSize);
  IpAddrToStr (&SolConfigNvdata->Gateway, PromptStr2);    
  PromptStrID = HiiSetString (mHiiHandle, 0, PromptStr, NULL);
  PromptStrID2 = HiiSetString (mHiiHandle, 0, PromptStr2, NULL);
  CrIfrNvdata->CrSolStrId[SOL_GATEWAY_STR_INDEX] = PromptStrID2;
  HiiCreateActionOpCodeEx (
    StartOpCodeHandle,
    (EFI_QUESTION_ID) KEY_SOL_PORT_GATEWAY,
    PromptStrID,
    StrTokenID,
    PromptStrID2,
    EFI_IFR_FLAG_CALLBACK,
    0
    );
  
  HiiUpdateForm (
    mHiiHandle,
    NULL,
    CONSOLE_REDIRECTION_NET_FORM_ID,
    StartOpCodeHandle,
    NULL
    );

  HiiFreeOpCodeHandle (StartOpCodeHandle);

  return EFI_SUCCESS;
}


EFI_STATUS
SolConfigFormInit (
  EFI_HII_HANDLE            HiiHandle,
  CR_SERIAL_DEV_INFO        *CrIfrNvdata
  )
{
  EFI_STATUS  Status;
  
  mHiiHandle = HiiHandle;

  InitCrSolStaticIpLabel (CrIfrNvdata);
  Status = InitCrSolFormLabel (CrIfrNvdata);
  
  return Status;
}
