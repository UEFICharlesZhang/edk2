/** @file
  BDS Check point Library Defintion.
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

#ifndef _BDS_CHECK_POINT_LIB_H_
#define _BDS_CHECK_POINT_LIB_H_

#include <Uefi.h>

typedef VOID *H2O_BDS_CP_HANDLE;

typedef enum {
  H2O_BDS_CP_LOW    = 6,
  H2O_BDS_CP_MEDIUM = TPL_CALLBACK,
  H2O_BDS_CP_HIGH   = TPL_NOTIFY
} H2O_BDS_CP_PRIORITY;


typedef
VOID
(EFIAPI *H2O_BDS_CP_HANDLER) (
  IN EFI_EVENT         Event,
  IN H2O_BDS_CP_HANDLE Handle
  );

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
  );

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
BdsCpLookup(
  IN  H2O_BDS_CP_HANDLE Handle,
  OUT VOID              **BdsCheckpointData,
  OUT EFI_GUID          *BdsCheckpoint       OPTIONAL
  );

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
  );

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
  );

#endif

