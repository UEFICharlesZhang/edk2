/** @file
  H2OIhisi Commond define

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

#ifndef _H2O_IHISI_H_
#define _H2O_IHISI_H_

#include <Uefi.h>

#define ROM_32M_BASE                0xFE000000
#define ROM_16M_BASE                0xFF000000
#define ROM_8M_BASE                 0xFF800000
#define ROM_4M_BASE                 0xFFC00000
#define ROM_2M_BASE                 0xFFE00000
#define ROM_1M_BASE                 0xFFF00000
#define ROM_512K_BASE               0xFFF80000
#define ROM_256K_BASE               0xFFFC0000
#define ROM_128K_BASE               0xFFFE0000

typedef struct {
  UINT8      SizeIndex;
  UINT32     Base;
} FLASH_BASE_MAP_TABLE;

//
//IHISI Sub function number
//
typedef enum {
//
// Variable Access through SMI (VATS)
//
  //Sub-functions definition for Variable Access through SMI (VATS).
  VATSRead  = 0x00,    //Read variable
  VATSWrite = 0x01,    //Write variable
  //Sub-functions definition for DMI Access through SMI(DATS).
  DATSWrite = 0x02,    //Write DMI Command
  DATSErase = 0x03,    //Erase DMI area
  DATSQuery = 0x04,    //Query all DMI variable

//
// Flash BIOS through SMI (FBTS)
//
  //Sub-functions definition for Flash BIOS through SMI(FBTS-1)
  FBTSGetSupportVersion           = 0x10,   //Get FBTS supported version and FBTS permission.
  FBTSGetPlatformInfo             = 0x11,   //Get platform information.
  FBTSGetPlatformRomMap           = 0x12,   //Get Platform ROM map protection.
  FBTSGetFlashPartInfo            = 0x13,   //Flash part information
  FBTSRead                        = 0x14,   //FBTS Read.
  FBTSWrite                       = 0x15,   //FBTS Write.
  FBTSComplete                    = 0x16,   //FBTS Flash complete.
  FBTSGetRomFileAndPlatformTable  = 0x17,   //Get external ROM file and platform region table (External ROM map)
  FBTSSecureFlashBIOS             = 0x18,   //Secure flash BIOS

  //Sub-functions definition for Flash BIOS through SMI(FBTS-2).
  FBTSOemCustomization1           = 0x19,   //OEM/ODM customization-1
  FBTSOemCustomization2           = 0x1A,   //OEM/ODM customization-2
  FBTSSkipMcCheckAndBinaryTrans   = 0x1B,   //Skip module check allows and binary file transmissions
  FBTSGetATpInformation           = 0x1C,   //Get AT-p information
  FBTSPassPlatforminiSettings     = 0x1D,   //Pass platform.ini settings
  FBTSGetWholeBiosRomMap          = 0x1E,   //Get whole BIOS ROM map (Internal BIOS map)
  FBTSApHookPoint                 = 0x1F,   //AP Hook Point for BIOS
  FBTSOEMCapsuleSecureFalsh       = 0x48,   //OEM capsule secure flash
  FBTSPassImageFromTool           = 0x4D,   //Passing Image to BIOS for specific check (BIOS is master to query image from tool via the interface)
  FBTSGetRuntimeBuffer            = 0x4E,   //Query BIOS to get runtime buffer address and size
  FBTSPassImagetoBios             = 0x50,   //Passing Image to BIOS for specific check (Tool is master to pass image to BIOS via the interface)
  FBTSWriteToSPIRom               = 0x51,   //Inform BIOS to write image data to SPI ROM
  //Sub-functions definition to do common feature
  FBTSCommonCommunication         = 0x49,   //Common Region Data Communication
  FBTSCommonWrite                 = 0x4A,   //Common Region Data Write
  FBTSCommonRead                  = 0x4B,   //Common Region Data Read

//
// Flash EC through SMI (FETS)
//
  FETSWrite         = 0x20,   //FETS Write
  FETSGetEcPartInfo = 0x21,   //EC part information
  FETSRead          = 0x22,   //FETS Read

//
// Flash ME through SMI (FMTS)
//
  FMTSGetMEPartInfo = 0x30,   //OEM ME part information.
  FMTSWrite         = 0x31,   //FMTS Write

//
// OEM special feature (OEMS)
//
  //Sub-functions definition for Get OEM1 information
  OEMSFGetOEMInfo1        = 0x40,    //Get OEM1 information
  OEMSFGetOEMInfoEx       = 0x46,    //Get OEM1 information extending
  OEMSFOEMSecureFlash     = 0x4C,    //OEM secure flash
  //Sub-functions definition to do OEM common feature
  OEMSFOEMExCommunication = 0x41,   //OEM Extra Data Communication
  OEMSFOEMExDataWrite     = 0x42,   //OEM Extra Data Write
  OEMSFOEMExDataRead      = 0x47,   //OEM Extra Data Read
  OEMSFCheckPassword      = 0x43,   //Check password by BIOS
  OEMSFPassFlashSetting   = 0x44,   //Pass Flash Settings to BIOS
  OEMSFCheckCondition     = 0x45,   //Check conditional data by BIOS

//BLD_Charles_035+s
  SubFuncF0NovoWrite = 0xF0,
  SubFuncF1NovoRead  = 0xF1,
//BLD_Charles_035+e

//
// BIOS General Communication Interface
//
  DATAAccessCommunication = 0x60,   //Data Access Communication
  DATARead                = 0x61,   //Data Read
  DATAWrite               = 0x62,   //Data Write
} IHISI_SMI;

//
// Ihisi Status Code
//
typedef UINT32 IHISI_STATUS;

typedef enum {
  IhisiSuccess                = 0x00,
  IhisiAccessProhibited       = 0x01,
  IhisiObLenTooSmall          = 0x02,
  IhisiInvalidParamFatalError = 0x03,
  IhisiFunNotSupported        = 0xEF,
  IhisiUnknownStatus          = 0xFF,
  //
  //  VATS status code.
  //
  VatsVarNotFound             = 0x10,
  VatsWrongObFormat           = 0x11,
  VatsObChecksumFailed        = 0x12,
  VataObTableRevUnsupported   = 0x13,
  VatsVarPartAccessErr        = 0x14,
  //
  // FBTS status code.
  //
  FbtsPermissionDeny          = 0x20,
  FbtsUnknowPlatformInfo      = 0x21,
  FbtsUnknowPlatformRomMap    = 0x22,
  FbtsUnknowFlashPartInfo     = 0x23,
  FbtsReadFail                = 0x24,
  FbtsWriteFail               = 0x25,
  FbtsEraseFail               = 0x26,
  FbtsCanNotSkipModuleCheck   = 0x27,
  FbtsNextBlock               = 0x28,
  FbtsSkipThisWriteBlock      = 0x29,
  FbtsMeLockReadFail          = 0x2A
} IHISI_STATUS_CODE;

#define IHISI_STATUS_BIT                       BIT15
#define ENCODE_IHISI_STATUS(StatusCode)        ((RETURN_STATUS)((IHISI_STATUS_BIT) | (StatusCode)))

#define IHISI_SUCCESS                          0x00
#define IHISI_ACCESS_PROHIBITED                ENCODE_IHISI_STATUS(0x01)
#define IHISI_OB_LEN_TOO_SMALL                 ENCODE_IHISI_STATUS(0x02)
#define IHISI_INVALID_PARAMETER                ENCODE_IHISI_STATUS(0x03)
#define IHISI_UNSUPPORTED_FUNCTION             ENCODE_IHISI_STATUS(0xEF)
#define IHISI_VATS_VARIABLE_NOT_FOUND          ENCODE_IHISI_STATUS(0x10)
#define IHISI_VATS_WRONG_OB_FORMAT             ENCODE_IHISI_STATUS(0x11)
#define IHISI_VATS_OB_CHECKSUM_FAILED          ENCODE_IHISI_STATUS(0x12)
#define IHISI_VATS_OB_TABLE_REV_UNSUPPORTED    ENCODE_IHISI_STATUS(0x13)
#define IHISI_VATS_VARIABLE_ACCESS_ERROR       ENCODE_IHISI_STATUS(0x14)
#define IHISI_FBTS_PERMISSION_DENIED           ENCODE_IHISI_STATUS(0x20)
#define IHISI_FBTS_UNKNOWN_PLATFORM_INFO       ENCODE_IHISI_STATUS(0x21)
#define IHISI_FBTS_UNKNOWN_PLATFORM_ROM_MAP    ENCODE_IHISI_STATUS(0x22)
#define IHISI_FBTS_UNKNOWN_FLASH_PART_INFO     ENCODE_IHISI_STATUS(0x23)
#define IHISI_FBTS_READ_FAILED                 ENCODE_IHISI_STATUS(0x24)
#define IHISI_FBTS_WRITE_FAILED                ENCODE_IHISI_STATUS(0x25)
#define IHISI_FBTS_ERASE_FAILED                ENCODE_IHISI_STATUS(0x26)
#define IHISI_FBTS_CANNOT_SKIP_MODULE_CHECK    ENCODE_IHISI_STATUS(0x27)
#define IHISI_FBTS_NEXT_BLOCK                  ENCODE_IHISI_STATUS(0x28)
#define IHISI_FBTS_SKIP_THIS_WRITE_BLOCK       ENCODE_IHISI_STATUS(0x29)
#define IHISI_FBTS_ME_LOCK_READ_FAILED         ENCODE_IHISI_STATUS(0x2A)

#define IHISI_END_FUNCTION_CHAIN  0x00001001

#define bit(a)                          1 << (a)

/**

AH=00h,01h; Read/write the specific variable into thespecified buffer.

**/
#define IHISI_H2O_SIGNATURE        SIGNATURE_32 ('$', 'H', '2', 'O')
#define IHISI_VAR_SIGNATURE        SIGNATURE_32 ('$', 'V', 'a', 'r')
#define IHISI_TBL_SIGNATURE        SIGNATURE_32 ('$', 'T', 'b', 'l')

// Output table elements, for VATS security check.
#define IHISI_TABLE_REVISION       0x10

#pragma pack(1)
// INPUT:
//  ESI:VATS Input Buffer Structure
typedef struct {
  EFI_GUID                              VarGuid;
} IHISI_VATS_INPUT_BUFFER;

// OUTPUT:
//  EDI:VATS Output Buffer Structure
typedef struct {
  UINT32                  TableId1;   // $H2O (IHISI_H2O_SIGNATURE)
  UINT32                  TableId2;   // $Var (IHISI_VAR_SIGNATURE)
  UINT32                  TableId3;   // $Tbl (IHISI_TBL_SIGNATURE)
  UINT8                   Reserved1[4];
  UINT32                  VarLength;
  UINT32                  Attribute;
  UINT8                   Reserved2[5];
  UINT8                   TableRev;
  UINT16                  VarChecksum;
} IHISI_VATS_OUTPUT_BUFFER;
#pragma pack()

/**

AH=02h, Write DMI Command

**/
typedef enum {
  UpdateCommand = 0x01,
  DeleteCommand = 0x02
} DATS_COMMAND;

