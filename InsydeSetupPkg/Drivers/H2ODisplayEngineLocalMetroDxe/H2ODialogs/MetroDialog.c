/** @file
  Entry point and initial functions for H2O local Metro display engine driver

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
#include "H2ODisplayEngineLocalMetro.h"
#include "UiControls.h"
#include "MetroDialog.h"
#include "MetroUi.h"
#include <Library/ConsoleLib.h>

extern H2O_DISPLAY_ENGINE_METRO_PRIVATE_DATA      *mMetroPrivate;
extern HWND                                       gWnd;
extern EFI_ABSOLUTE_POINTER_STATE                 mPreviousAbsPtrState;

HWND                            mDialogWnd = NULL;
HWND                            mOverlayWnd = NULL;
H2O_FORM_BROWSER_D              *mFbDialog;
BOOLEAN                         mTitleVisible;
BOOLEAN                         mIsSendForm;

VOID
H2OCommonDialogWithHelpOnSetState (
  UI_CONTROL                    *Control,
  UI_STATE                      SetState,
  UI_STATE                      ClearState
  )
{
  UI_CONTROL                    *Child;


  if (SetState & UISTATE_FOCUSED) {
    Child = UiFindChildByName (Control, L"DialogText");
    UiSetAttribute (Child, L"textcolor", L"0xFFFFFFFF");
  }
  if (ClearState & UISTATE_FOCUSED) {
    Child = UiFindChildByName (Control, L"DialogText");
    UiSetAttribute (Child, L"textcolor", L"0xFFA8D2DF");
  }
}

CHAR16 *mCommonDialogChilds = L""
  L"<VerticalLayout padding='40,30,40,30' bkcolor='@menucolor' name='PopUpDialog'>"
    L"<VerticalLayout name='TitleLayout'>"
      L"<Label padding='0,0,30,0' textcolor='0xFFFFFFFF' fontsize='29' name='DialogTitle' height='110'/>"
      L"<Label padding='0,0,45,0' textcolor='0xFFA8D2DF' fontsize='19' name='DialogText'/>"
    L"</VerticalLayout>"
    L"<Control bkimage='@DialogSeparator' bkcolor='0x0' name='DialogSeparator' height='10'/>"
    L"<VerticalLayout>"
      L"<HorizontalLayout padding='10,0,0,0' visible='false' name='DialogPasswordInput' height='51'>"
        L"<Label name='ConfirmNewPasswordLabel' height='41' width='119' fontsize='16' bkcolor='0xFFF2F2F2' textcolor='0xFF4D4D4D'/>"
        L"<VerticalLayout padding='6,8,6,8' bkcolor='0xFFF2F2F2' height='41'>"
          L"<Control padding='1,1,1,1' bkcolor='0xFF999999' height='29'>"
            L"<UiEdit name='ConfirmPasswordInput' focusbkcolor='@menulightcolor' height='27' padding='7,3,0,3' bkcolor='0xFFF2F2F2' password='true'/>"
          L"</Control>"
        L"</VerticalLayout>"
      L"</HorizontalLayout>"
      L"<HorizontalLayout padding='10,0,0,0' childpadding='2' name='DialogButtonList'/>"
    L"</VerticalLayout>"
  L"</VerticalLayout>";

CHAR16 *mCommonDialogWithHelpChilds = L""
  L"<VerticalLayout padding='40,30,40,30' bkcolor='@menucolor' name='Dialog'>"
    L"<VerticalLayout name='TitleLayout'>"
      L"<HorizontalLayout padding='0,0,30,0' height='80'>"
        L"<Label width='200' textcolor='0xFFFFFFFF' fontsize='29' name='DialogTitle'/>"
        L"<Control padding='0,25,0,25'>"
          L"<Texture name='DialogImage' bkimagestyle='stretch|light'/>"
        L"</Control>"
      L"</HorizontalLayout>"
      L"<ScrollView taborder='20' name='DialogTextScrollView' vscrollbar='true' scale9grid='1,13,1,13'>"
        L"<Label name='DialogText' height='wrap_content' padding='0,0,0,0' width='match_parent' fontsize='19' textcolor='0xFFA8D2DF'/>"
      L"</ScrollView>"
    L"</VerticalLayout>"
    L"<Control bkimage='@DialogSeparator' bkcolor='0x0' height='2'/>"
    L"<Control height='5'/>"
    L"<Control name='Content'/>"
  L"</VerticalLayout>";

CHAR16 *mDialogWithoutSendFormChilds = L""
  L"<VerticalLayout name='DialogWithoutSendForm'>"
    L"<Control/>"
    L"<Control bkcolor='@menucolor' name='parent' height='wrap_content'>"
      L"<VerticalLayout padding='20,30,20,30' bkcolor='@menucolor' height='wrap_content'>"
        L"<VerticalLayout name='TitleLayout' height='wrap_content'>"
          L"<Label name='DialogTitle' align='center' height='40' visible='false' fontsize='19' textcolor='0xFFFFFFFF'/>"
          L"<Label width='match_parent' textcolor='0xFFFFFFFF' fontsize='19' name='DialogText' height='wrap_content'/>"
          L"<Control height='15'/>"
          L"<Control height='10' bkimage='@DialogSeparator' bkcolor='0x0' bkimagestyle='center'/>"
          L"<Control height='15'/>"
        L"</VerticalLayout>"
        L"<VerticalLayout padding='0,0,10,0' visible='false' minheight='51' name='DialogPasswordInput' height='wrap_content'>"
          L"<Label name='ConfirmNewPasswordLabel' height='wrap_content' padding='0,0,2,0' width='match_parent' fontsize='18' textcolor='0xFFFFFFFF'/>"
          L"<Control padding='2,2,2,2' minheight='32' bkcolor='0xFFCCCCCC' height='wrap_content'>"
            L"<UiEdit name='ConfirmPasswordInput' focusbkcolor='0xFFFFFFFF' tabstop='true' height='27' padding='7,3,0,3' taborder='1' bkcolor='@menulightcolor' password='true'/>"
          L"</Control>"
        L"</VerticalLayout>"
        L"<HorizontalLayout padding='1,0,0,0' childpadding='2' visible='false' name='DialogButtonList' height='32'/>"
      L"</VerticalLayout>"
      L"<Texture name='FormHalo' float='true' height='-1' width='-1' bkimage='@FormHalo' scale9grid='23,26,22,31'/>"
    L"</Control>"
    L"<Control/>"
  L"</VerticalLayout>";


VOID
FreeDialogEvent (
  IN H2O_FORM_BROWSER_D         **DialogEvt
  )
{
  H2O_FORM_BROWSER_D                       *Event;

  if (DialogEvt == NULL || *DialogEvt == NULL) {
    return;
  }

  Event = *DialogEvt;

  if (Event->TitleString != NULL) {
    FreePool (Event->TitleString);
  }

  if (Event->BodyStringArray != NULL && Event->BodyStringCount != 0) {
    FreeStringArray (Event->BodyStringArray, Event->BodyStringCount);
  }

  if (Event->BodyInputStringArray != NULL && Event->BodyInputCount != 0) {
    FreeStringArray (Event->BodyInputStringArray, Event->BodyInputCount);
  }

  if (Event->ButtonStringArray != NULL && Event->ButtonCount != 0) {
    FreeStringArray (Event->ButtonStringArray, Event->ButtonCount);
  }

  if (Event->BodyHiiValueArray != NULL && Event->BodyStringCount != 0) {
    FreeHiiValueArray (Event->BodyHiiValueArray, Event->BodyStringCount);
  }

  if (Event->ButtonHiiValueArray != NULL && Event->ButtonCount != 0) {
    FreeHiiValueArray (Event->ButtonHiiValueArray, Event->ButtonCount);
  }

  if (Event->ConfirmHiiValue.Type == EFI_IFR_TYPE_BUFFER && Event->ConfirmHiiValue.Buffer != NULL && Event->ConfirmHiiValue.BufferLen != 0) {
    FreePool (Event->ConfirmHiiValue.Buffer);
  }

  FreePool (Event);

  *DialogEvt = NULL;
}

H2O_FORM_BROWSER_D *
CopyDialogEvent (
  IN H2O_FORM_BROWSER_D                    *SrcDialogEvt
  )
{
  H2O_FORM_BROWSER_D                       *DialogEvt;

  if (SrcDialogEvt == NULL) {
    return NULL;
  }

  DialogEvt = AllocateCopyPool (sizeof (H2O_FORM_BROWSER_D), SrcDialogEvt);
  if (DialogEvt == NULL) {
    return NULL;
  }

  if (SrcDialogEvt->TitleString != NULL) {
    DialogEvt->TitleString = AllocateCopyPool (StrSize (SrcDialogEvt->TitleString), SrcDialogEvt->TitleString);
    if (DialogEvt->TitleString == NULL) {
      goto Error;
    }
  }

  if (SrcDialogEvt->BodyStringArray != NULL && SrcDialogEvt->BodyStringCount != 0) {
    DialogEvt->BodyStringArray = CopyStringArray (SrcDialogEvt->BodyStringArray, SrcDialogEvt->BodyStringCount);
    if (DialogEvt->BodyStringArray == NULL) {
      goto Error;
    }
  }

  if (SrcDialogEvt->BodyInputStringArray != NULL && SrcDialogEvt->BodyInputCount != 0) {
    DialogEvt->BodyInputStringArray = CopyStringArray (SrcDialogEvt->BodyInputStringArray, SrcDialogEvt->BodyInputCount);
    if (DialogEvt->BodyInputStringArray == NULL) {
      goto Error;
    }
  }

  if (SrcDialogEvt->ButtonStringArray != NULL && SrcDialogEvt->ButtonCount != 0) {
    DialogEvt->ButtonStringArray = CopyStringArray (SrcDialogEvt->ButtonStringArray, SrcDialogEvt->ButtonCount);
    if (DialogEvt->ButtonStringArray == NULL) {
      goto Error;
    }
  }

  if (SrcDialogEvt->BodyHiiValueArray != NULL && SrcDialogEvt->BodyStringCount != 0) {
    DialogEvt->BodyHiiValueArray = CopyHiiValueArray (SrcDialogEvt->BodyHiiValueArray, SrcDialogEvt->BodyStringCount);
    if (DialogEvt->BodyHiiValueArray == NULL) {
      goto Error;
    }
  }

  if (SrcDialogEvt->ButtonHiiValueArray != NULL && SrcDialogEvt->ButtonCount != 0) {
    DialogEvt->ButtonHiiValueArray = CopyHiiValueArray (SrcDialogEvt->ButtonHiiValueArray, SrcDialogEvt->ButtonCount);
    if (DialogEvt->ButtonHiiValueArray == NULL) {
      goto Error;
    }
  }

  if (SrcDialogEvt->ConfirmHiiValue.Type == EFI_IFR_TYPE_BUFFER && SrcDialogEvt->ConfirmHiiValue.Buffer != NULL && SrcDialogEvt->ConfirmHiiValue.BufferLen != 0) {
    DialogEvt->ConfirmHiiValue.Buffer = AllocateCopyPool (SrcDialogEvt->ConfirmHiiValue.BufferLen, SrcDialogEvt->ConfirmHiiValue.Buffer);
    if (DialogEvt->ConfirmHiiValue.Buffer == NULL) {
      goto Error;
    }
  }

  return DialogEvt;

Error:
  FreeDialogEvent (&DialogEvt);
  return NULL;
}

HWND
CreateModalDialog (
  HINSTANCE                     Instance,
  CHAR16                        *XmlBuffer,
  HWND                          ParentWnd,
  WNDPROC                       DialogProc,
  LPARAM                        Param,
  INT32                         X,
  INT32                         Y,
  INT32                         Width,
  INT32                         Height,
  BOOLEAN                       CloseDlgWhenTouchOutside
  )
{
  UI_DIALOG                     *DialogData;
  HWND                          Dlg;
  UI_CONTROL                    *Control;
  UI_MANAGER                    *Manager;

  DialogData = AllocateZeroPool (sizeof (UI_DIALOG));
  if (DialogData == NULL) {
    return NULL;
  }
  DialogData->Instance          = Instance;
  DialogData->ParentWnd         = ParentWnd;
  DialogData->Proc              = DialogProc;
  DialogData->Param             = Param;
  DialogData->XmlBuffer         = XmlBuffer;
  DialogData->Running           = TRUE;
  DialogData->CloseDlgWhenTouchOutside = CloseDlgWhenTouchOutside;

  Dlg = CreateWindowEx (
          WS_EX_NOACTIVATE, L"DIALOG", L"", WS_VISIBLE | WS_POPUP,
          X, Y, Width, Height, ParentWnd, NULL, Instance, DialogData
          );
  ASSERT (Dlg != NULL);
  if (Dlg == NULL) {
    return NULL;
  }

  Control = (UI_CONTROL *) (UINTN) GetWindowLongPtr (Dlg, 0);
  Manager = Control->Manager;

  SendMessage (Manager->MainWnd, UI_NOTIFY_WINDOWINIT, (WPARAM)Manager->Root, 0);

  return Dlg;
}

BOOLEAN
ClickOnOutsideOfDlgRegion (
  IN EFI_ABSOLUTE_POINTER_STATE               *AbsPtrState
  )
{
  RECT                                        DlgRect;
  POINT                                       Pt;

  ASSERT (mDialogWnd != NULL);
  ASSERT (AbsPtrState != NULL);

  if (GetCapture () != NULL) {
    return FALSE;
  }

  if (AbsPtrState->ActiveButtons == 0) {
    return FALSE;
  }

  Pt.x = (INT32) AbsPtrState->CurrentX;
  Pt.y = (INT32) AbsPtrState->CurrentY;

  GetWindowRect (mDialogWnd, &DlgRect);
  if (PtInRect (&DlgRect, Pt)) {
    return FALSE;
  }

  return TRUE;
}

UINT32
GetButtonWidthByStr (
  VOID
  )
{
  UINT32                                   Index;
  UINT32                                   ButtonWidth;

  ButtonWidth = 0;
  for (Index = 0; Index < mFbDialog->ButtonCount; Index++) {
    if (mFbDialog->ButtonStringArray[Index] != NULL) {
      ButtonWidth = (UINT32) MAX (GetStringWidth (mFbDialog->ButtonStringArray[Index]), ButtonWidth);
    }
  }

  return (ButtonWidth * 5 + 4);
}

VOID
EnableTitle (
  HWND          Wnd,
  BOOLEAN       Enable
  )
{
  CHAR16           *Str;
  UI_DIALOG        *Dialog;
  UI_CONTROL       *Control;
  HWND             FocusedWnd;
  STATIC UINTN     CheckTimes = 0;

  //
  // Prevent from button location change to cause invalid button press, NOT do
  // enable/disable password during this period.
  //
  FocusedWnd = GetFocus ();
  Control  = (UI_CONTROL *) GetWindowLongPtr (FocusedWnd, 0);
  if (Control != NULL && (StrCmp (Control->Name, L"Ok") == 0 ||
      StrCmp (Control->Name, L"Cancel") == 0) && CheckTimes <= 10) {
    CheckTimes++;
    return;
  }
  CheckTimes = 0;
  Dialog  = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);
  Str = Enable ? L"true" : L"false";

  Control = UiFindChildByName (Dialog, L"TitleLayout");
  UiSetAttribute (Control, L"visible", Str);
  mTitleVisible = Enable;
  CONTROL_CLASS_INVALIDATE (Dialog);
}


LRESULT
WINAPI
DialogCallback (
  HWND Wnd,
  UINT Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  KEYBOARD_ATTRIBUTES  KeyboardAttributes;

  mSetupMouse->GetKeyboardAttributes (mSetupMouse, &KeyboardAttributes);
  if (KeyboardAttributes.IsStart && mTitleVisible) {
    EnableTitle  (Wnd, FALSE);
  } else if (!KeyboardAttributes.IsStart && !mTitleVisible) {
    EnableTitle  (Wnd, TRUE);
  }
  return 0;
}


EFI_STATUS
GetDialogRegion (
  IN  CHAR16                               *PanelName,
  OUT RECT                                 *DlgRect
  )
{
  EFI_STATUS                               Status;
  RECT                                     PanelRect;
  UINT32                                   PanelWidth;
  UINT32                                   PanelHeight;
  UINT32                                   DlgWidth;
  UINT32                                   DlgHeight;

  ASSERT (PanelName != NULL);
  ASSERT (DlgRect != NULL);

  Status = GetRectByName (gWnd, PanelName, &PanelRect);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  PanelWidth  = PanelRect.right  - PanelRect.left;
  PanelHeight = PanelRect.bottom - PanelRect.top;

  if ((mFbDialog->DialogType & H2O_FORM_BROWSER_D_TYPE_QUESTIONS) == 0) {
    if ((mFbDialog->DialogType & H2O_FORM_BROWSER_D_TYPE_SELECTION) == H2O_FORM_BROWSER_D_TYPE_SELECTION) {
      DlgWidth  = 360;
      DlgHeight = PanelHeight;
    } else if ((mFbDialog->DialogType & H2O_FORM_BROWSER_D_TYPE_MSG) == H2O_FORM_BROWSER_D_TYPE_MSG) {
      DlgWidth  = 360;
      DlgHeight = PanelHeight;
    } else {
      return EFI_UNSUPPORTED;
    }
  } else {
    switch (GetOpCodeByDialogType (mFbDialog->DialogType)) {

    case EFI_IFR_ONE_OF_OP:
      DlgWidth  = PanelWidth;
      DlgHeight = PanelHeight;
      break;

    case EFI_IFR_NUMERIC_OP:
      DlgWidth  = 360;
      DlgHeight = 200;
      break;

    case EFI_IFR_PASSWORD_OP:
      DlgWidth  = 400;
      DlgHeight = PanelHeight;
      break;

    default:
      return EFI_UNSUPPORTED;
      break;
    }
  }

  DlgRect->left   = PanelRect.left + ((PanelWidth - DlgWidth) / 2);
  DlgRect->top    = PanelRect.top  + ((PanelHeight - DlgHeight) / 2);
  DlgRect->right  = DlgRect->left + DlgWidth;
  DlgRect->bottom = DlgRect->top + DlgHeight;

  return EFI_SUCCESS;
}

EFI_STATUS
SetDialogText (
  IN UI_DIALOG                             *Dialog
  )
{
  EFI_STATUS                               Status;
  UINTN                                    Index;
  UI_CONTROL                               *Control;
  UI_CONTROL                               *ListControl;
  UI_CONTROL                               *FocusControl;
  CHAR16                                   *BodyString;
  INTN                                     Result;

  if ((mFbDialog->DialogType & H2O_FORM_BROWSER_D_TYPE_QUESTIONS) != 0) {
    return EFI_UNSUPPORTED;
  }

  FocusControl = NULL;

  if ((mFbDialog->DialogType & H2O_FORM_BROWSER_D_TYPE_SELECTION) == H2O_FORM_BROWSER_D_TYPE_SELECTION) {
    if (mFbDialog->BodyStringArray != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogText");
      BodyString = CatStringArray (mFbDialog->BodyStringCount, (CONST CHAR16 **) mFbDialog->BodyStringArray);
      if (BodyString != NULL) {
        UiSetAttribute (Control, L"text", BodyString);
        if (GetSystemMetrics(SM_CYSCREEN) < 600) {
          UiSetAttribute (Control, L"fontsize", L"16");
        }
        FreePool (BodyString);
      }
    }

    ListControl = UiFindChildByName (Dialog, L"DialogButtonList");
    UiSetAttribute (ListControl, L"visible", L"true");

    Control = CreateControl (L"Control", ListControl);
    CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);

    for (Index = 0; Index < mFbDialog->ButtonCount; Index++) {
      if (mFbDialog->ButtonStringArray[Index] == NULL) {
        continue;
      }

      Control = CreateControl (L"Button", ListControl);
      UiSetAttribute (Control, L"text",  mFbDialog->ButtonStringArray[Index]);
      SetWindowLongPtr (Control->Wnd, GWLP_USERDATA, (INTN) Index);
      UiApplyAttributeList (Control, L"name='Button' height='30' width='75' fontsize='19' textcolor='0xFFFFFFFF' align='center' align='singleline' bkcolor='0xFFCCCCCC' focusbkcolor='@menulightcolor'");
      CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);

      Status = CompareHiiValue (&mFbDialog->ButtonHiiValueArray[Index], &mFbDialog->ConfirmHiiValue, &Result);
      if (!EFI_ERROR(Status) && Result == 0) {
        FocusControl = Control;
      }
    }

    Control = CreateControl (L"Control", ListControl);
    CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);

    if (FocusControl != NULL) {
      SetFocus (FocusControl->Wnd);
    }
  }

  return EFI_SUCCESS;
}


LRESULT
DialogWithoutSendFormProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_DIALOG                     *Dialog;
  UINTN                         Index;
  UI_CONTROL                    *Control;

  Dialog  = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);

  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    SetDialogText (Dialog);
    break;

  case UI_NOTIFY_CLICK:
    Control = (UI_CONTROL *)WParam;

    if ((mFbDialog->DialogType & H2O_FORM_BROWSER_D_TYPE_SELECTION) == H2O_FORM_BROWSER_D_TYPE_SELECTION) {
      Index = (UINTN) GetWindowLongPtr (Control->Wnd, GWLP_USERDATA);
      SendChangeQNotify (0, 0, &mFbDialog->ButtonHiiValueArray[Index]);
    }
    break;

  case WM_DESTROY:
    FreeDialogEvent (&mFbDialog);
    return 0;

  default:
    return 0;
  }

  return 1;
}


LRESULT
MsgPopUpProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_DIALOG                     *Dialog;
  UI_CONTROL                    *Control;
  UI_CONTROL                    *ListControl;
  UI_CONTROL                    *FocusControl;
  CHAR16                        *BodyString;
  CHAR16                        ButtonWidthStr[20];
  UINTN                         Index;
  INTN                          Result;
  EFI_STATUS                    Status;

  Dialog  = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);

  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    if (mFbDialog->TitleString != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogTitle");
      UiSetAttribute (Control, L"text", mFbDialog->TitleString);
      UiSetAttribute (Control, L"visible", L"true");
    }

    if (mFbDialog->BodyStringArray != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogText");
      BodyString = CatStringArray (mFbDialog->BodyStringCount, (CONST CHAR16 **) mFbDialog->BodyStringArray);
      if (BodyString != NULL) {
        UiSetAttribute (Control, L"text", BodyString);
        FreePool (BodyString);
      }
    }

    ListControl = UiFindChildByName (Dialog, L"DialogButtonList");
    UiSetAttribute (ListControl, L"visible", L"true");

    Control = CreateControl (L"Control", ListControl);
    CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);

    UnicodeSPrint (ButtonWidthStr, sizeof (ButtonWidthStr), L"%d", GetButtonWidthByStr ());
    FocusControl = NULL;
    for (Index = 0; Index < mFbDialog->ButtonCount; Index++) {
      if (mFbDialog->ButtonStringArray[Index] == NULL) {
        continue;
      }

      Control = CreateControl (L"Button", ListControl);
      SetWindowLongPtr (Control->Wnd, GWLP_USERDATA, (INTN) Index);

      UiApplyAttributeList (Control, L"name='Button' height='30' fontsize='19' textcolor='0xFFFFFFFF' align='center' align='singleline' bkcolor='0xFFCCCCCC' focusbkcolor='@menulightcolor'");
      UiSetAttribute (Control, L"text",  mFbDialog->ButtonStringArray[Index]);
      UiSetAttribute (Control, L"width", ButtonWidthStr);

      Status = CompareHiiValue (&mFbDialog->ButtonHiiValueArray[Index], &mFbDialog->ConfirmHiiValue, &Result);
      if (!EFI_ERROR(Status) && Result == 0) {
        FocusControl = Control;
      }
      CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);
    }
    if (FocusControl != NULL) {
      SetFocus (FocusControl->Wnd);
    }

    Control = CreateControl (L"Control", ListControl);
    CONTROL_CLASS(ListControl)->AddChild (ListControl, Control);
    break;

  case UI_NOTIFY_CLICK:
    Control = (UI_CONTROL *) WParam;
    Index = (UINTN) GetWindowLongPtr (Control->Wnd, GWLP_USERDATA);
    SendChangeQNotify (0, 0, &mFbDialog->ButtonHiiValueArray[Index]);
    break;

  case WM_HOTKEY:
    if (HIWORD(LParam) == VK_ESCAPE) {
      SendShutDNotify ();
      return 0;
    }
    return 1;

  case WM_DESTROY:
    FreeDialogEvent (&mFbDialog);
    return 0;

  default:
    return 0;
  }

  return 1;
}


HWND
FindOverlayDialog (
  )
{
  UI_CONTROL                    *Control;
  UI_CONTROL                    *Overlay;
  UI_MANAGER                    *Manager;


  Control = GetUiControl (gWnd);
  Manager = Control->Manager;
  Overlay = Manager->FindControlByName (Manager, L"overlay");
  if (Overlay != NULL) {
    CONTROL_CLASS(Overlay)->SetAttribute (Overlay, L"visible", L"true");
    return Overlay->Wnd;
  }

  return NULL;
}


HWND
LocalMetroCreateDialog (
  IN CONST H2O_DISPLAY_ENGINE_EVT          *Notify
  )
{
  HWND                          Wnd;
  CHAR16                        *XmlBuffer;
  H2O_DISPLAY_ENGINE_EVT_OPEN_D *OpenDNotify;
  H2O_FORM_BROWSER_D            *Dialog;
  WNDPROC                       DialogProc;
  BOOLEAN                       CloseOnTouchOutside;
  UINT8                         OpCode;
  HWND                          ParentWnd;
  RECT                          DlgRect;
  EFI_STATUS                    Status;

  OpenDNotify = (H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify;
  Dialog      = &OpenDNotify->Dialog;

  Wnd = NULL;
  XmlBuffer = NULL;
  DialogProc = NULL;
  if (gFB->CurrentP != NULL) {
    CloseOnTouchOutside = TRUE;
    mIsSendForm         = TRUE;
  } else {
    CloseOnTouchOutside = FALSE;
    mIsSendForm         = FALSE;
    ZeroMem (&mPreviousAbsPtrState, sizeof (EFI_ABSOLUTE_POINTER_STATE));
  }

  SetRectEmpty (&DlgRect);
  ParentWnd = GetFocus();
  mFbDialog = Dialog;

  if ((Dialog->DialogType & H2O_FORM_BROWSER_D_TYPE_QUESTIONS) == 0) {
    if ((Dialog->DialogType & H2O_FORM_BROWSER_D_TYPE_SHOW_HELP) == H2O_FORM_BROWSER_D_TYPE_SHOW_HELP) {
      Status = GetInformationField (&DlgRect);
      ASSERT (!EFI_ERROR (Status));
      XmlBuffer = mHelpDialogChilds;
      DialogProc = H2OHelpDialogProc;
    } else if ((Dialog->DialogType & H2O_FORM_BROWSER_D_TYPE_SHOW_CONFIRM_PAGE) == H2O_FORM_BROWSER_D_TYPE_SHOW_CONFIRM_PAGE) {
      Status = GetInformationField (&DlgRect);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_INFO, "GetInformationField(): %r", Status));
      }

      XmlBuffer = mConfirmPageDialogChilds;
      DialogProc = H2OConfirmPageDialogProc;

    } else if ((Dialog->DialogType & H2O_FORM_BROWSER_D_TYPE_SELECTION) == H2O_FORM_BROWSER_D_TYPE_SELECTION) {
      mFbDialog = CopyDialogEvent (Dialog);
      if (mFbDialog == NULL) {
        return NULL;
      }

      if (mIsSendForm) {
        Status = GetRectByName (gWnd, L"H2OHelpTextPanel", &DlgRect);
        ASSERT (!EFI_ERROR (Status));
        XmlBuffer = mCommonDialogChilds;
        DialogProc = H2OPopupDialogProc;
      } else {
        Status = GetDialogRegion (L"DialogPanel", &DlgRect);
        ASSERT_EFI_ERROR (Status);
        XmlBuffer = mDialogWithoutSendFormChilds;
        DialogProc = DialogWithoutSendFormProc;
      }
    } else if ((Dialog->DialogType & H2O_FORM_BROWSER_D_TYPE_MSG) == H2O_FORM_BROWSER_D_TYPE_MSG) {
      if (mIsSendForm) {
        Status = GetRectByName (gWnd, L"H2OHelpTextPanel", &DlgRect);
        ASSERT_EFI_ERROR (Status);
        XmlBuffer = mCommonDialogChilds;
      } else {
        Status = GetDialogRegion (L"DialogPanel", &DlgRect);
        ASSERT_EFI_ERROR (Status);
        XmlBuffer = mDialogWithoutSendFormChilds;
      }

      DialogProc = MsgPopUpProc;
      mFbDialog = CopyDialogEvent (Dialog);
      if (mFbDialog == NULL) {
        return NULL;
      }
    } else {
      return NULL;
    }
  } else {
    OpCode = GetOpCodeByDialogType (Dialog->DialogType);

    switch (OpCode) {

    case EFI_IFR_ONE_OF_OP:
      mFbDialog = CopyDialogEvent (Dialog);
      if (mFbDialog == NULL) {
        return NULL;
      }

      if (mIsSendForm) {
        Status = GetRectByName (gWnd, L"H2OHelpTextPanel", &DlgRect);
        XmlBuffer = mCommonDialogWithHelpChilds;
      } else {
        Status = GetDialogRegion (L"DialogPanel", &DlgRect);
        XmlBuffer = mOneOfDialogWithoutSendFormChilds;
      }
      DialogProc = H2OOneOfInputProc;
      ASSERT (!EFI_ERROR (Status));
      break;

    case EFI_IFR_NUMERIC_OP:
      //
      // TODO: Need receive and sending H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q to synchronize
      // with other display engine.
      //
      if (mIsSendForm) {
        Status = GetRectByName (gWnd, L"H2OHelpTextPanel", &DlgRect);
        ASSERT (!EFI_ERROR (Status));
        XmlBuffer = mCommonDialogWithHelpChilds;
        DialogProc = H2ONumericDialogProc;
      } else {
        Status = GetDialogRegion (L"DialogPanel", &DlgRect);
        ASSERT_EFI_ERROR (Status);
        OffsetRect (&DlgRect, 0, -30);
        XmlBuffer = mNumericDialogWithoutSendFormChilds;
        DialogProc = H2ONumericDialogWithoutSendFormProc;
      }
      CloseOnTouchOutside = FALSE;
      break;

    case EFI_IFR_ORDERED_LIST_OP:
      Status = GetRectByName (gWnd, L"H2OHelpTextPanel", &DlgRect);
      ASSERT (!EFI_ERROR (Status));
      XmlBuffer = mCommonDialogWithHelpChilds;
      DialogProc = H2OOrderedListInputProc;
      CloseOnTouchOutside = FALSE;
      break;

    case EFI_IFR_CHECKBOX_OP:
    case EFI_IFR_PASSWORD_OP:
      //
      // TODO: Need receive and sending H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q to synchronize
      // with other display engine.
      //
      if (mIsSendForm) {
        Status = GetRectByName (gWnd, L"H2OHelpTextPanel", &DlgRect);
        ASSERT (!EFI_ERROR (Status));
        XmlBuffer = mCommonDialogWithHelpChilds;
        DialogProc = H2OPasswordDialogProc;
      } else {
        Status = GetDialogRegion (L"DialogPanel", &DlgRect);
        ASSERT_EFI_ERROR (Status);
        XmlBuffer = mDialogWithoutSendFormChilds;
        DialogProc = H2OConfirmPasswordProc;
      }
      CloseOnTouchOutside = FALSE;
      break;

    case EFI_IFR_STRING_OP:
      //
      // TODO: Need receive and sending H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q to synchronize
      // with other display engine.
      //
      Status = GetRectByName (gWnd, L"H2OHelpTextPanel", &DlgRect);
      ASSERT (!EFI_ERROR (Status));
      XmlBuffer = mCommonDialogWithHelpChilds;
      DialogProc = H2OStringDialogProc;
      CloseOnTouchOutside = FALSE;
      break;

    case EFI_IFR_DATE_OP:
    case EFI_IFR_TIME_OP:
      Status = GetRectByName (gWnd, L"H2OHelpTextPanel", &DlgRect);
      ASSERT (!EFI_ERROR (Status));
      XmlBuffer  = mCommonDialogWithHelpChilds;
      DialogProc = H2ODateTimeDialogProc;
      break;

    default:
      ASSERT (FALSE);
    }
  }

  Wnd = CreateModalDialog (
          NULL,
          XmlBuffer,
          ParentWnd,
          DialogProc,
          0,
          DlgRect.left,
          DlgRect.top,
          DlgRect.right - DlgRect.left,
          DlgRect.bottom - DlgRect.top,
          CloseOnTouchOutside
          );

  UnregisterHotKey (Wnd, 1);
  MwRegisterHotKey (Wnd, 1,  0, VK_ESCAPE);

  return Wnd;
}

VOID
LocalMetroRegisterHotkey (
  HWND                          Wnd
  );

HWND
GetNextTabItemByTabOrder (
  IN  HWND                      Dialog,
  IN  UI_CONTROL                *Current,
  IN  BOOL                      Previous
  )
{
  INTN                          Style;
  INTN                          ExStyle;
  HWND                          Child;
  HWND                          Node;
  UI_CONTROL                    *Candidate;
  UI_CONTROL                    *Next;
  UI_CONTROL                    *Control;

  Next      = NULL;
  Candidate = NULL;

  Node = GetWindow (Dialog, GW_CHILD);
  while (Node != NULL && Node != Dialog) {

    Child = GetWindow (Node, GW_CHILD);
    Style = GetWindowLongPtr (Node, GWL_STYLE);
    ExStyle = GetWindowLongPtr (Node, GWL_EXSTYLE);

    do {
      if (!((Style & WS_VISIBLE) && !(Style & WS_DISABLED))) {
        Child = NULL;
        break;
      }

      if (!(ExStyle & WS_EX_CONTROLPARENT)) {
        Child = NULL;
        break;
      }

      if (Style & WS_TABSTOP) {
        Child = NULL;
        Control = GetUiControl (Node);
        ASSERT (Control != NULL);
        if (Control == NULL) {
          break;
        }
        if (Control->TabOrder == (Current->TabOrder + 1)) {
          return Control->Wnd;
        }
        if ((Control->TabOrder > Current->TabOrder) &&
            ((Next == NULL) || (Next->TabOrder > Control->TabOrder))) {
          Next = Control;
        }
        if ((Control->TabOrder < Current->TabOrder) &&
            ((Candidate == NULL) || (Candidate->TabOrder > Control->TabOrder))) {
          Candidate = Control;
        }
      }
    } while (0);

    if (Child != NULL) {
      Node = Child;
    } else if (GetWindow (Node, GW_HWNDNEXT) != NULL) {
      Node = GetWindow (Node, GW_HWNDNEXT);
    } else {
      Node = GetParent (Node);
      while (Node != NULL && Node != Dialog) {
        if (GetWindow (Node, GW_HWNDNEXT) != NULL) {
          Node = GetWindow (Node, GW_HWNDNEXT);
          break;
        } else {
          Node = GetParent (Node);
        }
      }
    }
  }

  if (Next == NULL) {
    Next = Candidate;
  }

  if (Next != NULL) {
    return Next->Wnd;
  }
  return NULL;
}

HWND
GetNextTabItem (
  IN  HWND MainWnd,
  IN  HWND DlgWnd,
  IN  HWND CtrlWnd,
  IN  BOOL Previous
  )
{
  INTN                          Style;
  INTN                          ExStyle;
  UINT32                        WndSearch;
  HWND                          ChildFirst;
  HWND                          RetWnd;

  WndSearch = Previous ? GW_HWNDPREV : GW_HWNDNEXT;

  if (DlgWnd == CtrlWnd) {
    CtrlWnd = NULL;
  }

  ChildFirst = NULL;
  RetWnd = NULL;

  if (!CtrlWnd) {
    ChildFirst = GetWindow (DlgWnd, GW_CHILD);
    if (Previous) {
      ChildFirst = GetWindow (ChildFirst, GW_HWNDLAST);
    }
  } else if (IsChild (MainWnd, CtrlWnd)) {
    ChildFirst = GetWindow (CtrlWnd, WndSearch);
    if (!ChildFirst) {
      if (GetParent (CtrlWnd) != MainWnd) {
        ChildFirst = GetWindow (GetParent (CtrlWnd), WndSearch);
      } else {
        ChildFirst = GetWindow (CtrlWnd, Previous ? GW_HWNDLAST : GW_HWNDFIRST);
      }
    }
  }

  while (ChildFirst != NULL) {
    Style = GetWindowLongPtr (ChildFirst, GWL_STYLE);
    ExStyle = GetWindowLongPtr (ChildFirst, GWL_EXSTYLE);

    if ((Style & WS_TABSTOP) && (Style & WS_VISIBLE) && !(Style & WS_DISABLED)) {
      return ChildFirst;
    } else if ((ExStyle & WS_EX_CONTROLPARENT) && (Style & WS_VISIBLE) && !(Style &WS_DISABLED)) {
      RetWnd = GetNextTabItem (MainWnd, ChildFirst, NULL, Previous);
      if (RetWnd != NULL) {
        return RetWnd;
      }
    }

    ChildFirst = GetWindow (ChildFirst, WndSearch);
  }

  if (CtrlWnd != NULL) {
    HWND Parent;

    Parent = GetParent (CtrlWnd);
    while (Parent != NULL) {
      if (Parent == MainWnd) {
        break;
      }
      RetWnd = GetNextTabItem (MainWnd, GetParent (Parent), Parent, Previous);
      if (RetWnd != NULL) {
        break;
      }
    }
    if(RetWnd == NULL) {
      RetWnd = GetNextTabItem (MainWnd, MainWnd, NULL, Previous);
    }
  }

  return RetWnd ? RetWnd : CtrlWnd;
}



BOOLEAN
IsUiDialogMessage (
  HWND                          DlgWnd,
  LPMSG                         Msg
  )
{
  HWND                          Focus;
  HWND                          Next;
  LRESULT                       DlgCode;

  if ((DlgWnd != Msg->hwnd) && !IsChild(DlgWnd, Msg->hwnd)) {
    return FALSE;
  }

  switch(Msg->message)
  {

  case WM_KEYDOWN:
    DlgCode = SendMessage (Msg->hwnd, WM_GETDLGCODE, Msg->wParam, (LPARAM) Msg);
    if (DlgCode & (DLGC_WANTMESSAGE)) break;

    switch (Msg->wParam) {

    case VK_TAB:
      if (DlgCode & DLGC_WANTTAB) break;

      Focus = GetFocus ();

      Next = GetNextTabItemByTabOrder (DlgWnd, GetUiControl (Focus), FALSE);
      if (Next == NULL) {
        Next = GetNextTabItem (DlgWnd, DlgWnd, Focus == DlgWnd ? NULL : Focus, FALSE);
      }
      if (Next != NULL) {
        SetFocus (Next);
        return TRUE;
      }
      break;
    }
  }

  TranslateMessage (Msg);
  DispatchMessage (Msg);
  return TRUE;
}

EFI_STATUS
EFIAPI
LocalMetroDialogNotify (
  IN       H2O_DISPLAY_ENGINE_PROTOCOL     *This,
  IN CONST H2O_DISPLAY_ENGINE_EVT          *Notify
  )
{
  MSG                                 Msg;
  H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE *AbsPtr;
  H2O_DISPLAY_ENGINE_EVT_KEYPRESS     *KeyPress;
  UI_DIALOG                           *Dialog;

  switch (Notify->Type) {

  case H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_D:
    //
    // don't allow re-entry
    //
    ASSERT (mDialogWnd == NULL);
    if (mDialogWnd != NULL) {
      return EFI_ALREADY_STARTED;
    }
    EnableWindow (gWnd, FALSE);
    EnableWindow (GetDesktopWindow(), FALSE);
    mDialogWnd = LocalMetroCreateDialog (Notify);
    if (mDialogWnd == NULL) {
      SendShutDNotify ();
    }
    GrayOutBackground (gWnd, TRUE);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D:
    if (mDialogWnd != NULL) {
      DestroyWindow(mDialogWnd);
      mDialogWnd = NULL;
    }
    This->Notify = LocalMetroNotify;
    EnableWindow (gWnd, TRUE);
    GrayOutBackground (gWnd, FALSE);
    SetFocus (gWnd);
    LocalMetroRegisterHotkey (gWnd);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE:
    AbsPtr = (H2O_DISPLAY_ENGINE_EVT_ABS_PTR_MOVE *) Notify;
    Dialog = (UI_DIALOG *) GetWindowLongPtr (mDialogWnd, 0);

    if (Dialog->CloseDlgWhenTouchOutside && ClickOnOutsideOfDlgRegion (&AbsPtr->AbsPtrState) && mPreviousAbsPtrState.ActiveButtons == 0) {
      CopyMem (&mPreviousAbsPtrState, &AbsPtr->AbsPtrState, sizeof (EFI_ABSOLUTE_POINTER_STATE));
      SendShutDNotify ();
      break;
    }

    CopyMem (&mPreviousAbsPtrState, &AbsPtr->AbsPtrState, sizeof (EFI_ABSOLUTE_POINTER_STATE));
    GdAddPointerData (NULL, &AbsPtr->AbsPtrState);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS:
    KeyPress = (H2O_DISPLAY_ENGINE_EVT_KEYPRESS *) Notify;
    GdAddEfiKeyData (&KeyPress->KeyData);
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_TIMER:
    if (((H2O_DISPLAY_ENGINE_EVT_TIMER *) Notify)->TimerId == H2O_METRO_DE_TIMER_ID_PERIODIC_TIMER) {
      RegisterTimerEvent (H2O_METRO_DE_TIMER_ID_PERIODIC_TIMER, H2O_METRO_DE_TIMER_PERIODIC_TIME);
    }
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH_Q:
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q:
    break;

  case H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_EXIT:
    break;

  default:
    ASSERT (FALSE);
  }

  while (PeekMessage (&Msg, NULL, 0, 0, PM_REMOVE)) {
    if (Msg.message == WM_RBUTTONUP) {
      SendMessage (mDialogWnd, WM_HOTKEY, 0, MAKELPARAM(0, VK_ESCAPE));
    } else if (mDialogWnd == NULL || !IsUiDialogMessage (mDialogWnd, &Msg)) {
      TranslateMessage (&Msg);
      DispatchMessage (&Msg);
    }
  }

  return EFI_SUCCESS;
}

