/** @file
Copyright (c) 2017, Byosoft Software Corporation. All Rights Reserved.

You may not reproduce, distribute, publish, display, perform, modify, adapt,
transmit, broadcast, present, recite, release, license or otherwise exploit
any part of this publication in any form, by any means, without the prior
written permission of Byosoft Software Corporation.

File Name:
  SetupUiLib.c

Abstract:
  Setup Ui Library implementation.

Revision History:

**/

#include "SetupUiLibInternal.h"

EFI_GUID          gSetupUiLibGuid = { 0xeecdab61, 0x5444, 0x429d, { 0xa7, 0x89, 0x6, 0xf3, 0x40, 0x5a, 0xc5, 0xe4 } };
EFI_HII_HANDLE    mSetupUiStringHandle;

#define DIALOG_WIDTH_MIN    (44)
#define DIALOG_BLANK_LINE    (1)

BOOLEAN    bSetButtonTitle_YesNo = FALSE;
CHAR16    *gYesButtonTitle;
CHAR16    *gNoButtonTitle;
BOOLEAN    bSetButtonTitle_Warning = FALSE;
CHAR16    *gEnterButton;
BOOLEAN    bSetButtonTitle_Info = FALSE;
CHAR16    *gContinueButton;
DIALOG_TYPE    gLastDialogType = DIALOG_MAX;
DIALOG_RANGE    gConfirmRange;

/**
  Set Title of Button form Left to Right according Type of Dialog which will 
  be created by UiConfirmDialog(). Setting valid once.

**/
BOOLEAN
UiSetButton (
  IN DIALOG_TYPE    Type,
  IN  CHAR16    *String,
  ...
  ) 
{
  VA_LIST    Marker;
  CHAR16    *StringTemp;
  BOOLEAN    ReturnFlag;
  
  bSetButtonTitle_YesNo = FALSE;
  if (gYesButtonTitle) {
    FreePool (gYesButtonTitle );
    gYesButtonTitle = NULL;
  }
  if (gNoButtonTitle) {
    FreePool (gNoButtonTitle );
    gNoButtonTitle = NULL;
  }

  bSetButtonTitle_Warning = FALSE;
  if (gEnterButton) {
    FreePool (gEnterButton );
    gEnterButton = NULL;
  }

  bSetButtonTitle_Info = FALSE;
  if (gContinueButton) {
    FreePool (gContinueButton );
    gContinueButton = NULL;
  }
	
  ReturnFlag = TRUE;
  if (Type == DIALOG_YESNO) {
    StringTemp = String;
    VA_START (Marker, String);

    gYesButtonTitle = AllocateCopyPool(StrSize(StringTemp), StringTemp);
    ASSERT (NULL != gYesButtonTitle);

    StringTemp = VA_ARG (Marker, CHAR16*);
    gNoButtonTitle = AllocateCopyPool(StrSize(StringTemp), StringTemp);
    ASSERT (NULL != gNoButtonTitle);

    VA_END(Marker);
    bSetButtonTitle_YesNo = TRUE;
  } else   if (Type == DIALOG_WARNING) {
    gEnterButton = AllocateCopyPool(StrSize(String), String);
    ASSERT (NULL != gEnterButton);
    bSetButtonTitle_Warning = TRUE;
  } else   if (Type == DIALOG_INFO) {
    gContinueButton = AllocateCopyPool(StrSize(String), String);
    ASSERT (NULL != gContinueButton);
    bSetButtonTitle_Info = TRUE;
  } else {
    bSetButtonTitle_YesNo = FALSE;
    bSetButtonTitle_Warning = FALSE;
    bSetButtonTitle_Info = FALSE;
    ReturnFlag = FALSE;
  }

  return ReturnFlag;
}