#pragma pack(1)
typedef struct {
  UINT8                             Type;
  UINT8                             FieldOffset;
  UINT8                             Length;
  UINT16                            Handle;
  UINT8                             DmiData[1];
} DMI_RECORD_ARRAY_STRUCTURE;

typedef struct {
  UINT8                             Command;  // DATS_COMMAND
  UINT16                            RecordNum;
  DMI_RECORD_ARRAY_STRUCTURE        DmiRecordArray[1];
} VARIABLE_DATA_STRUCTURE;

// INPUT:
//  ESI:Pointer to the beginning of the DATS input buffer.
typedef struct {
  UINT32                            TableId1; // $H2O (IHISI_H2O_SIGNATURE)
  UINT32                            TableId2; // $Var (IHISI_VAR_SIGNATURE)
  UINT32                            TableId3; // $Tbl (IHISI_TBL_SIGNATURE)
  UINT32                            Reserved1;
  UINT32                            VarLength;
  UINT8                             Reserved2[9];
  UINT8                             TableRev;
  UINT16                            VarChecksum;
  VARIABLE_DATA_STRUCTURE           VarData[1];
} DATS_INPUT_BUFFER;
#pragma pack()

/**

AH=03h, Erase DMI area

**/
// INPUT:NA
// OUTPUT:NA

/**

AH=04h, Query all DMI variable

**/
// INPUT:
//  EDI:Pointer to DATS returned data buffer.
//      Data structure same as DATS_INPUT_BUFFER

/**

AH=10h, Get FBTS supported version and FBTS permission.

**/
#define FBTS_VERSION_SIGNATURE                    SIGNATURE_32 ('$', 'V', 'E', 'R')
#define BIOS_CHECK_SIGNATURE                      SIGNATURE_64 ('$', 'B', 'I', 'O', 'S', 'C', 'H', 'K')
#define COMMAND_FILTER_SIGNATURE                  SIGNATURE_64 ('$', 'C', 'M', 'D', 'F', 'I', 'L', 'T')
#define FBTS_ATp_VERSION_SIGNATURE                0x495349484924    // $IHISI

typedef enum {
  AC_PlugOut,
  AC_PlugIn
} AC_STATUS;

typedef enum {
  Dos     = 0x01,
  Windows = 0x02,
  Linux   = 0x03,
  Shell   = 0x04
} TOOL_ENVIRONMENT;

#pragma pack(1)
typedef union {
  struct {
    UINT32  Argument_U    : 1;  // bit 0
    UINT32  Argument_N    : 1;  // bit 1
    UINT32  Argument_S    : 1;  // bit 2
    UINT32  Argument_G    : 1;  // bit 3
    UINT32  Argument_AC   : 1;  // bit 4
    UINT32  Argument_AB   : 1;  // bit 5
    UINT32  Argument_MC   : 1;  // bit 6
    UINT32  Argument_PQ   : 1;  // bit 7
    UINT32  Argument_I    : 1;  // bit 8
    UINT32  Argument_O    : 1;  // bit 9
    UINT32  Argument_LG   : 1;  // bit 10
    UINT32  Argument_B    : 1;  // bit 11
    UINT32  Argument_ALL  : 1;  // bit 12
    UINT32  Argument_E    : 1;  // bit 13
    UINT32  Reserved_1    : 1;  // bit 14
    UINT32  Reserved_2    : 1;  // bit 15
    UINT32  Argument_FM   : 1;  // bit 16
    UINT32  Argument_FV   : 1;  // bit 17
    UINT32  Argument_FD   : 1;  // bit 18
    UINT32  Argument_FE   : 1;  // bit 19
    UINT32  Argument_FL   : 1;  // bit 20
    UINT32  Argument_FP   : 1;  // bit 21
    UINT32  Argument_FN   : 1;  // bit 22
    UINT32  Argument_FT   : 1;  // bit 23
    UINT32  Argument_LF   : 1;  // bit 24
    UINT32  Argument_C    : 1;  // bit 25
    UINT32  Argument_PS   : 1;  // bit 26
    UINT32  Argument_PN   : 1;  // bit 27
    UINT32  Argument_V    : 1;  // bit 28
    UINT32  Reserved_3    : 1;  // bit 29
    UINT32  Disable_Vendor: 1;  // bit 30
    UINT32  Disable_ALL   : 1;  // bit 31
  } Bits;
  UINT32  Data;
} OEM_HELP_1;

typedef union {
  struct {
    UINT32  Argument_RV   : 1;  // bit 0
    UINT32  Argument_WV   : 1;  // bit 1
    UINT32  Argument_WU   : 1;  // bit 2
    UINT32  Reserved_1    : 1;  // bit 3
    UINT32  Argument_1    : 1;  // bit 4
    UINT32  Argument_2    : 1;  // bit 5
    UINT32  Argument_4    : 1;  // bit 6
    UINT32  Argument_8    : 1;  // bit 7
    UINT32  Argument_DI   : 1;  // bit 8
    UINT32  Argument_RB   : 1;  // bit 9
    UINT32  Argument_WB   : 1;  // bit 10
    UINT32  Argument_UU   : 1;  // bit 11
    UINT32  Argument_SE   : 1;  // bit 12
    UINT32  Argument_GU   : 1;  // bit 13
    UINT32  Reserved_2    : 1;  // bit 14
    UINT32  Reserved_3    : 1;  // bit 15
    UINT32  Argument_PMCA : 1;  // bit 16
    UINT32  Reserved_4    : 15; // bit 17~31
  } Bits;
  UINT32  Data;
} OEM_HELP_2;

typedef union {
  struct {
    UINT8 All : 1;
    UINT8 PEI : 1;
    UINT8 CPU_Microcode : 1;
    UINT8 Variable : 1;
    UINT8 DXE : 1;
    UINT8 EC :1;
    UINT8 Logo :1;
    UINT8 Password :1;
  } Bits;
  UINT8  Data;
} FLASH_COMMAND1;

typedef union {
  struct {
    UINT8 OEM_NVS : 1;
    UINT8 OEM_Special_Type : 1;
    UINT8 BIOS_Protect_Region : 1;
    UINT8 UpdateBlock : 1;
    UINT8 UpdateSpecificData : 1;
    UINT8 UpdateNvStorage :1;
    UINT8 UpdateFactoryCopy :1;
    UINT8 ALP :1;
  } Bits;
  UINT8  Data;
} FLASH_COMMAND2;

typedef union {
  struct {
    UINT8 BIOS : 1;
    UINT8 GbE : 1;
    UINT8 ME : 1;
    UINT8 DESC : 1;
    UINT8 PlatformData : 1;
    UINT8 Reserved :2;
    UINT8 ECR :1;
  } Bits;
  UINT8  Data;
} FLASH_INTEL_REGION;

typedef union {
  struct {
    UINT8 EcFile : 1;
    UINT8 FlashEc : 1;
    UINT8 FlashEcWithPercentage : 1;
    UINT8 EcBiosBinary : 1;
    UINT8 FlashEcBiosWithPercentage : 1;
    UINT8 BiosOnlyOfEcBiosBinary :1;
    UINT8 EC_Compare :1;
    UINT8 EC_Verify :1;
  } Bits;
  UINT8  Data;
} FLASH_EC_COMMAND;

typedef union {
  struct {
    UINT8 CheckBatteryPercentage : 1;
    UINT8 NoCheckAC : 1;
    UINT8 CheckACAndBatteryPercentage : 1;
    UINT8 SkipPlatformModelCheck : 1;
    UINT8 Reserved : 2;
    UINT8 RunInManufacturingMode :1;
    UINT8 SkipSecureBootProtectionCheck :1;
  } Bits;
  UINT8  Data;
} CHECK_COMMAND;

typedef union {
  struct {
    UINT8 CheckBatteryPercentage : 1;
    UINT8 NoCheckAC : 1;
    UINT8 CheckModelName : 1;
    UINT8 CheckBIOSVersion : 1;
    UINT8 ConditionalVersionDefinitionForBiosUpdate : 1;
    UINT8 Reserved : 3;
  } Bits;
  UINT8  Data;
} CHECK_SETTING;

typedef union {
  struct {
    UINT8 ShowHelp : 1;
    UINT8 ShowHelpList : 1;
    UINT8 SaveCurrentBiosToFile : 1;
    UINT8 ShowConfirmMessage : 1;
    UINT8 ShowSupportIHISIVersion : 1;
    UINT8 QueryROMProtectionMAP : 1;
    UINT8 QueryRegionMAP :1;
    UINT8 DisableComparison : 1;
  } Bits;
  UINT8  Data;
} GENERAL_COMMAND;

typedef union {
  struct {
    UINT8 DoNotReboot : 1;
    UINT8 Reboot : 1;
    UINT8 Shutdown : 1;
    UINT8 FlashComplete : 1;
    UINT8 OSFlashComplete : 1;
    UINT8 Reserved : 3;
  } Bits;
  UINT8  Data;
} FLASH_COMPLETE_ACTION_COMMAND;

typedef struct {
  UINT64                                Signature;      // $CMDFILT (COMMAND_FILTER_SIGNATURE)
  UINT32                                StructureSize;
  FLASH_COMMAND1                        FlashCommand1;
  FLASH_COMMAND2                        FlashCommand2;
  FLASH_INTEL_REGION                    IntelRegion;
  FLASH_EC_COMMAND                      EcCommand;
  CHECK_COMMAND                         CheckCommand;
  CHECK_SETTING                         CheckSetting;
  GENERAL_COMMAND                       GeneralCommand;
  FLASH_COMPLETE_ACTION_COMMAND         FlashCompleteActionCommand;
} COMMAND_FILTER_TABLE;

// INPUT:
//  EDI:Pointer to returned data buffer with flash tool version
typedef struct {
  UINT32                                Signature;        // $VER (FBTS_VERSION_SIGNATURE)
  UINT8                                 Environment;      // TOOL_ENVIRONMENT
  UINT8                                 Version[4];
  UINT8                                 AtpSignature[6];  // $IHISI (FBTS_ATp_VERSION_SIGNATURE)
  UINT8                                 ATpVersion[3];
} FBTS_TOOLS_VERSION_BUFFER;

// OUTPUT:
//  AH:Permission
#define FBTS_PERMISSION_DENY            0x0000
#define FBTS_PERMISSION_ALLOWED         0x0100

//  CX:
//    CH = Major version.
//    CL = Minor version.

//  EDI:Pointer to platform status structure as below
typedef struct {
  UINT8                                 AcStatus;         // AC_STATUS
  UINT8                                 Battery;
  UINT8                                 Bound;
  OEM_HELP_1                            OemHelp1;         // OEM_HELP_1
  OEM_HELP_2                            OemHelp2;         // OEM_HELP_2
  UINT16                                Customer;
  UINT64                                Signature;        // $BIOSCHK (BIOS_CHECK_SIGNATURE)
  UINT32                                StructureSize;
  UINT8                                 SkipCheck;
  COMMAND_FILTER_TABLE                  CommandFilterTable;
} FBTS_PLATFORM_STATUS_BUFFER;
#pragma pack()

