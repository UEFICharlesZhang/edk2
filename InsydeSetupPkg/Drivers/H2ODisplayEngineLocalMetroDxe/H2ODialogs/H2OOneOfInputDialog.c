/** @file
  UI One Of Input Dialog

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

#include "H2ODisplayEngineLocalMetro.h"
#include "UiControls.h"
#include "MetroUi.h"
#include <Library/ConsoleLib.h>

extern H2O_FORM_BROWSER_D       *mFbDialog;
extern BOOLEAN                  mIsSendForm;

VOID
FreeDialogEvent (
  IN H2O_FORM_BROWSER_D                    **DialogEvt
  );

CHAR16 *mOneOfDialogChilds = L""
  L"<ListView name='OneOfList' vscrollbar='true' tabstop='true' padding='15,0,0,0' childpadding='2' taborder='1'/>";

CHAR16 *mOneOfDialogWithoutSendFormChilds = L""
  L"<VerticalLayout name='OneOfDialog'>"
    L"<Control/>"
    L"<HorizontalLayout height='wrap_content'>"
      L"<Control/>"
      L"<Control width='wrap_content' height='wrap_content'>"
        L"<VerticalLayout name='Content' width='wrap_content' height='wrap_content' padding='20,30,20,30' childpadding='15' bkcolor='@menucolor'>"
          L"<Label name='DialogTitle' align='center' height='wrap_content' width='match_parent' fontsize='19' textcolor='0xFFFFFFFF'/>"
          L"<Control height='10' bkimage='@DialogSeparator' bkcolor='0x0' bkimagestyle='center'/>"
          L"<ListView name='OneOfList' vscrollbar='true' width='wrap_content' height='wrap_content' padding='0,0,0,0' childpadding='2'/>"
        L"</VerticalLayout>"
        L"<Texture name='FormHalo' float='true' height='-1' width='-1' bkimage='@FormHalo' scale9grid='23,26,22,31'/>"
      L"</Control>"
      L"<Control/>"
    L"</HorizontalLayout>"
    L"<Control/>"
  L"</VerticalLayout>";

CHAR16 *mOneOfOptionChilds = L""
  L"<HorizontalLayout height='40'>"
    L"<Label name='Option' align='singleline|center' height='40' fontsize='19' bkcolor='0xFFE6E6E6' textcolor='0xFF404040'/>"
    L"<Texture name='OptionSelectedIcon' float='true' pos='260,12,281,29' visible='false' bkimage='@DialogSelectedIcon' bkcolor='0x0'/>"
  L"</HorizontalLayout>";

CHAR16 *mOneOfOptionChildsWithoutSendForm = L""
  L"<VerticalLayout height='40'>"
    L"<Label name='Option' align='singleline' height='40' padding='0,10,0,10' width='match_parent' fontsize='19' bkcolor='0xFFE6E6E6' textcolor='0xFF404040'/>"
  L"</VerticalLayout>";



VOID
AdjustImage (
  UI_CONTROL                    *Control,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Src,
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Dst
  )
{
  INT16                         HueDiff;
  INT8                          SaturationDiff;
  INT8                          ValueDiff;
  CHAR16                        Str[20];
  HSV_VALUE                     SrcHsv;
  HSV_VALUE                     DstHsv;

  RGB2HSV(Src, &SrcHsv);
  RGB2HSV(Dst, &DstHsv);

  if (DstHsv.Hue == 0 && DstHsv.Saturation == 0 && DstHsv.Value == 0) {
    return ;
  }
  HueDiff        = (INT16) (DstHsv.Hue - SrcHsv.Hue);
  SaturationDiff = (INT8)  (DstHsv.Saturation - SrcHsv.Saturation);
  ValueDiff      = (INT8)  (DstHsv.Value - SrcHsv.Value);
  UnicodeSPrint (Str, sizeof (Str), L"%d,%d,%d",  HueDiff, SaturationDiff, ValueDiff);
  UiSetAttribute (Control, L"hsvadjust", Str);
}



VOID
H2OOneOfOptionOnSetState (
  UI_CONTROL                    *Control,
  UI_STATE                      SetState,
  UI_STATE                      ClearState
  )
{
  UI_CONTROL                    *Child;
  COLORREF                      Color;
  EFI_IMAGE_INPUT               *Image;

  if (!((SetState & UISTATE_SELECTED) || (ClearState & UISTATE_SELECTED))) {
    return ;
  }

  if (SetState & UISTATE_SELECTED) {
    Child = UiFindChildByName (Control, L"Option");
    UiApplyAttributeList (Child, L"textcolor='0xFFFFFFFF' bkcolor='@menulightcolor' ");
    if (mIsSendForm) {
      Child = UiFindChildByName (Control, L"OptionSelectedIcon");
      ASSERT (Child != NULL);
      if (Child == NULL) {
        return;
      }

      if (IsWindowVisible (Child->Wnd)) {
        Color = GetColorValue (L"0xFFFFFFFF");
        Image = GetImageByString (Child->BkImage);
        if (Image != NULL) {
          AdjustImage (Child, Image->Bitmap, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)&Color);
        }
      }
    }

  } else {
    Child = UiFindChildByName (Control, L"Option");
    UiApplyAttributeList (Child, L"textcolor='0xFF404040' bkcolor='0xFFE6E6E6' ");
    if (mIsSendForm) {
      Child = UiFindChildByName (Control, L"OptionSelectedIcon");
      ASSERT (Child != NULL);
      if (Child == NULL) {
        return;
      }

      if (IsWindowVisible (Child->Wnd)) {
        Color = GetColorValue (L"@menulightcolor");
        Image = GetImageByString (Child->BkImage);
        if (Image != NULL) {
          AdjustImage (Child, Image->Bitmap, (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)&Color);
        }
      }
    }
  }
}

VOID
H2OOneOfOptionOnItemClick (
  UI_LIST_VIEW                  *This,
  UI_CONTROL                    *Item,
  UINT32                        Index
  )
{
  Index = (UINT32) GetWindowLongPtr (Item->Wnd, GWLP_USERDATA);
  SendChangeQNotify (0, 0, &mFbDialog->BodyHiiValueArray[Index]);
}

LRESULT
H2OOneOfInputProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_CONTROL                    *Layout;
  UI_DIALOG                     *Dialog;
  UI_MANAGER                    *Manager;
  UINTN                         Index;
  UI_CONTROL                    *Control;
  CHAR16                        Str[20];
  EFI_IMAGE_INPUT               *FormsetImage;
  UI_CONTROL                    *CreatedControl;
  INTN                          Result;
  EFI_STATUS                    Status;
  INT32                         MaxWidth;
  SIZE                          Size;
  HDC                           Hdc;
  INT32                         Padding;
  UI_CONTROL                    *Content;
  SIZE                          ContentSize;

  Dialog  = (UI_DIALOG *) GetWindowLongPtr (Wnd, 0);
  Manager = (UI_MANAGER *)((UI_CONTROL *)Dialog)->Manager;

  switch (Msg) {

  case UI_NOTIFY_WINDOWINIT:
    if (mFbDialog->TitleString != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogTitle");
      UiSetAttribute (Control, L"text", mFbDialog->TitleString);
    }

    Control = UiFindChildByName (Dialog, L"Content");
    if (Control != NULL &&  gFB->CurrentQ != NULL) {
      XmlCreateControl (mOneOfDialogChilds, Control);
    }

    Control = UiFindChildByName (Dialog, L"DialogTextScrollView");
    if (Control != NULL) {
      Control->OnSetState = H2OCommonDialogWithHelpOnSetState;
    }

    if ((mFbDialog->DialogType & H2O_FORM_BROWSER_D_TYPE_FROM_H2O_DIALOG) == 0 && gFB->CurrentQ != NULL && gFB->CurrentQ->Help != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogText");
      UiSetAttribute (Control, L"text", gFB->CurrentQ->Help);
    }

    FormsetImage = GetCurrentFormSetImage ();
    if (FormsetImage != NULL) {
      Control = UiFindChildByName (Dialog, L"DialogImage");
      UnicodeSPrint (Str, sizeof (Str), L"0x%p", FormsetImage);
      UiSetAttribute (Control, L"bkimage", Str);
      UiSetAttribute (Control, L"visible", L"true");
    }

    Layout = UiFindChildByName (Dialog, L"OneOfList");
    if (Layout->VScrollBar != NULL) {
      UiSetAttribute (Layout->VScrollBar, L"displayscrollbar", L"false");
      CONTROL_CLASS_INVALIDATE (Layout);
    }

    CreatedControl = NULL;
    ((UI_LIST_VIEW *)Layout)->OnItemClick = H2OOneOfOptionOnItemClick;
    for (Index = 0, Result = 1; Index < mFbDialog->BodyStringCount; Index++) {
      if (gFB->CurrentP != NULL) {
        CreatedControl = XmlCreateControl (mOneOfOptionChilds, Layout);
      } else {
        CreatedControl = XmlCreateControl (mOneOfOptionChildsWithoutSendForm, Layout);
      }

      SetWindowLongPtr (CreatedControl->Wnd, GWLP_USERDATA, (INTN) Index);
      CreatedControl->OnSetState = H2OOneOfOptionOnSetState;

      Control = UiFindChildByName (CreatedControl, L"Option");
      UiSetAttribute (Control, L"text", mFbDialog->BodyStringArray[Index]);
      Status = CompareHiiValue (&mFbDialog->BodyHiiValueArray[Index], &mFbDialog->ConfirmHiiValue, &Result);
      if (!EFI_ERROR (Status) && Result == 0) {
        Control = UiFindChildByName (CreatedControl, L"OptionSelectedIcon");
        if (Control != NULL) {
          UiSetAttribute (Control, L"visible", L"true");
        }

        LIST_VIEW_CLASS (Layout)->SetSelection (
                                  (UI_LIST_VIEW *)Layout,
                                  (INT32)Index,
                                  TRUE
                                  );
      }
    }

    if (gFB->CurrentP == NULL && CreatedControl != NULL) {
      //
      // calculate maximum width of option
      //
      MaxWidth = 360;
      Control = UiFindChildByName (CreatedControl, L"Option");
      Padding = Control->Padding.left + Control->Padding.right;
      Hdc = CreateCompatibleDC(NULL);
      Hdc->font->FontSize = ((UI_LABEL *)Control)->FontSize;
      for (Index = 0; Index < mFbDialog->BodyStringCount; Index++) {
        GetTextExtentPoint32 (Hdc, mFbDialog->BodyStringArray[Index], -1, &Size);
        MaxWidth = MAX (MaxWidth, Size.cx + Padding);
      }
      DeleteDC (Hdc);
      UnicodeSPrint (Str, sizeof (Str), L"%d", MaxWidth);
      UiSetAttribute (Layout, L"width", Str);

      //
      // calculate content width for estimate dialog title size correctly.
      //
      Content = UiFindChildByName (Dialog, L"Content");
      ContentSize.cx = 9999;
      ContentSize.cy = WRAP_CONTENT;
      ContentSize = CONTROL_CLASS (Content)->EstimateSize (Content, ContentSize);
      UnicodeSPrint (Str, sizeof (Str), L"%d", ContentSize.cx);
      UiSetAttribute (Content, L"width", Str);
    }

    SetFocus (Layout->Wnd);
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
