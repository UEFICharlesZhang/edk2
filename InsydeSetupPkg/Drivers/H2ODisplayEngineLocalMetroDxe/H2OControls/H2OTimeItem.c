/** @file
  UI time item Controls

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

STATIC H2O_TIME_ITEM_CLASS      *mTimeItemClass = NULL;
#define CURRENT_CLASS           mTimeItemClass

CHAR16 *mTimeItemChilds = L""
  L"<HorizontalLayout height='156'>"
    L"<NumberPicker width='92' bkcolor='@menucolor' name='Hour'/>"
    L"<VerticalLayout width='2' bkcolor='0xFF333333'>"
      L"<Label height='52'/>"
      L"<Label name='Separation1' text=':' align='singleline|center' height='39' fontsize='29' bkcolor='0xFF787878' textcolor='0xFF999999'/>"
      L"<Label height='65'/>"
    L"</VerticalLayout>"
    L"<NumberPicker width='92' bkcolor='@menucolor' name='Minute'/>"
    L"<VerticalLayout width='2' bkcolor='0xFF333333'>"
      L"<Label height='52'/>"
      L"<Label name='Separation2' text=':' align='singleline|center' height='39' fontsize='29' bkcolor='0xFF787878' textcolor='0xFF999999'/>"
      L"<Label height='65'/>"
    L"</VerticalLayout>"
    L"<NumberPicker width='92' bkcolor='@menucolor' name='Second'/>"
  L"</HorizontalLayout>";

EFI_STATUS
H2OTimeItemNumberPickerUpdate (
  IN H2O_TIME_ITEM               *This,
  IN EFI_TIME                   *EfiTime
  )
{
  UI_CONTROL                    *ChildControl;
  CHAR16                        Str[20];
  CHAR16                        **ListStr;
  UINT32                        MinValue;
  UINT32                        MaxValue;

  //
  // Hour
  //
  ChildControl = (UI_CONTROL *) This->HourNumberPicker;

  MinValue = 0;
  MaxValue = 23;
  ListStr = CreateStrArrayByNumRange (MinValue, MaxValue, 1, L"%02d");
  UnicodeSPrint (Str, sizeof (Str), L"0x%p", ListStr);
  UiSetAttribute (ChildControl, L"list", Str);
  UnicodeSPrint (Str, sizeof (Str), L"%02d", EfiTime->Hour);
  UiSetAttribute (ChildControl, L"currentindex", Str);
  FreeStringArray (ListStr, (MaxValue - MinValue + 1));

  //
  // Minute and second
  //
  MinValue = 0;
  MaxValue = 59;
  ListStr = CreateStrArrayByNumRange (MinValue, MaxValue, 1, L"%02d");

  ChildControl = (UI_CONTROL *) This->MinuteNumberPicker;
  UnicodeSPrint (Str, sizeof (Str), L"0x%p", ListStr);
  UiSetAttribute (ChildControl, L"list", Str);
  UnicodeSPrint (Str, sizeof (Str), L"%d", EfiTime->Minute);
  UiSetAttribute (ChildControl, L"currentindex", Str);

  ChildControl = (UI_CONTROL *) This->SecondNumberPicker;
  UnicodeSPrint (Str, sizeof (Str), L"0x%p", ListStr);
  UiSetAttribute (ChildControl, L"list", Str);
  UnicodeSPrint (Str, sizeof (Str), L"%02d", EfiTime->Second);
  UiSetAttribute (ChildControl, L"currentindex", Str);

  FreeStringArray (ListStr, (MaxValue - MinValue + 1));

  CONTROL_CLASS_INVALIDATE (This);
  return EFI_SUCCESS;
}

VOID
H2OTimeItemSetSelectedControl (
  IN H2O_TIME_ITEM               *This,
  IN UI_NUMBER_PICKER           *SelectedControl
  )
{
  if (This->SelectedControl != NULL) {
    CONTROL_CLASS_SET_STATE (This->SelectedControl, 0, UISTATE_SELECTED);
  }

  if (SelectedControl != NULL) {
    CONTROL_CLASS_SET_STATE (SelectedControl, UISTATE_SELECTED, 0);
  }

  This->SelectedControl = SelectedControl;
  SetFocus (((UI_CONTROL *)This)->Wnd);
}

EFI_STATUS
H2OTimeItemSetNextValue (
  IN H2O_TIME_ITEM               *This,
  IN BOOLEAN                    Increasement
  )
{
  H2O_DATE_TIME_ITEM            DateTimeItem;
  EFI_STATUS                    Status;

  if (This->SelectedControl == This->HourNumberPicker) {
    DateTimeItem = HourItem;
  } else if (This->SelectedControl == This->MinuteNumberPicker) {
    DateTimeItem = MinuteItem;
  } else if (This->SelectedControl == This->SecondNumberPicker) {
    DateTimeItem = SecondItem;
  } else {
    return EFI_UNSUPPORTED;
  }

  Status = GetNextDateTimeValue (DateTimeItem, Increasement, &This->EfiTime);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  H2OTimeItemNumberPickerUpdate (This, &This->EfiTime);
  if (This->OnTimeChange != NULL) {
    This->OnTimeChange (This, &This->EfiTime);
  }

  return EFI_SUCCESS;
}

VOID
H2OTimeItemNumberPickerOnCurrentIndexChange (
  UI_NUMBER_PICKER              *This,
  INT32                         CurrentIndex
  )
{
  EFI_STATUS                    Status;
  UINT8                         Value;
  UI_CONTROL                    *Control;
  H2O_TIME_ITEM                  *TimeItem;
  EFI_TIME                      EfiTime;

  Control  = (UI_CONTROL *) This;
  TimeItem = (H2O_TIME_ITEM *) GetWindowLongPtr (Control->Wnd, GWLP_USERDATA);
  Value    = (UINT8) StrToUInt (This->List[CurrentIndex], 10, &Status);
  CopyMem (&EfiTime, &TimeItem->EfiTime, sizeof (EFI_TIME));

  if (StrCmp (Control->Name, L"Hour") == 0) {
    EfiTime.Hour    = Value;
  } else if (StrCmp (Control->Name, L"Minute") == 0) {
    EfiTime.Minute  = Value;
  } else if (StrCmp (Control->Name, L"Second") == 0) {
    EfiTime.Second  = Value;
  } else {
    return;
  }

  CopyMem (&TimeItem->EfiTime, &EfiTime, sizeof (EFI_TIME));

  if (TimeItem->OnTimeChange != NULL) {
    TimeItem->OnTimeChange (TimeItem, &TimeItem->EfiTime);
  }

  return;
}

VOID
H2OTimeItemNumberPickerOnItemSelected (
  UI_NUMBER_PICKER              *This
  )
{
  UI_CONTROL                    *Control;
  H2O_TIME_ITEM                  *TimeItem;

  Control  = (UI_CONTROL *) This;
  TimeItem = (H2O_TIME_ITEM *) GetWindowLongPtr (Control->Wnd, GWLP_USERDATA);

  if (TimeItem->SelectedControl != NULL) {
    CONTROL_CLASS_SET_STATE (TimeItem->SelectedControl, 0, UISTATE_SELECTED);
  }

  CONTROL_CLASS_SET_STATE (This, UISTATE_SELECTED, 0);
  TimeItem->SelectedControl = This;
}

VOID
EFIAPI
H2OTimeNumberPickOnSetState (
  UI_CONTROL                    *Control,
  UI_STATE                      SetState,
  UI_STATE                      ClearState
  )
{
  UI_CONTROL                    *DateItem;

  if (SetState & UISTATE_CAPTURED) {
    DateItem = (UI_CONTROL *) GetWindowLongPtr (Control->Wnd, GWLP_USERDATA);
    SetFocus (DateItem->Wnd);
  }
}

VOID
H2OTimeItemCreate (
  IN OUT H2O_TIME_ITEM           *This
  )
{
  UI_CONTROL                    *ChildControl;

  This->HourNumberPicker        = (UI_NUMBER_PICKER *) UiFindChildByName (This, L"Hour" );
  This->MinuteNumberPicker      = (UI_NUMBER_PICKER *) UiFindChildByName (This, L"Minute");
  This->SecondNumberPicker      = (UI_NUMBER_PICKER *) UiFindChildByName (This, L"Second");

  This->HourNumberPicker->OnItemSelected   = H2OTimeItemNumberPickerOnItemSelected;
  This->MinuteNumberPicker->OnItemSelected = H2OTimeItemNumberPickerOnItemSelected;
  This->SecondNumberPicker->OnItemSelected = H2OTimeItemNumberPickerOnItemSelected;

  ((UI_CONTROL *)(This->HourNumberPicker))->OnSetState   = H2OTimeNumberPickOnSetState;
  ((UI_CONTROL *)(This->MinuteNumberPicker))->OnSetState = H2OTimeNumberPickOnSetState;
  ((UI_CONTROL *)(This->SecondNumberPicker))->OnSetState = H2OTimeNumberPickOnSetState;

  This->HourNumberPicker->OnCurrentIndexChange   = H2OTimeItemNumberPickerOnCurrentIndexChange;
  This->MinuteNumberPicker->OnCurrentIndexChange = H2OTimeItemNumberPickerOnCurrentIndexChange;
  This->SecondNumberPicker->OnCurrentIndexChange = H2OTimeItemNumberPickerOnCurrentIndexChange;

  ChildControl = (UI_CONTROL *) This->HourNumberPicker;
  SetWindowLongPtr (ChildControl->Wnd, GWLP_USERDATA, (INTN) This);
  SetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE  , GetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE);

  ChildControl = (UI_CONTROL *) This->MinuteNumberPicker;
  SetWindowLongPtr (ChildControl->Wnd, GWLP_USERDATA, (INTN) This);
  SetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE  , GetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE);

  ChildControl = (UI_CONTROL *) This->SecondNumberPicker;
  SetWindowLongPtr (ChildControl->Wnd, GWLP_USERDATA, (INTN) This);
  SetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE  , GetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE);

  H2OTimeItemSetSelectedControl (This, This->HourNumberPicker);
  ChildControl = UiFindChildByName (This, L"Separation1");
  UiSetAttribute (ChildControl, L"textcolor", L"0xFFFFFFFF");
  ChildControl = UiFindChildByName (This, L"Separation2");
  UiSetAttribute (ChildControl, L"textcolor", L"0xFFFFFFFF");

  UiSetAttribute (This, L"tabstop", L"true");
}

VOID
H2OTimeItemKeyDown (
  HWND                          Wnd,
  UINT32                        Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  H2O_TIME_ITEM                  *This;
  UI_CONTROL                    *Control;
  UI_MANAGER                    *Manager;

  This    = (H2O_TIME_ITEM *) GetWindowLongPtr (Wnd, 0);
  if (This == NULL) {
    ASSERT (FALSE);
    return;
  }
  Control = (UI_CONTROL *) This;

  switch (WParam) {

  case VK_RETURN:
    Manager = Control->Manager;
    SendMessage (Manager->MainWnd, UI_NOTIFY_CARRIAGE_RETURN, (WPARAM) Control, 0);
    break;

  case VK_TAB:
  case VK_RIGHT:
    if (This->SelectedControl == This->HourNumberPicker) {
      H2OTimeItemSetSelectedControl (This, This->MinuteNumberPicker);
    } else if (This->SelectedControl == This->MinuteNumberPicker) {
      H2OTimeItemSetSelectedControl (This, This->SecondNumberPicker);
    }
    break;

  case VK_LEFT:
    if (This->SelectedControl == This->SecondNumberPicker) {
      H2OTimeItemSetSelectedControl (This, This->MinuteNumberPicker);
    } else if (This->SelectedControl == This->MinuteNumberPicker) {
      H2OTimeItemSetSelectedControl (This, This->HourNumberPicker);
    }
    break;

  case VK_F5:
    H2OTimeItemSetNextValue (This, FALSE);
    break;

  case VK_F6:
    H2OTimeItemSetNextValue (This, TRUE);
    break;

  default:
    if (This->SelectedControl != NULL) {
      CONTROL_CLASS_WNDPROC (This->SelectedControl, ((UI_CONTROL *) This->SelectedControl)->Wnd, Msg, WParam, LParam);
    }
    break;
  }
}

BOOLEAN
EFIAPI
H2OTimeItemSetAttribute (
  UI_CONTROL *Control,
  CHAR16     *Name,
  CHAR16     *Value
  )
{
  H2O_TIME_ITEM                  *This;
  EFI_STATUS                    Status;
  EFI_TIME                      *EfiTime;

  This = (H2O_TIME_ITEM *) Control;

  if (StrCmp (Name, L"efitime") == 0) {
    EfiTime = (EFI_TIME *) (UINTN) StrToUInt (Value, 16, &Status);

    CopyMem (&This->EfiTime, EfiTime, sizeof(EFI_TIME));
    H2OTimeItemNumberPickerUpdate (This, &This->EfiTime);
  } else {
    return PARENT_CLASS_SET_ATTRIBUTE (CURRENT_CLASS, Control, Name, Value);
  }

  CONTROL_CLASS_INVALIDATE (This);

  return TRUE;
}


VOID
EFIAPI
H2OTimeItemSetState (
  UI_CONTROL                    *Control,
  UI_STATE                      SetState,
  UI_STATE                      ClearState
  )
{
  H2O_TIME_ITEM                  *This;
  UI_CONTROL                    *ChildControl;

  This = (H2O_TIME_ITEM *) Control;

  if (SetState & UISTATE_FOCUSED) {
    if (This->SelectedControl == NULL) {
      This->SelectedControl = This->HourNumberPicker;
    }
    H2OTimeItemSetSelectedControl (This, This->SelectedControl);
  }

  if (ClearState & UISTATE_FOCUSED) {
    ChildControl = UiFindChildByName (This, L"Separation1");
    UiSetAttribute (ChildControl, L"textcolor", L"0xFF999999");
    ChildControl = UiFindChildByName (This, L"Separation2");
    UiSetAttribute (ChildControl, L"textcolor", L"0xFF999999");
    H2OTimeItemSetSelectedControl (This, NULL);
  }

  PARENT_CONTROL_CLASS(CURRENT_CLASS)->SetState (Control, SetState, ClearState);
}



LRESULT
EFIAPI
H2OTimeItemProc (
  HWND                          Wnd,
  UINT32                        Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  H2O_TIME_ITEM                  *This;
  UI_CONTROL                    *Control;

  This    = (H2O_TIME_ITEM *) GetWindowLongPtr (Wnd, 0);
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  Control = (UI_CONTROL *) This;

  switch (Msg) {

  case WM_CREATE:
    This = (H2O_TIME_ITEM *) AllocateZeroPool (sizeof (H2O_TIME_ITEM));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Wnd, 0);
      SetWindowLongPtr (Wnd, 0, (INTN)This);
      SendMessage (Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    XmlCreateControl (mTimeItemChilds, Control);
    SetWindowLongPtr (Control->Wnd, GWL_EXSTYLE, GetWindowLongPtr (Control->Wnd, GWL_EXSTYLE) & ~WS_EX_NOACTIVATE);
    H2OTimeItemCreate (This);
    break;

  case WM_KEYDOWN:
    H2OTimeItemKeyDown (Wnd, Msg, WParam, LParam);
    break;

  case WM_GETDLGCODE:
    if (This->SelectedControl == This->SecondNumberPicker) {
      return 0;
    }
    return DLGC_WANTTAB;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam, LParam);
  }

  return 0;
}

H2O_TIME_ITEM_CLASS *
EFIAPI
GetTimeItemClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"TimeItem", (UI_CONTROL_CLASS *)GetControlClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc      = H2OTimeItemProc;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetState     = H2OTimeItemSetState;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetAttribute = H2OTimeItemSetAttribute;

  return CURRENT_CLASS;
}