/**

AH=11h, Get platform information.

**/
#define EXTEND_PLATFORM_INPUT_BUFFER_SIGNATURE    SIGNATURE_64 ('$', 'B', 'U', 'F', 'F', 'E', 'R', '_')
#define EXTEND_PLATFORM_OUTPUT_BUFFER_SIGNATURE   SIGNATURE_64 ('$', 'E', 'X', 'T', 'I', 'N', 'F', 'O')
#define EXTEND_OFFSET(structure, field)  (((UINT8 *) (&(((structure *) (0))->field))) - (UINT8 *) (0))

#define MODEL_NAME_SIZE                 0x20
#define MODEL_VERSION_SIZE              0x80
#define MODEL_DATE_SIZE                 0x10
#define MODEL_TIME_SIZE                 0x10

typedef enum {
  PlatformRevisionID  = 0x01,
  BuildDaeTimeID      = 0x02
} DATA_ID_TYPE;

// AH:Ap check capability.
#define AP_DO_NOTHING                   0
#define MODEL_NAME_CHECK                bit(0)
#define MODEL_VERSION_CHECK             bit(1)
#define ALLOW_SAME_VERSION_FILE         bit(2)
#define VERIFY_FILE_CHECKSUM            bit(3)
#define DISABLE_DISPLAY_MODE_NAME       bit(4)
#define DISABLE_DISPLAY_MODE_VERSION    bit(5)
#define DISABLE_READ_COMPARISON         bit(6)
#define ENABLE_EXTENDED_FLAG            bit(7)
// ECX:Ap check capability
#define ALLOW_ALP_COMMAND               bit(0)

typedef union {
  struct {
    UINT8 ModelName : 1;
    UINT8 ModelVersion : 1;
    UINT8 AllowSameVersion : 1;
    UINT8 VerifyFileChecksum : 1;
    UINT8 DisableDisplayModelName : 1;
    UINT8 DisableDisplayModelVersion : 1;
    UINT8 DisableReadComparison : 1;
    UINT8 EnableExtendedFlag : 1;
  } Bits;
  UINT8  Data;
} PLATFORM_INFO_APCHECK;

#pragma pack(1)
typedef struct {
  UINT8                                 DataID;   // DATA_ID_TYPE
  UINT32                                DataSize;
  UINT8                                 Data[1];
} EXTEND_PLATFORM_DATA_ITEM;

// INPUT:
//  EDI:Pointer to returned data buffer.

//  ESI:Buffer for Extend platform information table.
typedef struct {
  UINT64                                Sigature; // $BUFFER_ (EXTEND_PLATFORM_INPUT_BUFFER_SIGNATURE)
  UINT32                                StructureSize;
  UINT8                                 Reserved[1];
} FBTS_EXTEND_PLATFORM_INFO_TABLE_INPUT;

// OUTPUT:
//  AH:PLATFORM_INFO_APCHECK

//  ECX:

//  EDI:Pointer to platform information structure as below.
typedef struct {
  CHAR16                                ModelName [MODEL_NAME_SIZE];
  CHAR16                                ModelVersion [MODEL_VERSION_SIZE];
} FBTS_PLATFORM_INFO_BUFFER;

//  ESI:Extend platform information table
typedef struct {
  UINT64                                Sigature;       // $EXTINFO (EXTEND_PLATFORM_OUTPUT_BUFFER_SIGNATURE)
  UINT32                                DataItemCount;
  EXTEND_PLATFORM_DATA_ITEM             DataItem[1];
} FBTS_EXTEND_PLATFORM_INFO_TABLE_OUTPUT;
#pragma pack()

typedef
EFI_STATUS
(EFIAPI *UPDATE_EXTITEM_FUNCTION) (
  IN OUT EXTEND_PLATFORM_DATA_ITEM     *ExtInfoDataItemPtr
);

typedef struct {
  UINT8                                 DataID;
  UPDATE_EXTITEM_FUNCTION               UpdateExtItemFun;
} UPDATE_EXT_ITEM_FUN_TABLE;

/**

AH=12h, Get Platform ROM map protection.

**/
#define DEFAULT_ROM_MAP_SIZE            40
#define DEFAULT_PRIVATE_MAP_SIZE        40

// Flash map type code.
typedef enum {
  FbtsRomMapPei           = 0x00,   // EFI_FLASH_AREA_RECOVERY_BIOS
  FbtsRomMapCpuMicrocode  = 0x01,   // EFI_FLASH_AREA_CPU_MICROCODE
  FbtsRomMapNVRam         = 0x02,   // EFI_FLASH_AREA_EFI_VARIABLES
  FbtsRomMapDxe           = 0x03,   // DXE EFI_FLASH_AREA_MAIN_BIOS
  FbtsRomMapEc            = 0x04,   // EFI_FLASH_AREA_FV_EC
  FbtsLogo                = 0x05,   //
  FbtsRomMapNvStorage     = 0x06,   // EFI_FLASH_AREA_GUID_DEFINED
  FbtsRomMapFtwBackup     = 0x07,   // EFI_FLASH_AREA_FTW_BACKUP
  FbtsRomMapFtwState      = 0x08,   // EFI_FLASH_AREA_FTW_STATE
  FbtsRomMapSmbiosLog     = 0x09,   // EFI_FLASH_AREA_SMBIOS_LOG
  FbtsRomMapOemData       = 0x0A,   // EFI_FLASH_AREA_OEM_BINARY
  FbtsRomMapGpnv          = 0x0B,   // EFI_FLASH_AREA_GPNV
  FbtsRomMapDmiFru        = 0x0C,   // EFI_FLASH_AREA_DMI_FRU
  FbtsRomMapPalB          = 0x0D,   // EFI_FLASH_AREA_PAL_B
  FbtsRomMapMcaLog        = 0x0E,   // EFI_FLASH_AREA_MCA_LOG
  FbtsRomMapPassword      = 0x0F,   // EFI_FLASH_AREA_RESERVED_03
  FbtsRomMapOemNvs        = 0x10,   // EFI_FLASH_AREA_RESERVED_04
  FbtsRomMapReserved07    = 0x11,   // EFI_FLASH_AREA_RESERVED_07
  FbtsRomMapReserved08    = 0x12,   // EFI_FLASH_AREA_RESERVED_08
  FbtsRomMapReserved09    = 0x13,   //
  FbtsRomMapReserved0A    = 0x14,   // EFI_FLASH_AREA_RESERVED_0A
  FbtsRomMapUnused        = 0x15,   // EFI_FLASH_AREA_UNUSED
  FbtsRomMapFactoryCopy   = 0x16,   // EFI_FLASH_AREA_RESERVED_09
  FbtsRomMapAMDPSPData    = 0x17,   // AMD PSP data region
  FbtsRomMapAMDBootBlockFile  = 0x18,   // AMD boot block file region
  FbtsRomMapUndefined  = 0x19,  // Undefined in BIOS. If there is new type in BIOS, BIOS can define in range 19h-50h.
  FbtsRomMapOemDefined = 0x51,  // To avoid that new defined type in OEM/ODM feature conflicts with original definition in BIOS.Reserved range 51h-0FDh for OEM/ODM.
  FbtsRomMapPrivateMapReserved = 0xFE,   // Reserved for Private Map
  FbtsRomMapEos = 0xFF    // End Of Structure
} FBST_ROM_MAP_CODE;

#pragma pack(1)
typedef struct {
  UINT32                                LinearAddress;
  UINT32                                Size;
} FBTS_PLATFORM_PRIVATE_ROM;

typedef struct {
  UINT8                                 Type;     // FBST_ROM_MAP_CODE
  UINT32                                Address;
  UINT32                                Length;
} FBTS_PLATFORM_ROM_MAP;

// OUTPUT:
//  ESI:Pointer to the returned platform's private map structure
typedef struct {
  FBTS_PLATFORM_PRIVATE_ROM             PlatFormRomMap [DEFAULT_PRIVATE_MAP_SIZE];
} FBTS_PLATFORM_PRIVATE_ROM_BUFFER;

//  EDI:Pointer to the returned platform's ROM map protection structure
typedef struct {
  FBTS_PLATFORM_ROM_MAP                 PlatFormRomMap [DEFAULT_ROM_MAP_SIZE];
} FBTS_PLATFORM_ROM_MAP_BUFFER;
#pragma pack()

/**

AH=13h, Flash part information

**/
#define BLOCK_SIZE_UNIT                 256
#define DEFAULT_BLOCK_MAP_SIZE          0x10
#define BlockSizeEOS                    0xFFFF

typedef enum {
  FlashSize128K = 0x01,
  FlashSize256K,
  FlashSize512K,
  FlashSize1024K,
  FlashSize2048K,
  FlashSize4096K,
  FlashSize8192K,
  FlashSize16384K,
  FlashSpecifiedSize = 0xFF
} FIRMWARE_FLASH_SIZE;

#pragma pack(1)
typedef struct {
  UINT16      BlockSize;   //Unit: 1 = 256 bytes (BLOCK_SIZE_UNIT)
                           //      0FFFFh=EOS (BlockSizeEOS)
  UINT16      Mutiple;     //flash how many times
} FBTS_FD_BLOCK_MAP;

// INPUT:
//  CL:
typedef enum {
  FromAP,
  SPI_Part,
  NonSPI_Part
} FLASH_PART_INTERFACE;

// OUTPUT:
//  EDI:Pointer to flash part information structure as below.
typedef struct {
  UINT8                                 Size;             //FIRMWARE_FLASH_SIZE
  CHAR8                                 VendorName [31];
  CHAR8                                 DeviceName [32];
  UINT32                                Id;
  UINT32                                SpecifiedSize;
} FBTS_FLASH_DEVICE;

//  ESI:Pointer to flash part block map structure as below.
typedef struct {
  FBTS_FD_BLOCK_MAP                     BlockMap [DEFAULT_BLOCK_MAP_SIZE];
} FBTS_FLASH_PART_BLOCK_MAP_BUFFER;
#pragma pack()

/**

AH=14h, FBTS Read.

**/
// INPUT:
//  ECX:Size to read.

//  ESI:Pointer to returned data buffer. Size in ECX.

//  EDI:Target linear address to read.

// OUTPUT:
//  ESI:Pointer to returned data buffe

/**

AH=15h, FBTS Write.

**/
// INPUT:
//  ECX:Size to write.

//  ESI:Pointer to data buffer for write. Size in ECX.

//  EDI:Target linear address to write.

/**

AH=16h, FBTS Flash complete

**/
#define FLASH_COMPLETE_STATUS_SIGNATURE           SIGNATURE_64 ('$', 'F', 'I', 'N', 'I', 'S', 'H', 'I')

typedef enum {
  ApTerminated = 0,
  NormalFlash,
  PartialFlash,
} FLASH_COMPLETE_STATUS;

#pragma pack(1)
// INPUT:
//  CH:
#define FBTS_FLASH_COMPLETE_CH_DO_NOTHING         0x00
#define FBTS_FLASH_COMPLETE_CH_CLEAN_CMOS         0x01

