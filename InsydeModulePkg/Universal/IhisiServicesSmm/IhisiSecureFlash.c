/** @file
  SecureFlashLib Library Instance implementation

;******************************************************************************
;* Copyright (c) 2012 - 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "SecureFlash.h"
#include "IhisiRegistration.h"
#include <IndustryStandard/PeImage.h>
#include <Library/VariableLib.h>
#include <Protocol/TianoCompress.h>

#define SECURE_FLASH_SIGNATURE      SIGNATURE_32 ('$', 'S', 'E', 'C')

BOOLEAN  mInPOST = TRUE;

/**
  Verify if the image is signed.

  @param[in] ImageBase       The address of the image.

  @retval EFI_SUCCESS        if the image is not signed
  @retval EFI_ACCESS_DENIED  if the image is signed
**/
EFI_STATUS
VerifySignedImage (
  IN UINT8                      *ImageBase
  )
{
  EFI_IMAGE_DOS_HEADER                *DosHeader;
  UINT32                              PeCoffHeaderOffset;
  UINT16                              Magic;
  EFI_IMAGE_DATA_DIRECTORY            *SectionDataDir;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION PeHeader;


  DosHeader = (EFI_IMAGE_DOS_HEADER *)(UINTN)(ImageBase);
  if (DosHeader->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    //
    // DOS image header is present,
    // so read the PE header after the DOS image header.
    //
    PeCoffHeaderOffset = DosHeader->e_lfanew;
  } else {
    PeCoffHeaderOffset = 0;
  }
  PeHeader.Pe32 = (EFI_IMAGE_NT_HEADERS32 *) (ImageBase + PeCoffHeaderOffset);
  if (PeHeader.Pe32->Signature != EFI_IMAGE_NT_SIGNATURE) {
    //
    // It is not a valid Pe/Coff file.
    //
    return EFI_ACCESS_DENIED;
  }

  Magic = PeHeader.Pe32->OptionalHeader.Magic;
  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    //
    // Use PE32 offset.
    //
    SectionDataDir = (EFI_IMAGE_DATA_DIRECTORY *) &PeHeader.Pe32->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY];
  } else {
    //
    // Use PE32+ offset.
    //
    SectionDataDir = (EFI_IMAGE_DATA_DIRECTORY *) &PeHeader.Pe32Plus->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY];
  }

  if (SectionDataDir->Size == 0) {
    //
    // This image is not signed.
    //
    return EFI_ACCESS_DENIED;
  }

  return EFI_SUCCESS;
}

