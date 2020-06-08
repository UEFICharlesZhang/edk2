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

STATIC H2O_FORM_TITLE_PANEL_CLASS        *mH2OFormTitlePanelClass = NULL;
#define CURRENT_CLASS                    mH2OFormTitlePanelClass

CHAR16 *mFormTitlePanelChilds = L""
  L"<HorizontalLayout bkcolor='0xFF333333' height='-1'>"
    L"<HorizontalLayout width='-1' float='true' height='-1'>"
      L"<Texture bkimage='@FormTitleBkg' bkimagestyle='stretch'/>"
    L"</HorizontalLayout>"
    L"<Control padding='13,0,15,30' width='67' height='65'>"
      L"<Texture name='TitleImage' bkimagestyle='stretch'/>"
    L"</Control>"
    L"<Label padding='15,0,15,8' width='1500' textcolor='0xFFFFFFFF' fontsize='30' name='TitleName'/>"
  L"</HorizontalLayout>";


EFI_STATUS
RefreshFormTitleMenu (
  H2O_FORM_TITLE_PANEL          *This
  )
{

  EFI_IMAGE_INPUT               *PageImage;
  UI_CONTROL                    *Control;
  UI_MANAGER                    *Manager;
  CHAR16                        *FormSetTitle;
  CHAR16                        *MenuTitle;
  CHAR16                        Str[20];

  Control   = (UI_CONTROL *) This;
  Manager   = Control->Manager;
  PageImage = GetCurrentFormSetImage ();
  Control = Manager->FindControlByName (Manager, L"TitleImage");
  if (PageImage != NULL) {
    UnicodeSPrint (Str, sizeof (Str), L"0x%p",  PageImage);
    UiSetAttribute (Control, L"bkimage", Str);
    UiSetAttribute (Control, L"visible", L"true");
  } else {
    UiSetAttribute (Control, L"visible", L"false");
  }

  Control = Manager->FindControlByName (Manager, L"TitleName");
  FormSetTitle = GetCurrentFormSetTitle ();
  if (gFB->CurrentQ == NULL) {
    UiSetAttribute (Control, L"text", L"");
  } else {
    if (!IsRootPage ()) {
      MenuTitle = CatSPrint (NULL, L"%s > %s", FormSetTitle, gFB->CurrentP->PageTitle);
      UiSetAttribute (Control, L"text", MenuTitle);
      FreePool (MenuTitle);
    } else {
      UiSetAttribute (Control, L"text", FormSetTitle);
    }
  }

  return EFI_SUCCESS;
}

LRESULT
EFIAPI
H2OFormTitlePanelProc (
  HWND   Hwnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  H2O_FORM_TITLE_PANEL          *This;
  UI_CONTROL                    *Control;

  This = (H2O_FORM_TITLE_PANEL *) GetWindowLongPtr (Hwnd, 0);
  Control = (UI_CONTROL *)This;

  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }

  switch (Msg) {

  case WM_CREATE:
    This = (H2O_FORM_TITLE_PANEL *) AllocateZeroPool (sizeof (H2O_FORM_TITLE_PANEL));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Hwnd, 0);
      SetWindowLongPtr (Hwnd, 0, (INTN)This);
      SendMessage (Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    XmlCreateControl (mFormTitlePanelChilds, Control);
    break;

  case FB_NOTIFY_REPAINT:
    if (This != NULL) {
      RefreshFormTitleMenu (This);
    }
    break;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);

  }
  return 0;

}


H2O_FORM_TITLE_PANEL_CLASS *
EFIAPI
GetH2OFormTitlePanelClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"H2OFormTitlePanel", (UI_CONTROL_CLASS *)GetControlClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }


  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc = H2OFormTitlePanelProc;

  return CURRENT_CLASS;
}

