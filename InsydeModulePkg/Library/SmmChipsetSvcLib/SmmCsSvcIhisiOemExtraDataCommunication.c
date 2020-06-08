/** @file
 SMM Chipset Services Library.

 This file contains Ihisi OemExtra Data Communication Chipset service Lib function.

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
#include <Library/DebugLib.h>
#include <Protocol/H2OSmmChipsetServices.h>

extern H2O_SMM_CHIPSET_SERVICES_PROTOCOL *mSmmChipsetSvc;

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
){
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiOemExtCommunication) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiOemExtCommunication == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiOemExtCommunication() isn't implement!\n"));
    return EFI_UNSUPPORTED;
  }

  return mSmmChipsetSvc->IhisiOemExtCommunication (ApCommDataBuffer, BiosCommDataBuffer);
}

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
){
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiOemExtDataWrite) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiOemExtDataWrite == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiOemExtDataWrite() isn't implement!\n"));
    return EFI_UNSUPPORTED;
  }

  return mSmmChipsetSvc->IhisiOemExtDataWrite (FunctionType, WriteDataBuffer, WriteSize, RomBaseAddress, ShutdownMode);
}

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
){
  if (mSmmChipsetSvc == NULL ||
      mSmmChipsetSvc->Size < (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, IhisiOemExtDataRead) + sizeof (VOID*)) ||
      mSmmChipsetSvc->IhisiOemExtDataRead == NULL) {
    DEBUG ((EFI_D_ERROR, "H2O SMM Chipset Services can not be found or member IhisiOemExtDataRead() isn't implement!\n"));
    return EFI_UNSUPPORTED;
  }

  return mSmmChipsetSvc->IhisiOemExtDataRead (FunctionType, DataBuffer, Size, RomBaseAddress);
}

