/** @file
Function definition for formbrowser
;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "InternalH2OFormBrowser.h"
#include <Protocol/FormBrowserEx2.h>
#include <Library/DxeOemSvcKernelLib.h>

extern EFI_GUID                     gEfiCallerIdGuid;
H2O_FORM_BROWSER_CONSOLE_DEV        **mConsoleDevList;
UINT32                              mConsoleDevListCount;

#pragma pack(push, 1)
typedef struct {
  UINT8  Id[4];
  UINT8  HIIVersion;
  UINT32 EZH2OVersion;
  UINT32 StringPackageoffset;
  UINT32 Reserved1;
  UINT32 StringPackageTotalSizeOffset;
  UINT32 Reserved2;
} EFI_OBS_HEAD;
#pragma pack(pop)

#ifndef __GNUC__
EFI_OBS_HEAD     OBS_HEADER =
                   { {'$', 'O', 'B', 'S'},
                     0x21,
                     0x02,
                     (UINT32)(UINTN) H2OFormBrowserDxeStrings,
                     0,
                     (UINT32)(UINTN) &H2OFormBrowserDxeStringsTotalSize,
                     0
                   };
#endif

typedef struct {
  CHAR16              *PcdStr;
  EFI_STRING_ID       DescriptionStr;
  H2O_EVT_TYPE        H2OEvtType;
  CHAR16              *Image;
  HOT_KEY_ACTION      HotkeyAction;
  UINT8               GroupId;        ///< For hotkey group
} HOT_KEY_PRIVATE_DATA;

HOT_KEY_PRIVATE_DATA mHotKeyListDefault[] = {
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeyShowHelpMsg)  , STRING_TOKEN (SCU_HOTKEY_BAR_HELP_DSC)   , H2O_DISPLAY_ENGINE_EVT_TYPE_SHOW_HELP,
                        L"@HotkeyShowHelp",       HotkeyShowHelpMsg,    HOT_KEY_INFO_GROUP_ID_NONE},
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeyDiscardExit)  , STRING_TOKEN (SCU_HOTKEY_BAR_ESC_DSC)    , H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_EXIT,
                        L"@HotKeyDiscardExit",    HotKeyDiscardExit,    HOT_KEY_INFO_GROUP_ID_NONE},
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeySelectItemUp) , STRING_TOKEN (SCU_HOTKEY_BAR_SELECT_DSC) , H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_Q,
                        L"@HotkeySelectItemUp",   HotKeySelectItemUp,   0x01},
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeySelectItemDn) , STRING_TOKEN (SCU_HOTKEY_BAR_SELECT_DSC) , H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_Q,
                        L"@HotkeySelectItemDown", HotKeySelectItemDown, 0x01} ,
                       };
HOT_KEY_PRIVATE_DATA mHotKeyListSelectPage[] = {
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeySelectPageUp) , STRING_TOKEN (SCU_HOTKEY_BAR_MENU_DSC)   , H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_P,
                        L"@HotkeySelectMenuUp",   HotkeySelectMenuUp,   0x02},
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeySelectPageDn) , STRING_TOKEN (SCU_HOTKEY_BAR_MENU_DSC)   , H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_P,
                        L"@HotkeySelectMenuDown", HotkeySelectMenuDown, 0x02},
                       };
HOT_KEY_PRIVATE_DATA mHotKeyListModifyValue[] = {
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeyModifyValueUp), STRING_TOKEN (SCU_HOTKEY_BAR_CHANGE_DSC) , H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGE_Q,
                        L"@HotKeyModifyValueUp",   HotkeyModifyValueUp,   0x03},
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeyModifyValueDn), STRING_TOKEN (SCU_HOTKEY_BAR_CHANGE_DSC) , H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGE_Q,
                        L"@HotKeyModifyValueDown", HotkeyModifyValueDown, 0x03},
                       };
HOT_KEY_PRIVATE_DATA mHotKeyListEnter[] = {
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeyEnter)        , STRING_TOKEN (SCU_HOTKEY_BAR_ENTER_DSC)  , H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_Q,
                        L"@HotkeyEnter", HotKeyEnter, HOT_KEY_INFO_GROUP_ID_NONE},
                       };
HOT_KEY_PRIVATE_DATA mHotKeyListVarstore[] = {
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeyLoadDefault)  , STRING_TOKEN (SCU_HOTKEY_BAR_DEFAULT_DSC), H2O_DISPLAY_ENGINE_EVT_TYPE_DEFAULT,
                        L"@HotKeyLoadDefault", HotKeyLoadDefault, HOT_KEY_INFO_GROUP_ID_NONE},
                       {(CHAR16*)(UINTN)PcdGetPtr (PcdHotKeySaveAndExit)  , STRING_TOKEN (SCU_HOTKEY_BAR_SAVE_DSC)   , H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_EXIT,
                        L"@HotKeySaveAndExit", HotKeySaveAndExit, HOT_KEY_INFO_GROUP_ID_NONE},
                       };

STATIC
BOOLEAN
NeedModifyValueHotKey (
  IN H2O_FORM_BROWSER_PRIVATE_DATA            *Private
  )
{
  FORM_BROWSER_FORMSET                        *FormSet;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_STATEMENT                      *Statement;
  LIST_ENTRY                                  *LinkFormSet;
  LIST_ENTRY                                  *LinkForm;
  LIST_ENTRY                                  *LinkStatement;

  if (Private->HiiHandleCount > 1) {
    return TRUE;
  }

  LinkFormSet = GetFirstNode (&Private->FormSetList);
  while (!IsNull (&Private->FormSetList, LinkFormSet)) {
    FormSet     = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (LinkFormSet);
    LinkFormSet = GetNextNode (&Private->FormSetList, LinkFormSet);

    LinkForm = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, LinkForm)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
      LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);

      LinkStatement = GetFirstNode (&Form->StatementListHead);
      while (!IsNull (&Form->StatementListHead, LinkStatement)) {
        Statement = FORM_BROWSER_STATEMENT_FROM_LINK (LinkStatement);
        LinkStatement = GetNextNode (&Form->StatementListHead, LinkStatement);
        if (Statement->Operand == EFI_IFR_ORDERED_LIST_OP ||
            Statement->Operand == EFI_IFR_ONE_OF_OP       ||
            Statement->Operand == EFI_IFR_NUMERIC_OP      ||
            Statement->Operand == EFI_IFR_TIME_OP         ||
            Statement->Operand == EFI_IFR_DATE_OP         ||
            Statement->Operand == EFI_IFR_CHECKBOX_OP) {
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

STATIC
BOOLEAN
NeedEnterHotKey (
  IN H2O_FORM_BROWSER_PRIVATE_DATA            *Private
  )
{
  FORM_BROWSER_FORMSET                        *FormSet;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_STATEMENT                      *Statement;
  LIST_ENTRY                                  *LinkFormSet;
  LIST_ENTRY                                  *LinkForm;
  LIST_ENTRY                                  *LinkStatement;

  LinkFormSet = GetFirstNode (&Private->FormSetList);
  while (!IsNull (&Private->FormSetList, LinkFormSet)) {
    FormSet     = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (LinkFormSet);
    LinkFormSet = GetNextNode (&Private->FormSetList, LinkFormSet);

    LinkForm = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, LinkForm)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
      LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);

      LinkStatement = GetFirstNode (&Form->StatementListHead);
      while (!IsNull (&Form->StatementListHead, LinkStatement)) {
        Statement = FORM_BROWSER_STATEMENT_FROM_LINK (LinkStatement);
        LinkStatement = GetNextNode (&Form->StatementListHead, LinkStatement);

        if (Statement->Statement.Selectable) {
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

STATIC
BOOLEAN
NeedVarStoreHotKey (
  IN H2O_FORM_BROWSER_PRIVATE_DATA            *Private
  )
{
  FORM_BROWSER_FORMSET                        *FormSet;
  LIST_ENTRY                                  *LinkFormSet;

  if (Private->HiiHandleCount > 1) {
    return TRUE;
  }

  LinkFormSet = GetFirstNode (&Private->FormSetList);
  while (!IsNull (&Private->FormSetList, LinkFormSet)) {
    FormSet     = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (LinkFormSet);
    LinkFormSet = GetNextNode (&Private->FormSetList, LinkFormSet);

    if (!IsListEmpty (&FormSet->StorageListHead)) {
      return TRUE;
    }
  }

  return FALSE;
}

STATIC
EFI_STATUS
SetHotKeyInfo (
  IN     HOT_KEY_PRIVATE_DATA      *HotKeyPrivateData,
  IN     UINT32                    HotKeyPrivateDataCount,
  IN     UINT32                    HotKeyInfoCount,
  IN OUT UINT32                    *HotKeyInfoIndex,
  OUT    HOT_KEY_INFO              *HotKeyInfo
  )
{
  H2O_KEY_DESC_PROTOCOL            *KeyDesc;
  EFI_STATUS                       Status;
  UINT32                           Index;

  if (HotKeyPrivateData == NULL || HotKeyInfoIndex == NULL || HotKeyInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gH2OKeyDescProtocolGuid, NULL, (VOID **) &KeyDesc);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HotKeyPrivateDataCount; Index++) {
    if (*HotKeyInfoIndex >= HotKeyInfoCount) {
      return EFI_BUFFER_TOO_SMALL;
    }

    Status = KeyDesc->KeyFromString (
                        KeyDesc,
                        HotKeyPrivateData[Index].PcdStr,
                        &HotKeyInfo[*HotKeyInfoIndex].KeyData
                        );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    HotKeyInfo[*HotKeyInfoIndex].Mark         = AllocateCopyPool (StrSize (HotKeyPrivateData[Index].PcdStr), HotKeyPrivateData[Index].PcdStr);
    HotKeyInfo[*HotKeyInfoIndex].String       = GetString (HotKeyPrivateData[Index].DescriptionStr, mHiiHandle);
    HotKeyInfo[*HotKeyInfoIndex].SendEvtType  = HotKeyPrivateData[Index].H2OEvtType;
    HotKeyInfo[*HotKeyInfoIndex].Image        = HotKeyPrivateData[Index].Image;
    HotKeyInfo[*HotKeyInfoIndex].HotkeyAction = HotKeyPrivateData[Index].HotkeyAction;
    HotKeyInfo[*HotKeyInfoIndex].GroupId      = HotKeyPrivateData[Index].GroupId;
    (*HotKeyInfoIndex)++;
  }

  return EFI_SUCCESS;
}

/**
 Formbrowser initialize hotkey information

 @param [in] Private         Formbrowser private data

 @retval EFI_SUCCESS         Formbrowser initialize hotkey success

**/
STATIC
EFI_STATUS
FBInitHotKeyInfo (
  IN H2O_FORM_BROWSER_PRIVATE_DATA               *Private
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      HotKeyInfoIndex;
  UINT32                                      HotKeyInfoCount;
  HOT_KEY_INFO                                *HotKeyInfo;

  HotKeyInfoCount = sizeof (mHotKeyListDefault)     / sizeof (HOT_KEY_PRIVATE_DATA) +
                    sizeof (mHotKeyListSelectPage)  / sizeof (HOT_KEY_PRIVATE_DATA) +
                    sizeof (mHotKeyListModifyValue) / sizeof (HOT_KEY_PRIVATE_DATA) +
                    sizeof (mHotKeyListEnter)       / sizeof (HOT_KEY_PRIVATE_DATA) +
                    sizeof (mHotKeyListVarstore)    / sizeof (HOT_KEY_PRIVATE_DATA);
  HotKeyInfo = AllocateZeroPool (sizeof (HOT_KEY_INFO) * HotKeyInfoCount);
  if (HotKeyInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  HotKeyInfoIndex = 0;

  Status = SetHotKeyInfo (mHotKeyListDefault, sizeof (mHotKeyListDefault) / sizeof (HOT_KEY_PRIVATE_DATA), HotKeyInfoCount, &HotKeyInfoIndex, HotKeyInfo);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Private->HiiHandleCount > 1) {
    Status = SetHotKeyInfo (mHotKeyListSelectPage, sizeof (mHotKeyListSelectPage) / sizeof (HOT_KEY_PRIVATE_DATA), HotKeyInfoCount, &HotKeyInfoIndex, HotKeyInfo);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (NeedModifyValueHotKey (Private)) {
    Status = SetHotKeyInfo (mHotKeyListModifyValue, sizeof (mHotKeyListModifyValue) / sizeof (HOT_KEY_PRIVATE_DATA), HotKeyInfoCount, &HotKeyInfoIndex, HotKeyInfo);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (NeedEnterHotKey (Private)) {
    Status = SetHotKeyInfo (mHotKeyListEnter, sizeof (mHotKeyListEnter) / sizeof (HOT_KEY_PRIVATE_DATA), HotKeyInfoCount, &HotKeyInfoIndex, HotKeyInfo);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (NeedVarStoreHotKey (Private)) {
    Status = SetHotKeyInfo (mHotKeyListVarstore, sizeof (mHotKeyListVarstore) / sizeof (HOT_KEY_PRIVATE_DATA), HotKeyInfoCount, &HotKeyInfoIndex, HotKeyInfo);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  DestroyHotKeyInfo (Private);

  if (HotKeyInfoIndex != 0) {
    Private->NumberOfHotKeys = HotKeyInfoIndex;
    Private->HotKeyInfo      = AllocateZeroPool (sizeof (HOT_KEY_INFO) * (HotKeyInfoIndex + 1));
    if (Private->HotKeyInfo == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    CopyMem (Private->HotKeyInfo, HotKeyInfo, sizeof (HOT_KEY_INFO) * HotKeyInfoIndex);
  }

  FreePool (HotKeyInfo);

  return EFI_SUCCESS;
}

/**
 Formbrowser initialize page information

 @param [in] Private         Formbrowser private data

 @retval EFI_SUCCESS         Formbrowser initialize hotkey success

**/
EFI_STATUS
FBInitPageInfo (
  IN H2O_FORM_BROWSER_PRIVATE_DATA               *Private
  )
{
  FORM_BROWSER_FORMSET                        *FormSet;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_STATEMENT                      *Statement;
  LIST_ENTRY                                  *LinkFormSet;
  LIST_ENTRY                                  *LinkForm;
  LIST_ENTRY                                  *LinkStatement;
  H2O_FORM_BROWSER_P                          *Page;
  EXPRESS_RESULT                                  ExpressResult;
  UI_MENU_SELECTION                           *Selection;
  UINTN                                       Index;

  Selection = gCurrentSelection;
  DestroyPageInfo (Private);

  //
  // Find all Form in Form Set
  //
  Private->PageCount = 0;
  LinkFormSet = GetFirstNode (&Private->FormSetList);
  while (!IsNull (&Private->FormSetList, LinkFormSet)) {
    FormSet     = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (LinkFormSet);
    LinkFormSet = GetNextNode (&Private->FormSetList, LinkFormSet);

    LinkForm = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, LinkForm)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
      LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);

      Private->PageCount++;
      Page = &Form->PageInfo;

      if (Form->FormId != Selection->FormId) {
        Page->Signature = 0;
        continue;
      }

      InitPage (Private, FormSet, Form, Page);

      LinkStatement = GetFirstNode (&Form->StatementListHead);
      while (!IsNull (&Form->StatementListHead, LinkStatement)) {
        Statement = FORM_BROWSER_STATEMENT_FROM_LINK (LinkStatement);
        LinkStatement = GetNextNode (&Form->StatementListHead, LinkStatement);

        ExpressResult = EvaluateExpressionList(Statement->Expression, FALSE, NULL, NULL);
        if (ExpressResult <= ExpressGrayOut) {
          Page->StatementIds[Page->NumberOfStatementIds++] = Statement->StatementId;
        }

        InitH2OStatement (
          FormSet,
          Form,
          ExpressResult,
          Page,
          Statement
          );

        CopyOption (Form, Statement);
      }

      FBInitHotKeyInfo (Private);
      Page->HotKeyInfo = Private->HotKeyInfo;
    }
  }

  Private->PageIdList = AllocateZeroPool (Private->PageCount * sizeof (H2O_PAGE_ID));
  if (Private->PageIdList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Index = 0;
  LinkFormSet = GetFirstNode (&Private->FormSetList);
  while (!IsNull (&Private->FormSetList, LinkFormSet)) {
    FormSet     = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (LinkFormSet);
    LinkFormSet = GetNextNode (&Private->FormSetList, LinkFormSet);

    LinkForm = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, LinkForm)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
      LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);
      Page = &Form->PageInfo;
      Private->PageIdList[Index++] = Page->PageId;
    }
  }

  return EFI_SUCCESS;
}

