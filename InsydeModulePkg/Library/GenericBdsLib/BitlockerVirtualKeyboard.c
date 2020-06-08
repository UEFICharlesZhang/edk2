/** @file
  Routines for displaying virtual keyboard in bitlocker

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <PiDxe.h>
#include <Guid/ReturnFromImage.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SetupMouse.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/AbsolutePointer.h>
#include "InternalBdsLib.h"

#define TICKS_PER_MS            10000U

typedef struct _BITLOCKER_PRIVATE {
  EFI_GRAPHICS_OUTPUT_PROTOCOL        *GraphicsOutput;
  EFI_SIMPLE_POINTER_PROTOCOL         *SimplePointer;
  EFI_ABSOLUTE_POINTER_PROTOCOL       *AbsolutePointer;
  EFI_SETUP_MOUSE_PROTOCOL            *SetupMouse;

  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL   *SimpleTextInputEx;
  EFI_INPUT_READ_KEY_EX               OrgReadKeyStrokeEx;

  EFI_EVENT                           CheckReadKeyEvent;
  EFI_EVENT                           DisableCheckReadKeyEvent;

  UINTN                               NoReadKeyCount;

  BOOLEAN                             FirstIn;
  BOOLEAN                             FirstDisplayKB;
  BOOLEAN                             ReadKeyFlag;
  BOOLEAN                             SetupMouseIsStart;
} BITLOCKER_PRIVATE;

STATIC BITLOCKER_PRIVATE *mBitlocker = NULL;

EFI_STATUS
EFIAPI
BdsLibGetImageHeader (
  IN  EFI_HANDLE                            Device,
  IN  CHAR16                                *FileName,
  OUT EFI_IMAGE_DOS_HEADER                  *DosHeader,
  OUT EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION   Hdr
  );

/**

 Check the boot device path whether is Windows.

 @param [in]   DeivcePath       Device path of boot option

 @return                        Whether is Windows boot option

**/
BOOLEAN
IsWindowsOS (
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  EFI_STATUS                    Status;
  EFI_DEVICE_PATH_PROTOCOL      *WorkingDevicePath;
  EFI_HANDLE                    Device;
  FILEPATH_DEVICE_PATH          *FilePath;
  EFI_IMAGE_DATA_DIRECTORY      ResourceDataDir;
  UINT8                         *ResourceData;
  UINT32                        ResourceDataSize;
  UINT32                        VersionMS;
  UINT32                        VersionLS;


  WorkingDevicePath = DevicePath;
  Status = gBS->LocateDevicePath (
                  &gEfiSimpleFileSystemProtocolGuid,
                  &WorkingDevicePath,
                  &Device
                  );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (DevicePathType (WorkingDevicePath) != MEDIA_DEVICE_PATH ||
      DevicePathSubType (WorkingDevicePath) != MEDIA_FILEPATH_DP) {
    return FALSE;
  }

  FilePath = (FILEPATH_DEVICE_PATH *) WorkingDevicePath;
  Status = GetResourceSectionData (Device, FilePath->PathName, &ResourceDataDir, &ResourceData, &ResourceDataSize);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  Status = GetWindowsOsVer (&ResourceDataDir, ResourceData, &VersionMS, &VersionLS);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}

/**

 Close check read key event

 @param [in] Event              Event
 @param [in] Context            Passed parameter to event handler

**/
VOID
EFIAPI
DisableCheckReadKeyHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  BITLOCKER_PRIVATE             *Private;

  Private = (BITLOCKER_PRIVATE *) Context;

  gBS->CloseEvent (Private->DisableCheckReadKeyEvent);
  Private->DisableCheckReadKeyEvent = NULL;

  if (Private->CheckReadKeyEvent != NULL) {
    gBS->CloseEvent (Private->CheckReadKeyEvent);
    Private->CheckReadKeyEvent = NULL;
  }
}


