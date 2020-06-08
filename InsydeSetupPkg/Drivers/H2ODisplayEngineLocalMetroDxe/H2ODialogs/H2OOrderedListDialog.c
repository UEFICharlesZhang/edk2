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
#include <Library/ConsoleLib.h>

extern H2O_FORM_BROWSER_D       *mFbDialog;

CHAR16 *mOrderedListDialogChilds = L""
  L"<VerticalLayout name='OrderedListDialog'>"
    L"<ListView moveitemsupport='true' name='OrderedListList' vscrollbar='true' tabstop='true' childpadding='2' taborder='1'/>"
    L"<Control height='12'/>"
    L"<HorizontalLayout padding='0,0,30,0' childpadding='2' width='300' height='55'>"
      L"<Control/>"
      L"<Button name='Ok' focusbkcolor='@menulightcolor' text='YES' align='singleline|center' height='30' width='55' taborder='2' fontsize='19' bkcolor='0xFFCCCCCC' textcolor='0xFFFFFFFF' tabstop='true'/>"
      L"<Button name='Cancel' focusbkcolor='@menulightcolor' text='NO' align='singleline|center' height='30' width='55' taborder='3' fontsize='19' bkcolor='0xFFCCCCCC' textcolor='0xFFFFFFFF' tabstop='true'/>"
    L"</HorizontalLayout>"
  L"</VerticalLayout>";

CHAR16 *mOrderedListOptionChilds = L""
  L"<HorizontalLayout width='-1' bkcolor='0xFFE6E6E6' name='Option' height='40'>"
    L"<Texture width='40' name='Icon' height='40'/>"
    L"<Label padding='7,0,7,0' align='center' textcolor='0xFF404040' fontsize='14' name='Words'/>"
    L"<Texture height='40' width='40' bkimage='@DialogSortIcon' name='SortIcon' bkimagestyle='stretch|gray'/>"
  L"</HorizontalLayout>";

UINT16
GetSizeByHiiValueType (
  IN UINT8                      Type
  )
{
  UINT8                         Width;

  //
  // Size of HII value type is referenced from IfrParse of form browser
  //
  Width = 1;
  switch (Type) {

  case EFI_IFR_TYPE_NUM_SIZE_8:
    Width = 1;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_16:
    Width = 2;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_32:
    Width = 4;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_64:
    Width = 8;
    break;

  default:
    break;
  }

  return Width;
}

VOID
H2OOrderedListOptionOnSetState (
  UI_CONTROL                    *Control,
  UI_STATE                      SetState,
  UI_STATE                      ClearState
  )
{
  UI_CONTROL                    *ChildControl;

  if (SetState & UISTATE_SELECTED) {
    ChildControl = UiFindChildByName (Control, L"Option");
    UiApplyAttributeList (ChildControl, L"bkcolor='@menulightcolor' ");
  }

  if (ClearState & UISTATE_SELECTED) {
    ChildControl = UiFindChildByName (Control, L"Option");
    UiApplyAttributeList (ChildControl, L"bkcolor='0xFFE6E6E6' ");
  }
}

