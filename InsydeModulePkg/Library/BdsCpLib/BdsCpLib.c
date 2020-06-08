/** @file
  Bds check point library. It provides functionalities to register, unregister
  and trigger check point and also has function to get check point information
  from handle.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/BdsCpLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>

#include <Protocol/SmmBase2.h>

#define H2O_BDS_CP_SIGNATURE                    SIGNATURE_32 ('H', 'B', 'C', 'P')

typedef struct {
  UINT32         Signature;
  LIST_ENTRY     Link;
  EFI_GUID       Guid;
  EFI_EVENT      Event;
} INTERNAL_BDS_CP_HANDLE;

#define BDS_CP_NODE_FROM_LINK(a)  CR (a, INTERNAL_BDS_CP_HANDLE, Link, H2O_BDS_CP_SIGNATURE)

STATIC LIST_ENTRY      *mCpList;
STATIC EFI_GUID        mInternalBdsCpListHeadGuid = {0xd1d417d5, 0x5264, 0x4369, 0xa3, 0x83, 0xf5, 0xf3, 0xc6, 0x19, 0x49, 0x28};

/**
  This function registers a handler for the specified checkpoint with the specified priority.

  @param[in]  BdsCheckpoint     Pointer to a GUID that specifies the checkpoint for which the
                                handler is being registered.
  @param[in]  Handler           Pointer to the handler function.
  @param[in]  Priority          Enumerated value that specifies the priority with which the function
                                will be associated.
  @param[out] Handle            Pointer to the returned handle that is associated with the newly
                                registered checkpoint handler.

  @retval EFI_SUCCESS           Register check point handle successfully.
  @retval EFI_INVALID_PARAMETER BdsCheckpoint ,Handler or Handle is NULL.
  @retval EFI_OUT_OF_RESOURCES  Allocate memory for Handle failed.
**/
EFI_STATUS
BdsCpRegisterHandler (
  IN  CONST EFI_GUID      *BdsCheckpoint,
  IN  H2O_BDS_CP_HANDLER  Handler,
  IN  H2O_BDS_CP_PRIORITY Priority,
  OUT H2O_BDS_CP_HANDLE   *Handle
  )
{
  EFI_EVENT               Event;
  EFI_STATUS              Status;
  VOID                    *Registration;
  INTERNAL_BDS_CP_HANDLE  *CpHandle;

  if (BdsCheckpoint == NULL || Handler == NULL || Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CpHandle = AllocateZeroPool (sizeof (INTERNAL_BDS_CP_HANDLE));
  if (CpHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  Priority,
                  Handler,
                  CpHandle,
                  &Event
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->RegisterProtocolNotify (
                  (EFI_GUID *) BdsCheckpoint,
                  Event,
                  &Registration
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CpHandle->Signature = H2O_BDS_CP_SIGNATURE;
  CopyGuid (&CpHandle->Guid, BdsCheckpoint);
  CpHandle->Event = Event;
  InsertTailList (mCpList, &CpHandle->Link);
  *Handle = (VOID *) CpHandle;
  return EFI_SUCCESS;
}

/**
  This function returns the checkpoint data structure that was installed when the checkpoint was
  triggered and, optionally, the GUID that was associated with the checkpoint.

  @param[in]  Handle            The handle associated with a previously registered checkpoint
                                handler.
  @param[out] BdsCheckpointData The pointer to the checkpoint structure that was installed.
  @param[out] BdsCheckpoint     Optional pointer to the returned pointer to the checkpoint GUID.

  @retval EFI_SUCCESS           Get check point data successfully.
  @retval EFI_INVALID_PARAMETER Handle or BdsCheckpointData is NULL or Handle is invalid.
  @retval EFI_INVALID_PARAMETER It does not refer to a previously registered checkpoint handler.
  @return Others                Other error occurred while getting check point information.
**/
EFI_STATUS
BdsCpLookup (
  IN  H2O_BDS_CP_HANDLE Handle,
  OUT VOID              **BdsCheckpointData,
  OUT EFI_GUID          *BdsCheckpoint       OPTIONAL
  )
{
  LIST_ENTRY              *List;
  INTERNAL_BDS_CP_HANDLE  *CpHandle;
  INTERNAL_BDS_CP_HANDLE  *CurrentCpHandle;
  EFI_STATUS              Status;
  UINTN                   BufferSize;

  if (Handle == NULL || BdsCheckpointData == NULL || IsListEmpty (mCpList)) {
    return EFI_INVALID_PARAMETER;
  }

  CpHandle = (INTERNAL_BDS_CP_HANDLE *) Handle;
  if (CpHandle->Signature != H2O_BDS_CP_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  List = GetNextNode (mCpList, mCpList);
  while (List != mCpList) {
    CurrentCpHandle = BDS_CP_NODE_FROM_LINK (List);
    if (CpHandle->Event == CurrentCpHandle->Event) {
      //
      // Should not have multiple protocol instances,
      //
      BufferSize = 0;
      Status = gBS->LocateHandle (
                      ByProtocol,
                      &CpHandle->Guid,
                      NULL,
                      &BufferSize,
                      NULL
                      );
      if (Status != EFI_BUFFER_TOO_SMALL || BufferSize / sizeof (EFI_HANDLE) != 1) {
        return EFI_INVALID_PARAMETER;
      }

      Status = gBS->LocateProtocol (
                      &CpHandle->Guid,
                      NULL,
                      BdsCheckpointData
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (BdsCheckpoint != NULL) {
        CopyGuid (BdsCheckpoint, &CpHandle->Guid);
      }
      return EFI_SUCCESS;
    }
    List = GetNextNode (mCpList, List);
  }

  return EFI_INVALID_PARAMETER;
}


/**
  This function install checks point data to check point GUID and triggers check point
  according to check point GUID.

  @param[in] BdsCheckpoint      Pointer to the GUID associated with the BDS checkpoint.
  @param[in] BdsCheckpointData  Pointer to the data associated with the BDS checkpoint.


  @retval EFI_SUCCESS           Trigger check point successfully.
  @retval EFI_INVALID_PARAMETER BdsCheckpoint or BdsCheckpointData is NULL.
  @retval Other                 Install BdsCheckpoint protocol failed.
**/
EFI_STATUS
BdsCpTrigger (
  IN CONST EFI_GUID *BdsCheckpoint,
  IN CONST VOID     *BdsCheckpointData
  )
{
  EFI_HANDLE          Handle;

  if (BdsCheckpoint == NULL || BdsCheckpointData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Handle = NULL;
  return gBS->InstallProtocolInterface (
                  &Handle,
                  (EFI_GUID *) BdsCheckpoint,
                  EFI_NATIVE_INTERFACE,
                  (VOID *) BdsCheckpointData
                  );
}

/**
  This function unregisters the handle and frees any associated resources.

  @param[in] Handle             The handle that is associated with the registered checkpoint handler.

  @retval EFI_SUCCESS           The function completed successfully.
  @return EFI_INVALID_PARAMETER Handle is NULL, Handle is invalid or does not refer to a previously
                                registered checkpoint handler.
**/
EFI_STATUS
BdsCpUnregisterHandler (
  IN H2O_BDS_CP_HANDLE Handle
  )
{
  LIST_ENTRY              *List;
  INTERNAL_BDS_CP_HANDLE  *CpHandle;
  INTERNAL_BDS_CP_HANDLE  *CurrentCpHandle;

  if (Handle == NULL || IsListEmpty (mCpList)) {
    return EFI_INVALID_PARAMETER;
  }

  CpHandle = (INTERNAL_BDS_CP_HANDLE *) Handle;
  if (CpHandle->Signature != H2O_BDS_CP_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  List = GetNextNode (mCpList, mCpList);
  while (List != mCpList) {
    CurrentCpHandle = BDS_CP_NODE_FROM_LINK (List);
    if (CpHandle->Event == CurrentCpHandle->Event) {
      gBS->CloseEvent (CpHandle->Event);
      RemoveEntryList (List);
      FreePool (Handle);
      return EFI_SUCCESS;
    }
    List = GetNextNode (mCpList, List);
  }

  return EFI_INVALID_PARAMETER;
}

/**
  Detect whether the system is in SMM mode.

  @retval TRUE                  System is at SMM mode.
  @retval FALSE                 System is not at SMM mode.
**/
STATIC
BOOLEAN
IsInSmm (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_SMM_BASE2_PROTOCOL         *SmmBase;
  BOOLEAN                        InSmm;

  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **) &SmmBase
                  );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  InSmm = FALSE;
  SmmBase->InSmm (SmmBase, &InSmm);
  return InSmm;
}

/**
  The constructor function saves the address of check point address in volatile variable.
  And then all of library instances can access the same database.

  It will ASSERT() if that operation fails and it will always return EFI_SUCCESS.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   This function completes  successfully.
  @return Others        Any error occurred while saving or getting check point address from
                        volatile variable.
**/
EFI_STATUS
EFIAPI
BdsCpLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS     Status;
  EFI_HANDLE     Handle;

  if (IsInSmm ()) {
    return EFI_SUCCESS;
  }

  Status = gBS->LocateProtocol (
                  &mInternalBdsCpListHeadGuid,
                  NULL,
                  (VOID **) &mCpList
                  );
  if (EFI_ERROR (Status)) {
    mCpList = AllocatePool (sizeof (LIST_ENTRY));
    ASSERT (mCpList != NULL);
    if (mCpList == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    mCpList->ForwardLink = mCpList;
    mCpList->BackLink    = mCpList;
    Handle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &mInternalBdsCpListHeadGuid,
                    EFI_NATIVE_INTERFACE,
                    (VOID *) mCpList
                    );
  }
  return Status;
}
