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
#include <Protocol/DriverBinding.h>

typedef struct _HOOK_DRIVER_BINDING_ENTRY {
  UINT32                        Signature;
  LIST_ENTRY                    Link;
  EFI_DRIVER_BINDING_PROTOCOL   *DriverBinding;
  EFI_DRIVER_BINDING_SUPPORTED  Supported;
  EFI_DRIVER_BINDING_START      Start;
  EFI_DRIVER_BINDING_STOP       Stop;
} HOOK_DRIVER_BINDING_ENTRY;

#define HOOK_DRIVER_BINDING_ENTRY_SIGNATURE    SIGNATURE_32 ('h', 'd', 'b', 'e')
#define HOOK_DRIVER_BINDING_ENTYRY_FROM_LINK(_link)  CR (_link, HOOK_DRIVER_BINDING_ENTRY, Link, HOOK_DRIVER_BINDING_ENTRY_SIGNATURE)

EFI_STATUS
EFIAPI
HookDriverBindingLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

EFI_DRIVER_BINDING_PROTOCOL *
EFIAPI
LocateDriverBindingByProtocolGuid (
  IN EFI_HANDLE                 ControllerHandle,
  IN EFI_GUID                   *ProtocolGuid
  );

HOOK_DRIVER_BINDING_ENTRY *
EFIAPI
FindHookDriverBindingEntry (
  EFI_DRIVER_BINDING_PROTOCOL   *DriverBinding
  );

EFI_STATUS
EFIAPI
HookDriverBinding (
  IN EFI_DRIVER_BINDING_PROTOCOL   *DriverBinding,
  IN EFI_DRIVER_BINDING_SUPPORTED  DriverBindingSupported OPTIONAL,
  IN EFI_DRIVER_BINDING_START      DriverBindingStart OPTIONAL,
  IN EFI_DRIVER_BINDING_STOP       DriverBindingStop OPTIONAL,
  OUT HOOK_DRIVER_BINDING_ENTRY    **Entry
  );

EFI_STATUS
EFIAPI
HookDriverBindingByDriverName (
  IN CHAR16                        *DriverName,
  IN EFI_DRIVER_BINDING_SUPPORTED  DriverBindingSupported OPTIONAL,
  IN EFI_DRIVER_BINDING_START      DriverBindingStart OPTIONAL,
  IN EFI_DRIVER_BINDING_STOP       DriverBindingStop OPTIONAL,
  OUT HOOK_DRIVER_BINDING_ENTRY    **Entry
  );

