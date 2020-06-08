/** @file
  Checkpoint related functions
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

#include "InternalBdsLib.h"

/**
  Internal function to check whether the specific checkpoint is triggered.

  @param[in] Checkpoint    The number ID of checkpoint.

  @retval TRUE             The checkpoint has been triggered.
  @retval FALSE            The checkpoint hasn't been triggered.
**/
STATIC
BOOLEAN
IsCheckpointTriggered (
  IN  EFI_GUID *Checkpoint
  )
{
  EFI_STATUS       Status;
  VOID             *Interface;

  Status = gBS->LocateProtocol (
                  Checkpoint,
                  NULL,
                  (VOID **) &Interface
                  );
  return Status == EFI_SUCCESS ? TRUE : FALSE;
}

/**
  Internal function to remove all of previous installed checkpoint and then
  use previous installed checkpoint data to install checkpoint again.

  @param[in] Checkpoint    The number ID of checkpoint.

  @retval EFI_SUCCESS      Reinstall checkpoint successfully.
  @retval EFI_NOT_FOUND    The checkpoint hasn't been triggered.
**/
STATIC
EFI_STATUS
ReInstallCheckPoint (
  IN  EFI_GUID *CheckPoint
  )
{
  EFI_STATUS       Status;
  UINTN            NoHandles;
  EFI_HANDLE       *Buffer;
  EFI_HANDLE       Handle;
  UINTN            Index;
  VOID             *Interface;


  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  CheckPoint,
                  NULL,
                  &NoHandles,
                  &Buffer
                  );
  if (EFI_ERROR (Status) || NoHandles == 0) {
    return EFI_NOT_FOUND;
  }

  Interface = NULL;
  for (Index = 0; Index < NoHandles; Index++) {
    Status = gBS->HandleProtocol (
                    Buffer[Index],
                    CheckPoint,
                    (VOID **) &Interface
                    );
    ASSERT (!EFI_ERROR (Status));
    Status = gBS->UninstallProtocolInterface (
                    Buffer[Index],
                    CheckPoint,
                    Interface
                    );
    if (Index != NoHandles - 1) {
      FreePool (Interface);
    }
  }
  FreePool (Buffer);
  Handle = NULL;
  return gBS->InstallProtocolInterface (
                Handle,
                CheckPoint,
                EFI_NATIVE_INTERFACE,
                Interface
                );
}

/**
  Internal function to only initialize size member in checkpoint data and
  use this checkpoint data to trigger checkpoint.

  @param[in] BdsCheckpoint        The number ID of checkpoint.
  @param[in] DataSize             Checkpoint data size by bytes.

  @retval EFI_SUCCESS             Trigger checkpoint successfully.
  @retval EFI_INVALID_PARAMETER   BdsCheckpoint is NULL or DataSize is smaller than 4 bytes.
  @retval EFI_OUT_OF_RESOURCES    Allocate chekcpoint data failed.
**/
STATIC
EFI_STATUS
CommonTriggerCpFunction (
  IN   EFI_GUID   *BdsCheckpoint,
  IN   UINT32     DataSize
  )
{
  VOID         *Interface;
  EFI_HANDLE   Handle;

  if (BdsCheckpoint == NULL || DataSize < sizeof (UINT32)) {
    return EFI_INVALID_PARAMETER;
  }
  if (IsCheckpointTriggered (BdsCheckpoint)) {
    return ReInstallCheckPoint (BdsCheckpoint);
  }
  Interface = AllocateZeroPool (DataSize);
  if (Interface == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (Interface, &DataSize, sizeof (UINT32));
  Handle = NULL;
  return gBS->InstallProtocolInterface (
                &Handle,
                BdsCheckpoint,
                EFI_NATIVE_INTERFACE,
                Interface
                );
}

/**
  Initalize H2O_BDS_CP_BOOT_AFTER_PROTOCOL
  data and trigger gH2OBdsCpBootAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpBootAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpBootAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpBootAfter (
  VOID
  )
{
  return CommonTriggerCpFunction (
           &gH2OBdsCpBootAfterProtocolGuid,
           sizeof (H2O_BDS_CP_BOOT_AFTER_PROTOCOL)
           );
}

/**
  Initalize H2O_BDS_CP_BOOT_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpBootBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpBootBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpBootBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpBootBefore (
  VOID
  )
{
  return CommonTriggerCpFunction (
           &gH2OBdsCpBootBeforeProtocolGuid,
           sizeof (H2O_BDS_CP_BOOT_BEFORE_PROTOCOL)
           );
}

/**
  Initalize H2O_BDS_CP_CON_IN_CONNECT_AFTER_PROTOCOL
  data and trigger gH2OBdsCpConInConnectAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConInConnectAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConInConnectAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConInConnectAfter (
  VOID
  )
{
  return CommonTriggerCpFunction (
           &gH2OBdsCpConInConnectAfterProtocolGuid,
           sizeof (H2O_BDS_CP_CON_IN_CONNECT_AFTER_PROTOCOL)
           );
}

/**
  Initalize H2O_BDS_CP_CON_IN_CONNECT_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpConInConnectBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConInConnectBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConInConnectBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConInConnectBefore (
  VOID
  )
{
  return CommonTriggerCpFunction (
           &gH2OBdsCpConInConnectBeforeProtocolGuid,
           sizeof (H2O_BDS_CP_CON_IN_CONNECT_BEFORE_PROTOCOL)
           );
}

/**
  Initalize H2O_BDS_CP_CON_OUT_CONNECT_AFTER_PROTOCOL
  data and trigger gH2OBdsCpConOutConnectAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConOutConnectAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConOutConnectAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConOutConnectAfter (
  VOID
  )
{
  return CommonTriggerCpFunction (
           &gH2OBdsCpConOutConnectAfterProtocolGuid,
           sizeof (H2O_BDS_CP_CON_OUT_CONNECT_AFTER_PROTOCOL)
           );
}

/**
  Initalize H2O_BDS_CP_CON_OUT_CONNECT_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpConOutConnectBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConOutConnectBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConOutConnectBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConOutConnectBefore (
  VOID
  )
{
  return CommonTriggerCpFunction (
           &gH2OBdsCpConOutConnectBeforeProtocolGuid,
           sizeof (H2O_BDS_CP_CON_OUT_CONNECT_BEFORE_PROTOCOL)
           );
}

/**
  Initalize H2O_BDS_CP_READY_TO_BOOT_AFTER_PROTOCOL
  data and trigger gH2OBdsCpReadyToBootAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpReadyToBootAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpReadyToBootAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpReadyToBootAfter (
  VOID
  )
{
  return CommonTriggerCpFunction (
           &gH2OBdsCpReadyToBootAfterProtocolGuid,
           sizeof (H2O_BDS_CP_READY_TO_BOOT_AFTER_PROTOCOL)
           );
}

/**
  Initalize H2O_BDS_CP_READY_TO_BOOT_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpReadyToBootBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpReadyToBootBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpReadyToBootBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpReadyToBootBefore (
  VOID
  )
{
  return CommonTriggerCpFunction (
           &gH2OBdsCpReadyToBootBeforeProtocolGuid,
           sizeof (H2O_BDS_CP_READY_TO_BOOT_BEFORE_PROTOCOL)
           );
}