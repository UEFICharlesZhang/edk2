/** @file
  IHISI Variable Services Implementation

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
#include "IhisiVats.h"

/**
  Get IHISI status.translated from EFI status

  @param[in] Status  EFI_STATUS

  @return UINT32     IHISI status
**/
UINT32
GetVatsIhisiStatus (
  IN EFI_STATUS                             Status
  )
{
  switch (Status) {
    case EFI_SUCCESS:
      return (UINT32) IHISI_SUCCESS;
      break;

    case EFI_BUFFER_TOO_SMALL:
      return (UINT32) IHISI_OB_LEN_TOO_SMALL;
      break;

    case EFI_NOT_FOUND:
      return (UINT32) IHISI_VATS_VARIABLE_NOT_FOUND;
      break;

    case EFI_INVALID_PARAMETER:
      return (UINT32) IHISI_VATS_WRONG_OB_FORMAT;
      break;

    default:
      return (UINT32) IHISI_VATS_VARIABLE_ACCESS_ERROR;
      break;
  }
}

/**
  IHISI checksum calulation.

  @param[in] Ptr       A pointer to to a buffer.
  @param[in] Size      The buffer size.
  @param[in] Checksum  An element in IHISI output table.

  @retval TURE         Ckecksum error.
  @return FALSE        Ckecksum correct.
**/
BOOLEAN
IhisiChecksumError (
  IN VOID                                *Ptr,
  IN UINTN                               Size,
  IN UINT16                              Checksum
  )
{
  UINTN      Index;
  UINT8      *TempPtr;
  UINT32     Sum;

  TempPtr = Ptr;
  Sum = 0;

  for (Index = 0 ; Index < Size ; Index++) {
    Sum = Sum + TempPtr[Index];
  }

  Sum = (UINT32) ((Checksum + Sum) & 0x0000ffff);

  return (BOOLEAN) Sum;
}

/**
  Vats security check.

  @return UINT32       IHISI status
**/
STATIC
EFI_STATUS
VatsSecurityCheckError (
  VOID
  )
{
  IHISI_VATS_OUTPUT_BUFFER              *OutBuffer;

  OutBuffer = (IHISI_VATS_OUTPUT_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  if ((OutBuffer->TableRev) != IHISI_TABLE_REVISION) {
    return IHISI_VATS_OB_TABLE_REV_UNSUPPORTED;
  }

  if (((OutBuffer->TableId1) != IHISI_H2O_SIGNATURE) ||
      ((OutBuffer->TableId2) != IHISI_VAR_SIGNATURE) ||
      ((OutBuffer->TableId3) != IHISI_TBL_SIGNATURE)) {
    return IHISI_VATS_WRONG_OB_FORMAT;
  }
  return EFI_SUCCESS;
}

/**
  AH=00h, Read the specific variable into the specified buffer.

  @retval EFI_SUCCESS        AL 00h = Function succeeded.
  @return Others             AL Returned error code.
**/
EFI_STATUS
VatsRead (
  VOID
  )
{
  IHISI_VATS_INPUT_BUFFER                *InBuffer;
  IHISI_VATS_OUTPUT_BUFFER               *OutBuffer;
  UINT8                                  *VarData;
  CHAR16                                 *VarName;
  EFI_STATUS                             Status;
  UINTN                                  VarLength;

  //
  // Check the valid signature "H2O$Var$Tbl" in obTableId filed.
  //
  Status = VatsSecurityCheckError ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // DS:ESI  Pointer to the beginning of the VATS input buffer.
  // DS:EDI  Pointer to the beginning of the VATS output buffer.
  //
  InBuffer  = (IHISI_VATS_INPUT_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  OutBuffer = (IHISI_VATS_OUTPUT_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);

  VarName   = (CHAR16 *) ((UINTN) InBuffer + (sizeof (IHISI_VATS_INPUT_BUFFER) / sizeof (UINT8)));
  VarData   = (UINT8 *) ((UINTN) OutBuffer + (sizeof (IHISI_VATS_OUTPUT_BUFFER) / sizeof (UINT8)));
  VarLength = (UINTN) OutBuffer->VarLength;
  if (BufferOverlapSmram ((VOID *) VarName, StrSize (VarName)) ||
      BufferOverlapSmram ((VOID *) VarData, VarLength)) {
    return  IHISI_VATS_WRONG_OB_FORMAT;
  }

  Status = EFI_UNSUPPORTED;
  if (mSmmVariable != NULL) {
    Status = mSmmVariable->SmmGetVariable (
                                           VarName,
                                           &InBuffer->VarGuid,
                                           NULL,
                                           &VarLength,
                                           VarData
                                           );
  }

  if (Status == EFI_SUCCESS || Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Status is EFI_BUFFER_TOO_SMALL also need return required data size to application
    //
    OutBuffer->VarLength = (UINT32) VarLength;
  }

  return Status;
}

/**
  AH=01h, Write the specified buffer to the specific variable.

  @retval EFI_SUCCESS        AL	00h = Function succeeded.
  @return Others             AL	Returned error code.
**/
EFI_STATUS
VatsWrite (
  VOID
  )
{
  IHISI_VATS_INPUT_BUFFER                *InBuffer;
  IHISI_VATS_OUTPUT_BUFFER               *OutBuffer;
  UINT8                                  *VarData;
  CHAR16                                 *VarName;
  EFI_STATUS                             Status;
  //
  // Check the valid signature "H2O$Var$Tbl" in obTableId filed.
  //
  Status = VatsSecurityCheckError ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // DS:ESI  Pointer to the beginning of the VATS input buffer.
  // DS:EDI  Pointer to the beginning of the VATS output buffer.
  //
  InBuffer = (IHISI_VATS_INPUT_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  OutBuffer = (IHISI_VATS_OUTPUT_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);

  VarName = (CHAR16 *) ((UINTN) InBuffer + (sizeof (IHISI_VATS_INPUT_BUFFER) / sizeof (UINT8)));
  VarData = (UINT8 *) ((UINTN) OutBuffer + (sizeof (IHISI_VATS_OUTPUT_BUFFER) / sizeof (UINT8)));

  if (BufferOverlapSmram ((VOID *) VarName, StrSize (VarName)) ||
      BufferOverlapSmram ((VOID *) VarData, OutBuffer->VarLength)) {
    return  IHISI_VATS_WRONG_OB_FORMAT;
  }

  if (IhisiChecksumError (VarData, OutBuffer->VarLength, OutBuffer->VarChecksum)) {
    return  IHISI_VATS_OB_CHECKSUM_FAILED;
  }

  if (OutBuffer->Attribute == 0) {
    OutBuffer->Attribute = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
  }

  Status = IHISI_VATS_VARIABLE_ACCESS_ERROR;
  if (mSmmVariable != NULL) {
    Status = mSmmVariable->SmmSetVariable (
                                           VarName,
                                           &InBuffer->VarGuid,
                                           OutBuffer->Attribute,
                                           (UINTN) OutBuffer->VarLength,
                                           VarData
                                           );
    if (EFI_ERROR (Status)) {
      Status = IHISI_VATS_VARIABLE_ACCESS_ERROR;
    }
  }

  return Status;
}

EFI_STATUS
InstallVatsServices (
  VOID
  )
{
  EFI_STATUS            Status;
  Status = IhisiRegisterCommand (VATSRead, VatsRead, IhisiNormalPriority);
  ASSERT_EFI_ERROR (Status);

  Status = IhisiRegisterCommand (VATSWrite, VatsWrite, IhisiNormalPriority);
  ASSERT_EFI_ERROR (Status);
  return EFI_SUCCESS;
}

