/** @file
 Internal include file for display type

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

#ifndef _INTERNALH2O_DISPLAY_TYPE_H_
#define _INTERNALH2O_DISPLAY_TYPE_H_

#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/DevicePath.h>

#include <Guid/H2ODisplayEngineType.h>
#include <Guid/GlobalVariable.h>

#include <IndustryStandard/Pci.h>

typedef enum {
  Igfx = 0,
  Peg,
  Pci,
  Serial,
  DisplayTypeMax
} DISPLAY_TYPE;

typedef enum {
  DisplayModeIgfx  = 0,
  DisplayModePeg,
  DisplayModePci,
  DisplayModeAuto,
  DisplayModeSg
} PRIMARY_DISPLAY;

typedef struct _DISPLAY_TYPE_PCDGUID_MAP {
  DISPLAY_TYPE                   DisplayType;
  EFI_GUID                       *PcdGuid;
} DISPLAY_TYPE_PCDGUID_MAP;

#define EISA_SERIAL_DEVICE_ID    0x0501

extern EFI_GUID                  gH2ODisplayTypePrimaryDisplayGuid;

#endif

