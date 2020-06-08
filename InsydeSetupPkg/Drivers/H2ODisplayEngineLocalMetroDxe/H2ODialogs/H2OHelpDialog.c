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

CHAR16 *mHelpDialogChilds = L""
  L"<VerticalLayout padding='20,20,0,20' bkcolor='@menucolor' name='HelpDialog'>"
    L"<VerticalLayout padding='0,0,20,0' vscrollbar='false'>"
      L"<Label textcolor='0xFFFFFFFF' fontsize='19' name='DialogText'/>"
    L"</VerticalLayout>"
    L"<HorizontalLayout height='50'>"
      L"<Control/>"
      L"<Button name='DialogButton' focusbkcolor='@menulightcolor' text='OK' align='singleline|center' height='30' width='55' fontsize='27' bkcolor='0xFFCCCCCC' textcolor='0xFFFFFFFF'/>"
      L"<Control/>"
    L"</HorizontalLayout>"
  L"</VerticalLayout>";


LRESULT
H2OHelpDialogProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_DIALOG                     *Dialog;
  UI_CONTROL                    *Control;
  CHAR16                        *BodyString;

  Dialog  = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);

  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    Control = UiFindChildByName (Dialog, L"DialogText");
    BodyString = CatStringArray (mFbDialog->BodyStringCount, (CONST CHAR16 **) mFbDialog->BodyStringArray);
    if (BodyString != NULL) {
      UiSetAttribute (Control, L"text", BodyString);
      FreePool (BodyString);
    }

    Control = UiFindChildByName (Dialog, L"DialogButton");
    SetFocus (Control->Wnd);
    break;

  case UI_NOTIFY_CLICK:
    SendShutDNotify ();
    break;

  case WM_HOTKEY:
    if (HIWORD(LParam) == VK_ESCAPE) {
      SendShutDNotify ();
      return 0;
    }
    return 1;

  default:
    return 0;
  }
  return 1;
}

