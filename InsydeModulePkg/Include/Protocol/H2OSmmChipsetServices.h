/** @file
 H2O SMM Chipset Services protocol interface definition.

 This file provides the H2O SMM Chipset Services protocol definition. It is
 consumed by drivers and libraries in the Kernel and Platform layers. It is
 produced by drivers and libraries in the Chipset layer.

 WARNING: This protocol should not be called directly. Instead, the library
 functions in the SmmChipsetSvcLib should be used, because they add the
 necessary error checking.

***************************************************************************
* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#ifndef _H2O_SMM_CHIPSET_SERVICES_H_
#define _H2O_SMM_CHIPSET_SERVICES_H_
#include <ChipsetSvc.h>
#include <H2OIhisi.h>

#define H2O_SMM_CHIPSET_SERVICES_PROTOCOL_GUID \
  {0x42b70ab8, 0xb17f, 0x48b9, {0x93, 0x35, 0x10, 0x24, 0xa6, 0xe0, 0xa, 0x7b }}

//
// Function prototypes
//
/**
 Platform specific function to enable/disable flash device write access.

 @param[in]         EnableWrites        TRUE  - Enable
                                        FALSE - Disable

 @retval            EFI_SUCCESS         Function alway returns successfully
*/
typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_ENABLE_FD_WRITES) (
  IN      BOOLEAN                         EnableWrites
);

/**
 Legacy Region Access Control.

 @param[in]         Start               Start of the region to lock or unlock.
 @param[in]         Length              Length of the region.
 @param[in]         Mode                If LEGACY_REGION_ACCESS_LOCK, then
                                        LegacyRegionAccessCtrl() lock the specific
                                        legacy region.

 @retval            EFI_SUCCESS         Function returns successfully
 @retval            EFI_INVALID_PARAMETER 1. The parameter Start is small then 0xC0000.
                                          2. The parameter Length is to long.
                                             The Start + Length - 1 should small then 0xF0000.
                                          3. The Mode parameter is neither
                                             LEGACY_REGION_ACCESS_LOCK nor LEGACY_REGION_ACCESS_UNLOCK

*/
typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_LEGACY_REGION_ACCESS_CTRL) (
  IN      UINT32                          Start,
  IN      UINT32                          Length,
  IN      UINT32                          Mode
  );

/**
 Resets the system.

 @param[in]         ResetType           The type of reset to perform.

 @retval            EFI_UNSUPPORTED     ResetType is invalid.
 @return            others              Error occurs
*/
typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_RESET_SYSTEM) (
  IN      EFI_RESET_TYPE                  ResetType
  );

/**
 This routine issues SATA COM reset on the specified SATA port

 @param[in]         PortNumber          The SATA port number to be reset

 @retval            EFI_SUCCESS         The SATA port has been reset successfully
 @retval            EFI_DEVICE_ERROR    1.SATA controller isn't in IDE, AHCI or RAID mode.
                                        2.Get error when getting PortNumberMapTable.
*/
typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_SATA_COMRESET) (
  IN  UINTN                    PortNumber
  );

/**
  AH=10h(FbtsGetSupportVersion), Get FBTS supported version and FBTS permission.

  @param[in] VersionStr          Flash tool version
  @param[in, out] Permission     Permission
**/
typedef
VOID
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_GET_PREMISSION)(
  IN  FBTS_TOOLS_VERSION_BUFFER           *VersionPtr,
  IN  OUT UINT16                          *Permission
  );

/**
  AH=10h(FbtsGetSupportVersion), Get Ac status.

  @retval 0     AC not plug in
  @retval 1     AC plug in
**/
typedef
UINT8
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_GET_ACSTATUS)(
  VOID
  );

/**
  AH=10h(FbtsGetSupportVersion), Get Battery life.

@retval   0~100       Battery life percent
@retval   101(0x65)   Battery is not installed.
**/
typedef
UINT8
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_GET_BATTERY_LIFE)(
  VOID
  );

