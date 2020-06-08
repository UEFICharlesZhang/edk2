/** @file
  UI ordered list control

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "H2ODisplayEngineLocalMetro.h"
#include "UiControls.h"
#include "MetroUi.h"
#include "MetroDialog.h"
#include <Library/ConsoleLib.h>


EFI_STATUS
SendConfirmPassword (
  IN  UI_DIALOG                 *Dialog
  )
{
  UI_CONTROL                    *Control;
  CHAR16                        *PasswordStr;
  EFI_HII_VALUE                 HiiValue;

  Control = UiFindChildByName (Dialog, L"ConfirmPasswordInput");
  PasswordStr = ((UI_LABEL *) Control)->Text;

  HiiValue.BufferLen = (UINT16) StrSize (PasswordStr);
  HiiValue.Buffer    = AllocatePool (HiiValue.BufferLen);
  ASSERT (HiiValue.Buffer != NULL);
  CopyMem (HiiValue.Buffer, PasswordStr, HiiValue.BufferLen);
  SendChangeQNotify (0, 0, &HiiValue);

  return EFI_SUCCESS;
}


LRESULT
H2OConfirmPasswordProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_DIALOG                     *Dialog;
  UI_CONTROL                    *Control;
  CHAR16                        Str[20];
  HWND                          FocusedWnd;
  UI_CONTROL                    *ListControl;

  Dialog  = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);

  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    if (gFB->CurrentP == NULL) {
      Control = UiFindChildByName (Dialog, L"TitleLayout");
      UiSetAttribute (Control, L"visible", L"false");
    }

    if (mFbDialog->TitleString != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogText");
      UiSetAttribute (Control, L"text", mFbDialog->TitleString);
      Control = UiFindChildByName (Dialog, L"ConfirmNewPasswordLabel");
      UiSetAttribute (Control, L"text", mFbDialog->TitleString);
    }

    if (mFbDialog->BodyInputCount != 0) {
      Control = UiFindChildByName (Dialog, L"DialogPasswordInput");
      UiSetAttribute (Control, L"visible", L"true");

      UnicodeSPrint (Str, sizeof (Str), L"%d", (mFbDialog->ConfirmHiiValue.BufferLen / sizeof (CHAR16) - 1));
      Control = UiFindChildByName (Dialog, L"ConfirmPasswordInput");
      UiSetAttribute (Control, L"maxlength", Str);
    }

    ListControl = UiFindChildByName (Dialog, L"DialogButtonList");
    UiSetAttribute (ListControl, L"visible", L"true");

    Control = CreateControl (L"Control", ListControl);
    CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);

    Control = CreateControl (L"Button", ListControl);
    UiApplyAttributeList (Control, L"name='Ok' text='Enter' height='30' taborder='2' width='75' fontsize='19' textcolor='0xFFFFFFFF' align='center' align='singleline' bkcolor='0xFFCCCCCC' focusbkcolor='@menulightcolor'");
    CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);

    Control = CreateControl (L"Button", ListControl);
    UiApplyAttributeList (Control, L"name='Cancel' text='Cancel' height='30' width='75' taborder='3' fontsize='19' textcolor='0xFFFFFFFF' align='center' align='singleline' bkcolor='0xFFCCCCCC' focusbkcolor='@menulightcolor'");
    CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);

    if (gFB->CurrentP != NULL) {
      mTitleVisible = TRUE;
      SetTimer (Wnd, 0, 1, DialogCallback);
    }

    if (mFbDialog->BodyInputCount != 0) {
      Control = UiFindChildByName (Dialog, L"ConfirmPasswordInput");
    } else {
      Control = UiFindChildByName (Dialog, L"Ok");
    }
    SetFocus (Control->Wnd);
    break;

  case UI_NOTIFY_CLICK:
    FocusedWnd = GetFocus ();
    Control  = (UI_CONTROL *) GetWindowLongPtr (FocusedWnd, 0);
    if (StrCmp (Control->Name, L"Cancel") == 0) {
      SendShutDNotify ();
    } else {
      SendConfirmPassword (Dialog);
    }
    break;

  case UI_NOTIFY_CARRIAGE_RETURN:
    FocusedWnd = GetFocus ();
    Control    = (UI_CONTROL *) GetWindowLongPtr (FocusedWnd, 0);
    if (StrCmp (Control->Name, L"ConfirmPasswordInput") == 0) {
      Control = UiFindChildByName (Dialog, L"Ok");
      SetFocus (Control->Wnd);
    } else if (StrCmp (Control->Name, L"Ok") == 0) {
      SendConfirmPassword (Dialog);
    } else if (StrCmp (Control->Name, L"Cancel") == 0) {
      SendShutDNotify ();
    }
    return 0;

  case WM_DESTROY:
    if (gFB->CurrentP != NULL) {
      KillTimer (Wnd, 0);
    }
    return 0;

  default:
    return 0;
  }

  return 1;
}
