/** @H2O pei storage hob data struct

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

#ifndef _H2O_PEI_STORAGE_HOB_H_
#define _H2O_PEI_STORAGE_HOB_H_

#define H2O_PEI_STORAGE_HOB_GUID  \
  { \
    0xac199b84, 0x161c, 0x45b7, 0x9a, 0x65, 0x5a, 0xf5, 0x2f, 0xe3, 0xef, 0x35 \
  }

typedef union {
  struct {
    UINT32  UsbEnable  :1;
    /* UsbEnable - Bits[0], RW, default = 1'b0
           Enable UsbPolicy in Pei Phase. The usb relate modules will dispatch.
           That can use usb device in Pei phase. (ex. usb mass storage)
      */
    UINT32  SataEnable :1;
    /* SataEnable - Bits[1], RW, default = 1'b0
           Enable Sata Driver in Pei Phase. It can access SATA HD in pei phase.
      */   
    UINT32  Reserved  :30;
  } Bits;
  UINT32 Data;
} PEI_STORAGE_HOB_DATA;

extern EFI_GUID gH2OPeiStorageHobGuid;

#endif
