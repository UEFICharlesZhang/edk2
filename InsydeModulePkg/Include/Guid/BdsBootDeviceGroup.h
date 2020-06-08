/** @file
  Definitions for BDS Boot Device Group Guid

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

#ifndef __BDS_BOOT_DEVICE_GROUP_GUID_H__
#define __BDS_BOOT_DEVICE_GROUP_GUID_H__

#define H2O_BDS_BOOT_GROUP_GUID \
  { \
    0x3575d6cf, 0x5f22, 0x49f0, 0xbe, 0x6e, 0xe6, 0x31, 0x51, 0xfb, 0x3d, 0x9d \
  };

#define H2O_BDS_BOOT_GROUP_NO_DELETE     0x0001
#define H2O_BDS_BOOT_GROUP_AUTO_CREATE   0x0002

typedef struct {
  VENDOR_DEFINED_DEVICE_PATH  Header;
  EFI_GUID                    VendorGuid;
  UINT16                      Flags;
//UINT8                       Data[];
} H2O_BDS_BOOT_GROUP_DEVICE_PATH;

extern EFI_GUID gH2OBdsBootDeviceGroupGuid;

#endif
