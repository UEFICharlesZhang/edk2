/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "IhisiOemExtraDataCommunication.h"
#include <IndustryStandard/Oa3_0.h>
#include <Library/SmmChipsetSvcLib.h>
#include <Library/BaseOemSvcKernelLib.h>
#include <Library/IoLib.h> //BLD_Charles_035+

UINT8     mOemExtraDataType = 0;

//
// Oem Extra Data Communication Kernel Functions
//
OEM_EXT_COMMUNICAT_FUNCTION
OemExt_DataCommunication_KernelFunction[]= {
  { Oa30ReadWrite,                OemExtraOa30ReadWriteFunction            }, \
  { Oa30Erase,                    OemExtraOa30EraseFunction                }, \
  { Oa30PopulateHeader,           OemExtraOa30PopulateHeaderFunction       }, \
  { Oa30DePopulateHeader,         OemExtraOa30DePopulateHeaderFunction     }, \
};

//
// Oem Extra Support Functions
//
IHISI_SMI_SUB_FUNCTION
OEM_COMMON_FEATURE_TABLE[] = {
  { OEMSFOEMExCommunication,    OemExtraDataCommunication,    IhisiNormalPriority}, \
  { OEMSFOEMExDataWrite,        OemExtraDataWrite,            IhisiNormalPriority}, \
  { OEMSFOEMExDataRead,         OemExtraDataRead,             IhisiNormalPriority}, \
  //BLD_Charles_035+s
  { SubFuncF0NovoWrite,        SubFunc_NovoDataWrite,            IhisiNormalPriority}, \
  { SubFuncF1NovoRead,         SubFunc_NovoDataRead,             IhisiNormalPriority}
  //BLD_Charles_035+e
};

STATIC
BOOLEAN
MsdmExist (
  )
{
  EFI_STATUS                       Status;
  UINTN                            DataSize;
  BOOLEAN                          MsdmExist;
  UINTN                            RomBaseAddress;
  EFI_ACPI_MSDM_DATA_STRUCTURE    *MsdmData;

  MsdmData = NULL;
  MsdmExist = FALSE;

  DataSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);
  RomBaseAddress = (UINTN) FixedPcdGet32 (PcdFlashNvStorageMsdmDataBase);

  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    DataSize,
                    (VOID **)&MsdmData
                    );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  Status = mSmmFwBlockService->Read (
                                 mSmmFwBlockService,
                                 RomBaseAddress,
                                 0,
                                 &DataSize,
                                 (UINT8*) MsdmData
                                 );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if ((MsdmData->MsdmVersion == 0xFFFFFFFF) &&
      (MsdmData->MdsmDataType == 0xFFFFFFFF) &&
      (MsdmData->MsdmDataLength == 0xFFFFFFFF)) {
    goto Done;
  }

  MsdmExist = TRUE;

Done:
  if (MsdmData != NULL) {
    gSmst->SmmFreePool (MsdmData);
  }
  return MsdmExist;
}

