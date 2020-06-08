/** @file
  Hook Lib

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

#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/SimpleTextInEx.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/AbsolutePointer.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName2.h>
#include "HookLib.h"

LIST_ENTRY mHookDriverBindingList;

EFI_STATUS
EFIAPI
HookDriverBindingLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  InitializeListHead (&mHookDriverBindingList);

  return EFI_SUCCESS;
}

EFI_DRIVER_BINDING_PROTOCOL *
EFIAPI
LocateDriverBindingByProtocolGuid (
  IN EFI_HANDLE                 ControllerHandle,
  IN EFI_GUID                   *ProtocolGuid
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            AgentHandle;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY   *OpenInfoBuffer;
  UINTN                                 EntryCount;
  UINTN                                 Index;
  EFI_DRIVER_BINDING_PROTOCOL           *DriverBinding;

  ASSERT (ProtocolGuid != NULL);

  Status = gBS->OpenProtocolInformation (
                  ControllerHandle,
                  (EFI_GUID *) ProtocolGuid,
                  &OpenInfoBuffer,
                  &EntryCount
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  AgentHandle = NULL;
  for (Index = 0; Index < EntryCount; Index++) {
    if ((OpenInfoBuffer[Index].ControllerHandle == ControllerHandle) &&
        (OpenInfoBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) != 0) {
      AgentHandle = OpenInfoBuffer[Index].AgentHandle;
      break;
    }
  }

  FreePool (OpenInfoBuffer);
  if (AgentHandle == NULL) {
    return NULL;
  }

  //
  // locate driver binding protocol
  //
  Status = gBS->HandleProtocol (
                  AgentHandle,
                  &gEfiDriverBindingProtocolGuid,
                  (VOID **) &DriverBinding
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return DriverBinding;
}

HOOK_DRIVER_BINDING_ENTRY *
EFIAPI
FindHookDriverBindingEntry (
  EFI_DRIVER_BINDING_PROTOCOL   *DriverBinding
  )
{
  HOOK_DRIVER_BINDING_ENTRY     *Entry;
  LIST_ENTRY                    *Link;

  for (Link = GetFirstNode (&mHookDriverBindingList);
       !IsNull (&mHookDriverBindingList, Link);
       Link = GetNextNode (&mHookDriverBindingList, Link)) {

    Entry = HOOK_DRIVER_BINDING_ENTYRY_FROM_LINK (Link);
    if (Entry->DriverBinding == DriverBinding) {
      return Entry;
    }
  }

  return NULL;
}

EFI_STATUS
EFIAPI
HookDriverBinding (
  IN EFI_DRIVER_BINDING_PROTOCOL   *DriverBinding,
  IN EFI_DRIVER_BINDING_SUPPORTED  DriverBindingSupported OPTIONAL,
  IN EFI_DRIVER_BINDING_START      DriverBindingStart OPTIONAL,
  IN EFI_DRIVER_BINDING_STOP       DriverBindingStop OPTIONAL,
  OUT HOOK_DRIVER_BINDING_ENTRY    **Entry
  )
{
  HOOK_DRIVER_BINDING_ENTRY     *HookEntry;

  ASSERT (Entry != NULL);

  HookEntry = FindHookDriverBindingEntry (DriverBinding);
  if (HookEntry != NULL) {
    return EFI_ALREADY_STARTED;
  }

  HookEntry = AllocateZeroPool (sizeof (HOOK_DRIVER_BINDING_ENTRY));
  if (HookEntry == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  HookEntry->Signature     = HOOK_DRIVER_BINDING_ENTRY_SIGNATURE;
  HookEntry->DriverBinding = DriverBinding;

  HookEntry->Supported     = DriverBinding->Supported;
  HookEntry->Start         = DriverBinding->Start;
  HookEntry->Stop          = DriverBinding->Stop;

  if (DriverBindingSupported != NULL) {
    DriverBinding->Supported = DriverBindingSupported;
  }
  if (DriverBindingStart != NULL) {
    DriverBinding->Start = DriverBindingStart;
  }
  if (DriverBindingStop != NULL) {
    DriverBinding->Stop = DriverBindingStop;
  }

  InsertTailList (&mHookDriverBindingList, &HookEntry->Link);
  *Entry = HookEntry;

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
HookDriverBindingByDriverName (
  IN CHAR16                        *DriverName,
  IN EFI_DRIVER_BINDING_SUPPORTED  DriverBindingSupported OPTIONAL,
  IN EFI_DRIVER_BINDING_START      DriverBindingStart OPTIONAL,
  IN EFI_DRIVER_BINDING_STOP       DriverBindingStop OPTIONAL,
  OUT HOOK_DRIVER_BINDING_ENTRY    **Entry
  )
{
  EFI_STATUS                    Status;
  UINTN                         HandleCount;
  EFI_HANDLE                    *TargetHandle;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         Index;
  EFI_COMPONENT_NAME2_PROTOCOL  *ComponentName2;
  CHAR16                        *NameToReturn;
  EFI_DRIVER_BINDING_PROTOCOL   *DriverBinding;
  CHAR8                         *Lang;

  ASSERT (Entry != NULL);

  HandleCount = 0;
  TargetHandle = NULL;
  HandleBuffer = NULL;
  ComponentName2 = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiComponentName2ProtocolGuid,
                   NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiComponentName2ProtocolGuid,
                    (VOID **) &ComponentName2
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Lang = GetBestLanguage (ComponentName2->SupportedLanguages, FALSE, ComponentName2->SupportedLanguages, NULL);
    Status = ComponentName2->GetDriverName (
                               ComponentName2,
                               Lang,
                               &NameToReturn
                               );
    if (!EFI_ERROR (Status) && StrCmp (NameToReturn, DriverName) == 0) {
      TargetHandle = HandleBuffer[Index];
      if (Lang != NULL) {
        FreePool (Lang);
      }
      break;
    }
    if (Lang != NULL) {
      FreePool (Lang);
    }
  }

  FreePool (HandleBuffer);

  if (TargetHandle == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = gBS->HandleProtocol (
                  TargetHandle,
                  &gEfiDriverBindingProtocolGuid,
                  (VOID **) &DriverBinding
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HookDriverBinding (
             DriverBinding,
             DriverBindingSupported,
             DriverBindingStart,
             DriverBindingStop,
             Entry
             );

  return Status;

}
