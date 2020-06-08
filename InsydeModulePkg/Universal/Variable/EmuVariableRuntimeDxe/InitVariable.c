/** @file
  Implment all four UEFI runtime variable services and
  install variable architeture protocol.

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

/**


Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Variable.h"

EFI_EVENT                   mVirtualAddressChangeEvent = NULL;

EFI_SMM_VARIABLE_PROTOCOL   *mSmmVariable;
ESAL_VARIABLE_GLOBAL        *mProtectedModeVariableModuleGlobal;
EFI_SMM_SYSTEM_TABLE2       *mSmst;

#define ECP_SMM_VARIABLE_PROTOCOL_GUID \
  { \
    0x5eb95b0a, 0xd085, 0x4a18, 0x9e, 0x41, 0x8e, 0xbd, 0xa7, 0x9f, 0x30, 0xc8 \
  }
EFI_GUID                            mEcpSmmVariableProtocolGuid = ECP_SMM_VARIABLE_PROTOCOL_GUID;

/*

  This code finds variable in storage blocks (Volatile or Non-Volatile).

  @param VariableName               Name of Variable to be found.
  @param VendorGuid                 Variable vendor GUID.
  @param Attributes                 Attribute value of the variable found.
  @param DataSize                   Size of Data found. If size is less than the
                                    data, this value contains the required size.
  @param Data                       Data pointer.

  @return EFI_INVALID_PARAMETER     Invalid parameter
  @return EFI_SUCCESS               Find the specified variable
  @return EFI_NOT_FOUND             Not found
  @return EFI_BUFFER_TO_SMALL       DataSize is too small for the result

**/
EFI_STATUS
EFIAPI
RuntimeServiceGetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  OUT UINT32       *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT VOID         *Data
  )
{
  return EmuGetVariable (
          VariableName,
          VendorGuid,
          Attributes OPTIONAL,
          DataSize,
          Data,
          &mVariableModuleGlobal->VariableGlobal[Physical]
          );
}

/**

  This code Finds the Next available variable.

  @param VariableNameSize           Size of the variable name
  @param VariableName               Pointer to variable name
  @param VendorGuid                 Variable Vendor Guid

  @return EFI_INVALID_PARAMETER     Invalid parameter
  @return EFI_SUCCESS               Find the specified variable
  @return EFI_NOT_FOUND             Not found
  @return EFI_BUFFER_TO_SMALL       DataSize is too small for the result

**/
EFI_STATUS
EFIAPI
RuntimeServiceGetNextVariableName (
  IN OUT UINTN     *VariableNameSize,
  IN OUT CHAR16    *VariableName,
  IN OUT EFI_GUID  *VendorGuid
  )
{
  return EmuGetNextVariableName (
          VariableNameSize,
          VariableName,
          VendorGuid,
          &mVariableModuleGlobal->VariableGlobal[Physical]
          );
}

/**

  This code sets variable in storage blocks (Volatile or Non-Volatile).

  @param VariableName                     Name of Variable to be found
  @param VendorGuid                       Variable vendor GUID
  @param Attributes                       Attribute value of the variable found
  @param DataSize                         Size of Data found. If size is less than the
                                          data, this value contains the required size.
  @param Data                             Data pointer

  @return EFI_INVALID_PARAMETER           Invalid parameter
  @return EFI_SUCCESS                     Set successfully
  @return EFI_OUT_OF_RESOURCES            Resource not enough to set variable
  @return EFI_NOT_FOUND                   Not found
  @return EFI_WRITE_PROTECTED             Variable is read-only

**/
EFI_STATUS
EFIAPI
RuntimeServiceSetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  IN UINT32        Attributes,
  IN UINTN         DataSize,
  IN VOID          *Data
  )
{
  return EmuSetVariable (
          VariableName,
          VendorGuid,
          Attributes,
          DataSize,
          Data,
          &mVariableModuleGlobal->VariableGlobal[Physical],
          mSmst == NULL ? &mVariableModuleGlobal->VolatileLastVariableOffset : &mProtectedModeVariableModuleGlobal->VolatileLastVariableOffset,
          mSmst == NULL ? &mVariableModuleGlobal->NonVolatileLastVariableOffset : &mProtectedModeVariableModuleGlobal->NonVolatileLastVariableOffset
          );
}

