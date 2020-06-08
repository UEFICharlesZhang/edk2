/** @file
  UI date item Controls

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

STATIC H2O_DATE_ITEM_CLASS      *mDateItemClass = NULL;
#define CURRENT_CLASS           mDateItemClass

CHAR16 *mDateItemChilds = L""
  L"<HorizontalLayout height='156'>"
    L"<NumberPicker width='105' bkcolor='@menucolor' name='Year'/>"
    L"<VerticalLayout width='5' bkcolor='0xFF333333'>"
      L"<Label height='52'/>"
      L"<Label name='Separation1' text=':' align='singleline|center' height='39' fontsize='29' bkcolor='0xFF787878' textcolor='0xFF999999'/>"
      L"<Label height='65'/>"
    L"</VerticalLayout>"
    L"<NumberPicker width='86' bkcolor='@menucolor' name='Month'/>"
    L"<VerticalLayout width='5' bkcolor='0xFF333333'>"
      L"<Label height='52'/>"
      L"<Label name='Separation2' text=':' align='singleline|center' height='39' fontsize='29' bkcolor='0xFF787878' textcolor='0xFF999999'/>"
      L"<Label height='65'/>"
    L"</VerticalLayout>"
    L"<NumberPicker width='85' bkcolor='@menucolor' name='Day'/>"
  L"</HorizontalLayout>";

EFI_STATUS
H2ODateItemNumberPickerUpdate (
  IN H2O_DATE_ITEM              *This,
  IN EFI_TIME                   *EfiTime
  )
{
  UI_CONTROL                    *ChildControl;
  CHAR16                        Str[20];
  CHAR16                        **ListStr;
  UINT32                        MinValue;
  UINT32                        MaxValue;
  UINT8                         DayOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  //
  // Year
  //
  ChildControl = (UI_CONTROL *) This->YearNumberPicker;

  MinValue = YEAR_MIN;
  MaxValue = YEAR_MAX;
  ListStr = CreateStrArrayByNumRange (MinValue, MaxValue, 1, L"%04d");
  UnicodeSPrint (Str, sizeof (Str), L"0x%p", ListStr);
  UiSetAttribute (ChildControl, L"list", Str);
  UnicodeSPrint (Str, sizeof (Str), L"%02d", EfiTime->Year - YEAR_MIN);
  UiSetAttribute (ChildControl, L"currentindex", Str);
  FreeStringArray (ListStr, (MaxValue - MinValue + 1));

  //
  // Month
  //
  ChildControl = (UI_CONTROL *) This->MonthNumberPicker;

  MinValue = 1;
  MaxValue = 12;
  ListStr = CreateStrArrayByNumRange (MinValue, MaxValue, 1, L"%02d");
  UnicodeSPrint (Str, sizeof (Str), L"0x%p", ListStr);
  UiSetAttribute (ChildControl, L"list", Str);
  UnicodeSPrint (Str, sizeof (Str), L"%d", EfiTime->Month - 1);
  UiSetAttribute (ChildControl, L"currentindex", Str);
  FreeStringArray (ListStr, (MaxValue - MinValue + 1));

  //
  // Day
  //
  ChildControl = (UI_CONTROL *) This->DayNumberPicker;

  MinValue = 1;
  if (EfiTime->Month == 2 && IsLeapYear (EfiTime->Year)) {
    MaxValue = 29;
  } else {
    MaxValue = (UINT32) DayOfMonth[EfiTime->Month - 1];
  }
  ListStr = CreateStrArrayByNumRange (MinValue, MaxValue, 1, L"%02d");
  UnicodeSPrint (Str, sizeof (Str), L"0x%p", ListStr);
  UiSetAttribute (ChildControl, L"list", Str);
  UnicodeSPrint (Str, sizeof (Str), L"%02d", EfiTime->Day - 1);
  UiSetAttribute (ChildControl, L"currentindex", Str);
  FreeStringArray (ListStr, (MaxValue - MinValue + 1));

  CONTROL_CLASS_INVALIDATE (This);
  return EFI_SUCCESS;
}

VOID
H2ODateItemSetSelectedControl (
  IN H2O_DATE_ITEM              *This,
  IN UI_NUMBER_PICKER           *SelectedControl
  )
{
  if (This->SelectedControl != NULL) {
    CONTROL_CLASS_SET_STATE (This->SelectedControl, 0, UISTATE_SELECTED);
  }

  if (SelectedControl != NULL) {
    CONTROL_CLASS_SET_STATE (SelectedControl, UISTATE_SELECTED, 0);
    SetFocus (((UI_CONTROL *)This)->Wnd);
  }

  This->SelectedControl = SelectedControl;
}

EFI_STATUS
H2ODateItemSetNextValue (
  IN H2O_DATE_ITEM              *This,
  IN BOOLEAN                    Increasement
  )
{
  H2O_DATE_TIME_ITEM            DateTimeItem;
  EFI_STATUS                    Status;

  if (This->SelectedControl == This->YearNumberPicker) {
    DateTimeItem = YearItem;
  } else if (This->SelectedControl == This->MonthNumberPicker) {
    DateTimeItem = MonthItem;
  } else if (This->SelectedControl == This->DayNumberPicker) {
    DateTimeItem = DayItem;
  } else {
    return EFI_UNSUPPORTED;
  }

  Status = GetNextDateTimeValue (DateTimeItem, Increasement, &This->EfiTime);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  H2ODateItemNumberPickerUpdate (This, &This->EfiTime);
  if (This->OnDateChange != NULL) {
    This->OnDateChange (This, &This->EfiTime);
  }

  return EFI_SUCCESS;
}

VOID
H2ODateItemNumberPickerOnCurrentIndexChange (
  UI_NUMBER_PICKER              *This,
  INT32                         CurrentIndex
  )
{
  BOOLEAN                       IsDay;
  BOOLEAN                       IsYear;
  EFI_STATUS                    Status;
  UINT16                        Value;
  UINT8                         DayOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  CHAR16                        Str[20];
  UINT16                        MinValue;
  UINT16                        MaxValue;
  UI_CONTROL                    *Control;
  UI_CONTROL                    *ChildControl;
  H2O_DATE_ITEM                  *DateItem;
  EFI_TIME                      EfiTime;
  CHAR16                        **ListStr;

  IsDay    = FALSE;
  IsYear   = FALSE;
  Control  = (UI_CONTROL *) This;
  DateItem = (H2O_DATE_ITEM *) GetWindowLongPtr (Control->Wnd, GWLP_USERDATA);
  Value    = (UINT16) StrToUInt (This->List[CurrentIndex], 10, &Status);
  CopyMem (&EfiTime, &DateItem->EfiTime, sizeof (EFI_TIME));

  if (StrCmp (Control->Name, L"Day") == 0) {
    IsDay = TRUE;
    EfiTime.Day   = (UINT8) Value;
  } else if (StrCmp (Control->Name, L"Month") == 0) {
    EfiTime.Month = (UINT8) Value;
  } else if (StrCmp (Control->Name, L"Year") == 0) {
    IsYear = TRUE;
    EfiTime.Year  = Value;
  } else {
    return;
  }

  while (!IsDayValid (&EfiTime)) {
    EfiTime.Day--;
    if (EfiTime.Day == 0) {
      ASSERT(FALSE);
      return;
    }
  }

  CopyMem (&DateItem->EfiTime, &EfiTime, sizeof (EFI_TIME));

  if (DateItem->OnDateChange != NULL) {
    DateItem->OnDateChange (DateItem, &DateItem->EfiTime);
  }

  if (IsDay || (IsYear && EfiTime.Month != 2)) {
    return;
  }

  //
  // Update string list of day
  //
  MinValue     = 1;
  if (EfiTime.Month == 2 && IsLeapYear (EfiTime.Year)) {
    MaxValue = 29;
  } else {
    MaxValue = DayOfMonth[EfiTime.Month - 1];
  }

  ListStr = CreateStrArrayByNumRange (MinValue, MaxValue, 1, L"%02d");
  ChildControl = (UI_CONTROL *) DateItem->DayNumberPicker;
  UnicodeSPrint (Str, sizeof (Str), L"0x%p", ListStr);
  UiSetAttribute (ChildControl, L"list", Str);
  UnicodeSPrint (Str, sizeof (Str), L"%02d", EfiTime.Day - 1);
  UiSetAttribute (ChildControl, L"currentindex", Str);
  FreeStringArray (ListStr, (MaxValue - MinValue + 1));

  return;
}

VOID
H2ODateItemNumberPickerOnItemSelected (
  UI_NUMBER_PICKER              *This
  )
{
  UI_CONTROL                    *Control;
  H2O_DATE_ITEM                  *DateItem;

  Control  = (UI_CONTROL *) This;
  DateItem = (H2O_DATE_ITEM *) GetWindowLongPtr (Control->Wnd, GWLP_USERDATA);

  if (DateItem->SelectedControl != NULL) {
    CONTROL_CLASS_SET_STATE (DateItem->SelectedControl, 0, UISTATE_SELECTED);
  }

  CONTROL_CLASS_SET_STATE (This, UISTATE_SELECTED, 0);
  DateItem->SelectedControl = This;
}

VOID
EFIAPI
H2ODateNumberPickOnSetState (
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
H2ODateItemCreate (
  IN OUT H2O_DATE_ITEM           *This
  )
{
  UI_CONTROL                    *ChildControl;

  This->YearNumberPicker        = (UI_NUMBER_PICKER *) UiFindChildByName (This, L"Year");
  This->MonthNumberPicker       = (UI_NUMBER_PICKER *) UiFindChildByName (This, L"Month");
  This->DayNumberPicker         = (UI_NUMBER_PICKER *) UiFindChildByName (This, L"Day");

  This->YearNumberPicker->OnCurrentIndexChange  = H2ODateItemNumberPickerOnCurrentIndexChange;
  This->MonthNumberPicker->OnCurrentIndexChange = H2ODateItemNumberPickerOnCurrentIndexChange;
  This->DayNumberPicker->OnCurrentIndexChange   = H2ODateItemNumberPickerOnCurrentIndexChange;

  This->YearNumberPicker->OnItemSelected  = H2ODateItemNumberPickerOnItemSelected;
  This->MonthNumberPicker->OnItemSelected = H2ODateItemNumberPickerOnItemSelected;
  This->DayNumberPicker->OnItemSelected   = H2ODateItemNumberPickerOnItemSelected;

  ((UI_CONTROL *)(This->YearNumberPicker))->OnSetState = H2ODateNumberPickOnSetState;
  ((UI_CONTROL *)(This->MonthNumberPicker))->OnSetState = H2ODateNumberPickOnSetState;
  ((UI_CONTROL *)(This->DayNumberPicker))->OnSetState = H2ODateNumberPickOnSetState;

  ChildControl = (UI_CONTROL *) This->YearNumberPicker;
  SetWindowLongPtr (ChildControl->Wnd, GWLP_USERDATA, (INTN) This);
  SetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE  , GetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE);

  ChildControl = (UI_CONTROL *) This->MonthNumberPicker;
  SetWindowLongPtr (ChildControl->Wnd, GWLP_USERDATA, (INTN) This);
  SetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE  , GetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE);

  ChildControl = (UI_CONTROL *) This->DayNumberPicker;
  SetWindowLongPtr (ChildControl->Wnd, GWLP_USERDATA, (INTN) This);
  SetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE  , GetWindowLongPtr (ChildControl->Wnd, GWL_EXSTYLE) | WS_EX_NOACTIVATE);

  H2ODateItemSetSelectedControl (This, This->YearNumberPicker);
  ChildControl = UiFindChildByName (This, L"Separation1");
  UiSetAttribute (ChildControl, L"textcolor", L"0xFFFFFFFF");
  ChildControl = UiFindChildByName (This, L"Separation2");
  UiSetAttribute (ChildControl, L"textcolor", L"0xFFFFFFFF");

  UiSetAttribute (This, L"tabstop", L"true");
}

VOID
H2ODateItemKeyDown (
  HWND                          Wnd,
  UINT32                        Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  H2O_DATE_ITEM                  *This;
  UI_CONTROL                    *Control;
  UI_MANAGER                    *Manager;

  This    = (H2O_DATE_ITEM *) GetWindowLongPtr (Wnd, 0);
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
    if (This->SelectedControl == This->YearNumberPicker) {
      H2ODateItemSetSelectedControl (This, This->MonthNumberPicker);
    } else if (This->SelectedControl == This->MonthNumberPicker) {
      H2ODateItemSetSelectedControl (This, This->DayNumberPicker);
    }
    break;

  case VK_LEFT:
    if (This->SelectedControl == This->DayNumberPicker) {
      H2ODateItemSetSelectedControl (This, This->MonthNumberPicker);
    } else if (This->SelectedControl == This->MonthNumberPicker) {
      H2ODateItemSetSelectedControl (This, This->YearNumberPicker);
    }
    break;

  case VK_F5:
    H2ODateItemSetNextValue (This, FALSE);
    break;

  case VK_F6:
    H2ODateItemSetNextValue (This, TRUE);
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
H2ODateItemSetAttribute (
  UI_CONTROL *Control,
  CHAR16     *Name,
  CHAR16     *Value
  )
{
  H2O_DATE_ITEM                  *This;
  EFI_STATUS                    Status;
  EFI_TIME                      *EfiTime;

  This = (H2O_DATE_ITEM *) Control;

  if (StrCmp (Name, L"efitime") == 0) {
    EfiTime = (EFI_TIME *) (UINTN) StrToUInt (Value, 16, &Status);
    ASSERT(IsDayValid (EfiTime));

    CopyMem (&This->EfiTime, EfiTime, sizeof(EFI_TIME));
    H2ODateItemNumberPickerUpdate (This, &This->EfiTime);
  } else {
    return PARENT_CLASS_SET_ATTRIBUTE (CURRENT_CLASS, Control, Name, Value);
  }

  CONTROL_CLASS_INVALIDATE (This);

  return TRUE;
}

VOID
EFIAPI
H2ODateItemSetState (
  UI_CONTROL                    *Control,
  UI_STATE                      SetState,
  UI_STATE                      ClearState
  )
{
  H2O_DATE_ITEM                  *This;
  UI_CONTROL                    *ChildControl;

  This = (H2O_DATE_ITEM *) Control;

  if (SetState & UISTATE_FOCUSED) {
    if (This->SelectedControl == NULL) {
      This->SelectedControl = This->YearNumberPicker;
    }
    H2ODateItemSetSelectedControl (This, This->SelectedControl);
  }

  if (ClearState & UISTATE_FOCUSED) {
    ChildControl = UiFindChildByName (This, L"Separation1");
    UiSetAttribute (ChildControl, L"textcolor", L"0xFF999999");
    ChildControl = UiFindChildByName (This, L"Separation2");
    UiSetAttribute (ChildControl, L"textcolor", L"0xFF999999");
    H2ODateItemSetSelectedControl (This, NULL);
  }

  PARENT_CONTROL_CLASS(CURRENT_CLASS)->SetState (Control, SetState, ClearState);
}

LRESULT
EFIAPI
H2ODateItemProc (
  HWND                          Wnd,
  UINT32                        Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  H2O_DATE_ITEM                  *This;
  UI_CONTROL                    *Control;

  This    = (H2O_DATE_ITEM *) GetWindowLongPtr (Wnd, 0);
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  Control = (UI_CONTROL *) This;

  switch (Msg) {

  case WM_CREATE:
    This = (H2O_DATE_ITEM *) AllocateZeroPool (sizeof (H2O_DATE_ITEM));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Wnd, 0);
      SetWindowLongPtr (Wnd, 0, (INTN)This);
      SendMessage (Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    XmlCreateControl (mDateItemChilds, Control);
    SetWindowLongPtr (Control->Wnd, GWL_EXSTYLE, GetWindowLongPtr (Control->Wnd, GWL_EXSTYLE) & ~WS_EX_NOACTIVATE);
    H2ODateItemCreate (This);
    break;

  case WM_KEYDOWN:
    H2ODateItemKeyDown (Wnd, Msg, WParam, LParam);
    break;

  case WM_GETDLGCODE:
    if (This->SelectedControl == This->DayNumberPicker) {
      return 0;
    }
    return DLGC_WANTTAB;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam, LParam);
  }

  return 0;
}

H2O_DATE_ITEM_CLASS *
EFIAPI
GetDateItemClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"DateItem", (UI_CONTROL_CLASS *)GetControlClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc      = H2ODateItemProc;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetState     = H2ODateItemSetState;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetAttribute = H2ODateItemSetAttribute;

  return CURRENT_CLASS;
}

