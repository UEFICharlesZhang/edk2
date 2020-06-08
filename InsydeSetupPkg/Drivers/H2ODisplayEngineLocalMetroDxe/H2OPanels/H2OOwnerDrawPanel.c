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
#include <Guid/H2ODisplayEngineType.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/LayoutLib.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <IndustryStandard/SmBios.h>
#include <Protocol/Smbios.h>

#define OWNER_DRAW_TIMER_ID 1
STATIC H2O_OWNER_DRAW_PANEL_CLASS        *mH2OOwnerDrawPanelClass = NULL;
#define CURRENT_CLASS                    mH2OOwnerDrawPanelClass

CHAR16 *mOwnerDrawPanelChilds = L""
  L"<Control>"
    L"<Texture name='OwnerDrawBackground' float='true' height='-1' width='-1' bkimage='@OwnerDrawBkg' bkcolor='0xFF333333'/>"
    L"<HorizontalLayout padding='13,30,19,35' name='OwnerDrawList'>"
      L"<VerticalLayout width='180'>"
        L"<Control width='117' bkimage='@OwnerDrawInsyde' bkcolor='0x0' height='31'/>"
        L"<Label textcolor='0xFFB3B3B3' fontsize='11' bkcolor='0x0' name='OwnerDrawSystemInfo'/>"
      L"</VerticalLayout>"
      L"<Control/>"
      L"<HorizontalLayout padding='20,0,10,0' width='700'>"
        L"<HorizontalLayout width='240'>"
          L"<Control width='33' bkimage='@OwnerDrawTime' bkcolor='0x0' height='35'/>"
          L"<VerticalLayout width='95'>"
            L"<Label textcolor='0xFFF2F2F2' fontsize='18' bkcolor='0x0' name='OwnerDrawDateValue'/>"
            L"<Label textcolor='0xFF999999' fontsize='18' bkcolor='0x0' name='OwnerDrawWeekdayValue'/>"
          L"</VerticalLayout>"
          L"<Label textcolor='0xFFFFFFFF' fontsize='28' bkcolor='0x0' name='OwnerDrawTimeValue'/>"
        L"</HorizontalLayout>"
        L"<Control/>"
        L"<HorizontalLayout width='230'>"
          L"<Control width='21' bkimage='@OwnerDrawTemperature' bkcolor='0x0' height='42'/>"
          L"<VerticalLayout width='120'>"
            L"<Label textcolor='0xFFF2F2F2' fontsize='18' bkcolor='0x0' name='OwnerDrawMainboardName'/>"
            L"<Label textcolor='0xFF999999' fontsize='18' bkcolor='0x0' name='OwnerDrawSystemName'/>"
          L"</VerticalLayout>"
          L"<Label textcolor='0xFFFFFFFF' fontsize='43' bkcolor='0x0' name='OwnerDrawSystemTemperature'/>"
        L"</HorizontalLayout>"
        L"<Control/>"
        L"<HorizontalLayout width='230'>"
          L"<Control width='21' bkimage='@OwnerDrawTemperature' bkcolor='0x0' height='42'/>"
          L"<VerticalLayout width='120'>"
            L"<Label textcolor='0xFFF2F2F2' fontsize='18' bkcolor='0x0' name='OwnerDrawCpuName'/>"
            L"<Label textcolor='0xFF999999' fontsize='18' bkcolor='0x0' name='OwnerDrawTemperatureName'/>"
          L"</VerticalLayout>"
          L"<Label textcolor='0xFFFFFFFF' fontsize='43' bkcolor='0x0' name='OwnerDrawCpuTemperature'/>"
        L"</HorizontalLayout>"
      L"</HorizontalLayout>"
      L"<Control width='13' bkimage='@OwnerDrawSeparator' bkcolor='0x0' height='86'/>"
      L"<Control height='73' padding='10,0,10,0' width='62' bkimage='@OwnerDrawInsydeH2O' bkcolor='0x0'/>"
    L"</HorizontalLayout>"
  L"</Control>";