/**
  AH=41h, OEM Extra Data Communication type 50h to read/write OA3.0.

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS  Read or write OA3.0 successful.
**/
EFI_STATUS
OemExtraOa30ReadWriteFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  )
{
  UINT32            MsdmDataSize;

  MsdmDataSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);
  switch (ApCommDataBuffer->DataSize) {

  case OemExtraReportReadSize:
    BiosCommDataBuffer->Signature        = BIOS_COMMUNICATION_SIGNATURE;
    BiosCommDataBuffer->StructureSize    = sizeof (BIOS_COMMUNICATION_DATA_TABLE);
    BiosCommDataBuffer->BlockSize        = OemExtraBlockSize4k;
    BiosCommDataBuffer->DataSize         = OemExtraReportReadSize;
    BiosCommDataBuffer->PhysicalDataSize = MsdmDataSize;
    break;

  case OemExtraSkipSizeCheck:
    BiosCommDataBuffer->Signature        = BIOS_COMMUNICATION_SIGNATURE;
    BiosCommDataBuffer->StructureSize    = sizeof (BIOS_COMMUNICATION_DATA_TABLE);
    BiosCommDataBuffer->BlockSize        = OemExtraBlockSize4k;
    BiosCommDataBuffer->DataSize         = OemExtraSkipSizeCheck;   //Don't care
    BiosCommDataBuffer->PhysicalDataSize = 0x00;                   //Don't care
    break;

  case OemExtraReportWriteSize:
    BiosCommDataBuffer->Signature        = BIOS_COMMUNICATION_SIGNATURE;
    BiosCommDataBuffer->StructureSize    = sizeof (BIOS_COMMUNICATION_DATA_TABLE);
    BiosCommDataBuffer->BlockSize        = OemExtraBlockSize4k;
    BiosCommDataBuffer->DataSize         = OemExtraReportWriteSize;
    BiosCommDataBuffer->PhysicalDataSize = MsdmDataSize;   //bin size
    break;

  default:
    break;
  }

  if (!MsdmExist ()) {
    BiosCommDataBuffer->ErrorReturn    = (BiosCommDataBuffer->ErrorReturn | ERROR_RETURE_OA30_NOT_EXIST);
  }

  return EFI_SUCCESS;
}

/**
  AH=41h, OEM Extra Data Communication type 51h to erase OA3.0 (reset to default).

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS  Erase OA3.0 successful.
**/
EFI_STATUS
OemExtraOa30EraseFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  )
{
  EFI_STATUS                   Status;
  UINT8                        LoopCount;
  UINTN                        RomBaseAddress;
  UINTN                        EraseSize = 0x1000;

  RomBaseAddress = (UINTN) FixedPcdGet32 (PcdFlashNvStorageMsdmDataBase);
  LoopCount = 0;
  Status = EFI_SUCCESS;

  if (!MsdmExist ()) {
    BiosCommDataBuffer->ErrorReturn = (BiosCommDataBuffer->ErrorReturn | ERROR_RETURE_OA30_NOT_EXIST);
    goto Done;
  }
  OemSvcEcIdle (TRUE);
  Status = EFI_UNSUPPORTED;
  while ((EFI_ERROR (Status)) && (LoopCount < 100)) {
    Status = mSmmFwBlockService->EraseBlocks (
                                   mSmmFwBlockService,
                                   RomBaseAddress,
                                   &EraseSize
                                   );
    LoopCount++;
  }
  OemSvcEcIdle (FALSE);

Done:
  BiosCommDataBuffer->DataSize = ApCommDataBuffer->DataSize;
  BiosCommDataBuffer->PhysicalDataSize = ApCommDataBuffer->PhysicalDataSize;

  return Status;
}


/**
  AH=41h, OEM Extra Data Communication type 52h to populate header.

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS   populate header successful.
**/
EFI_STATUS
OemExtraOa30PopulateHeaderFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  )
{

  return EFI_SUCCESS;
}

/**
  AH=41h, OEM Extra Data Communication type 53h to de-populate header.

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS   populate header successful.
**/
EFI_STATUS
OemExtraOa30DePopulateHeaderFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  )
{

  return EFI_SUCCESS;
}