/**
 Formbrowser repaint scene.

 @param [in] Private         Formbrowser private data

 @retval EFI_SUCCESS         Formbrowser repaint scene success

**/
EFI_STATUS
FBRepaint (
  IN H2O_FORM_BROWSER_PRIVATE_DATA               *Private
  )
{
  H2O_DISPLAY_ENGINE_EVT_OPEN_L          OpenL;
  H2O_DISPLAY_ENGINE_EVT_OPEN_P          OpenP;

  //
  // If selection not ready, do nothing (maybe only pop dialog)
  //
  if (gCurrentSelection == NULL) {
    return EFI_SUCCESS;
  }

  //
  // send OPEN_L, OPEN_P
  //
  ZeroMem (&OpenL, sizeof (H2O_DISPLAY_ENGINE_EVT_OPEN_L));
  OpenL.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_OPEN_L);
  OpenL.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
  OpenL.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_L;
  FBBroadcastEvent ((H2O_DISPLAY_ENGINE_EVT*)&OpenL);

  ZeroMem (&OpenP, sizeof (H2O_DISPLAY_ENGINE_EVT_OPEN_P));
  OpenP.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_OPEN_P);
  OpenP.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
  OpenP.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_P;
  FBBroadcastEvent ((H2O_DISPLAY_ENGINE_EVT*)&OpenP);

  return EFI_SUCCESS;
}

/**
  Get the form set image  based on the input IFR and HII Package List Handle.

  @param  [in] Formset     Pointer to EFI_IFR_FORM_SET instance.
  @param  [in] HiiHandle   The package list in the HII database to search for
                           the specified string.
  @return Other            The output EFI_IMAGE_INPUT.
  @retval NULL             Cannot find formset image.

**/
STATIC
EFI_IMAGE_INPUT *
GetFormSetImage (
  IN  EFI_IFR_FORM_SET     *Formset,
  IN  EFI_HII_HANDLE       HiiHandle
  )
{
  EFI_IFR_OP_HEADER       *OpHeader;
  EFI_STATUS              Status;
  EFI_IMAGE_INPUT         *ImageIn;

  ASSERT (Formset != NULL && Formset->Header.OpCode == EFI_IFR_FORM_SET_OP );

  OpHeader = (EFI_IFR_OP_HEADER *) (((UINT8 *) Formset) + Formset->Header.Length);
  //
  // Skip all of default store OP code
  //
  while (OpHeader->OpCode == EFI_IFR_DEFAULTSTORE_OP || OpHeader->OpCode == EFI_IFR_GUID_OP) {
    OpHeader = (EFI_IFR_OP_HEADER *) (((UINT8 *) OpHeader) + OpHeader->Length);
  }
  if (OpHeader->OpCode != EFI_IFR_IMAGE_OP) {
    return NULL;
  }

  //
  // Try to get image by image ID from HII handle.
  //
  ImageIn = AllocateZeroPool (sizeof (EFI_IMAGE_INPUT));
  ASSERT (ImageIn != NULL);
  if (ImageIn == NULL) {
    return NULL;
  }
  Status = gHiiImage->GetImage (
                        gHiiImage,
                        HiiHandle,
                        ((EFI_IFR_IMAGE *) OpHeader)->Id,
                        ImageIn
                        );
   ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    FreePool (ImageIn);
    return NULL;
  }
  return ImageIn;
}


/**
  Extract the displayed formset for given HII handle and class guid.

  @param Handle          The HII handle.
  @param SetupClassGuid  The class guid specifies which form set will be displayed.
  @param SkipCount       Skip some formsets which has processed before.
  @param FormSetTitle    Formset title string.
  @param FormSetHelp     Formset help string.
  @param FormSetGuid     Return the formset guid for this formset.

  @retval  TRUE          The formset for given HII handle will be displayed.
  @return  FALSE         The formset for given HII handle will not be displayed.

**/
BOOLEAN
LocateFormSetInfo (
  IN      EFI_HII_HANDLE        Handle,
  IN OUT  CHAR16                **FormSetTitle,
  IN OUT  EFI_IMAGE_INPUT       **FormSetImage
  )
{
  EFI_STATUS                   Status;
  UINTN                        BufferSize;
  EFI_HII_PACKAGE_LIST_HEADER  *HiiPackageList;
  UINT8                        *Package;
  UINT8                        *OpCodeData;
  UINT32                       Offset;
  UINT32                       Offset2;
  UINT32                       PackageListLength;
  EFI_HII_PACKAGE_HEADER       PackageHeader;


  ASSERT (Handle != NULL);
  ASSERT (FormSetTitle != NULL);
  ASSERT (FormSetImage != NULL);

  *FormSetTitle = NULL;
  *FormSetImage = NULL;

  //
  // Get HII PackageList
  //
  BufferSize = 0;
  HiiPackageList = NULL;
  Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, Handle, &BufferSize, HiiPackageList);
  //
  // Handle is a invalid handle. Check if Handle is corrupted.
  //
  ASSERT (Status != EFI_NOT_FOUND);
  //
  // The return status should always be EFI_BUFFER_TOO_SMALL as input buffer's size is 0.
  //
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

  HiiPackageList = AllocatePool (BufferSize);
  ASSERT (HiiPackageList != NULL);
  if (HiiPackageList == NULL) {
    return FALSE;
  }

  Status = gHiiDatabase->ExportPackageLists (gHiiDatabase, Handle, &BufferSize, HiiPackageList);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // Get Form package from this HII package List
  //
  Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  Offset2 = 0;
  PackageListLength = ReadUnaligned32 (&HiiPackageList->PackageLength);

  while (Offset < PackageListLength) {
    Package = ((UINT8 *) HiiPackageList) + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
      //
      // Search FormSet Opcode in this Form Package
      //
      Offset2 = sizeof (EFI_HII_PACKAGE_HEADER);
      while (Offset2 < PackageHeader.Length) {
        OpCodeData = Package + Offset2;
        Offset2 += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
          *FormSetTitle = GetString (((EFI_IFR_FORM_SET *) OpCodeData)->FormSetTitle, Handle);
          *FormSetImage = GetFormSetImage ((EFI_IFR_FORM_SET *) OpCodeData, Handle);
          FreePool (HiiPackageList);
          return TRUE;
        }
      }
    }

    //
    // Go to next package
    //
    Offset += PackageHeader.Length;
  }

  FreePool (HiiPackageList);

  return FALSE;
}