CHAR16  mWeekdayStr[7][4] = {L"SUN", L"MON", L"TUE", L"WED", L"THU", L"FRI", L"SAT"};

CHAR16 *
GetWeekdayStr (
  IN EFI_TIME                             *Time
  )
{
  UINT32                                   CenturyOffset;
  UINT16                                   Year;
  UINT32                                   YearOffset;
  UINT32                                   MonthOffset[] = {0,3,3,6,1,4,6,2,5,0,3,5};
  UINT32                                   Weekday;

  if (Time == NULL) {
    return L"ERROR";
  }

  if (Time->Year < 1900 || Time->Year > 2099) {
    return L"ERROR";
  }

  if (Time->Year >= 2000) {
    CenturyOffset = 6;
    Year = Time->Year - 2000;
  } else {
    CenturyOffset = 0;
    Year = Time->Year - 1900;
  }

  YearOffset = Year + Year / 4;

  Weekday = CenturyOffset + YearOffset + MonthOffset[Time->Month - 1] + (UINTN) Time->Day;
  if (IsLeapYear (Time->Year) && Time->Month <= 2) {
    Weekday--;
  }
  Weekday %= 7;

  return mWeekdayStr[Weekday];
}

/**
  Acquire the string associated with the Index from smbios structure and return it.
  The caller is responsible for free the string buffer.

  @param    OptionalStrStart  The start position to search the string
  @param    Index             The index of the string to extract
  @param    String            The string that is extracted

  @retval   EFI_SUCCESS            Get index string successfully.
  @retval   EFI_INVALID_PARAMETER  Index is zero. It is invalid value.
  @retval   EFI_ABORTED            Get missing string fail .
  @retval   EFI_OUT_OF_RESOURCES   Allocate memory fail.
  @retval   Other                  Get setup browser data fail.

**/
EFI_STATUS
GetOptionalStringByIndex (
  IN  CHAR8                        *OptionalStrStart,
  IN  UINT8                        Index,
  OUT CHAR16                       **String
  )
{
  UINTN                                 StrSize;
  CHAR16                                *StringBuffer;

  if (Index == 0) {
    return EFI_INVALID_PARAMETER;
  }

  StrSize = 0;
  do {
    Index--;
    OptionalStrStart += StrSize;
    StrSize           = AsciiStrSize (OptionalStrStart);
  } while (OptionalStrStart[StrSize] != 0 && Index != 0);

  if ((Index != 0) || (StrSize == 1)) {
    //
    // Meet the end of strings set but Index is non-zero
    //
    return EFI_ABORTED;
  }

  StringBuffer = AllocatePool (StrSize * sizeof (CHAR16));
  if (StringBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  AsciiStrToUnicodeStr (OptionalStrStart, StringBuffer);

  *String = StringBuffer;

  return EFI_SUCCESS;
}

CHAR16 *
GetSystemInfoStr (
  VOID
  )
{
  EFI_SMBIOS_TABLE_HEADER     *Record;
  EFI_SMBIOS_HANDLE           SmbiosHandle;
  EFI_SMBIOS_PROTOCOL         *Smbios;
  EFI_STATUS                  Status;
  SMBIOS_TABLE_TYPE4          *Type4Record;
  SMBIOS_TABLE_TYPE17         *Type17Record;
  SMBIOS_TABLE_TYPE19         *Type19Record;
  CHAR16                      *ProcessorVerStr;
  CHAR16                      *MemClockSpeedStr;
  CHAR16                      *MemSizeStr;
  CHAR16                      *InfoStr;
  CHAR16                      *OrgInfoStr;
  UINTN                       InfoStrSize;
  UINT64                      MemorySize;

  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **) &Smbios);
  if (EFI_ERROR(Status)) {
    return NULL;
  }

  InfoStr      = NULL;
  InfoStrSize  = 0;
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  ProcessorVerStr  = NULL;
  MemClockSpeedStr = NULL;
  MemSizeStr       = NULL;

  do {
    Status = Smbios->GetNext (Smbios, &SmbiosHandle, NULL, &Record, NULL);
    if (EFI_ERROR(Status)) {
      break;
    }

    if (Record->Type == EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION) {
      Type4Record = (SMBIOS_TABLE_TYPE4 *) Record;
      GetOptionalStringByIndex ((CHAR8*)((UINT8*) Type4Record + Type4Record->Hdr.Length), Type4Record->ProcessorVersion, &ProcessorVerStr);
    }

    if (Record->Type == EFI_SMBIOS_TYPE_MEMORY_DEVICE) {
      Type17Record = (SMBIOS_TABLE_TYPE17 *) Record;

     if (Type17Record->ConfiguredMemoryClockSpeed != 0) {
        MemClockSpeedStr = CatSPrint (NULL, L"DRAM Frequency: %d MHz", Type17Record->ConfiguredMemoryClockSpeed);
      }
    }

    if (Record->Type == EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS) {
      Type19Record = (SMBIOS_TABLE_TYPE19 *) Record;

      if (Type19Record->StartingAddress == 0xFFFFFFFF && Type19Record->EndingAddress == 0xFFFFFFFF) {
        MemorySize = (RShiftU64((Type19Record->ExtendedEndingAddress - Type19Record->ExtendedStartingAddress), 30) + 1);
        MemSizeStr = CatSPrint (NULL, L"Memory Size: %d GB", MemorySize);
      } else {
        MemorySize = (RShiftU64((Type19Record->EndingAddress - Type19Record->StartingAddress), 10) + 1);
        MemSizeStr = CatSPrint (NULL, L"Memory Size: %d MB", MemorySize);
      }
    }
  } while(Status == EFI_SUCCESS);

  if (ProcessorVerStr != NULL) {
    InfoStr = ProcessorVerStr;
  }
  if (MemClockSpeedStr != NULL) {
    if (InfoStr == NULL) {
      InfoStr = MemClockSpeedStr;
    } else {
      OrgInfoStr = InfoStr;
      InfoStr = CatSPrint (NULL, L"%s\n%s", OrgInfoStr, MemClockSpeedStr);
      FreePool (OrgInfoStr);
    }
  }
  if (MemSizeStr != NULL) {
    if (InfoStr == NULL) {
      InfoStr = MemSizeStr;
    } else {
      OrgInfoStr = InfoStr;
      InfoStr = CatSPrint (NULL, L"%s\n%s", OrgInfoStr, MemSizeStr);
      FreePool (OrgInfoStr);
    }
  }

  return InfoStr;
}

