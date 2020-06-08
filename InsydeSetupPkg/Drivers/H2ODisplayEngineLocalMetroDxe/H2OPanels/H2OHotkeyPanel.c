/** @file
  UI Common Controls

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
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

STATIC H2O_HOTKEY_PANEL_CLASS            *mH2OHotkeyPanelClass = NULL;
#define CURRENT_CLASS                    mH2OHotkeyPanelClass

EFI_STATUS
RefreshHotkeyList (
  H2O_HOTKEY_PANEL              *This
  )
{
  UINTN                         Index;
  HOT_KEY_INFO                  *HotkeyInfo;
  HOT_KEY_INFO                  *CurrentHotkey;
  UI_CONTROL                    *HotkeyControl;
  CHAR16                        Str[20];
  UINTN                         GroupIdIndex;
  UINT8                         GroupId[HOT_KEY_INFO_GROUP_ID_MAX];
  BOOLEAN                       NewGroup;
  BOOLEAN                       OldGroup;

  //
  // Init background
  //
  UiApplyAttributeList ((UI_CONTROL*) This, L"bkcolor='0xFF333333' bkimage='@OwnerDrawBkg' padding='5,5,5,5'");

  //
  // remove old items
  //
  CONTROL_CLASS (This)->RemoveAllChild ((UI_CONTROL *)This);

  //
  // Create hotkey items
  //
  ASSERT (gFB->CurrentP != NULL);
  ASSERT (gFB->CurrentP->HotKeyInfo != NULL);
  HotkeyInfo = gFB->CurrentP->HotKeyInfo;

  for (GroupIdIndex = 0; GroupIdIndex < HOT_KEY_INFO_GROUP_ID_MAX; GroupIdIndex ++) {
    GroupId[GroupIdIndex] = HOT_KEY_INFO_GROUP_ID_NONE;
  }

  for (Index = 0; HotkeyInfo[Index].SendEvtType != 0; Index++) {

    CurrentHotkey = &HotkeyInfo[Index];

    NewGroup = FALSE;
    OldGroup = FALSE;
    if (CurrentHotkey->GroupId != HOT_KEY_INFO_GROUP_ID_NONE) {
      for (GroupIdIndex = 0; GroupIdIndex < HOT_KEY_INFO_GROUP_ID_MAX; GroupIdIndex ++) {
        if (GroupId[GroupIdIndex] == HOT_KEY_INFO_GROUP_ID_NONE) {
          //
          // New GroupId
          //
          NewGroup = TRUE;
          GroupId[GroupIdIndex] = CurrentHotkey->GroupId;
          break;
        } else if (GroupId[GroupIdIndex] == CurrentHotkey->GroupId) {
          //
          // Same GroupId
          //
          OldGroup = TRUE;
          UnicodeSPrint (Str, sizeof (Str), L"Hotkey_Group%02x", CurrentHotkey->GroupId);
          HotkeyControl = UiFindChildByName (This, Str);
          UnicodeSPrint (Str, sizeof (Str), L"0x%p", CurrentHotkey);
          UiSetAttribute (HotkeyControl, L"hotkey", Str);
          break;
        }
      }
    }

    if (OldGroup) {
      continue;
    }

    //
    // Create new HotkeyItem
    //
    HotkeyControl = CreateControl (L"HotkeyItem", (UI_CONTROL*) This);
    UnicodeSPrint (Str, sizeof (Str), L"0x%p", CurrentHotkey);
    UiSetAttribute (HotkeyControl, L"hotkey", Str);

    if (NewGroup) {
      UnicodeSPrint (Str, sizeof (Str), L"Hotkey_Group%02x", CurrentHotkey->GroupId);
      UiSetAttribute (HotkeyControl, L"name", Str);
    } else {
      UiSetAttribute (HotkeyControl, L"name", L"Hotkey");
    }

    CONTROL_CLASS(This)->AddChild ((UI_CONTROL *)This, HotkeyControl);
  }

  return EFI_SUCCESS;
}

LRESULT
EFIAPI
H2OHotkeyPanelProc (
  HWND   Wnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  H2O_HOTKEY_PANEL              *This;
  UI_CONTROL                    *Control;


  This = (H2O_HOTKEY_PANEL *) GetWindowLongPtr (Wnd, 0);
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  Control = (UI_CONTROL *)This;

  switch (Msg) {

  case WM_CREATE:
    This = (H2O_HOTKEY_PANEL *) AllocateZeroPool (sizeof (H2O_HOTKEY_PANEL));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Wnd, 0);
      SetWindowLongPtr (Wnd, 0, (INTN)This);
      SendMessage (Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    break;

  case FB_NOTIFY_REPAINT:
    RefreshHotkeyList (This);
    break;


  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam, LParam);

  }
  return 0;

}

H2O_HOTKEY_PANEL_CLASS *
EFIAPI
GetH2OHotkeyPanelClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"H2OHotkeyPanel", (UI_CONTROL_CLASS *)GetHorizontalLayoutClass ());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc = H2OHotkeyPanelProc;

  return CURRENT_CLASS;
}

