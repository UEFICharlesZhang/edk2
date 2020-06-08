/**
 SMM Chipset Services Library.

  This file provides the prototype of all SMM Chipset Services Library function.

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

#ifndef _SMM_CHIPSET_SVC_LIB_H_
#define _SMM_CHIPSET_SVC_LIB_H_
#include <Uefi.h>
#include <ChipsetSvc.h>
#include <H2OIhisi.h>

/**
 Platform specific function to enable/disable flash device write access.

 @param[in]         EnableWrites        TRUE  - Enable
                                        FALSE - Disable

 @retval            EFI_SUCCESS         Function returns successfully
 @retval            EFI_UNSUPPORTED     If the function is not implemented.
*/
EFI_STATUS
EFIAPI
SmmCsSvcEnableFdWrites (
  IN  BOOLEAN           EnableWrites
  );

/**
 Legacy Region Access Control.

 @param[in]         Start               Start of the region to lock or unlock.
 @param[in]         Length              Length of the region.
 @param[in]         Mode                If LEGACY_REGION_ACCESS_LOCK, then
                                        LegacyRegionAccessCtrl() lock the specific
                                        legacy region.

 @retval            EFI_SUCCESS         Function returns successfully
 @retval            EFI_UNSUPPORTED       If the function is not implemented.
 @retval            EFI_INVALID_PARAMETER 1. The parameter Start is small then 0xC0000.
                                          2. The parameter Length is to long.
                                             The Start + Length - 1 should small then 0xF0000.
                                          3. The Mode parameter is neither
                                             LEGACY_REGION_ACCESS_LOCK nor LEGACY_REGION_ACCESS_UNLOCK

*/
EFI_STATUS
EFIAPI
SmmCsSvcLegacyRegionAccessCtrl (
  IN  UINT32                        Start,
  IN  UINT32                        Length,
  IN  UINT32                        Mode
  );

/**
 Resets the system.

 @param[in]         ResetType           The type of reset to perform.

 @retval            EFI_SUCCESS         Function returns successfully
 @retval            EFI_UNSUPPORTED     1. If the function is not implemented.
                                        2. ResetType is invalid.
 @return            others              Error occurs
*/
EFI_STATUS
EFIAPI
SmmCsSvcResetSystem (
  IN EFI_RESET_TYPE   ResetType
  );

/**
 This routine issues SATA COM reset on the specified SATA port

 @param[in]         PortNumber          The SATA port number to be reset

 @retval            EFI_SUCCESS         The SATA port has been reset successfully
 @retval            EFI_DEVICE_ERROR    1.SATA controller isn't in IDE, AHCI or RAID mode.
                                        2.Get error when getting PortNumberMapTable.
 @retval            EFI_UNSUPPORTED     If the function is not implemented.
*/
EFI_STATUS
EFIAPI
SmmCsSvcSataComReset (
  IN  UINTN         PortNumber
  );

/**
  AH=10h(FbtsGetSupportVersion), Get FBTS supported version and FBTS permission.

  @param[in] VersionStr          Flash tool version
  @param[in, out] Permission     Permission
**/
VOID
EFIAPI
SmmCsSvcIhisiFbtsGetPermission (
  IN FBTS_TOOLS_VERSION_BUFFER              *VersionPtr,
  IN OUT UINT16                             *Permission
  );

/**
  AH=10h(FbtsGetSupportVersion), Get Ac status.

  @retval 0     AC not plug in
  @retval 1     AC plug in
**/
UINT8
EFIAPI
SmmCsSvcIhisiFbtsGetAcStatus (
  VOID
  );

/**
  AH=10h(FbtsGetSupportVersion), Get Battery life.

@retval   0~100       Battery life percent
@retval   101(0x65)   Battery is not installed.
**/
UINT8
EFIAPI
SmmCsSvcIhisiFbtsGetBatteryLife (
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
UINT8
EFIAPI
SmmCsSvcIhisiFbtsApCheck (
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
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsGetOemFlashMap (
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
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsDoBeforeWriteProcess (
  IN OUT    UINT8                      *WriteDataBufferPtr,
  IN OUT    UINTN                      *WriteSizePtr,
  IN OUT    UINTN                      *RomBaseAddressPtr
  );

/**
  AH=15h(FBTS write) : Hook function after Write process.

  @param[in]      WriteStatus

  @retval EFI_SUCCESS        Function returns successfully
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsDoAfterWriteProcess (
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
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsDoBeforeReadProcess (
  IN OUT UINTN                                   *ReadAddress,
  IN OUT UINTN                                   *ReadSize,
  IN OUT UINT8                                   *DataBuffer
  );

/**
  AH=14h(FbtsRead) : Hook function fter read process

  @param[in]      ReadStatus

  @retval EFI_SUCCESS        Function returns successfully
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsDoAfterReadProcess (
  IN EFI_STATUS        ReadStatus
  );

/**
  AH=16h(Fbtscomplete), This function uses to execute Ap terminate.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsApTerminated (
  VOID
  );

/**
  AH=16h(Fbtscomplete), This function uses to execute normal flash.
                        (Update whole image or BIOS region by normal or secure flash.)

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsNormalFlash (
  VOID
  );

/**
  AH=16h(Fbtscomplete), This function uses to execute Partial flash.
                        (Update specific address or update single firmware volume.)

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsPartialFlash (
  VOID
  );

/**
  AH=16h(Fbtscomplete), This function is a hook funciton berfore ApRequest execute.

  @param[in]      ApReques   Ap reuest to do behavior

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsOemComplete (
  IN UINT8           ApRequest
  );

/**
  Function to do system shutdown.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsShutDown (
  VOID
  );

/**
  Function to reboot system.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsReboot(
  VOID
  );

/**
  Function to flash complete do nothing.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsApRequestDoNothing (
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
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsDoBeforeFlashing (
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
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsEcFlash (
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
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsDoAfterFlashing (
  IN UINT8                          ActionAfterFlashing
  );

/**
  Fets reset system function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsReboot (
  VOID
  );

/**
  Fets shutdown function.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsShutdown (
  VOID
  );

/**
  Fets EC idle function

  @param[in] Idle     Ec idle mode.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsEcIdle (
  IN BOOLEAN              Idle
  );

/**
  AH=21h(GetEcPartInfo), Get EC part information.

  @param[in, out] EcPartSize        EC part size

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFetsGetEcPartInfo (
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
EFI_STATUS
EFIAPI
SmmCsSvcIhisiOemExtCommunication (
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
EFI_STATUS
EFIAPI
SmmCsSvcIhisiOemExtDataWrite (
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
EFI_STATUS
EFIAPI
SmmCsSvcIhisiOemExtDataRead (
  IN UINT8                        FunctionType,
  IN OUT  UINT8                  *DataBuffer,
  IN OUT  UINTN                  *Size,
  IN OUT  UINTN                  *RomBaseAddress
);
#endif