//  CL:
typedef enum {
  FlashCompleteDoNothing        = 0x00,
  FlashCompleteShutdown         = 0x01,
  FlashCompleteReboot           = 0x02
} FLASH_COMPLETE_APREQUEST;

//  ESI: Flash complete status
typedef struct {
  UINT64                    Signature;      // $FINISHI (FLASH_COMPLETE_STATUS_SIGNATURE)
  UINT32                    StructureSize;
  UINT8                     CompleteStatus; // FLASH_COMPLETE_STATUS
} FBTS_FLASH_COMPLETE_STATUS;
#pragma pack()

/**

AH=17h, Get external ROM file and platform region table (External ROM map)
  This ROM map is indicated external ROM map, such as ME, GBE and DESC etc.
  If internal BIOS map is defined in function AH=1Eh.

**/
#define EXTERNAL_ACCESS_AVAILABLE     bit(0)
#define APP_NOT_REFER_BIT0            bit(1)


// Region type.
typedef enum {
  DESC                    = 0x00,
  BIOS                    = 0x01,
  ME                      = 0x02,
  GBE                     = 0x03,
  PLATFORM_DATA           = 0x04,
  ME_SUBREGION_IGNITION   = 0x20,
  RegionTypeEOS           = 0xFF
} REGION_TYPE;

#pragma pack(1)
typedef struct {
  UINT8         Type;   // REGION_TYPE
  UINT32        Offset;
  UINT32        Size;
  UINT8         Access;
} REGION_STRUCTURE;

// INPUT:
//  ECX: Size to input.

//  ESI:Pointer to input file data buffer. Size in ECX

//  EDI:Pointer to returned data buffer.

// OUTPUT:
//  EDI:Pointer to region information structure
typedef struct {
  UINT8       PlatformRegion[256];
  UINT8       RomFileRegion [256];
} FBTS_REGION_INFORMATION_STRUCTURE;
#pragma pack()

/**

AH=18h, Secure flash BIOS

**/
// INPUT:
//  ECX:Size to read.

//  ESI:Pointer to input file data buffer. Size in ECX.

//  EDI:Pointer to returned data buffer.

/**

AH=19h, OEM/ODM customization-1

**/
#define END_OF_POINT        0xFF
#define END_OF_DATA         0x00

// INPUT:
//  CL:
#define CUSTOMIZATION1_RESERVED   0
#define BEFORE_CONFIRM_MESSAGE    1

//  EDI:A pointer point will include specific parameter string (ASCII) and send BIOS by the pointer buffer.

// OUTPUT:
//  EDI:A pointer point will include specific parameter string (ASCII) and send back application by the pointer buffer

/**

AH=1Ah, OEM/ODM customization-2

**/
// INPUT:
//  CL:
#define BEFORE_WRITE_PROCESS  1
#define AFTER_WRITE_PROCESS   2

//  CH:
#define FLASH_SUCCESS         1
#define FLASH_FAILURE         2

/**

AH=1Bh, Skip module check allows and binaryfile transmissions

**/
// INPUT:
//  ECX:Size to input

//  ESI:Pointer to input file data buffer. Size in ECX.

//  EDI:Target linear address to read

/**

AH=1Ch, Get AT-p information

**/
#pragma pack(1)
// OUTPUT:
//  CL: AT-p status
typedef union {
  struct {
    UINT8 ATp_enrolled : 1;
    UINT8 ATp_NotStolen : 1;
    UINT8 ATp_Enough_Time : 1;
    UINT8 Reserved : 3;
    UINT8 Flashable : 1;
    UINT8 ATp_Support : 1;
  } Bits;
  UINT8  Data;
} FBTS_ATP_INFORMATION_STRUCTURE;

#pragma pack()

/**

AH=1Dh, Pass platform.ini settings

**/
#define END_OF_PLATFORM_INI_STRUCTURE  0x55AA

#pragma pack(1)
typedef struct {
  UINT32 SectionNamePtr;
  UINT32 KeyNamePtr;
  UINT32 ValueBufferPtr;
} PLATFORM_INI_SETTING_ITEM;

// INPUT:
//  ESI:Pointer to input data buffer.
typedef struct {
  PLATFORM_INI_SETTING_ITEM     SettingItem[1];
} FBTS_PASS_PLATFORM_INI_SETTING_STRUCTURE;
#pragma pack()

/**

AH=1Eh, Get whole BIOS ROM map (Internal BIOS map)
  This ROM map is indicated internal ROM map, such asPEI, DXE and NVStorage etc.
  If external ROM map is defined in function AH=17h.

**/
#define END_OF_WHOLE_BIOS_ROM_MAP  0xFF
#define COMPARE_REGION    bit(0)
#define SINGLE_ROM_MAP    bit(1)
#define PROTECT_REGION    bit(2)
#define PRIVATE_REGION    bit(3)

typedef union {
  struct {
    UINT32 CompareRegion :1;
    UINT32 SingleRomMap : 1;
    UINT32 ProtectRegion : 1;
    UINT32 PrivateRegion : 1;
    UINT32 Reserved : 28;
  } Bits;
  UINT32  Data;
} BIOS_ROM_MAP_ATTRIBUTE;

#pragma pack(1)
// OUTPUT:
//  EDI:Pointer to the returned platform's whole ROM map structure for application can get whole ROM map with attribute.
typedef struct {
  UINT8                         Type;       // FBST_ROM_MAP_CODE
  UINT32                        Address;
  UINT32                        Size;
  UINT32                        Attribute;  // BIOS_ROM_MAP_ATTRIBUTE
} FBTS_INTERNAL_BIOS_ROM_MAP;
#pragma pack()

/**

AH=1Fh, AP Hook Point for BIOS
  AP calling this IHISI to let BIOS know which step is running now.
  BIOS can base on it to do specific hook such as EC idle and weak up.

**/
// INPUT:
//  ECX: AP hook action type.
typedef enum {
  BeforeApTerminate       = 0x00,
  AfterApStart            = 0x01,
  BeforeReadRom           = 0x02,
  BeforeWriteRom          = 0x03,
  BeforeWriteEc           = 0x04,
  BeforeDialogPopup       = 0x05,
  AfterDialogClose        = 0x06
} AP_HOOK_ACTION_TYPE;

/**

AH=48h, OEM capsule secure flash.

**/
#define OEM_SECURE_FLASH_EXTEND_FLAG_SIGNATURE      SIGNATURE_32 ('$', 'E', 'X', 'T')

#define IMAGE_BLOCK_BUFFERSIZE_FOR_COMPRESS         0x10000     // 64K

#pragma pack(1)

#define FROME_IHISI    0    //Update secure capsule via IHISI. (Default)
#define FROME_ESP      1    //ESP(EFI System Partition)

typedef union {
  struct {
    UINT8 SecureCapsuleOrigin :1;   // (FROME_IHISI / FROME_ESP)
    UINT8 Reserved : 7;
  } Bits;
  UINT8  Data;
} FBTS_EXTEND_FLAG;

typedef struct {
  UINT64                        ImageBlockSize;
  UINT64                        ImageBlockAddress;
} IMAGE_BLOCK_DATA_ITEM_STRUCTURE;

// INPUT:
//  ECX:Whole size of isFlash.bin image

//  ESI:Pointer to input partial isFlash.bin image block structure.
typedef struct {
  UINT32                              BlockNum;
  UINT32                              Reserve;
  IMAGE_BLOCK_DATA_ITEM_STRUCTURE     BlockDataItem[1];
} FBTS_SECURE_FLASH_IMAGE_BLOCK_STRUCTURE;

//  EDI:Extend Flag Structure:
typedef struct {
  UINT32                        Signature;    // $EXT (OEM_SECURE_FLASH_EXTEND_FLAG_SIGNATURE)
  UINT32                        StructureSize;
  FBTS_EXTEND_FLAG              Flag;
} FBTS_EXTEND_FLAG_STRUCTURE;
#pragma pack()

// OUTPUT:
//  ECX:BIOS support secure flash type.
typedef enum {
  WINFLASH_DO_S3          = 0x00,
  WINFLASH_DO_REBOOT      = 0x01,
  WINFLASH_DO_SHUT_DOWN   = 0x02,
  WINFLASH_DO_NOTHING     = 0x03
} WINFLASH_ACTION_TYPE;

/**

AH=4Dh, Passing Image to BIOS for specific check (BIOS is master to query image from tool via the interface)

**/
// INPUT:
//  ECX:Image block size, max size is 64KB (0x10000).
#define QUERY_ACTION    0x00
#define MAX_IMAGE_SIZE  0x10000

//  EDI:Pointer to input image block. Size is defined in ECX.

// OUTPUT:
//  ECX:Request image block size, max size is 64KB (0x10000).

//  ESI:The result of each verification feature.
#define VERIFICATION_CHECK_FAIL    bit(0)

//  EDI:Request next image block address

/**

AH=4Eh, Query BIOS to get runtime buffer address and size
  AP will try to call this interface to get a runtimebuffer as buffer to communicate with BIOS to pass BIOS
  image or other purpose. For example, Linux flash tool, because of tool has to use driver to allocate
  physical address and driver has to recompile for each Linux kernel version, this is inconvenience to end
  user.

**/
// OUTPUT:
//  ECX:Return the buffer size. It!|s allocated by BIOS, size has to >= flash part size or flash image size.

//  ESI:Return the buffer address. It!|s allocated by BIOS, it is Reserved type. The address is under 0xFFFFFFFF.

/**

AH=50h, Passing Image to BIOS for specificcheck (Tool is master to pass image to BIOS via the interface.)

**/
// INPUT:
//  ECX:
#define PASS_IMAGE_BLOCK_UINT         0x10000  //64K
//    1. When EDI (input register) is 0, ECX (input register) represents Block Number of the image size. (Block Number = Total Image size (bytes) / 64KB).
//    2. When EDI (input register) is NOT 0, it means starting to pass image data with max size 64KB for each transfer

//  EDI: Pointer for a new block of input image data.

// OUTPUT:
//  ECX:
#define CHECK_SUM_SUCCESS    0x00
#define CHECK_SUM_FAILURE    0x01
typedef union {
  struct {
    UINT32 CheckSumStatus : 1;      // (CHECK_SUM_SUCCESS / CHECK_SUM_FAILURE)
    UINT32 Reserved : 31;
  } Bits;
  UINT32 Data;
} IMAGE_CHECK_SUM_VERIFICATION;

/**

AH=51h, Inform BIOS to write image data to SPI ROM

**/
// INPUT:
//  ECX:Size to write.

//  ESI:Image Offset start address.

//  EDI:Target linear address to write

// OUTPUT:
//  CL:
#define WAIT_VERIFY_BIT     0x01

/**

AH=52h, H2O UEFI variable edit SMI service - Confirm the legality of the variables.
  Use case:
    It can confirm the legality of every variables (Setup Variable, Console Redirection Variable and other
    OEM Variable) before setting it.

**/
#pragma pack(1)
typedef struct {
  UINT16                        CallbackID;
  UINT16                        CallbackResult;
} RESULT_LIST_CALL_BACK_DATA;