/**
 Formbrowser initialize setup menu information

 @param [in] Private         Formbrowser private data

 @retval EFI_SUCCESS         Formbrowser initialize setup menu success

**/
EFI_STATUS
FBInitSMInfo (
  IN H2O_FORM_BROWSER_PRIVATE_DATA          *Private
  )
{

  UINT32                                    Index;
  EFI_HII_HANDLE                            *SetupMenuHiiHandle;
  UINTN                                     SetupMenuCount;
  UINT32                                    SetupMenuIndex;
  EFI_GUID                                  ZeroGuid;

  ZeroMem (&ZeroGuid, sizeof (EFI_GUID));
  SetupMenuCount     = Private->HiiHandleCount;
  SetupMenuHiiHandle = AllocateCopyPool (sizeof(EFI_HII_HANDLE) * SetupMenuCount, Private->HiiHandleList);
  if (SetupMenuHiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (GetScuFormset () != NULL) {
    //
    // Call OEM service function to determine setup menu when SCU
    //
    OemSvcAdjustNavigationMenu (SetupMenuHiiHandle, &SetupMenuCount, &ZeroGuid);
  }

  //
  // Initialize page ID for each setup menu info
  //
  Private->SetupMenuInfo = AllocateZeroPool (sizeof (SETUP_MENU_INFO) * SetupMenuCount);
  if (Private->SetupMenuInfo == NULL) {
    FreePool (SetupMenuHiiHandle);
    return EFI_OUT_OF_RESOURCES;
  }

  for (SetupMenuIndex = 0; SetupMenuIndex < SetupMenuCount; SetupMenuIndex++) {
    for (Index = 0; Index < Private->HiiHandleCount; Index++) {
      if (Private->HiiHandleList[Index] != SetupMenuHiiHandle[SetupMenuIndex]) {
        continue;
      }

      Private->SetupMenuInfo[Private->NumberOfSetupMenus].PageId = Index << 16;
      Private->NumberOfSetupMenus++;
      break;
    }
  }

  FreePool (SetupMenuHiiHandle);
  return EFI_SUCCESS;
}

/**
 Update setup menu information (page ID, title string and image)

 @param [in] Private         Formbrowser private data

 @retval EFI_SUCCESS         Successfully update setup menu information
**/
EFI_STATUS
FBUpdateSMInfo (
  IN H2O_FORM_BROWSER_PRIVATE_DATA          *Private
  )
{
  UINT32                                    Index;
  UINT32                                    HiiHandleIndex;
  LIST_ENTRY                                *Link;
  FORM_BROWSER_FORM                         *FirstForm;
  SETUP_MENU_INFO                           *SetupMenuInfo;

  for (Index = 0; Index < Private->NumberOfSetupMenus; Index++) {
    SetupMenuInfo  = &Private->SetupMenuInfo[Index];
    HiiHandleIndex = SetupMenuInfo->PageId >> 16;

    //
    // Update current FormSet Root page id
    //
    if (HiiHandleIndex == Private->HiiHandleIndex) {
      Link      = GetFirstNode (&gCurrentSelection->FormSet->FormListHead);
      FirstForm = FORM_BROWSER_FORM_FROM_LINK (Link);
      SetupMenuInfo->PageId = (H2O_PAGE_ID) ((HiiHandleIndex << 16) + FirstForm->FormId);
    }

    //
    // Update tilte string and image for each setup menu
    //
    if (HiiHandleIndex < Private->HiiHandleCount) {
      FBFreePool ((VOID **) &SetupMenuInfo->PageTitle);
      if (SetupMenuInfo->PageImage != NULL) {
        FreePool (SetupMenuInfo->PageImage->Bitmap);
        FreePool (SetupMenuInfo->PageImage);
      }

      LocateFormSetInfo (
        Private->HiiHandleList[HiiHandleIndex],
        &SetupMenuInfo->PageTitle,
        &SetupMenuInfo->PageImage
        );
    }
  }

  return EFI_SUCCESS;
}

/**
  Load specific Hii formsets in the global maintain list of form browser.

  @param[in] Handles              A pointer to an array of Handles.
  @param[in] HandleCount          The number of Handles specified in Handle.

  @retval EFI_SUCCESS             Successfully load Hii formsets
  @retval EFI_INVALID_PARAMETER   Input pointer is NULL
  @retval EFI_OUT_OF_RESOURCES    Allocate pool fail
**/
EFI_STATUS
LoadHiiFormset (
  IN EFI_HII_HANDLE                   *Handles,
  IN UINTN                            HandleCount
  )
{
  FORM_BROWSER_FORMSET                *LocalFormSet;
  UINTN                               Index;
  EFI_GUID                            ZeroGuid;
  EFI_STATUS                          Status;
  FORM_BROWSER_FORMSET                *OldFormset;
  FORM_BROWSER_FORM                   *Form;
  LIST_ENTRY                          *Link;

  if (Handles == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  OldFormset = mSystemLevelFormSet;

  for (Index = 0; Index < HandleCount; Index++) {
    //
    // Check Hii handle does exist in global maintain list.
    //
    if (GetFormSetFromHiiHandle (Handles[Index]) != NULL) {
      continue;
    }

    //
    // Initilize FormSet Setting
    //
    LocalFormSet = AllocateZeroPool (sizeof (FORM_BROWSER_FORMSET));
    if (LocalFormSet == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    mSystemLevelFormSet = LocalFormSet;

    ZeroMem (&ZeroGuid, sizeof (ZeroGuid));
    Status = InitializeFormSet (Handles[Index], &ZeroGuid, LocalFormSet);
    if (IsListEmpty (&LocalFormSet->FormListHead)) {
      DestroyFormSet (LocalFormSet);
      continue;
    } else if (EFI_ERROR (Status)) {
      FreePool (LocalFormSet);
      continue;
    }
    InitCurrentSetting (LocalFormSet);

    //
    // Initilize Questions' Value
    //
    Status = LoadFormSetConfig (NULL, LocalFormSet);
    if (EFI_ERROR (Status)) {
      DestroyFormSet (LocalFormSet);
      continue;
    }

    //
    // Evaluate form expressions for non-SCU formset, because user may load default without going to these formsets.
    //
    if (CompareGuid (LocalFormSet->ClassGuid, &mScuFormSetGuid)) {
      continue;
    }

    Link = GetFirstNode (&LocalFormSet->FormListHead);
    while (!IsNull (&LocalFormSet->FormListHead, Link)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (Link);
      EvaluateFormExpressions (LocalFormSet, Form);
      Link = GetNextNode (&LocalFormSet->FormListHead, Link);
    }
  }

  mSystemLevelFormSet = OldFormset;

  return EFI_SUCCESS;
}

/**
  Destroy specific Hii formsets from the global maintain list of form browser.

  @param[in] Handles              A pointer to an array of Handles.
  @param[in] HandleCount          The number of Handles specified in Handle.

  @retval EFI_SUCCESS             Successfully load Hii formsets
  @retval EFI_INVALID_PARAMETER   Input pointer is NULL
**/
EFI_STATUS
DestroyLoadHiiFormset (
  IN EFI_HII_HANDLE                   *Handles,
  IN UINTN                            HandleCount
  )
{
  FORM_BROWSER_FORMSET                *FormSet;
  FORM_BROWSER_FORMSET                *OldFormset;
  UINTN                               Index;

  if (Handles == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  OldFormset = mSystemLevelFormSet;

  for (Index = 0; Index < HandleCount; Index++) {
    FormSet = GetFormSetFromHiiHandle (Handles[Index]);
    if (FormSet == NULL) {
      continue;
    }

    mSystemLevelFormSet = FormSet;

    CleanBrowserStorage (FormSet);
    RemoveEntryList (&FormSet->Link);
    DestroyFormSet (FormSet);
  }

  mSystemLevelFormSet = OldFormset;

  return EFI_SUCCESS;
}

/**
  Check if two browser storages are idential or not

  @param[in] BrowserStorage1      A pointer to browser storage
  @param[in] BrowserStorage2      A pointer to browser storage

  @retval TRUE                    Two browser storages are idential
  @retval FALSE                   Two browser storages are different or input pointer is NULL
**/
BOOLEAN
IsStorageIdentical (
  IN BROWSER_STORAGE                  *BrowserStorage1,
  IN BROWSER_STORAGE                  *BrowserStorage2
  )
{
  if (BrowserStorage1 == NULL || BrowserStorage2 == NULL) {
    return FALSE;
  }

  if ((BrowserStorage1->Type != BrowserStorage2->Type) ||
      !CompareGuid (&BrowserStorage1->Guid, &BrowserStorage2->Guid)) {
    return FALSE;
  }

  switch (BrowserStorage1->Type) {

  case EFI_HII_VARSTORE_NAME_VALUE:
    if (BrowserStorage1->HiiHandle == BrowserStorage2->HiiHandle) {
      return TRUE;
    }
    break;

  case EFI_HII_VARSTORE_EFI_VARIABLE:
  case EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER:
    if (StrCmp (BrowserStorage1->Name, BrowserStorage2->Name) == 0) {
      return TRUE;
    }

  case EFI_HII_VARSTORE_BUFFER:
    if (BrowserStorage1->HiiHandle == BrowserStorage2->HiiHandle &&
        StrCmp (BrowserStorage1->Name, BrowserStorage2->Name) == 0) {
      return TRUE;
    }
    break;
  }

  return FALSE;
}

/**
  Sync formset storage

  @param[out] DstFormSetStorage    A pointer to destination browser storage
  @param[in]  SrcFormSetStorage    A pointer to source browser storage

  @retval EFI_SUCCESS              Successfully sync formset storage
  @retval EFI_INVALID_PARAMETER    Input pointer is NULL
  @retval EFI_ABORTED              Two storage are not idential
  @retval EFI_UNSUPPORTED          Function does not support for storage type
**/
EFI_STATUS
SyncStorage (
  OUT FORMSET_STORAGE                 *DstFormSetStorage,
  IN  FORMSET_STORAGE                 *SrcFormSetStorage
  )
{
  LIST_ENTRY                          *SrcLink;
  NAME_VALUE_NODE                     *SrcNode;
  LIST_ENTRY                          *DstLink;
  NAME_VALUE_NODE                     *DstNode;

  if (DstFormSetStorage == NULL || SrcFormSetStorage == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsStorageIdentical (DstFormSetStorage->BrowserStorage, SrcFormSetStorage->BrowserStorage)) {
    return EFI_ABORTED;
  }

  switch (DstFormSetStorage->BrowserStorage->Type) {

  case EFI_HII_VARSTORE_BUFFER:
  case EFI_HII_VARSTORE_EFI_VARIABLE_BUFFER:
    CopyMem (
      DstFormSetStorage->BrowserStorage->EditBuffer,
      SrcFormSetStorage->BrowserStorage->EditBuffer,
      MIN(DstFormSetStorage->BrowserStorage->Size, SrcFormSetStorage->BrowserStorage->Size)
      );
    break;

  case EFI_HII_VARSTORE_NAME_VALUE:
    //
    // Update each nmae value of source formset storage to destination formset storage.
    //
    SrcLink = GetFirstNode (&SrcFormSetStorage->BrowserStorage->NameValueListHead);
    while (!IsNull (&SrcFormSetStorage->BrowserStorage->NameValueListHead, SrcLink)) {
      SrcNode = NAME_VALUE_NODE_FROM_LINK (SrcLink);
      SrcLink = GetNextNode (&SrcFormSetStorage->BrowserStorage->NameValueListHead, SrcLink);

      DstLink = GetFirstNode (&DstFormSetStorage->BrowserStorage->NameValueListHead);
      while (!IsNull (&DstFormSetStorage->BrowserStorage->NameValueListHead, DstLink)) {
        DstNode = NAME_VALUE_NODE_FROM_LINK (DstLink);
        DstLink = GetNextNode (&DstFormSetStorage->BrowserStorage->NameValueListHead, DstLink);
        if (StrCmp (DstNode->Name, SrcNode->Name) == 0) {
          NewStringCpy (&DstNode->EditValue, SrcNode->EditValue);
          break;
        }
      }
    }
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Sync all storages of two formset

  @param[out] DstFormSet           A pointer to destination formset
  @param[in]  SrcFormSet           A pointer to source formset

  @retval EFI_SUCCESS              Successfully sync formset
  @retval EFI_INVALID_PARAMETER    Input pointer is NULL
**/
EFI_STATUS
SyncStorageByFormset (
  OUT FORM_BROWSER_FORMSET      *DstFormSet,
  IN  FORM_BROWSER_FORMSET      *SrcFormSet
  )
{
  LIST_ENTRY                   *DstLink;
  FORMSET_STORAGE              *DstFormSetStorage;
  LIST_ENTRY                   *SrcLink;
  FORMSET_STORAGE              *SrcFormSetStorage;

  if (DstFormSet == NULL || SrcFormSet == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DstLink = GetFirstNode (&DstFormSet->StorageListHead);
  while (!IsNull (&DstFormSet->StorageListHead, DstLink)) {
    DstFormSetStorage = (FORMSET_STORAGE_FROM_LINK (DstLink));
    DstLink           = GetNextNode (&DstFormSet->StorageListHead, DstLink);

    //
    // Skip if there is no RequestElement
    //
    if (DstFormSetStorage->ElementCount == 0) {
      continue;
    }

    SrcLink = GetFirstNode (&SrcFormSet->StorageListHead);
    while (!IsNull (&SrcFormSet->StorageListHead, SrcLink)) {
      SrcFormSetStorage = (FORMSET_STORAGE_FROM_LINK (SrcLink));
      SrcLink           = GetNextNode (&SrcFormSet->StorageListHead, SrcLink);

      //
      // Skip if there is no RequestElement
      //
      if (SrcFormSetStorage->ElementCount == 0) {
        continue;
      }

      SyncStorage (DstFormSetStorage, SrcFormSetStorage);
    }
  }

  return EFI_SUCCESS;
}

/**
  Sync storages of current formset to all formsets of global maintain list
**/
VOID
SyncCurrentStorageToAllHiiHandles (
  VOID
  )
{
  FORM_BROWSER_FORMSET         *FormSet;
  LIST_ENTRY                   *FormSetLink;

  FormSetLink = GetFirstNode (&gBrowserFormSetList);
  while (!IsNull (&gBrowserFormSetList, FormSetLink)) {
    FormSet     = FORM_BROWSER_FORMSET_FROM_LINK (FormSetLink);
    FormSetLink = GetNextNode (&gBrowserFormSetList, FormSetLink);
    if (!ValidateFormSet (FormSet) || FormSet == mSystemLevelFormSet || CompareGuid (FormSet->ClassGuid, &mScuFormSetGuid)) {
      continue;
    }

    SyncStorageByFormset (FormSet, mSystemLevelFormSet);
    LoadFormSetConfig (gCurrentSelection, FormSet);
    UpdateStatementStatus (FormSet, NULL, FormSetLevel);
  }
}

/**
 Get SCU browser formset data from gBrowserFormSetList

 @return The pointer of SCU browser formset or NULL if there is no SCU browser formset data in gBrowserFormSetList
**/
FORM_BROWSER_FORMSET *
GetScuFormset (
  VOID
  )
{
  FORM_BROWSER_FORMSET    *LocalFormSet;
  LIST_ENTRY              *Link;

  //
  // Check current browser formset first.
  //
  if (mSystemLevelFormSet != NULL && CompareGuid (mSystemLevelFormSet->ClassGuid, &mScuFormSetGuid)) {
    return mSystemLevelFormSet;
  }

  Link = GetFirstNode (&gBrowserFormSetList);
  while (!IsNull (&gBrowserFormSetList, Link)) {
    LocalFormSet = FORM_BROWSER_FORMSET_FROM_LINK (Link);
    if (CompareGuid (LocalFormSet->ClassGuid, &mScuFormSetGuid)) {
      return LocalFormSet;
    }
    Link = GetNextNode (&gBrowserFormSetList, Link);
  }

  return NULL;
}

/**
 Formbrowser tranform page id to Form

 @param [in] Private         Formbrowser private data.
 @param [in] PageId          Formbrowser page identifer.

 @retval EFI_SUCCESS         Formbrowser tranform page id to Form success.
**/
FORM_BROWSER_FORM *
FBPageIdToForm (
  IN H2O_FORM_BROWSER_PRIVATE_DATA               *Private,
  IN H2O_PAGE_ID                                 PageId
  )
{

  UINT32                                      Index;
  FORM_BROWSER_FORMSET                        *FormSet;
  FORM_BROWSER_FORM                           *Form;
  LIST_ENTRY                                  *LinkFormSet;
  LIST_ENTRY                                  *LinkForm;

  Index = 0;
  Form = NULL;
  LinkFormSet = GetFirstNode (&Private->FormSetList);
  while (!IsNull (&Private->FormSetList, LinkFormSet)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (LinkFormSet);
    LinkFormSet = GetNextNode (&Private->FormSetList, LinkFormSet);

    LinkForm = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, LinkForm)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
      LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);

      if (Form->PageInfo.PageId == PageId) {
        return Form;
      }
    }

  }

  return NULL;
}

/**
 Formbrowser set question default value.

 @param [in] Private            Formbrowser private data
 @param [in] QuestionDefault    Question default structure
 @param [in] PageId             Specified page id
 @param [in] QuestionId         Specified question id
 @param [in] HiiValue           Specified default HII value
**/
STATIC
VOID
FBSetQuestionDefault (
  IN      H2O_FORM_BROWSER_PRIVATE_DATA  *Private,
  IN      H2O_FORM_BROWSER_QD            *QuestionDefault,
  IN      H2O_PAGE_ID                    PageId,
  IN      EFI_QUESTION_ID                QuestionId,
  IN      EFI_HII_VALUE                  *HiiValue
  )
{
  QuestionDefault->Signature  = H2O_FORM_BROWSER_QUESTION_DEFAULT_SIGNATURE;
  QuestionDefault->Size       = sizeof (H2O_FORM_BROWSER_QD);
  QuestionDefault->PageId     = PageId;
  QuestionDefault->QuestionId = QuestionId;

  CopyHiiValue (&QuestionDefault->Value, HiiValue);
}


/**
  Get page data according to specific page ID.

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in]  PageId             Specific input page ID.
  @param[out] PageInfo           A pointer to point H2O_FORM_BROWSER_P instance.

  @retval EFI_SUCCESS            Get page data successful.
  @retval EFI_INVALID_PARAMETER  This is NULL, or PageInfo is NULL.
  @retval EFI_NOT_FOUND          Cannot find specific input page ID.
  @retval EFI_OUT_OF_RESOURCES   Could not allocate resources for page.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetPInfo (
  IN     H2O_FORM_BROWSER_PROTOCOL      *This,
  IN     H2O_PAGE_ID                    PageId,
  OUT    H2O_FORM_BROWSER_P             **PageInfo
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);

  if (Private->FB.CurrentP == NULL) {
    return EFI_NOT_FOUND;
  }

  if (PageId != Private->FB.CurrentP->PageId) {
    return EFI_UNSUPPORTED;
  }

  *PageInfo = AllocateCopyPool (sizeof (H2O_FORM_BROWSER_P), Private->FB.CurrentP);
  if (*PageInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}



/**
  Get all of page IDs

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[out] PageCount          A pointer to the number of page ID in PageBuffer.
  @param[out] PageBuffer         A pointer to a buffer which will be allocated by the function which contains all
                                 of H2O_PAGE_ID. The buffer should be freed by the caller.

  @retval EFI_SUCCESS            Get array of page IDs successful.
  @retval EFI_NOT_FOUND          No page IDs match the search.
  @retval EFI_OUT_OF_RESOURCES   There are not enough resources available to allocate PageIdBuffer.
  @retval EFI_INVALID_PARAMETER  This is NULL, PageIdCount is NULL or PageIdBuffer is NULL.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetPAll (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  OUT     UINT32                         *PageIdCount,
  OUT     H2O_PAGE_ID                    **PageIdBuffer
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;
  H2O_PAGE_ID                                 *Buffer;


  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);

  if (Private->PageCount == 0) {
    return EFI_NOT_FOUND;
  }

  Buffer = AllocateCopyPool (sizeof (H2O_PAGE_ID) * Private->PageCount, Private->PageIdList);
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *PageIdCount  = Private->PageCount;
  *PageIdBuffer = Buffer;

  return EFI_SUCCESS;
}

/**
 Formbrowser get statement information.

 @param [in] Private             Formbrowser private data
 @param [in] PageId              Formbrowser page identifer
 @param [in] StatementId         Formbrowser statement identifier
 @param [out] StatementData      Formbrowser statement data

 @retval EFI_SUCCESS             Formbrowser get statement information success
 @retval EFI_INVALID_PARAMETER   Current page is not exist
 @retval EFI_UNSUPPORTED         Page id is not current page id.
 @retval EFI_OUT_OF_RESOURCES    There are not enough resources available to allocate statement data Buffer.

**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetSInfo (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  IN      H2O_PAGE_ID                    PageId,
  IN      H2O_STATEMENT_ID               StatementId,
  OUT     H2O_FORM_BROWSER_S             **StatementData
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_STATEMENT                      *Statement;
  LIST_ENTRY                                  *Link;

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);

    if (Private->FB.CurrentP == NULL) {
      DEBUG ((EFI_D_INFO, "Current page is NULL\n"));
      return EFI_INVALID_PARAMETER;
    }
  ASSERT (Private->FB.CurrentP != NULL);
  if (PageId == Private->FB.CurrentP->PageId) {
    Form = FORM_BROWSER_FORM_FROM_H2O_PAGE (Private->FB.CurrentP);
  } else {
    return EFI_UNSUPPORTED;
  }

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&Form->StatementListHead, Link);

    if (Statement->StatementId == StatementId) {
      *StatementData = AllocateCopyPool (sizeof (H2O_FORM_BROWSER_S), &Statement->Statement);
      if (*StatementData == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}



/**
  Get Question data according to Page identifier and Question identifier.

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in]  PageId             Input Page identifier.
  @param[in]  QuestionId         Input Question ID.
  @param[out] QuestionData       A pointer to H2O_FORM_BROWSER_Q instance.

  @retval EFI_SUCCESS            Get Question data successful.
  @retval EFI_INVALID_PARAMETER  This is NULL, QuestionData is NULL.
  @retval EFI_NOT_FOUND          Cannot find specific Question data.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetQInfo (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  IN      H2O_PAGE_ID                    PageId,
  IN      EFI_QUESTION_ID                QuestionId,
  OUT     H2O_FORM_BROWSER_Q             **QuestionData
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_STATEMENT                      *Statement;
  LIST_ENTRY                                  *Link;

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);

  if (Private->FB.CurrentP == NULL) {
    DEBUG ((EFI_D_INFO, "Current page is NULL\n"));
    return EFI_INVALID_PARAMETER;
  }
  ASSERT (Private->FB.CurrentP != NULL);
  if (PageId == Private->FB.CurrentP->PageId) {
    Form = FORM_BROWSER_FORM_FROM_H2O_PAGE(Private->FB.CurrentP);
  } else {
    return EFI_UNSUPPORTED;
  }

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&Form->StatementListHead, Link);

    if (Statement->QuestionId == QuestionId) {
      *QuestionData = AllocateCopyPool (sizeof (H2O_FORM_BROWSER_Q), &Statement->Statement);
      if (*QuestionData == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Get all of Question identifiers from specific Page.

  @param[in]  This               Get array of Question identifiers successful.
  @param[in]  PageId             Input Page identifier.
  @param[out] QuestionIdCount    A pointer to the number of Question ID in QuestionIdBuffer.
  @param[out] QuestionIdBuffer   A pointer to a buffer which will be allocated by the function which contains all
                                 of Question IDs. The buffer should be freed by the caller.

  @retval EFI_SUCCESS            Get array of Question IDs successful.
  @retval EFI_NOT_FOUND          No Question ID match the search.
  @retval EFI_OUT_OF_RESOURCES   There are not enough resources available to allocate QuestionIdBuffer.
  @retval EFI_INVALID_PARAMETER  This is NULL, QuestionIdCount is NULL or QuestionIdBuffer is NULL.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetQAll (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  IN      H2O_PAGE_ID                    PageId,
  OUT     UINT32                         *QuestionIdCount,
  OUT     EFI_QUESTION_ID                **QuestionIdBuffer
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_STATEMENT                      *Statement;
  LIST_ENTRY                                  *LinkStatement;
  UINT32                                      QuestionCount;
  EFI_QUESTION_ID                             *QuestionIdPtr;

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);
  Form = FBPageIdToForm (Private, PageId);
  if (Form == NULL) {
    return EFI_NOT_FOUND;
  }

  QuestionCount = 0;
  LinkStatement = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, LinkStatement)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (LinkStatement);
    LinkStatement = GetNextNode (&Form->StatementListHead, LinkStatement);
    if (Statement->QuestionId != 0) {
      QuestionCount++;
    }
  }

  QuestionIdPtr = AllocatePool (sizeof (EFI_QUESTION_ID) * QuestionCount);
  ASSERT (QuestionIdPtr != NULL);
  if (QuestionIdPtr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *QuestionIdCount = QuestionCount;
  *QuestionIdBuffer = QuestionIdPtr;

  LinkStatement = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, LinkStatement)) {
    Statement = FORM_BROWSER_STATEMENT_FROM_LINK (LinkStatement);
    LinkStatement = GetNextNode (&Form->StatementListHead, LinkStatement);
    if (Statement->QuestionId != 0) {
      *(QuestionIdPtr++) = Statement->QuestionId;
    }
  }

  return EFI_SUCCESS;
}

/**
  According to Page identifier and Question identifier to get all of types default values in this question.

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in]  PageId             Input Page identifier.
  @param[in]  QuestionId         Input Question ID.
  @param[out] DefaultCount       A pointer to the number of H2O_FORM_BROWSER_QD instance in DefaultBuffer.
  @param[out] DefaultBuffer      A pointer to a buffer which will be allocated by the function which contains all
                                 of H2O_FORM_BROWSER_QD instances. The buffer should be freed by the caller.

  @retval EFI_SUCCESS            Get Question defaults successful.
  @retval EFI_NOT_FOUND          Cannot find default values from specific Question data.
  @retval EFI_OUT_OF_RESOURCES   There are not enough resources available to allocate DefaultBuffer.
  @retval EFI_INVALID_PARAMETER  This is NULL, DefaultCount is NULL or DefaultBuffer is NULL.
**/
EFI_STATUS
H2OFormBrowserGetQDefaults (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  IN      H2O_PAGE_ID                    PageId,
  IN      EFI_QUESTION_ID                QuestionId,
  OUT     UINT32                         *DefaultCount,
  OUT     H2O_FORM_BROWSER_QD            **DefaultBuffer
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_STATEMENT                      *Statement;
  EFI_HII_VALUE                               BackupQuestionValue;
  UINTN                                       Count;
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_QD                         *QuestionDefault;

  ASSERT (DefaultBuffer != NULL);

  if (QuestionId == 0) {
    *DefaultCount = 0;
    *DefaultBuffer = NULL;
    return EFI_SUCCESS;
  }

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);
  Form = FBPageIdToForm (Private, PageId);
  if (Form == NULL) {
    return EFI_NOT_FOUND;
  }

  Statement = IdToQuestion2 (Form, QuestionId);
  if (Statement == NULL) {
    return EFI_NOT_FOUND;
  }


//#define EFI_HII_DEFAULT_CLASS_STANDARD       0x0000
//#define EFI_HII_DEFAULT_CLASS_MANUFACTURING  0x0001
//#define EFI_HII_DEFAULT_CLASS_SAFE           0x0002

  CopyMem (&BackupQuestionValue, &Statement->HiiValue, sizeof (EFI_HII_VALUE));

  Count = 0;
  QuestionDefault = (H2O_FORM_BROWSER_QD*)AllocateZeroPool (sizeof (H2O_FORM_BROWSER_QD) * 3);

  Status = GetQuestionDefault (Form->FormSet, Form, Statement, EFI_HII_DEFAULT_CLASS_STANDARD);
  if (!EFI_ERROR (Status)) {
    FBSetQuestionDefault (Private, QuestionDefault + Count, PageId, QuestionId, &Statement->HiiValue);
    Count++;
  }
  Status = GetQuestionDefault (Form->FormSet, Form, Statement, EFI_HII_DEFAULT_CLASS_MANUFACTURING);
  if (!EFI_ERROR (Status)) {
    FBSetQuestionDefault (Private, QuestionDefault + Count, PageId, QuestionId, &Statement->HiiValue);
    Count++;;
  }
  Status = GetQuestionDefault (Form->FormSet, Form, Statement, EFI_HII_DEFAULT_CLASS_SAFE);
  if (!EFI_ERROR (Status)) {
    FBSetQuestionDefault (Private, QuestionDefault + Count, PageId, QuestionId, &Statement->HiiValue);
    Count++;;
  }
  CopyMem (&Statement->HiiValue, &BackupQuestionValue, sizeof (EFI_HII_VALUE));

  //
  // default id some is count
  //
  if (Count != 0) {
    *DefaultBuffer = QuestionDefault;
    *DefaultCount  = (UINT32) Count;
    return EFI_SUCCESS;
  }


  *DefaultBuffer = NULL;
  *DefaultCount  = 0;
  FreePool (QuestionDefault);

  return EFI_NOT_FOUND;

}

/**
  Get Variable Store data according to specific Variable Store ID.

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in]  PageId             Input Page identifier.
  @param[in]  VarStoreId         Specific input Variable Store ID.
  @param[out] VarbleStoreData    Pointer to H2O_FORM_BROWSER_VS instance.

  @retval EFI_SUCCESS            Get Variable Store successful.
  @retval EFI_INVALID_PARAMETER  This is NULL, or VariableStoreData is NULL.
  @retval EFI_NOT_FOUND          Cannot find specific Variable Store data.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetVSInfo (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  IN      H2O_PAGE_ID                    PageId,
  IN      EFI_VARSTORE_ID                VarStoreId,
  OUT     H2O_FORM_BROWSER_VS            **VarbleStoreData
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_FORMSET                        *FormSet;
  LIST_ENTRY                                  *LinkStorage;
  FORMSET_STORAGE                             *Storage;
  BROWSER_STORAGE                             *BrowserStorage;
  H2O_FORM_BROWSER_VS                         *VarStoreData;

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);
  DEBUG ((EFI_D_INFO, "H2OFormBrowserGetVSInfo()\n"));
  Form = FBPageIdToForm (Private, PageId);
  if (Form == NULL) {
    return EFI_NOT_FOUND;
  }
  DEBUG ((EFI_D_INFO, "Form->FormSet: %g\n", &Form->FormSet->Guid));
  FormSet = Form->FormSet;
  LinkStorage = GetFirstNode (&FormSet->StorageListHead);
  while (!IsNull (&FormSet->StorageListHead, LinkStorage)) {
    Storage = FORMSET_STORAGE_FROM_LINK (LinkStorage);
    DEBUG ((EFI_D_INFO, "Storage->VarStoreId: %x\n", Storage->VarStoreId));
    if (VarStoreId == Storage->VarStoreId) {
      BrowserStorage = Storage->BrowserStorage;
      VarStoreData = AllocateZeroPool (sizeof (H2O_FORM_BROWSER_VS));
      if (VarStoreData == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      VarStoreData->Signature  = (UINT32)BrowserStorage->Signature;
      VarStoreData->Size       = sizeof (H2O_FORM_BROWSER_VS);
      VarStoreData->Type       = BrowserStorage->Type;
      VarStoreData->VarStoreId = Storage->VarStoreId;
      CopyMem (&VarStoreData->Guid, &BrowserStorage->Guid, sizeof(EFI_GUID));
      VarStoreData->Name       = BrowserStorage->Name;
      VarStoreData->StorageSize = BrowserStorage->Size;
      VarStoreData->Buffer     = BrowserStorage->Buffer;
      VarStoreData->EditBuffer = BrowserStorage->EditBuffer;
      *VarbleStoreData = VarStoreData;
  return EFI_SUCCESS;
    }
    LinkStorage = GetNextNode (&FormSet->StorageListHead, LinkStorage);
  }
  return EFI_NOT_FOUND;
}


/**
  Get all of Variable Store identifiers from specific Page.

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in]  PageId             Input Page identifier.
  @param[out] VarStoreCount      A pointer to the number of Variable Store ID in VarStoreBuffer.
  @param[out] VarStoreBuffer     A pointer to a buffer which will be allocated by the function which contains all
                                 of EFI_VARSTORE_ID instances. The buffer should be freed by the caller.

  @retval EFI_SUCCESS            Get array of Variable Store IDs successful.
  @retval EFI_NOT_FOUND          No Variable Store ID match the search.
  @retval EFI_OUT_OF_RESOURCES   There are not enough resources available to allocate VarStoreBuffer.
  @retval EFI_INVALID_PARAMETER  This is NULL,, VarStoreCount is NULL or VarStoreBuffer is NULL.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetVSAll (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  IN      H2O_PAGE_ID                    PageId,
  OUT     UINTN                          *VarStoreCount,
  OUT     EFI_VARSTORE_ID                **VarStoreBuffer
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_FORMSET                        *FormSet;
  LIST_ENTRY                                  *LinkStorage;
  FORMSET_STORAGE                             *Storage;
  UINTN                                       StorageCount;
  EFI_VARSTORE_ID                             *StorageIdPtr;

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);
  Form = FBPageIdToForm (Private, PageId);
  if (Form == NULL) {
    return EFI_NOT_FOUND;
  }
  FormSet = Form->FormSet;
  StorageCount = 0;
  LinkStorage = GetFirstNode (&FormSet->StorageListHead);
  while (!IsNull (&FormSet->StorageListHead, LinkStorage)) {
    Storage = FORMSET_STORAGE_FROM_LINK (LinkStorage);
    StorageCount++;
    LinkStorage = GetNextNode (&FormSet->StorageListHead, LinkStorage);
  }

  StorageIdPtr = AllocateZeroPool (sizeof(FORMSET_STORAGE) * StorageCount);
  if (StorageIdPtr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *VarStoreCount  = StorageCount;
  *VarStoreBuffer = StorageIdPtr;

  LinkStorage = GetFirstNode (&FormSet->StorageListHead);
  while (!IsNull (&FormSet->StorageListHead, LinkStorage)) {
    Storage = FORMSET_STORAGE_FROM_LINK (LinkStorage);
    LinkStorage = GetNextNode (&FormSet->StorageListHead, LinkStorage);
    *(StorageIdPtr++) = Storage->VarStoreId;
  }

  return EFI_SUCCESS;
}


/**
  Get Default Store data according to specific Default Store identifier.

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in]  PageId             Input Page identifier.
  @param[in]  DefaultStoreId     Specific input Variable Store ID.
  @param[out] DefaultStoreData   A pointer to H2O_FORM_BROWSER_DS instance.

  @retval EFI_SUCCESS            Get Default Store successful.
  @retval EFI_INVALID_PARAMETER  This is NULL, or DefaultStoreData is NULL.
  @retval EFI_NOT_FOUND          Cannot find specific Default Store data.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetDSInfo (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  IN      H2O_PAGE_ID                    PageId,
  IN      EFI_DEFAULT_ID                 DefaultStoreId,
  OUT     H2O_FORM_BROWSER_DS            **DefaultStoreData
  )
{
  return EFI_SUCCESS;
}


/**
  Get all of Default Store identifiers from specific Page.

  @param[in]  This                  A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in]  PageId                Input Page identifier.
  @param[out] DefaultStoreIdCount   A pointer to the number of Default Store ID in DefaultStoreIdBuffer.
  @param[out] DefaultStoreIdBuffer  A pointer to a buffer which will be allocated by the function which contains all
                                    of EFI_DEFAULT_ID instances.

  @retval EFI_SUCCESS               Get array of Default Store IDs successful.
  @retval EFI_NOT_FOUND             No Default Store ID match the search.
  @retval EFI_OUT_OF_RESOURCES      There are not enough resources available to allocate DefaultStoreIdBuffer.
  @retval EFI_INVALID_PARAMETER     This is NULL, DefaultStoreIdCount is NULL or DefaultStoreIdBuffer is NULL.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetDSAll (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  IN      H2O_PAGE_ID                    PageId,
  OUT     UINT32                         *DefaultStoreIdCount,
  OUT     EFI_DEFAULT_ID                 **DefaultStoreIdBuffer
  )
{
  return EFI_SUCCESS;
}


/**
  Add console device to supported list.

  @param[in] This                 A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in] ConsoleDev           A pointer to H2O_FORM_BROWSER_CONSOLE_DEV instance.

  @retval EFI_SUCCESS             Add console device to supported list successful.
  @retval EFI_ALREADY_STARTED     The console device is already in supported list.
  @retval EFI_INVALID_PARAMETER   This is NULL or CosoleDev is NULL.
**/
EFI_STATUS
H2OFormBrowserAddC (
  IN      H2O_FORM_BROWSER_PROTOCOL      *This,
  IN      H2O_FORM_BROWSER_CONSOLE_DEV   *ConsoleDev
  )
{
  return EFI_SUCCESS;
}




/**
  Return information about all installed Consoles, the devices associated with them and
  which Display Engine is currently managing them.

  @param[in]  This               Pointer to current instance of this protocol.
  @param[in]  CId                Unsigned integer that specifies the console index. -1 = all consoles.
  @param[out] CDevCount          Pointer to returned console device count.
  @param[out] CDevs              Pointer to returned pointer to array of console device descriptors.
                                 The array should be freed by the caller. CDevCount indicates
                                 the number of elements in the array.

  @retval EFI_SUCCESS            Get array of console devices successful.
  @retval EFI_NOT_FOUND          No consoleID match the search.
  @retval EFI_OUT_OF_RESOURCES   There are not enough resources available to allocate CDevs.
  @retval EFI_INVALID_PARAMETER  This is NULL, CDevCount is NULL or CDevs is NULL.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetCInfo (
  IN     H2O_FORM_BROWSER_PROTOCOL             *This,
  IN     H2O_CONSOLE_ID                        CId,
  OUT    UINT32                                *CDevCount,
  OUT    H2O_FORM_BROWSER_CONSOLE_DEV          **CDevs
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;
  H2O_FORM_BROWSER_CONSOLE_DEV                *ConsoleDevList;
  UINT32                                       Index;
  UINT32                                       Count;
  UINTN                                       ConsoleDevIndex;

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);

  if (mConsoleDevList == NULL || mConsoleDevListCount == 0) {
    return EFI_NOT_FOUND;
  }

  Count = 0;
  for (Index = 0; Index < mConsoleDevListCount; Index++) {
    if (CId == -1 || mConsoleDevList[Index]->ConsoleId == CId) {
      Count++;
    }
  }


  //
  // the last console device is NULL
  //
  ConsoleDevIndex = 0;
  ConsoleDevList = AllocateZeroPool ((Count + 1) * sizeof (H2O_FORM_BROWSER_CONSOLE_DEV));
  if (ConsoleDevList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  for (Index = 0; Index < mConsoleDevListCount; Index++) {
    if (CId == -1 || mConsoleDevList[Index]->ConsoleId == CId) {
      CopyMem (
        &ConsoleDevList[ConsoleDevIndex],
        mConsoleDevList[Index],
        sizeof (H2O_FORM_BROWSER_CONSOLE_DEV)
        );
      ConsoleDevIndex++;
    }
  }

  *CDevCount = (UINT32) Count;
  *CDevs     = ConsoleDevList;

  return EFI_SUCCESS;
}


/**
  Return array of Console IDs.

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[out] CIdCount           Pointer to returned console ID count.
  @param[out] CIdBuffer          Pointer to returned pointer to array of console ID. The array should be freed by
                                 the caller. CIdCount indicates the number of elements in the array.

  @retval EFI_SUCCESS            Get array of console IDs successful.
  @retval EFI_NOT_FOUND          No console IDs ID match the search.
  @retval EFI_OUT_OF_RESOURCES   There are not enough resources available to allocate CIdBuffer.
  @retval EFI_INVALID_PARAMETER  This is NULL, CIdCount is NULL or CIdBuffer is NULL.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetCAll (
  IN     H2O_FORM_BROWSER_PROTOCOL             *This,
  OUT    UINT32                                *CIdCount,
  OUT    H2O_CONSOLE_ID                        **CIdBuffer
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA                *Private;
  UINT32                                       IdCount;
  H2O_CONSOLE_ID                               *IdBuffer;
  UINT32                                       Index;

  if (This == NULL || CIdCount == NULL || CIdBuffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);
  if (Private->EngineListCount == 0) {
    return EFI_NOT_FOUND;
  }

  IdCount  = Private->EngineListCount;
  IdBuffer = (H2O_CONSOLE_ID *) AllocatePool (sizeof (H2O_CONSOLE_ID) * IdCount);
  if (IdBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < IdCount; Index++) {
    IdBuffer[Index] = Index + 1;
  }

  *CIdCount  = IdCount;
  *CIdBuffer = IdBuffer;

  return EFI_SUCCESS;
}



/**
  Return data about the Setup Menu.

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[out] SetupMenuData      A pointer to returned setup menu data

  @retval EFI_SUCCESS            Get setup menu data successful.
  @retval EFI_INVALID_PARAMETER  This is NULL or SetupMenuData is NULL.
  @retval EFI_NOT_FOUND          Cannot find setup menu data.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetSMInfo (
  IN     H2O_FORM_BROWSER_PROTOCOL            *This,
  OUT    H2O_FORM_BROWSER_SM                  **SetupMenuData
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA               *Private;
  H2O_FORM_BROWSER_SM                         *SetupMenu;

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);

  if (Private->SetupMenuInfo == NULL || Private->NumberOfSetupMenus == 0) {
    return EFI_NOT_FOUND;
  }

  SetupMenu = (H2O_FORM_BROWSER_SM *) AllocateZeroPool (sizeof (H2O_FORM_BROWSER_SM));
  if (SetupMenu == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetupMenu->Signature = H2O_FORMSET_BROWSER_SETUP_MENU_SIGNATURE;
  if (GetScuFormset () != NULL) {
    SetupMenu->CoreVersionString = GetString (STRING_TOKEN (SCU_CORE_VERSION_STRING), mHiiHandle);
    SetupMenu->TitleString       = GetString (STRING_TOKEN (SCU_TITLE_STRING)       , mHiiHandle);
  } else {
    SetupMenu->CoreVersionString = NULL;
    SetupMenu->TitleString       = GetString (gCurrentSelection->FormSet->FormSetTitle, gCurrentSelection->FormSet->HiiHandle);
  }
  CopyGuid (&SetupMenu->FormSetGuid, &gCurrentSelection->FormSet->Guid);
  SetupMenu->NumberOfSetupMenus  = Private->NumberOfSetupMenus;
  SetupMenu->SetupMenuInfoList   = AllocateCopyPool (
                                     sizeof (SETUP_MENU_INFO) * Private->NumberOfSetupMenus,
                                     Private->SetupMenuInfo
                                     );
  if (SetupMenu->SetupMenuInfoList == NULL) {
    FreePool (SetupMenu);
    return EFI_OUT_OF_RESOURCES;
  }

  *SetupMenuData = SetupMenu;

  return EFI_SUCCESS;
}


/**
  Update setup menu data.

  @param[in]  This               A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[out] SetupMenuData      A pointer to input setup menu data

  @retval EFI_SUCCESS            Update setup menu info successful.
  @retval EFI_INVALID_PARAMETER  This is NULL or SetupMenuData is NULL.
  @retval EFI_UNSUPPORTED        Cannot update setup menu data.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserUpdateSMInfo (
  IN     H2O_FORM_BROWSER_PROTOCOL       *This,
  IN     H2O_FORM_BROWSER_SM             *SetupMenuData
  )
{
  return EFI_SUCCESS;
}


/**
  Add the notification to the notification queue and signal the Notification event.

  @param[in] This                A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in] Notify              A pointer to the H2O_DISPLAY_ENGINE_EVT instance.

  @retval EFI_SUCCESS            Register notify successful.
  @retval EFI_INVALID_PARAMETER  This is NULL or Notify is NULL.
**/
EFI_STATUS
H2OFormBrowserNotify (
  IN       H2O_FORM_BROWSER_PROTOCOL     *This,
  IN CONST H2O_DISPLAY_ENGINE_EVT        *Notify
  )
{
  QueueEvent ((H2O_DISPLAY_ENGINE_EVT *)Notify);
  return EFI_SUCCESS;
}


/**
  Add the notification to the notification queue and signal the notification event after
  a period of time.

  @param[in] This                A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[in] TriggerTime         The number of 100ns units until the timer expires.  A
                                 TriggerTime of 0 is legal. If TriggerTime is 0, signal
                                 event immediately.
  @param[in] Notify              A pointer to the H2O_DISPLAY_ENGINE_EVT instance.

  @retval EFI_SUCCESS            Register notify successful.
  @retval EFI_INVALID_PARAMETER  This is NULL or Notify is NULL.
**/
EFI_STATUS
EFIAPI
H2OFormBrowserRegisterTimer (
  IN       H2O_FORM_BROWSER_PROTOCOL     *This,
  IN CONST H2O_DISPLAY_ENGINE_EVT        *Notify,
  IN       UINT64                        TriggerTime
  )
{
  EFI_STATUS                             Status;

  if (This == NULL || Notify == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = FBSetTimer (
             Notify->Target,
             0,
             H2O_FORM_BROWSER_TIMER_TYPE_RELATIVE,
             Notify,
             MultU64x32 (TriggerTime, 100)
             );

  return Status;
}

EFI_STATUS
EFIAPI
H2OFormBrowserGetVfrProp (
  IN     H2O_FORM_BROWSER_PROTOCOL       *This,
  IN     H2O_PAGE_ID                     PageId,
  IN     H2O_STATEMENT_ID                StatementId,
  IN     UINT64                          OptionValue,
  OUT    LIST_ENTRY                      **FormsetPropList,
  OUT    LIST_ENTRY                      **FormPropList,
  OUT    LIST_ENTRY                      **StatementPropList,
  OUT    LIST_ENTRY                      **OptionPropList
  )
{
  H2O_FORM_BROWSER_PRIVATE_DATA          *Private;
  FORM_BROWSER_FORMSET                   *Formset;
  FORM_BROWSER_FORM                      *Form;
  FORM_BROWSER_STATEMENT                 *Statement;
  FORM_BROWSER_STATEMENT                 *TempStatement;
  QUESTION_OPTION                        *Option;
  QUESTION_OPTION                        *TempOption;
  LIST_ENTRY                             *Link;

  Private = H2O_FORM_BROWSER_DATA_FROM_PROTOCOL (This);

  Form = NULL;
  Formset = NULL;
  if (PageId == Private->FB.CurrentP->PageId) {
    Form = FORM_BROWSER_FORM_FROM_H2O_PAGE (Private->FB.CurrentP);
    Formset = Form->FormSet;
  }
  if (Form == NULL || Formset == NULL) {
    return EFI_NOT_FOUND;
  }

  Statement = NULL;
  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    TempStatement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&Form->StatementListHead, Link);

    if (TempStatement->StatementId == StatementId) {
      Statement = TempStatement;
      break;
    }
  }
  if (Statement == NULL) {
    return EFI_NOT_FOUND;
  }

  Option = NULL;
  Link = GetFirstNode (&Statement->OptionListHead);
  while (!IsNull (&Statement->OptionListHead, Link)) {
    TempOption = QUESTION_OPTION_FROM_LINK (Link);
    Link = GetNextNode (&Statement->OptionListHead, Link);

    if (TempOption->Value.Value.u64 == OptionValue) {
      Option = TempOption;
      break;
    }
  }

  if (StatementPropList != NULL) {
    *StatementPropList = &Statement->PropertyListHead;
  }
  if (FormPropList != NULL) {
    *FormPropList = &Form->PropertyListHead;
  }
  if (FormsetPropList != NULL) {
    *FormsetPropList = &Formset->PropertyListHead;
  }
  if (OptionPropList != NULL && Option != NULL) {
    *OptionPropList = &Option->PropertyListHead;
  }

  return EFI_SUCCESS;
}

/**
  Get changed Questions data buffer.

  @param[in]  This                   A pointer to the H2O_FORM_BROWSER_PROTOCOL instance.
  @param[out] ChangedQuestionCount   A pointer to the number of changed Questions in ChangedQuestionBuffer.
  @param[out] ChangedQuestionBuffer  A pointer to a buffer which will be allocated by the function which
                                     contains all changed Questions.

  @retval EFI_SUCCESS                Get changed Questions successful.
  @retval EFI_NOT_FOUND              No changed Questions.

**/
EFI_STATUS
EFIAPI
H2OFormBrowserGetChangedQuestions (
  IN H2O_FORM_BROWSER_PROTOCOL *This,
  OUT UINT32                   *ChangedQuestionCount,
  OUT H2O_FORM_BROWSER_Q       **ChangedQuestionBuffer
  )
{
  LIST_ENTRY                   *LinkFormSet;
  LIST_ENTRY                   *LinkForm;
  LIST_ENTRY                   *LinkStatement;
  FORM_BROWSER_FORMSET         *FormSet;
  FORM_BROWSER_FORM            *Form;
  FORM_BROWSER_STATEMENT       *Statement;
  EXPRESS_RESULT               ExpressResult;
  H2O_FORM_BROWSER_P           *Page;
  UINT32                       QuestionIndex;
  UINT32                       QuestionCount;
  H2O_FORM_BROWSER_Q           *QuestionBufPtr;

  QuestionCount = 0;
  LinkFormSet = GetFirstNode (&gBrowserFormSetList);
  while (!IsNull (&gBrowserFormSetList, LinkFormSet)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_LINK (LinkFormSet);

    LinkForm = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, LinkForm)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);

      LinkStatement = GetFirstNode (&Form->StatementListHead);
      while (!IsNull (&Form->StatementListHead, LinkStatement)) {
        Statement = FORM_BROWSER_STATEMENT_FROM_LINK (LinkStatement);
        if (Statement->ValueChanged == TRUE) {
          //
          // Get the count of questions which has been changed.
          //
          QuestionCount++;
        }

        LinkStatement = GetNextNode (&Form->StatementListHead, LinkStatement);
      }
      LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);
    }
    LinkFormSet = GetNextNode (&gBrowserFormSetList, LinkFormSet);
  }

  if (QuestionCount == 0) {
    *ChangedQuestionCount = 0;
    return EFI_SUCCESS;
  }

  QuestionBufPtr = NULL;
  QuestionBufPtr = AllocateZeroPool (sizeof(H2O_FORM_BROWSER_Q) * QuestionCount);
  if (QuestionBufPtr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  QuestionIndex  = 0;

  LinkFormSet = GetFirstNode (&gBrowserFormSetList);
  while (!IsNull (&gBrowserFormSetList, LinkFormSet)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_LINK (LinkFormSet);

    LinkForm = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, LinkForm)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
      LinkStatement = GetFirstNode (&Form->StatementListHead);
      while (!IsNull (&Form->StatementListHead, LinkStatement)) {
        Statement = FORM_BROWSER_STATEMENT_FROM_LINK (LinkStatement);
        if (Statement->ValueChanged == TRUE) {
          //
          // Initialize the H2O Statement content, and copy to the returned buffer.
          //
          ExpressResult = EvaluateExpressionList(Statement->Expression, FALSE, NULL, NULL);
          Page = &Form->PageInfo;
          InitH2OStatement (
            FormSet,
            Form,
            ExpressResult,
            Page,
            Statement
            );
          CopyOption (Form, Statement);
          CopyMem (&QuestionBufPtr[QuestionIndex], &Statement->Statement, sizeof(H2O_FORM_BROWSER_Q));
          QuestionIndex++;
        }
        LinkStatement = GetNextNode (&Form->StatementListHead, LinkStatement);
      }
      LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);
    }
    LinkFormSet = GetNextNode (&gBrowserFormSetList, LinkFormSet);
  }

  *ChangedQuestionCount  = QuestionCount;
  *ChangedQuestionBuffer = QuestionBufPtr;

  return EFI_SUCCESS;
}