EFI_STATUS
UpdateOwnerDrawText (
  IN UI_CONTROL                            *PanelControl,
  IN CHAR16                                *ControlName,
  IN CHAR16                                *TextStr
  )
{
  UI_MANAGER                               *Manager;
  UI_CONTROL                               *Control;

  if (PanelControl == NULL || ControlName == NULL || TextStr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Manager = PanelControl->Manager;

  Control = Manager->FindControlByName (Manager, ControlName);
  UiSetAttribute (Control, L"text", TextStr);

  return EFI_SUCCESS;
}

EFI_STATUS
OwnerDrawTimerFunc (
  IN UI_CONTROL                            *PanelControl
  )
{
  EFI_STATUS                               Status;
  EFI_TIME                                 Time;
  CHAR16                                   String[20];

  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (String, sizeof (String), L"%04d/%02d/%02d", Time.Year, Time.Month, Time.Day);
    UpdateOwnerDrawText (PanelControl, L"OwnerDrawDateValue", String);

    UnicodeSPrint (String, sizeof (String), L"%s", GetWeekdayStr (&Time));
    UpdateOwnerDrawText (PanelControl, L"OwnerDrawWeekdayValue", String);

    UnicodeSPrint (String, sizeof (String), L"%02d:%02d:%02d", Time.Hour, Time.Minute, Time.Second);
    UpdateOwnerDrawText (PanelControl, L"OwnerDrawTimeValue", String);
  }

  UpdateOwnerDrawText (PanelControl, L"OwnerDrawMainboardName"    , L"MAINBOARD");
  UpdateOwnerDrawText (PanelControl, L"OwnerDrawSystemName"       , L"SYSTEM");
  UpdateOwnerDrawText (PanelControl, L"OwnerDrawSystemTemperature", L"27\xB0" L"C");
  UpdateOwnerDrawText (PanelControl, L"OwnerDrawCpuName"          , L"CPU");
  UpdateOwnerDrawText (PanelControl, L"OwnerDrawTemperatureName"  , L"TEMPERATURE");
  UpdateOwnerDrawText (PanelControl, L"OwnerDrawCpuTemperature"   , L"30\xB0" L"C");

  return EFI_SUCCESS;
}


