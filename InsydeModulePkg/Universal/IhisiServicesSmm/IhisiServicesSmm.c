/** @file
  SMM driver implementation for the IHISI Services

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "IhisiServicesSmm.h"
/**
  IhisiSmm module entry point

  @param ImageHandle            A handle for the image that is initializing this driver
  @param SystemTable            A pointer to the EFI system table

  @retval EFI_SUCCESS:          Module initialized successfully
  @retval Others                Module initialized unsuccessfully
**/
EFI_STATUS
EFIAPI
IhisiServicesSmmEntry(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS    Status;
  if (!InSmm ()) {
    return EFI_SUCCESS;
  }

  Status = InitIhisi();
  ASSERT_EFI_ERROR (Status);

  Status = InstallVatsServices ();
  ASSERT_EFI_ERROR (Status);

  Status = InstallFbtsServices ();
  ASSERT_EFI_ERROR (Status);

  Status = InstallFetsServices ();
  ASSERT_EFI_ERROR (Status);

  Status = InstallCommonRegionCommunicationServices ();
  ASSERT_EFI_ERROR (Status);

  Status = InstallOemExtraDataCommunicationServices ();
  ASSERT_EFI_ERROR (Status);

  Status = InstallSecureFlashServices();
  ASSERT_EFI_ERROR (Status);

  Status = InstallIhisiProtocol ();
  ASSERT_EFI_ERROR (Status);

  return Status;
}
