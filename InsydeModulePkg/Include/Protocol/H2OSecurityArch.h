/** @file

  Definition of H2O Security Architectural protocol.
    
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


#ifndef __H2O_SECURITY_ARCHITECTURAL_H__
#define __H2O_SECURITY_ARCHITECTURAL_H__

///
/// Global ID for the H2O Security Architectural Protocol
///
#define H2O_SECURITY_ARCH_PROTOCOL_GUID \
  { 0x335ED5DE, 0x9194, 0x4FCC, {0x8C, 0x48, 0x75, 0xB5, 0x89, 0xA0, 0x75, 0xAC} }

//
// Define the ROM type which contained in ROM image
//
#define HAVE_LEGACY_ROM                      0x01
#define HAVE_UNSIGNED_EFI_ROM                0x02
#define HAVE_SIGNED_EFI_ROM                  0x04 

typedef enum {
  SEC_FORCE_SUCCESS = 0,
  SEC_FORCE_DENIED  = 1,
  SEC_DO_NOTHING    = 2
} H2O_SECURITY_ARCH_STATUS;

/**
  Check the input condition of the option ROM to determine if the option ROM is available.
  
  @param[in]  DevicePath       A pointer to the device path of the file that is
                               being dispatched.
  @param[in]  VendorId         Vendor ID for the PCI device of the dispatched option ROM.
  @param[in]  DeviceId         Device ID for the PCI device of the dispatched option ROM.
  @param[in]  IsLegacyOpRom    To determine the Option ROM type: Legacy or UEFI.
  
  @retval SEC_FORCE_SUCCESS       The Option ROM can be accessed.
  @retval SEC_FORCE_DENIED        The Option ROM can't be accessed.
  @retval SEC_DO_NOTHING          Don't match the condition to determine the access of the Option ROM.
  
**/
typedef 
H2O_SECURITY_ARCH_STATUS 
(EFIAPI *FILE_AUTHENTICATION) (  
  IN CONST EFI_DEVICE_PATH_PROTOCOL               *DevicePath, 
  IN UINT16                                       VendorId, 
  IN UINT16                                       DeviceId, 
  IN BOOLEAN                                      IsLegacyOpRom
);

typedef struct _H2O_SECURITY_ARCH_PROTOCOL {
  FILE_AUTHENTICATION FileAuthentication;
} H2O_SECURITY_ARCH_PROTOCOL;

extern EFI_GUID gH2OSecurityArchProtocolGuid;

#endif
