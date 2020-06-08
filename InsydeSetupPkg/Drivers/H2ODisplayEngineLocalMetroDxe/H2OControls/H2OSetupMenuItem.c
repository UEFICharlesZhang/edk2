/** @file
  UI Common Controls

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

#include "H2OControls.h"
#include "H2ODisplayEngineLocalMetro.h"
#include "MetroUi.h"

STATIC H2O_SETUP_MENU_ITEM_CLASS  *mSetupMenuItemClass = NULL;
#define CURRENT_CLASS            mSetupMenuItemClass

CHAR16 *mSetupMenuItemChilds = L""
  L"<VerticalLayout bkcolor='0xFFFF0000'>"
    L"<Texture name='bkimage' bkimagestyle='stretch' float='true' height='match_parent' width='match_parent' bkimage='@MenuBackgroundNormal'/>"
    L"<VerticalLayout padding='30, 30, 30, 30' width='match_parent' float='true' height='match_parent'>"
      L"<Texture name='image' bkimagestyle='stretch|light'/>"
    L"</VerticalLayout>"
    L"<VerticalLayout/>"
    L"<Label name='text' align='center' height='33' padding='4,0,8,0' fontsize='20' textcolor='0xFFDDDDDD'/>"
  L"</VerticalLayout>";

VOID
EFIAPI
H2OSetupMenuItemSetPosition (
  UI_CONTROL                    *Control,
  CONST RECT                    *Pos
  )
{
  PARENT_CONTROL_CLASS(CURRENT_CLASS)->SetPosition (Control,Pos);
}

VOID
EFIAPI
H2OSetupMenuItemSetState (
  UI_CONTROL                    *Control,
  UI_STATE                      SetState,
  UI_STATE                      ClearState
  )
{
  H2O_SETUP_MENU_ITEM            *SetupMenuItem;
  UI_CONTROL                    *Child;

  PARENT_CONTROL_CLASS(CURRENT_CLASS)->SetState (Control, SetState, ClearState);

  SetupMenuItem = (H2O_SETUP_MENU_ITEM *) Control;
  Child = UiFindChildByName (Control, L"image");
}

BOOLEAN
EFIAPI
H2OSetupMenuItemSetAttribute (
  UI_CONTROL *Control,
  CHAR16     *Name,
  CHAR16     *Value
  )
{
  H2O_SETUP_MENU_ITEM            *This;
  EFI_STATUS                    Status;
  UI_CONTROL                    *Child;
  SETUP_MENU_INFO               *SetupMenuInfo;
  CHAR16                        Str[20];

  This = (H2O_SETUP_MENU_ITEM *) Control;

  if (StrCmp (Name, L"setupmenuinfo") == 0) {
    SetupMenuInfo = (SETUP_MENU_INFO *)(UINTN) StrToUInt (Value, 16, &Status);
    This->SetupMenuInfo = SetupMenuInfo;

    if (SetupMenuInfo->PageImage != NULL) {
      Child = UiFindChildByName (This, L"image");
      UnicodeSPrint (Str, sizeof (Str), L"0x%p",  SetupMenuInfo->PageImage);
      UiSetAttribute (Child, L"bkimage", Str);
      UiSetAttribute (Child, L"bkimagestyle", L"stretch");
      if ((SetupMenuInfo->PageId >> 16) != (gFB->CurrentP->PageId >> 16)) {
        UiSetAttribute (Child, L"bkimagestyle", L"stretch|light");
      }
    }

    Child = UiFindChildByName (This, L"text");
    UiSetAttribute (Child, L"text", SetupMenuInfo->PageTitle);

  } else {
    return PARENT_CLASS_SET_ATTRIBUTE (CURRENT_CLASS, Control, Name, Value);
  }

  CONTROL_CLASS_INVALIDATE (This);

  return TRUE;
}

LRESULT
EFIAPI
H2OSetupMenuItemProc (
  HWND   Wnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  H2O_SETUP_MENU_ITEM            *This;
  UI_CONTROL                    *Control;

  This = (H2O_SETUP_MENU_ITEM *) GetWindowLongPtr (Wnd, 0);
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  Control = (UI_CONTROL *)This;

  switch (Msg) {

  case WM_CREATE:
    This = (H2O_SETUP_MENU_ITEM *) AllocateZeroPool (sizeof (H2O_SETUP_MENU_ITEM));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Wnd, 0);
      SetWindowLongPtr (Wnd, 0, (INTN)This);
      SendMessage (Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    XmlCreateControl (mSetupMenuItemChilds, Control);
    break;

  case UI_NOTIFY_PAINT:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam, LParam);

    if (WParam == PAINT_ALL &&
        (CONTROL_CLASS_GET_STATE (This) & (UISTATE_SELECTED | UISTATE_FOCUSED)) == (UISTATE_SELECTED | UISTATE_FOCUSED)) {
      COLORREF                  Color;
      UI_MANAGER                *Manager;
      RECT                      Rc;
      HDC                       Hdc;

      Color = 0xFFFF0000;
      Manager = Control->Manager;
      Hdc     = Manager->PaintDC;

      Manager->GetControlRect (Manager, Control, &Rc);

      SelectObject (Hdc, GetStockObject (PS_NULL));
      SelectObject (Hdc, GetStockObject (DC_PEN));
      SetDCPenColor (Hdc, Color);
      Rectangle (Hdc, Rc.left, Rc.top, Rc.right, Rc.bottom);
   }
   break;

  case WM_NCHITTEST:
    return HTTRANSPARENT;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam, LParam);
  }

  return 0;
}

H2O_SETUP_MENU_ITEM_CLASS *
EFIAPI
GetSetupMenuItemClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"SetupMenuItem", (UI_CONTROL_CLASS *)GetControlClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc      = H2OSetupMenuItemProc;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetAttribute = H2OSetupMenuItemSetAttribute;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetPosition  = H2OSetupMenuItemSetPosition;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetState     = H2OSetupMenuItemSetState;


  return CURRENT_CLASS;
}