H2O_FORM_BROWSER_PRIVATE_DATA mFBPrivate = {
  H2O_FORM_BROWSER_SIGNATURE,
  NULL,
  {
    sizeof (H2O_FORM_BROWSER_PROTOCOL),
    NULL,
    NULL,
    H2OFormBrowserGetPInfo,
    H2OFormBrowserGetPAll,
    H2OFormBrowserGetSInfo,
    H2OFormBrowserGetQInfo,
    H2OFormBrowserGetQAll,
    H2OFormBrowserGetQDefaults,
    H2OFormBrowserGetVSInfo,
    H2OFormBrowserGetVSAll,
    H2OFormBrowserGetDSInfo,
    H2OFormBrowserGetDSAll,
    H2OFormBrowserAddC,
    H2OFormBrowserGetCInfo,
    H2OFormBrowserGetCAll,
    H2OFormBrowserGetSMInfo,
    H2OFormBrowserUpdateSMInfo,
    H2OFormBrowserNotify,
    H2OFormBrowserRegisterTimer,
    H2OFormBrowserGetVfrProp,
    H2OFormBrowserGetChangedQuestions
  },
  {
    SendForm,
    BrowserCallback
  },
  {
    SetScope,
    RegisterHotKey,
    RegiserExitHandler,
    SaveReminder
  },
  {
    BROWSER_EXTENSION2_VERSION_1,
    SetScope,
    RegisterHotKey,
    RegiserExitHandler,
    IsBrowserDataModified,
    ExecuteAction,
  }
};

