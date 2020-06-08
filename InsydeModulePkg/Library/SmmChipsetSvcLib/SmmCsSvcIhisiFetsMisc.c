/** @file
 SMM Chipset Services Library.

 This file contains Ihisi Fets Chipset service Lib function.

***************************************************************************
* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#include <Uefi.h>
#include <H2OIhisi.h>
#include <Library/SmmChipsetSvcLib.h>
#include <Library/DebugLib.h>
#include <Protocol/H2OSmmChipsetServices.h>

extern H2O_SMM_CHIPSET_SERVICES_PROTOCOL *mSmmChipsetSvc;

/**
  AH=20h(FetsWrite), Hook function before flashing EC part.

  @param[in, out] FlashingDataBuffer Double pointer to data buffer.
  @param[in, out] SizeToFlash        Data size by bytes want to flash.
  @param[in, out] DestBlockNo        Dsstination block number.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsDoBeforeFlashing (
  IN OUT  UINT8        **FlashingDataBuffer,
  IN OUT  UINTN         *SizeToFlash,
  IN OUT  UINT8         *DestBlockNo
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFetsDoBeforeFlash) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFetsDoBeforeFlash == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFetsDoBeforeFlash() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFetsDoBeforeFlash (FlashingDataBuffer, SizeToFlash, DestBlockNo);
}

/**
   AH=20h(FetsWrite), Hook function uses to flash EC part.

  @param[in, out] FlashingDataBuffer Pointer to input file data buffer
  @param[in, out] SizeToFlash        Data size.
  @param[in, out] DestBlockNo        Dsstination block number.

  @retval EFI_SUCCESS       Successfully returns.
  @return Other             Error occurred in this function.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsEcFlash (
  IN  UINT8            *FlashingDataBuffer,
  IN  UINTN             SizeToFlash,
  IN  UINT8             DestBlockNo
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFetsEcFlash) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFetsEcFlash == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFetsEcFlash() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFetsEcFlash (FlashingDataBuffer, SizeToFlash, DestBlockNo);
}

/**
  AH=20h(FetsWrite), Hook function after flashing EC part.

  @param[in] ActionAfterFlashing    Input action flag.

  @retval EFI_UNSUPPORTED       Returns unsupported by default.
  @retval EFI_SUCCESS           The service is customized in the project.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsDoAfterFlashing (
  IN  UINT8             ActionAfterFlashing
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFetsDoAfterFlash) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFetsDoAfterFlash == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFetsDoAfterFlash() isn't implement!\n"));
    return EFI_UNSUPPORTED;
  }

  return mSmmChipsetSvc->IhisiFetsDoAfterFlash (ActionAfterFlashing);
}

/**
  Fets reset system function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsReboot (
  VOID
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFetsReboot) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFetsReboot == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFetsReboot() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFetsReboot ();
}

/**
  Fets shutdown function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsShutdown (
  VOID
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFetsShutdown) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFetsShutdown == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFetsShutdown() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFetsShutdown ();
}

/**
  Fets EC idle function

  @param[in] Idle     Ec idle mode.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsEcIdle (
  IN  BOOLEAN             Idle
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFetsEcIdle) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFetsEcIdle == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFetsEcIdle() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFetsEcIdle (Idle);
}

/**
  AH=21h(GetEcPartInfo), Get EC part information.

  @param[in, out] EcPartSize        EC part size

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsGetEcPartInfo (
  IN OUT UINT32          *EcPartSize
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFetsGetEcPartInfo) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFetsGetEcPartInfo == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFetsGetEcPartInfo() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFetsGetEcPartInfo (EcPartSize);
}