/**

  This code returns information about the EFI variables.

  @param Attributes                     Attributes bitmask to specify the type of variables
                                        on which to return information.
  @param MaximumVariableStorageSize     Pointer to the maximum size of the storage space available
                                        for the EFI variables associated with the attributes specified.
  @param RemainingVariableStorageSize   Pointer to the remaining size of the storage space available
                                        for EFI variables associated with the attributes specified.
  @param MaximumVariableSize            Pointer to the maximum size of an individual EFI variables
                                        associated with the attributes specified.

  @return EFI_INVALID_PARAMETER         An invalid combination of attribute bits was supplied.
  @return EFI_SUCCESS                   Query successfully.
  @return EFI_UNSUPPORTED               The attribute is not supported on this platform.

**/
EFI_STATUS
EFIAPI
RuntimeServiceQueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  return EmuQueryVariableInfo (
          Attributes,
          MaximumVariableStorageSize,
          RemainingVariableStorageSize,
          MaximumVariableSize,
          &mVariableModuleGlobal->VariableGlobal[Physical]
          );
}

/**
  Dummy GetVariable function.

  @param[in]  VariableName  Name of Variable to be found.
  @param[in]  VendorGuid    Variable vendor GUID.
  @param[out] Attributes    Attribute value of the variable found.
  @param[in]  DataSize      Size of Data found. If size is less than the
                            data, this value contains the required size.
  @param[out] Data          Data pointer.

  @retval EFI_UNSUPPORTED   Always return EFI_UNSUPPOTED.
**/
EFI_STATUS
EFIAPI
DummyGetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  OUT UINT32       *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT VOID         *Data
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Dummy GetNextVariableName function.

  @param[in,out] VariableNameSize  Size of the variable name
  @param[in,out] VariableName      Pointer to variable name
  @param[in,out] VendorGuid        Variable Vendor Guid

  @retval EFI_UNSUPPORTED   Always return EFI_UNSUPPOTED.
**/
EFI_STATUS
EFIAPI
DummyGetNextVariableName (
  IN OUT UINTN     *VariableNameSize,
  IN OUT CHAR16    *VariableName,
  IN OUT EFI_GUID  *VendorGuid
  )
{
  return EFI_UNSUPPORTED;
}

/**

  Dummy SetVariable function.

  @param[in] VariableName   Name of Variable to be found
  @param[in] VendorGuid     Variable vendor GUID
  @param[in] Attributes     Attribute value of the variable found
  @param[in] DataSize       Size of Data found. If size is less than the
                            data, this value contains the required size.
  @param[in] Data           Data pointer

  @retval EFI_UNSUPPORTED   Always return EFI_UNSUPPOTED.
**/
EFI_STATUS
EFIAPI
DummySetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  IN UINT32        Attributes,
  IN UINTN         DataSize,
  IN VOID          *Data
  )
{
  return EFI_UNSUPPORTED;
}

/**

  This code returns information about the EFI variables.

  @param[in]  Attributes                   Attributes bitmask to specify the type of variables
                                           on which to return information.
  @param[out] MaximumVariableStorageSize   Pointer to the maximum size of the storage space available
                                           for the EFI variables associated with the attributes specified.
  @param[out] RemainingVariableStorageSize Pointer to the remaining size of the storage space available
                                           for EFI variables associated with the attributes specified.
  @param[out] MaximumVariableSize          Pointer to the maximum size of an individual EFI variables
                                           associated with the attributes specified.

  @retval EFI_UNSUPPORTED   Always return EFI_UNSUPPOTED.
**/
EFI_STATUS
EFIAPI
DummyQueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  return EFI_UNSUPPORTED;
}


/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It convers pointer to new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
VariableClassAddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->PlatformLangCodes);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->LangCodes);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->PlatformLang);
  gRT->ConvertPointer (
    0x0,
    (VOID **) &mVariableModuleGlobal->VariableGlobal[Physical].NonVolatileVariableBase
    );
  gRT->ConvertPointer (
    0x0,
    (VOID **) &mVariableModuleGlobal->VariableGlobal[Physical].VolatileVariableBase
    );
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal);
}

/**
  Notification function of gEfiEventExitBootServicesGuid.

  This is a notification function registered on gEfiEventExitBootServicesGuid event.
  It convers pointer to new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT                            Event,
  IN VOID                                 *Context
  )
{
  mVariableModuleGlobal->AtRuntime = TRUE;
  gBS->CloseEvent (Event);
}


/**
  Callback function for legacy boot event.

  @param Event   Event whose notification function is being invoked.
  @param Context Pointer to the notification function's context.

**/
VOID
EFIAPI
LegacyBootCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  mSmmVariable->SmmGetVariable          = DummyGetVariable;
  mSmmVariable->SmmSetVariable          = DummySetVariable;
  mSmmVariable->SmmGetNextVariableName  = DummyGetNextVariableName;
  mSmmVariable->SmmQueryVariableInfo    = DummyQueryVariableInfo;
  gBS->CloseEvent (Event);
}

