/** @file
  This protocol is produced by the Boot Manager during the BDS phase, prior to
  launching any hot key handlers or boot loaders. It is designed to give access
  to the current working state of the Boot Manager, such as the current working
  boot order, the boot mode, and the screen condition. It also allows certain
  behaviors to be triggered, such as boot-fail or boot-success.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef __H2O_BDS_SERVICES_PROTOCOL_H__
#define __H2O_BDS_SERVICES_PROTOCOL_H__


#define H2O_BDS_SERVICES_PROTOCOL_GUID \
  { \
    0x8bfff2c1, 0xbb8, 0x47fd, {0x9f, 0x6a, 0xae, 0xc1, 0x1, 0xac, 0xbe, 0x6e} \
  }

typedef struct _H2O_BDS_SERVICES_PROTOCOL  H2O_BDS_SERVICES_PROTOCOL;


#define DRIVER_OPTION          FALSE
#define BOOT_OPTION            TRUE

#define BOOT_PREVIOUS_LOAD_ORDER  0xFFFD
#define BOOT_CURRENT_LOAD_ORDER   0xFFFE
#define BOOT_NEXT_LOAD_ORDER      0xFFFF


#define H2O_BOOT_PREVIOUS_VARIABLE_NAME                L"BootPrev"

//
// H2O_BDS_LOAD_OPTION related definitions
//
#define H2O_BDS_LOAD_OPTION_SIGNATURE SIGNATURE_32 ('H', 'B', 'L', 'O')

typedef struct _H2O_BDS_LOAD_OPTION {
  UINT32                    Signature;              ///< Particular statement signature. This signature must be 'H', 'B', 'L', 'O'
  LIST_ENTRY                Link;                   ///< Linked entry to the next and previous option in the list.
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;            ///< Pointer to a device path. The device path may be multi-instance.
  BOOLEAN                   Connected;              ///< Boolean that specifies whether the Boot Manager has connected this device path (TRUE) or not (FALSE).
  BOOLEAN                   DriverOrBoot;           ///< Boolean that specifies whether this load option represents a Driver load option (FALSE) or Boot load option (TRUE).
  UINT16                    LoadOrder;              ///< Unsigned integer that specifies the current boot option being booted. Corresponds to the four hexadecimal digits in
                                                    ///< the #### portion of the UEFI variable name Boot#### or Driver####. For BootNext, this will be initialized 0xFFFF.
  CHAR16                    LoadOrderVarName[11];   ///< Null-terminated string that contains the Boot#### or Driver#### variable name for this boot option (or BootNext).
  EFI_GUID                  LoadOrderVarGuid;       ///< A unique identifier for the load order variable.
  BOOLEAN                   Expanded;               ///< Boolean that specifies whether the Boot Manager has tried to expand this device path (TRUE) or not (FALSE).
  LIST_ENTRY                ExpandedLoadOptions;    ///< If Expanded is TRUE, then this linked list specifies zero or more H2O_BDS_LOAD_OPTION structures that represent the
                                                    ///< expanded boot options. Each of the boot options in this list will refer to a single device.
  UINT32                    Attributes;             ///< Bitmask that specifies load option attributes. Values are specified in section 3.1.3 of the UEFI specification.
  CHAR16                    *Description;           ///< Pointer to a null-terminated string associated with the load option.
  VOID                      *LoadOptionalData;      ///< Pointer to optional data associated with the load option.
  UINT32                    LoadOptionalDataSize;   ///< Unsigned integer that specifies the number of bytes pointed to by LoadOptionalData.
  CHAR16                    *StatusString;          ///< Pointer to null-terminated user-readable string that indicates the status of an attempt to launch this boot option or
                                                    ///< NULL if launch has not been tried.
} H2O_BDS_LOAD_OPTION;

#define BDS_OPTION_FROM_EXPAND_LINK(a)  CR (a, H2O_BDS_LOAD_OPTION, ExpandedLoadOptions, H2O_BDS_LOAD_OPTION_SIGNATURE)
#define BDS_OPTION_FROM_LINK(a)         CR (a, H2O_BDS_LOAD_OPTION, Link, H2O_BDS_LOAD_OPTION_SIGNATURE)

/**
  Return the current value of the OsIndications and OsIndicationsSupported UEFI variable.

  @param[in]  This                    A Pointer to current instance of this protocol.
  @param[out] OsIndications           A pointer to contain the value of the OsIndications.
  @param[out] OsIndicationsSupported  A pointer to contain the value of the OsIndicationsSupported.

  @retval EFI_SUCCESS            Get OsIndications and OsIndicationsSupported successfully.
  @retval EFI_INVALID_PARAMETER  OsIndications or OsIndicationsSupported is NULL.
  @retval EFI_NOT_FOUND          Cannot find OsIndications or OsIndicationsSupported value.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_GET_OS_INDICATIONS)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  UINT64                      *OsIndications,
  OUT  UINT64                      *OsIndicationsSupported
  );

/**
  Return the current value of the OS loader timeout.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[out] Timeout            A pointer to contain the value of the timeout.

  @retval EFI_SUCCESS            Get Timeout value successfully.
  @retval EFI_INVALID_PARAMETER  Timeout is NULL.
  @retval EFI_NOT_FOUND          Cannot find Timeout value.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_GET_TIMEOUT)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  UINT16                      *Timeout
  );

/**
  Return the current boot mode, such as S4 resume, diagnostics, full configuration, etc.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BootMode           A pointer to contain the value of the boot mode.

  @retval EFI_SUCCESS            Get BootMode successfully.
  @retval EFI_INVALID_PARAMETER  BootMode is NULL.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_GET_BOOT_MODE)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  EFI_BOOT_MODE               *BootMode
  );

/**
  Return the current boot type, uch as legacy, UEFI or dual.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BootType           A pointer to contain the value of the boot mode.

  @retval EFI_SUCCESS            Get BootType successfully.
  @retval EFI_INVALID_PARAMETER  BootType is NULL.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_GET_BOOT_TYPE)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  UINT8                       *BootType
  );

/**
  Return linked list of BDS boot options derived from the BootOrder and Boot#### UEFI variables.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[out] BootList           Pointer to linked list of BDS boot options derived from the BootOrder and Boot#### UEFI variables.

  @retval EFI_SUCCESS            Get boot list successfully.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_GET_BOOT_LIST)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  LIST_ENTRY                  **BootList
  );

/**
  Return linked list of BDS boot options derived from the DriverOrder and Driver#### UEFI variables.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[out] DriverList         Pointer to linked list of BDS boot options derived from the DriverOrder and Driver#### UEFI variables.

  @retval EFI_SUCCESS            Get driver list successfully.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_GET_DRIVER_LIST)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  LIST_ENTRY                  **DriverList
  );

/**
  Create a BDS load option in a buffer allocated from pool.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  DriverOrBoot       Boolean that specifies whether the Boot Manager has connected this device
                                 path (TRUE) or not (FALSE).
  @param[in]  OptionName         A Null-terminated string that is the name of the vendor's variable.
  @param[in]  OptionGuid         A unique identifier for the vendor.
  @param[in]  Attributes         The attributes for this load option entry.
  @param[in]  DevicePath         A Pointer to a packed array of UEFI device paths.
  @param[in]  Description        The user readable description for the load option.
  @param[in]  OptionalData       A Pointer to optional data for load option.
  @param[in]  OptionalDataSize   The size by bytes of optional data.
  @param[out] LoadOption         Dynamically allocated memory that contains a new created H2O_BDS_LOAD_OPTION
                                 instance. Caller is responsible freeing the buffer

  @retval EFI_SUCCESS            Create load option successfully.
  @retval EFI_INVALID_PARAMETER  DevicePath is NULL or LoadOption is NULL.
  @retval EFI_INVALID_PARAMETER  The OptionName is correct boot#### or driver#### variable name.
  @retval EFI_OUT_OF_RESOURCES   Allocate memory to create H2O_BDS_LOAD_OPTION failed.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_CREATE_LOAD_OPTION)(
  IN        H2O_BDS_SERVICES_PROTOCOL    *This,
  IN        BOOLEAN                      DriverOrBoot,
  IN CONST  CHAR16                       *OptionName OPTIONAL,
  IN CONST  EFI_GUID                     *OptionGuid OPTIONAL,
  IN        UINT32                       Attributes,
  IN CONST  EFI_DEVICE_PATH_PROTOCOL     *DevicePath,
  IN CONST  CHAR16                       *Description OPTIONAL,
  IN CONST  UINT8                        *OptionalData OPTIONAL,
  IN        UINT32                       OptionalDataSize,
  OUT       H2O_BDS_LOAD_OPTION          **LoadOption
  );

/**
  Free the memory associated with a BDS load option.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  LoadOption         The allocated H2O_BDS_LOAD_OPTION instance to free.

  @retval EFI_SUCCESS            Get BootType successfully.
  @retval EFI_INVALID_PARAMETER  LoadOption is NULL.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_FREE_LOAD_OPTION)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *LoadOption
  );

/**
  Converts a UEFI variable formatted as a UEFI load option to a BDS load option.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  VariableName       A Null-terminated string that is the name of the vendor's variable.
  @param[in]  VariableGuid       A unique identifier for the vendor.
  @param[out] BdsLoadOption      Dynamically allocated memory that contains a new created H2O_BDS_LOAD_OPTION
                                 instance. Caller is responsible freeing the buffer

  @retval EFI_SUCCESS            Convert BDS load option to UEFI load option successfully.
  @retval EFI_INVALID_PARAMETER  VariableName is NULL VariableGuid is NULL or BdsLoadOption is NULL.
  @retval EFI_NOT_FOUND          Variable doesn't exist.
  @retval EFI_OUT_OF_RESOURCES   Allocate memory to create H2O_BDS_LOAD_OPTION failed.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_VAR_TO_LOAD_OPTION)(
  IN        H2O_BDS_SERVICES_PROTOCOL   *This,
  IN  CONST CHAR16                      *VariableName,
  IN  CONST EFI_GUID                    *VariableGuid,
  OUT       H2O_BDS_LOAD_OPTION         **BdsLoadOption
  );

/**
  Converts a BDS load option in a UEFI variable formatted as a UEFI load option.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BdsLoadOption      A pointer to BDS load option.
  @param[out] VariableName       A Null-terminated string that is the name of the vendor's variable.
                                 Caller is responsible freeing the buffer.
  @Param[out] VariableGuid       A unique identifier for the vendor.

  @retval EFI_SUCCESS            Convert UEFI load option to BDS load option successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL, VariableName is NULL or VariableGuid is NULL.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption format is incorrect.
  @retval EFI_OUT_OF_RESOURCES   Allocate memory to contain variable name is failed.
  @retval Others                 Any other error occurred in this function.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_LOAD_OPTION_TO_VAR)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption,
  OUT  CHAR16                      **VariableName,
  OUT  EFI_GUID                    *VariableGuid
  );

/**
  Insert a BDS load option into either the Driver or Boot order.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BdsLoadOption      A pointer to BDS load option.

  @retval EFI_SUCCESS            Insert a BDS load option into either the Driver or Boot order successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption format is incorrect.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_INSERT_LOAD_OPTION)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption
  );

/**
  Remove a BDS load option from either the Driver or Boot order.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  DriverOrBoot       Boolean that specifies whether this load option represents a Driver load option
                                 (FALSE) or Boot load option (TRUE).
  @param[in]  LoadOrder          Unsigned integer that specifies the current boot option being booted. Corresponds to the four
                                 hexadecimal digits in the #### portion of the UEFI variable name Boot#### or Driver####.

  @retval EFI_SUCCESS            Remove a BDS load option from either the Driver or Boot order successfully.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_REMOVE_LOAD_OPTION)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   BOOLEAN                     DriverOrBoot,
  IN   UINT16                      LoadOrder
  );

/**
  Expand a partial load option to one or more fully qualified load options using the rules specified in the
  UEFI specification (USB WWID, hard disk) and the InsydeH2O boot groups.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BdsLoadOption      A pointer to BDS load option.

  @retval EFI_SUCCESS            Expand load option successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption format is incorrect.
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_EXPAND_LOAD_OPTION)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption
  );

/**
  Launch a BDS load option.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BdsLoadOption      A pointer to BDS load option.
  @param[out] ExitDataSize       Pointer to the size, in bytes, of ExitData.
  @param[out] ExitData           Pointer to a pointer to a data buffer that includes a Null-terminated
                                 string, optionally followed by additional binary data.

  @retval EFI_SUCCESS            Boot from the input boot option successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption format is incorrect.
  @retval EFI_NOT_FOUND          If the Device Path is not found in the system
**/
typedef
EFI_STATUS
(EFIAPI *H2O_BDS_SERVICES_LAUNCH_LOAD_OPTION)(
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption,
  OUT  UINTN                       *ExitDataSize,
  OUT  CHAR16                      **ExitData OPTIONAL
  );

