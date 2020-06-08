/** @file
  RotateScreen

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
#include <Uefi.h>

#include <Guid/DebugMask.h>

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/SimpleTextInEx.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/AbsolutePointer.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/LoadedImage.h>
#include "HookLib.h"

#define RESOURCE_SECTION_RESOURCE_TYPE_VERSION               16
#define VS_FIXEDFILEINFO_SIGNATURE                           0xFEEF04BD
#define MICROSOFT_OS_VERSION_WINDOWS_8                       0x00060002

typedef struct {
  UINT32             Signature;
  UINT32             StrucVersion;
  UINT32             FileVersionMS;
  UINT32             FileVersionLS;
  UINT32             ProductVersionMS;
  UINT32             ProductVersionLS;
  UINT32             FileFlagsMask;
  UINT32             FileFlags;
  UINT32             FileOS;
  UINT32             FileType;
  UINT32             FileSubtype;
  UINT32             FileDateMS;
  UINT32             FileDateLS;
} VS_FIXEDFILEINFO;

/**
  Get the headers (dos, image, optional header) from an image

  @param  Device                SimpleFileSystem device handle
  @param  FileName              File name for the image
  @param  DosHeader             Pointer to dos header
  @param  Hdr                   The buffer in which to return the PE32, PE32+, or TE header.

  @retval EFI_SUCCESS           Successfully get the machine type.
  @retval EFI_NOT_FOUND         The file is not found.
  @retval EFI_LOAD_ERROR        File is not a valid image file.

**/
EFI_STATUS
EFIAPI
GetImageDataDirectory (
  IN  UINT8                                 *FileBuffer,
  IN  UINT64                                FileSize,
  IN  UINT8                                 DirectoryType,
  OUT EFI_IMAGE_DATA_DIRECTORY              *DirectoryEntry
  )
{
  EFI_IMAGE_DOS_HEADER                *DosHeader;
  UINT64                              BufferSize;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION Hdr;
  UINT16                              Magic;
  UINT32                              NumberOfRvaAndSizes;
  EFI_IMAGE_DATA_DIRECTORY            *Directory;

  //
  // dos header
  //
  BufferSize = sizeof (EFI_IMAGE_DOS_HEADER);
  DosHeader  = (EFI_IMAGE_DOS_HEADER *) FileBuffer;
  if (FileSize < BufferSize ||
      FileSize <= DosHeader->e_lfanew ||
      DosHeader->e_magic != EFI_IMAGE_DOS_SIGNATURE) {
    return EFI_UNSUPPORTED;
  }

  //
  // check PE signature
  //
  Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)(FileBuffer + DosHeader->e_lfanew);
  if (Hdr.Pe32->Signature != EFI_IMAGE_NT_SIGNATURE) {
    return EFI_UNSUPPORTED;
  }

  Magic = Hdr.Pe32->OptionalHeader.Magic;

  //
  // Check PE32 or PE32+ magic
  //
  if (Magic != EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
      Magic != EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    return EFI_UNSUPPORTED;
  }

  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    //
    // Use PE32 offset
    //
    NumberOfRvaAndSizes = Hdr.Pe32->OptionalHeader.NumberOfRvaAndSizes;
    Directory = (EFI_IMAGE_DATA_DIRECTORY *) &Hdr.Pe32->OptionalHeader.DataDirectory[DirectoryType];
  } else {
    //
    // Use PE32+ offset
    //
    NumberOfRvaAndSizes = Hdr.Pe32Plus->OptionalHeader.NumberOfRvaAndSizes;
    Directory = (EFI_IMAGE_DATA_DIRECTORY *) &Hdr.Pe32Plus->OptionalHeader.DataDirectory[DirectoryType];
  }

  if ((DirectoryType >= NumberOfRvaAndSizes) || Directory->Size == 0) {
    return EFI_NOT_FOUND;
  }

  CopyMem (DirectoryEntry, Directory, sizeof (EFI_IMAGE_DATA_DIRECTORY));

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
GetImageSectionHeader (
  IN  UINT8                                 *FileBuffer,
  IN  UINT64                                FileSize,
  IN  CHAR8                                 *SectionName,
  OUT EFI_IMAGE_SECTION_HEADER              *SectionHeader
  )
{
  EFI_IMAGE_DOS_HEADER                *DosHeader;
  UINT64                              BufferSize;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION Hdr;
  UINT16                              Magic;
  EFI_IMAGE_SECTION_HEADER            *SectionHdr;
  UINTN                               Index;
  UINTN                               NumberOfSections;
  UINTN                               StrLength;
  //
  // dos header
  //
  BufferSize = sizeof (EFI_IMAGE_DOS_HEADER);
  DosHeader  = (EFI_IMAGE_DOS_HEADER *) FileBuffer;
  if (FileSize < BufferSize ||
      FileSize <= DosHeader->e_lfanew ||
      DosHeader->e_magic != EFI_IMAGE_DOS_SIGNATURE) {
    return EFI_UNSUPPORTED;
  }

  //
  // check PE signature
  //
  Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)(FileBuffer + DosHeader->e_lfanew);
  if (Hdr.Pe32->Signature != EFI_IMAGE_NT_SIGNATURE) {
    return EFI_UNSUPPORTED;
  }

  Magic = Hdr.Pe32->OptionalHeader.Magic;

  //
  // Check PE32 or PE32+ magic
  //
  if (Magic != EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
      Magic != EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    return EFI_UNSUPPORTED;
  }

  NumberOfSections = (UINTN) (Hdr.Pe32->FileHeader.NumberOfSections);
  SectionHdr = (EFI_IMAGE_SECTION_HEADER *)((UINT8 *)Hdr.Pe32 +
                                             sizeof (UINT32) +
                                             sizeof (EFI_IMAGE_FILE_HEADER) +
                                             Hdr.Pe32->FileHeader.SizeOfOptionalHeader
                                             );
  StrLength = AsciiStrLen (SectionName);
  for (Index = 0; Index < NumberOfSections; Index++) {
    if (AsciiStrnCmp (SectionHdr[Index].Name, SectionName, StrLength) == 0) {
      CopyMem (SectionHeader, &SectionHdr[Index], sizeof (EFI_IMAGE_SECTION_HEADER));
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Get version type resource directory

  @param[in]  ResourceData       The pointer of resource data
  @param[out] VersionDir         The double pointer of version type resource directory
  @param[out] VersionDirEntry    The double pointer of version type resource directory entry

  @retval EFI_SUCCESS            Successfully get version type resource directory
  @retval EFI_INVALID_PARAMETER  Input pointer is NULL
  @retval EFI_NOT_FOUND          There is no version type resource directory
**/
EFI_STATUS
EFIAPI
GetVersionResourceDir (
  IN  UINT8                                 *ResourceData,
  OUT EFI_IMAGE_RESOURCE_DIRECTORY          **VersionDir,
  OUT EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY    **VersionDirEntry
  )
{
  EFI_IMAGE_RESOURCE_DIRECTORY              *Dir;
  EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY        *DirEntry;
  UINT16                                    Index;
  UINT16                                    Count;

  if (ResourceData == NULL || VersionDir == NULL || VersionDirEntry == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Dir      = (EFI_IMAGE_RESOURCE_DIRECTORY *) ResourceData;
  DirEntry = (EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY *) ((UINT8 *) Dir + sizeof (EFI_IMAGE_RESOURCE_DIRECTORY));

  Count = Dir->NumberOfNamedEntries + Dir->NumberOfIdEntries;
  for (Index = 0; Index < Count; Index++) {
    if (DirEntry[Index].u1.Id == RESOURCE_SECTION_RESOURCE_TYPE_VERSION &&
        DirEntry[Index].u2.s.DataIsDirectory) {
      *VersionDir      = (EFI_IMAGE_RESOURCE_DIRECTORY *) (ResourceData + DirEntry[Index].u2.s.OffsetToDirectory);
      *VersionDirEntry = (EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY *) ((UINT8 *) *VersionDir + sizeof (EFI_IMAGE_RESOURCE_DIRECTORY));
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Get the binary version number of Windows OS

  @param[in]  ResourceDir        The pointer of resource directory of image
  @param[in]  ResourceData       The pointer of resource data of image
  @param[out] VersionMS          The most significant 32 bits of the binary version number of Windows OS
  @param[out] VersionLS          The least significant 32 bits of the binary version number of Windows OS

  @retval EFI_SUCCESS            Successfully get binary version number
  @retval EFI_INVALID_PARAMETER  Input pointer is NULL or data size is zero
  @retval EFI_NOT_FOUND          There is no version number information in image
**/
EFI_STATUS
EFIAPI
GetWindowsOsVer (
  IN  EFI_IMAGE_DATA_DIRECTORY          *ResourceDir,
  IN  UINT8                             *ResourceData,
  OUT UINT32                            *VersionMS,
  OUT UINT32                            *VersionLS
  )
{
  EFI_STATUS                            Status;
  EFI_IMAGE_RESOURCE_DIRECTORY          *VersionDir;
  EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY    *VersionDirEntry;
  EFI_IMAGE_RESOURCE_DIRECTORY          *Dir;
  EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY    *DirEntry;
  EFI_IMAGE_RESOURCE_DATA_ENTRY         *DataEntry;
  UINT16                                Index;
  UINT16                                Count;
  UINT8                                 *Ptr;
  UINT8                                 *VersionInfo;
  CHAR16                                *KeyStr;
  UINT16                                MaxPaddingCount;
  VS_FIXEDFILEINFO                      *FixedFileInfo;

  if (ResourceDir == NULL || ResourceData == NULL || VersionMS == NULL || VersionLS == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetVersionResourceDir (ResourceData, &VersionDir, &VersionDirEntry);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Count = VersionDir->NumberOfNamedEntries + VersionDir->NumberOfIdEntries;
  for (Index = 0; Index < Count; Index++) {
    if (!VersionDirEntry[Index].u2.s.DataIsDirectory) {
      continue;
    }

    Dir      = (EFI_IMAGE_RESOURCE_DIRECTORY *) (ResourceData + VersionDirEntry[Index].u2.s.OffsetToDirectory);
    DirEntry = (EFI_IMAGE_RESOURCE_DIRECTORY_ENTRY *) ((UINT8 *) Dir + sizeof (EFI_IMAGE_RESOURCE_DIRECTORY));
    if (DirEntry->u2.s.DataIsDirectory) {
      continue;
    }

    DataEntry = (EFI_IMAGE_RESOURCE_DATA_ENTRY *) (ResourceData + DirEntry->u2.OffsetToData);
    if (DataEntry->OffsetToData < ResourceDir->VirtualAddress) {
      continue;
    }

    //
    // Get version info data entry VS_VERSIONINFO by comparing key string.
    // Key string is located after VersionInfoSize (UINT16), ValueSize (UINT16) and Type (UINT16).
    //
    VersionInfo = ResourceData + (DataEntry->OffsetToData - ResourceDir->VirtualAddress);
    KeyStr      = (CHAR16 *) (VersionInfo + sizeof (UINT16) + sizeof (UINT16) + sizeof (UINT16));
    if (StrCmp (KeyStr, L"VS_VERSION_INFO") != 0) {
      continue;
    }
    Ptr  = (UINT8 *) KeyStr;
    Ptr += StrSize (KeyStr);

    MaxPaddingCount = 3;
    while (*Ptr == 0 && MaxPaddingCount > 0) {
      Ptr++;
      MaxPaddingCount--;
    }
    if (MaxPaddingCount == 0) {
      continue;
    }

    FixedFileInfo = (VS_FIXEDFILEINFO *) Ptr;
    if (FixedFileInfo->Signature != VS_FIXEDFILEINFO_SIGNATURE) {
      continue;
    }

    *VersionMS = FixedFileInfo->ProductVersionMS;
    *VersionLS = FixedFileInfo->ProductVersionLS;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

BOOLEAN
EFIAPI
IsWindows (
  EFI_HANDLE                    Handle
  )
{
  EFI_STATUS                    Status;
  EFI_LOADED_IMAGE              *LoadedImage;
  EFI_IMAGE_DATA_DIRECTORY      ResourceEntry;
  EFI_IMAGE_SECTION_HEADER      SectionHeader;
  UINT32                        VersionMS;
  UINT32                        VersionLS;

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiLoadedImageProtocolGuid,
                  &LoadedImage
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  Status = GetImageDataDirectory (
             LoadedImage->ImageBase,
             LoadedImage->ImageSize,
             EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE,
             &ResourceEntry
             );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  Status = GetImageSectionHeader (
             LoadedImage->ImageBase,
             LoadedImage->ImageSize,
             ".rsrc",
             &SectionHeader
             );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  Status = GetWindowsOsVer (
             &ResourceEntry,
             (UINT8 *) LoadedImage->ImageBase + SectionHeader.VirtualAddress,
             &VersionMS,
             &VersionLS
             );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return TRUE;
}



