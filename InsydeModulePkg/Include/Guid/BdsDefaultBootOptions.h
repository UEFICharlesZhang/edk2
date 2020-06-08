/** @file
  Definitions for BDS default boot options

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

#ifndef __BDS_DEFAULT_BOOT_OPTIONS_GUID_H__
#define __BDS_DEFAULT_BOOT_OPTIONS_GUID_H__

#define H2O_BDS_DEFAULT_BOOT_OPTIONS_TOKEN_SPACE_GUID \
  { \
    0x44294584, 0x3431, 0x4cde, 0x82, 0x20, 0xc9, 0xb3, 0xe4, 0x60, 0x4a, 0x34 \
  };

#define H2O_BDS_DEFAULT_BOOT_LIST_GENERIC_OS_TOKEN_SPACE_GUID \
  { \
    0xd5eac930, 0xe571, 0x4a98, 0xa8, 0xb8, 0x1d, 0x79, 0x99, 0xb0, 0x1c, 0xbf \
  }


extern EFI_GUID gH2OBdsDefaultBootOptionsTokenSpaceGuid;
extern EFI_GUID gH2OBdsDefaultBootListGenericOsTokenSpaceGuid;

#endif
