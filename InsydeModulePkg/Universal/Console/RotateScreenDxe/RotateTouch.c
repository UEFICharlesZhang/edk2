/** @file
  RotateScreen

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
#include <Uefi.h>
#include <Guid/DebugMask.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/AbsolutePointer.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/UsbIo.h>
#include <Protocol/I2cIo.h>
#include "HookLib.h"

typedef struct _ROTATE_TOUCH_ENTRY {
  UINT32                           Signature;
  LIST_ENTRY                       Link;

  EFI_HANDLE                       Handle;
  EFI_ABSOLUTE_POINTER_PROTOCOL    *AbsolutePointer;
  EFI_ABSOLUTE_POINTER_GET_STATE   GetState;
  EFI_ABSOLUTE_POINTER_MODE        Mode;
} ROTATE_TOUCH_ENTRY;

#define ROTATE_TOUCH_ENTRY_SIGNATURE    SIGNATURE_32 ('r', 's', 't', 'e')
#define ROTATE_TOUCH_ENTRY_FROM_LINK(_link)  CR (_link, ROTATE_TOUCH_ENTRY, Link, ROTATE_TOUCH_ENTRY_SIGNATURE)

LIST_ENTRY                      mTouchEntryList;
VOID                            *mTouchRegistration;

BOOLEAN
EFIAPI
IsRotateScreen (
  VOID
  );

ROTATE_TOUCH_ENTRY *
FindTouchEntryByHandle (
  IN  EFI_HANDLE                Handle
  )
{
  ROTATE_TOUCH_ENTRY            *Entry;
  LIST_ENTRY                    *Link;

  for (Link = GetFirstNode (&mTouchEntryList);
       !IsNull (&mTouchEntryList, Link);
       Link = GetNextNode (&mTouchEntryList, Link)) {

    Entry = ROTATE_TOUCH_ENTRY_FROM_LINK (Link);
    if (Entry->Handle == Handle) {
      return Entry;
    }
  }

  return NULL;
}


ROTATE_TOUCH_ENTRY *
FindTouchEntry (
  IN  EFI_ABSOLUTE_POINTER_PROTOCOL *AbsolutePointer
  )
{
  ROTATE_TOUCH_ENTRY            *Entry;
  LIST_ENTRY                    *Link;

  for (Link = GetFirstNode (&mTouchEntryList);
       !IsNull (&mTouchEntryList, Link);
       Link = GetNextNode (&mTouchEntryList, Link)) {

    Entry = ROTATE_TOUCH_ENTRY_FROM_LINK (Link);
    if (Entry->AbsolutePointer == AbsolutePointer) {
      return Entry;
    }
  }

  return NULL;
}


EFI_STATUS
EFIAPI
RotateTouchGetState (
  IN EFI_ABSOLUTE_POINTER_PROTOCOL   *This,
  IN OUT EFI_ABSOLUTE_POINTER_STATE  *State
  )
{
  EFI_STATUS                    Status;
  ROTATE_TOUCH_ENTRY            *Entry;
  UINT64                        TmpValue;

  Entry = FindTouchEntry (This);
  ASSERT (Entry != NULL);
  if (Entry == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = Entry->GetState (This, State);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // flip x, y axis
  //
  if (IsRotateScreen ()) {
    This->Mode->AbsoluteMinX = Entry->Mode.AbsoluteMinY;
    This->Mode->AbsoluteMinY = Entry->Mode.AbsoluteMinX;

    This->Mode->AbsoluteMaxX = Entry->Mode.AbsoluteMaxY;
    This->Mode->AbsoluteMaxY = Entry->Mode.AbsoluteMaxX;
//BLD_Quber_029	+S
    if (PcdGetBool(PcdH2ORotateScreenIs90DegreeClockwise))  {
      TmpValue = State->CurrentY;
      State->CurrentY = State->CurrentX;
      State->CurrentX = This->Mode->AbsoluteMaxX - TmpValue;
    } else {
//BLD_Quber_029	+E
    TmpValue = State->CurrentX;
    State->CurrentX = State->CurrentY;
    State->CurrentY = This->Mode->AbsoluteMaxY - TmpValue;
    }//BLD_Quber_029+

  } else {
    This->Mode->AbsoluteMinX = Entry->Mode.AbsoluteMinX;
    This->Mode->AbsoluteMinY = Entry->Mode.AbsoluteMinY;

    This->Mode->AbsoluteMaxX = Entry->Mode.AbsoluteMaxX;
    This->Mode->AbsoluteMaxY = Entry->Mode.AbsoluteMaxY;
  }

  return Status;
}

EFI_STATUS
EFIAPI
RotateTouchAbsolutePointerDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL           *This,
  IN EFI_HANDLE                            ControllerHandle,
  IN UINTN                                 NumberOfChildren,
  IN EFI_HANDLE                            *ChildHandleBuffer
  )
{
  EFI_STATUS                    Status;
  ROTATE_TOUCH_ENTRY            *Entry;
  HOOK_DRIVER_BINDING_ENTRY     *HookEntry;

  HookEntry = FindHookDriverBindingEntry (This);
  ASSERT (HookEntry != NULL);
  if (HookEntry == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Entry = FindTouchEntryByHandle (ControllerHandle);
  if (Entry != NULL) {
    RemoveEntryList (&Entry->Link);
    FreePool (Entry);
  }

  Status = HookEntry->Stop (This, ControllerHandle, NumberOfChildren, ChildHandleBuffer);

  return Status;
}

EFI_STATUS
RotateTouchHookTouch (
  IN EFI_HANDLE                    TouchHandle,
  IN EFI_ABSOLUTE_POINTER_PROTOCOL *AbsolutePointer
  )
{
  EFI_STATUS                    Status;
  HOOK_DRIVER_BINDING_ENTRY     *HookEntry;
  EFI_DRIVER_BINDING_PROTOCOL   *DriverBinding;
  ROTATE_TOUCH_ENTRY            *RotateTouch;

  DriverBinding = LocateDriverBindingByProtocolGuid (
                    TouchHandle,
                    &gEfiUsbIoProtocolGuid
                    );
  if (DriverBinding == NULL) {
    DriverBinding = LocateDriverBindingByProtocolGuid (
                      TouchHandle,
                      &gEfiI2cIoProtocolGuid
                      );
  }

  if (DriverBinding != NULL) {
    //
    // Former I2CTouchDxe isn't Driver Model driver,
    // so we don't care hook return status
    //
    Status = HookDriverBinding (
               DriverBinding,
               NULL,
               NULL,
               RotateTouchAbsolutePointerDriverBindingStop,
               &HookEntry
               );
  }

  //
  // hook touch information
  //
  RotateTouch = AllocateZeroPool (sizeof (ROTATE_TOUCH_ENTRY));
  if (RotateTouch == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  RotateTouch->Signature = ROTATE_TOUCH_ENTRY_SIGNATURE;
  RotateTouch->AbsolutePointer = AbsolutePointer;
  RotateTouch->GetState        = AbsolutePointer->GetState;
  CopyMem (&RotateTouch->Mode, AbsolutePointer->Mode, sizeof (EFI_ABSOLUTE_POINTER_MODE));

  AbsolutePointer->GetState = RotateTouchGetState;

  InsertTailList (&mTouchEntryList, &RotateTouch->Link);

  return EFI_SUCCESS;
}

VOID
RotateTouchCallback (
  IN EFI_EVENT                  Event,
  IN VOID                       *Content
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    Handle;
  UINTN                         BufferSize;
  VOID                          *DevicePath;
  EFI_ABSOLUTE_POINTER_PROTOCOL *AbsolutePointer;

  while (TRUE) {
    BufferSize = sizeof (EFI_HANDLE);
    Status = gBS->LocateHandle (
                    ByRegisterNotify,
                    NULL,
                    mTouchRegistration,
                    &BufferSize,
                    &Handle
                    );
    if (EFI_ERROR (Status)) {
      break;
    }

    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiDevicePathProtocolGuid,
                    (VOID **) &DevicePath
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiAbsolutePointerProtocolGuid,
                    (VOID **) &AbsolutePointer
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }
    RotateTouchHookTouch (Handle, AbsolutePointer);
  }
}


/*++

Routine Description:
  Install Driver to produce rotate screen

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:

  EFI_SUCCESS - RotateScreen init success

  Other       - No protocol installed, unload driver.

--*/
EFI_STATUS
EFIAPI
RegisterRotateTouchCallback (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_EVENT                     Event;

  InitializeListHead (&mTouchEntryList);

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  RotateTouchCallback,
                  NULL,
                  &Event
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  //
  // Register for protocol notifications on this event
  //
  mTouchRegistration = NULL;
  Status = gBS->RegisterProtocolNotify (
                  &gEfiAbsolutePointerProtocolGuid,
                  Event,
                  &mTouchRegistration
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (Event);
  }

  return Status;
}

