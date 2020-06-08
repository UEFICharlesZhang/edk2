/** @file
  Definitions for BDS check point

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

#ifndef __H2O_BDS_CHECK_POINT_H__
#define __H2O_BDS_CHECK_POINT_H__

#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImage.h>
#include <Protocol/OEMBadgingSupport.h>

#define H2O_BDS_CP_BOOT_AFTER_PROTOCOL_GUID \
  { \
    0x4173b6cd, 0x5711, 0x462c, 0x9c, 0x6e, 0x1f, 0x3c, 0x95, 0x49, 0xcc, 0x9f \
  };

#define H2O_BDS_CP_BOOT_BEFORE_PROTOCOL_GUID \
  { \
    0xe7e316d5, 0xffb9, 0x4cec, 0xba, 0x82, 0x3b, 0x0, 0xe5, 0x3e, 0x12, 0xa8 \
  };

#define H2O_BDS_CP_CON_IN_CONNECT_AFTER_PROTOCOL_GUID \
  { \
    0xf32791c1, 0x6551, 0x449d, 0xbe, 0x5a, 0x99, 0xbe, 0x0, 0xe, 0xe6, 0x6b \
  };

#define H2O_BDS_CP_CON_IN_CONNECT_BEFORE_PROTOCOL_GUID \
  { \
    0x8a1a9798, 0xadfb, 0x4483, 0xa1, 0xc3, 0x29, 0x9d, 0xab, 0x99, 0x46, 0xdb \
  };

#define H2O_BDS_CP_CON_OUT_CONNECT_AFTER_PROTOCOL_GUID \
  { \
    0xd2c4a682, 0xa6ef, 0x4410, 0xb1, 0x2d, 0x58, 0xd6, 0xdd, 0xc1, 0xf0, 0xb2 \
  };

#define H2O_BDS_CP_CON_OUT_CONNECT_BEFORE_PROTOCOL_GUID \
  { \
    0x68655f09, 0x464a, 0x4642, 0xa7, 0x22, 0xd1, 0x18, 0x93, 0xbd, 0x3f, 0x7b \
  };

#define H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL_GUID \
  { \
    0xc5d30c2c, 0xd1f6, 0x401b, 0xbb, 0xd3, 0x74, 0xe6, 0xde, 0x1b, 0x4f, 0xf \
  };

#define H2O_BDS_CP_DISPLAY_STRING_BEFORE_PROTOCOL_GUID \
  { \
    0x9ba004e5, 0xf300, 0x453a, 0xb3, 0xbe, 0x3b, 0x7c, 0xeb, 0x21, 0xb7, 0x74 \
  };

#define H2O_BDS_CP_DRIVER_CONNECT_AFTER_PROTOCOL_GUID \
  { \
    0x64b0026a, 0x6886, 0x4ca3, 0xa4, 0xa8, 0x95, 0xed, 0x36, 0x68, 0x1f, 0x54 \
  };

#define H2O_BDS_CP_DRIVER_CONNECT_BEFORE_PROTOCOL_GUID \
  { \
    0x3faf7df3, 0x9b55, 0x46f8, 0xad, 0xfa, 0x12, 0x8e, 0x9d, 0x90, 0x7a, 0x1c \
  };

#define H2O_BDS_CP_END_OF_DXE_AFTER_PROTOCOL_GUID \
  { \
    0x5b02c9e2, 0x7ce3, 0x403a, 0x99, 0x7c, 0xca, 0x10, 0xd8, 0x66, 0x50, 0x7d \
  };

#define H2O_BDS_CP_END_OF_DXE_BEFORE_PROTOCOL_GUID \
  { \
    0x6adc6c22, 0x72e1, 0x410b, 0xa9, 0xf2, 0x9a, 0xc6, 0xed, 0xe9, 0xfe, 0x5e \
  };

#define H2O_BDS_CP_EXIT_BOOT_SERVICES_BEFORE_PROTOCOL_GUID \
  { \
    0x1dd55b9d, 0x3e0c, 0x41b1, 0xb0, 0xcf, 0x5b, 0xfd, 0x70, 0xb5, 0xc2, 0x12 \
  };

#define H2O_BDS_CP_INIT_PROTOCOL_GUID \
  { \
    0xd74ef78e, 0xc75b, 0x4786, 0xba, 0x79, 0xd2, 0xc2, 0xb6, 0x72, 0x52, 0x50 \
  };

#define H2O_BDS_CP_LEGACY_BOOT_AFTER_PROTOCOL_GUID \
  { \
    0xb96a8be, 0xb79b, 0x419e, 0x8b, 0x13, 0x70, 0x45, 0x37, 0xac, 0x3c, 0xdb \
  };

#define H2O_BDS_CP_LEGACY_BOOT_BEFORE_PROTOCOL_GUID \
  { \
    0xf9afcfac, 0x68bb, 0x4c3c, 0x9e, 0xe1, 0x33, 0x5a, 0x91, 0x2b, 0x30, 0xb5 \
  };

#define H2O_BDS_CP_READY_TO_BOOT_AFTER_PROTOCOL_GUID \
  { \
    0xef208648, 0xb128, 0x4257, 0x9f, 0x21, 0xba, 0xaa, 0x51, 0x17, 0xef, 0xa8 \
  };

#define H2O_BDS_CP_READY_TO_BOOT_BEFORE_PROTOCOL_GUID \
  { \
    0x463ee173, 0x1485, 0x4749, 0xb7, 0x82, 0x91, 0x30, 0xdd, 0xf2, 0x2e, 0xaa \
  };


typedef struct _H2O_BDS_CP_BOOT_AFTER_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_BOOT_AFTER_PROTOCOL;

typedef struct _H2O_BDS_CP_BOOT_BEFORE_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_BOOT_BEFORE_PROTOCOL;

typedef struct _H2O_BDS_CP_CON_IN_CONNECT_AFTER_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_CON_IN_CONNECT_AFTER_PROTOCOL;

typedef struct _H2O_BDS_CP_CON_IN_CONNECT_BEFORE_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_CON_IN_CONNECT_BEFORE_PROTOCOL;

typedef struct _H2O_BDS_CP_CON_OUT_CONNECT_AFTER_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_CON_OUT_CONNECT_AFTER_PROTOCOL;

typedef struct _H2O_BDS_CP_CON_OUT_CONNECT_BEFORE_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_CON_OUT_CONNECT_BEFORE_PROTOCOL;

typedef struct _H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL {
  UINT32          Size;
  UINT32          Status;
  UINT32          Features;
  EFI_IMAGE_INPUT *Image;
} H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL;

//
// Task behavior for H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL structure Status member
//
typedef enum {
  H2O_BDS_TASK_NORMAL,
  H2O_BDS_TASK_SKIP,
} BDS_TASK_BEHAVIOR;

//
// Bitmask that specifies the kernel¡¦s current policy concerning the primary display contents
//
#define H2O_BDS_CP_DISPLAY_LOGO          0x0000000000000001
#define H2O_BDS_CP_DISPLAY_INFO          0x0000000000000002
#define H2O_BDS_CP_DISPLAY_BGRT          0x0000000000000004

typedef struct _H2O_BDS_CP_DISPLAY_STRING_BEFORE_PROTOCOL {
  UINT32                             Size;
  UINT32                             Status;
  EFI_OEM_BADGING_SUPPORT_PROTOCOL   *Badging;
  BOOLEAN                            AfterSelect;
  UINT8                              SelectedStringNum;
} H2O_BDS_CP_DISPLAY_STRING_BEFORE_PROTOCOL;

typedef struct _H2O_BDS_CP_DRIVER_CONNECT_AFTER_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_DRIVER_CONNECT_AFTER_PROTOCOL;

typedef struct _H2O_BDS_CP_DRIVER_CONNECT_BEFORE_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_DRIVER_CONNECT_BEFORE_PROTOCOL;

typedef struct _H2O_BDS_CP_END_OF_DXE_AFTER_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_END_OF_DXE_AFTER_PROTOCOL;

typedef struct _H2O_BDS_CP_END_OF_DXE_BEFORE_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_END_OF_DXE_BEFORE_PROTOCOL;

typedef struct _H2O_BDS_CP_EXIT_BOOT_SERVICES_BEFORE_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_EXIT_BOOT_SERVICES_BEFORE_PROTOCOL;

typedef struct _H2O_BDS_CP_INIT_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_INIT_PROTOCOL;

typedef struct _H2O_BDS_CP_LEGACY_BOOT_AFTER_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_LEGACY_BOOT_AFTER_PROTOCOL;

typedef struct _H2O_BDS_CP_LEGACY_BOOT_BEFORE_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_LEGACY_BOOT_BEFORE_PROTOCOL;

typedef struct _H2O_BDS_CP_READY_TO_BOOT_AFTER_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_READY_TO_BOOT_AFTER_PROTOCOL;

typedef struct _H2O_BDS_CP_READY_TO_BOOT_BEFORE_PROTOCOL {
  UINT32          Size;
} H2O_BDS_CP_READY_TO_BOOT_BEFORE_PROTOCOL;

extern EFI_GUID gH2OBdsCpBootAfterProtocolGuid;
extern EFI_GUID gH2OBdsCpBootBeforeProtocolGuid;
extern EFI_GUID gH2OBdsCpConInConnectAfterProtocolGuid;
extern EFI_GUID gH2OBdsCpConInConnectBeforeProtocolGuid;
extern EFI_GUID gH2OBdsCpConOutConnectAfterProtocolGuid;
extern EFI_GUID gH2OBdsCpConOutConnectBeforeProtocolGuid;
extern EFI_GUID gH2OBdsCpDisplayBeforeProtocolGuid;
extern EFI_GUID gH2OBdsCpDisplayStringBeforeProtocolGuid;
extern EFI_GUID gH2OBdsCpDriverConnectAfterProtocolGuid;
extern EFI_GUID gH2OBdsCpDriverConnectBeforeProtocolGuid;
extern EFI_GUID gH2OBdsCpEndOfDxeAfterProtocolGuid;
extern EFI_GUID gH2OBdsCpEndOfDxeBeforeProtocolGuid;
extern EFI_GUID gH2OBdsCpExitBootServicesBeforeProtocolGuid;
extern EFI_GUID gH2OBdsCpInitProtocolGuid;
extern EFI_GUID gH2OBdsCpLegacyBootAfterProtocolGuid;
extern EFI_GUID gH2OBdsCpLegacyBootBeforeProtocolGuid;
extern EFI_GUID gH2OBdsCpReadyToBootAfterProtocolGuid;
extern EFI_GUID gH2OBdsCpReadyToBootBeforeProtocolGuid;


#endif