/**
 Driver entry point

 @param[in]  ImageHandle  ImageHandle of the loaded driver.
 @param[in]  SystemTable  Pointer to the EFI System Table.

 @retval EFI_SUCCESS         Destroy hotkey information success

**/
EFI_STATUS
EFIAPI
FBEntryPoint (
  IN     EFI_HANDLE                            ImageHandle,
  IN     EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_STATUS                                   Status;

  //
  // Locate required Hii relative protocols
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **) &gHiiDatabase
                  );
  ASSERT_EFI_ERROR (Status);
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID **) &gHiiConfigRouting
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gEfiDevicePathFromTextProtocolGuid,
                  NULL,
                  (VOID **) &mPathFromText
                  );

  ASSERT_EFI_ERROR (Status);
  //
  // Publish our HII data
  //
  mHiiHandle = HiiAddPackages (
                 &gEfiCallerIdGuid,
                 ImageHandle,
                 H2OFormBrowserDxeStrings,
                 NULL
                 );
  DEBUG ((EFI_D_INFO, "Hii add package fail\n"));
  ASSERT (mHiiHandle != NULL);


  InitializeListHead (&mFBPrivate.FormSetList);
  InitializeListHead (&mFBPrivate.TimerList);
  InitializeListHead (&mFBPrivate.RefreshList);
  InitializeListHead (&mFBPrivate.FormBrowserEx2.FormViewHistoryHead);
  InitializeListHead (&mFBPrivate.FormBrowserEx2.OverrideQestListHead);

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mFBPrivate.ImageHandle,
                  &gEfiFormBrowser2ProtocolGuid,
                  &mFBPrivate.FormBrowser2,
                  &gH2OFormBrowserProtocolGuid,
                  &mFBPrivate.FB,
                  &gEdkiiFormBrowserEx2ProtocolGuid,
                  &mFBPrivate.FormBrowserEx2,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  InitializeDisplayFormData ();

  Status = InstallH2ODialogProtocol (mFBPrivate.ImageHandle);
  ASSERT_EFI_ERROR (Status);


  return Status;
}

