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

CHAR16 *mConfirmPageDialogChilds = L""
  L"<VerticalLayout padding='20,20,0,20' bkcolor='@menucolor' name='ConfirmPageDialog'>"
    L"<VerticalLayout name='TitleLayout' height='70'>"
      L"<Label padding='0,0,30,0' textcolor='0xFFFFFFFF' fontsize='29' name='DialogTitle'/>"
    L"</VerticalLayout>"
    L"<VerticalLayout padding='0,0,20,0' vscrollbar='false'>"
      L"<Label textcolor='0xFFFFFFFF' fontsize='19' name='ConfirmPageDialogText'/>"
    L"</VerticalLayout>"
    L"<HorizontalLayout height='30'>"
      L"<Label align='center' textcolor='0xFFFFFFFF' fontsize='20' name='ConfirmPageConfirmString'/>"
    L"</HorizontalLayout>"
    L"<HorizontalLayout height='50'>"
      L"<HorizontalLayout padding='10,0,0,0' childpadding='2' name='DialogButtonList'/>"
    L"</HorizontalLayout>"
  L"</VerticalLayout>";


LRESULT
H2OConfirmPageDialogProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_DIALOG                                *Dialog;
  UI_CONTROL                               *Control;
  EFI_STATUS                               Status;
  UI_CONTROL                               *DialogControl;
  UI_CONTROL                               *FocusControl;
  UINT32                                   Index;
  CHAR16                                   ButtonWidthStr[20];
  INTN                                     Result;

  Dialog  = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);

  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    if (mFbDialog->TitleString != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogTitle");
      UiSetAttribute (Control, L"text", mFbDialog->TitleString);
    }

    Control = UiFindChildByName (Dialog, L"ConfirmPageConfirmString");
    if (mFbDialog->BodyStringArray[0] != NULL) {
      UiSetAttribute (Control, L"text", mFbDialog->BodyStringArray[0]);
    }

    Control = UiFindChildByName (Dialog, L"ConfirmPageDialogText");
    if (mFbDialog->BodyStringArray[1] != NULL) {
      UiSetAttribute (Control, L"text", mFbDialog->BodyStringArray[1]);
    }

    if (mFbDialog->ButtonCount == 0) {
      return EFI_SUCCESS;
    }

    DialogControl = UiFindChildByName (Dialog, L"DialogButtonList");

    Control = CreateControl (L"Control", DialogControl);
    CONTROL_CLASS(DialogControl)->AddChild (DialogControl, Control);

    UnicodeSPrint (ButtonWidthStr, sizeof (ButtonWidthStr), L"%d", GetButtonWidthByStr ());
    FocusControl = NULL;
    for (Index = 0; Index < mFbDialog->ButtonCount; Index++) {
      if (mFbDialog->ButtonStringArray[Index] == NULL) {
        continue;
      }

      Control = CreateControl (L"Button", DialogControl);
      UiSetAttribute (Control, L"text",  mFbDialog->ButtonStringArray[Index]);
      SetWindowLongPtr (Control->Wnd, GWLP_USERDATA, (INTN) Index);

      UiApplyAttributeList (Control, L"name='Button' height='30' fontsize='19' textcolor='0xFFFFFFFF' align='center' align='singleline' bkcolor='0xFFCCCCCC' focusbkcolor='@menulightcolor'");
      UiSetAttribute (Control, L"width", ButtonWidthStr);

      Status = CompareHiiValue (&mFbDialog->ButtonHiiValueArray[Index], &mFbDialog->ConfirmHiiValue, &Result);
      if (!EFI_ERROR(Status) && Result == 0) {
        FocusControl = Control;
      }
      CONTROL_CLASS(DialogControl)->AddChild (DialogControl, Control);
    }
    if (FocusControl != NULL) {
      SetFocus (FocusControl->Wnd);
    }

    Control = CreateControl (L"Control", DialogControl);
    CONTROL_CLASS(DialogControl)->AddChild (DialogControl, Control);
    break;

  case UI_NOTIFY_CLICK:
    Control = (UI_CONTROL *) WParam;
    Index = (UINT32) GetWindowLongPtr (Control->Wnd, GWLP_USERDATA);
    SendChangeQNotify (0, 0, &mFbDialog->ButtonHiiValueArray[Index]);
    break;

  default:
    return 0;
  }
  return 1;
}
