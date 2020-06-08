/** @file

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

#include <H2ODisplayEngineLocalText.h>
#include <LTDESpecificQuestionBehavior.h>
#include <LTDEControl.h>

EFI_STATUS
DERefreshQuestion (
  IN H2O_FORM_BROWSER_PROTOCOL                *FBProtocol,
  IN BOOLEAN                                  HightLight,
  IN BOOLEAN                                  RefreshHelpStr,
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  );

H2O_CONTROL_INFO *
GetControlInfo (
  IN H2O_CONTROL_INFO                         *ControlArray,
  IN UINT32                                   ControlCount,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  );

EFI_STATUS
CheckFBHotKey (
  IN  H2O_DISPLAY_ENGINE_PRIVATE_DATA       *Private,
  IN  H2O_DISPLAY_ENGINE_USER_INPUT_DATA    *UserInputData,
  OUT H2O_EVT_TYPE                          *HotKeyEvtType,
  OUT BOOLEAN                               *IsIncreaseHotKey
  )
{
  H2O_DISPLAY_ENGINE_USER_INPUT_DATA        InputData;
  HOT_KEY_INFO                              *HotKey;
  EFI_KEY_DATA                              *KeyData;
  H2O_CONTROL_INFO                          Control;
  H2O_CONTROL_INFO                          *ControlPtr;
  H2O_PANEL_INFO                            *Panel;
  H2O_EVT_TYPE                              EvtType;
  BOOLEAN                                   IsIncrease;

  if (Private == NULL || UserInputData == NULL || HotKeyEvtType == NULL || IsIncreaseHotKey == NULL ||
      Private->FBProtocol->CurrentP->HotKeyInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (&InputData, UserInputData, sizeof (InputData));

  //
  // Translate to hot key keyboard value if user input is not keyboard.
  //
  if (!InputData.IsKeyboard) {
    ZeroMem (&Control, sizeof (H2O_CONTROL_INFO));
    ControlPtr = &Control;

    Panel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_HOTKEY);
    if (Panel == NULL) {
      return EFI_NOT_FOUND;
    }
    if (!CheckPressControls (InputData.IsKeyboard, &InputData.KeyData.Key, InputData.CursorX, InputData.CursorY, Panel->ControlList.Count, Panel->ControlList.ControlArray, &ControlPtr)) {
      return EFI_NOT_FOUND;
    }
    if (ControlPtr == NULL || ControlPtr->HiiValue.Buffer == NULL) {
      return EFI_NOT_FOUND;
    }

    CopyMem (&InputData.KeyData, (EFI_KEY_DATA *)(UINTN *)ControlPtr->HiiValue.Buffer, sizeof (EFI_KEY_DATA));
  }

  //
  // Get hot key event type
  //
  KeyData    = &InputData.KeyData;
  HotKey     = Private->FBProtocol->CurrentP->HotKeyInfo;
  EvtType    = 0;
  IsIncrease = FALSE;

  while (HotKey->String != NULL) {
    if ((HotKey->KeyData.Key.ScanCode    == KeyData->Key.ScanCode) &&
        (HotKey->KeyData.Key.UnicodeChar == KeyData->Key.UnicodeChar)) {
      EvtType = HotKey->SendEvtType;
      IsIncrease = TRUE;
    } else if (EvtType == HotKey->SendEvtType) {
      IsIncrease = FALSE;
    }
    HotKey++;
  }

  if (EvtType == 0) {
    return EFI_NOT_FOUND;
  }

  *HotKeyEvtType    = EvtType;
  *IsIncreaseHotKey = IsIncrease;

  return EFI_SUCCESS;
}

EFI_STATUS
CheckDEHotKey (
  IN  H2O_DISPLAY_ENGINE_PRIVATE_DATA       *Private,
  IN  H2O_DISPLAY_ENGINE_USER_INPUT_DATA    *UserInputData,
  OUT H2O_EVT_TYPE                          *HotKeyEvtType,
  OUT BOOLEAN                               *IsIncreaseHotKey
  )
{
  H2O_DISPLAY_ENGINE_USER_INPUT_DATA        InputData;
  H2O_PANEL_INFO                            *Panel;

  if (Private == NULL || UserInputData == NULL || HotKeyEvtType == NULL || IsIncreaseHotKey == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *IsIncreaseHotKey = FALSE;
  CopyMem (&InputData, UserInputData, sizeof (InputData));

  switch (InputData.KeyData.Key.ScanCode) {

  case SCAN_PAGE_UP:
  case SCAN_PAGE_DOWN:
    //
    // BUGBUG: Page Up and Page Down
    //
    Panel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_PAGE);
    if (InputData.KeyData.Key.ScanCode == SCAN_PAGE_UP) {
      PanelPageUp (Panel);
    } else {
      PanelPageDown (Panel);
    }

    *HotKeyEvtType = H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH;
     return EFI_SUCCESS;

  case SCAN_ESC:
    *HotKeyEvtType = H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_EXIT;
     return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}


BOOLEAN
IsValidHighlightStatement (
  IN H2O_CONTROL_INFO                            *StatementControl
  )
{
  if (StatementControl == NULL) {
    return FALSE;
  }

  if (StatementControl->Selectable) {
    return TRUE;
  }

  return FALSE;
}

/**
  Check input question ID is current highlight or not

  @param[in] Private             A pointer to private data
  @param[in] QuestionId          Query question ID
  @param[in] IfrOpCode           IFR opcode pointer of Query question

  @retval TRUE                   Query question is current highlight
  @retval FALSE                  Query question is not current highlight
**/
BOOLEAN
IsCurrentHighlight (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private,
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  )
{
  if (Private != NULL &&
      Private->MenuSelected != NULL &&
      Private->MenuSelected->PageId == PageId &&
      ((QuestionId != 0    && Private->MenuSelected->QuestionId == QuestionId) ||
       (IfrOpCode  != NULL && Private->MenuSelected->IfrOpCode  == IfrOpCode))) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
SendEvtByHotKey (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private,
  IN H2O_EVT_TYPE                             HotKeyEvtType,
  IN BOOLEAN                                  IsIncreaseHotKey
  )
{
  EFI_STATUS                                  Status;
  EFI_INPUT_KEY                               NewKey;
  EFI_HII_VALUE                               HiiValue;
  EFI_TIME                                    EfiTime;
  H2O_CONTROL_INFO                            *MenuSelected;
  H2O_CONTROL_INFO                            *PageTagSelected;
  H2O_CONTROL_INFO                            *TempPageTagSelected;
  H2O_PANEL_INFO                              *SetupMenuPanel;
  H2O_PANEL_INFO                              *SetupPagePanel;

  if (Private == NULL || HotKeyEvtType == 0) {
    return EFI_INVALID_PARAMETER;
  }

  SetupMenuPanel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_MENU);
  SetupPagePanel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_PAGE);

  switch (HotKeyEvtType) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_Q:
    if (SetupMenuPanel == NULL || Private->MenuSelected == NULL || SetupPagePanel == NULL) {
      break;
    }
    if (Private->MenuSelected->Operand == EFI_IFR_ORDERED_LIST_OP) {
      if (IsIncreaseHotKey) {
        if (Private->MenuSelected->Sequence + 1 < (UINT8) Private->FBProtocol->CurrentQ->NumberOfOptions) {
          Private->MenuSelected->Sequence++;
          return DisplayHighLightControl (FALSE, Private->MenuSelected);
        }
      } else {
        if (Private->MenuSelected->Sequence > 0) {
          Private->MenuSelected->Sequence--;
          return DisplayHighLightControl (FALSE, Private->MenuSelected);
        }
      }
    }
    MenuSelected = FindNextSelectableControl (
                     SetupPagePanel->ControlList.ControlArray,
                     SetupPagePanel->ControlList.Count,
                     Private->MenuSelected,
                     IsIncreaseHotKey,
                     FALSE
                     );
    if (MenuSelected != NULL && IsValidHighlightStatement (MenuSelected)) {
      SendSelectQNotify (
        MenuSelected->PageId,
        MenuSelected->QuestionId,
        MenuSelected->IfrOpCode
        );
    }
    break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_P:
      if (SetupPagePanel == NULL || SetupMenuPanel == NULL) {
        break;
      }

      if (!IsRootPage ()) {
        //
        // Do nothing when it is not in root Form.
        //
        return EFI_SUCCESS;
      }

      NewKey.UnicodeChar  = CHAR_NULL;
      NewKey.ScanCode     = IsIncreaseHotKey ? SCAN_RIGHT : SCAN_LEFT;
      TempPageTagSelected = Private->PageTagSelected;

      while (TRUE) {
        PageTagSelected = TempPageTagSelected;
        CheckPressControls (TRUE, &NewKey, 0, 0, SetupMenuPanel->ControlList.Count, SetupMenuPanel->ControlList.ControlArray, &PageTagSelected);
        if (PageTagSelected->Selectable) {
          Private->PageTagSelected = PageTagSelected;
          SendSelectPNotify (PageTagSelected->PageId);
          break;
        }

        if ((UINTN)(UINTN *) PageTagSelected == (UINTN)(UINTN*) &SetupPagePanel->ControlList.ControlArray[0] ||
            (UINTN)(UINTN *) PageTagSelected == sizeof (H2O_CONTROL_INFO) * (SetupPagePanel->ControlList.Count - 1) + (UINTN)(UINTN *) &SetupPagePanel->ControlList.ControlArray[0]
            ) {
          //
          // If
          // 1.It is a "keyboard select" and select first page ||
          // 2.It is a "keyboard select" and select end page ||
          // Then, do not search the next selectable page.
          //
          break;
        }
        TempPageTagSelected = PageTagSelected;
      }
      break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGE_Q:
      if (SetupMenuPanel == NULL || Private->MenuSelected == NULL || SetupPagePanel == NULL) {
        break;
      }

      switch (Private->MenuSelected->Operand) {

      case EFI_IFR_DATE_OP:
      case EFI_IFR_TIME_OP:
        TransferHiiValueToEfiTime (&Private->MenuSelected->HiiValue, &EfiTime);
        Status = GetNextDateTimeValue (
                   DateTimeOpCodeGetItemValue (Private->MenuSelected),
                   IsIncreaseHotKey,
                   &EfiTime
                   );
        if (EFI_ERROR (Status)) {
          return Status;
        }
        TransferEfiTimeToHiiValue (Private->MenuSelected->Operand == EFI_IFR_DATE_OP, &EfiTime, &Private->MenuSelected->HiiValue);
        SendChangeQNotify (
          Private->MenuSelected->PageId,
          Private->MenuSelected->QuestionId,
          &Private->MenuSelected->HiiValue
          );
        break;

      case EFI_IFR_ORDERED_LIST_OP:
        OrderListOpCodeShiftOrder (Private->MenuSelected, !IsIncreaseHotKey);
        break;

      default:
        Status = GetNextQuestionValue (Private->FBProtocol->CurrentQ, IsIncreaseHotKey, &HiiValue);
        if (EFI_ERROR (Status)) {
          return Status;
        }

        SendChangeQNotify (
          Private->FBProtocol->CurrentQ->PageId,
          Private->FBProtocol->CurrentQ->QuestionId,
          &HiiValue
          );
        break;
      }
      break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_DEFAULT:
      SendDefaultNotify ();
      break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_EXIT:
      SendSubmitExitNotify ();
      break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_EXIT:
      SendDiscardExitNotify ();
      break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT:
      SendSubmitNotify ();
      break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD:
      SendDiscardNotify ();
      break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_SHOW_HELP:
      SendShowHelpNotify ();
      break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH:
      SendRefreshNotify ();
      break;

    case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_Q:
      if (Private->FBProtocol->CurrentQ != NULL && Private->FBProtocol->CurrentQ->Selectable) {
        SendOpenQNotify (
          Private->FBProtocol->CurrentQ->PageId,
          Private->FBProtocol->CurrentQ->QuestionId,
          Private->FBProtocol->CurrentQ->IfrOpCode
          );
      }
      break;

    default:
      return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InitSetupMenu (
  VOID
  )
{
  EFI_STATUS                                     Status;
  UINT32                                         Index;
  H2O_FORM_BROWSER_SM                            *SetupMenuData;
  UINT32                                         SetupMenuInfoCount;
  SETUP_MENU_INFO                                *SetupMenuInfoList;
  H2O_PANEL_INFO                                 *SetupMenuPanel;
  H2O_CONTROL_INFO                               *Control;
  H2O_CONTROL_INFO                               *ControlArray;

  SetupMenuPanel = GetPanelInfoByType (&mDEPrivate->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_MENU);
  if (SetupMenuPanel == NULL) {
    return EFI_SUCCESS;
  }

  FreeControlList (&SetupMenuPanel->ControlList);

  if (!IsVisibility (SetupMenuPanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL)) {
    return EFI_SUCCESS;
  }

  Status = mDEPrivate->FBProtocol->GetSMInfo (mDEPrivate->FBProtocol, &SetupMenuData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SetupMenuInfoCount = SetupMenuData->NumberOfSetupMenus;
  SetupMenuInfoList  = SetupMenuData->SetupMenuInfoList;
  if (SetupMenuInfoCount == 0 || SetupMenuInfoList == NULL) {
    FreeSetupMenuData (SetupMenuData);
    return EFI_SUCCESS;
  }

  //
  // Initialize contorls of setup menu
  //
  ControlArray = (H2O_CONTROL_INFO *) AllocateZeroPool (sizeof (H2O_CONTROL_INFO) * SetupMenuInfoCount);
  if (ControlArray == NULL) {
    FreeSetupMenuData (SetupMenuData);
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < SetupMenuInfoCount; Index++) {
    Control                           = &ControlArray[Index];
    Control->Visible                  = IsRootPage ();
    Control->PageId                   = SetupMenuInfoList[Index].PageId;
    Control->Selectable               = TRUE;
    Control->ParentPanel              = SetupMenuPanel;
    Control->ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_SELECTABLE;

    if (SetupMenuInfoList[Index].PageTitle != NULL) {
      Control->Text.String = CatSPrint (NULL, L"%s%s%s", L" ", SetupMenuInfoList[Index].PageTitle, L" ");
    }
    if (SetupMenuInfoList[Index].PageImage != NULL) {
      CopyMem (&Control->HelpImage, SetupMenuInfoList[Index].PageImage, sizeof (EFI_IMAGE_INPUT));
    }
  }

  SetupMenuPanel->ControlList.Count        = SetupMenuInfoCount;
  SetupMenuPanel->ControlList.ControlArray = ControlArray;

  FreeSetupMenuData (SetupMenuData);
  return EFI_SUCCESS;
}

/**
 Initialize hotkey bar data

 @param[in] HotKey               Pointer to hot key information

 @retval EFI_SUCCESS             Initialize hotkey bar data successfully
 @retval EFI_INVALID_PARAMETER   HotKey pointer is NULL or there is no hot key data
 @retval EFI_NOT_FOUND           There is no hot key panel information
 @retval EFI_OUT_OF_RESOURCES    Allocate pool fail
**/
EFI_STATUS
InitHotKeyBar (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA             *Private
  )
{
  HOT_KEY_INFO                                   *HotKey;
  H2O_PANEL_INFO                                 *HotKeyPanel;
  UINT32                                         Index;
  UINT32                                         HotKeyCount;
  H2O_CONTROL_INFO                               *HotKeyControls;
  H2O_CONTROL_INFO                               *HotKeyDescriptionControls;

  if (Private->FBProtocol == NULL || Private->FBProtocol->CurrentP == NULL || Private->FBProtocol->CurrentP->HotKeyInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HotKeyPanel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_HOTKEY);
  if (HotKeyPanel == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Count the number of hot keys
  //
  HotKey      = Private->FBProtocol->CurrentP->HotKeyInfo;
  HotKeyCount = 0;
  while (HotKey[HotKeyCount].String != NULL) {
    HotKeyCount++;
  }
  if (HotKeyCount == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Init hot key bar data
  //
  FreeControlList (&HotKeyPanel->ControlList);
  HotKeyPanel->ControlList.Count        = HotKeyCount * 2;
  HotKeyPanel->ControlList.ControlArray = AllocateZeroPool (sizeof (H2O_CONTROL_INFO) * HotKeyPanel->ControlList.Count);
  if (HotKeyPanel->ControlList.ControlArray == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  HotKeyControls            = HotKeyPanel->ControlList.ControlArray;
  HotKeyDescriptionControls = HotKeyPanel->ControlList.ControlArray + HotKeyCount;

  for (Index = 0; Index < HotKeyCount; Index++) {
    HotKeyControls[Index].Text.String              = AllocateCopyPool (StrSize (HotKey[Index].Mark), HotKey[Index].Mark);
    HotKeyControls[Index].HiiValue.BufferLen       = sizeof (EFI_KEY_DATA);
    HotKeyControls[Index].HiiValue.Buffer          = (UINT8 *) &HotKey[Index].KeyData;
    HotKeyControls[Index].ParentPanel              = HotKeyPanel;
    HotKeyControls[Index].ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_HIGHLIGHT;

    HotKeyDescriptionControls[Index].Text.String              = AllocateCopyPool (StrSize (HotKey[Index].String), HotKey[Index].String);
    HotKeyDescriptionControls[Index].ParentPanel              = HotKeyPanel;
    HotKeyDescriptionControls[Index].ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_NORMAL;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InitSetupPage (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA         *Private
  )
{
  EFI_STATUS                                 Status;
  H2O_FORM_BROWSER_PROTOCOL                  *FBProtocol;
  H2O_FORM_BROWSER_S                         *Statement;
  UINT32                                     Index;
  LIST_ENTRY                                 *PanelListHead;
  UINT32                                     NumberOfStatementIds;
  UINT32                                     MenuCount;
  H2O_CONTROL_INFO                           *PromptArray;
  H2O_CONTROL_INFO                           *HelpMsgArray;
  H2O_PANEL_INFO                             *SetupPagePanel;
  H2O_PANEL_INFO                             *HelpTextPanel;
  H2O_CONTROL_LIST                           *SetupPageControls;
  H2O_CONTROL_LIST                           *HelpControls;
  UINT32                                     PseudoClass;
  H2O_CONTROL_INFO                           *Control;
  H2O_CONTROL_INFO                           *ControlArray;

  PanelListHead = &Private->Layout->PanelListHead;
  if (IsNull (PanelListHead, PanelListHead->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  FBProtocol           = Private->FBProtocol;
  NumberOfStatementIds = FBProtocol->CurrentP->NumberOfStatementIds;
  PromptArray  = AllocateZeroPool (sizeof (H2O_CONTROL_INFO) * NumberOfStatementIds);
  HelpMsgArray = AllocateZeroPool (sizeof (H2O_CONTROL_INFO) * NumberOfStatementIds);
  if (PromptArray == NULL || HelpMsgArray == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetupPagePanel = GetPanelInfoByType (PanelListHead, H2O_PANEL_TYPE_SETUP_PAGE);
  HelpTextPanel  = GetPanelInfoByType (PanelListHead, H2O_PANEL_TYPE_HELP_TEXT);

  MenuCount = 0;
  for (Index = 0; Index < NumberOfStatementIds; Index++) {
    Status = FBProtocol->GetSInfo (FBProtocol, FBProtocol->CurrentP->PageId, FBProtocol->CurrentP->StatementIds[Index], &Statement);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (Statement->Selectable) {
      PseudoClass = H2O_STYLE_PSEUDO_CLASS_SELECTABLE;
    } else if (Statement->GrayedOut) {
      PseudoClass = H2O_STYLE_PSEUDO_CLASS_GRAYOUT;
    } else {
      PseudoClass = H2O_STYLE_PSEUDO_CLASS_NORMAL;
    }

    //
    // Fill Prompt control info
    //
    CopyMem (&PromptArray[MenuCount].HiiValue, &Statement->HiiValue, sizeof (EFI_HII_VALUE));
    PromptArray[MenuCount].Text.String         = GetQuestionPromptStr (Statement);
    PromptArray[MenuCount].ValueStrInfo.String = GetQuestionValueStr (Statement);
    PromptArray[MenuCount].PageId      = Statement->PageId;
    PromptArray[MenuCount].StatementId = Statement->StatementId;
    PromptArray[MenuCount].QuestionId  = Statement->QuestionId;
    PromptArray[MenuCount].Operand     = Statement->Operand;
    PromptArray[MenuCount].IfrOpCode   = Statement->IfrOpCode;
    PromptArray[MenuCount].Visible     = TRUE;
    PromptArray[MenuCount].Selectable  = Statement->Selectable;
    PromptArray[MenuCount].Modifiable  = TRUE;
    PromptArray[MenuCount].ParentPanel = SetupPagePanel;
    if (Statement->Image != NULL) {
      CopyMem (&PromptArray[MenuCount].HelpImage, Statement->Image, sizeof (EFI_IMAGE_INPUT));
    }
    PromptArray[MenuCount].ControlStyle.StyleType   = GetStyleTypeByOpCode (Statement->Operand);
    PromptArray[MenuCount].ControlStyle.PseudoClass = PseudoClass;

    //
    // Fill Help Message control info
    //
    CopyMem (&HelpMsgArray[MenuCount].HiiValue, &Statement->HiiValue, sizeof (EFI_HII_VALUE));
    HelpMsgArray[MenuCount].Text.String = (Statement->Help == NULL || *Statement->Help == L'\0') ? AllocateCopyPool (sizeof(L" "), L" ") :
                                                                                                   AllocateCopyPool (StrSize (Statement->Help), Statement->Help);
    HelpMsgArray[MenuCount].PageId      = Statement->PageId;
    HelpMsgArray[MenuCount].StatementId = Statement->StatementId;
    HelpMsgArray[MenuCount].QuestionId  = Statement->QuestionId;
    HelpMsgArray[MenuCount].Operand     = Statement->Operand;
    HelpMsgArray[MenuCount].IfrOpCode   = Statement->IfrOpCode;
    HelpMsgArray[MenuCount].Visible     = TRUE;
    HelpMsgArray[MenuCount].Selectable  = Statement->Selectable;
    HelpMsgArray[MenuCount].Modifiable  = TRUE;
    HelpMsgArray[MenuCount].ParentPanel = HelpTextPanel;
    HelpMsgArray[MenuCount].ControlStyle.StyleType   = GetStyleTypeByOpCode (Statement->Operand);
    HelpMsgArray[MenuCount].ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_NORMAL;

    MenuCount++;
    SafeFreePool ((VOID **) &Statement);
  }

  if (SetupPagePanel != NULL) {
    ControlArray = AllocateCopyPool (sizeof (H2O_CONTROL_INFO) * MenuCount, PromptArray);

    if (FBProtocol->CurrentQ != NULL &&
        IsCurrentHighlight (Private, FBProtocol->CurrentQ->PageId, FBProtocol->CurrentQ->QuestionId, FBProtocol->CurrentQ->IfrOpCode)) {
      Control = GetControlInfo (
                  ControlArray,
                  MenuCount,
                  Private->MenuSelected->QuestionId,
                  Private->MenuSelected->IfrOpCode
                  );
      if (Control != NULL) {
        Control->Sequence     = Private->MenuSelected->Sequence;
        Private->MenuSelected = Control;
      } else {
        Private->MenuSelected = NULL;
      }
    } else {
      Private->MenuSelected = NULL;
    }

    SetupPageControls = &SetupPagePanel->ControlList;
    FreeControlList (SetupPageControls);
    SafeFreePool ((VOID **) &SetupPagePanel->ContentsImage.CurrentBlt);

    SetupPageControls->Count        = MenuCount;
    SetupPageControls->ControlArray = ControlArray;
  }

  if (HelpTextPanel != NULL) {
    //
    // Help Text Panel
    //
    HelpControls = &HelpTextPanel->ControlList;
    FreeControlList (HelpControls);
    SafeFreePool ((VOID **) &HelpTextPanel->ContentsImage.CurrentBlt);

    HelpControls->Count        = MenuCount;
    HelpControls->ControlArray = (H2O_CONTROL_INFO*) AllocateCopyPool (sizeof (H2O_CONTROL_INFO) * MenuCount, HelpMsgArray);
  }

  SafeFreePool ((VOID **) &PromptArray);
  SafeFreePool ((VOID **) &HelpMsgArray);

  return EFI_SUCCESS;
}

H2O_CONTROL_INFO *
GetControlInfo (
  IN H2O_CONTROL_INFO                         *ControlArray,
  IN UINT32                                   ControlCount,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  )
{
  UINT32                                      Index;

  if (ControlArray == NULL || ControlCount == 0) {
    return NULL;
  }

  for (Index = 0; Index < ControlCount; Index++) {
    if ((QuestionId != 0    && ControlArray[Index].QuestionId == QuestionId) ||
        (IfrOpCode  != NULL && ControlArray[Index].IfrOpCode  == IfrOpCode)) {
      return &ControlArray[Index];
    }
  }

  return NULL;
}

EFI_STATUS
DERefresh (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      Index;
  H2O_PANEL_INFO                              *SetupPagePanel;
  H2O_PANEL_INFO                              *HelpTextPanel;
  H2O_CONTROL_INFO                            *Control;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  RECT                                       AbsField;
  INT32                                      BorderLineWidth;
  UINT32                                     Attribute;

  Private->DEStatus = DISPLAY_ENGINE_STATUS_AT_MENU;

  SetupPagePanel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL) {
    return EFI_SUCCESS;
  }

  FBProtocol = Private->FBProtocol;

  //
  // Init attribute and location for each item of menu
  //
  Status = InitSetupPage (Private);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = DisplaySetupPage (FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Display Current Question (High-light)
  //
  if (FBProtocol->CurrentQ != NULL &&
      IsCurrentHighlight (Private, FBProtocol->CurrentQ->PageId, FBProtocol->CurrentQ->QuestionId, FBProtocol->CurrentQ->IfrOpCode)) {
    DERefreshQuestion (
      FBProtocol,
      TRUE,
      TRUE,
      Private->MenuSelected->PageId,
      Private->MenuSelected->QuestionId,
      Private->MenuSelected->IfrOpCode
      );
    return EFI_SUCCESS;
  }

  //
  // mDEPrivate->MenuSelected is not ready.
  // First, check if current question is in current page. If not, select question on first valid highlight question.
  //
  Private->MenuSelected = NULL;
  Control = NULL;
  if (FBProtocol->CurrentQ != NULL) {
    Control = GetControlInfo (
                SetupPagePanel->ControlList.ControlArray,
                SetupPagePanel->ControlList.Count,
                FBProtocol->CurrentQ->QuestionId,
                FBProtocol->CurrentQ->IfrOpCode
                );
  }
  if (Control == NULL || !IsValidHighlightStatement (Control)) {
    Control = NULL;
    for (Index = 0; Index < SetupPagePanel->ControlList.Count; Index++) {
      if (IsValidHighlightStatement (&SetupPagePanel->ControlList.ControlArray[Index])) {
        Control = &SetupPagePanel->ControlList.ControlArray[Index];
        break;
      }
    }
  }

  if (Control != NULL) {
    SendSelectQNotify (
      Control->PageId,
      Control->QuestionId,
      Control->IfrOpCode
      );
  } else {
    HelpTextPanel = GetPanelInfoByType (&mDEPrivate->Layout->PanelListHead, H2O_PANEL_TYPE_HELP_TEXT);
    if (HelpTextPanel != NULL) {
      if (SetupPagePanel->ControlList.Count != 0) {
        Control = GetControlInfo (
                    HelpTextPanel->ControlList.ControlArray,
                    HelpTextPanel->ControlList.Count,
                    SetupPagePanel->ControlList.ControlArray[0].QuestionId,
                    SetupPagePanel->ControlList.ControlArray[0].IfrOpCode
                    );
      }

      if (Control != NULL) {
        DisplayHighLightControl (FALSE, Control);
      } else {
        BorderLineWidth = GetBorderWidth (HelpTextPanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);
        CopyRect (&AbsField, &HelpTextPanel->PanelField);
        OffsetRect (&AbsField, BorderLineWidth, BorderLineWidth);
        GetPanelColorAttribute (HelpTextPanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL, &Attribute);
        ClearField (Attribute, &AbsField, NULL);
      }
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DERefreshQuestion (
  IN H2O_FORM_BROWSER_PROTOCOL                *FBProtocol,
  IN BOOLEAN                                  HightLight,
  IN BOOLEAN                                  RefreshHelpStr,
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_S                          *Statement;
  H2O_PANEL_INFO                              *SetupPagePanel;
  H2O_PANEL_INFO                              *HelpTextPanel;
  H2O_CONTROL_INFO                            *Control;

  //
  // Refresh Text and TextTwo string in Setup Page panel
  //
  SetupPagePanel = GetPanelInfoByType (&mDEPrivate->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL) {
    return EFI_NOT_FOUND;
  }
  Control = GetControlInfo (
              SetupPagePanel->ControlList.ControlArray,
              SetupPagePanel->ControlList.Count,
              QuestionId,
              IfrOpCode
              );
  if (Control == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = FBProtocol->GetSInfo (FBProtocol, PageId, Control->StatementId, &Statement);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  SafeFreePool ((VOID **) &Control->Text.String);
  SafeFreePool ((VOID **) &Control->ValueStrInfo.String);
  Control->Text.String    = GetQuestionPromptStr (Statement);
  Control->ValueStrInfo.String = GetQuestionValueStr (Statement);
  CopyMem (&Control->HiiValue, &Statement->HiiValue, sizeof (EFI_HII_VALUE));

  if (HightLight) {
    DisplayHighLightControl (FALSE, Control);
  } else {
    DisplayNormalControls (1, Control);
  }

  if (!RefreshHelpStr) {
    SafeFreePool ((VOID **) &Statement);
    return EFI_SUCCESS;
  }

  //
  // Refresh Help string in Help panel
  //
  HelpTextPanel = GetPanelInfoByType (&mDEPrivate->Layout->PanelListHead, H2O_PANEL_TYPE_HELP_TEXT);
  if (HelpTextPanel == NULL || !IsVisibility (HelpTextPanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL)) {
    SafeFreePool ((VOID **) &Statement);
    return EFI_SUCCESS;
  }

  Control = GetControlInfo (
              HelpTextPanel->ControlList.ControlArray,
              HelpTextPanel->ControlList.Count,
              QuestionId,
              IfrOpCode
              );
  if (Control != NULL) {
    SafeFreePool ((VOID **) &Control->Text.String);
    Control->Text.String = AllocateCopyPool (StrSize (Statement->Help), Statement->Help);
    CopyMem (&Control->HiiValue, &Statement->HiiValue, sizeof (EFI_HII_VALUE));
    DisplayHighLightControl (FALSE, Control);
  }

  SafeFreePool ((VOID **) &Statement);
  return EFI_SUCCESS;
}

EFI_STATUS
DEOpenLayout (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA         *Private
  )
{
  EFI_STATUS                                 Status;
  H2O_FORM_BROWSER_SM                        *SetupMenuData;
  H2O_FORM_BROWSER_PROTOCOL                  *FBProtocol;


  Status = ProcessLayout ();
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = DisplayLayout ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FBProtocol = Private->FBProtocol;
  if (FBProtocol->CurrentP != NULL) {
    Status = FBProtocol->GetSMInfo (FBProtocol, &SetupMenuData);
    if (!EFI_ERROR (Status)) {
      DisplayTitle ((CONST CHAR16*)SetupMenuData->TitleString, (CONST CHAR16*)SetupMenuData->CoreVersionString);
      FreeSetupMenuData (SetupMenuData);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DEShutLayout (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA         *Private
  )
{
  LIST_ENTRY                                 *PanelLink;
  H2O_PANEL_INFO                             *Panel;

  PanelLink = &Private->Layout->PanelListHead;
  if (IsNull (PanelLink, PanelLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  do {
    PanelLink = PanelLink->ForwardLink;
    Panel = H2O_PANEL_INFO_NODE_FROM_LINK (PanelLink);
    FreePanel (Panel);
  } while (!IsNodeAtEnd (&Private->Layout->PanelListHead, PanelLink));

  Private->PageTagSelected = NULL;
  Private->MenuSelected    = NULL;
  Private->PopUpSelected   = NULL;

  return EFI_SUCCESS;
}

EFI_STATUS
DESelectPage (
  IN     H2O_PAGE_ID                          PageId
  )
{
  UINT32                                      Index;
  STATIC H2O_PAGE_ID                          OldPageId = 0;
  UINT32                                      SetupMenuCount;
  H2O_PANEL_INFO                              *SetupMenuPanel;
  LIST_ENTRY                                  *PanelLink;

  PanelLink = &mDEPrivate->Layout->PanelListHead;
  if (IsNull (PanelLink, PanelLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  SetupMenuPanel = GetPanelInfoByType (PanelLink, H2O_PANEL_TYPE_SETUP_MENU);
  if (SetupMenuPanel == NULL) {
    return EFI_SUCCESS;
  }
  SetupMenuCount = SetupMenuPanel->ControlList.Count;

  //
  // Set "PageTagSelected"
  //
  for (Index = 0; Index < SetupMenuCount; Index ++) {
    if ((SetupMenuPanel->ControlList.ControlArray[Index].PageId >> 16) == (PageId >> 16)) {
      //
      // Finding "PageTagSelected" is success
      //
      mDEPrivate->PageTagSelected = &SetupMenuPanel->ControlList.ControlArray[Index];
      //
      // Display new current page tag with high-light color
      //
      DisplayHighLightControl (FALSE, mDEPrivate->PageTagSelected);
    } else if ((SetupMenuPanel->ControlList.ControlArray[Index].PageId >> 16) == (OldPageId >> 16)) {
      //
      // Display old current page tag with original color
      //
      DisplayNormalControls (1, &SetupMenuPanel->ControlList.ControlArray[Index]);
    }
  }
  //
  // Update old page id
  //
  OldPageId = mDEPrivate->PageTagSelected->PageId;

  return EFI_SUCCESS;
}

EFI_STATUS
DEOpenPage (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA         *Private
  )
{
  EFI_STATUS                                 Status;
  H2O_FORM_BROWSER_PROTOCOL                  *FBProtocol;

  FBProtocol = Private->FBProtocol;

  //
  // Display Setup Menu
  //
  Status = InitSetupMenu ();
  if (!EFI_ERROR (Status)) {
    Status = DisplaySetupMenu (Private->FBProtocol);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = InitHotKeyBar (Private);
  if (!EFI_ERROR (Status)) {
    Status = DisplayHotkey ();
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Display Current Page Menu
  //
  Status = DERefresh (Private);

  return Status;
}

EFI_STATUS
DEShutPage (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA             *Private
  )
{
  LIST_ENTRY                                     *PanelLink;
  H2O_PANEL_INFO                                 *Panel;
  INT32                                          BorderLineWidth;

  PanelLink = &Private->Layout->PanelListHead;
  if (IsNull (PanelLink, PanelLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  do {
    PanelLink = PanelLink->ForwardLink;
    Panel = H2O_PANEL_INFO_NODE_FROM_LINK (PanelLink);
    switch (Panel->PanelType) {

    case H2O_PANEL_TYPE_HELP_TEXT:
    case H2O_PANEL_TYPE_QUESTION:
      FreePanel (Panel);
      break;

    case H2O_PANEL_TYPE_SETUP_PAGE:
      if (Private->FBProtocol->CurrentQ != NULL &&
          IsCurrentHighlight (Private, Private->FBProtocol->CurrentQ->PageId, Private->FBProtocol->CurrentQ->QuestionId, Private->FBProtocol->CurrentQ->IfrOpCode)) {
        break;
      }
      SetRect (
        &Panel->PanelRelField,
        0,
        0,
        Panel->PanelField.right  - Panel->PanelField.left,
        Panel->PanelField.bottom - Panel->PanelField.top
        );
      BorderLineWidth = GetBorderWidth (Panel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);
      if (BorderLineWidth > 0) {
        Panel->PanelRelField.bottom -= BorderLineWidth * 2;
        Panel->PanelRelField.right  -= BorderLineWidth * 2;
      }
      break;
    }
  } while (!IsNodeAtEnd (&Private->Layout->PanelListHead, PanelLink));

  return EFI_SUCCESS;
}

EFI_STATUS
DESelectQuestion (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private,
  IN H2O_DISPLAY_ENGINE_EVT_SELECT_Q          *SelectQ
  )
{
  H2O_CONTROL_INFO                            *OldCurrentQ;
  H2O_PANEL_INFO                              *SetupPagePanel;

  SetupPagePanel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_PAGE);
  if (SetupPagePanel == NULL || SetupPagePanel->ControlList.Count == 0) {
    return EFI_SUCCESS;
  }

  //
  // Display old current question control with original color
  //
  OldCurrentQ = Private->MenuSelected;
  if (OldCurrentQ != NULL &&
      OldCurrentQ->PageId == SelectQ->PageId &&
      !((OldCurrentQ->QuestionId != 0    && OldCurrentQ->QuestionId == SelectQ->QuestionId) ||
        (OldCurrentQ->IfrOpCode  != NULL && OldCurrentQ->IfrOpCode  == SelectQ->IfrOpCode))) {
    DERefreshQuestion (Private->FBProtocol, FALSE, FALSE, OldCurrentQ->PageId, OldCurrentQ->QuestionId, OldCurrentQ->IfrOpCode);
  }

  Private->MenuSelected = GetControlInfo (
                            SetupPagePanel->ControlList.ControlArray,
                            SetupPagePanel->ControlList.Count,
                            SelectQ->QuestionId,
                            SelectQ->IfrOpCode
                            );

  Private->DEStatus = DISPLAY_ENGINE_STATUS_SELECT_Q;
  ProcessEvtInQuestionFunc (Private, (H2O_DISPLAY_ENGINE_EVT *) &SelectQ, NULL);
  Private->DEStatus = DISPLAY_ENGINE_STATUS_AT_MENU;

#if 1
  //
  // Ensure selected question is on this page now
  //
  if (EnsureControlInPanel (Private->MenuSelected) == EFI_SUCCESS) {
    DERefresh (Private);
  }
#endif

  DERefreshQuestion (Private->FBProtocol, TRUE, TRUE, SelectQ->PageId, SelectQ->QuestionId, SelectQ->IfrOpCode);

  return EFI_SUCCESS;
}

EFI_STATUS
DEShutQuestion (
  IN     H2O_FORM_BROWSER_PROTOCOL           *FBProtocol,
  IN CONST H2O_DISPLAY_ENGINE_EVT            *Notify
  )
{
  EFI_STATUS                                 Status;

  mDEPrivate->DEStatus = DISPLAY_ENGINE_STATUS_SHUT_Q;
  Status = ProcessEvtInQuestionFunc (mDEPrivate, (H2O_DISPLAY_ENGINE_EVT *) Notify, NULL);
  mDEPrivate->DEStatus = DISPLAY_ENGINE_STATUS_AT_MENU;

  return Status;
}

EFI_STATUS
DEChangingQuestion (
  IN     H2O_FORM_BROWSER_PROTOCOL           *FBProtocol,
  IN CONST H2O_DISPLAY_ENGINE_EVT            *Notify
  )
{
  EFI_STATUS                                 Status;

  mDEPrivate->DEStatus = DISPLAY_ENGINE_STATUS_CHANGING_Q;
  Status = ProcessEvtInQuestionFunc (mDEPrivate, Notify, NULL);
  mDEPrivate->DEStatus = DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG;
  if (Status == EFI_NOT_FOUND) {
    Status = EFI_SUCCESS;
  }

  return Status;
}

EFI_STATUS
DEOpenDialog (
  IN     H2O_FORM_BROWSER_PROTOCOL           *FBProtocol,
  IN CONST H2O_DISPLAY_ENGINE_EVT            *Notify
  )
{
  EFI_STATUS                                 Status;

  if (mDEPrivate->Layout == NULL) {
    Status = ProcessLayout ();
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  mDEPrivate->DEStatus = DISPLAY_ENGINE_STATUS_OPEN_D;
  Status = ProcessEvtInQuestionFunc (mDEPrivate, Notify, NULL);
  mDEPrivate->DEStatus = (!EFI_ERROR (Status)) ? DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG : DISPLAY_ENGINE_STATUS_AT_MENU;
  if (Status == EFI_NOT_FOUND) {
    Status = EFI_SUCCESS;
  }

  return Status;
}

EFI_STATUS
DEShutDialog (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA   *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT            *Notify
  )
{
  EFI_STATUS                                 Status;

  Private->DEStatus = DISPLAY_ENGINE_STATUS_SHUT_D;
  Status = ProcessEvtInQuestionFunc (Private, Notify, NULL);
  Private->DEStatus = DISPLAY_ENGINE_STATUS_AT_MENU;
  if (Status == EFI_NOT_FOUND) {
    Status = EFI_SUCCESS;
  }

  if (Private->FBProtocol->CurrentP != NULL) {
    DisplayLayout ();
    DEOpenPage (Private);
  }
  DEConOutSetAttribute (Private, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);

  return Status;
}

EFI_STATUS
DEExit (
  VOID
  )
{
  LIST_ENTRY                                     *PanelLink;
  H2O_PANEL_INFO                                 *Panel;

  PanelLink = &mDEPrivate->Layout->PanelListHead;
  if (IsNull (PanelLink, PanelLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  //
  // Clear layout info
  //
  do {
    PanelLink = PanelLink->ForwardLink;
    Panel = H2O_PANEL_INFO_NODE_FROM_LINK (PanelLink);
    FreePanel (Panel);
  } while (!IsNodeAtEnd (&mDEPrivate->Layout->PanelListHead, PanelLink));

  mDEPrivate->PageTagSelected = NULL;
  mDEPrivate->MenuSelected = NULL;
  mDEPrivate->PopUpSelected = NULL;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ProcessInputEvtAtMenu (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  EFI_STATUS                                  Status;
  H2O_CONTROL_INFO                            *SelectedControl;
  H2O_PANEL_INFO                              *Panel;
  H2O_EVT_TYPE                                HotKeyEvtType;
  BOOLEAN                                     IsIncreaseHotKey;
  BOOLEAN                                     Found;
  UINT32                                      Count;
  H2O_CONTROL_INFO                            *ControlArray;

  Status = ProcessEvtInQuestionFunc (Private, Notify, UserInputData);
  if (!EFI_ERROR (Status)) {
    return Status;
  }

  Status = CheckFBHotKey (Private, UserInputData, &HotKeyEvtType, &IsIncreaseHotKey);
  if (!EFI_ERROR (Status)) {
    return SendEvtByHotKey (Private, HotKeyEvtType, IsIncreaseHotKey);
  }

  Status = CheckDEHotKey (Private, UserInputData, &HotKeyEvtType, &IsIncreaseHotKey);
  if (!EFI_ERROR (Status)) {
    return SendEvtByHotKey (Private, HotKeyEvtType, IsIncreaseHotKey);
  }

  Status = EFI_UNSUPPORTED;

  if (UserInputData->IsKeyboard) {
    return Status;
  }

  //
  // Check if mouse click on Setup Page panel
  //
  Panel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_PAGE);
  if (Panel == NULL) {
    return Status;
  }
  SelectedControl = Private->MenuSelected;
  ControlArray    = Panel->ControlList.ControlArray;
  Count           = Panel->ControlList.Count;
  Found = CheckPressControls (FALSE, &UserInputData->KeyData.Key, UserInputData->CursorX, UserInputData->CursorY, Count, ControlArray, &SelectedControl);
  if (Found) {
    if (SelectedControl == NULL ||
        !SelectedControl->Selectable ||
        SelectedControl->PageId != Private->MenuSelected->PageId ||
        UserInputData->KeyData.Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
      return Status;
    }

    if (SelectedControl->QuestionId == Private->MenuSelected->QuestionId &&
        SelectedControl->IfrOpCode  == Private->MenuSelected->IfrOpCode) {
      return SendOpenQNotify (
               Private->FBProtocol->CurrentQ->PageId,
               Private->FBProtocol->CurrentQ->QuestionId,
               Private->FBProtocol->CurrentQ->IfrOpCode
               );
    } else {
      return SendSelectQNotify (
               SelectedControl->PageId,
               SelectedControl->QuestionId,
               SelectedControl->IfrOpCode
               );
    }
  }

  //
  // Check if mouse click on Setup Menu panel
  //
  if (IsRootPage ()) {
    Panel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_MENU);
    if (Panel == NULL) {
      return Status;
    }

    SelectedControl = Private->PageTagSelected;
    ControlArray    = Panel->ControlList.ControlArray;
    Count           = Panel->ControlList.Count;
    Found = CheckPressControls (FALSE, &UserInputData->KeyData.Key, UserInputData->CursorX, UserInputData->CursorY, Count, ControlArray, &SelectedControl);
    if (Found) {
      if (SelectedControl == NULL ||
          !SelectedControl->Selectable ||
          SelectedControl->PageId == Private->PageTagSelected->PageId ||
          UserInputData->KeyData.Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
        return EFI_UNSUPPORTED;
      }

      Private->PageTagSelected = SelectedControl;
      return SendSelectPNotify (SelectedControl->PageId);
    }
  }

  return Status;
}

STATIC
EFI_STATUS
ProcessInputEvtAtPopUpDialog (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  EFI_STATUS                                  Status;
  H2O_CONTROL_INFO                            *MenuSelected;
  H2O_CONTROL_LIST                            *PopUpControls;
  LIST_ENTRY                                  *PanelLink;
  H2O_PANEL_INFO                              *Panel;

  PanelLink = &Private->Layout->PanelListHead;
  if (IsNull (PanelLink, PanelLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  //
  // Specific Question At Pop-Up Dialog Behavior
  //
  Status = ProcessEvtInQuestionFunc (Private, Notify, UserInputData);
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Panel = GetPanelInfoByType (PanelLink, H2O_PANEL_TYPE_QUESTION);
  if (Panel == NULL) {
    return EFI_SUCCESS;
  }
  PopUpControls = &Panel->ControlList;
  //
  // Standard At Pop-Up Dialog Behavior
  //
  MenuSelected = Private->PopUpSelected;
  if (CheckPressControls (UserInputData->IsKeyboard, &UserInputData->KeyData.Key, UserInputData->CursorX, UserInputData->CursorY, PopUpControls->Count, PopUpControls->ControlArray, &MenuSelected)) {
    if (MenuSelected == NULL) {
      SendShutDNotify ();
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ProcessInputEvt (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify
  )
{
  EFI_STATUS                                  Status;
  H2O_DISPLAY_ENGINE_USER_INPUT_DATA          UserInputData;
  H2O_DISPLAY_ENGINE_EVT_REL_PTR_MOVE         *RelPtrMoveNotify;
  H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE         *AbsPtrMoveNotify;
  STATIC UINT32                               PreviousActiveButtons = 0;
  STATIC INT32                                RealMouseX = 0;
  STATIC INT32                                RealMouseY = 0;

  //
  // Read user input from input event
  //
  ZeroMem (&UserInputData, sizeof (H2O_DISPLAY_ENGINE_USER_INPUT_DATA));

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
    //
    // Keyboard
    //
    UserInputData.IsKeyboard = TRUE;
    CopyMem (&UserInputData.KeyData, &((H2O_DISPLAY_ENGINE_EVT_KEYPRESS *) Notify)->KeyData, sizeof (EFI_KEY_DATA));
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
    RelPtrMoveNotify = (H2O_DISPLAY_ENGINE_EVT_REL_PTR_MOVE *) Notify;
    //
    // Rel Mouse
    //
    if (RelPtrMoveNotify->State.LeftButton) {
      UserInputData.KeyData.Key.ScanCode    = SCAN_NULL;
      UserInputData.KeyData.Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
    }
    if (RelPtrMoveNotify->State.RightButton) {
      UserInputData.KeyData.Key.ScanCode    = SCAN_ESC;
      UserInputData.KeyData.Key.UnicodeChar = CHAR_NULL;
    }
    RealMouseX += RelPtrMoveNotify->State.RelativeMovementX;
    RealMouseY += RelPtrMoveNotify->State.RelativeMovementY;
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    AbsPtrMoveNotify = (H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE *) Notify;
    //
    // Abs Mouse
    //
    if ((AbsPtrMoveNotify->AbsPtrState.ActiveButtons & EFI_ABSP_TouchActive) == EFI_ABSP_TouchActive &&
        (AbsPtrMoveNotify->AbsPtrState.ActiveButtons & EFI_ABSP_TouchActive) != (PreviousActiveButtons & EFI_ABSP_TouchActive)) {
      //
      // Left Button
      //
      UserInputData.KeyData.Key.ScanCode    = SCAN_NULL;
      UserInputData.KeyData.Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
    }
    if ((AbsPtrMoveNotify->AbsPtrState.ActiveButtons & EFI_ABS_AltActive) == EFI_ABS_AltActive &&
        (AbsPtrMoveNotify->AbsPtrState.ActiveButtons & EFI_ABS_AltActive) != (PreviousActiveButtons & EFI_ABS_AltActive)) {
      //
      // Right Button
      //
      UserInputData.KeyData.Key.ScanCode    = SCAN_ESC;
      UserInputData.KeyData.Key.UnicodeChar = CHAR_NULL;
    }
    PreviousActiveButtons = AbsPtrMoveNotify->AbsPtrState.ActiveButtons;
    RealMouseX = (INT32)AbsPtrMoveNotify->AbsPtrState.CurrentX;
    RealMouseY = (INT32)AbsPtrMoveNotify->AbsPtrState.CurrentY;
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  UserInputData.CursorX = RealMouseX;
  UserInputData.CursorY = RealMouseY;

  //
  // Process user input
  //
  Status = EFI_UNSUPPORTED;

  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_MENU:
    Status = ProcessInputEvtAtMenu (Private, Notify, &UserInputData);
    break;

  case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
    Status = ProcessInputEvtAtPopUpDialog (Private, Notify, &UserInputData);
    break;

  default:
    break;
  }

  return Status;
}

EFI_STATUS
DEEventCallback (
  IN       H2O_DISPLAY_ENGINE_PROTOCOL        *This,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify
  )
{
  H2O_DISPLAY_ENGINE_PRIVATE_DATA             *Private;
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_REFRESH_Q            *RefreshQNotify;
  H2O_DISPLAY_ENGINE_EVT_SELECT_P             *SelectPNotify;

  Private = H2O_DISPLAY_ENGINE_PRIVATE_DATA_FROM_PROTOCOL (This);

  if (IsNull (&Private->ConsoleDevListHead, Private->ConsoleDevListHead.ForwardLink)) {
    //
    // Do nothing
    //
    return EFI_SUCCESS;
  }

  FBProtocol = Private->FBProtocol;
  Status     = EFI_UNSUPPORTED;

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE:
  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    Status = ProcessInputEvt (Private, Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_L:
    Status = DEOpenLayout (Private);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_L:
    Status = DEShutLayout (Private);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_P:
    Status = DEShutPage (Private);
    Status = DEOpenPage (Private);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_P:
    Status = DEShutPage (Private);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
    Status = DEOpenDialog (FBProtocol, Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
    Status = DEShutDialog (Private, Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH:
    Status = DERefresh (Private);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH_Q:
    RefreshQNotify = (H2O_DISPLAY_ENGINE_EVT_REFRESH_Q *) Notify;
    if (IsDialogStatus (Private->DEStatus) ||
        (IsCurrentHighlight (Private, RefreshQNotify->PageId, RefreshQNotify->QuestionId, RefreshQNotify->IfrOpCode) &&
         IsQuestionBeingModified (Private->MenuSelected))) {
      break;
    }
    DERefreshQuestion (
      FBProtocol,
      IsCurrentHighlight (Private, RefreshQNotify->PageId, RefreshQNotify->QuestionId, RefreshQNotify->IfrOpCode),
      FALSE,
      RefreshQNotify->PageId,
      RefreshQNotify->QuestionId,
      RefreshQNotify->IfrOpCode
      );
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_Q:
    Status = DESelectQuestion (Private, (H2O_DISPLAY_ENGINE_EVT_SELECT_Q *) Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_Q:
    Status = DEShutQuestion (FBProtocol, Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_P:
    SelectPNotify = (H2O_DISPLAY_ENGINE_EVT_SELECT_P *) Notify;
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q:
    Status = DEChangingQuestion (FBProtocol, Notify);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_EXIT:
    Status = DEExit ();
    break;

  default:
    ASSERT (FALSE);
    break;
  }

  return Status;
}