/**
 Destroy page information

 @param [in] Page            Formbrowser page

**/
STATIC
VOID
FBDestroyPage (
  IN H2O_FORM_BROWSER_P       *Page
  )
{
  Page->Signature = 0;
  FBFreePool ((VOID **)&Page->PageTitle);
  FBFreePool ((VOID **)&Page->StatementIds);
}

/**
 Destroy statement information

 @param [in] H2OStatement    Formbrowser statement

**/
STATIC
VOID
FBDestroyStatement (
  IN H2O_FORM_BROWSER_S       *H2OStatement
  )
{
  FBFreePool ((VOID **)&H2OStatement->Prompt);
  FBFreePool ((VOID **)&H2OStatement->Help);
  FBFreePool ((VOID **)&H2OStatement->TextTwo);
}
/**
 Destroy hotkey information

 @param [in] Private         Formbrowser private data

 @retval EFI_SUCCESS         Destroy hotkey information success

**/
EFI_STATUS
DestroyHotKeyInfo (
  IN H2O_FORM_BROWSER_PRIVATE_DATA       *Private
  )
{
  UINT32                                       Index;

  for (Index = 0; Index < Private->NumberOfHotKeys; Index ++) {
    FBFreePool ((VOID **)&Private->HotKeyInfo[Index].Mark);
    FBFreePool ((VOID **)&Private->HotKeyInfo[Index].String);
  }
  Private->NumberOfHotKeys = 0;
  FBFreePool ((VOID **)&Private->HotKeyInfo);

  return EFI_SUCCESS;
}
/**
 Destroy page information

 @param [in] Private         Formbrowser private data

 @retval EFI_SUCCESS         Destroy page information success

**/
EFI_STATUS
DestroyPageInfo (
  IN H2O_FORM_BROWSER_PRIVATE_DATA       *Private
  )
{
  FORM_BROWSER_FORMSET                        *FormSet;
  FORM_BROWSER_FORM                           *Form;
  FORM_BROWSER_STATEMENT                      *Statement;
  LIST_ENTRY                                  *LinkFormSet;
  LIST_ENTRY                                  *LinkForm;
  LIST_ENTRY                                  *LinkStatement;
  H2O_FORM_BROWSER_P                          *Page;
  H2O_FORM_BROWSER_S                          *H2OStatement;
  H2O_FORM_BROWSER_O                          *H2OOption;
  LIST_ENTRY                                  *LinkOption;

  //
  // Calculate all form of Form Set
  //
  LinkFormSet = GetFirstNode (&Private->FormSetList);
  while (!IsNull (&Private->FormSetList, LinkFormSet)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (LinkFormSet);
    LinkFormSet = GetNextNode (&Private->FormSetList, LinkFormSet);

    LinkForm = GetFirstNode (&FormSet->FormListHead);
    while (!IsNull (&FormSet->FormListHead, LinkForm)) {
      Form = FORM_BROWSER_FORM_FROM_LINK (LinkForm);
      LinkForm = GetNextNode (&FormSet->FormListHead, LinkForm);

      Page = &Form->PageInfo;
      if (Page->Signature != H2O_FORM_PAGE_SIGNATURE) {
        continue;
      }

      FBDestroyPage(Page);

      LinkStatement = GetFirstNode (&Form->StatementListHead);
      while (!IsNull (&Form->StatementListHead, LinkStatement)) {
        Statement = FORM_BROWSER_STATEMENT_FROM_LINK (LinkStatement);
        LinkStatement = GetNextNode (&Form->StatementListHead, LinkStatement);
        H2OStatement = &Statement->Statement;
        FBDestroyStatement(H2OStatement);


        H2OOption = H2OStatement->Options;
        LinkOption = GetFirstNode (&Statement->OptionListHead);
        while (!IsNull (&Statement->OptionListHead, LinkOption)) {
          LinkOption = GetNextNode (&Statement->OptionListHead, LinkOption);

          FBFreePool ((VOID **)&H2OOption->Text);
          H2OOption++;
        }
        FBFreePool ((VOID **)&H2OStatement->Options);
      }
    }
  }

  FBFreePool ((VOID **) &Private->PageIdList);

  DestroyHotKeyInfo (Private);

  return EFI_SUCCESS;
}
/**
 Destroy FormSet list

 @param [in] Private         Formbrowser private data

 @retval EFI_SUCCESS         Destroy FormSet list success

**/
EFI_STATUS
DestroyFormSetList (
  IN H2O_FORM_BROWSER_PRIVATE_DATA       *Private
  )
{
  LIST_ENTRY                             *Link;
  FORM_BROWSER_FORMSET                   *FormSet;

  Link = GetFirstNode (&Private->FormSetList);
  while (!IsNull (&Private->FormSetList, Link)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (Link);
    Link = GetNextNode (&Private->FormSetList, Link);

    RemoveEntryList (&FormSet->Link);
    DestroyFormSet (FormSet);
  }

  return EFI_SUCCESS;
}
/**
 Destroy setup menu information

 @param [in] Private         Formbrowser private data

 @retval EFI_SUCCESS         Destroy setup menu information success

**/
EFI_STATUS
DestroySetupMenuInfo (
  IN H2O_FORM_BROWSER_PRIVATE_DATA       *Private
  )
{
  UINT32                                    Index;

  if (Private->NumberOfSetupMenus != 0 && Private->SetupMenuInfo != NULL) {
    for (Index = 0; Index < (UINT32)Private->NumberOfSetupMenus; Index++) {
      FBFreePool ((VOID **) &Private->SetupMenuInfo[Index].PageTitle);
      if (Private->SetupMenuInfo[Index].PageImage != NULL) {
        FreePool (Private->SetupMenuInfo[Index].PageImage->Bitmap);
        FreePool (Private->SetupMenuInfo[Index].PageImage);
      }
    }
  }

  FBFreePool ((VOID **)&Private->SetupMenuInfo);
  Private->NumberOfSetupMenus = 0;

  return EFI_SUCCESS;
}

