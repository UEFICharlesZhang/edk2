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

CHAR16 *mStringDialogChilds = L""
  L"<VerticalLayout>"
    L"<HorizontalLayout width='300' bkcolor='0xFFF2F2F2' height='41'>"
      L"<Label name='StringLabel' text='input string:' height='41' padding='12,0,0,0' width='119' fontsize='16' textcolor='0xFF4D4D4D'/>"
      L"<VerticalLayout padding='6,8,6,0' width='181' height='41'>"
        L"<Control/>"
        L"<Control padding='1,1,1,1' width='173' bkcolor='0xFF999999' height='29'>"
          L"<UiEdit name='InputStringText' focusbkcolor='@menulightcolor' tabstop='true' height='27' width='171' taborder='1' bkcolor='0xFFF2F2F2'/>"
        L"</Control>"
        L"<Control/>"
      L"</VerticalLayout>"
    L"</HorizontalLayout>"
    L"<Control height='2'/>"
    L"<Control height='10'/>"
    L"<HorizontalLayout padding='0,0,30,0' childpadding='2' width='300' height='55'>"
      L"<Control/>"
      L"<Button name='Ok' focusbkcolor='@menulightcolor' text='Enter' align='singleline|center' height='30' width='55' taborder='2' fontsize='19' bkcolor='0xFFCCCCCC' textcolor='0xFFFFFFFF' tabstop='true'/>"
      L"<Button name='Cancel' focusbkcolor='@menulightcolor' text='Cancel' align='singleline|center' height='30' width='55' taborder='3' fontsize='19' bkcolor='0xFFCCCCCC' textcolor='0xFFFFFFFF' tabstop='true'/>"
    L"</HorizontalLayout>"
  L"</VerticalLayout>";


EFI_STATUS
SendStringChange (
  IN UI_DIALOG                  *Dialog
  )
{
  UI_CONTROL                    *Control;
  CHAR16                        *String;
  UINTN                         StringSize;
  EFI_HII_VALUE                 HiiValue;

  Control    = UiFindChildByName (Dialog, L"InputStringText");
  String     = ((UI_LABEL *) Control)->Text;
  StringSize = StrSize (String);

  ZeroMem (&HiiValue, sizeof (HiiValue));
  HiiValue.Type      = EFI_IFR_TYPE_STRING;
  HiiValue.BufferLen = (UINT16) StringSize;
  HiiValue.Buffer    = (UINT8 *) AllocateCopyPool (StringSize, String);
  SendChangeQNotify (0, 0, &HiiValue);

  return EFI_SUCCESS;
}

LRESULT
H2OStringDialogProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_DIALOG                     *Dialog;
  UI_CONTROL                    *Control;
  EFI_IMAGE_INPUT               *FormsetImage;
  CHAR16                        Str[20];
  UINTN                         Index;
  HWND                          FocusedWnd;
  H2O_FORM_BROWSER_Q            *CurrentQ;

  Dialog   = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);
  CurrentQ = mFbDialog->H2OStatement;;
  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    if (CurrentQ == NULL) {
      break;
    }

    Control = UiFindChildByName (Dialog, L"DialogTitle");
    if (mFbDialog->TitleString != NULL) {
      UiSetAttribute (Control, L"text", mFbDialog->TitleString);
    } else {
      UiSetAttribute (Control, L"text", CurrentQ->Prompt);
    }

    FormsetImage = GetCurrentFormSetImage ();
    if (FormsetImage != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogImage");
      UnicodeSPrint (Str, sizeof (Str), L"0x%p", FormsetImage);
      UiSetAttribute (Control, L"bkimage", Str);
      UiSetAttribute (Control, L"visible", L"true");
    }

    if (CurrentQ->Help != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogText");
      UiSetAttribute (Control, L"text", CurrentQ->Help);
    }

    Control = UiFindChildByName (Dialog, L"DialogTextScrollView");
    if (Control != NULL) {
      Control->OnSetState = H2OCommonDialogWithHelpOnSetState;
    }

    Control = UiFindChildByName (Dialog, L"Content");
    XmlCreateControl (mStringDialogChilds, Control);

    if (mFbDialog->BodyInputCount != 0) {
      Control = UiFindChildByName (Dialog, L"InputStringText");
      UnicodeSPrint (Str, sizeof (Str), L"%d", CurrentQ->Maximum);
      UiSetAttribute (Control, L"maxlength", Str);
    }

    if (mFbDialog->ButtonCount == 2) {
      Index = 0;
      Control = UiFindChildByName (Dialog, L"Ok");
      UiSetAttribute (Control, L"text", mFbDialog->ButtonStringArray[Index++]);
      Control = UiFindChildByName (Dialog, L"Cancel");
      UiSetAttribute (Control, L"text", mFbDialog->ButtonStringArray[Index++]);
    }

    if (mFbDialog->BodyInputCount != 0) {
      Control = UiFindChildByName (Dialog, L"InputStringText");
    } else {
      Control = UiFindChildByName (Dialog, L"Ok");
    }
    SetFocus (Control->Wnd);

    mTitleVisible = TRUE;
    SetTimer (Wnd, 0, 1, DialogCallback);
    break;

  case UI_NOTIFY_CLICK:
    FocusedWnd = GetFocus ();
    Control  = (UI_CONTROL *) GetWindowLongPtr (FocusedWnd, 0);
    if (StrCmp (Control->Name, L"Cancel") == 0) {
      SendShutDNotify ();
    } else {
      SendStringChange (Dialog);
    }
    break;

  case UI_NOTIFY_CARRIAGE_RETURN:
    FocusedWnd = GetFocus ();
    Control    = (UI_CONTROL *) GetWindowLongPtr (FocusedWnd, 0);
    if (StrCmp (Control->Name, L"InputStringText") == 0) {
      Control = UiFindChildByName (Dialog, L"Ok");
      if (Control == NULL) {
        SendStringChange (Dialog);
      } else {
        SetFocus (Control->Wnd);
      }
    } else if (StrCmp (Control->Name, L"Ok") == 0) {
      SendStringChange (Dialog);
    } else if (StrCmp (Control->Name, L"Cancel") == 0) {
      SendShutDNotify ();
    }
    return 0;

  case WM_HOTKEY:
    if (HIWORD(LParam) == VK_ESCAPE) {
      SendShutDNotify ();
      return 0;
    }
    return 1;

  case WM_DESTROY:
    KillTimer (Wnd, 0);
    return 0;

  default:
    return 0;
  }
  return 1;
}