/**
  This function allows the caller to determine if UEFI ExitBootServices() has been called.

  This function returns TRUE after all the EVT_SIGNAL_EXIT_BOOT_SERVICES functions have
  executed as a result of the OS calling ExitBootServices().  Prior to this time FALSE
  is returned. This function is used by runtime code to decide it is legal to access
  services that go away after ExitBootServices().

  @retval  TRUE  The system has finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.
  @retval  FALSE The system has not finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.

**/
BOOLEAN
EFIAPI
EfiAtRuntime (
  VOID
  )
{
  return mSmst == NULL ? mVariableModuleGlobal->AtRuntime : mProtectedModeVariableModuleGlobal->AtRuntime;
}


/**
  Callback for SMM runtime

  @param    Event     Event whose notification function is being invoked.
  @param    Context   Pointer to the notification function's context.

  None

**/
VOID
EFIAPI
SmmRuntimeCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                      Status;
  EFI_SMM_RUNTIME_PROTOCOL        *SmmRT;
  EFI_SMM_RUNTIME_PROTOCOL        *SmSmmRT;
  EFI_HANDLE                      Handle;

  Status = gBS->LocateProtocol (&gEfiSmmRuntimeProtocolGuid, NULL, (VOID **)&SmmRT);
  if (EFI_ERROR (Status)) {
    return;
  }
  Status = SmmRT->LocateProtocol (
                    &gEfiSmmRuntimeProtocolGuid,
                    NULL,
                    (VOID **)&SmSmmRT
                    );
  if (EFI_ERROR (Status)) {
    return;
  }
  gBS->CloseEvent (Event);

  //
  // Since MdePkg defined gEfiSmmVariableProtocolGuid but the GUID and we defined in ECP are different,
  // using SmmRT to use mEcpSmmVariableProtocolGuid insatll SmmVarialbe services for ECP driver
  //
  Handle = NULL;
  Status = SmSmmRT->InstallProtocolInterface (
                    &Handle,
                    &mEcpSmmVariableProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    Context
                    );
  ASSERT_EFI_ERROR (Status);
  //
  // Use gBS to install mEcpSmmVariableProtocolGuid for ECP driver dependency compatible
  //
  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &mEcpSmmVariableProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  return;
}

/**
  Creates and returns a notification event and registers that event with all the protocol
  instances specified by ProtocolGuid.

  This function causes the notification function to be executed for every protocol of type
  ProtocolGuid instance that exists in the system when this function is invoked. In addition,
  every time a protocol of type ProtocolGuid instance is installed or reinstalled, the notification
  function is also executed. This function returns the notification event that was created.
  If ProtocolGuid is NULL, then ASSERT().
  If NotifyTpl is not a legal TPL value, then ASSERT().
  If NotifyFunction is NULL, then ASSERT().
  If Registration is NULL, then ASSERT().


  @param  ProtocolGuid    Supplies GUID of the protocol upon whose installation the event is fired.
  @param  NotifyTpl       Supplies the task priority level of the event notifications.
  @param  NotifyFunction  Supplies the function to notify when the event is signaled.
  @param  NotifyContext   The context parameter to pass to NotifyFunction.
  @param  Registration    A pointer to a memory location to receive the registration value.
                          This value is passed to LocateHandle() to obtain new handles that
                          have been added that support the ProtocolGuid-specified protocol.

  @return The notification event that was created.

**/
STATIC
EFI_EVENT
VariableCreateProtocolNotifyEvent(
  IN  EFI_GUID          *ProtocolGuid,
  IN  EFI_TPL           NotifyTpl,
  IN  EFI_EVENT_NOTIFY  NotifyFunction,
  IN  VOID              *NotifyContext,  OPTIONAL
  OUT VOID              **Registration
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  ASSERT (ProtocolGuid != NULL);
  ASSERT (NotifyFunction != NULL);
  ASSERT (Registration != NULL);

  //
  // Create the event
  //

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  NotifyTpl,
                  NotifyFunction,
                  NotifyContext,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for protocol notifications on this event
  //

  Status = gBS->RegisterProtocolNotify (
                  ProtocolGuid,
                  Event,
                  Registration
                  );

  ASSERT_EFI_ERROR (Status);

  return Event;
}


