/** @file
  Update link menu for one of parent formset.

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

#include <Library/DxeOemSvcKernelLib.h>

/**
  Update link menu for one of parent formset.

  @param[in] FormsetGuid                 Pointer to parent formset guid
  @param[in, out] FormsetCount         Pointer to number of child formsets
  @param[in, out] HiiHandleList          Pointer to child formsets' handle list
  @param[in, out] FormsetGuidList      Pointer to child formsets' formset guid list
  @param[in, out] FormIdList              Pointer to child formsets' entry form id list linking by parent formset

  @retval EFI_UNSUPPORTED              Returns unsupported by default.
  @retval EFI_MEDIA_CHANGED          The value of IN OUT parameter is changed.
**/
EFI_STATUS
OemSvcUpdateLinkFormSetIds (
  IN CONST EFI_GUID                   *FormsetGuid,
  IN OUT UINT32                       *FormsetCount,
  IN OUT EFI_HII_HANDLE               **HiiHandleList,
  IN OUT EFI_GUID                     **FormsetGuidList,
  IN OUT UINT32                       **EntryFormIdList
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  return EFI_UNSUPPORTED;
}

