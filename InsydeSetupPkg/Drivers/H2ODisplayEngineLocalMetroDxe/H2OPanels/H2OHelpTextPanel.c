/** @file
  UI Common Controls

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "H2ODisplayEngineLocalMetro.h"
#include "H2OPanels.h"
#include "MetroUi.h"

STATIC H2O_HELP_TEXT_PANEL_CLASS         *mH2OHelpTextPanelClass = NULL;
#define CURRENT_CLASS                    mH2OHelpTextPanelClass

CHAR16 *mHelpTextPanelChilds = L""
  L"<VerticalLayout width='match_parent' float='true' height='match_parent'>"
    L"<Control />"
    L"<Texture width='300' height='300' name='HelpBkgImage' bkimagestyle='stretch|gray'/>"
  L"</VerticalLayout>"
  L"<VerticalLayout padding='40,30,40,30' width='-1' float='true' name='HelpMenu' height='-1'>"
    L"<HorizontalLayout padding='0,0,10,0' minheight='50' height='wrap_content'>"
      L"<Label width='200' textcolor='0xFF268FB0' fontsize='29' name='HelpTitle' height='wrap_content'/>"
      L"<Control padding='0,25,0,25' height='50'>"
        L"<Texture width='50' height='50' name='HelpImage' bkimagestyle='stretch'/>"
      L"</Control>"
    L"</HorizontalLayout>"
    L"<Label width='-1' textcolor='0xFF666666' fontsize='19' name='HelpText'/>"
  L"</VerticalLayout>";

EFI_STATUS
RefreshHelpMenu (
  H2O_HELP_TEXT_PANEL          *This
  )
{
  UI_CONTROL                    *Control;
  UI_MANAGER                    *Manager;
  H2O_FORM_BROWSER_Q            *CurrentQ;
  CHAR16                        Str[20];
  EFI_IMAGE_INPUT               *PageImage;

  Control   = (UI_CONTROL *) This;
  Manager   = Control->Manager;
  CurrentQ  = gFB->CurrentQ;

  if (CurrentQ != NULL && CurrentQ->Prompt != NULL) {
    Control = UiFindChildByName (This, L"HelpTitle");
    UiSetAttribute (Control, L"text", CurrentQ->Prompt);
  }

  PageImage = GetCurrentFormSetImage ();
  if (PageImage != NULL) {
    Control = UiFindChildByName (This, L"HelpImage");
    UnicodeSPrint (Str, sizeof (Str), L"0x%p",  PageImage);
    UiSetAttribute (Control, L"bkimage", Str);
    UiSetAttribute (Control, L"visible", L"true");
    Control = UiFindChildByName (This, L"HelpBkgImage");
    UiSetAttribute (Control, L"bkimage", Str);
    UiSetAttribute (Control, L"visible", L"true");
  } else {
    Control = UiFindChildByName (This, L"HelpImage");
    UiSetAttribute (Control, L"visible", L"false");
    Control = UiFindChildByName (This, L"HelpBkgImage");
    UiSetAttribute (Control, L"visible", L"false");
  }
  if (CurrentQ != NULL && CurrentQ->Help != NULL) {
    Control = UiFindChildByName (This, L"HelpText");
    UiSetAttribute (Control, L"text", CurrentQ->Help);
  }
  return EFI_SUCCESS;
}



LRESULT
EFIAPI
H2OHelpTextPanelProc (
  HWND   Hwnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  H2O_HELP_TEXT_PANEL           *This;
  UI_CONTROL                    *Control;


  This = (H2O_HELP_TEXT_PANEL *) GetWindowLongPtr (Hwnd, 0);
  Control = (UI_CONTROL *)This;
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }

  switch (Msg) {

  case WM_CREATE:
    This = (H2O_HELP_TEXT_PANEL *) AllocateZeroPool (sizeof (H2O_HELP_TEXT_PANEL));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Hwnd, 0);
      SetWindowLongPtr (Hwnd, 0, (INTN)This);
      SendMessage (Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    XmlCreateControl (mHelpTextPanelChilds, Control);
    break;

  case FB_NOTIFY_SELECT_Q:
  case FB_NOTIFY_REPAINT:
    RefreshHelpMenu (This);
    break;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);

  }
  return 0;

}


H2O_HELP_TEXT_PANEL_CLASS *
EFIAPI
GetH2OHelpTextPanelClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"H2OHelpTextPanel", (UI_CONTROL_CLASS *)GetControlClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }

  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc = H2OHelpTextPanelProc;

  return CURRENT_CLASS;
}

