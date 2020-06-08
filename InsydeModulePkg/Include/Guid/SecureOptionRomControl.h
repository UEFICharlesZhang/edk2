/** @file

   The definition of Secure Option Rom Control Implementation.

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

#ifndef _SECURE_OPTION_ROM_CONTROL_H_
#define _SECURE_OPTION_ROM_CONTROL_H_

#define SECURE_OPTION_ROM_CONTROL_FORMSET_GUID \
  { \
    0x5080D490, 0x07C3, 0x4A70, {0xB0, 0xE1, 0xF7, 0x68, 0x49, 0x6C, 0x5D, 0x65} \
  } 

#define SECURE_OPTION_ROM_CONTROL_FORM_ID           0x330

extern EFI_GUID gH2OSecureOptionRomControlFormsetGuid;

#endif