typedef struct {
  UINT16                        CallbackID;
  CHAR8                         CallbackString[1];
} CALLBACK_LIST_CALL_BACK_DATA;

// INPUT:
//  ECX:Pointer to the beginning of the RESULT_LIST Input buffer.

//  EDI:Pointer to the beginning of the CALLBACK_LIST Input buffer.
typedef struct {
  UINT32                            BufferSize;
  UINT32                            ListCount;
  CALLBACK_LIST_CALL_BACK_DATA      CallbackData[1];
} FBTS_CALLBACK_LIST_STRUCTURE;

//  ESI:Point to the beginning of variable buffer
typedef struct {
  UINT32                        BufferSize;
  EFI_GUID                      VariableGuid;
  UINT32                        DataSize;
  CHAR16                        VariableName[1];
} FBTS_VARIABLE_BUFFER_STRUCTURE;

// OUTPUT:
//  ECX:Pointer to the beginning of the RESULT_LIST Output buffer.
typedef struct {
  UINT32                        BufferSize;
  UINT32                        ListCount;
  RESULT_LIST_CALL_BACK_DATA    CallbackData[1];
} FBTS_RESULT_LIST_STRUCTURE;
#pragma pack()

/**

AH=53h, H2O UEFI Variable Edit SMI service- Boot information-related services.
  Use case:
    It can Get/Set boot option information and boot option order.

**/
#pragma pack(1)
typedef struct {
  UINT16                        BootOptionNum;
  UINT16                        BootOptionType;
} FBTS_BOOT_OPTION_DATA;

// INPUT:
//  ECX:
typedef enum {
  GetBootOptionInformation = 0x01,
  SetBootOptionInformation,
  GetBootTypeInformation,
  SetBootTypInformatione,
  GetBootTypeName,
} IHISI_CURRENT_BOOT_INFORMATION_TYPE;

//  EDI:Pointer to the beginning of the boot information Input buffer.
// (GetBootOptionInformation/SetBootOptionInformation)boot option information Structure .
typedef struct {
  UINT32                        BufferSize;
  UINT16                        Count;
  UINT8                         BootOrderByType;
  UINT8                         EfiFirst;
  UINT8                         Reserve[8];
  FBTS_BOOT_OPTION_DATA         BootOption[1];
} FBTS_BOOT_OPTION_INFORMATION_STRUCTURE;

// (GetBootTypeInformation/SetBootTypInformatione)boot type information Structure
typedef struct {
  UINT32                        BufferSize;
  UINT16                        Count;
  UINT16                        BootType[1];
} FBTS_BOOT_TYPE_INFORMATION_STRUCTURE;

// (GetBootTypeName)boot type Name Structure.
typedef struct {
  UINT32                        BufferSize;
  UINT16                        BootType;
  CHAR16                        BootTypeName[1];
} FBTS_BOOT_TYPE_NAME_STRUCTURE;
#pragma pack()

/**

AH=49h,Common Region Data Communication

**/
#define COMMON_REGION_INPUT_SIGNATURE   SIGNATURE_32 ('$', 'R', 'D', 'I')
#define COMMON_REGION_OUTPUT_SIGNATURE  SIGNATURE_32 ('$', 'C', 'D', 'O')

// Data type definition
typedef enum {
  FACTORY_COPY_EVENT = 0x01,
  FACTORY_COPY_READ,
  FACTORY_COPY_RESTORE_WITH_CLEARING_ALL_SETTINGS,
  FACTORY_COPY_RESTORE_WITH_RESERVING_OTHER_SETTINGS,
} COMMON_REGION_DATA_TYPE;

//
// Output Data size definition
//
typedef enum {
  COMMON_REGION_SIZE_64K,                 // 64k
  COMMON_REGION_SIZE_128K,                // 128k
  COMMON_REGION_SIZE_256K,                // 256k
  COMMON_REGION_SIZE_512K,                // 512k
  COMMON_REGION_SIZE_1024K,               // 1024k
  COMMON_REGION_SIZE_2048K,               // 2048k
  COMMON_REGION_SIZE_4096K,               // 4096k
  COMMON_REGION_SIZE_8192K,               // 8192k
  COMMON_REGION_REPORT_READ_SIZE  = 0xFD, // BIOS send Data size is not contained in above 00~07h definition.
                                          // The data size should refer to "Physical data size" filed which defines in offset 0Ah.
                                          // (BIOS will send data to application for extra data read function 47h if field is FDh)
  COMMON_REGION_SKIP_SIZE_CHECK   = 0xFE, // BIOS acknowledge skip size check request from caller;
                                          // BIOS will also skip flash data size check too.
                                          // (Application will send data to BIOS for extra data write function 42h if field is FEh)
  COMMON_REGION_REPORT_WRITE_SIZE = 0xFF, // BIOS report it allows the physical size to caller,
                                          // the physical data size should refer to "Physical data size" filed which define in offset 0Ah.
                                          // (Application will send data to BIOS for extra data write function 42h if field is FFh)
} COMMON_REGION_DATA_SIZE;

// Block size definition
typedef enum {
  COMMON_REGION_BLOCK_SIZE_4K,
  COMMON_REGION_BLOCK_SIZE_64K,
  COMMON_REGION_BLOCK_SIZE_SPECIFIC = 0xFF
} COMMON_REGION_BLOCK_SIZE;

// Status definition
#define UNSUPPORTED_TYPE        bit(15)

#pragma pack (1)
// INPUT:
//  ECX: Pointer to AP communication data buffer.
typedef struct {
  UINT32                        Signature;    // $RDI (COMMON_REGION_INPUT_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         DataType;     // COMMON_REGION_DATA_TYPE
  UINT8                         DataSize;     // COMMON_REGION_DATA_SIZE
  UINT32                        PhysicalDataSize;
} INPUT_DATA_STRUCTURE;

// OUTPUT:
//  ECX: Pointer to BIOS communication data buffer
typedef struct {
  UINT32                        Signature;    // $CDO (COMMON_REGION_OUTPUT_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         BlockSize;    // COMMON_REGION_BLOCK_SIZE
  UINT8                         DataSize;     // COMMON_REGION_DATA_SIZE
  UINT32                        PysicalDataSize;
  UINT16                        Status;       // UNSUPPORTED_TYPE
} OUTPUT_DATA_STRUCTURE;

#pragma pack ()

/**

AH=4Ah, Common Region Data Write
  Function 49h and 4Ah (or 4Bh) are pairs. The function 49h has to be called before calling into function
  50h.

**/
// INPUT:
//  CL:
typedef enum {
  DoNothing,                  // 00h=Do nothing
  WindowsReboot,              // 01h=Windows reboot
  WindowsShutdown,            // 02h=Windows shutdown
  DosReboot,                  // 03h=DOS reboot
  DosShutdown,                // 04h=DOS shutdown
  ContinueToFlash = 0xEF,    // Continue to flash
} SHUTDOWN_MODE_TABLE;

//  ESI:Pointer to input file data buffer. Size in EDI.

//  EDI:Size to input.

//  CH:Target offset to write.
/**

AH=4Bh, Common Region Data Read
  Function 49h and 4Bh (or 4Ah) are pairs.
  The function 49h has to be called before calling into function 51h

**/
// OUTPUT:
//  ESI:Pointer to return data buffer. Size in EDI.

//  EDI:Size to output.

/**

AH=20h, FETS Write

**/
// INPUT:
//  CL:
typedef enum {
  EcFlashDoNothing       = 0x00,    // Do nothing
  EcFlashDosReboot       = 0x01,    // DOS Shutdown
  EcFlashOSShutdown      = 0x02,    // OS Shutdown
  EcFlashOSReboot        = 0x04,    // OS Reboot
  EcFlashDoshutdown      = 0x05,    // DOS Reboot
  EcFlashContinueToFlash = 0xEF     // Continue to flash
} IHISI_EC_FLASH_ACTION_AFTER_FLASHING;

//  ESI:Pointer to input file data buffer. Size in EDI

//  EDI:Size to input.

//  CH:Target offset to write

/**

AH=21h, Get EC part information.

**/
#define EC_PARTSIZE_SIGNATURE             SIGNATURE_64 ('$', 'B', 'U', 'F','F', 'E', 'R', '_')
#define MULTIPLE_EC_PARTSIZE_SIGNATURE    SIGNATURE_64 ('$', 'E', 'C', 'I','N', 'F', 'O', '_')

#define MAX_EC_PART_COUNT 10

#pragma pack(1)
// INPUT:
//  CH: EC_PART_SIZE
typedef enum {
  EC64K,
  EC128K,
  EC256K,
  EC512K,
  EC1024K,
  EC2048K,
  REPORT_FROM_ESI = 0xFF
} EC_PART_SIZE;

//  ESI:Buffer for EC part size table.
typedef struct {
  UINT64                  Signature;    // $BUFFER_ (EC_PARTSIZE_SIGNATURE)
  UINT32                  Size;
  UINT32                  Reserved[MAX_EC_PART_COUNT];
} FETS_EC_PART_SIZE_STRUCTURE_INPUT;

// OUTPUT:
//  CL:EC_PART_SIZE

//  ESI:EC part size table for multiple EC part on single platform.
typedef struct {
  UINT64                  Signature;    //$ECINFO_ (MULTIPLE_EC_PARTSIZE_SIGNATURE)
  UINT32                  EcPartCount;
  UINT32                  EcPartSize[MAX_EC_PART_COUNT];
} FETS_EC_PART_SIZE_STRUCTURE_OUTPUT;
#pragma pack()

/**

AH=22h, FETS Read

**/
#define READ_EC_SIGNATURE           SIGNATURE_32 ('$', 'E', 'C', 'I')
#define FETS_READ_BUFFER_SIZE       0x10000

// Function type
typedef enum {
  QueryEcActualSize,
  ReadEc
} EC_READ_FUNCTION_TYPE;

// Data size
typedef enum {
  EcReadSize4K,
  EcReadSize64K
} EC_READ_DATA_SIZE;

#pragma pack(1)
// INPUT:
//  ECX:
typedef struct {
  UINT32                        Signature;      // $ECI (READ_EC_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         FunctionType;   // EC_READ_FUNCTION_TYPE
  UINT8                         ReadSize;       // EC_READ_DATA_SIZE
  UINT8                         IndexOfEcPart;
} FETS_READ_EC_STRUCTURE_INPUT;

//  ESI: FETS_READ_BUFFER_SIZE
//    Pointer to returned data buffer. Buffer size is 64K.

// OUTPUT:
//  ECX:
//    When Function_Type = 00h-> EC actual size.
//    When Function_Type = 01h-> Actual read size.

//  ESI:Pointer to returned data buffer
#pragma pack()

/**

AH=30h, Calpella OEM ME part information.

**/
#define ME_PART_AP_COMMUNICATION_SIGNATURE         SIGNATURE_32 ('$', 'S', 'P', 'S')
#define ME_PART_BIOS_COMMUNICATION_SIGNATURE       SIGNATURE_32 ('$', 'S', 'P', 'W')

