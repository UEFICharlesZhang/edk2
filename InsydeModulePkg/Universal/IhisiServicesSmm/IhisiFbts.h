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

#ifndef _IHISI_FBTS_H_
#define _IHISI_FBTS_H_

#include "IhisiServicesSmm.h"

typedef struct {
  UINT8      IhisiAreaType;
  UINT8      FlashAreaType;
} FLASH_MAP_MAPPING_TABLE;

UINT32
GetFbtsIhisiStatus (
  EFI_STATUS                             Status
  );

EFI_STATUS
FbtsGetSupportVersion (
  VOID
  );

EFI_STATUS
FbtsGetPlatformInfo (
  VOID
  );

EFI_STATUS
FbtsGetPlatformRomMap (
  VOID
  );

EFI_STATUS
FbtsGetFlashPartInfo (
  VOID
  );

EFI_STATUS
SkipMcCheckAndBinaryTrans (
  VOID
  );

EFI_STATUS
FbtsRead (
  VOID
  );

EFI_STATUS
FbtsWrite (
  VOID
  );

EFI_STATUS
FbtsComplete (
  VOID
  );

EFI_STATUS
FbtsGetWholeBiosRomMap (
  VOID
  );
#endif