/**
  Initialize the HII String Token to the correct values.

**/
VOID
InitializeBrowserStrings (
  VOID
  )
{
  FreeBrowserStrings ();

  gPressEnter           = GetString (STRING_TOKEN (PRESS_ENTER), mHiiHandle);
  gEmptyString          = GetString (STRING_TOKEN (EMPTY_STRING), mHiiHandle);
  gAreYouSure           = GetString (STRING_TOKEN (ARE_YOU_SURE), mHiiHandle);
  gYesResponse          = GetString (STRING_TOKEN (ARE_YOU_SURE_YES), mHiiHandle);
  gNoResponse           = GetString (STRING_TOKEN (ARE_YOU_SURE_NO), mHiiHandle);
  gSaveChanges          = GetString (STRING_TOKEN (SAVE_CHANGES), mHiiHandle);

  gSpaceString          = GetString (STRING_TOKEN (STR_SPACE), mHiiHandle);

  gSaveFailed           = GetString (STRING_TOKEN (SAVE_FAILED), mHiiHandle);
  gFormNotFound         = GetString (STRING_TOKEN (STATUS_BROWSER_FORM_NOT_FOUND), mHiiHandle);
  gFormSuppress         = GetString (STRING_TOKEN (FORM_SUPPRESSED), mHiiHandle);
  gProtocolNotFound     = GetString (STRING_TOKEN (PROTOCOL_NOT_FOUND), mHiiHandle);
  gBrwoserError         = GetString (STRING_TOKEN (STATUS_BROWSER_ERROR), mHiiHandle);
}

/**
  Free up the resource allocated for all strings required
  by Setup Browser.

**/
VOID
FreeBrowserStrings (
  VOID
  )
{
  FBFreePool ((VOID **)&gPressEnter);
  FBFreePool ((VOID **)&gEmptyString);
  FBFreePool ((VOID **)&gAreYouSure);
  FBFreePool ((VOID **)&gYesResponse);
  FBFreePool ((VOID **)&gNoResponse);
  FBFreePool ((VOID **)&gSaveChanges);

  FBFreePool ((VOID **)&gSpaceString);

  FBFreePool ((VOID **)&gSaveFailed);
  FBFreePool ((VOID **)&gFormNotFound);
  FBFreePool ((VOID **)&gFormSuppress);
  FBFreePool ((VOID **)&gProtocolNotFound);
  FBFreePool ((VOID **)&gBrwoserError);
}

