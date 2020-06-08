/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _IHISI_FETS_H_
#define _IHISI_FETS_H_

#include "IhisiRegistration.h"

/**
  AH=20h, Flash EC through SMI (FETS) Write.

  @retval EFI_SUCCESS    Flash EC successful.
  @return Other          Flash EC failed.
**/
EFI_STATUS
FetsWrite (
  VOID
  );

/**
  AH=21h, Get EC part information.

  @retval EFI_SUCCESS     Get EC part information successful.
**/
EFI_STATUS
GetPartInfo (
  VOID
  );

#endif