/**
  AH=47h, OEM Extra Data Write type 50h to read OA30.
  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
OemExtraOa30DataWrite (
  UINT8                  *WriteDataBuffer,
  UINTN                   WriteSize,
  UINTN                   RomBaseAddress
  )
{
  EFI_STATUS              Status;
  UINT8                   LoopCount;
  UINT32                  MsdmDataSize;

  MsdmDataSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);

  if (WriteSize == MsdmDataSize) {
    UINT8                          *ReturnDataBuffer = NULL;
    UINTN                          Index2 = 0;
    UINTN                          EraseSize = 0x1000;
    UINT8                          *TEMP;

    Status = gSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      0x1000,
                      (VOID **)&ReturnDataBuffer
                      );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }

    Status = mSmmFwBlockService->Read (
                                  mSmmFwBlockService,
                                  RomBaseAddress,
                                  0,
                                  &EraseSize,
                                  ReturnDataBuffer
                                  );
    //
    // Only modify the first 0x31 bytes
    //
    TEMP = ReturnDataBuffer;
    for (Index2 = 0; Index2 < MsdmDataSize; Index2++) {
      TEMP[Index2] = WriteDataBuffer[Index2];
    }

    LoopCount = 0;
    Status = EFI_UNSUPPORTED;
    *(mSmmFwBlockService->FlashMode) = SMM_FW_FLASH_MODE;
    OemSvcEcIdle (TRUE);

    while ((EFI_ERROR (Status)) && (LoopCount < 100)) {
      Status = mSmmFwBlockService->EraseBlocks (
                                     mSmmFwBlockService,
                                     RomBaseAddress,
                                     &EraseSize
                                     );
      Status = mSmmFwBlockService->Write (
                                     mSmmFwBlockService,
                                     RomBaseAddress,
                                     &EraseSize,
                                     ReturnDataBuffer
                                     );
      LoopCount++;
    }
    gSmst->SmmFreePool (ReturnDataBuffer);
    OemSvcEcIdle (FALSE);
  } else {
    return EFI_INVALID_PARAMETER;
  }

  return Status;
}

/**
  AH=47h, OEM Extra Data Write type 50h to read OA30.
  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
OemExtraOa30DataRead (
  UINT8                  *ReadDataBuffer,
  UINTN                  *ReadSize,
  UINTN                   RomBaseAddress
  )
{
  EFI_STATUS                       Status;
  UINT8                            *ReturnDataBuffer = NULL;
  UINTN                            DataSize = 0x1000;

  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    DataSize,
                    (VOID **)&ReturnDataBuffer
                    );
  if (EFI_ERROR(Status)) {
    return IHISI_FBTS_READ_FAILED;
  }

  Status = mSmmFwBlockService->Read (
                                 mSmmFwBlockService,
                                 RomBaseAddress,
                                 0,
                                 &DataSize,
                                 ReturnDataBuffer
                                 );

  *ReadSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);
  CopyMem (ReadDataBuffer, ReturnDataBuffer, *ReadSize);

  return Status;
}

/**
  AH=41h, OEM Extra Data Communication

  01h = VBIOS
  02h~03h = Reserved
  04h~0Ch = User Define
  0Dh~4Fh = Reserved
  50h = OA 3.0 Read/Write
  51h = OA 3.0 Erase (Reset to default)
  52h = OA 3.0 Populate Header
  53h = OA 3.0 De-Populate Header
  54h = Logo Update (Write)
  55h = Check BIOS sign by System BIOS
  56~FFh = Reserved

  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
OemExtraDataCommunication (
  VOID
  )
{
  EFI_STATUS                        Status;
  UINT8                             Index;
  BIOS_COMMUNICATION_DATA_TABLE     BiosCommDataBuffer;
  AP_COMMUNICATION_DATA_TABLE      *ApCommDataBuffer;

  mOemExtraDataType = 00;
  Status = IHISI_UNSUPPORTED_FUNCTION;
  //
  // Initialize the output communication data buffer.
  //
  ZeroMem (&BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));
  BiosCommDataBuffer.Signature        = BIOS_COMMUNICATION_SIGNATURE;
  BiosCommDataBuffer.StructureSize    = sizeof (BIOS_COMMUNICATION_DATA_TABLE);
  BiosCommDataBuffer.BlockSize        = OemExtraBlockSize4k;
  BiosCommDataBuffer.DataSize         = OemExtraDataSize64k;
  BiosCommDataBuffer.PhysicalDataSize = 0;
  BiosCommDataBuffer.ErrorReturn      = 0;

  ApCommDataBuffer = (AP_COMMUNICATION_DATA_TABLE*) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  if (ApCommDataBuffer->Signature != AP_COMMUNICATION_SIGNATURE) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  if (BufferOverlapSmram ((VOID *) ApCommDataBuffer, ApCommDataBuffer->StructureSize)){
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  mOemExtraDataType = ApCommDataBuffer->DataType;

  Status = SmmCsSvcIhisiOemExtCommunication(ApCommDataBuffer, &BiosCommDataBuffer);

  if (EFI_ERROR (Status)) {
    for (Index = 0; Index < sizeof(OemExt_DataCommunication_KernelFunction)/sizeof(OemExt_DataCommunication_KernelFunction[0]); Index++) {
      if (ApCommDataBuffer->DataType == OemExt_DataCommunication_KernelFunction[Index].SubFunID) {
        Status = OemExt_DataCommunication_KernelFunction[Index].OemExtCommunicationSubFun (ApCommDataBuffer, &BiosCommDataBuffer);
        break;
      }
    }

    if (EFI_ERROR (Status)) {
      mOemExtraDataType = 0;
      return IHISI_UNSUPPORTED_FUNCTION;
    }
  }

  CopyMem (ApCommDataBuffer, &BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));
  return EFI_SUCCESS;
}

/**
  AH=42h, OEM Extra Data Write.

  Function 41h and 42h (or 47h) are pairs. The function 41h has to be called before calling
  into function 42h.

  @retval EFI_SUCCESS            OEM Extra Data Write successful.
  @return Other                  OEM Extra Data Write failed.
**/
EFI_STATUS
EFIAPI
OemExtraDataWrite (
  VOID
  )
{
  EFI_STATUS          Status;
  UINTN               WriteSize;
  UINT8               ShutdownMode;
  UINTN               RomBaseAddress;
  UINT8              *WriteDataBuffer;

  WriteDataBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  WriteSize       = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  RomBaseAddress  = (UINTN) (IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX) >> 8);
  ShutdownMode    = (UINT8) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);

  if (BufferOverlapSmram ((VOID *) WriteDataBuffer, WriteSize)){
    return IHISI_INVALID_PARAMETER;
  }

  Status = SmmCsSvcIhisiOemExtDataWrite(mOemExtraDataType, WriteDataBuffer, &WriteSize, &RomBaseAddress, ShutdownMode);
  if (Status == EFI_SUCCESS) {
    return Status;
  }

  switch (mOemExtraDataType) {
    case Oa30ReadWrite:
      if (Status == EFI_UNSUPPORTED) {
        RomBaseAddress = (UINTN) FixedPcdGet32 (PcdFlashNvStorageMsdmDataBase);
      }
      OemExtraOa30DataWrite(WriteDataBuffer, WriteSize, RomBaseAddress);
      break;

    default:
      break;
  }

  switch (ShutdownMode) {
    case DoNothing:
    case WindowsReboot:
    case WindowsShutdown:
      Status = EFI_SUCCESS;
      break;

    case DosReboot:
      Status = SmmCsSvcIhisiFbtsReboot ();
      break;

    case DosShutdown:
      Status = SmmCsSvcIhisiFbtsShutDown ();
      break;

    default:
      Status = EFI_UNSUPPORTED;
      break;
  }

  return Status;
}