/**
  Draw a confirm pop up windows based on the Type, strings and Text Align. 
  TRUE will be return If YES be selected.

**/
BOOLEAN
UiConfirmDialog (
  IN DIALOG_TYPE    Type,
  IN CHAR16    *Title,
  IN TEXT_ALIGIN   Align,
  IN  CHAR16    *String,
  ...
  )
{
  VA_LIST    Marker;
  UINTN    Index;
  UINTN    Count;
  EFI_SCREEN_DESCRIPTOR    ScreenDimensions;
  UINTN    DimensionsWidth;
  UINTN    DimensionsHeight;
  UINTN    Start;
  UINTN    End;
  UINTN    Top;
  UINTN    Bottom;
  UINTN    YesStart;
  UINTN    NoStart;
  UINTN    EnterStart;
  UINTN    ContinueStart;  
  CHAR16    Character;
  CHAR16    *StringTemp;
  CHAR16    *StringNew;
  CHAR16    *SetupConfirmation;
  CHAR16    *EnterButton;
  CHAR16    *ContinueButton;
  CHAR16    *Buffer;
  EFI_INPUT_KEY    Key;
  CHAR16    ResponseArray[2];
  UINTN    ArrayIndex;
  USER_INPUT_TYPE    InputType;
  MOUSE_ACTION_INFO    Mouse;
  MOUSE_CURSOR_RANGE    RangeYes;
  MOUSE_CURSOR_RANGE    RangeNo;
  MOUSE_CURSOR_RANGE    RangeEnter;  
  UINTN    NumberOfStrings;
  UINTN    MaxWidth;
  UINTN    BasicColor;
  UINTN    FrameColor;
  UINTN    DisableColor;
  UINTN    SelectedColor;
  CHAR16    *mYesResponse;
  CHAR16    *mNoResponse;
  UINTN    PrintWidth;
  BOOLEAN    ReturnFlag;
  UINTN    OriginalAttribute;
  BOOLEAN    OriginalCursorVisible;
  //
  // Catch screen dimension info.
  //
  gST->ConOut->QueryMode (
                   gST->ConOut,
                   gST->ConOut->Mode->Mode,
                   &ScreenDimensions.RightColumn,
                   &ScreenDimensions.BottomRow
                   );
  DimensionsWidth = ScreenDimensions.RightColumn ;
  DimensionsHeight = ScreenDimensions.BottomRow;

  OriginalAttribute  = gST->ConOut->Mode->Attribute;
  OriginalCursorVisible = gST->ConOut->Mode->CursorVisible;
  gST->ConOut->EnableCursor (gST->ConOut, FALSE);


  //
  // Calculate string lines and Max length.
  // 
  if (NULL != Title) {
    SetupConfirmation = Title;
  } else {
    if (Type == DIALOG_WARNING) {
       SetupConfirmation = LibGetToken (STRING_TOKEN (STR_WARNING), mSetupUiStringHandle);
     } else {
      SetupConfirmation = LibGetToken (STRING_TOKEN (STR_CONFIRMATION_DIALOG), mSetupUiStringHandle);
    }
  }

  VA_START (Marker, String);
  NumberOfStrings = 0;
  StringTemp = String;
  MaxWidth = LibGetStringWidth(StringTemp)/2;
  while (NULL != StringTemp) {
    if (LibGetStringWidth(StringTemp)/2 > MaxWidth) {
      MaxWidth = LibGetStringWidth(StringTemp)/2;
    }
    NumberOfStrings++;
    StringTemp = VA_ARG (Marker, CHAR16*);
  }  
  VA_END(Marker);

  if (LibGetStringWidth(SetupConfirmation)/2 > MaxWidth) {
    MaxWidth = LibGetStringWidth(SetupConfirmation)/2;
  }    
  //if (MaxWidth < DIALOG_WIDTH_MIN) {
    // MaxWidth = DIALOG_WIDTH_MIN;
  //}
  if (MaxWidth > DimensionsWidth - 10) {
    MaxWidth = DimensionsWidth - 10;
  }
  if (NumberOfStrings > DimensionsHeight - 12) {
    NumberOfStrings = DimensionsHeight - 12;
   }
  MaxWidth += MaxWidth%2;

  //
  // Check Dialog Type.
  //
  if (Type >= DIALOG_MAX ) {
    Type = DIALOG_YESNO;
  }
  if (gBltBuffer) {
    FreePool (gBltBuffer);
    gBltBuffer = NULL;
  }
  gLastDialogType = Type;

  if (Type == DIALOG_YESNO || Type == DIALOG_INFO || Type == DIALOG_NO_KEY) {
    BasicColor = EFI_WHITE | EFI_BACKGROUND_BLUE;
    FrameColor = EFI_GREEN |EFI_BACKGROUND_BLUE;
    DisableColor = EFI_LIGHTGRAY |EFI_BACKGROUND_BLUE;
    SelectedColor = EFI_WHITE |EFI_BACKGROUND_CYAN;
  }else if (Type == DIALOG_WARNING) {
    BasicColor = EFI_WHITE | EFI_BACKGROUND_RED;
    FrameColor = EFI_GREEN |EFI_BACKGROUND_RED;
    DisableColor = EFI_LIGHTGRAY |EFI_BACKGROUND_RED;
    SelectedColor = EFI_WHITE |EFI_BACKGROUND_CYAN;
  } else {
    BasicColor = EFI_WHITE | EFI_BACKGROUND_BLUE;
    FrameColor = EFI_GREEN |EFI_BACKGROUND_BLUE;
    DisableColor = EFI_LIGHTGRAY |EFI_BACKGROUND_BLUE;
    SelectedColor = EFI_WHITE |EFI_BACKGROUND_CYAN;
  }

  //
  // Catch String every time.
  //
  if (bSetButtonTitle_YesNo) {
    mYesResponse  = gYesButtonTitle;
    mNoResponse  = gNoButtonTitle;    
  } else {
    mYesResponse  = LibGetToken (STRING_TOKEN (ARE_YOU_SURE_YES), mSetupUiStringHandle);
    mNoResponse  = LibGetToken (STRING_TOKEN (ARE_YOU_SURE_NO), mSetupUiStringHandle);
  }

  //
  //Calculate Dimesion.
  //
  Start = (DimensionsWidth - MaxWidth - 2) / 2;
  End = Start + MaxWidth + 2;

  Top = ((DimensionsHeight - NumberOfStrings - 6) / 2);
  if (Type != DIALOG_NO_KEY) {
    Bottom = Top + NumberOfStrings + 4 + DIALOG_BLANK_LINE;
  } else {
    Bottom = Top + NumberOfStrings + 4;
  }

 YesStart = Start  + (MaxWidth * 1)/4 - LibGetStringWidth(mYesResponse)/4;
  if (YesStart <= Start  + MaxWidth/16) {
     YesStart = Start  + MaxWidth/16;
  }
 NoStart =  Start  + (MaxWidth * 3)/4 - LibGetStringWidth(mNoResponse)/4;
 if (NoStart <= Start + (MaxWidth*9)/16) {
    NoStart = Start  + (MaxWidth * 9)/16;
 }

  if (bSetButtonTitle_Warning) {
    EnterButton = gEnterButton;
  } else {
    EnterButton = LibGetToken (STRING_TOKEN (STR_ENTER_BUTTON), mSetupUiStringHandle);
  }
  EnterStart = Start + (MaxWidth - LibGetStringWidth(EnterButton)/2)/2 +1;
  if (EnterStart <= Start + MaxWidth/4) {
    EnterStart = Start + MaxWidth/4;
  }

  if (bSetButtonTitle_Info) {
    ContinueButton = gContinueButton;
  } else {
    ContinueButton = LibGetToken (STRING_TOKEN (STR_CONTINUE), mSetupUiStringHandle);
  }
  ContinueStart = Start + (MaxWidth - LibGetStringWidth(ContinueButton)/2)/2 +1;
  if (ContinueStart <= Start + MaxWidth/4) {
    ContinueStart = Start + MaxWidth/4;
  }

  //
  // Store the Popup background.
  //
  gConfirmRange.Left = Start * EFI_GLYPH_WIDTH;
  gConfirmRange.Right = End  * EFI_GLYPH_WIDTH; 
  gConfirmRange.Top = Top * EFI_GLYPH_HEIGHT;
  gConfirmRange.Bottom = Bottom * EFI_GLYPH_HEIGHT;
  
  UiStoreScreen(gBS, TRUE);
  DEBUG((EFI_D_ERROR, "UIsave gBltBuffer:%x,gLastDialogType:%x,Type:%x\n", gBltBuffer,gLastDialogType,Type));

  //
  // Draw Dialog.
  //
  UiClearLines (
    Start,
    End,
    Top,
    Bottom,
    BasicColor
    );

  gST->ConOut->SetAttribute (gST->ConOut, FrameColor);
  Buffer = NULL;
  Buffer = AllocateZeroPool (0x100);
  ASSERT(NULL != Buffer);
  Character = BOXDRAW_HORIZONTAL;
  for(Index = 0; Index + 2 < End - Start; Index++){
    Buffer[Index] = Character;
  }

  Character = BOXDRAW_DOWN_RIGHT;
  UiPrintCharAt (Start, Top, Character);
  UiPrintString (Buffer);

  Character = BOXDRAW_DOWN_LEFT;
  UiPrintChar (Character);
  Character = BOXDRAW_VERTICAL;
  UiPrintCharAt (Start, Top + 1, Character);
  UiPrintCharAt (End - 1, Top + 1, Character);

  //
  // Dialog Title.
  //
  gST->ConOut->SetAttribute (gST->ConOut, BasicColor);
  if (LibGetStringWidth(SetupConfirmation)/2 > MaxWidth) {
    StringTemp = NULL;
    StringTemp = AllocateCopyPool ((MaxWidth + 1) * 2, SetupConfirmation);
    if (StringTemp ) {		
      StringTemp[MaxWidth -5] = L'.';
      StringTemp[MaxWidth -4] = L'.';
      StringTemp[MaxWidth -3] = L'.';
      StringTemp[MaxWidth -2] = L'.';
      StringTemp[MaxWidth -1] = CHAR_NULL;
      StringTemp[MaxWidth] = CHAR_NULL;
      UiPrintStringAt (((DimensionsWidth - MaxWidth) / 2) + 1, Top + 1, StringTemp);
      FreePool (StringTemp);
      StringTemp = NULL;
    }
  } else {
    UiPrintStringAt(((DimensionsWidth - LibGetStringWidth (SetupConfirmation) / 2) / 2) + 1, Top + 1, SetupConfirmation);
  }
  if (NULL == Title) {
    FreePool (SetupConfirmation);
  }

  gST->ConOut->SetAttribute (gST->ConOut, FrameColor);
  Character = BOXDRAW_VERTICAL_RIGHT;
  UiPrintCharAt (Start, Top + 2, Character);
  UiPrintString (Buffer);
  Character = BOXDRAW_VERTICAL_LEFT;
  UiPrintChar (Character);
  Character = BOXDRAW_VERTICAL;
  UiPrintCharAt (Start, Top + 3, Character);
  UiPrintCharAt (End - 1, Top + 3, Character);
  UiPrintCharAt (Start, Top + 4, Character);
  UiPrintCharAt (End - 1, Top + 4, Character);

  if (Align >= TEXT_ALIGIN_MAX) {
    Align = TEXT_ALIGIN_CENTER;
  }
  
  Character = BOXDRAW_VERTICAL;
  VA_START (Marker, String);
  StringTemp = String;
  for (Index = Top, Count = 0; Count < NumberOfStrings; Index++, Count++) {
    gST->ConOut->SetAttribute (gST->ConOut, BasicColor);  	

    StringNew = AllocateCopyPool (LibGetStringWidth(StringTemp), StringTemp);
    if (NULL == StringNew) {
      continue;
    }
	
    PrintWidth = LibGetStringWidth(StringTemp)/2;
	
    if (PrintWidth > MaxWidth) {
     StringNew[MaxWidth-5] = L'.';
     StringNew[MaxWidth-4] = L'.';
     StringNew[MaxWidth-3] = L'.';
     StringNew[MaxWidth-2] = L'.';
     StringNew[MaxWidth-1] = CHAR_NULL;
     StringNew[MaxWidth] = CHAR_NULL;
     PrintWidth = MaxWidth;
    }
	
    if (TEXT_ALIGIN_CENTER == Align) {
      UiPrintStringAt (
        ((DimensionsWidth - PrintWidth) / 2) + 1,
        Index + 3,
        StringNew
        );

    }	
    if (TEXT_ALIGIN_LEFT == Align) {
      UiPrintStringAt (
        Start + 2,
        Index + 3,
        StringNew
        );
    }	
    if (TEXT_ALIGIN_RIGHT == Align) {
      UiPrintStringAt (
        End - PrintWidth,
        Index + 3,
        StringNew
        );
    }

    if (StringNew) {
      FreePool (StringNew);
      StringNew = NULL;
    }
    gST->ConOut->SetAttribute (gST->ConOut, FrameColor);
    UiPrintCharAt (Start, Index + 3, Character);
    UiPrintCharAt (End - 1, Index + 3, Character);

    StringTemp = VA_ARG (Marker, CHAR16*);
  }
  if (NULL != StringTemp) {
      UiPrintStringAt (
        Start + 2,
        Index + 3,
        L"..."
        );
  }
  VA_END(Marker);

  //
  // Print Blank Line.
  //
  if (Type != DIALOG_NO_KEY) {
    for (Count = 0; Count < DIALOG_BLANK_LINE; Index++, Count++) {
      gST->ConOut->SetAttribute (gST->ConOut, FrameColor);
      UiPrintCharAt (Start, Index + 3, Character);
      UiPrintCharAt (End - 1, Index + 3, Character);
    }
  }

  if (Type == DIALOG_YESNO) {
  gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
  UiPrintStringAt(NoStart, Bottom - 1, L"[");
  gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
  PrintWidth = UiPrintStringAtWithWidth(NoStart + 1, Bottom - 1, mNoResponse, (MaxWidth*3)/8);
  gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
  UiPrintStringAt(NoStart + 1 + PrintWidth, Bottom - 1, L"]");
  SetRange (&RangeNo, NoStart + 1, PrintWidth, Bottom - 1, 0);

  gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
  UiPrintStringAt(YesStart, Bottom - 1, L"[");
  gST->ConOut->SetAttribute (gST->ConOut, SelectedColor);
  PrintWidth = UiPrintStringAtWithWidth (YesStart + 1, Bottom - 1, mYesResponse, (MaxWidth*3)/8);  
  gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
  UiPrintStringAt(YesStart + 1 + PrintWidth, Bottom - 1, L"]");
  SetRange (&RangeYes, YesStart + 1, PrintWidth, Bottom - 1, 0);
  }

  if (Type == DIALOG_WARNING) {
  gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
  UiPrintStringAt(EnterStart, Bottom - 1, L"[");
  gST->ConOut->SetAttribute (gST->ConOut, SelectedColor);
  PrintWidth = UiPrintStringAtWithWidth (EnterStart + 1, Bottom - 1, EnterButton, MaxWidth/2);
  gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
  UiPrintStringAt(EnterStart + 1 + PrintWidth, Bottom - 1, L"]");
  SetRange (&RangeEnter, EnterStart + 1, PrintWidth, Bottom - 1, 0);
  }

  if (Type == DIALOG_INFO) {
  gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
  UiPrintStringAt(ContinueStart, Bottom - 1, L"[");
  gST->ConOut->SetAttribute (gST->ConOut, SelectedColor);
  PrintWidth = UiPrintStringAtWithWidth(ContinueStart + 1, Bottom - 1, ContinueButton, MaxWidth/2);
  gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
  UiPrintStringAt(ContinueStart + 1 + PrintWidth, Bottom - 1, L"]");
  SetRange (&RangeEnter, ContinueStart + 1, PrintWidth, Bottom - 1, 0);
  }
  
  gST->ConOut->SetAttribute (gST->ConOut, FrameColor);
  Character = BOXDRAW_VERTICAL;
  UiPrintCharAt (Start, Bottom - 1, Character);
  UiPrintCharAt (End - 1, Bottom - 1, Character);

  Character = BOXDRAW_UP_RIGHT;
  UiPrintCharAt (Start, Bottom, Character);
  UiPrintString (Buffer);
  Character = BOXDRAW_UP_LEFT;
  UiPrintChar (Character);

  gST->ConOut->EnableCursor (gST->ConOut, FALSE);
  FreePool (Buffer);
  

  //
  //Corfirm Dialog Process.
  //
  ReturnFlag = FALSE;
  if (Type == DIALOG_YESNO) {
    ResponseArray[0] = L'Y';
    ResponseArray[1] = L'N';
    ArrayIndex = 0;
    do {
      InputType = WaitForUserInput(&Key, &Mouse);
      if (USER_INPUT_MOUSE == InputType) {
          if (IsMouseInRange (&RangeYes, &Mouse)) {
            if (MOUSE_LEFT_CLICK == Mouse.Action) {
              if (ArrayIndex == 1) {
                Key.UnicodeChar = CHAR_NULL;
                Key.ScanCode = SCAN_RIGHT;
                ArrayIndex = 1;
              } else {
                Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
                Key.ScanCode = SCAN_NULL;
                ArrayIndex = 0;
              }
            }
            if (MOUSE_LEFT_DOUBLE_CLICK == Mouse.Action) {
              Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
              Key.ScanCode = SCAN_NULL;
              ArrayIndex = 0;
            }
          }
          if (IsMouseInRange (&RangeNo, &Mouse)) {
            if (MOUSE_LEFT_CLICK == Mouse.Action) {
              if (ArrayIndex == 0) {
                Key.UnicodeChar = CHAR_NULL;
                Key.ScanCode = SCAN_RIGHT;
                ArrayIndex = 0;
              } else {
                Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
                Key.ScanCode = SCAN_NULL;
                ArrayIndex = 1;
              }
            }
            if (MOUSE_LEFT_DOUBLE_CLICK == Mouse.Action) {
              Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
              Key.ScanCode = SCAN_NULL;
              ArrayIndex = 1;
            }
          }
          if (MOUSE_RIGHT_CLICK == Mouse.Action) {
            Key.UnicodeChar = CHAR_NULL;
            Key.ScanCode = SCAN_ESC;
            ArrayIndex = 1;
          }
      }
    
      if (Key.UnicodeChar == CHAR_TAB) {
        Key.UnicodeChar = CHAR_NULL;
        Key.ScanCode = SCAN_LEFT;
      }
      switch (Key.UnicodeChar){
      case CHAR_NULL:
        switch (Key.ScanCode) {
        case SCAN_UP:
        case SCAN_LEFT:
          if(ArrayIndex == 1){
            ArrayIndex = 0;
            gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
            UiPrintStringAtWithWidth(NoStart + 1, Bottom - 1, mNoResponse, (MaxWidth*3)/8);
    
            gST->ConOut->SetAttribute (gST->ConOut, SelectedColor);
            UiPrintStringAtWithWidth(YesStart + 1, Bottom - 1, mYesResponse, (MaxWidth*3)/8);
          } else {
            ArrayIndex = 1;
            gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
            UiPrintStringAtWithWidth(YesStart + 1, Bottom - 1, mYesResponse, (MaxWidth*3)/8);
    
            gST->ConOut->SetAttribute (gST->ConOut, SelectedColor);
            UiPrintStringAtWithWidth(NoStart + 1, Bottom - 1, mNoResponse, (MaxWidth*3)/8);
          }
          break;
    
        case SCAN_DOWN:
        case SCAN_RIGHT:
          if(ArrayIndex == 0){
            ArrayIndex = 1;
            gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
            UiPrintStringAtWithWidth(YesStart + 1, Bottom - 1, mYesResponse, (MaxWidth*3)/8);
    
            gST->ConOut->SetAttribute (gST->ConOut, SelectedColor);
            UiPrintStringAtWithWidth(NoStart + 1, Bottom - 1, mNoResponse, (MaxWidth*3)/8);
          } else {
            ArrayIndex = 0;
            gST->ConOut->SetAttribute (gST->ConOut, DisableColor);
            UiPrintStringAtWithWidth(NoStart + 1, Bottom - 1, mNoResponse, (MaxWidth*3)/8);
    
            gST->ConOut->SetAttribute (gST->ConOut, SelectedColor);
            UiPrintStringAtWithWidth(YesStart + 1, Bottom - 1, mYesResponse, (MaxWidth*3)/8);
          }
          break;
    
        case SCAN_ESC:
          ReturnFlag = FALSE;
          goto EXIT_CONFIRM_DIALOG;
    
        default:
          break;
        }
        break;
    
      case CHAR_CARRIAGE_RETURN:
        if((ResponseArray[ArrayIndex] | UPPER_LOWER_CASE_OFFSET) == (L'Y'| UPPER_LOWER_CASE_OFFSET)){
          gST->ConOut->SetCursorPosition (gST->ConOut, End-2, Bottom + 1);
          gST->ConOut->EnableCursor (gST->ConOut, FALSE);
          ReturnFlag = TRUE;
          goto EXIT_CONFIRM_DIALOG;
        }
        ReturnFlag = FALSE;
        goto EXIT_CONFIRM_DIALOG;
    
      default:
        break;
      }
    } while (TRUE);
  }

  //
  //Warning Dialog Process.
  //
  ReturnFlag = FALSE;
  if (Type == DIALOG_WARNING || Type == DIALOG_INFO) {
    do {
      InputType = WaitForUserInput(&Key, &Mouse);
      if (USER_INPUT_MOUSE == InputType) {
        if (IsMouseInRange (&RangeEnter, &Mouse)) {
          if (MOUSE_LEFT_CLICK == Mouse.Action || MOUSE_LEFT_DOUBLE_CLICK == Mouse.Action) {
            Key.UnicodeChar = CHAR_CARRIAGE_RETURN;
            Key.ScanCode = SCAN_NULL;
          }
        }
        if (MOUSE_RIGHT_CLICK == Mouse.Action) {
          Key.UnicodeChar = CHAR_NULL;
          Key.ScanCode = SCAN_ESC;
        }
      }
      if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        gST->ConOut->SetCursorPosition (gST->ConOut, End-2, Bottom + 1);
        gST->ConOut->EnableCursor (gST->ConOut, FALSE);
        ReturnFlag = TRUE;
        goto EXIT_CONFIRM_DIALOG;
      } else if (Key.ScanCode == SCAN_ESC ) {
        gST->ConOut->SetCursorPosition (gST->ConOut, End-2, Bottom + 1);
        gST->ConOut->EnableCursor (gST->ConOut, FALSE);
        ReturnFlag = FALSE;
        goto EXIT_CONFIRM_DIALOG;
      }

    } while (TRUE);
  }

  EXIT_CONFIRM_DIALOG:  

  if (!bSetButtonTitle_YesNo) {
    if (mYesResponse) { 
      FreePool (mYesResponse);
    }
    if (mNoResponse) { 
      FreePool (mNoResponse);
    }
  } else {
    if (gYesButtonTitle) {
      FreePool (gYesButtonTitle );
      gYesButtonTitle = NULL;
    }
    if (gNoButtonTitle) {
      FreePool (gNoButtonTitle );
      gNoButtonTitle = NULL;
    }
  }

  if (!bSetButtonTitle_Warning) {
    FreePool (EnterButton);
  } else {
    if (gEnterButton) {
      FreePool (gEnterButton );
      gEnterButton = NULL;
    }
  }

  if (!bSetButtonTitle_Info) {
    FreePool (ContinueButton);
  } else {
    if (gContinueButton) {
      FreePool (gContinueButton );
      gContinueButton = NULL;
    }
  }

  if(Type != DIALOG_NO_KEY){
    DEBUG((EFI_D_ERROR, "Clear 1 gBltBuffer:%x,gLastDialogType:%x,Type:%x\n", gBltBuffer,gLastDialogType,Type));
    UiStoreScreen(gBS, FALSE);
  }

  bSetButtonTitle_YesNo = FALSE;
  bSetButtonTitle_Warning = FALSE;
  bSetButtonTitle_Info = FALSE;

  gST->ConOut->SetAttribute (gST->ConOut, OriginalAttribute);
  gST->ConOut->EnableCursor (gST->ConOut, OriginalCursorVisible);
  
  return ReturnFlag;
}

