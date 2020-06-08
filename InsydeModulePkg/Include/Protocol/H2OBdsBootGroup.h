/** @file
  This protocol extends BDS by allowing a driver to produce new groups of boot
  devices. These boot groups appear as a single boot device in the UEFI-defined
  boot order. However, when being processed by BDS, they can expand to a list
  of zero or more load options which will be processed in order as-if they
  appear in the same position in the boot order as the boot option.

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

#ifndef __H2O_BDS_BOOT_GROUP_PROTOCOL_H__
#define __H2O_BDS_BOOT_GROUP_PROTOCOL_H__


#define H2O_BDS_BOOT_GROUP_PROTOCOL_GUID \
  { \
    0x29e05736, 0x8d1c, 0x4565, {0xa4, 0xd8, 0x86, 0xbf, 0x1f, 0x03, 0xb7, 0x0e} \
  }

typedef struct _H2O_BDS_BOOT_GROUP_PROTOCOL  H2O_BDS_BOOT_GROUP_PROTOCOL;

#define H2O_BDS_BOOT_ORDER_DEFAULT       0x80000000
/**
  Return information about a boot group that will be automatically added to the boot order.

  @param[in]  This                    A pointer to this instance of the protocol.
  @param[in]  BdsServices             A pointer to H2O_BDS_SERVICES_PROTOCOL instance.
  @param[out] BootOptionArrayLen      Unsigned integer that specifies the number of entries in the
                                      BootOptionArray.
  @param[out] BootOptionArray         Pointer to an array of pointers to boot options.

  @retval EFI_SUCCESS                 Get Boot options successfully.
  @retval EFI_INVALID_PARAMETER       BdsServices is NULL or BootOptionArrayLen is NULL or BootOptionArray is NULL.
  @retval EFI_NOT_FOUND               Cannot find boot options.
  @retval EFI_OUT_OF_RESOURCES        Allocate memory to create BootOptionArray failed.
*/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_BOOT_GROUP_UPDATE_DEFAULT_BOOT_LIST)(
  IN  H2O_BDS_BOOT_GROUP_PROTOCOL   *This,
  IN  H2O_BDS_SERVICES_PROTOCOL     *BdsServices,
  OUT UINT32                        *BootOptionArrayLen,
  OUT H2O_BDS_LOAD_OPTION           **BootOptionArray
  );

/**
  This function returns the devices associated with a boot group.

  @param[in]  This                A Pointer to current instance of this protocol.
  @param[in]  BootGroup           Pointer to a boot option group. The VendorGuid in the DevicePath member
                                  will match the VendorGuid in the structure pointed to by This;
  @param[out] BootDeviceArrayLen  Unsigned integer that specifies the number of entries in the BootDeviceArray.
  @param[out] BootDeviceArray     Pointer to an array of pointers to boot device options.

  @retval EFI_SUCCESS              Get evices associated with a boot group successfully.
  @retval EFI_INVALID_PARAMETER    BootGroup is NULL or BootDeviceArrayLen is NULL or BootDeviceArray is NULL.
  @retval EFI_NOT_FOUND            Cannot find boot options.
  @retval EFI_OUT_OF_RESOURCES     Allocate memory to create BootOptionArray failed.
*/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_BOOT_GROUP_GET_GROUP_DEVICES)(
  IN        H2O_BDS_BOOT_GROUP_PROTOCOL   *This,
  IN  CONST H2O_BDS_LOAD_OPTION           *BootGroup,
  OUT       UINT32                        *BootDeviceArrayLen,
  OUT       H2O_BDS_LOAD_OPTION           **BootDeviceArray
  );

struct _H2O_BDS_BOOT_GROUP_PROTOCOL {
  UINT32                                          Size;
  EFI_GUID                                        VendorGuid;
  UINT32                                          DefaultBootListPriority;
  H2O_BDS_BOOT_GROUP_UPDATE_DEFAULT_BOOT_LIST     UpdateDefaultBootList;
  H2O_BDS_BOOT_GROUP_GET_GROUP_DEVICES            GetGroupDevices;
};

extern EFI_GUID gH2OBdsBootGroupProtocolGuid;

#endif