#define VERSION_SAME          0x01
#define VERSION_NOT_SAME      0x00
#define CHECK_VERSION         0x01
#define NOT_CHECK_VERSION     0x00

#pragma pack(1)
// INPUT:
//  CH:
typedef enum {
  MEPart64K,
  MEPart128K,
  MEPart256K,
  MEPart512K,
  MEPart1024K,
  MEPart2048K,
  MEPart4096K,
  MEPart8192K
} ME_PART_SIZE;

//  EDI:Pointer to AP communication data buffer.
typedef struct {
  UINT32                        Signature;      // $SPS (ME_PART_AP_COMMUNICATION_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         Version[8];
} FMTS_AP_COMMUNICATION_STRUCTURE;

// OUTPUT:
//  CL: ME_PART_SIZE

//  EDI:Pointer to BIOS communication data buffer
typedef struct {
  UINT32                        Signature;      // $SPW (ME_PART_BIOS_COMMUNICATION_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         VersionSame;    // (VERSION_SAME / VERSION_NOT_SAME)
  UINT8                         VersionCheck;   // (CHECK_VERSION / NOT_CHECK_VERSION)
} FMTS_BIOS_COMMUNICATION_STRUCTURE;
#pragma pack()

/**

AH=31h, FMTS Write

**/
// INPUT:
//  CL:Same as SHUTDOWN_MODE_TABLE

//  ESI:Pointer to input file data buffer. Size in EDI.

//  EDI:Size to input.

//  CH:Target offset to write.

/**

AH=40h, Get OEM1 information.

**/
// INPUT:
//  CH:
#define EFI_FLASH                 bit(0)
#define CLEAN_UEFI_OS_VARIABLE    bit(1)
typedef union {
  struct {
    UINT8 EfiFlash : 1;
    UINT8 CleanOSVariable : 1;
    UINT8 Reserve : 6;
  } Bits;
  UINT8 Data;
} GET_OEM1_INFO_INPUT_CH;

// OUTPUT:
//  CL:
#define BIOS_SIGN_CHECK           bit(0)
#define CHECK_EFI_FLASH_SUPPORT   bit(1)

typedef union {
  struct {
    UINT8 BiosSignChek : 1;
    UINT8 CheckEfiflashSupport : 1;
    UINT8 Reserve : 6;
  } Bits;
  UINT8 Data;
} GET_OEM1_INFO_OUTPUT_CL;

/**

AH=46h, Get OEM1 information extending

**/

#define BIOS_COMMUNICATION_DATA_SIGNATURE    SIGNATURE_32 ('$', 'B', 'B', 'O')
#define BOOT_BLOCK_STATUS_SIGNATURE          SIGNATURE_32 ('$', 'B', 'B', 'S')

#pragma pack(1)
typedef struct {
  UINT32                        BootBlockAddress;
  UINT32                        BootBlockSize;
} BOOT_BLOCK_DATA_STRUCTURE;

// INPUT:
//  ESI:Pointer to BIOS communication data structure in output.

//  EDI:Pointers to Boot block status structure in output.

// OUTPUT:
//  ECX:Boot block sign check
typedef enum {
  BBL1andBBL2 = 0x00,
  BBL3        = 0x08
} BBL_LEVEL;

typedef union {
  struct {
    UINT32 BootBlockSignCheck : 1;  // Bit0: Boot block sign check
    UINT32 BootBlockStatus : 1;     // Bit1~BIT2: Boot block function status
    UINT32 EcLock : 1;              // Bit3: EC lock function status
    UINT32 BBL_Level : 4;           // Bit4~Bit7: BBL level support (BBL_LEVEL)
    UINT32 MPM_Status : 1;          // Bit8: MPM status
    UINT32 Reserved : 23;           // Bit9~31: Reserved.
  } Bits;
  UINT32 Data;
} GET_OEM1_INFO_EXT_OUTPUT_ECX;

//  ESI:Pointer to BIOS communication data structure
typedef struct {
  UINT32                        Signature;      // $BBO (BIOS_COMMUNICATION_DATA_SIGNATURE)
  UINT32                        StructureSize;
  CHAR8                         ModelVersion[255];
  UINT8                         SignCheckDataNum;
  EFI_GUID                      SignCheckDataGUID;
} OEM1_BIOS_COMMUNICATION_STRUCTURE;

//  EDI:Pointers to Boot block status structure in output
typedef struct {
  UINT32                        Signature;      // $BBS (BOOT_BLOCK_STATUS_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         BootBlockNum;
  BOOT_BLOCK_DATA_STRUCTURE     BootBlockData[1];
} OEM1_BOOT_BLOCK_STATUS_STRUCTURE;
#pragma pack()

/**

AH=4Ch, OEM secure flash

**/
#pragma pack(1)
typedef struct {
  UINT64                        ImageBlockSize;
  UINT64                        ImageBlockAddress;
} IMAGE_BLOCK_DATA_STRUCTURE;

// INPUT:
//  ECX:Whole size of secure image

//  ESI:Pointer to input partial secure image block structure.
typedef struct {
  UINT32                        ImageBlockNum;
  UINT32                        Reserved;
  IMAGE_BLOCK_DATA_STRUCTURE    ImageBlockData[1];
} OEM_SECURE_FLASH_STRUCTURE;
#pragma pack()
//  EDI:
#define FLASH_AFTER_CERTIFICATE_VALIDATEION_SUCCESS   0
#define ONLY_VERIFY_CERTIFICATE                       1

// OUTPUT:
//  ECX:Same as WINFLASH_ACTION_TYPE

//  AH:
typedef union {
  struct {
    UINT8 CertificateValidationResul : 1;   // Bit 0: Certificate validation result.
                                            //      0 - Certificate validation fail.
                                            //      1 - Certificate validation success.
    UINT8 ContinueFlash : 1;                // Bit 1: Should AP continue to flash when certificatevalidation failed. Only be referenced when Bit 0 is 0.
                                            //      0 - Do not flash.
                                            //      1 - Continue to flash with non-secure (normal) way.
    UINT8 UpdateResult : 1;                 // Bit 2: Secure image update result.
                                            //      0 - Flash failed.
                                            //      1 - Flash success.
    UINT8 Reserved : 5;                     // Bit3~7: Reserved.
  } Bits;
  UINT8 Data;
} OEM_SECURE_FLASH_OUTPUT_AH;

/**

AH=41h, OEM Extra Data Communication

**/
#define AP_COMMUNICATION_SIGNATURE              SIGNATURE_32 ('$', 'E', 'D', 'I')
#define BIOS_COMMUNICATION_SIGNATURE            SIGNATURE_32 ('$', 'E', 'D', 'O')
#define AP_COMMUNICATION_5E_TYPE_SIGNATURE          SIGNATURE_32 ('$', '5', 'E', 'I')
#define AP_COMMUNICATION_5F_TYPE_SIGNATURE          SIGNATURE_32 ('$', '5', 'F', 'I')
#define BIOS_COMMUNICATION_5D_TYPE_SIGNATURE        SIGNATURE_32 ('$', '5', 'D', 'O')

#define AP_COMMUNICATION_EXTENDING_IEXSIGN_SIGNATURE  SIGNATURE_64 ('$', 'I', 'E', 'X', 'S', 'I', 'G', 'N')
#define AP_COMMUNICATION_EXTENDING_OEXSIGN_SIGNATURE  SIGNATURE_64 ('$', 'O', 'E', 'X', 'S', 'I', 'G', 'N')

//
// Error return status:
//
#define ERROR_RETURE_RETURN_CODE_VALID        bit(13)
#define ERROR_RETURE_EXT_ERROR_RETURN_VALID   bit(14)
#define ERROR_RETURE_NOT_SUPPORT              bit(15)

// Data type 50h, 51h, 52h, 53h:
#define ERROR_RETURE_MSDM_FAIL                bit(0)
#define ERROR_RETURE_ERASE_FAIL               bit(1)
#define ERROR_RETURE_OAKEY_LOCK               bit(2)
#define ERROR_RETURE_OA30_NOT_EXIST           bit(3)

typedef union {
  struct {
    UINT16 MSDMTableCheck : 1;         // Bit 0 : BIOS ACPI MSDM Table Check (0-Pass; 1-Fail)
    UINT16 EraseActionResult : 1;      // Bit 1 : Erase action result (0-Pass; 1-Fail)
    UINT16 CheckOAkeyLockStatus : 1;  // Bit 2 : Check OA key (MSDM table) lock status:(0-Unlock; 1-Lock)
    UINT16 OA30KeyExist : 1;          // Bit 3 : OA3.0 Key exist:(0-Exist; 1-Not exist)
    UINT16 Reserved : 9;              // Bit 4~12 : Set 0 (Reserved)
    UINT16 ReturnCodeVaild : 1;       // Bit 13:
                                      //        0-Default;
                                      //        1-The return code in 10h is valid.
    UINT16 ExtendErrorReturnVaild : 1;// Bit 14:
                                      //        0-Default;
                                      //        1-Refer to expending 30h (Extending Error return)of utput register ECX if bit14 is present.
    UINT16 NotSupport : 1;            // Bit 15:
                                      //        0-Success;
                                      //        1-Not support Data_Type. (The IHISI 41 is implement in BIOS but cannot recognize the new type)
  } Bits;
  UINT16 Data;
} ERRORY_RETURN_TYPE_50_51_52_53;

// Data type 54h:
#define ERROR_RETURE_SIZE_NOT_MATCH           bit(0)

typedef union {
  struct {
    UINT16 SizeCheckResult  : 1;      // Bit 0 : Size check result (0-Match; 1-Not match)
    UINT16 Reserved : 12;             // Bit 1~12 : Set 0 (Reserved)
    UINT16 ReturnCodeVaild : 1;       // Bit 13:
                                      //        0-Default;
                                      //        1-The return code in 10h is valid.
    UINT16 ExtendErrorReturnVaild : 1;// Bit 14:
                                      //        0-Default;
                                      //        1-Refer to expending 30h (Extending Error return)of utput register ECX if bit14 is present.
    UINT16 NotSupport : 1;            // Bit 15:
                                      //        0-Success;
                                      //        1-Not support Data_Type. (The IHISI 41 is implement in BIOS but cannot recognize the new type)
  } Bits;
  UINT16 Data;
} ERRORY_RETURN_TYPE_54;

// Data type 55h:
#define ERROR_RETURE_SIGN_NOT_MATCH           bit(0)

typedef union {
  struct {
    UINT16 SignCheckResult  : 1;      // Bit 0 : Sign check result (0-Match; 1-Not match)
    UINT16 Reserved : 12;             // Bit 1~12 : Set 0 (Reserved)
    UINT16 ReturnCodeVaild : 1;       // Bit 13:
                                      //        0-Default;
                                      //        1-The return code in 10h is valid.
    UINT16 ExtendErrorReturnVaild : 1;// Bit 14:
                                      //        0-Default;
                                      //        1-Refer to expending 30h (Extending Error return)of utput register ECX if bit14 is present.
    UINT16 NotSupport : 1;            // Bit 15:
                                      //        0-Success;
                                      //        1-Not support Data_Type. (The IHISI 41 is implement in BIOS but cannot recognize the new type)
  } Bits;
  UINT16 Data;
} ERRORY_RETURN_TYPE_55;