/**
  Wait for user Input by Key or Mouse.

  @param Key         The key which is pressed by user.
  @param Mouse      The Mouse which is pressed by user.

  @retval USER_INPUT_TYPE  The Input type

**/
USER_INPUT_TYPE
WaitForUserInput (
  OUT  EFI_INPUT_KEY    *Key,
  OUT  MOUSE_ACTION_INFO *Mouse
  )
{
  EFI_STATUS  Status;
  UINTN       Index;
  EFI_EVENT   WaitList[2];
  EFI_SETUP_MOUSE_PROTOCOL    *SetupMouse;

  if (Key == NULL || Mouse == NULL) {
    return USER_INPUT_MAX;
  }

  WaitList[0] =  gST->ConIn->WaitForKey;

  SetupMouse = NULL;
  Status = gBS->LocateProtocol (
                  &gEfiSetupMouseProtocolGuid,
                  NULL,
                  (VOID **) &SetupMouse
                  );
  if (SetupMouse && PcdGetBool(PcdSetupMouseEnable)) {
    WaitList[1] =  SetupMouse->WaitForMouse;
  }
	
  while (TRUE) {

    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
    if (!EFI_ERROR (Status)) {
      return USER_INPUT_KEY;
    }

    if (Status != EFI_NOT_READY) {
      continue;
    }

    if (SetupMouse && PcdGetBool(PcdSetupMouseEnable)) {
      Status = gBS->WaitForEvent (2, WaitList, &Index);
    } else {
      gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index);
    }

    if (Index == 1  && PcdGetBool(PcdSetupMouseEnable)) {
      SetupMouse->GetData(Mouse);
      return USER_INPUT_MOUSE;
    }	
  }

  return USER_INPUT_MAX;
}

