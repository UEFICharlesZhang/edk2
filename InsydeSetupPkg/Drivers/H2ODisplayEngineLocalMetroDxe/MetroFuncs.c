/** @file
  Entry point and initial functions for H2O local Metro display engine driver

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
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
#include <Guid/BdsHii.h>

/**
  Get target value of metro display engine from H2O form browser.

  @param[out] TargetValue        Target value of metro display engine

  @retval EFI_SUCCESS            Get target value successful
  @retval EFI_NOT_FOUND          Can not find the target value.
  @retval Other                  Call H2O_FORM_BROWSER_PROTOCOL.GetCAll fail
**/
STATIC
EFI_STATUS
GetTargetValue (
  OUT INT32                                *TargetValue
  )
{
  EFI_STATUS                               Status;
  EFI_GUID                                 *MetroDEGuid;
  H2O_FORM_BROWSER_PROTOCOL                *FBProtocol;
  UINT32                                   Index;
  UINT32                                   IdCount;
  H2O_CONSOLE_ID                           *IdBuffer;
  UINT32                                   DevCount;
  H2O_FORM_BROWSER_CONSOLE_DEV             *DevBuffer;

  MetroDEGuid = &mMetroPrivate->DisplayEngine.Id;
  FBProtocol  = mMetroPrivate->FBProtocol;

  Status = FBProtocol->GetCAll (FBProtocol, &IdCount, &IdBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < IdCount; Index++) {
    Status = FBProtocol->GetCInfo (FBProtocol, IdBuffer[Index], &DevCount, &DevBuffer);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (CompareGuid (&DevBuffer[0].DisplayEngine, MetroDEGuid)) {
      *TargetValue = IdBuffer[Index];
      FreePool (DevBuffer);
      break;
    }

    FreePool (DevBuffer);
  }
  FreePool (IdBuffer);

  if (Index == IdCount) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

/**
  Register timer event in H2O form browser.

  @param[in] TimerId             A identifier to the fnction to be notified
  @param[in] TriggerTime         The number of 100ns units until the timer expires.

  @retval EFI_SUCCESS            Register timer successful.
  @retval Other                  Can not find target value or fail to register timer
**/
EFI_STATUS
RegisterTimerEvent (
  IN INT32                                 TimerId,
  IN UINT64                                TriggerTime
  )
{
  EFI_STATUS                               Status;
  H2O_DISPLAY_ENGINE_EVT_TIMER             TimerEvent;
  INT32                                    TargetValue;

  TargetValue = 0;
  Status = GetTargetValue (&TargetValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&TimerEvent, sizeof (H2O_DISPLAY_ENGINE_EVT_TIMER));
  TimerEvent.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_TIMER);
  TimerEvent.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_TIMER;
  TimerEvent.Hdr.Target = TargetValue;
  TimerEvent.TimerId    = TimerId;
  TimerEvent.Time       = TriggerTime;

  Status = mMetroPrivate->FBProtocol->RegisterTimer (
                                        mMetroPrivate->FBProtocol,
                                        (H2O_DISPLAY_ENGINE_EVT *) &TimerEvent,
                                        TimerEvent.Time
                                        );
  return Status;
}

