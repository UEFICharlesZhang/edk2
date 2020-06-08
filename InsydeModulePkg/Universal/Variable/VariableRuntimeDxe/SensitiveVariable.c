/** @file
    Provide support functions for sensitive variable.

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

#include "SensitiveVariable.h"

EFI_STATUS
EFIAPI
SmmSetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  IN UINT32        Attributes,
  IN UINTN         DataSize,
  IN VOID          *Data
  );

/**
  This fucnitons uses to set sensitive variable in SMM.

  @retval EFI_SUCCESS     Set sensitive variable successfully.
  @return Other           Set sensitive variable failed.

**/
EFI_STATUS
SmmSetSensitiveVariable (
  VOID
  )
{
  UINT32                              BufferSize;
  SENSITIVE_VARIABLE_AUTHENTICATION   *VariableBuffer;
  SENSITIVE_VARIABLE_STORE_HEADER     *SensitiveStoreHeader;
  SENSITIVE_VARIABLE_HEADER           *SensitiveVariable;
  UINT16                              VariableIndex;
  EFI_STATUS                          Status;


  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RBX ,&BufferSize);
  VariableBuffer = NULL;
  GetDwordRegister (EFI_SMM_SAVE_STATE_REGISTER_RSI ,(UINT32 *) &VariableBuffer);

  if (VariableBuffer->Signature != SET_SENSITIVE_VARIABLE_SIGNATURE || VariableBuffer->DataSize != BufferSize) {
    return EFI_UNSUPPORTED;
  }

  VariableBuffer->Status = EFI_SUCCESS;
  SensitiveStoreHeader   = (SENSITIVE_VARIABLE_STORE_HEADER *)(VariableBuffer + 1);
  VariableIndex          = 0;
  SensitiveVariable      = GetNextSensitiveVariable (SensitiveStoreHeader, NULL, &VariableIndex);
  while (SensitiveVariable != NULL) {
    if ((SensitiveVariable->Attributes & (EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS | EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)) != 0) {
      VariableBuffer->Status = EFI_ACCESS_DENIED;
      break;
    }
    Status = SmmSetVariable (
               (CHAR16 *) (SensitiveVariable + 1),
               &SensitiveVariable->VendorGuid,
               SensitiveVariable->Attributes,
               SensitiveVariable->DataSize,
               GetSensitiveVariableDataPtr (SensitiveVariable)
               );
    if (EFI_ERROR (Status)) {
      VariableBuffer->Status = Status;
      ASSERT (FALSE);
      break;
    }
    SensitiveVariable = GetNextSensitiveVariable (SensitiveStoreHeader, SensitiveVariable, &VariableIndex);
  }

  return VariableBuffer->Status;
}

