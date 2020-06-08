/** @file

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

#ifndef _SECURE_OPTION_ROM_CONTROL_DXE_H_
#define _SECURE_OPTION_ROM_CONTROL_DXE_H_

#include <IndustryStandard/Pci.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/VariableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/Decompress.h>
#include <Protocol/H2OSecurityArch.h>
#include <Protocol/PciIo.h>
#include <Protocol/Variable.h>
#include <Protocol/VariableLock.h>
#include <Protocol/Security2.h>
#include <Guid/SecureOptionRomControl.h>
#include <KernelSetupConfig.h>
#include "CommonDefinition.h"

/**
  Load the EFI Image from Option ROM

  @param[in]  PciIo         PCI IO protocol.
  @param[in]  FilePath      The file path of the EFI Image
  @param[out] BufferSize    On input the size of Buffer in bytes. On output with a return
                            code of EFI_SUCCESS, the amount of data transferred to Buffer.
                            On output with a return code of EFI_BUFFER_TOO_SMALL,
                            the size of Buffer required to retrieve the requested file.
  @param[in]  Buffer        The memory buffer to transfer the file to. If Buffer is NULL,
                            then no the size of the requested file is returned in BufferSize.

  @retval EFI_SUCCESS           The file was loaded.
  @retval EFI_INVALID_PARAMETER FilePath is not a valid device path, or
                                BufferSize is NULL.
  @retval EFI_NOT_FOUND         Not found PCI Option Rom on PCI device.
  @retval EFI_DEVICE_ERROR      Failed to decompress PCI Option Rom image.
  @retval EFI_BUFFER_TOO_SMALL  The BufferSize is too small to read the current directory entry.
                                BufferSize has been updated with the size needed to complete the request.
**/
EFI_STATUS
EFIAPI
LocalLoadFile (
  IN EFI_PCI_IO_PROTOCOL      *PciIo,
  IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer      OPTIONAL
  );

/**
  Build the OpRom#### option from the VariableName and the build OpRom#### will 
  also be linked to BdsCommonOptionList.

  @param[in,out] *OpRomList   - The header of the OpRom#### option link list
  @param[in]     OptionNumber - The number for the OpRom#### variable.  

  @retval OPTION_ROM_LIST_OPTION  - Get the option just been created
  @retval NULL                    - Failed to get the new option

**/
OPTION_ROM_LIST_OPTION *
EFIAPI
PciOpRomVariableToOption (
  IN OUT EFI_LIST_ENTRY               *OpRomList,
  IN     UINT16                       OptionNumber  
  );

/**
  This function uses PCI VendorID, DeviceID to get supported Option ROM type from option ROM list.

  @param[in]  VendorId               - PCI Vendor ID.
  @param[in]  DeviceId               - PCI Device ID.
  @param[out] SupportedPolicy        - Pointer to save supported option ROM type.

  @retval EFI_SUCCESS            - Get supported ROM image type successful.
  @retval EFI_NOT_FOUND          - Cannot find Supported type from option ROM list
  @retval EFI_INVALID_PARAMETER  - Any input parameter is invalid.

**/
EFI_STATUS
EFIAPI
GetOpRomPolicyForDevice (
  IN     UINT16                             VendorId,
  IN     UINT16                             DeviceId,   
  OUT    UINT8                              *SupportedPolicy
 );

/**
  According to vendor id and device id to check the option ROM policy exists or not.

  @param[in]  VendorId    - PCI Vendor ID.
  @param[in]  DeviceId    - PCI Device ID.
  @param[out] Option      - On return, a pointer to matched OPTION_ROM_LIST_OPTION instance.
                            If Option ROM policy doesn't exist, the option is set to NULL.

  @retval TRUE       - The option ROM policy exists.
  @retval FALSE      - The option ROM policy doesn't exists.

**/
BOOLEAN
EFIAPI
DoesOpRomPolicyExist (
  IN  UINT16                             VendorId,
  IN  UINT16                             DeviceId, 
  OUT OPTION_ROM_LIST_OPTION             **OpRomOption
  );

/**
  Get new Option ROM option number for OpRom#### variable

  @param NewOptionNum  - Pointer to new OP ROM option number

  @retval EFI_SUCCESS   - Get New OP ROM option number

**/
EFI_STATUS
EFIAPI
GetNewOpRomOptionNo (
  OUT UINT16             *NewOptionNum
  ) ;

/**
  
  Add Specific PCI Rom to option ROM list.  
  
  @param[in] VendorId         - PCI Vendor ID.
  @param[in] DeviceId         - PCI Device ID.
  @param[in] FoundRomType     - ROM type which contained in specific ROM image
  @param[in] OptionRomPolicy  - The option ROM policy for this type PCI option ROM
  @param[in] Active           - This option is active or not.

  @retval EFI_SUCCESS            - Add option ROM to option ROM list successful.
  @retval EFI_NOT_READY          - The option ROM list isn't initialized
  @retval EFI_INVALID_PARAMETER  - Any input parameter is invalid.

**/
EFI_STATUS
EFIAPI
AddRomImageToList (
  IN  UINT16                             VendorId,
  IN  UINT16                             DeviceId, 
  IN  UINT8                              FoundRomType,
  IN  UINT8                              OptionRomPolicy,
  IN  BOOLEAN                            Active
  );