/**
  AH=47h, OEM Extra Data Read.

  Function 41h and 47h (or 42h) are pairs. The function 41h has to be called before calling into
  function 47h.

  @retval EFI_SUCCESS            OEM Extra Data Read successful.
  @return Other                  OEM Extra Data Read failed.
**/
EFI_STATUS
EFIAPI
OemExtraDataRead (
  VOID
  )
{
  EFI_STATUS                       Status;
  UINTN                            ReadSize;
  UINTN                            RomBaseAddress;
  UINT8                           *ReadDataBuffer;

  ReadSize = 0;
  RomBaseAddress = 0;
  ReadDataBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  ReadSize = *(UINT32 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);

  if (BufferOverlapSmram ((VOID *) ReadDataBuffer, ReadSize)){
    return IHISI_INVALID_PARAMETER;
  }

  Status = SmmCsSvcIhisiOemExtDataRead(mOemExtraDataType, ReadDataBuffer, &ReadSize, &RomBaseAddress);
  if (Status == EFI_SUCCESS) {
    return Status;
  }

  switch (mOemExtraDataType) {
    case Oa30ReadWrite:
      if (Status == EFI_UNSUPPORTED) {
        RomBaseAddress = (UINTN) FixedPcdGet32 (PcdFlashNvStorageMsdmDataBase);
      }
      Status = OemExtraOa30DataRead(ReadDataBuffer, &ReadSize, RomBaseAddress);
      break;

    default:
      break;
  }

  *(UINT32 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI) = (UINT32)ReadSize;

  return Status;

}
//BLD_Charles_035+s
EFI_STATUS
SubFunc_NovoDataWrite (
  VOID
  )
{
  EFI_STATUS                    Status;
  UINT8                            *WriteDataBuffer;
  UINTN                            WriteSize;
  UINTN                            RomBaseAddress;
  UINT8                            LoopCount;

    WriteDataBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RSI);
    WriteSize = 0x2000; 
    RomBaseAddress = (UINTN) FixedPcdGet32 (PcdFlashFvEepromBase);
		IoWrite8 (0x70, 0x55);
        IoWrite8 (0x71, 0xAA);
    LoopCount = 0;
    Status = EFI_UNSUPPORTED;
    *(mSmmFwBlockService->FlashMode) = SMM_FW_DEFAULT_MODE; //Have 2 mode, must use Deault.

    while ((EFI_ERROR (Status)) && (LoopCount < 10)) {
      Status = mSmmFwBlockService->EraseBlocks (
                                    mSmmFwBlockService,
                                    RomBaseAddress,
                                    &WriteSize
                                    );
      
      Status = mSmmFwBlockService->Write (
                                    mSmmFwBlockService,
                                    RomBaseAddress,
                                    &WriteSize,
                                    WriteDataBuffer
      							);
      LoopCount++;
    }
  
  return Status;
}

