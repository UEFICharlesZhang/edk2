/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2012 - 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "IhisiFets.h"
#include <Library/SmmChipsetSvcLib.h>

#define     EC_PART_SIZE     EC256K
UINT16      mEcActualSize = (0x40 << EC_PART_SIZE );
UINT16      mReadCount = 0;

/**
  AH=20h, Flash EC through SMI (FETS) Write.

  @retval EFI_SUCCESS    Flash EC successful.
  @return Other          Flash EC failed.
**/
EFI_STATUS
FetsWrite (
  VOID
  )
{
  EFI_STATUS          Status;
  UINTN               SizeToFlash;
  UINT8               DestBlockNo;
  UINT8               ActionAfterFlashing;
  UINT8              *FlashingDataBuffer;

  Status              = EFI_SUCCESS;
  FlashingDataBuffer  = NULL;
  SizeToFlash         = 0;
  DestBlockNo         = 0;
  ActionAfterFlashing = 0;

  //
  // Get flashing data from address stored in ESI
  //
  FlashingDataBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);

  //
  // Get the size to flash from EDI
  //
  SizeToFlash = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);

  if (BufferOverlapSmram ((VOID *) FlashingDataBuffer, SizeToFlash)) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  //
  // Get destined block number from CH
  //
  DestBlockNo = (UINT8) (IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX) >> 8);

  //
  // Get action after flashing from CL
  //
  ActionAfterFlashing = (UINT8) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);

  if ((SizeToFlash == 0) || (FlashingDataBuffer == NULL)) {
    return IHISI_FBTS_READ_FAILED;
  }

  Status = SmmCsSvcIhisiFetsDoBeforeFlashing (&FlashingDataBuffer, &SizeToFlash, &DestBlockNo);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SmmCsSvcIhisiFetsEcIdle (TRUE);

  Status = SmmCsSvcIhisiFetsEcFlash (FlashingDataBuffer, SizeToFlash, DestBlockNo);

  SmmCsSvcIhisiFetsEcIdle (FALSE);

  if (EFI_ERROR (Status)) {
    //
    // BUGBUG : Temporary use IHISI_FBTS_WRITE_FAILED to report error. ( Specification has no FETS error code definitions yet. )
    //
    return IHISI_FBTS_WRITE_FAILED;
  }

  Status = SmmCsSvcIhisiFetsDoAfterFlashing (ActionAfterFlashing);
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  switch (ActionAfterFlashing) {
    case EcFlashDosReboot:
      SmmCsSvcIhisiFetsReboot ();
      break;

    case EcFlashDoshutdown:
      SmmCsSvcIhisiFetsShutdown ();
      break;

    case EcFlashOSShutdown:
    case EcFlashOSReboot:
      break;

    case EcFlashDoNothing:
      break;

    case EcFlashContinueToFlash:
    default:
      break;
  }

  return EFI_SUCCESS;
}

/**
  AH=21h, Get EC part information.

  @retval EFI_SUCCESS     Get EC part information successful.
**/
EFI_STATUS
GetEcPartInfo (
  VOID
  )
{
  EFI_STATUS                            Status;
  UINT32                                Ecx;
  UINT32                                EcPartSize;
  FETS_EC_PART_SIZE_STRUCTURE_INPUT    *EcPartSizeTable;

  EcPartSizeTable = (FETS_EC_PART_SIZE_STRUCTURE_INPUT *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  if (BufferOverlapSmram ((VOID *) EcPartSizeTable, sizeof(FETS_EC_PART_SIZE_STRUCTURE_INPUT))) {
    if (EcPartSizeTable->Signature == EC_PARTSIZE_SIGNATURE) {
      return IHISI_INVALID_PARAMETER;
    }
  }

  EcPartSize = EC_PART_SIZE;
  Status = SmmCsSvcIhisiFetsGetEcPartInfo (&EcPartSize);

  Ecx = IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX) & 0xFFFFFF00;
  IhisiWriteCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX, Ecx|EcPartSize);

  return Status;
}

EFI_STATUS
InstallFetsServices (
  VOID
  )
{
  EFI_STATUS            Status;

  Status = IhisiRegisterCommand (FETSWrite, FetsWrite, IhisiNormalPriority);
  ASSERT_EFI_ERROR (Status);

  Status = IhisiRegisterCommand (FETSGetEcPartInfo, GetEcPartInfo, IhisiNormalPriority);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