/**

 Close setup mouse if no read key at a period time

 @param [in] Event              Event
 @param [in] Context            Passed parameter to event handler

**/
VOID
EFIAPI
CheckReadKeyHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  BITLOCKER_PRIVATE             *Private;

  Private = (BITLOCKER_PRIVATE *) Context;

  if (!Private->ReadKeyFlag) {
    Private->NoReadKeyCount++;
  } else {
    Private->NoReadKeyCount = 0;
  }

  Private->ReadKeyFlag = FALSE;

  if (Private->NoReadKeyCount > 2) {
    gBS->CloseEvent (Private->CheckReadKeyEvent);
    Private->CheckReadKeyEvent = NULL;

    if (Private->DisableCheckReadKeyEvent != NULL) {
      gBS->CloseEvent (Private->DisableCheckReadKeyEvent);
      Private->DisableCheckReadKeyEvent = NULL;
    }

    Private->SetupMouseIsStart = FALSE;
    Private->SetupMouse->Close (Private->SetupMouse);
  }
}

/**

 Monitor readkeystroke function to create timer event of bitlocker.

 @param [in]  This              Protocol instance pointer of SimpleTextInputEx.
 @param [out] KeyData           EFI key data.

 @retval                        Origianl ReadKeyStrokeEx function return status

**/
EFI_STATUS
EFIAPI
BitlockerKeyboardReadKeyStrokeEx (
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  )
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_POINTER_STATE      SimplePointerState;
  EFI_ABSOLUTE_POINTER_STATE    AbsolutePointerState;
  KEYBOARD_ATTRIBUTES           KeyboardAttrs;
  BITLOCKER_PRIVATE             *Private;
  UINTN                         X;
  UINTN                         Y;
  BOOLEAN                       LeftButton;
  BOOLEAN                       RightButton;
  EFI_SETUP_MOUSE_PROTOCOL      *SetupMouse;
  BOOLEAN                       ShowVirtualKeyboard;

  Private = (BITLOCKER_PRIVATE *) mBitlocker;


  if (Private->FirstIn) {
    Private->FirstIn = FALSE;
    Private->SimplePointer->Reset (Private->SimplePointer, TRUE);
    Private->AbsolutePointer->Reset (Private->AbsolutePointer, TRUE);
  }

  ShowVirtualKeyboard = FALSE;
  if (!Private->SetupMouseIsStart) {
    Status = Private->SimplePointer->GetState(Private->SimplePointer, &SimplePointerState);
    if (!EFI_ERROR (Status)) {
      Private->SetupMouseIsStart = TRUE;
      if (SimplePointerState.LeftButton) {
        ShowVirtualKeyboard = TRUE;
      }
    }

    Status = Private->AbsolutePointer->GetState(Private->AbsolutePointer, &AbsolutePointerState);
    if (!EFI_ERROR (Status)) {
      Private->SetupMouseIsStart = TRUE;
      if ((AbsolutePointerState.ActiveButtons & EFI_ABSP_TouchActive) == EFI_ABSP_TouchActive) {
        ShowVirtualKeyboard = TRUE;
      }
    }
  }

  if (Private->SetupMouseIsStart) {
    SetupMouse = Private->SetupMouse;
    SetupMouse->Start (SetupMouse);
    SetupMouse->GetKeyboardAttributes (SetupMouse, &KeyboardAttrs);
    if (!KeyboardAttrs.IsStart) {
      LeftButton = FALSE;
      if (!ShowVirtualKeyboard) {
        Status = SetupMouse->QueryState (SetupMouse, &X, &Y, &LeftButton, &RightButton);
        if (!EFI_ERROR (Status) && LeftButton) {
          ShowVirtualKeyboard = TRUE;
        }
      }
    }

    if (ShowVirtualKeyboard) {
      if (Private->FirstDisplayKB) {
        Private->FirstDisplayKB = FALSE;
        SetupMouse->StartKeyboard (SetupMouse, (UINTN) 10000, (UINTN) 10000); // right-bottom
      } else {
        SetupMouse->StartKeyboard (SetupMouse, (UINTN) KeyboardAttrs.X, (UINTN) KeyboardAttrs.Y);
      }
    }
  }

  Private->ReadKeyFlag = TRUE;
  Status = Private->OrgReadKeyStrokeEx (This, KeyData);
  if (EFI_ERROR (Status) || !Private->SetupMouseIsStart) {
    return Status;
  }

  if (Private->DisableCheckReadKeyEvent == NULL &&
      Private->CheckReadKeyEvent == NULL &&
      (KeyData->Key.ScanCode == SCAN_ESC || KeyData->Key.UnicodeChar == CHAR_CARRIAGE_RETURN)) {

    Status = gBS->CreateEvent (
                    EVT_TIMER | EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    DisableCheckReadKeyHandler,
                    Private,
                    &Private->DisableCheckReadKeyEvent
                    );
    ASSERT_EFI_ERROR (Status);
    if (!EFI_ERROR (Status)) {
      Status = gBS->SetTimer(Private->DisableCheckReadKeyEvent, TimerRelative, 500 * TICKS_PER_MS);
    }

    Private->NoReadKeyCount = 0;
    Status = gBS->CreateEvent (
                    EVT_TIMER | EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    CheckReadKeyHandler,
                    Private,
                    &Private->CheckReadKeyEvent
                    );
    ASSERT_EFI_ERROR (Status);
    if (!EFI_ERROR (Status)) {
      Status = gBS->SetTimer(Private->CheckReadKeyEvent, TimerPeriodic, 50 * TICKS_PER_MS);
    }
  }

  return EFI_SUCCESS;
}