/**
  
  This function uses option list to save OpRom#### and OpRomOrder variable
  
  @param None
  
  @retval EFI_SUCCESS         - Update OpRom#### and OpRomOrder variable Successful.
  @retval EFI_NOT_READY       - The option ROM list hasn't been initialized 

**/
EFI_STATUS
EFIAPI
UpdateOptionRomListToVariable (
  VOID
  );

/**
  
  This function uses to free all of allocated option ROM list
  
  @param None
  
  @retval EFI_SUCCESS         - All of allocated option ROM list has been freed.
  
**/
EFI_STATUS
EFIAPI
FreeOptionRomList (
  VOID
  );

/**
  
  This function uses PCI VendorID, DeviceID, and RomType to check if the Option ROM type is supported.
  
  @param[in]      VendorId               - PCI Vendor ID.
  @param[in]      DeviceId               - PCI Device ID.
  @param[in]      RomType                - The Rom Type of the image.
  @param[in,out]  RomSupported           - TRUE : The ROM type is supported.
                                         - FALSE: The ROM type is unsupported.
  
  @retval EFI_SUCCESS                    - Successfully checking if the ROM type is supported.
  
**/
EFI_STATUS
EFIAPI
CheckOpRomSupportPolicy (
  IN     UINT16                             VendorId,
  IN     UINT16                             DeviceId, 
  IN     UINT8                              RomType,
  IN OUT BOOLEAN                            *RomSupported
 );

/**
    
  According to Contained ROM type and supported ROM type to check this ROM is 
  supported by current policy.
  
  
  @param ContaindRomType   - All of the ROM type which contained in specific ROM image.
  @param SupportedRomType  - Supported ROM type for specific PCI device (Distingusished by 
                             Vendor ID and Device ID).
  
  @retval TRUE              - Platform can support this ROM image.
  @retval FALSE             - Platform cannot support this ROM image.
  
**/
BOOLEAN 
EFIAPI
IsSupportedRom (
  IN  UINT8     ContaindRomType,
  IN  UINT8     SupportedRomType
  );

/**
    
  Get the VendorID, DeviceID, and ROM type of this ROM from Device Path.
  
  @param[in]     DevicePath             - The file path of the Option ROM Image.
  @param[in,out] VendorId               - PCI Vendor ID.
  @param[in,out] DeviceId               - PCI Device ID.
  @param[in,out] RomType                - The Rom Type of the image.s 
  
  @retval EFI_SUCCESS                   Get the information of the option ROM.
  
**/
EFI_STATUS 
EFIAPI
GetInfoFromDevicePath (
  IN CONST EFI_DEVICE_PATH_PROTOCOL         *DevicePath,
  IN OUT UINT16                             *VendorId,
  IN OUT UINT16                             *DeviceId, 
  IN OUT UINT8                              *RomType 
  );

/**
    
  Get the ROM type of this ROM: Signed or Unsigned ROM type.
  
  @param[in]     PciIoDevice            - Pointer PCI_IO_DEVICE structure.
  @param[in]     EfiOpRomImageFilePath  - The file path of the EFI Image.
  @param[in,out] RomType                - The Rom Type of the image.s 
  
  @retval EFI_SUCCESS                Get the ROM type.
  
**/
EFI_STATUS 
EFIAPI
GetUefiRomType (
  IN     EFI_PCI_IO_PROTOCOL      *PciIo,  
  IN     EFI_DEVICE_PATH_PROTOCOL *EfiOpRomImageFilePath,
  IN OUT UINT8                    *RomType 
  ) ;

/**       
 Free the option ROM list.
 
 @retval EFI_SUCCESS                 
*/
EFI_STATUS
EFIAPI
SecurOptionRomControlReadyToBootCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );

/**

  @param  File             A pointer to the device path of the file that is
                           being dispatched. This will optionally be used for logging.
  @param  FileBuffer       A pointer to the buffer with the UEFI file image.
  @param  FileSize         The size of the file.
  @param  BootPolicy       A boot policy that was used to call LoadImage() UEFI service. If
                           FileAuthentication() is invoked not from the LoadImage(),
                           BootPolicy must be set to FALSE.
  
  @retval EFI_SUCCESS             The file specified by DevicePath and non-NULL
                                  FileBuffer did authenticate, and the platform policy dictates
                                  that the DXE Foundation may use the file.
  @retval EFI_SUCCESS             The device path specified by NULL device path DevicePath
                                  and non-NULL FileBuffer did authenticate, and the platform
                                  policy dictates that the DXE Foundation may execute the image in
                                  FileBuffer.
  @retval EFI_SUCCESS             FileBuffer is NULL and current user has permission to start
                                  UEFI device drivers on the device path specified by DevicePath.
  @retval EFI_SECURITY_VIOLATION  The file specified by DevicePath and FileBuffer did not

**/
H2O_SECURITY_ARCH_STATUS
EFIAPI
SecurOptionRomControlAuthenticate (
  IN CONST EFI_DEVICE_PATH_PROTOCOL               *DevicePath,
  IN UINT16                                       VendorId,
  IN UINT16                                       DeviceId,
  IN BOOLEAN                                      IsLegacyOpRom
  );


#endif
