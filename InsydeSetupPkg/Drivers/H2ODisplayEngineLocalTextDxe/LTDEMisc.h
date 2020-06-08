/** @file

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _LTDE_MISC_H_
#define _LTDE_MISC_H_

CHAR16 *
PrintFormattedNumber (
  IN H2O_FORM_BROWSER_Q                       *Question
  );

CHAR16 *
CreateSpaceString (
  IN UINT32                                   StringLength
  );

#endif
