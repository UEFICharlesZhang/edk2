/** @file
  UI Common Controls

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

#include "H2OControls.h"
#include "H2ODisplayEngineLocalMetro.h"
#include "MetroUi.h"

STATIC H2O_SETUP_PAGE_ITEM_CLASS    *mSetupPageItemClass = NULL;
#define CURRENT_CLASS              mSetupPageItemClass

CHAR16 *mSubtitleChilds = L""
  L"<Label name='OptionPrompt' align='singleline' height='35' padding='0,0,0,20' fontsize='21' textcolor='0xFF4D4D4D'/>";

CHAR16 *mH2OCheckBoxOpChilds = L""
  L"<Texture float='true' bkimage='@OptionBkg' name='OptionBkg' bkimagestyle='stretch'/>"
  L"<HorizontalLayout>"
    L"<Control width='60'/>"
    L"<Label align='singleline' textcolor='0xFF666666' fontsize='21' name='OptionPrompt'/>"
    L"<HorizontalLayout padding='17,0,17,0' width='120' height='70'>"
      L"<Switch switchcolor='@menucolor' name='CheckBox'/>"
    L"</HorizontalLayout>"
    L"<Label width='30' />"
  L"</HorizontalLayout>";

CHAR16 *mStatementChilds = L""
  L"<Texture float='true' bkimage='@OptionBkg' name='OptionBkg' bkimagestyle='stretch'/>"
  L"<HorizontalLayout>"
    L"<Control width='60' name='OptionImagePadding'>"
      L"<Texture name='OptionImage' bkimagestyle='stretch'/>"
    L"</Control>"
    L"<HorizontalLayout name='OptionLayout'>"
      L"<Label align='singleline' textcolor='0xFF666666' fontsize='21' name='OptionPrompt'/>"
      L"<Label width='110' align='singleline' textcolor='0xFF666666' fontsize='21' name='OptionValue'/>"
      L"<Label name='OptionEnd' text='>' align='singleline' width='60' fontsize='21' textcolor='0xFF666666'/>"
    L"</HorizontalLayout>"
  L"</HorizontalLayout>";

CHAR16 *mH2OTextOpChilds = L""
  L"<HorizontalLayout childpadding='2' width='match_parent' minheight='35' height='wrap_content'>"
    L"<HorizontalLayout childpadding='2' float='true' width='match_parent' height='match_parent'>"
      L"<Texture scale9grid='1,1,1,1' bkimage='@SetupMenuTextOpBkg' name='OptionPromptBackground' width='258' height='match_parent'/>"
      L"<Texture scale9grid='1,1,1,1' bkimage='@SetupMenuTextOpBkg' name='OptionValueBackground' height='match_parent'/>"
    L"</HorizontalLayout>"
    L"<Label name='OptionPrompt' minheight='35' height='wrap_content' padding='7,0,7,56' width='258' fontsize='20' textcolor='0xFF4D4D4D'/>"
    L"<Label name='OptionValue' align='center' minheight='35' height='wrap_content' padding='7,0,7,0' width='258' fontsize='20' textcolor='@menucolor'/>"
    L"<Texture float='true' bkcolor='@menucolor' pos='30,11,38,19'/>"
  L"</HorizontalLayout>";


#define  OPTION_FONT_SIZE         21
#define  OPTION_IMAGE_SIZE        25

#define  OPTION_LEFT_PADDING      60
#define  OPTION_MIN_PROMPT_WIDTH  200
#define  OPTION_VALUE_WIDTH       105
#define  OPTION_RIGHT_PADDING     30


STATIC
COLORREF
GetImageColor (
  VOID
  )
{
  EFI_IMAGE_INPUT                 *ImageIn;
  UINTN                           Index;
  UINTN                           BufferLen;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *Pixel;

  ImageIn = GetCurrentFormSetImage ();
  if (ImageIn == NULL) {
    return 0;
  }

  BufferLen = ImageIn->Width * ImageIn->Height;
  for (Index = 0; Index < BufferLen; Index++) {
    if (ImageIn->Bitmap[Index].Reserved != 0) {
      Pixel = &ImageIn->Bitmap[Index];
      return (0xFF000000 | (Pixel->Red << 16) | (Pixel->Green << 8) | Pixel->Blue);
    }
  }
  return 0;
}


VOID
UpdateSetupPageItem (
  UI_CONTROL                    *Control
  )
{
  H2O_SETUP_PAGE_ITEM          *This;
  H2O_FORM_BROWSER_S        *Statement;
  CHAR16                    Str[20];
  UI_CONTROL                *Child;
  UINT32                    Index;
  INTN                      Result;
  EFI_STATUS                Status;
  INT32                     Height;
  UI_CONTROL                *CheckBox;

  Height = 70;
  This      = (H2O_SETUP_PAGE_ITEM *) Control;
  if (This->Statement == NULL) {
    return ;
  }

  Statement = This->Statement;
  ASSERT (Statement != NULL);
  if (Statement == NULL) {
    return;
  }

  //
  // Add Statement image if need
  //
  if (Statement->Image != NULL) {
    Child = UiFindChildByName (This, L"OptionImage");
    UnicodeSPrint (Str, sizeof (Str), L"0x%p", Statement->Image);
    UiSetAttribute (Child, L"bkimage", Str);
    UnicodeSPrint (
      Str,
      sizeof (Str),
      L"%d,%d,%d,%d",
      (Height - OPTION_IMAGE_SIZE) / 2 + (Height - OPTION_IMAGE_SIZE) % 2,
      17,
      (Height - OPTION_IMAGE_SIZE) / 2,
      18
      );
    Child = UiFindChildByName (This, L"OptionImagePadding");
    UiSetAttribute (Child, L"padding", Str);
  }

  //
  // Adjust text and text color
  //
  if (Statement->Prompt != NULL) {
    Child = UiFindChildByName (This, L"OptionPrompt");
    UiSetAttribute (Child, L"text", Statement->Prompt);
  }

  if (Statement->Operand == EFI_IFR_CHECKBOX_OP) {
    CheckBox = UiFindChildByName (This, L"CheckBox");
    if (Statement->HiiValue.Value.b) {
      UiSetAttribute (CheckBox, L"checkboxvalue", L"true");
    } else {
      UiSetAttribute (CheckBox, L"checkboxvalue", L"false");
    }
  } else if (Statement->Operand == EFI_IFR_TEXT_OP) {
    if (Statement->TextTwo != NULL && Statement->TextTwo[0] != '\0') {
      Child = UiFindChildByName (This, L"OptionValue");
      UiSetAttribute (Child, L"text", Statement->TextTwo);
    } else {
      Child = UiFindChildByName (This, L"OptionPrompt");
      UiSetAttribute (Child, L"width", L"0");

      Child = UiFindChildByName (This, L"OptionValue");
      UiSetAttribute (Child, L"visible", L"false");

      Child = UiFindChildByName (This, L"OptionPromptBackground");
      UiSetAttribute (Child, L"width", L"0");

      Child = UiFindChildByName (This, L"OptionValueBackground");
      UiSetAttribute (Child, L"visible", L"false");
    }
  } else if (Statement->NumberOfOptions != 0) {
    UnicodeSPrint (Str, sizeof (Str), L"0x%x", GetImageColor ());
    for (Index = 0; Index < Statement->NumberOfOptions; Index++) {
      Status = CompareHiiValue (&Statement->Options[Index].HiiValue, &Statement->HiiValue, &Result);
      if (!EFI_ERROR (Status) && Result == 0) {
        Child = UiFindChildByName (This, L"OptionValue");
        UiSetAttribute (Child, L"text", Statement->Options[Index].Text);
        if (Statement->Selectable) {
          UiSetAttribute (Child, L"textcolor", Str);
          Child = UiFindChildByName (This, L"OptionEnd");
          UiSetAttribute (Child, L"textcolor", Str);
        }
        break;
      }
    }
  } else if (Statement->Operand == EFI_IFR_NUMERIC_OP) {
    Child = UiFindChildByName (This, L"OptionValue");
    UnicodeSPrint (Str, sizeof (Str), L"%d", Statement->HiiValue.Value.u64);
    UiSetAttribute (Child, L"text", Str);

    if (Statement->Selectable) {
      UnicodeSPrint (Str, sizeof (Str), L"0x%x", GetImageColor ());
      UiSetAttribute (Child, L"textcolor", Str);
      Child = UiFindChildByName (This, L"OptionEnd");
      UiSetAttribute (Child, L"textcolor", Str);
    }
  } else if (Statement->Operand == EFI_IFR_ACTION_OP) {
    Child = UiFindChildByName (This, L"OptionValue");
    if (Child != NULL) {
      if (Statement->TextTwo != NULL && Statement->TextTwo[0] != '\0') {
        UiSetAttribute (Child, L"text", Statement->TextTwo);
        if (Statement->Selectable) {
          UnicodeSPrint (Str, sizeof (Str), L"0x%x", GetImageColor ());
        } else {
          UnicodeSPrint (Str, sizeof (Str), L"0x%x", L"0xFF666666");
        }
        UiSetAttribute (Child, L"textcolor", Str);
      } else {
        UiSetAttribute (Child, L"visible", L"false");
      }
      Child = UiFindChildByName (This, L"OptionEnd");
      UiSetAttribute (Child, L"visible", L"false");
    }
  } else {
    Child = UiFindChildByName (This, L"OptionValue");
    if (Child != NULL) {
      UiSetAttribute (Child, L"visible", L"false");
      Child = UiFindChildByName (This, L"OptionEnd");
      UiSetAttribute (Child, L"visible", L"false");
    }
  }
}

BOOLEAN
EFIAPI
H2OSetupPageItemSetAttribute (
  UI_CONTROL *Control,
  CHAR16     *Name,
  CHAR16     *Value
  )
{
  H2O_SETUP_PAGE_ITEM    *This;
  EFI_STATUS          Status;
  UI_CONTROL          *Switch;

  This = (H2O_SETUP_PAGE_ITEM *) Control;

  if (StrCmp (Name, L"statement") == 0) {
    CONTROL_CLASS (This)->RemoveAllChild (Control);

    This->Statement = (H2O_FORM_BROWSER_S *)(UINTN) StrToUInt (Value, 16, &Status);
    if (This->Statement->Operand == EFI_IFR_SUBTITLE_OP) {
      CONTROL_CLASS_SET_STATE(Control, UISTATE_DISABLED, 0);
      XmlCreateControl (mSubtitleChilds, Control);
    } else if (This->Statement->Operand == EFI_IFR_CHECKBOX_OP) {
      XmlCreateControl (mH2OCheckBoxOpChilds, Control);
      Switch = UiFindChildByName (This, L"CheckBox");
      UiApplyAttributeList (Control, L"minheight='70'");
      SetWindowLongPtr (Switch->Wnd, GWLP_USERDATA, This->Statement->StatementId);
    } else if (This->Statement->Operand == EFI_IFR_TEXT_OP) {
      XmlCreateControl (mH2OTextOpChilds, Control);
    } else {
      switch (This->Statement->Operand) {

      case EFI_IFR_REF_OP:
      case EFI_IFR_ACTION_OP:
      case EFI_IFR_PASSWORD_OP:
      case EFI_IFR_NUMERIC_OP:
      case EFI_IFR_ONE_OF_OP:
      case EFI_IFR_TIME_OP:
      case EFI_IFR_DATE_OP:
      case EFI_IFR_ORDERED_LIST_OP:
      case EFI_IFR_RESET_BUTTON_OP:
      case EFI_IFR_STRING_OP:
        XmlCreateControl (mStatementChilds, Control);
        UiApplyAttributeList (Control, L"minheight='70'");
        break;

      default:
        DEBUG ((EFI_D_ERROR, "Unsupported opcode : %d\n", This->Statement->Operand));
        This->Statement = NULL;
        ASSERT (FALSE);
        break;
      }
    }
    UpdateSetupPageItem (Control);
    CONTROL_CLASS_INVALIDATE (This);
    return TRUE;
  }

  return PARENT_CLASS_SET_ATTRIBUTE (CURRENT_CLASS, Control, Name, Value);

}


LRESULT
EFIAPI
H2OSetupPageItemProc (
  HWND   Hwnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  H2O_SETUP_PAGE_ITEM      *This;
  UI_CONTROL            *Control;

  This = (H2O_SETUP_PAGE_ITEM *) GetWindowLongPtr (Hwnd, 0);
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  Control = (UI_CONTROL *)This;

  switch (Msg) {

  case WM_CREATE:
    This = (H2O_SETUP_PAGE_ITEM *) AllocateZeroPool (sizeof (H2O_SETUP_PAGE_ITEM));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Hwnd, 0);
      SetWindowLongPtr (Hwnd, 0, (INTN)This);
      SendMessage (Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, UI_NOTIFY_CREATE, WParam, LParam);
    Control->FixedSize.cy = -2;
    Control->MinSize.cy   = 35;
    break;

  case UI_NOTIFY_PAINT:
    if (WParam == PAINT_BKCOLOR || WParam == PAINT_STATUSIMAGE) {
      break;
    }
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);

    if (WParam == PAINT_ALL &&
        (CONTROL_CLASS_GET_STATE (This) & (UISTATE_SELECTED | UISTATE_FOCUSED)) == (UISTATE_SELECTED | UISTATE_FOCUSED)) {
      COLORREF                  Color;
      UI_MANAGER                *Manager;
      RECT                      Rc;
      HDC                       Hdc;

      Color = 0xFFFF0000;
      Manager = Control->Manager;
      Hdc     = Manager->PaintDC;

      Manager->GetControlRect (Manager, Control, &Rc);

      SelectObject (Hdc, GetStockObject (PS_NULL));
      SelectObject (Hdc, GetStockObject (DC_PEN));
      SetDCPenColor (Hdc, Color);
      Rectangle (Hdc, Rc.left, Rc.top, Rc.right, Rc.bottom);
   }
   break;

  case WM_NCHITTEST:
    return HTTRANSPARENT;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Hwnd, Msg, WParam, LParam);
  }

  return 0;
}

H2O_SETUP_PAGE_ITEM_CLASS *
EFIAPI
GetSetupPageItemClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"SetupPageItem", (UI_CONTROL_CLASS *) GetControlClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc      = H2OSetupPageItemProc;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetAttribute = H2OSetupPageItemSetAttribute;

  return CURRENT_CLASS;
}