EFI_STATUS
H2OOrderedListCreateOptions (
  IN UI_CONTROL                 *Layout
  )
{
  UINT8                         *ValueBuffer;
  UINT8                         ContainerCount;
  UINTN                         NumberOfOptions;
  H2O_FORM_BROWSER_O            *OptionArray;
  UINT64                        ContainerHiiValue;
  UINT32                        ContainerIndex;
  UINT32                        OptionIndex;
  H2O_FORM_BROWSER_O            *Option;
  UI_CONTROL                    *CreatedControl;
  UI_CONTROL                    *ChildControl;
  CHAR16                        Str[20];
  BOOLEAN                       Found;

  Found           = FALSE;
  ValueBuffer     = gFB->CurrentQ->HiiValue.Buffer;
  ContainerCount  = gFB->CurrentQ->ContainerCount;
  NumberOfOptions = gFB->CurrentQ->NumberOfOptions;
  OptionArray     = gFB->CurrentQ->Options;

  if (ContainerCount == 0 || NumberOfOptions == 0 || ValueBuffer == NULL || OptionArray == NULL) {
    return EFI_NOT_FOUND;
  }

  for (ContainerIndex = 0; ContainerIndex < ContainerCount; ContainerIndex ++) {
    ContainerHiiValue = GetHiiBufferValue (ValueBuffer, OptionArray[0].HiiValue.Type, ContainerIndex);

    for (OptionIndex = 0; OptionIndex < NumberOfOptions; OptionIndex ++) {
      Option = &OptionArray[OptionIndex];
      if (Option->HiiValue.Value.u64 != ContainerHiiValue) {
        continue;
      }

      CreatedControl             = XmlCreateControl (mOrderedListOptionChilds, Layout);
      CreatedControl->OnSetState = H2OOrderedListOptionOnSetState;
      SetWindowLongPtr (CreatedControl->Wnd, GWLP_USERDATA, (INTN) Option->HiiValue.Value.u64);

      //
      // Get Option button and set its text and bkimage
      //
      ChildControl = UiFindChildByName (CreatedControl, L"Words");
      UiSetAttribute (ChildControl, L"text", Option->Text);

      if (Option->Image != NULL) {
        UnicodeSPrint (Str, sizeof (Str), L"0x%p", Option->Image);
        ChildControl = UiFindChildByName (CreatedControl, L"Icon");
        UiSetAttribute (ChildControl, L"bkimage", Str);
      }

      Found = TRUE;
      break;
    }
  }

  if (!Found) {
    return EFI_NOT_FOUND;
  }

  LIST_VIEW_CLASS (Layout)->SetSelection ((UI_LIST_VIEW *) Layout, (INT32)0, TRUE);
  return EFI_SUCCESS;
}

EFI_STATUS
H2OOrderedListGetHiiValue (
  IN  UI_CONTROL                *Layout,
  OUT EFI_HII_VALUE             *HiiValue
  )
{
  UINT8                         HiiValueType;
  UINT32                        Index;
  UI_CONTROL                    *ChildControl;

  if (Layout == NULL || Layout->ItemCount == 0 || HiiValue == NULL) {
    return EFI_ABORTED;
  }

  HiiValueType = gFB->CurrentQ->Options[0].HiiValue.Type;

  ZeroMem (HiiValue, sizeof (EFI_HII_VALUE));
  HiiValue->Type      = EFI_IFR_TYPE_BUFFER;
  HiiValue->BufferLen = (UINT16) (GetSizeByHiiValueType (HiiValueType) * Layout->ItemCount);
  HiiValue->Buffer    = AllocateZeroPool (HiiValue->BufferLen);
  if (HiiValue->Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < Layout->ItemCount; Index++) {
    ChildControl = Layout->Items[Index];
    SetHiiBufferValue (HiiValue->Buffer, HiiValueType, Index, (UINT64) GetWindowLongPtr (ChildControl->Wnd, GWLP_USERDATA));
  }

  return EFI_SUCCESS;
}

