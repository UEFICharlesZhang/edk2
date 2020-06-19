/** @file
  Provides application point extension for "C" style main funciton

  Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <FlashUpdate.h>

/**
  UEFI entry point for an application that will in turn call the
  ShellAppMain function which has parameters similar to a standard C
  main function.

  An application that uses UefiShellCEntryLib must have a ShellAppMain
  function as prototyped in Include/Library/ShellCEntryLib.h.

  @param  ImageHandle  The image handle of the UEFI Application.
  @param  SystemTable  A pointer to the EFI System Table.

  @retval  EFI_SUCCESS               The application exited normally.
  @retval  Other                     An error occurred.

**/
EFI_STATUS
EFIAPI
EntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_SHELL_PARAMETERS_PROTOCOL *EfiShellParametersProtocol;
  EFI_SHELL_INTERFACE           *EfiShellInterface;
  CHAR16                        *Argv[] = {L"ByoShellFlash64.efi",
                                           L"-a",
                                           L"FT1500A_UEFI_BIOS.fd"};
  EFI_STATUS                    Status;

  EfiShellParametersProtocol = NULL;
  EfiShellInterface = NULL;

  Status = SystemTable->BootServices->OpenProtocol(ImageHandle,
                             &gEfiShellParametersProtocolGuid,
                             (VOID **)&EfiShellParametersProtocol,
                             ImageHandle,
                             NULL,
                             EFI_OPEN_PROTOCOL_GET_PROTOCOL
                            );
  if (!EFI_ERROR(Status)) {
    //
    // use shell 2.0 interface
    //
    Status = ShellAppMain (
               EfiShellParametersProtocol->Argc,
               EfiShellParametersProtocol->Argv
               );
  } else {
    //
    // try to get shell 1.0 interface instead.
    //
    Status = SystemTable->BootServices->OpenProtocol(ImageHandle,
                               &gEfiShellInterfaceGuid,
                               (VOID **)&EfiShellInterface,
                               ImageHandle,
                               NULL,
                               EFI_OPEN_PROTOCOL_GET_PROTOCOL
                              );
    if (!EFI_ERROR(Status)) {
      //
      // use shell 1.0 interface
      //
      Status = ShellAppMain (
                 EfiShellInterface->Argc,
                 EfiShellInterface->Argv
                 );
    } else {
      // Update BIOS with fixed file name
      Status = ShellAppMain (3, Argv);
    }
  }

  return Status;
}