/**
  Check whether mouse in the range.

  @param Range      The Range of a item.
  @param Mouse      The Mouse which is pressed by user.

  @retval BOOLEAN  The Input type

**/
BOOLEAN 
IsMouseInRange (
  IN  MOUSE_CURSOR_RANGE    *Range,
  IN  MOUSE_ACTION_INFO    *Mouse
)
{
  if (Range == NULL || Mouse == NULL) {
    return FALSE;
  }

  if (Range->StartX <= Mouse->Column && 
    Range->EndX >  Mouse->Column &&
    Range->StartY <= Mouse->Row &&
    Range->EndY >= Mouse->Row ) {

    return TRUE;
  }

  return FALSE;
}

/**
  Set range of item.

  @param Range      The Range of a item.
  @param StartColum
  @param Length
  @param StartRow
  @param Heigth
  
  @retval EFI_STATUS

**/
EFI_STATUS 
SetRange (
  IN  MOUSE_CURSOR_RANGE    *Range,
  IN  UINTN    StartColum,
  IN  UINTN    Length,
  IN  UINTN    StartRow,
  IN  UINTN    Height 
)
{
  if (Range == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Range->StartX = StartColum;
  Range->EndX = StartColum + Length;

  Range->StartY = StartRow;
  Range->EndY = StartRow + Height;

  return EFI_SUCCESS;
}

/**
  Constructor of Customized Display Library Instance.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
SetupUiLibConstructor (
  IN      EFI_HANDLE                ImageHandle,
  IN      EFI_SYSTEM_TABLE          *SystemTable
  )
{
  //InitializeLanguage(TRUE);

  mSetupUiStringHandle = HiiAddPackages (&gSetupUiLibGuid, ImageHandle, SetupUiLibStrings, NULL);
  ASSERT (mSetupUiStringHandle != NULL);

  InitializeLibStrings();

  return EFI_SUCCESS;
}

/**
  Destructor of Customized Display Library Instance.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The destructor completed successfully.
  @retval Other value   The destructor did not complete successfully.

**/
EFI_STATUS
EFIAPI
SetupUiLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{  
  HiiRemovePackages(mSetupUiStringHandle);
  
  FreeLibStrings ();

  if (gBltBuffer) {
    FreePool (gBltBuffer);
    gBltBuffer = NULL;
  }
  
  if (gYesButtonTitle) {
    FreePool (gYesButtonTitle );
    gYesButtonTitle = NULL;
  }

  if (gNoButtonTitle) {
    FreePool (gNoButtonTitle );
    gNoButtonTitle = NULL;
  }
  
  if (gEnterButton) {
    FreePool (gEnterButton );
    gEnterButton = NULL;
  }

  if (gContinueButton) {
    FreePool (gContinueButton );
    gContinueButton = NULL;
  }

  return EFI_SUCCESS;
}


/**
  Restore Backgroud before creating it and Free memory.
  This service  is only for DIALOG_NO_KEY dialog now.

**/
BOOLEAN
UiClearDialog (
  IN DIALOG_TYPE    Type
  ) 
{
  BOOLEAN    ReturnFlag;
  
  ReturnFlag = FALSE;
  DEBUG((EFI_D_ERROR, "Clear 2 gBltBuffer:%x,gLastDialogType:%x,Type:%x\n", gBltBuffer,gLastDialogType,Type));
  if (Type == gLastDialogType) {
    if (gLastDialogType == DIALOG_NO_KEY && gBltBuffer != NULL) {
      UiStoreScreen(gBS, FALSE);
      ReturnFlag = TRUE;
    }
  }

  return ReturnFlag;
}

