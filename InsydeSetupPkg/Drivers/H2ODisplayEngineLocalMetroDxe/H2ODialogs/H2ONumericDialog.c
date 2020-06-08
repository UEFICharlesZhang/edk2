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

extern H2O_FORM_BROWSER_D       *mFbDialog;

CHAR16 *mNumericDialogChilds = L""
  L"<VerticalLayout>"
    L"<HorizontalLayout width='300' bkcolor='0xFFF2F2F2' height='41'>"
      L"<Label name='NumericLabel' text='input number:' height='41' padding='12,0,0,0' width='119' fontsize='16' textcolor='0xFF4D4D4D'/>"
      L"<VerticalLayout padding='6,8,6,0' width='181' height='41'>"
        L"<Control/>"
        L"<Control padding='1,1,1,1' width='173' bkcolor='0xFF999999' height='29'>"
          L"<UiEdit name='InputNumericText' focusbkcolor='@menulightcolor' tabstop='true' height='27' width='171' taborder='1' bkcolor='0xFFF2F2F2' valuetype='dec'/>"
        L"</Control>"
        L"<Control/>"
      L"</VerticalLayout>"
    L"</HorizontalLayout>"
    L"<Control height='2'/>"
    L"<Control height='10'/>"
    L"<HorizontalLayout padding='0,0,30,0' childpadding='2' width='300' height='55'>"
      L"<Control/>"
      L"<Button name='Ok' focusbkcolor='@menulightcolor' text='Enter' align='singleline|center' height='30' width='55' taborder='2' fontsize='19' bkcolor='0xFFCCCCCC' textcolor='0xFFFFFFFF'/>"
      L"<Button name='Cancel' focusbkcolor='@menulightcolor' text='Cancel' align='singleline|center' height='30' width='55' taborder='3' fontsize='19' bkcolor='0xFFCCCCCC' textcolor='0xFFFFFFFF'/>"
    L"</HorizontalLayout>"
  L"</VerticalLayout>";

CHAR16 *mNumericDialogWithoutSendFormChilds = L""
  L"<Control name='NumericDialogWithoutSendForm'>"
    L"<VerticalLayout padding='20,30,20,30' bkcolor='@menucolor'>"
      L"<VerticalLayout name='TitleLayout'>"
        L"<Label align='center' textcolor='0xFFFFFFFF' fontsize='19' name='DialogTitle' height='40'/>"
        L"<Control height='15'/>"
        L"<Control height='10' bkimage='@DialogSeparator' bkcolor='0x0' bkimagestyle='center'/>"
        L"<Control height='15'/>"
      L"</VerticalLayout>"
      L"<HorizontalLayout padding='0,0,10,0' name='DialogPasswordInput' height='51'>"
        L"<VerticalLayout padding='6,8,6,8' bkcolor='0xFFF2F2F2' height='41'>"
          L"<Control padding='1,1,1,1' bkcolor='0xFF999999' height='29'>"
            L"<UiEdit padding='7,3,0,3' name='InputNumericText' bkcolor='0xFFF2F2F2' focusbkcolor='@menulightcolor' height='27'/>"
          L"</Control>"
        L"</VerticalLayout>"
      L"</HorizontalLayout>"
      L"<HorizontalLayout childpadding='2' visible='false' name='DialogButtonList' height='30'/>"
    L"</VerticalLayout>"
    L"<Texture name='FormHalo' float='true' height='-1' width='-1' bkimage='@FormHalo' scale9grid='23,26,22,31'/>"
  L"</Control>";

EFI_STATUS
SendNumericChange (
  IN UI_DIALOG                  *Dialog
  )
{
  UI_CONTROL                    *Control;
  CHAR16                        *ValueStr;
  H2O_FORM_BROWSER_Q            *CurrentQ;
  EFI_HII_VALUE                 HiiValue;
  UINT64                        EditValue;
  EFI_STATUS                    Status;

  Control  = UiFindChildByName (Dialog, L"InputNumericText");
  ValueStr = ((UI_LABEL *) Control)->Text;
  CurrentQ = mFbDialog->H2OStatement;

  ZeroMem (&HiiValue, sizeof (HiiValue));
  if (((UI_EDIT *) Control)->ValueType == HEX_VALUE) {
    EditValue = StrToUInt (ValueStr, 16, &Status);
  } else {
    EditValue = StrToUInt (ValueStr, 10, &Status);
  }

  if (EditValue >= ((UI_EDIT *) Control)->MinValue) {
    CopyMem (&HiiValue, &CurrentQ->HiiValue, sizeof (EFI_HII_VALUE));
    HiiValue.Value.u64 = EditValue;
    SendChangeQNotify (0, 0, &HiiValue);
  }

  return EFI_SUCCESS;
}