/**
  AH=11h(FbtsGetPlatformInfo), AP check.

@retval
  BIT 0: Model name check
  BIT 1: Model version check
  BIT 2: Allow same version file.
  BIT 3: Verify file checksum
  BIT 4: Disable display model name
  BIT 5: Disable display model version
  BIT 6: Disable read comparison while flash process
  BIT 7: Enable ECX to return extended flag.
**/

typedef
UINT8
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_AP_CHECK)(
  VOID
  );

/**
  AH=12h(FbtsGetPlatformRomMap), Get Oem define flash map.

  @param[in, out] RomMapBuffer          Pointer to the returned platform's ROM map protection structure.
                                        After version 1.7.6, InsydeFlash will skip this structure if it found definition in BVDT
  @param[in, out] PrivateRomMapBuffer   Pointer to the returned platform's private map structure.
                                        Flash utility will not flash these areas
                                        (even userenter /all in flashit or all=1 in platform.ini).

  @retval EFI_SUCCESS       Get OEM flash map successful.
  @retval EFI_UNSUPPORTED   FBTS_OEM_ROM_MAP_COUNT is 0 or module rom map buffer is full.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_GET_OEMFLASHMAP)(
  IN OUT   FBTS_PLATFORM_ROM_MAP       **RomMapBuffer,
  IN OUT   FBTS_PLATFORM_PRIVATE_ROM   **PrivateRomMapBuffer
  );

/**
  AH=15h(FBTS write) : Hook function before Write process.

  @param[in, out] WriteDataBufferPtr    Pointer to data buffer for write.
  @param[in, out] WriteSizePtr          Write size.
  @param[in, out] RomBaseAddressPtr     Target linear address to write.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_DO_BEFORE_WRITE_PROCESS)(
  IN OUT    UINT8                      *WriteDataBufferPtr,
  IN OUT    UINTN                      *WriteSizePtr,
  IN OUT    UINTN                      *RomBaseAddressPtr
  );

/**
  AH=15h(FBTS write) : Hook function after Write process.

  @param[in]      WriteStatus

  @retval EFI_SUCCESS        Function returns successfully
**/
typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_DO_AFTER_WRITE_PROCESS) (
  IN EFI_STATUS        WriteStatus
  );

