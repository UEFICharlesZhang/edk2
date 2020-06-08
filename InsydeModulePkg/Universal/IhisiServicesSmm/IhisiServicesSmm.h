/** @file
  Type definition for the IhisiSmm module

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

#ifndef _IHISI_SERVICES_SMM_H_
#define _IHISI_SERVICES_SMM_H_

#include "IhisiRegistration.h"
#include <Library/SmmChipsetSvcLib.h>
#include <Library/MemoryAllocationLib.h>

EFI_STATUS
InstallVatsServices (
  VOID
  );

EFI_STATUS
InstallFbtsServices (
  VOID
  );

EFI_STATUS
InstallFetsServices (
  VOID
  );

EFI_STATUS
InstallCommonRegionCommunicationServices (
  VOID
  );

EFI_STATUS
InstallOemExtraDataCommunicationServices (
  VOID
  );

EFI_STATUS
InstallSecureFlashServices (
  VOID
  );

#endif