LRESULT
H2ONumericDialogProc (
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
  CurrentQ = mFbDialog->H2OStatement;
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
    XmlCreateControl (mNumericDialogChilds, Control);

    //
    // set maximum value and display type
    //
    if (mFbDialog->BodyInputCount != 0) {
      Control = UiFindChildByName (Dialog, L"InputNumericText");
      UnicodeSPrint (Str, sizeof (Str), L"%d", CurrentQ->Maximum);
      UiSetAttribute (Control, L"maxvalue", Str);
      UnicodeSPrint (Str, sizeof (Str), L"%d", CurrentQ->Minimum);
      UiSetAttribute (Control, L"minvalue", Str);
      if ((CurrentQ->Flags & EFI_IFR_DISPLAY) == EFI_IFR_DISPLAY_UINT_HEX) {
        UiSetAttribute (Control, L"valuetype", L"hex");
      } else {
        UiSetAttribute (Control, L"valuetype", L"dec");
      }
    }

    if (mFbDialog->ButtonCount == 2) {
      Index = 0;
      Control = UiFindChildByName (Dialog, L"Ok");
      UiSetAttribute (Control, L"text", mFbDialog->ButtonStringArray[Index++]);
      Control = UiFindChildByName (Dialog, L"Cancel");
      UiSetAttribute (Control, L"text", mFbDialog->ButtonStringArray[Index++]);
    }

    if (mFbDialog->BodyInputCount != 0) {
      Control = UiFindChildByName (Dialog, L"InputNumericText");
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
      SendNumericChange (Dialog);
    }
    break;

  case UI_NOTIFY_CARRIAGE_RETURN:
    FocusedWnd = GetFocus ();
    Control    = (UI_CONTROL *) GetWindowLongPtr (FocusedWnd, 0);
    if (StrCmp (Control->Name, L"InputNumericText") == 0) {
      Control = UiFindChildByName (Dialog, L"Ok");
      if (Control == NULL) {
        SendNumericChange (Dialog);
      } else {
        SetFocus (Control->Wnd);
      }
    } else if (StrCmp (Control->Name, L"Ok") == 0) {
      SendNumericChange (Dialog);
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

LRESULT
H2ONumericDialogWithoutSendFormProc (
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
    if (mFbDialog->TitleString != NULL) {
      Control = UiFindChildByName (Dialog, L"NumericDialogTitle");
      UiSetAttribute (Control, L"text", mFbDialog->TitleString);
    }

    if (mFbDialog->BodyInputCount != 0) {
      Control = UiFindChildByName (Dialog, L"InputNumericText");

      UnicodeSPrint (Str, sizeof (Str), L"%d", mFbDialog->H2OStatement->Minimum);
      UiSetAttribute (Control, L"minvalue", Str);
      UnicodeSPrint (Str, sizeof (Str), L"%d", mFbDialog->H2OStatement->Maximum);
      UiSetAttribute (Control, L"maxvalue", Str);

      if ((mFbDialog->H2OStatement->Flags & EFI_IFR_DISPLAY) == EFI_IFR_DISPLAY_UINT_HEX) {
        UiSetAttribute (Control, L"valuetype", L"hex");
      } else {
        UiSetAttribute (Control, L"valuetype", L"dec");
      }
    }

    if (mFbDialog->ButtonCount != 0) {
      ListControl = UiFindChildByName (Dialog, L"DialogButtonList");
      UiSetAttribute (ListControl, L"visible", L"true");

      Control = CreateControl (L"Control", ListControl);
      CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);

      Control = CreateControl (L"Button", ListControl);
      UiApplyAttributeList (Control, L"name='Ok' text='Enter' height='30' width='75' fontsize='19' textcolor='0xFFFFFFFF' align='center' align='singleline' bkcolor='0xFFCCCCCC' focusbkcolor='@menulightcolor'");
      CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);

      Control = CreateControl (L"Button", ListControl);
      UiApplyAttributeList (Control, L"name='Cancel' text='Cancel' height='30' width='75' fontsize='19' textcolor='0xFFFFFFFF' align='center' align='singleline' bkcolor='0xFFCCCCCC' focusbkcolor='@menulightcolor'");
      CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);
    }

    if (mFbDialog->BodyInputCount != 0) {
      Control = UiFindChildByName (Dialog, L"InputNumericText");
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
      SendNumericChange (Dialog);
    }
    break;

  case UI_NOTIFY_CARRIAGE_RETURN:
    FocusedWnd = GetFocus ();
    Control    = (UI_CONTROL *) GetWindowLongPtr (FocusedWnd, 0);
    if (StrCmp (Control->Name, L"InputNumericText") == 0) {
      Control = UiFindChildByName (Dialog, L"Ok");
      if (Control == NULL) {
        SendNumericChange (Dialog);
      } else {
        SetFocus (Control->Wnd);
      }
    } else if (StrCmp (Control->Name, L"Ok") == 0) {
      SendNumericChange (Dialog);
    } else if (StrCmp (Control->Name, L"Cancel") == 0) {
      SendShutDNotify ();
    }
    return 0;

  default:
    return 0;
  }

  return 1;
}