/**
  AH=14h(FbtsRead) : Hook function before read process

  @param[in, out] ReadAddress           Target linear address to read.
  @param[in, out] ReadSize              Read size.
  @param[in, out] DataBuffer            Pointer to returned data buffer.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_DO_BEFORE_READ_PROCESS) (
  IN OUT UINTN                                   *ReadAddress,
  IN OUT UINTN                                   *ReadSize,
  IN OUT UINT8                                   *DataBuffer
  );

/**
  AH=14h(FbtsRead) : Hook function fter read process

  @param[in]      ReadStatus

  @retval EFI_SUCCESS        Function returns successfully
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_DO_AFTER_READ_PROCESS) (
  IN EFI_STATUS        ReadStatus
  );

/**
  AH=16h(Fbtscomplete), This function uses to execute Ap terminate.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_AP_TERMINATED) (
  VOID
  );

/**
  AH=16h(Fbtscomplete), This function uses to execute normal flash.
                        (Update whole image or BIOS region by normal or secure flash.)

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_NORMAL_FLASH) (
  VOID
  );

/**
  AH=16h(Fbtscomplete), This function uses to execute Partial flash.
                        (Update specific address or update single firmware volume.)

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_PARTIAL_FLASH) (
  VOID
  );

/**
  AH=16h(Fbtscomplete), This function is a hook funciton berfore ApRequest execute.
  
  @param[in]      ApReques   Ap reuest to do behavior

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_OEM_COMPLETE) (
  IN UINT8           ApRequest
  );

/**
  Function to do system shutdown.

  @retval EFI_SUCCESS      Successfully returns.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_SHUTDOWN) (
  VOID
  );

/**
  Function to reboot system.

  @retval EFI_SUCCESS      Successfully returns.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_REBOOT) (
  VOID
  );

/**
  Function to flash complete do nothing.

  @retval EFI_SUCCESS      Successfully returns.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FBTS_AP_REQUEST_DO_NOTHING) (
  VOID
  );

/**
  AH=20h(FetsWrite), Hook function before flashing EC part.

  @param[in, out] FlashingDataBuffer Double pointer to data buffer.
  @param[in, out] SizeToFlash        Data size by bytes want to flash.
  @param[in, out] DestBlockNo        Dsstination block number.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FETS_DO_BEFORE_FLASHING) (
  IN OUT  UINT8         **FlashingDataBuffer,
  IN OUT  UINTN          *SizeToFlash,
  IN OUT  UINT8          *DestBlockNo
  );

/**
   AH=20h(FetsWrite), Hook function uses to flash EC part.

  @param[in, out] FlashingDataBuffer Pointer to input file data buffer
  @param[in] SizeToFlash        Data size.
  @param[in] DestBlockNo        Dsstination block number.

  @retval EFI_SUCCESS       Successfully returns.
  @return Other             Error occurred in this function.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FETS_EC_FLASH) (
  IN UINT8                          *FlashingDataBuffer,
  IN UINTN                           SizeToFlash,
  IN UINT8                           DestBlockNo
  );

/**
  AH=20h(FetsWrite), Hook function after flashing EC part.

  @param[in] ActionAfterFlashing    Input action flag.

  @retval EFI_UNSUPPORTED       Returns unsupported by default.
  @retval EFI_SUCCESS           The service is customized in the project.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FETS_DO_AFTER_FLASHING) (
  IN UINT8                          ActionAfterFlashing
  );

/**
  Fets reset system function.

  @retval EFI_SUCCESS   Successfully returns.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FETS_REBOOT) (
  VOID
  );

/**
  Fets shutdown function.

  @retval EFI_SUCCESS   Successfully returns.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FETS_SHUTDOWN) (
  VOID
  );

/**
  Fets EC idle function

  @param[in] Idle     Ec idle mode.

  @retval EFI_SUCCESS   Successfully returns.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FETS_EC_IDLE) (
  IN BOOLEAN              Idle
  );

/**
  AH=21h(GetEcPartInfo), Get EC part information.

  @param[in, out] EcPartSize        EC part size

  @retval EFI_SUCCESS   Successfully returns.
**/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_FETS_GET_EC_PART_INFO) (
  IN OUT UINT32     *EcPartSize
  );

