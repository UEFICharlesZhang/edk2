/** @file
  Header file for .FDM region GUID
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

#ifndef _FLASH_REGION_LAYOUT_H_
#define _FLASH_REGION_LAYOUT_H_


#define H2O_FLASH_MAP_REGION_BVDT_GUID \
  { 0x32415dfc, 0xd106, 0x48c7, 0x9e, 0xb5, 0x80, 0x6c, 0x11, 0x4d, 0xd1, 0x07} 


#define H2O_FLASH_MAP_REGION_EC_GUID \
  { 0xa73ef3bf, 0x33cc, 0x43a9, 0xb3, 0x9c, 0xa9, 0x12, 0xc7, 0x48, 0x9a, 0x57}

  
#define H2O_FLASH_MAP_REGION_FTW_BACKUP_GUID \
  { 0xb78e15d3, 0xf0a5, 0x4248, 0x8e, 0x2f, 0xd3, 0x15, 0x7a, 0xef, 0x88, 0x36}


#define H2O_FLASH_MAP_REGION_FTW_STATE_GUID \
  { 0xc8416e04, 0x9934, 0x4079, 0xbe, 0x9a, 0x39, 0xf8, 0xd6, 0x02, 0x84, 0x98}


#define H2O_FLASH_MAP_REGION_FV_GUID \
  { 0xb5e8e758, 0xa7e6, 0x4c8b, 0xab, 0x85, 0xff, 0x2a, 0x95, 0x9b, 0x99, 0xba}

  
#define H2O_FLASH_MAP_REGION_FLASH_MAP_GUID \
  { 0xf078c1a0, 0xfc52, 0x4c3f, 0xbe, 0x1f, 0xd6, 0x88, 0x81, 0x5a, 0x62, 0xc0} 


#define H2O_FLASH_MAP_REGION_GPNV_GUID \
  { 0x29280631, 0x623b, 0x43e4, 0xbc, 0xa1, 0x00, 0x52, 0x14, 0xc4, 0x83, 0xa6}

  
#define H2O_FLASH_MAP_REGION_LOGO_GUID \
  { 0xdacfab69, 0xf977, 0x4784, 0x8a, 0xd8, 0x77, 0x24, 0xa6, 0xf4, 0xb4, 0x40}

  
#define H2O_FLASH_MAP_REGION_MICROCODE_GUID \
  { 0xb49866f8, 0x8cd2, 0x49e4, 0xa1, 0x6d, 0xb6, 0x0f, 0xbe, 0xc3, 0x1c, 0x4b}
  
#define H2O_FLASH_MAP_REGION_MULTI_CONFIG_GUID \
  { 0x5994b592, 0x2f14, 0x48d5, 0xbb, 0x40, 0xbd, 0x27, 0x96, 0x9c, 0x77, 0x80}

  
#define H2O_FLASH_MAP_REGION_ODM_GUID \
  { 0xa42c1051, 0x73b5, 0x41a9, 0xb6, 0x35, 0x0c, 0xc5, 0x1c, 0x82, 0x72, 0xf8}  

  
#define H2O_FLASH_MAP_REGION_OEM_GUID \
  { 0x2fd91ad6, 0xd8e3, 0x4fd6, 0xb6, 0x79, 0x30, 0x30, 0xe8, 0x6a, 0xe5, 0x7a}

 
#define H2O_FLASH_MAP_REGION_PASSWORD \
  { 0xc0027e32, 0x8ee5, 0x4d17, 0x9b, 0x28, 0xba, 0x50, 0x16, 0x6c, 0x4c, 0xb4}


#define H2O_FLASH_MAP_REGION_SMBIOS_EVENT_LOG_GUID \
  { 0xb95d2198, 0x8e70, 0x4cdc, 0x93, 0x7d, 0x9a, 0x3f, 0x79, 0x5f, 0x99, 0x05}

  
#define H2O_FLASH_MAP_REGION_SMBIOS_UPDATE_GUID \
  { 0x8964fedc, 0x6fe7, 0x4e1e, 0xa5, 0x5e, 0xff, 0x82, 0x1d, 0x71, 0xff, 0xcf}


#define H2O_FLASH_MAP_REGION_VAR_GUID \
  { 0x773c5374, 0x81d1, 0x4d43, 0xb2, 0x93, 0xf3, 0xd7, 0x4f, 0x18, 0x1d, 0x6b}


#define H2O_FLASH_MAP_REGION_UNKNOWN \
  { 0x201d65e5, 0xbe23, 0x4875, 0x80, 0xf8, 0xb1, 0xd4, 0x79, 0x5e, 0x7e, 0x08}

  
#define H2O_FLASH_MAP_REGION_UNUSED \
  { 0x13c8b020, 0x4f27, 0x453b, 0x8f, 0x80, 0x1b, 0xfc, 0xa1, 0x87, 0x38, 0x0f}


extern EFI_GUID gH2OFlashMapRegionBvdtGuid;
extern EFI_GUID gH2OFlashMapRegionEcGuid;
extern EFI_GUID gH2OFlashMapRegionFtwBackupGuid;
extern EFI_GUID gH2OFlashMapFtwStateGuid;
extern EFI_GUID gH2OFlashMapRegionFvGuid;
extern EFI_GUID gH2OFlashMapRegionFlashMapGuid;
extern EFI_GUID gH2OFlashMapRegionGpnvGuid;
extern EFI_GUID gH2OFlashMapRegionLogoGuid;
extern EFI_GUID gH2OFlashMapRegionMicrocodeGuid;
extern EFI_GUID gH2OFlashMapRegionMultiConfigGuid;
extern EFI_GUID gH2OFlashMapRegionOdmGuid;
extern EFI_GUID gH2OFlashMapRegionOemGuid;
extern EFI_GUID gH2OFlashMapRegionPasswordGuid;
extern EFI_GUID gH2OFlashMapRegionSmbiosEventLogGuid;
extern EFI_GUID gH2OFlashMapRegionSmbiosUpdateGuid;
extern EFI_GUID gH2OFlashMapRegionVarGuid;
extern EFI_GUID gH2OFlashMapRegionUnknownGuid;
extern EFI_GUID gH2OFlashMapRegionUnusedGuid;


#endif

