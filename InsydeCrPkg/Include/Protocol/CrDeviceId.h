/** @file
  Console Redirection Device ID protocol definition
  
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

#ifndef _EFI_CR_DEVICE_ID_
#define _EFI_CR_DEVICE_ID_

#include <Uefi.h>

#define EFI_CR_DEVICE_ID_PROTOCOL_GUID \
  { 0x0b27cfe0, 0xe35b, 0x4d45, {0xb1, 0x7b, 0xb4, 0x4e, 0xb5, 0xfc, 0x5e, 0xaa} }

extern EFI_GUID gCrDeviceIdProtocolGuid;

#endif
