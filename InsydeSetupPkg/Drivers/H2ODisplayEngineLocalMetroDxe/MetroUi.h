/** @file

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

#ifndef __METRO_UI_H
#define __METRO_UI_H

#include "UiControls.h"

#define FB_NOTIFY_REFRESH          (WM_APP + 1)
#define FB_NOTIFY_REPAINT          (WM_APP + 2)
#define FB_NOTIFY_CHECK_SELECTION  (WM_APP + 3)
#define FB_NOTIFY_RESET            (WM_APP + 4)
#define FB_NOTIFY_LOAD_DEFAULT     (WM_APP + 5)
#define FB_NOTIFY_SAVE_AND_EXIT    (WM_APP + 6)
#define FB_NOTIFY_SELECT_Q         (WM_APP + 7)
#define FB_NOTIFY_REFRESH_Q        (WM_APP + 8)

extern HWND gWnd;

EFI_STATUS
EFIAPI
LocalMetroDialogNotify (
  IN       H2O_DISPLAY_ENGINE_PROTOCOL     *This,
  IN CONST H2O_DISPLAY_ENGINE_EVT          *Notify
  );

VOID
AddHiiImagePackage (
  VOID
  );

EFI_STATUS
HotKeyFunc (
  IN HOT_KEY_ACTION             HotkeyAction
  );

EFI_IMAGE_INPUT *
GetCurrentFormSetImage (
  VOID
  );

CHAR16 *
GetCurrentFormSetTitle (
  VOID
  );

BOOLEAN
NeedShowSetupMenu (
  VOID
  );


CHAR16 *
CatStringArray (
  IN CONST UINT32        StringCount,
  IN CONST CHAR16        **StringArray
  );

VOID
FreeStringArray (
  IN CHAR16                                **StringArray,
  IN UINT32                                StringArrayCount
  );

CHAR16 **
CopyStringArray (
  IN CHAR16                                **StringArray,
  IN UINT32                                StringArrayCount
  );

UINT32
GetStringArrayCount (
  IN CHAR16                                **StringArray
  );

VOID
FreeHiiValueArray (
  IN EFI_HII_VALUE                         *HiiValueArray,
  IN UINT32                                HiiValueArrayCount
  );

CHAR16 **
CreateStrArrayByNumRange (
  IN UINT32                                MinValue,
  IN UINT32                                MaxValue,
  IN UINT32                                Step,
  IN CHAR16                                *FormattedStr
  );

EFI_HII_VALUE *
CopyHiiValueArray (
  IN EFI_HII_VALUE                         *HiiValueArray,
  IN UINT32                                HiiValueArrayCount
  );

EFI_STATUS
CompareHiiValue (
  IN  EFI_HII_VALUE                        *Value1,
  IN  EFI_HII_VALUE                        *Value2,
  OUT INTN                                 *Result
  );

EFI_STATUS
GetRectByName (
  IN  HWND             Wnd,
  IN  CHAR16           *Name,
  OUT RECT             *Rect
  );

VOID
GrayOutBackground (
  IN   HWND       Wnd,
  IN   BOOLEAN    Grayout
  );

EFI_STATUS
InitializeWindows (
  VOID
  );

VOID
RGB2HSV (
  IN CONST EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *Pixel,
  OUT HSV_VALUE                            *Hsv
  );


VOID
HSV2RGB (
  IN CONST HSV_VALUE                      *Hsv,
  OUT      EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *Pixel
  );


VOID
GetCurrentHaloHsv (
  OUT HSV_VALUE      *Hsv
  );

VOID
GetCurrentMenuHsv (
  OUT HSV_VALUE      *Hsv
  );

EFI_STATUS
GetInformationField (
  OUT RECT                                    *InfoField
  );

BOOLEAN
IsFrontPage (
  VOID
  );

VOID
H2OCommonDialogWithHelpOnSetState (
  UI_CONTROL                    *Control,
  UI_STATE                      SetState,
  UI_STATE                      ClearState
  );


#endif