EFI_STATUS
HotKeyFunc (
  IN HOT_KEY_ACTION             HotkeyAction
  )
{
  EFI_STATUS                    Status;
  BOOLEAN                       GoDown;
  EFI_HII_VALUE                 HiiValue;
  HWND                          Wnd;
  UI_CONTROL                    *Control;

  Status = EFI_SUCCESS;

  switch (HotkeyAction) {

  case HotkeyShowHelpMsg:
    Status = SendShowHelpNotify ();
    break;

  case HotKeyLoadDefault:
    Status = SendDefaultNotify ();
    break;

  case HotKeySaveAndExit:
    Status = SendSubmitExitNotify ();
    break;

  case HotKeyEnter:
    if (gFB->CurrentQ != NULL) {
      Status = SendOpenQNotify (gFB->CurrentQ->PageId, gFB->CurrentQ->QuestionId, gFB->CurrentQ->IfrOpCode);
    }
    break;

  case HotKeyDiscardExit:
    Status = SendDiscardExitNotify ();
    break;

  case HotKeySelectItemUp:
  case HotKeySelectItemDown:
    Wnd     = GetFocus ();
    Control = (UI_CONTROL *) GetWindowLongPtr (Wnd, 0);
    if (Control != NULL) {
      CONTROL_CLASS_WNDPROC (Control, Control->Wnd, WM_KEYDOWN, (HotkeyAction == HotKeySelectItemUp) ? VK_UP : VK_DOWN, 0);
    }
    break;

  case HotkeySelectMenuUp:
  case HotkeySelectMenuDown:
    Wnd     = GetFocus ();
    Control = (UI_CONTROL *) GetWindowLongPtr (Wnd, 0);
    if (Control != NULL) {
      CONTROL_CLASS_WNDPROC (Control, Control->Wnd, WM_KEYDOWN, (HotkeyAction == HotkeySelectMenuUp) ? VK_LEFT : VK_RIGHT, 0);
    }
    break;

  case HotkeyModifyValueUp:
  case HotkeyModifyValueDown:
    if (gFB->CurrentQ != NULL) {
      GoDown = HotkeyAction == HotkeyModifyValueDown ? TRUE : FALSE;

      Status = GetNextQuestionValue (gFB->CurrentQ, GoDown, &HiiValue);
      if (!EFI_ERROR (Status)) {
        Status = SendChangeQNotify (gFB->CurrentQ->PageId, gFB->CurrentQ->QuestionId, &HiiValue);
      }
    }
    break;

  case HotkeyNoAction:
    break;

  default:
    ASSERT(FALSE);
    Status = EFI_UNSUPPORTED;
    break;
  }

  return Status;
}

EFI_IMAGE_INPUT *
GetCurrentFormSetImage (
  VOID
  )
{
  EFI_STATUS            Status;
  EFI_IMAGE_INPUT       *PageImage;
  UINT32                Index;
  UINT32                HiiHandleIndex;
  H2O_FORM_BROWSER_SM   *SetupMenuData;

  Status = gFB->GetSMInfo (gFB, &SetupMenuData);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  PageImage      = NULL;
  HiiHandleIndex = gFB->CurrentP->PageId >> 16;
  for (Index = 0; Index < SetupMenuData->NumberOfSetupMenus; Index++) {
    if ((SetupMenuData->SetupMenuInfoList[Index].PageId >> 16) == HiiHandleIndex) {
      PageImage = SetupMenuData->SetupMenuInfoList[Index].PageImage;
      break;
    }
  }

  FreeSetupMenuData (SetupMenuData);

  return PageImage;
}

CHAR16 *
GetCurrentFormSetTitle (
  VOID
  )
{
  EFI_STATUS            Status;
  CHAR16                *PageTitle;
  UINT32                Index;
  UINT32                HiiHandleIndex;
  H2O_FORM_BROWSER_SM   *SetupMenuData;

  Status = gFB->GetSMInfo (gFB, &SetupMenuData);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  PageTitle      = NULL;
  HiiHandleIndex = gFB->CurrentP->PageId >> 16;
  for (Index = 0; Index < SetupMenuData->NumberOfSetupMenus; Index++) {
    if ((SetupMenuData->SetupMenuInfoList[Index].PageId >> 16) == HiiHandleIndex) {
      PageTitle = SetupMenuData->SetupMenuInfoList[Index].PageTitle;
      break;
    }
  }

  FreeSetupMenuData (SetupMenuData);

  return PageTitle;
}

BOOLEAN
NeedShowSetupMenu (
  VOID
  )
{
  EFI_STATUS            Status;
  UINT32                 NumberOfMenus;
  H2O_FORM_BROWSER_SM   *SetupMenuData;

  Status = gFB->GetSMInfo (gFB, &SetupMenuData);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }
  NumberOfMenus = SetupMenuData->NumberOfSetupMenus;
  FreeSetupMenuData (SetupMenuData);
  return (BOOLEAN) (NumberOfMenus != 1);
}

CHAR16 *
CatStringArray (
  IN CONST UINT32        StringCount,
  IN CONST CHAR16        **StringArray
  )
{
  UINT32        Index;
  UINTN         BufferSize;
  CHAR16        *StrBuffer;

  if (StringCount == 0 || StringArray == NULL) {
    return NULL;
  }

  BufferSize = 0;
  for (Index = 0; Index < StringCount; Index++) {
    BufferSize += StrSize (StringArray[Index]);
  }
  BufferSize += sizeof (CHAR16);

  StrBuffer = AllocateZeroPool (BufferSize);
  if (StrBuffer == NULL) {
    return NULL;
  }

  for (Index = 0; Index < StringCount; Index++) {
    StrCat (StrBuffer, StringArray[Index]);
    if (Index != StringCount - 1) {
      StrCat (StrBuffer, L"\n");
    }
  }
  return StrBuffer;
}

