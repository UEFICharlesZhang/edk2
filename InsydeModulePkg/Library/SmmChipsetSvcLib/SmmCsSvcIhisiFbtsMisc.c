/** @file
 SMM Chipset Services Library.

 This file contains Ihisi Fbts Chipset service Lib function.

***************************************************************************
* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#include <Uefi.h>
#include <H2OIhisi.h>
#include <Library/SmmChipsetSvcLib.h>
#include <Library/DebugLib.h>
#include <Protocol/H2OSmmChipsetServices.h>

extern H2O_SMM_CHIPSET_SERVICES_PROTOCOL *mSmmChipsetSvc;

/**
  AH=10h(FbtsGetSupportVersion), Get FBTS supported version and FBTS permission.

  @param[in] VersionStr          Flash tool version
  @param[in, out] Permission     Permission
**/
VOID
EFIAPI
SmmCsSvcIhisiFbtsGetPermission (
  IN  FBTS_TOOLS_VERSION_BUFFER           *VersionPtr,
  IN  OUT UINT16                          *Permission
  )
{
  if (Permission == NULL) {
    return ;
  }
  *Permission = FBTS_PERMISSION_ALLOWED;
  //
  // Verify that the protocol interface structure contains the function
  // pointer and whether that function pointer is non-NULL. If not, return
  // an error.
  //
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsGetPermission) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsGetPermission == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsGetPermission() isn't implement!\n"));
    return;
  }
  mSmmChipsetSvc->IhisiFbtsGetPermission (VersionPtr, Permission);
  return;
}

/**
  AH=10h(FbtsGetSupportVersion), Get Ac status.

  @retval 0     AC not plug in
  @retval 1     AC plug in
**/
UINT8
EFIAPI
SmmCsSvcIhisiFbtsGetAcStatus (
  VOID
  )
{
  UINT8             ACStatus;

  ACStatus = AC_PlugIn;

  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsGetAcStatus) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsGetAcStatus == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsGetAcStatus() isn't implement!\n"));
    return ACStatus;
  }

  ACStatus = mSmmChipsetSvc->IhisiFbtsGetAcStatus ();

  return  ACStatus;
}

/**
  AH=10h(FbtsGetSupportVersion), Get Battery life.

@retval   0~100       Battery life percent
@retval   101(0x65)   Battery is not installed.
**/
UINT8
EFIAPI
SmmCsSvcIhisiFbtsGetBatteryLife (
  VOID
  )
{
  UINT8             BattLife;

  BattLife = 100;

  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsGetBatteryLife) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsGetBatteryLife == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsGetBatteryLife() isn't implement!\n"));
    return BattLife;
  }

  BattLife = mSmmChipsetSvc->IhisiFbtsGetBatteryLife ();

  return  BattLife;
}

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
  )
{
  UINT8             ApStatus;

  ApStatus = AP_DO_NOTHING;

  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsApCheck) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsApCheck == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsApCheck() isn't implement!\n"));
    return ApStatus;
  }

  ApStatus = mSmmChipsetSvc->IhisiFbtsApCheck ();

  return ApStatus;
}

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
)
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsGetOemFlashMap) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsGetOemFlashMap == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsGetOemFlashMap() isn't implement!\n"));
    return EFI_UNSUPPORTED;
  }

  return mSmmChipsetSvc->IhisiFbtsGetOemFlashMap (RomMapBuffer, PrivateRomMapBuffer);
}

/**
  AH=15h(FBTS write) :  Hook function before Write process

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
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsDoBeforeWriteProcess) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsDoBeforeWriteProcess == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsDoBeforeWriteProcess() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFbtsDoBeforeWriteProcess (WriteDataBufferPtr, WriteSizePtr, RomBaseAddressPtr);
}

/**
  AH=15h(FBTS write) :  Hook function after Write process

  @param[in]      WriteStatus

  @retval EFI_SUCCESS        Function returns successfully
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsDoAfterWriteProcess (
  IN EFI_STATUS        WriteStatus
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsDoAfterWriteProcess) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsDoAfterWriteProcess == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsDoAfterWriteProcess() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFbtsDoAfterWriteProcess (WriteStatus);
}

/**
  AH=14h(FbtsRead) :  Hook function before read process

  @param[in, out] ReadAddress           Target linear address to read.
  @param[in, out] ReadSize              Read size.
  @param[in, out] DataBuffer            Pointer to returned data buffer.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsDoBeforeReadProcess (
  IN OUT UINTN                            *ReadAddress,
  IN OUT UINTN                            *ReadSize,
  IN OUT UINT8                            *DataBuffer
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsDoBeforeReadProcess) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsDoBeforeReadProcess == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsDoBeforeReadProcess() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFbtsDoBeforeReadProcess (ReadAddress, ReadSize, DataBuffer);
}

/**
  AH=14h(FbtsRead) :  Hook function after read process
        
  @param[in]      ReadStatus
  
  @retval EFI_SUCCESS        Function returns successfully
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsDoAfterReadProcess (
  IN EFI_STATUS        ReadStatus
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsDoAfterReadProcess) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsDoAfterReadProcess == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsDoAfterReadProcess() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFbtsDoAfterReadProcess (ReadStatus);
}

/**
  AH=16h(Fbtscomplete), This function uses to execute Ap terminate.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsApTerminated (
  VOID
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsApTerminated) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsApTerminated == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsApTerminated() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFbtsApTerminated ();
}

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
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsNormalFlash) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsNormalFlash == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsNormalFlash() isn't implement!\n"));
    return EFI_UNSUPPORTED;
  }

  return mSmmChipsetSvc->IhisiFbtsNormalFlash ();
}

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
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsPartialFlash) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsPartialFlash == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsPartialFlash() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFbtsPartialFlash ();
}

/**
  AH=16h(Fbtscomplete), This function is a hook funciton berfore ApRequest execute.

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsOemComplete (
  IN UINT8           ApRequest
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsOemComplete) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsOemComplete == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsOemComplete() isn't implement!\n"));
    return EFI_UNSUPPORTED;
  }

  return mSmmChipsetSvc->IhisiFbtsOemComplete (ApRequest);
}

/**
  Function to do system shutdown.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsShutDown (
  VOID
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsShutdown) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsShutdown == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsShutdown() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFbtsShutdown ();
}

/**
  Function to reboot system.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsReboot (
  VOID
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsReboot) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsReboot == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsReboot() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFbtsReboot ();
}

/**
  Function to flash complete do nothing.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
SmmCsSvcIhisiFbtsApRequestDoNothing (
  VOID
  )
{
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiFbtsApRequestDoNothing) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiFbtsApRequestDoNothing == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiFbtsReboot() isn't implement!\n"));
    return EFI_SUCCESS;
  }

  return mSmmChipsetSvc->IhisiFbtsApRequestDoNothing ();
}