/**
  Set attribute to UI control data of UI owner draw

  @param[in] Control               Pointer to UI control
  @param[in] Name                  Attribute name
  @param[in] Value                 Attribute string

  @retval EFI_SUCCESS              Perform owner draw success
  @retval EFI_ABORTED              Control or manager data is not found
**/
BOOLEAN
EFIAPI
H2OOwnerDrawPanelSetAttribute (
  IN UI_CONTROL                 *Control,
  IN CHAR16                     *Name,
  IN CHAR16                     *Value
  )
{
  H2O_OWNER_DRAW_PANEL          *This;
  EFI_STATUS                    Status;

  This = (H2O_OWNER_DRAW_PANEL *) Control;

  if (StrCmp (Name, L"RefreshInterval") == 0) {
    This->RefreshInterval = (UINT32) StrToUInt (Value, 10, &Status);
    if (This->RefreshInterval != 0) {
      SetTimer (Control->Wnd, OWNER_DRAW_TIMER_ID, (UINT) This->RefreshInterval, NULL);
    }
  } else {
    return PARENT_CLASS_SET_ATTRIBUTE (CURRENT_CLASS, Control, Name, Value);
  }

  return TRUE;
}


LRESULT
EFIAPI
H2OOwnerDrawPanelProc (
  HWND   Hwnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  H2O_OWNER_DRAW_PANEL          *This;
  UI_CONTROL                    *Control;
  UI_MANAGER                    *Manager;
  CHAR16                        *SystemInfoStr;

  Manager = NULL;

  This = (H2O_OWNER_DRAW_PANEL *) GetWindowLongPtr (Hwnd, 0);
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  Control = (UI_CONTROL *)This;

  switch (Msg) {

  case WM_CREATE:
    This = (H2O_OWNER_DRAW_PANEL *) AllocateZeroPool (sizeof (H2O_OWNER_DRAW_PANEL));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Hwnd, 0);
      SetWindowLongPtr (Hwnd, 0, (INTN)This);
      SendMessage (Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    Manager = Control->Manager;
    XmlCreateControl (mOwnerDrawPanelChilds, Control);
    This->OwnerDrawList = Manager->FindControlByName (Manager, L"OwnerDrawList");
    OwnerDrawTimerFunc (Control);

    SystemInfoStr = GetSystemInfoStr ();
    if (SystemInfoStr != NULL) {
      UpdateOwnerDrawText (Control, L"OwnerDrawSystemInfo", SystemInfoStr);
      FreePool (SystemInfoStr);
    }
    break;

  case WM_TIMER:
    if (WParam == OWNER_DRAW_TIMER_ID) {
      OwnerDrawTimerFunc (Control);
    }
    break;

  case WM_DESTROY:
    if (This->RefreshInterval != 0) {
      KillTimer (Hwnd, OWNER_DRAW_TIMER_ID);
    }
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);
    break;


  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);
  }

  return 0;
}


H2O_OWNER_DRAW_PANEL_CLASS *
EFIAPI
GetH2OOwnerDrawPanelClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"H2OOwnerDrawPanel", (UI_CONTROL_CLASS *)GetControlClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc      = H2OOwnerDrawPanelProc;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetAttribute = H2OOwnerDrawPanelSetAttribute;

  return CURRENT_CLASS;
}