VOID
FreeStringArray (
  IN CHAR16                                **StringArray,
  IN UINT32                                StringArrayCount
  )
{
  UINT32                                   Index;

  if (StringArray == NULL || StringArrayCount == 0) {
    return;
  }

  for (Index = 0; Index < StringArrayCount; Index++) {
    if (StringArray[Index] != NULL) {
      FreePool (StringArray[Index]);
    }
  }
  FreePool (StringArray);
}

CHAR16 **
CopyStringArray (
  IN CHAR16                                **StringArray,
  IN UINT32                                StringArrayCount
  )
{
  CHAR16                                   **Buffer;
  UINT32                                   Index;

  if (StringArray == NULL || StringArrayCount == 0) {
    return NULL;
  }

  Buffer = AllocateZeroPool (sizeof (CHAR16 *) * StringArrayCount);
  if (Buffer == NULL) {
    return NULL;
  }

  for (Index = 0; Index < StringArrayCount; Index++) {
    if (StringArray[Index] == NULL) {
      continue;
    }

    Buffer[Index] = AllocateCopyPool (StrSize (StringArray[Index]), StringArray[Index]);
    if (Buffer[Index] == NULL) {
      goto Error;
    }
  }

  return Buffer;

Error:
  FreeStringArray (Buffer, StringArrayCount);

  return NULL;
}

UINT32
GetStringArrayCount (
  IN CHAR16                                **StringArray
  )
{
  UINT32                                   Count;

  Count = 0;
  if (StringArray == NULL) {
    return Count;
  }

  while (StringArray[Count] != NULL) {
    Count++;
  }

  return Count;
}

VOID
FreeHiiValueArray (
  IN EFI_HII_VALUE                         *HiiValueArray,
  IN UINT32                                HiiValueArrayCount
  )
{
  UINT32                                   Index;

  if (HiiValueArray == NULL || HiiValueArrayCount == 0) {
    return;
  }

  for (Index = 0; Index < HiiValueArrayCount; Index++) {
    if (HiiValueArray[Index].Type == EFI_IFR_TYPE_BUFFER && HiiValueArray[Index].Buffer != NULL) {
      FreePool (HiiValueArray[Index].Buffer);
    }
  }
  FreePool (HiiValueArray);
}

CHAR16 **
CreateStrArrayByNumRange (
  IN UINT32                                MinValue,
  IN UINT32                                MaxValue,
  IN UINT32                                Step,
  IN CHAR16                                *FormattedStr
  )
{
  UINT32                                   Index;
  UINT32                                   StrListCount;
  CHAR16                                   **StrList;
  CHAR16                                   Str[20];

  StrListCount = (MaxValue - MinValue) / Step + 1;
  StrList      = AllocatePool ((StrListCount + 1) * sizeof(CHAR16 *));
  if (StrList == NULL) {
    return NULL;
  }

  for (Index = 0; Index < StrListCount; Index += Step) {
    UnicodeSPrint (Str, sizeof (Str), FormattedStr, MinValue + Index);
    StrList[Index] = AllocateCopyPool (StrSize (Str), Str);
    if (StrList[Index] == NULL) {
      return NULL;
    }
  }
  StrList[StrListCount] = NULL;

  return StrList;
}


EFI_HII_VALUE *
CopyHiiValueArray (
  IN EFI_HII_VALUE                         *HiiValueArray,
  IN UINT32                                HiiValueArrayCount
  )
{
  EFI_HII_VALUE                            *Buffer;
  UINT32                                   Index;

  if (HiiValueArray == NULL || HiiValueArrayCount == 0) {
    return NULL;
  }

  Buffer = AllocatePool (sizeof (EFI_HII_VALUE) * HiiValueArrayCount);
  if (Buffer == NULL) {
    return NULL;
  }

  for (Index = 0; Index < HiiValueArrayCount; Index++) {
    CopyMem (&Buffer[Index], &HiiValueArray[Index], sizeof(EFI_HII_VALUE));

    if (HiiValueArray[Index].Type == EFI_IFR_TYPE_BUFFER && HiiValueArray[Index].Buffer != NULL && HiiValueArray[Index].BufferLen != 0) {
      Buffer->Buffer = AllocateCopyPool (HiiValueArray[Index].BufferLen, HiiValueArray[Index].Buffer);
    }
  }

  return Buffer;
}

