/** @file
  Console Redirection Device Support protocol definition

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

#ifndef _EFI_CR_DEVICE_SUPPORT_
#define _EFI_CR_DEVICE_SUPPORT_

#include <Uefi.h>
#include <Protocol/CRPolicy.h>

#define EFI_CR_DEVICE_SUPPORT_PROTOCOL_GUID \
  { 0xe094e6c6, 0xdd1e, 0x4422, {0xb5, 0x63, 0xa4, 0x18, 0x0d, 0x5d, 0x16, 0x60} }

extern EFI_GUID gCrDeviceSupportProtocolGuid;

typedef enum {
  CR_NonPnpDevice,
  CR_PnpDevice,
  CR_AllDevice
} CR_DEVICE_CATEGORY;

typedef 
EFI_STATUS
(EFIAPI *CONNECT_TERMINAL_DEVICE) ( 
  VOID
);

typedef struct _CR_DEVICE_SUPPORT_PROTOCOL CR_DEVICE_SUPPORT_PROTOCOL;

struct _CR_DEVICE_SUPPORT_PROTOCOL {
  CR_DEVICE_CATEGORY        CrDevCategory;
  UINT8                     ClassCode[3];
  CONNECT_TERMINAL_DEVICE   ConnectTerminalDevice;
};

#endif