/**

 Disable Bitlocker handler when receive return from image event

 @param [in] Event              Event
 @param [in] Context            Passed parameter to event handler

**/
VOID
EFIAPI
DisableBitlockerHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  BITLOCKER_PRIVATE             *Private;

  Private = (BITLOCKER_PRIVATE *) Context;

  gBS->CloseEvent (Event);
  if (Private->CheckReadKeyEvent != NULL) {
    gBS->CloseEvent (Private->CheckReadKeyEvent);
    Private->CheckReadKeyEvent = NULL;
  }

  if (Private->DisableCheckReadKeyEvent != NULL) {
    gBS->CloseEvent (Private->DisableCheckReadKeyEvent);
    Private->DisableCheckReadKeyEvent = NULL;
  }

  Private->SetupMouseIsStart = FALSE;
  Private->SetupMouse->Close (Private->SetupMouse);
  Private->SimpleTextInputEx->ReadKeyStrokeEx = Private->OrgReadKeyStrokeEx;
}


/**

  Bitlocker virtual keybaord support

  @param [in] DevicePath        Device path of boot option

  @retval EFI_SUCCESS           Success
  @retval EFI_UNSUPPORTED       It isn't Windows boot option.

**/
EFI_STATUS
EFIAPI
BitlockerVirtualKeyboardSupport (
  IN EFI_DEVICE_PATH_PROTOCOL   *DevicePath
  )
{
  EFI_STATUS                    Status;
  EFI_EVENT                     Event;
  VOID                          *Registration;
  BITLOCKER_PRIVATE             *Private;

  if (!IsWindowsOS (DevicePath)) {
    return EFI_UNSUPPORTED;
  }

  if (mBitlocker == NULL) {
    mBitlocker = AllocateZeroPool (sizeof (BITLOCKER_PRIVATE));
    if (mBitlocker == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  } else {
    ZeroMem (mBitlocker, sizeof (BITLOCKER_PRIVATE));
  }

  Private = mBitlocker;

  Status = gBS->HandleProtocol (
                  gST->ConsoleOutHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &Private->GraphicsOutput
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // SimplePointer, Absolute should be install by ConSplitter
  //
  Status = gBS->HandleProtocol (
                  gST->ConsoleInHandle,
                  &gEfiSimplePointerProtocolGuid,
                  (VOID **) &Private->SimplePointer
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }


  Status = gBS->HandleProtocol (
                  gST->ConsoleInHandle,
                  &gEfiAbsolutePointerProtocolGuid,
                  (VOID **) &Private->AbsolutePointer
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (
                  &gSetupMouseProtocolGuid,
                  NULL,
                  (VOID **) &Private->SetupMouse
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->HandleProtocol (
                  gST->ConsoleInHandle,
                  &gEfiSimpleTextInputExProtocolGuid,
                  (VOID **) &Private->SimpleTextInputEx
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  DisableBitlockerHandler,
                  Private,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->RegisterProtocolNotify (
                  &gReturnFromImageGuid,
                  Event,
                  &Registration
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (Event);
    return Status;
  }

  Private->FirstIn        = TRUE;
  Private->FirstDisplayKB = TRUE;

  //
  // hook ReadKeyStrokeEx of SimpleTextInEx
  //
  Private->OrgReadKeyStrokeEx = Private->SimpleTextInputEx->ReadKeyStrokeEx;
  Private->SimpleTextInputEx->ReadKeyStrokeEx = BitlockerKeyboardReadKeyStrokeEx;

  return EFI_SUCCESS;
}

