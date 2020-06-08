/** @file
  Provide support functions header for MultiConfig Update.
  
;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _OEM_MULTI_CONFIG_SMBIOS_H_
#define _OEM_MULTI_CONFIG_SMBIOS_H_

#define EFI_SMBIOS_ENTRY_SIGNATURE      SIGNATURE_32 ('S', 'r', 'e', 'c')
#define SMBIOS_INSTANCE_SIGNATURE       SIGNATURE_32 ('S', 'B', 'i', 's')
#define SMBIOS_INSTANCE_FROM_THIS(this) CR (this, SMBIOS_INSTANCE, Smbios, SMBIOS_INSTANCE_SIGNATURE)
#define SMBIOS_ENTRY_FROM_LINK(link)    CR (link, EFI_SMBIOS_ENTRY, Link, EFI_SMBIOS_ENTRY_SIGNATURE)
#define SMBIOS_REMOVE_TYPE              0x7E
#define SMBIOS_DMI_END_TYPE             0xFF
#define SMBIOS_DMI_VALID                0xFF

#define SMBIOS_MC_UPDATE_SMBIOS         0x03
#define MC_VALID_SKUID                  0x00
#define MC_UPDATE_SMBIOS_MAX_LENGTH     0xFF
#define MC_SMBIOS_HANDLE_PI_RESERVED    0xFFFE

typedef struct {
  UINT16      Version;
  UINT16      HeaderSize;
  UINTN       RecordSize;
  EFI_HANDLE  ProducerHandle;
  UINTN       NumberOfStrings;
} EFI_SMBIOS_RECORD_HEADER;

typedef struct {
  UINT32                    Signature;
  LIST_ENTRY                Link;
  EFI_SMBIOS_RECORD_HEADER  *RecordHeader;
  UINTN                     RecordSize;
} EFI_SMBIOS_ENTRY;

#pragma pack(1)

typedef enum {
  SMBIOS_REMOVE_ATTR,
  SMBIOS_BYTE_ATTR,
  SMBIOS_WORD_ATTR,
  SMBIOS_DWORD_ATTR,
  SMBIOS_STRING_ATTR,
  SMBIOS_BLOCK_ATTR,
  SMBIOS_UUID_ATTR,
} MC_SMBIOS_DATA_ATTR;

//
//    --------------------
//   |  MC_SMBIOS_HEADER  |
//   |--------------------|
//   |  MC_SMBIOS_SETTING |
//   |--------------------|
//   |  MC_SMBIOS_SETTING |
//   |--------------------|
//   |         :          |
//   |         :          |
//

typedef struct {
  UINT8             Type;
  UINT8             Instance;
  UINT16            SmbiosOffset;
  UINT8             DataAttr;
  UINT32            DataSize;
} MC_SMBIOS_SETTING_L;

typedef struct {
  UINT8             Type;
  UINT8             Instance;
  UINT16            SmbiosOffset;
  UINT8             DataAttr;
  UINT32            DataSize;
  UINT8             Data[1];
} MC_SMBIOS_SETTING;

typedef struct {
  UINT16            TotalItem;
  MC_SMBIOS_SETTING FirstData;
} MC_SMBIOS_HEADER;

#pragma pack()

#endif

