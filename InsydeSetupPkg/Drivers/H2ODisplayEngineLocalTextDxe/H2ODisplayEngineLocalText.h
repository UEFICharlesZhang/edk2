/** @file
 Function definition for H2O local text display engine

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

#ifndef _H2O_DISPLAY_ENGINE_LOCAL_TEXT_H_
#define _H2O_DISPLAY_ENGINE_LOCAL_TEXT_H_

#include <Uefi.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/H2OFormBrowser.h>
#include <Protocol/H2ODisplayEngine.h>
#include <Guid/H2ODisplayEngineType.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/RectLib.h>
#include <Library/ConsoleLib.h>
#include <Library/LayoutLib.h>
#include <Library/H2ODisplayEngineLib.h>
#include <Include/Guid/MdeModuleHii.h>
#include <KernelSetupConfig.h>
#include "LTDEHotPlug.h"
#include "LTDEMisc.h"

#define EISA_SERIAL_DEVICE_ID    0x0501

#define CHAR_SPACE            0x0020
#define CHAR_ADD              0x002B
#define CHAR_SUB              0x002D

#define NARROW_ATTRIBUTE      0x7F
#define WIDE_ATTRIBUTE        0x80

#define NARROW_TEXT_WIDTH     8
#define WIDE_TEXT_WIDTH       16
#define TEXT_HEIGHT           19
#define MOUSE_WIDTH           16
#define MOUSE_HEIGHT          19

//
// Display definitions
//
#define REF_OP_DELIMITER          GEOMETRICSHAPE_RIGHT_TRIANGLE

#define LEFT_ONEOF_DELIMITER      L'<'
#define RIGHT_ONEOF_DELIMITER     L'>'

#define LEFT_NUMERIC_DELIMITER    L'['
#define RIGHT_NUMERIC_DELIMITER   L']'

#define LEFT_CHECKBOX_DELIMITER   L'['
#define RIGHT_CHECKBOX_DELIMITER  L']'

#define CHECK_ON                  L'X'
#define CHECK_OFF                 L' '

#define TIME_SEPARATOR            L':'
#define DATE_SEPARATOR            L'/'

#define PROMPT_VALUE_SEPARATOR_WIDTH    1

#define   BORDER_TOP_WIDTH_DEFLULT      0x01
#define   BORDER_BOTTOM_WIDTH_DEFLULT   0x01
#define   BORDER_LEFT_WIDTH_DEFLULT     0x01
#define   BORDER_RIGHT_WIDTH_DEFLULT    0x01

#define H2O_DISPLAY_ENGINE_SIGNATURE SIGNATURE_32 ('H', 'D', 'E', 'S')
typedef struct _H2O_DISPLAY_ENGINE_PRIVATE_DATA {
  UINT32                                       Signature;
  EFI_HANDLE                                   ImageHandle;

  H2O_DISPLAY_ENGINE_PROTOCOL                  DisplayEngine;
  UINT8                                        DEStatus;

  H2O_LAYOUT_INFO                              *Layout;
  H2O_CONTROL_INFO                             *PageTagSelected;
  H2O_CONTROL_INFO                             *MenuSelected;
  H2O_CONTROL_INFO                             *PopUpSelected;

  H2O_FORM_BROWSER_PROTOCOL                    *FBProtocol;
  LIST_ENTRY                                   ConsoleDevListHead;

} H2O_DISPLAY_ENGINE_PRIVATE_DATA;

#define H2O_DISPLAY_ENGINE_PRIVATE_DATA_FROM_PROTOCOL(a) CR (a, H2O_DISPLAY_ENGINE_PRIVATE_DATA, DisplayEngine, H2O_DISPLAY_ENGINE_SIGNATURE)

typedef enum {
  H2O_DISPLAY_ENGINE_TYPE_UNDEFINED = 0,
  H2O_DISPLAY_ENGINE_TYPE_LTDE,
  H2O_DISPLAY_ENGINE_TYPE_RTDE,
  H2O_DISPLAY_ENGINE_TYPE_RBDE,
  H2O_DISPLAY_ENGINE_TYPE_LCDE,
  H2O_DISPLAY_ENGINE_TYPE_LMDE
} H2O_DISPLAY_ENGINE_TYPE;

typedef enum {
  CONSOLE_DEVICE_STATUS_INIT = 0,
  CONSOLE_DEVICE_STATUS_READY
} CONSOLE_DEVICE_STATUS;

#define H2O_FORM_BROWSER_CONSOLE_DEV_NODE_SIGNATURE SIGNATURE_32 ('F', 'B', 'C', 'D')

typedef struct _H2O_FORM_BROWSER_CONSOLE_DEV_NODE {
  UINT32                                       Signature;
  H2O_FORM_BROWSER_CONSOLE_DEV                 *ConsoleDev;
  LIST_ENTRY                                   Link;
  EFI_HANDLE                                   Handle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL              *SimpleTextOut;
  EFI_GRAPHICS_OUTPUT_PROTOCOL                 *GraphicsOut;
  UINT8                                        ConDevStatus;
} H2O_FORM_BROWSER_CONSOLE_DEV_NODE;


#define H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK(a) CR (a, H2O_FORM_BROWSER_CONSOLE_DEV_NODE, Link, H2O_FORM_BROWSER_CONSOLE_DEV_NODE_SIGNATURE)

typedef enum {
  DISPLAY_ENGINE_STATUS_INIT = 0,
  DISPLAY_ENGINE_STATUS_AT_MENU,
  DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG,
  DISPLAY_ENGINE_STATUS_OPEN_D,
  DISPLAY_ENGINE_STATUS_SHUT_D,
  DISPLAY_ENGINE_STATUS_CHANGING_Q,
  DISPLAY_ENGINE_STATUS_SELECT_Q,
  DISPLAY_ENGINE_STATUS_SHUT_Q
} DISPLAY_ENGINE_STATUS;

EFI_STATUS
DEEventCallback (
  IN       H2O_DISPLAY_ENGINE_PROTOCOL        *This,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify
  );

EFI_STATUS
DEConOutSetAttribute (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA        *DEPrivateData,
  IN UINTN                                  Attribute
  );

EFI_STATUS
DEConOutSetCursorPosition (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA        *DEPrivateData,
  IN UINTN                                  Column,
  IN UINTN                                  Row
  );

EFI_STATUS
DEConOutEnableCursor (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA        *DEPrivateData,
  IN BOOLEAN                                Visible
  );

EFI_STATUS
DEConOutQueryModeWithoutModeNumer (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA        *DEPrivateData,
  OUT UINT32                                *Columns,
  OUT UINT32                                *Rows
  );

EFI_STATUS
DEConOutGetAttribute (
  IN    H2O_DISPLAY_ENGINE_PRIVATE_DATA     *DEPrivateData,
  OUT   INT32                               *Attribute
  );

EFI_STATUS
DEConOutSetNarrowAttribute (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA        *DEPrivateData
  );

EFI_STATUS
DEConOutOutputString (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA        *DEPrivateData,
  IN CHAR16                                 *String
  );

EFI_STATUS
DEGraphicsOutQueryModeWithoutModeNumer (
  IN  H2O_DISPLAY_ENGINE_PRIVATE_DATA       *DEPrivateData,
  OUT UINT32                                *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  );

EFI_STATUS
DEGraphicsOutBlt (
  IN  H2O_DISPLAY_ENGINE_PRIVATE_DATA       *DEPrivateData,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *BltBuffer OPTIONAL,
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION     BltOperation,
  IN  UINTN                                 SourceX,
  IN  UINTN                                 SourceY,
  IN  UINTN                                 DestinationX,
  IN  UINTN                                 DestinationY,
  IN  UINTN                                 Width,
  IN  UINTN                                 Height,
  IN  UINTN                                 Delta OPTIONAL
  );

EFI_STATUS
TransferToTextModePosition (
  IN  H2O_DISPLAY_ENGINE_PRIVATE_DATA        *Private,
  IN  INT32                                  GopX,
  IN  INT32                                  GopY,
  OUT UINT32                                 *Column,
  OUT UINT32                                 *Row
  );

EFI_STATUS
PanelPageUp (
  IN     H2O_PANEL_INFO                      *Panel
  );

EFI_STATUS
PanelPageDown (
  IN     H2O_PANEL_INFO                      *Panel
  );

EFI_STATUS
EnsureControlInPanel (
  IN     H2O_CONTROL_INFO                    *Control
  );

EFI_STATUS
ProcessLayout (
  VOID
  );

EFI_STATUS
DisplayLayout (
  VOID
  );

EFI_STATUS
DisplaySetupMenu (
  IN     H2O_FORM_BROWSER_PROTOCOL           *FBProtocol
  );

EFI_STATUS
DisplayHotkey (
  VOID
  );

EFI_STATUS
DisplayTitle (
  IN     CONST CHAR16                              *TitleString,
  IN     CONST CHAR16                              *CoreVersionString
  );

EFI_STATUS
DisplaySetupPage (
  IN     H2O_FORM_BROWSER_PROTOCOL           *FBProtocol
  );

EFI_STATUS
DisplayHighLightControl (
  IN     BOOLEAN                             BottonStartEndChar,
  IN     H2O_CONTROL_INFO                    *Control
  );

EFI_STATUS
DisplayControls (
  IN     CONST BOOLEAN                       IsCentered,
  IN     BOOLEAN                             ButtonStartEndChar,
  IN     UINT32                              ControlCount,
  IN     H2O_CONTROL_INFO                    *ControlArray
  );

EFI_STATUS
DisplayNormalControls (
  IN     UINT32                              ControlCount,
  IN     H2O_CONTROL_INFO                    *ControlArray
  );

EFI_STATUS
DisplayButtonControls (
  IN     BOOLEAN                             BottonStartEndChar,
  IN     UINT32                              ControlCount,
  IN     H2O_CONTROL_INFO                    *ControlArray
  );

EFI_STATUS
DisplayDialog (
  IN     BOOLEAN                              Refresh,
  IN     CONST H2O_FORM_BROWSER_D             *Dialog,
  OUT    H2O_PANEL_INFO                       **OldDialogPanel
  );

VOID
InitControlList (
 IN     H2O_CONTROL_LIST                    *ControlList
 );

EFI_STATUS
FreeControlList (
  IN     H2O_CONTROL_LIST                    *ControlList
  );

VOID
CopyPanel(
  IN   H2O_PANEL_INFO                        *DestinationPanel,
  IN   H2O_PANEL_INFO                        *SourcePanel
);

H2O_PANEL_INFO *
GetPanelInfoByType (
  IN LIST_ENTRY                              *PanelListHead,
  IN UINT32                                  PanelType
  );

CHAR16 *
GetDevicePathStr (
  IN EFI_DEVICE_PATH_PROTOCOL                *DevicePath
  );

EFI_STATUS
DEReplaceFakeConOutDev (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA       *Private,
  IN EFI_HANDLE                            ControllerHandle
  );

EFI_STATUS
DERemoveFakeConOutDev (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA       *Private
  );

EFI_STATUS
CreateValueAsUint64 (
  IN EFI_HII_VALUE                         *TargetHiiValue,
  IN UINT64                                ValueUint64
  )
  ;

EFI_STATUS
CreateValueAsString (
  IN EFI_HII_VALUE                         *TargetHiiValue,
  IN UINT16                                BufferLen,
  IN UINT8                                 *Buffer
  );

BOOLEAN
CheckPressControls (
  IN     BOOLEAN                              Keyboard,
  IN     EFI_INPUT_KEY                        *Key,
  IN     INT32                                RealMouseX,
  IN     INT32                                RealMouseY,
  IN     UINT32                               ControlCount,
  IN     H2O_CONTROL_INFO                     *ControlArray,
  IN OUT H2O_CONTROL_INFO                     **SelectedControl
  );

BOOLEAN
IsDialogStatus (
  IN UINT8                                    DEStatus
  );

EFI_STATUS
FreePanel (
  IN     H2O_PANEL_INFO                      *Panel
  );

BOOLEAN
IsPointOnField (
  IN RECT                                     *ControlAbsField,
  IN INT32                                    Column,
  IN INT32                                    Row
  );

EFI_STATUS
GetControlAbsField (
  IN  RECT                                    *ParentPanelField,
  IN  INT32                                   BorderLineWidth,
  IN  RECT                                    *ControlRelField,
  OUT RECT                                    *ControlAbsField
  );

UINT8
GetSequenceNum (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private,
  IN H2O_CONTROL_INFO                         *Control,
  IN INT32                                    Column,
  IN INT32                                    Row
  );

EFI_STATUS
DisplayString (
  IN UINT32                        StartX,
  IN UINT32                        StartY,
  IN CHAR16                        *String
  );

EFI_STATUS
DisplaySetupPageControl (
  IN H2O_PANEL_INFO                          *SetupPagePanel,
  IN H2O_CONTROL_INFO                        *Control,
  IN BOOLEAN                                 IsHighlight
  );

EFI_STATUS
DisplaySetupPageControls (
  IN H2O_PANEL_INFO                          *SetupPagePanel
  );

EFI_STATUS
SetSetupPageControlsField (
  IN H2O_PANEL_INFO                           *SetupPagePanel
  );

H2O_DISPLAY_ENGINE_PRIVATE_DATA             *mDEPrivate;
#endif