EFI_STATUS
SubFunc_NovoDataRead (
  VOID
  )
{
  EFI_STATUS                       Status;
  UINT8                            *ReadDataBuffer;
  UINTN                            ReadSize;
  UINTN                            RomBaseAddress;

  ReadDataBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  ReadSize = 0x2000;  //Just define our 8KB size
  
  RomBaseAddress = (UINTN) FixedPcdGet32 (PcdFlashFvEepromBase);

  if(ReadSize < 0x2000){
     return EFI_WARN_BUFFER_TOO_SMALL;
  }
		IoWrite8 (0x70, 0x55);
        IoWrite8 (0x71, 0xBB);
	 Status = mSmmFwBlockService->Read (
								   mSmmFwBlockService,
								   RomBaseAddress,
								   0,
								   &ReadSize,
								   ReadDataBuffer
								   );
    return Status;
}
//BLD_Charles_035+e

EFI_STATUS
InstallOemExtraDataCommunicationServices (
  VOID
  )
{
  EFI_STATUS            Status;
  UINTN                 Index;

  for (Index = 0; Index < sizeof(OEM_COMMON_FEATURE_TABLE)/sizeof(OEM_COMMON_FEATURE_TABLE[0]); Index++) {
    Status = IhisiRegisterCommand (OEM_COMMON_FEATURE_TABLE[Index].CmdNumber,
                                   OEM_COMMON_FEATURE_TABLE[Index].IhisiFunction,
                                   OEM_COMMON_FEATURE_TABLE[Index].Priority);
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}

