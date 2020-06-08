/** @file
  This driver provides IHISI interface in SMM mode

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

#ifndef _IHISI_COMMON_REGION_COMMUNICATION_H_
#define _IHISI_COMMON_REGION_COMMUNICATION_H_

#include "IhisiRegistration.h"

#define FACTORY_COPY_HEADER_SIZE                             0x08

typedef
EFI_STATUS
(EFIAPI *COMMON_REGION_COMMUNICATION_FUNCTION) (
  VOID
  );

typedef struct {
  BOOLEAN      WantToReadData;
  BOOLEAN      InCommonReadProcess;
  UINTN        BlockSize;
  UINTN        DataSize;
  UINTN        RemainDataSize;
} COMMON_REGION_READ_STRUCTURE;

typedef struct {
  UINT8                                     DataType;
  COMMON_REGION_COMMUNICATION_FUNCTION      Function;
} COMMON_REGION_COMMUNICATION_FUNCTION_TABLE;

STATIC
EFI_STATUS
WriteDefaultSettingsToFactoryCopy (
  VOID
  );

STATIC
EFI_STATUS
ReadDefaultSettingsToFactoryCopy (
  VOID
  );

STATIC
EFI_STATUS
RestoreFactoryCopyWithClearingAllSettings (
  VOID
  );

STATIC
EFI_STATUS
RestoreFactoryCopyWithReservingOtherSettings (
  VOID
  );

EFI_STATUS
CommonRegionDataCommunication (
  VOID
  );

EFI_STATUS
CommonRegionDataRead (
  VOID
  );
#endif
