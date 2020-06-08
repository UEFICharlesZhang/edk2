/** @file
  This file for CR SOL reference

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef __SOL_H__
#define __SOL_H__

#include <CrSetupConfig.h>

#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>

#include <Protocol/H2ODialog.h>
#include <Protocol/CRPolicy.h>


EFI_STATUS
SolConfigCallback (
  EFI_QUESTION_ID          QuestionId,
  CR_SERIAL_DEV_INFO       *CrIfrNvdata
  );
  
EFI_STATUS
SolConfigFormInit (
  EFI_HII_HANDLE           HiiHandle,
  CR_SERIAL_DEV_INFO       *CrIfrNvdata
  );

VOID
SolConfigSetToDefault (
  CR_SOL_CONFIG       *SolConfigNvdata
  );

VOID
SolUpdateEntrys (
  CR_SERIAL_DEV_INFO       *CrIfrNvdata
  );

#endif // __SOL_H__
