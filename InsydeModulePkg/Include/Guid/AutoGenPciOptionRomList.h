/** @file
 Guid used to get and parsing AutoGen Pci option roms table

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

#ifndef _AUTO_GEN_PCI_OPTION_ROM_LIST_H_
#define _AUTO_GEN_PCI_OPTION_ROM_LIST_H_

#define AUTO_GEN_PCI_OPTION_ROM_LIST_GUID \
  { 0xbfc113fe, 0xad4b, 0x4f97, { 0x9f, 0x70, 0x07, 0x47, 0x36, 0x9f, 0xbe, 0xba }}

extern EFI_GUID gAutoGenPciOptionRomListGuid;

#pragma pack(1)
typedef struct _AUTO_GEN_PCI_OPTION_ROM_TABLE {
  EFI_GUID  FileName;
  UINT16    VendorId;
  UINT16    DeviceIdCount;
  UINT16    DeviceId[1];
} AUTO_GEN_PCI_OPTION_ROM_TABLE;
#pragma pack()

#endif