/**
  AH=41h(OemExtraDataCommunication),
  This function offers an interface to do IHISI Sub function AH=41h.

 @param[in]         ApCommDataBuffer    Pointer to AP communication data buffer.
 @param[in, out]    BiosCommDataBuffer  On entry, pointer to BIOS communication data buffer.
                                        On exit, points to updated BIOS communication data buffer.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_OEMEXT_COMMUNICATION) (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  IN OUT BIOS_COMMUNICATION_DATA_TABLE     *BiosCommDataBuffer
);

/**
  AH=42h(OemExtraDataWrite)
  This function offers an interface to do IHISI Sub function AH=42h,

 @param[in]         FunctionType      Function type.
 @param[in, out]    WriteDataBuffer   Pointer to input file data buffer.
 @param[in, out]    WriteSize         Write size.
 @param[in, out]    RomBaseAddress    Target offset to write.
 @param[in]         ShutdownMode      Shutdown mode.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_OEMEXT_DATA_WRITE) (
  IN      UINT8                   FunctionType,
  IN OUT  UINT8                  *WriteDataBuffer,
  IN OUT  UINTN                  *WriteSize,
  IN OUT  UINTN                  *RomBaseAddress,
  IN      UINT8                   ShutdownMode
);

/**
 AH=47h(OemExtraDataRead),
 This function offers an interface to do IHISI Sub function AH=47h,

 @param[in]         FunctionType        Function type
 @param[in, out]    DataBuffer          Pointer to return data buffer.
 @param[in, out]    Size                Read size.
 @param[in, out]    RomBaseAddress      Read address.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/

typedef
EFI_STATUS
(EFIAPI *H2O_CS_SVC_IHISI_OEMEXT_DATA_READ) (
  IN UINT8                        FunctionType,
  IN OUT  UINT8                  *DataBuffer,
  IN OUT  UINTN                  *Size,
  IN OUT  UINTN                  *RomBaseAddress
);


typedef struct _H2O_SMM_CHIPSET_SERVICES_PROTOCOL {
  UINT32 Size;                          ///< size of this structure, in bytes.

  H2O_CS_SVC_ENABLE_FD_WRITES                   EnableFdWrites;
  H2O_CS_SVC_LEGACY_REGION_ACCESS_CTRL          LegacyRegionAccessCtrl;
  H2O_CS_SVC_RESET_SYSTEM                       ResetSystem;
  H2O_CS_SVC_SATA_COMRESET                      SataComReset;
  H2O_CS_SVC_IHISI_FBTS_GET_PREMISSION          IhisiFbtsGetPermission;
  H2O_CS_SVC_IHISI_FBTS_GET_ACSTATUS            IhisiFbtsGetAcStatus;
  H2O_CS_SVC_IHISI_FBTS_GET_BATTERY_LIFE        IhisiFbtsGetBatteryLife;
  H2O_CS_SVC_IHISI_FBTS_AP_CHECK                IhisiFbtsApCheck;
  H2O_CS_SVC_IHISI_FBTS_GET_OEMFLASHMAP         IhisiFbtsGetOemFlashMap;
  H2O_CS_SVC_IHISI_FBTS_DO_BEFORE_WRITE_PROCESS IhisiFbtsDoBeforeWriteProcess;
  H2O_CS_SVC_IHISI_FBTS_DO_AFTER_WRITE_PROCESS  IhisiFbtsDoAfterWriteProcess;
  H2O_CS_SVC_IHISI_FBTS_DO_BEFORE_READ_PROCESS  IhisiFbtsDoBeforeReadProcess;
  H2O_CS_SVC_IHISI_FBTS_DO_AFTER_READ_PROCESS   IhisiFbtsDoAfterReadProcess;
  H2O_CS_SVC_IHISI_FBTS_AP_TERMINATED           IhisiFbtsApTerminated;
  H2O_CS_SVC_IHISI_FBTS_NORMAL_FLASH            IhisiFbtsNormalFlash;
  H2O_CS_SVC_IHISI_FBTS_PARTIAL_FLASH           IhisiFbtsPartialFlash;
  H2O_CS_SVC_IHISI_FBTS_OEM_COMPLETE            IhisiFbtsOemComplete;
  H2O_CS_SVC_IHISI_FBTS_SHUTDOWN                IhisiFbtsShutdown;
  H2O_CS_SVC_IHISI_FBTS_REBOOT                  IhisiFbtsReboot;
  H2O_CS_SVC_IHISI_FBTS_AP_REQUEST_DO_NOTHING   IhisiFbtsApRequestDoNothing;
  H2O_CS_SVC_IHISI_FETS_DO_BEFORE_FLASHING      IhisiFetsDoBeforeFlash;
  H2O_CS_SVC_IHISI_FETS_EC_FLASH                IhisiFetsEcFlash;
  H2O_CS_SVC_IHISI_FETS_DO_AFTER_FLASHING       IhisiFetsDoAfterFlash;
  H2O_CS_SVC_IHISI_FETS_REBOOT                  IhisiFetsReboot;
  H2O_CS_SVC_IHISI_FETS_SHUTDOWN                IhisiFetsShutdown;
  H2O_CS_SVC_IHISI_FETS_EC_IDLE                 IhisiFetsEcIdle;
  H2O_CS_SVC_IHISI_FETS_GET_EC_PART_INFO        IhisiFetsGetEcPartInfo;
  H2O_CS_SVC_IHISI_OEMEXT_COMMUNICATION         IhisiOemExtCommunication;
  H2O_CS_SVC_IHISI_OEMEXT_DATA_WRITE            IhisiOemExtDataWrite;
  H2O_CS_SVC_IHISI_OEMEXT_DATA_READ             IhisiOemExtDataRead;
} H2O_SMM_CHIPSET_SERVICES_PROTOCOL;

extern EFI_GUID gH2OSmmChipsetServicesProtocolGuid;

#endif