struct _H2O_BDS_SERVICES_PROTOCOL {
  UINT32                                Size;
  H2O_BDS_SERVICES_GET_OS_INDICATIONS   GetOsIndications;
  H2O_BDS_SERVICES_GET_TIMEOUT          GetTimeout;
  H2O_BDS_SERVICES_GET_BOOT_MODE        GetBootMode;
  H2O_BDS_SERVICES_GET_BOOT_TYPE        GetBootType;
  H2O_BDS_LOAD_OPTION                   *BootCurrentLoadOption;
  CONST H2O_BDS_LOAD_OPTION             *BootNextLoadOption;
  H2O_BDS_LOAD_OPTION                   *BootPrevLoadOption;
  H2O_BDS_SERVICES_GET_BOOT_LIST        GetBootList;
  H2O_BDS_SERVICES_GET_DRIVER_LIST      GetDriverList;
  H2O_BDS_SERVICES_CREATE_LOAD_OPTION   CreateLoadOption;
  H2O_BDS_SERVICES_FREE_LOAD_OPTION     FreeLoadOption;
  H2O_BDS_SERVICES_VAR_TO_LOAD_OPTION   ConvertVarToLoadOption;
  H2O_BDS_SERVICES_LOAD_OPTION_TO_VAR   ConvertLoadOptionToVar;
  H2O_BDS_SERVICES_INSERT_LOAD_OPTION   InsertLoadOption;
  H2O_BDS_SERVICES_REMOVE_LOAD_OPTION   RemoveLoadOption;
  H2O_BDS_SERVICES_EXPAND_LOAD_OPTION   ExpandLoadOption;
  H2O_BDS_SERVICES_LAUNCH_LOAD_OPTION   LaunchLoadOption;
};

extern EFI_GUID gH2OBdsServicesProtocolGuid;

#endif
