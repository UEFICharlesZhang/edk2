/** @file
  UI ordered list control

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
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


EFI_STATUS
PopupDialogInit (
  IN UI_DIALOG                             *Dialog
  )
{
  EFI_STATUS                               Status;
  UI_CONTROL                               *Control;
  UI_CONTROL                               *DialogControl;
  UI_CONTROL                               *FocusControl;
  UINT32                                   Index;
  CHAR16                                   ButtonWidthStr[20];
  CHAR16                                   *BodyString;
  INTN                                     Result;

  if (mFbDialog->TitleString != NULL) {
    Control = UiFindChildByName (Dialog, L"DialogTitle");
    UiSetAttribute (Control, L"text", mFbDialog->TitleString);
  }

  if (mFbDialog->BodyStringArray != NULL) {
    Control = UiFindChildByName (Dialog, L"DialogText");
    BodyString = CatStringArray (mFbDialog->BodyStringCount, (CONST CHAR16 **) mFbDialog->BodyStringArray);
    if (BodyString != NULL) {
      UiSetAttribute (Control, L"text", BodyString);
      FreePool (BodyString);
    }
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
  return EFI_SUCCESS;
}

INTN
H2OPopupDialogProc (
  HWND         Wnd,
  UINT         Msg,
  WPARAM       WParam,
  LPARAM       lParam
  )
{
  UI_DIALOG                                *Dialog;
  UI_CONTROL                               *Control;
  UINTN                                    Index;

  Dialog = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);

  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    PopupDialogInit (Dialog);
    break;

  case UI_NOTIFY_CLICK:
    Control = (UI_CONTROL *) WParam;
    Index = (UINTN) GetWindowLongPtr (Control->Wnd, GWLP_USERDATA);
    SendChangeQNotify (0, 0, &mFbDialog->ButtonHiiValueArray[Index]);
    break;

  case WM_HOTKEY:
    if (HIWORD(lParam) == VK_ESCAPE) {
      SendShutDNotify ();
      return 0;
    }
    return 1;

  case WM_DESTROY:
    FreeDialogEvent (&mFbDialog);
    return 0;

  default:
    return 0;
  }

  return 1;
}