// Data type 56h:
#define ERROR_RETURE_PK_NOT_ENROLL            bit(0)

typedef union {
  struct {
    UINT16 CheckPKStatus  : 1;        // Bit 0 : Check PK existing status before cleaning secure key
                                      //      0 - It's user mode (PK is enrolled)
                                      //      1 - It's setup mode (PK is not enrolled)
    UINT16 Reserved : 12;             // Bit 1~12 : Set 0 (Reserved)
    UINT16 ReturnCodeVaild : 1;       // Bit 13:
                                      //        0-Default;
                                      //        1-The return code in 10h is valid.
    UINT16 ExtendErrorReturnVaild : 1;// Bit 14:
                                      //        0-Default;
                                      //        1-Refer to expending 30h (Extending Error return)of utput register ECX if bit14 is present.
    UINT16 NotSupport : 1;            // Bit 15:
                                      //        0-Success;
                                      //        1-Not support Data_Type. (The IHISI 41 is implement in BIOS but cannot recognize the new type)
  } Bits;
  UINT16 Data;
} ERRORY_RETURN_TYPE_56;

// Data type 57h:
#define ERROR_RETURE_PK_ENROLL                bit(0)
typedef union {
  struct {
    UINT16 CheckPKStatus  : 1;        // Bit 0 : Check PK existing status before cleaning secure key
                                      //      0 - It is setup mode (PK is not enrolled).
                                      //      1 - It is user mode (PK is enrolled).
    UINT16 Reserved : 12;             // Bit 1~12 : Set 0 (Reserved)
    UINT16 ReturnCodeVaild : 1;       // Bit 13:
                                      //        0-Default;
                                      //        1-The return code in 10h is valid.
    UINT16 ExtendErrorReturnVaild : 1;// Bit 14:
                                      //        0-Default;
                                      //        1-Refer to expending 30h (Extending Error return)of utput register ECX if bit14 is present.
    UINT16 NotSupport : 1;            // Bit 15:
                                      //        0-Success;
                                      //        1-Not support Data_Type. (The IHISI 41 is implement in BIOS but cannot recognize the new type)
  } Bits;
  UINT16 Data;
} ERRORY_RETURN_TYPE_57;

// Data type 59h:
#define ERROR_RETURE_ERASE_PASSWORD_FAIL      bit(0)

typedef union {
  struct {
    UINT16 EraseBiosPassword : 1;     // Bit 0 : Erase BIOS password (0-Pass ; 1-Fail)
    UINT16 Reserved : 12;             // Bit 1~12 : Set 0 (Reserved)
    UINT16 ReturnCodeVaild : 1;       // Bit 13:
                                      //        0-Default;
                                      //        1-The return code in 10h is valid.
    UINT16 ExtendErrorReturnVaild : 1;// Bit 14:
                                      //        0-Default;
                                      //        1-Refer to expending 30h (Extending Error return)of utput register ECX if bit14 is present.
    UINT16 NotSupport : 1;            // Bit 15:
                                      //        0-Success;
                                      //        1-Not support Data_Type. (The IHISI 41 is implement in BIOS but cannot recognize the new type)
  } Bits;
  UINT16 Data;
} ERRORY_RETURN_TYPE_59;

// Data type 5Bh, 5Ch, 5Dh, 5Eh, 5Fh:
#define ERROR_RETURE_FAIL                     bit(0)
typedef union {
  struct {
    UINT16 Result : 1;                 // Bit 0 : Result (0-Pass ; 1-Fail)
    UINT16 Reserved : 12;             // Bit 1~12 : Set 0 (Reserved)
    UINT16 ReturnCodeVaild : 1;       // Bit 13:
                                      //        0-Default;
                                      //        1-The return code in 10h is valid.
    UINT16 ExtendErrorReturnVaild : 1;// Bit 14:
                                      //        0-Default;
                                      //        1-Refer to expending 30h (Extending Error return)of utput register ECX if bit14 is present.
    UINT16 NotSupport : 1;            // Bit 15:
                                      //        0-Success;
                                      //        1-Not support Data_Type. (The IHISI 41 is implement in BIOS but cannot recognize the new type)
  } Bits;
  UINT16 Data;
} ERRORY_RETURN_TYPE_5B_5C_5D_5E_5F;

// OEM Extra Data Communication Data type code.
typedef enum {
  Vbios = 1,                      // VBIOS
  Reserved02,                     // Reserved
  Reserved03,                     // Reserved
  // 04h~0Ch  User Define
  // 0Dh~4Fh  Reserved
  Oa30ReadWrite = 0x50,          // 50h = OA 3.0 Read/Write
  Oa30Erase,                      // 51h = OA 3.0 Erase (Reset to default)
  Oa30PopulateHeader,             // 52h = OA 3.0 Populate Header
  Oa30DePopulateHeader,           // 53h = OA 3.0 De-Populate Header
  LogoUpdate,                     // 54h = Logo Update (Write)
  CheckBiosSignBySystemBios,      // 55h = Check BIOS sign by System BIOS
  ClaenSecureKey,                 // 56h = OEM function event to notice system BIOS to clean secure key.
  FactoryKeyRestore,              // 57h = OEM function event for factory key restore.
  PfatUpdate,                     // 58h = PFAT Update
  BiosPasswordErase,              // 59h = Erase BIOS Password
  PfatReturn,                     // 5Ah = PFAT Return
  StartTimeMeasure,               // 5Bh = Start time measuring.
  QueryTimeInterval,              // 5Ch = Query time interval.
  GetSpiLockTable,                // 5Dh = Get SPI Read/Write lock status table.
  AccessViaSpi,                   // 5Eh = Access via SPI transfer protocol
  PassErrorLog,                   // 5Fh = Pass Data to BIOS for error log.
  // 60h~EFh  Reserved
  // F0h~FEh= Reserved for user's function definition (Tool generator)
  RefereExtendDataType = 0xFF     // FFh = Refer to expending item 26h (Extending Data type) of input register CX if Data type is FFh.
} OEM_EXTRA_DATA_INPUT_DATATYPE_CODE;

//
// OEM Extra Data Communication Data size definition.
//
typedef enum {
  OemExtraDataSize64k,                 // 64k
  OemExtraDataSize128k,                // 128k
  OemExtraDataSize256k,                // 256k
  OemExtraDataSize512k,                // 512k
  OemExtraDataSize1024k,               // 1024k
  OemExtraDataSize2048k,               // 2048k
  OemExtraDataSize4096k,               // 4096k
  OemExtraDataSize8192k,               // 8192k
  OemExtraReadFromBios    = 0xFC,     // Application will read the data from BIOS and request BIOS to store it in ESI
  OemExtraReportReadSize  = 0xFD,     // BIOS send Data size is not contained in above 00~07h definition.
                                      // The data size should refer to "Physical data size" filed which defines in offset 0Ah.
                                      // (BIOS will send data to application for extra data read function 47h if field is FDh)
  OemExtraSkipSizeCheck   = 0xFE,     // BIOS acknowledge skip size check request from caller;
                                      // BIOS will also skip flash data size check too.
                                      // (Application will send data to BIOS for extra data write function 42h if field is FEh)
  OemExtraReportWriteSize = 0xFF,     // BIOS report it allows the physical size to caller,
                                      // the physical data size should refer to "Physical data size" filed which define in offset 0Ah.
                                      // (Application will send data to BIOS for extra data write function 42h if field is FFh)
} OEM_EXTRA_DATA_DATASIZE_TABLE;

//
// OEM Extra Data Block size definition.
//
typedef enum {
  OemExtraBlockSize4k,        // 4k
  OemExtraBlockSize64k,       // 64k
  // 02h~FEh = Reserved
  OemExtraMaximunBlockSize = 0xFF,     // Maximum block size.
                               // Note: Maximum block size is defined in Physical Data size field (offset 0Ah).
                               // If this field is FFh and block size is smaller than 64k,
                               // Data size field (offset 09h) must be set to FFh or FDh too.
} OEM_EXTRA_DATA_OUTPUT_BLOCK_SIZE_TABLE;

#pragma pack(1)
typedef struct {
  UINT8                         Type;
  UINT8                         Size;
  UINT8                         LogData[1];
} LOG_DATA_ITEM_STRUCTURE;

typedef struct {
  UINT32                        DeviceAddress;
  UINT8                         DataLength ;
  UINT8                         OPCodePtr;
  UINT8                         PrefixPtr;
  UINT8                         Atomic;
  UINT8                         CheckComplete;
} SPI_TRANSFER_PARAMETER_STRUCTURE;

//INPUT:
// ECX: Pointer to AP communication data buffer.
typedef struct {
  UINT32                        Signature;          // $EDI (AP_COMMUNICATION_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         DataType;           // OEM_EXTRA_DATA_INPUT_DATATYPE_CODE
  UINT8                         DataSize;           // OEM_EXTRA_DATA_DATASIZE_TABLE
  UINT32                        PhysicalDataSize;
  UINT64                        ExtendSignature;    // $IEXSIGN (AP_COMMUNICATION_EXTENDING_IEXSIGN_SIGNATURE)
  EFI_GUID                      IdentificationGUID;
  UINT32                        ExtendDataType;
} AP_COMMUNICATION_DATA_TABLE;

//  EDI:
//    When data type is 5Dh(GetSpiLockTable), pointer to return data buffer.

//    When data type is 5Eh(AccessViaSpi), the input structure as below:
typedef struct {
  UINT32                        Signature;    // $5EI (AP_COMMUNICATION_5E_TYPE_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         DataField[8];
  UINT32                        DeviceAddress;
  UINT8                         DataLength;
  UINT8                         OPCodePtr;
  UINT8                         PrefixPtr;
  UINT8                         Atomic;
  UINT8                         Data;
  UINT8                         CheckComplete;
} AP_COMMUNICATION_5E_DATA_STRUCTURE;

//    When data type is 5Fh(PassErrorLog), the input structure as below:
typedef struct {
  UINT32                        Signature;    // $5FI (AP_COMMUNICATION_5F_TYPE_SIGNATURE)
  UINT32                        StructureSize;
  LOG_DATA_ITEM_STRUCTURE       DataField[1];
} AP_COMMUNICATION_5F_DATA_STRUCTURE;

// OUTPUT:
//  ECX: Pointer to BIOS communication data buffer
typedef struct {
  UINT32                        Signature;    // $EDO (BIOS_COMMUNICATION_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         BlockSize;    // OEM_EXTRA_DATA_OUTPUT_BLOCK_SIZE_TABLE
  UINT8                         DataSize;     // OEM_EXTRA_DATA_DATASIZE_TABLE
  UINT32                        PhysicalDataSize;
  UINT16                        ErrorReturn;
  UINT64                        ReturnCode;
  UINT64                        ExtSignature; // $OEXSIGN (AP_COMMUNICATION_EXTENDING_OEXSIGN_SIGNATURE)
  EFI_GUID                      ExtGuid;
  UINT32                        ExtErrorReturn;
} BIOS_COMMUNICATION_DATA_TABLE;

