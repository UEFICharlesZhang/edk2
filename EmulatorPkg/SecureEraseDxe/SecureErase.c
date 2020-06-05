// ## @file
// #
// #******************************************************************************
// #* Copyright (c) 2018, Insyde Software Corp. All Rights Reserved.
// #*
// #* You may not reproduce, distribute, publish, display, perform, modify, adapt,
// #* transmit, broadcast, present, recite, release, license or otherwise exploit
// #* any part of this publication in any form, by any means, without the prior
// #* written permission of Insyde Software Corporation.
// #*
// #******************************************************************************
/**@file
Secure Erase Implementation.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2019 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

#include "SecureErase.h"
#include "Ui.h"
#include "AtaSecureErase.h"
#include "NvmeSecureErase.h"
#include "SimulatedSecureErase.h"
#include <Library/PcdLib.h>


/**
  Attempts to erase ATA and NVMe drives.

  @retval EFI_SUCCESS         Erase device succeed
  @retval EFI_UNSUPPORTED     The device is not supported
  @retval EFI_ACCESS_DENIED   User has entered wrong password too many times
  @retval EFI_ABORTED         The device is supported, but the system
                              has failed to erase it
**/
EFI_STATUS
EraseDevices (
  VOID
  )
{
  EFI_STATUS                           Status;
  EFI_TPL                              OldTpl;

  DEBUG ((DEBUG_INFO, "SecureErase :: EraseDevice\n"));
  OldTpl = EfiGetCurrentTpl ();

  gBS->RestoreTPL (TPL_APPLICATION);
  InitUiLib ();
  SetScreenTitle (L"Secure erase");
  ClearScreen ();

  if (1) {
    DEBUG ((DEBUG_INFO, "SecureErase in progress\n"));
    Status = EraseAtaDevice ();
    DEBUG ((DEBUG_INFO, "AtaStatus = %r\n", Status));
    if (!EFI_ERROR (Status)) {
      Status = EraseNvmeDevice ();
      DEBUG ((DEBUG_INFO, "NvmeStatus = %r\n", Status));
    }
  } else {
    Status = SimulateSecureErase ();
  }

  if (0) {
    SimulateSecureEraseDone ();
    //
    // If demo mode is enabled, don't restart. Use dead loop to make sure screen keeps
    // displaying "missing operating system" message from SimulatedSecureErase.c forever.
    //
    CpuDeadLoop ();
  }

  PrintSummary (Status);

  gBS->RaiseTPL (OldTpl);
  return Status;
}

/**
  Drivers entry point. Checks if secure erase has been requested via boot options,
  if so, starts the procedure.

  @param[in] ImageHandle      The firmware allocated handle for the EFI image.
  @param[in] SystemTable      A pointer to the EFI System Table.

  @retval EFI_SUCCESS         Secure erase procedure has been started
  @retval EFI_ABORTED         Failed to register ready to boot event
  @retval EFI_UNSUPPORTED     Feature is not supported by the FW
**/
EFI_STATUS
EFIAPI
SecureEraseEntryPoint (
  IN EFI_HANDLE                        ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  )
{
  EFI_STATUS                           Status;
  DEBUG ((DEBUG_INFO, "SecureErase :: SecureEraseEntryPoint\n"));

  Status = EraseDevices ();

  return EFI_SUCCESS;
}