/**
  EmuVariable Driver main entry point. The Variable driver places the 4 EFI
  runtime services in the EFI System Table and installs arch protocols
  for variable read and write services being available. It also registers
  notification function for EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       Variable service successfully initialized.

**/
EFI_STATUS
EFIAPI
VariableServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_HANDLE                  NewHandle;
  EFI_STATUS                  Status;
  EFI_SMM_BASE2_PROTOCOL      *SmmBase;
  BOOLEAN                     InSmm;
  EFI_EVENT                   Event;
  EFI_SMM_RUNTIME_PROTOCOL    *SmmRT;
  VOID                        *Registration;

  if (PcdGetBool (PcdUseFastCrisisRecovery)) {
    if (GetBootModeHob () != BOOT_IN_RECOVERY_MODE) {
      return EFI_UNSUPPORTED;
    }
  }
  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **)&SmmBase
                  );
  InSmm = FALSE;
  if (!EFI_ERROR (Status)) {
    SmmBase->InSmm (SmmBase, &InSmm);
  }

  if (!InSmm) {

    if (!IsRuntimeDriver (ImageHandle)) {
       Status = RelocateImageToRuntimeDriver (ImageHandle);
       ASSERT_EFI_ERROR (Status);
        //
        // We only want to load runtime services code to memory and don't load boot services code to memory,
        // so just return EFI_ALREADY_STARTED if it isn't a runtime driver.
        //
        return EFI_ALREADY_STARTED;
    }
    Status = VariableCommonInitialize (ImageHandle, SystemTable);
    ASSERT_EFI_ERROR (Status);

    SystemTable->RuntimeServices->GetVariable         = RuntimeServiceGetVariable;
    SystemTable->RuntimeServices->GetNextVariableName = RuntimeServiceGetNextVariableName;
    SystemTable->RuntimeServices->SetVariable         = RuntimeServiceSetVariable;
    SystemTable->RuntimeServices->QueryVariableInfo   = RuntimeServiceQueryVariableInfo;

    //
    // Now install the Variable Runtime Architectural Protocol on a new handle
    //
    NewHandle = NULL;
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &NewHandle,
                    &gEfiVariableArchProtocolGuid,
                    NULL,
                    &gEfiVariableWriteArchProtocolGuid,
                    NULL,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);

    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    VariableClassAddressChangeEvent,
                    NULL,
                    &gEfiEventVirtualAddressChangeGuid,
                    &mVirtualAddressChangeEvent
                    );
    ASSERT_EFI_ERROR (Status);
    //
    // Note!!!
    // This driver is SMM and RuntimeDxe hybrid driver and UefiRuntimeLib creates ExitBootServices and
    // VariableAddressChange event in constructor, so we cannot use this library to prevent from system
    // calls function located in SMM RAM in protected mode when ExitBootServices event is signaled.
    //
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    ExitBootServicesEvent,
                    NULL,
                    &gEfiEventExitBootServicesGuid,
                    &Event
                    );
    ASSERT_EFI_ERROR (Status);

    NewHandle = NULL;
    Status = gBS->InstallProtocolInterface (
                  &NewHandle,
                  &gEfiEmuNonVolatileVariableProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  mVariableModuleGlobal
                  );

    Status = EfiCreateEventLegacyBootEx (
               TPL_NOTIFY,
               LegacyBootCallback,
               NULL,
               &Event
               );

  } else {
    //
    // Get Smm Syatem Table
    //
    Status = SmmBase->GetSmstLocation(
                        SmmBase,
                        &mSmst
                        );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    mVariableModuleGlobal = AllocateRuntimeZeroPool (sizeof (ESAL_VARIABLE_GLOBAL));
    if (mVariableModuleGlobal == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Status = gBS->LocateProtocol (
                    &gEfiEmuNonVolatileVariableProtocolGuid,
                    NULL,
                    (VOID **)&mProtectedModeVariableModuleGlobal
                  );
    CopyMem (mVariableModuleGlobal, mProtectedModeVariableModuleGlobal, sizeof (ESAL_VARIABLE_GLOBAL));

    mSmmVariable = AllocateRuntimeZeroPool (sizeof (EFI_SMM_VARIABLE_PROTOCOL));
    if (mSmmVariable == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    mSmmVariable->SmmGetVariable          = RuntimeServiceGetVariable;
    mSmmVariable->SmmSetVariable          = RuntimeServiceSetVariable;
    mSmmVariable->SmmGetNextVariableName  = RuntimeServiceGetNextVariableName;
    mSmmVariable->SmmQueryVariableInfo    = RuntimeServiceQueryVariableInfo;

    NewHandle = NULL;
    Status = mSmst->SmmInstallProtocolInterface (
                      &NewHandle,
                      &gEfiSmmVariableProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      mSmmVariable
                      );
    //
    // For compatible with native EDKII variable driver, install gEfiSmmVariableProtocolGuid in gBS.
    //
    NewHandle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &NewHandle,
                    &gEfiSmmVariableProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    mSmmVariable
                  );
    Status = gBS->LocateProtocol (&gEfiSmmRuntimeProtocolGuid, NULL, (VOID **)&SmmRT);
    if (!EFI_ERROR(Status)) {
      SmmRuntimeCallback (NULL, mSmmVariable);
    } else {
      Event = VariableCreateProtocolNotifyEvent (
                &gEfiSmmRuntimeProtocolGuid,
                TPL_CALLBACK,
                SmmRuntimeCallback,
                mSmmVariable,
                &Registration
                );
    }

  }
  return EFI_SUCCESS;
}