//  EDI: When data type is 5Dh, the input structure as below:
typedef struct {
  UINT32                                Signature;        // $5DO (BIOS_COMMUNICATION_5D_TYPE_SIGNATURE)
  UINT32                                StructureSize;
  UINT8                                 Mask[8];
  UINT8                                 Mask_Length;
  SPI_TRANSFER_PARAMETER_STRUCTURE      ReadSPI;
  SPI_TRANSFER_PARAMETER_STRUCTURE      WriteSPI;
} BIOS_COMMUNICATION_5D_DATA_STRUCTURE;
#pragma pack()

/**

AH=42h, OEM Extra Data Write
  Function 41h and 42h (or 47h) are pairs.
  The function 41h has to be called before calling into function 42h.

**/
// INPUT:
//  CL: Same as SHUTDOWN_MODE_TABLE

//  CH: Target offset to write.

//  ESI:Pointer to input file data buffer. Size in EDI

//  EDI:Size to input.

/**

AH=47h, OEM Extra Data Read
  Function 41h and 47h (or 42h) are pairs.
  The function 41h has to be called before calling into function 47h

**/
// INPUT:
//  ESI: Pointer to return data buffer. Size in EDI.

//  EDI: Pointer to return size to output.

// OUTPUT:
//  ESI: Return file data buffer. Size in EDI.

//  EDI: Size to output.

/**

AH=43h, Check password by BIOS
  Application will pass user password to BIOS by Input buffer and then BIOS compare them with original password in BIOS.
  BIOS also have to return comparing result to application by Output buffer.
  Application bases these results to decide what to do for next stage.
  Besides AH=43h interface is notexisted in BIOS, else AL register must be 00h and also comparing result must be updated by BIOS.

**/
#define CHECK_PASSWORD_INPUT_SIGNATURE    SIGNATURE_64 ('$', 'P', 'A', 'S', 'S', 'W', 'D', 'I')
#define CHECK_PASSWORD_OUTPUT_SIGNATURE   SIGNATURE_64 ('$', 'P', 'A', 'S', 'S', 'W', 'D', 'O')

typedef enum {
  OriginalPasswordStatus,
  PasswordComparingResult,
  QPS_NotSupport = 0xEF
} QUERY_PASSWORD_STATUS;

typedef enum {
  PassworeReset,
  PassworeSetUp,
  OPS_NotSupport = 0xEF
} ORIGINAL_PASSWORD_STATUS;

typedef enum {
  Identical,
  NotIdentical,
  NotIdentical_NotRetry
} PASSWORD_COMPARE_RESULT;

#pragma pack(1)
// INPUT:
//  ECX:Pointer to AP communication data buffer.
typedef struct {
  UINT64                        Signature;            // $PASSWDI (CHECK_PASSWORD_INPUT_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         QueryPasswordStatus;  // QUERY_PASSWORD_STATUS
  UINT8                         NewPassword[256];
} PASSWORD_AP_COMMUNICATION_STRUCTURE;

// OUTPUT:
//  ECX:
typedef struct {
  UINT64                        Signature;              // $PASSWDO (CHECK_PASSWORD_OUTPUT_SIGNATURE)
  UINT32                        StructureSize;
  UINT8                         OriginalPasswordStatus; // ORIGINAL_PASSWORD_STATUS
  UINT8                         PasswordComparingResult;// PASSWORD_COMPARE_RESULT
} PASSWORD_BIOS_COMMUNICATION_STRUCTURE;
#pragma pack()

/**

AH=44h, Pass Flash Settings to BIOS
  Application passes flash settings and needed information to BIOS by Input buffer.
  After BIOS doing decision it will return the error code and result for allow flash or not.
  Application bases on the result to do following flash process.

**/
typedef enum {
  DC_PlugOut,
  DC_PlugIn
} DC_STATUS;

//
// Flash progress continue flash or not.
//
typedef enum {
  PassSetting_NotAllowFlash,
  PassSetting_AllowFlash,
} FLASH_PROGRESS_STATUS;

//
// Error message number.
//
typedef enum {
  MSG_Num_No_Error,
  MSG_Num_Show_AC_Error,
  MSG_Num_Show_DC_Error,
  MSG_Num_BIOS_Version_Error,
  MSG_Num_Model_Name_Error
} ERROR_MESSAGE_NUMBER;

//
// System behavior after flash completely.
//
typedef enum {
  PassSetting_DoNothing,
  PassSetting_Reboot,
  PassSetting_Shutdown
} FLASH_COMPLETE_BEHAVIOR;

#pragma pack(1)
// INPUT:A pointer point to input data buffer.
//  ESI:
typedef struct {
  UINT8                         AC_Present;     // AC_STATUS
  UINT8                         DC_Present;     // DC_STATUS
  UINT8                         Battery_Gas_Gauge;
  CHAR8                         Old_BIOS_Version[10];
  CHAR8                         New_BIOS_Version[10];
  CHAR8                         Old_Model_String[15];
  CHAR8                         New_Model_String[15];
} PASS_FLASH_SETTING_STRUCTURE;

//  EDI:A pointer point to an ASCII string for disable/enable specific function of protection when flash BIOS.

// OUTPUT:
//  EDI:The pointer to the structure for return parameter.
typedef struct {
  UINT16                        Flash_Progress; // FLASH_PROGRESS_STATUS
  UINT16                        Msg_Number;     // ERROR_MESSAGE_NUMBER
  UINT16                        Flash_Complete; // FLASH_COMPLETE_BEHAVIOR
} PASS_FLASH_SETTING_RETURN_PARTMETER;
#pragma pack()

/**

AH=45h, Check conditional data by BIOS
  Application will pass user conditional data to BIOS by Input buffer and then BIOS compare them with original data in BIOS.
  BIOS also have to return comparing result to application by Output buffer.
  Application bases these results to decide what to do for next stage.
  Besides AH=45h interface is not existed in BIOS, else AL register must be 00h and also comparing result must be updated by BIOS.

**/
#define CHECK_CONDITIONAL_DATA_CHKCNDI_SIGNATURE    SIGNATURE_64 ('$', 'C', 'H', 'K', 'C', 'N', 'D', 'I')
#define CHECK_CONDITIONAL_DATA_CHKCNDO_SIGNATURE    SIGNATURE_64 ('$', 'C', 'H', 'K', 'C', 'N', 'D', 'O')

//
// Check conditional data type
//
typedef enum {
  FeatureFlag = 0x01,
  SystemID,
  ResellerID,
  BIOSVersion,
  RomPartCheck
} CHECK_CONDITIONAL_DATA_TYPE;

typedef enum {
  BinaryType,
  ASCIIStringFormat,
  WordFormat,
  DWordFormat,
  DataFormatNoSuport = 0xEF
} CHECK_CONDITIONAL_DATA_FORMAT;

typedef enum {
  CheckConditional_NotAllowFlash,
  CheckConditional_AllowFlash,
  CheckConditional_NotSupport = 0xFF
} CHECK_CONDITIONAL_FLASH_PROGRESS;

#pragma pack(1)
// INPUT:
//  ECX:Pointer to AP communication data buffer.
typedef struct {
  UINT64                        Signature;    // $CHKCNDI (CHECK_CONDITIONAL_DATA_CHKCNDI_SIGNATURE)
  UINT32                        SignatureSize;
  UINT8                         DataType;     // CHECK_CONDITIONAL_DATA_TYPE
  UINT8                         DataFormate;  // CHECK_CONDITIONAL_DATA_FORMAT
  UINT16                        Data_Length;
  CHAR8                         Data_String[1];
} CHECK_CONDITIONAL_DATA_AP_COMMUNICATION_STRUCTURE;

// OUTPUT:
//  ECX:Pointer to BIOS communication data buffer.
typedef struct {
  UINT64                        Signature;      // $CHKCNDO (CHECK_CONDITIONAL_DATA_CHKCNDO_SIGNATURE)
  UINT32                        SignatureSize;
  UINT8                         Flash_Progress ;// CHECK_CONDITIONAL_FLASH_PROGRESS
} CHECK_CONDITIONAL_DATA_BIOS_COMMUNICATION_STRUCTURE;
#pragma pack()

/**

AH=60h, Data Access Communication

**/
#define GUID_NOT_MATCH              bit(63)
#define COMMAND_NUMBER_NOT_SUPPORT  bit(62)
#define DATA_SIZE_NOT_ACCEPTED      bit(61)

typedef union {
  struct {
    UINT64 UserDefine : 32;           // Bit 0~31 : reserved for user definition via tool generator.
    UINT64 ToolReserve : 29;          // Bit 32~60 : reserved for tool generator use only.
    UINT64 DataSizeNotAccept : 1;     // Bit 61: The data size is not accepted
    UINT64 CommandNotSupport : 1;     // Bit 62: The command number is not supported
    UINT64 GuidNotMatch : 1;          // Bit 63: GUID ID is not matched
  } Bits;
  UINT16 Data;
} DATA_ACCESS_COMMUNICATION_RETURN_STATUS;

#pragma pack(1)
// INPUT:
//  ECX:Communication data buffer.
typedef struct {
  UINT32                        Size;
  EFI_GUID                      IdentificationGuid;
  UINT32                        CommandNum;
  UINT32                        DataSize;
} DATA_ACCESS_COMMUNICATION_INPUT_STRUCTURE;


//  EDI:Parameters for "event" command
typedef enum {
  Windows_Event,
  DOS_Event,
  EFI_Event
} ENVIRONMENT_EVENT;

// OUTPUT:
//  ECX:Communication data buffer
typedef struct {
  UINT32                        Size;
  EFI_GUID                      IdentificationGuid;
  UINT32                        CommandNum;
  UINT32                        DataSize;
  UINT64                        Status;
} DATA_ACCESS_COMMUNICATION_OUTPUT_STRUCTURE;
#pragma pack()

/**

AH=61h, Data Read
  The function 60h has to be called before calling into function 61h.

***/
#pragma pack(1)
// INPUT:
//  ESI:Pointer to data buffer. BIOS fill the data to the right position by the input data offset.

//  EDI:
typedef struct {
  UINT32                        Size;
  UINT32                        DataSize;
  UINT32                        DataOffset;
} DATA_READ_BUFFER_STRUCTURE;
#pragma pack()

/**

AH=62h, Data Write
  The function 60h has to be called before calling into function 62h.

**/
#pragma pack(1)
// INPUT:
//  ESI:Pointer to data buffer. BIOS fill the data to the right position by the input data offset.

//  EDI:
typedef struct {
  UINT32                        Size;
  UINT32                        DataSize;
  UINT32                        DataOffset;
} DATA_WRITE_BUFFER_STRUCTURE;
#pragma pack()
#endif