LRESULT
H2OOrderedListInputProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_DIALOG                     *Dialog;
  UI_CONTROL                    *Control;
  UI_CONTROL                    *Layout;
  UI_CONTROL                    *ChildControl;
  CHAR16                        Str[20];
  EFI_IMAGE_INPUT               *FormsetImage;
  EFI_HII_VALUE                 HiiValue;
  EFI_STATUS                    Status;
  INT32                         SelectedIndex;
  INT32                         NextIndex;

  Dialog  = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);
  Control = (UI_CONTROL *) Dialog;

  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    if (mFbDialog->TitleString != NULL) {
      ChildControl = UiFindChildByName (Dialog, L"DialogTitle");
      UiSetAttribute (ChildControl, L"text", mFbDialog->TitleString);
    }

    if (gFB->CurrentQ != NULL && gFB->CurrentQ->Help != NULL) {
      ChildControl = UiFindChildByName (Dialog, L"DialogText");
      UiSetAttribute (ChildControl, L"text", gFB->CurrentQ->Help);
    }

    Control = UiFindChildByName (Dialog, L"Content");
    if (Control != NULL &&  gFB->CurrentQ != NULL) {
      XmlCreateControl (mOrderedListDialogChilds, Control);
    }

    Control = UiFindChildByName (Dialog, L"DialogTextScrollView");
    if (Control != NULL) {
      Control->OnSetState = H2OCommonDialogWithHelpOnSetState;
    }

    FormsetImage = GetCurrentFormSetImage ();
    if (FormsetImage != NULL) {
      ChildControl = UiFindChildByName (Dialog, L"DialogImage");
      UnicodeSPrint (Str, sizeof (Str), L"0x%p", FormsetImage);
      UiSetAttribute (ChildControl, L"bkimage", Str);
      UiSetAttribute (ChildControl, L"visible", L"true");
    }

    Layout = UiFindChildByName (Dialog, L"OrderedListList");
    if (Layout->VScrollBar != NULL) {
      UiSetAttribute (Layout->VScrollBar, L"displayscrollbar", L"false");
      CONTROL_CLASS_INVALIDATE (Layout);
    }

    Status = H2OOrderedListCreateOptions (Layout);
    if (!EFI_ERROR (Status)) {
      ChildControl = Layout;
    } else {
      ChildControl = UiFindChildByName (Dialog, L"Ok");
    }
    SetFocus (ChildControl->Wnd);

    UnregisterHotKey (Wnd, 4);
    UnregisterHotKey (Wnd, 5);
    UnregisterHotKey (Wnd, 8);
    UnregisterHotKey (Wnd, 9);
    MwRegisterHotKey (Wnd, 4,  0, VK_F5);
    MwRegisterHotKey (Wnd, 5,  0, VK_F6);
    MwRegisterHotKey (Wnd, 8,  0, VK_OEM_PLUS);
    MwRegisterHotKey (Wnd, 9,  0, VK_OEM_MINUS);
    break;

  case UI_NOTIFY_CARRIAGE_RETURN:
  case UI_NOTIFY_CLICK:
    ChildControl = (UI_CONTROL *) WParam;
    if (StrCmp (ChildControl->Name, L"OrderedListList") == 0) {
      ChildControl = UiFindChildByName (Dialog, L"Ok");
      SetFocus (ChildControl->Wnd);
    } else if (StrCmp (ChildControl->Name, L"Cancel") == 0) {
      SendShutDNotify ();
    } else if (StrCmp (ChildControl->Name, L"Ok") == 0) {
      Layout = UiFindChildByName (Dialog, L"OrderedListList");
      H2OOrderedListGetHiiValue (Layout, &HiiValue);
      SendChangeQNotify (0, 0, &HiiValue);
    }
    break;

  case WM_HOTKEY:
    switch (HIWORD(LParam)) {

    case VK_ESCAPE:
      SendShutDNotify ();
      return 0;

    case VK_F5:
    case VK_F6:
    case VK_OEM_PLUS:
    case VK_OEM_MINUS:
      Layout        = UiFindChildByName (Dialog, L"OrderedListList");
      SelectedIndex = LIST_VIEW_CLASS (Layout)->GetSelection ((UI_LIST_VIEW *) Layout);
      NextIndex     = LIST_VIEW_CLASS (Layout)->FindNextSelection (
                                                  (UI_LIST_VIEW *) Layout,
                                                  SelectedIndex,
                                                  (HIWORD(LParam) == VK_F6 || HIWORD(LParam) == VK_OEM_PLUS) ? VK_UP : VK_DOWN
                                                  );
      if (SelectedIndex != -1 && NextIndex != -1) {
        LIST_VIEW_CLASS (Layout)->SwitchItems ((UI_LIST_VIEW *) Layout, SelectedIndex, NextIndex);
        LIST_VIEW_CLASS (Layout)->SetSelection ((UI_LIST_VIEW *) Layout, NextIndex, TRUE);
      }
      return 0;

    default:
      break;
    }
    return 1;

  case WM_DESTROY:
    UnregisterHotKey (Wnd, 4);
    UnregisterHotKey (Wnd, 5);
    UnregisterHotKey (Wnd, 8);
    UnregisterHotKey (Wnd, 9);
    return 0;

  default:
    return 0;
  }

  return 1;
}