EFI_STATUS
CompareHiiValue (
  IN  EFI_HII_VALUE                        *Value1,
  IN  EFI_HII_VALUE                        *Value2,
  OUT INTN                                 *Result
  )
{
  INT64   Temp64;
  UINTN   Len;

  if (Value1->Type >= EFI_IFR_TYPE_OTHER || Value2->Type >= EFI_IFR_TYPE_OTHER ) {
    if (Value1->Type != EFI_IFR_TYPE_BUFFER && Value2->Type != EFI_IFR_TYPE_BUFFER) {
      return EFI_UNSUPPORTED;
    }
  }

  if (Value1->Type == EFI_IFR_TYPE_STRING || Value2->Type == EFI_IFR_TYPE_STRING ) {
    if (Value1->Type != Value2->Type) {
      //
      // Both Operator should be type of String
      //
      return EFI_UNSUPPORTED;
    }

    if (Value1->Value.string == 0 || Value2->Value.string == 0) {
      //
      // StringId 0 is reserved
      //
      return EFI_INVALID_PARAMETER;
    }

    if (Value1->Value.string == Value2->Value.string) {
      *Result = 0;
      return EFI_SUCCESS;
    }

    *Result = 1;
    return EFI_SUCCESS;
  }

  if (Value1->Type == EFI_IFR_TYPE_BUFFER || Value2->Type == EFI_IFR_TYPE_BUFFER) {
    if (Value1->Type != Value2->Type) {
      //
      // Both Operator should be type of Buffer.
      //
      return EFI_UNSUPPORTED;
    }
    Len = Value1->BufferLen > Value2->BufferLen ? Value2->BufferLen : Value1->BufferLen;
    *Result = CompareMem (Value1->Buffer, Value2->Buffer, Len);
    if ((*Result == 0) && (Value1->BufferLen != Value2->BufferLen)) {
      //
      // In this case, means base on samll number buffer, the data is same
      // So which value has more data, which value is bigger.
      //
      *Result = Value1->BufferLen > Value2->BufferLen ? 1 : -1;
    }
    return EFI_SUCCESS;
  }

  //
  // Take remain types(integer, boolean, date/time) as integer
  //
  Temp64 = (INT64) (Value1->Value.u64 - Value2->Value.u64);
  if (Temp64 > 0) {
    *Result = 1;
  } else if (Temp64 < 0) {
    *Result = -1;
  } else {
    *Result = 0;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetRectByName (
  IN  HWND             Wnd,
  IN  CHAR16           *Name,
  OUT RECT             *Rect
  )
{
  UI_CONTROL                    *Control;


  Control = (UI_CONTROL *) GetWindowLongPtr (Wnd, 0);
  if (Control == NULL) {
    return EFI_NOT_FOUND;
  }
  Control = UiFindChildByName (Control, Name);
  if (Control == NULL) {
    return  EFI_NOT_FOUND;
  }
  GetWindowRect (Control->Wnd, Rect);

  return EFI_SUCCESS;
}

VOID
GrayOutBackground (
  IN   HWND       Wnd,
  IN   BOOLEAN    GrayOut
  )
{
  UI_CONTROL                    *Control;

  Control = (UI_CONTROL *) GetWindowLongPtr (Wnd, 0);
  if (Control == NULL) {
    return;
  }
  Control = UiFindChildByName (Control, L"overlay");
  if (Control == NULL) {
    return;
  }

  if (GrayOut) {
    UiSetAttribute (Control, L"visible", L"true");
  } else {
    UiSetAttribute (Control, L"visible", L"false");
  }
}


VOID
RGB2HSV (
  IN CONST EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *Pixel,
  OUT HSV_VALUE                            *Hsv
  )
{
  UINTN    Max;
  UINTN    Min;
  INTN     Delta;
  INTN    TmpH;
  UINT8   R;
  UINT8   G;
  UINT8   B;

  R = Pixel->Red;
  G = Pixel->Green;
  B = Pixel->Blue;
  ASSERT (R <= 0xFF && G <= 0xFF && B <= 0xFF);
  ASSERT (Pixel != NULL && Hsv != NULL);

  Max = MAX (MAX (R, G), B);
  Min = MIN (MIN (R, G), B);
  Delta = (INTN) (Max - Min);

  Hsv->Value = (UINT8)(Max * 100 / 255);
  if (Max != 0) {
    Hsv->Saturation = (UINT8) (Delta * 100 / Max);
  } else {
    Hsv->Saturation = 0;
  }

  if (Hsv->Saturation == 0) {
    TmpH = 0;
  } else {
    if (R == Max) {
      TmpH = ((INTN) G - (INTN) B) * 60 / Delta;
    } else if (G == Max) {
      TmpH = 120 + ((INTN) B - (INTN) R) * 60 / Delta;
    } else { // B == MAX
      TmpH = 240 + ((INTN) R - (INTN) G) * 60 / Delta;
    }

    if (TmpH < 0) {
      TmpH += 360;
    }
  }
  ASSERT (TmpH <= 360 && TmpH >= 0);
  Hsv->Hue = (UINT16) TmpH;
}

VOID
HSV2RGB (
  IN CONST HSV_VALUE                      *Hsv,
  OUT      EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *Pixel
  )
{
  UINTN   I;
  UINTN   F;
  UINTN   M;
  UINTN   N;
  UINTN   K;
  UINT16  H;
  UINT8   S;
  UINT8   V;

  ASSERT (Hsv->Hue <= 360 && Hsv->Saturation <= 100 && Hsv->Value <= 100);
  if (Hsv->Saturation == 0) {
    Pixel->Red   = 0;
    Pixel->Green = 0;
    Pixel->Blue  = 0;
  } else {
    H = Hsv->Hue;
    S = Hsv->Saturation;
    V = Hsv->Value;
    if ( H >= 360) {
      H = 0;
    }

    I = H / 60;
    F = H - I * 60;

    M = ((V * (100 - S)) * 255) / 10000;
    N = (V * (6000 - S * F) * 255) / 600000;
    K = ((V * (6000 - S * (60 - F))) * 255) / 600000;

    V = V * 255 / 100 ;

    Pixel->Red = 0;
    Pixel->Green = 0;
    Pixel->Blue = 0;

    if (I == 0) { Pixel->Red = V;         Pixel->Green = (UINT8) K; Pixel->Blue = (UINT8) M; }
    if (I == 1) { Pixel->Red = (UINT8) N; Pixel->Green = V;         Pixel->Blue = (UINT8) M; }
    if (I == 2) { Pixel->Red = (UINT8) M; Pixel->Green = V;         Pixel->Blue = (UINT8) K; }
    if (I == 3) { Pixel->Red = (UINT8) M; Pixel->Green = (UINT8) N; Pixel->Blue = V;         }
    if (I == 4) { Pixel->Red = (UINT8) K; Pixel->Green = (UINT8) M; Pixel->Blue = V;         }
    if (I == 5) { Pixel->Red = V;         Pixel->Green = (UINT8) M; Pixel->Blue = (UINT8) N; }

  }
}


VOID
GetCurrentHaloHsv (
  OUT HSV_VALUE      *Hsv
  )
{
  EFI_IMAGE_INPUT         *Image;

  Image = GetImageByString (L"@FormHalo");
  if (Image != NULL) {
    RGB2HSV (Image->Bitmap, Hsv);
  }
}

STATIC
EFI_STATUS
GetImageHsv (
  IN  EFI_IMAGE_INPUT                *ImageIn,
  OUT HSV_VALUE                      *Hsv
  )
{
  UINTN    Index;
  UINTN    BufferLen;

  ASSERT (ImageIn != NULL && ImageIn->Bitmap != NULL && Hsv != NULL);

  BufferLen = ImageIn->Width * ImageIn->Height;

  for (Index = 0; Index < BufferLen; Index++) {
    if (ImageIn->Bitmap[Index].Reserved != 0) {
      RGB2HSV (&ImageIn->Bitmap[Index], Hsv);
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}


VOID
GetCurrentMenuHsv (
  OUT HSV_VALUE      *Hsv
  )
{

  EFI_IMAGE_INPUT               *PageImage;

  PageImage = GetCurrentFormSetImage ();
  if (PageImage == NULL) {
    ZeroMem (Hsv, sizeof (HSV_VALUE));
    return;
  }
  GetImageHsv (PageImage, Hsv);
}

BOOLEAN
IsFrontPage (
  VOID
  )
{
  H2O_FORM_BROWSER_SM           *SetupMenuData;
  EFI_STATUS                    Status;

  ASSERT (gFB != NULL);

  if (gFB->CurrentP == NULL) {
    return FALSE;
  }

  Status = gFB->GetSMInfo (gFB, &SetupMenuData);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (CompareGuid (&SetupMenuData->FormSetGuid, &gFrontPageFormSetGuid)) {
    FreeSetupMenuData (SetupMenuData);
    return TRUE;
  }

  FreeSetupMenuData (SetupMenuData);
  return FALSE;
}
