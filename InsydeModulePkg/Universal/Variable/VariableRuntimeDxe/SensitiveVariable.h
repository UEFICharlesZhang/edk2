/** @file
    The header file for sensitive variable functions.

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

#ifndef _SENSITIVE_VARIABLE_H_
#define _SENSITIVE_VARIABLE_H_
#include "Variable.h"

#include <SensitiveVariableFormat.h>


/**
  This fucnitons uses to set sensitive variable in SMM.

  @retval EFI_SUCCESS     Set sensitive variable successfully.
  @return Other           Set sensitive variable failed.
**/
EFI_STATUS
SmmSetSensitiveVariable (
  VOID
  );


#endif