/**
  Gather all block images and merge them into TargetImageAddress.

  @param[in] TargetImageAddress The address of the complete image.

  @retval EFI_SUCCESS           Function executes successful.
  @retval EFI_INVALID_PARAMETER The image size is incorrect.
**/
EFI_STATUS
MergeImageBlockWithoutCompress (
  IN EFI_PHYSICAL_ADDRESS       TargetImageAddress
  )
{
  UINTN                                       Index;
  UINT64                                      TotalImageSize;
  UINT32                                      NumberOfImageBlk;
  UINT8                                      *Source;
  IMAGE_BLOCK_DATA_ITEM_STRUCTURE            *ImageBlock;
  FBTS_SECURE_FLASH_IMAGE_BLOCK_STRUCTURE    *ImageBlkPtr;

  TotalImageSize = 0;
  ImageBlkPtr = (FBTS_SECURE_FLASH_IMAGE_BLOCK_STRUCTURE*)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  //
  // The first DWORD is number of blocks, the second DWORD is reserved.
  //
  NumberOfImageBlk = ImageBlkPtr->BlockNum;
  ImageBlock = ImageBlkPtr->BlockDataItem;
  if (BufferOverlapSmram ((VOID *) ImageBlkPtr, NumberOfImageBlk)) {
    return EFI_INVALID_PARAMETER;
  }
  for (Index = 0; Index < NumberOfImageBlk; Index++) {
    Source = (UINT8*)(UINTN) ImageBlock->ImageBlockAddress;
    if (BufferOverlapSmram ((VOID *) Source, (UINTN) ImageBlock->ImageBlockSize)) {
      return EFI_INVALID_PARAMETER;
    }
    CopyMem ((UINT8*)(UINTN) TargetImageAddress, Source, (UINTN)ImageBlock->ImageBlockSize);
    TotalImageSize += ImageBlock->ImageBlockSize;
    TargetImageAddress += ImageBlock->ImageBlockSize;
    ImageBlock++;
  }

  if (TotalImageSize == (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX)) {
    return EFI_SUCCESS;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}

/**
  Gather all block images and merge them into reserved memory.
  Compress the image data Before copying memory.

  @param[in,out] ImageInfo      The information of the image.

  @retval EFI_SUCCESS           Function executes successful.
  @retval EFI_BUFFER_TOO_SMALL  The reserved buffer is not enough.
  @retval EFI_INVALID_PARAMETER The image size is incorrect.
  @retval EFI_PROTOCOL_ERROR    Cannot locate TianoCompress Protocol.
**/
EFI_STATUS
MergeImageBlock (
  IN OUT IMAGE_INFO             *ImageInfo
  )
{
  UINTN                                         Index;
  UINTN                                         Index2;
  EFI_STATUS                                    Status;
  UINT32                                        RemainSize;
  UINT32                                        NumberOfImageBlk;
  UINT32                                        MaxNumOfSplitBlock;
  COMPRESS_DATA_HEADER                          CompressDataHeader;
  COMPRESS_TABLE_HEADER                         CompressTableHeader;
  UINT8                                        *Source;
  IMAGE_BLOCK_DATA_ITEM_STRUCTURE              *ImageBlock;
  FBTS_SECURE_FLASH_IMAGE_BLOCK_STRUCTURE      *ImageBlkPtr;
  TIANO_COMPRESS_PROTOCOL                      *TianoCompress;
  UINT8                                        *TargetImagePtr;

  TianoCompress = NULL;
  Status = gSmst->SmmLocateProtocol (
                    &gTianoCompressProtocolGuid,
                    NULL,
                    (VOID **)&TianoCompress
                    );
  if (EFI_ERROR (Status)) {
    return EFI_PROTOCOL_ERROR;
  }

  Index2 = 0;
  Source = NULL;
  MaxNumOfSplitBlock = 0;

  //
  // Initialize the information of table header.
  //
  CompressTableHeader.NumOfBlock = 0;
  CompressTableHeader.TotalImageSize = 0;
  CompressTableHeader.TotalCompressedDataSize = sizeof (COMPRESS_TABLE_HEADER);

  //
  // The first data area is for COMPRESS_TABLE_HEADER.
  //
  TargetImagePtr = (UINT8*)(UINTN) ImageInfo->ImageAddress + sizeof (COMPRESS_TABLE_HEADER);

  //
  // The first DWORD is number of blocks, the second DWORD is reserved.
  //
  ImageBlkPtr = (FBTS_SECURE_FLASH_IMAGE_BLOCK_STRUCTURE*)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  NumberOfImageBlk = ImageBlkPtr->BlockNum;
  ImageBlock = ImageBlkPtr->BlockDataItem;
  if (BufferOverlapSmram ((VOID *) ImageBlkPtr, NumberOfImageBlk)) {
    return EFI_INVALID_PARAMETER;
  }
  for (Index = 0; Index < NumberOfImageBlk; Index++) {
    Source = (UINT8*)(UINTN) ImageBlock->ImageBlockAddress;
    if (BufferOverlapSmram ((VOID *) Source, (UINTN) ImageBlock->ImageBlockSize)) {
      return EFI_INVALID_PARAMETER;
    }
    RemainSize = (UINT32)(UINTN) ImageBlock->ImageBlockSize;
    //
    // Calculate the max block number of size of IMAGE_BLOCK_BUFFERSIZE_FOR_COMPRESS.(at least one)
    //
    if ((RemainSize % IMAGE_BLOCK_BUFFERSIZE_FOR_COMPRESS) == 0) {
      MaxNumOfSplitBlock = RemainSize / IMAGE_BLOCK_BUFFERSIZE_FOR_COMPRESS;
    } else {
      MaxNumOfSplitBlock = (RemainSize / IMAGE_BLOCK_BUFFERSIZE_FOR_COMPRESS) + 1;
    }

    for (Index2 = 0; Index2 < MaxNumOfSplitBlock; Index2++) {
      if (((RemainSize % IMAGE_BLOCK_BUFFERSIZE_FOR_COMPRESS) == 0) || (RemainSize >= IMAGE_BLOCK_BUFFERSIZE_FOR_COMPRESS)) {
        CompressDataHeader.SourceSize = IMAGE_BLOCK_BUFFERSIZE_FOR_COMPRESS;
      } else {
        CompressDataHeader.SourceSize = RemainSize;
      }

      CompressDataHeader.CompressedSize = CompressDataHeader.SourceSize * 2;
      //
      // We should not use the memory over reserved image size.
      //
      if (CompressTableHeader.TotalCompressedDataSize + CompressDataHeader.CompressedSize > ImageInfo->ImageSize) {
        return EFI_BUFFER_TOO_SMALL;
      }
      Status = TianoCompress->Compress (
                                Source,
                                CompressDataHeader.SourceSize,
                                TargetImagePtr + sizeof (COMPRESS_DATA_HEADER),
                                &CompressDataHeader.CompressedSize
                                );
      if (Status == EFI_BUFFER_TOO_SMALL) {
        //
        // We should not use the memory over reserved image size.
        //
        if (CompressTableHeader.TotalCompressedDataSize + CompressDataHeader.CompressedSize > ImageInfo->ImageSize) {
          return EFI_BUFFER_TOO_SMALL;
        }
        Status = TianoCompress->Compress (
                                  Source,
                                  CompressDataHeader.SourceSize,
                                  TargetImagePtr + sizeof (COMPRESS_DATA_HEADER),
                                  &CompressDataHeader.CompressedSize
                                  );
      }
      CopyMem (TargetImagePtr, &CompressDataHeader, sizeof (COMPRESS_DATA_HEADER));

      CompressTableHeader.TotalImageSize += CompressDataHeader.SourceSize;
      CompressTableHeader.TotalCompressedDataSize += CompressDataHeader.CompressedSize + sizeof (COMPRESS_DATA_HEADER);

      TargetImagePtr += CompressDataHeader.CompressedSize + sizeof (COMPRESS_DATA_HEADER);
      Source += CompressDataHeader.SourceSize;
      RemainSize -= CompressDataHeader.SourceSize;
    }

    ImageBlock++;
    CompressTableHeader.NumOfBlock += MaxNumOfSplitBlock;
  }

  ImageInfo->ImageSize = CompressTableHeader.TotalCompressedDataSize;
  //
  // Write information to the table header.
  //
  TargetImagePtr = (UINT8*)(UINTN) ImageInfo->ImageAddress;
  CopyMem (TargetImagePtr, &CompressTableHeader, sizeof (COMPRESS_TABLE_HEADER));

  if (CompressTableHeader.TotalImageSize == (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX)) {
    return EFI_SUCCESS;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}

/**
  OEM capsule secure flash.(AH=48h)

  @retval EFI_SUCCESS            Function succeeded.
  @return Others                 Error occurred in this function.
**/

EFI_STATUS
SecureFlashFunction (
  VOID
  )
{
  UINTN                                       Size;
  EFI_STATUS                                  Status;
  IMAGE_INFO                                  ImageInfo;
  UINT32                                      TotalImageSize;
  UINT8                                      *ImageBase;
  IMAGE_BLOCK_DATA_ITEM_STRUCTURE            *ImageBlock;
  FBTS_SECURE_FLASH_IMAGE_BLOCK_STRUCTURE    *ImageBlkPtr;
  FBTS_EXTEND_FLAG_STRUCTURE                 *ExtendFlagPtr;

  if (!PcdGetBool (PcdSecureFlashSupported)) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  //
  // mInPOST is a flag used for disabling IHISI write function at Runtime.
  // There is a SMI triggerd by SecureFlashDxe to modify this flag.
  //
  if ((mInPOST) && (IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX) == SECURE_FLASH_SIGNATURE)) {
    mInPOST = FALSE;
    return EFI_SUCCESS;
  }
  //
  // The image size cannot be zero.
  //
  TotalImageSize = IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  if (TotalImageSize == 0) {
    return IHISI_ACCESS_PROHIBITED;
  }

  //
  // Get reserved memory address to save the image we need.
  //
  Size = sizeof (IMAGE_INFO);
  Status = CommonGetVariable (
               L"SecureFlashInfo",
               &gSecureFlashInfoGuid,
               &Size,
               &ImageInfo
               );
  if (EFI_ERROR (Status)) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  //
  // EDI offset8 Bit0: 0 = Update secure capsule via IHISI. (Default)
  //                   1 = Update secure capsule via ESP (EFI System Partition)
  //
  ExtendFlagPtr = (FBTS_EXTEND_FLAG_STRUCTURE *)(UINTN)IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  if (ExtendFlagPtr->Flag.Bits.SecureCapsuleOrigin == FROME_ESP) {
    //
    // The image is saved in EFI Partition.
    // Only save ImageSize.
    //
    Status = EFI_SUCCESS;
    ImageInfo.ImageSize = TotalImageSize;
    ImageInfo.Compressed = FALSE;
  } else {
    //
    // Get the address of the signed image(from ESI) and verify if it is a signed image.
    // The first DWORD is number of blocks, the second DWORD is reserved, skip them.
    // We only need the first block to verify if it is a signed image.
    //
    ImageBlkPtr = (FBTS_SECURE_FLASH_IMAGE_BLOCK_STRUCTURE*)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
    ImageBlock = ImageBlkPtr->BlockDataItem;
    ImageBase = (UINT8*)(UINTN)(ImageBlock->ImageBlockAddress);
    if (BufferOverlapSmram ((VOID *) ImageBlock, 1) || BufferOverlapSmram ((VOID *) ImageBase, 1)) {
      return IHISI_UNSUPPORTED_FUNCTION;
    }
    Status = VerifySignedImage (ImageBase);
    if (Status == EFI_SUCCESS) {
      //
      // The image is saved in reserved memroy.
      // If the image size is smaller than the size of reserved memroy, no need to be compressed.
      //
      if (TotalImageSize <= ImageInfo.ImageSize) {
        Status = MergeImageBlockWithoutCompress (ImageInfo.ImageAddress);
        ImageInfo.ImageSize = TotalImageSize;
        ImageInfo.Compressed = FALSE;
      } else {
        Status = MergeImageBlock (&ImageInfo);
        ImageInfo.Compressed = TRUE;
      }
    } else {
      return IHISI_UNSUPPORTED_FUNCTION;
    }
  }
  if (EFI_ERROR (Status)) {
    return IHISI_OB_LEN_TOO_SMALL;
  }

  //
  // Set this variable to inform BIOS of doing secure flash.
  //
  ImageInfo.FlashMode = TRUE;
  Status = CommonSetVariable (
               L"SecureFlashInfo",
               &gSecureFlashInfoGuid,
               EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
               sizeof (IMAGE_INFO),
               &ImageInfo
             );

  //
  // return BIOS support secure flash type
  //   0: winflash should do S3
  //   1: winflash should do reboot
  //   2: winflash should do shutdown
  //   3: winflash should do nothing
  //
  IhisiWriteCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX, WINFLASH_DO_REBOOT);

  return EFI_SUCCESS;
}

EFI_STATUS
InstallSecureFlashServices (
  VOID
  )
{
  EFI_STATUS            Status;

  Status = IhisiRegisterCommand (FBTSOEMCapsuleSecureFalsh, SecureFlashFunction, IhisiNormalPriority);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

