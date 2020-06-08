/** @file
  Control (which make up panel) related Functions for H2O display engine driver.

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "H2ODisplayEngineLocalText.h"
#include "LTDESpecificQuestionBehavior.h"

/**
 Get current resolution of screen

 @param [out] X           The horizontal value of screen
 @param [out] Y           The vertical value of screen

 @retval EFI_SUCCESS      Get screen resolution successfully
 @retval Other            Fail to get screen resolution

**/
EFI_STATUS
GetScreenMaxXY (
  OUT    UINT32                              *X,
  OUT    UINT32                              *Y
  )
{
  return DEConOutQueryModeWithoutModeNumer (mDEPrivate, X, Y);
}

/**
 Combine the region of multiple input fields

 @param [out] ResultField       Result of combined field
 @param  ...                    A variable argument list containing series of
                                field, the last field must be NULL.

 @retval EFI_SUCCESS            Get screen resolution successfully
 @retval EFI_INVALID_PARAMETER  ResultField is NULL
 @retval EFI_NOT_FOUND          There is no input field

**/
STATIC
EFI_STATUS
CombineField (
  OUT    RECT                                *ResultField,
  ...
  )
{
  VA_LIST                                    Args;
  RECT                                       *Field;

  if (ResultField == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  VA_START (Args, ResultField);

  Field = VA_ARG (Args, RECT *);
  if (Field == NULL) {
    return EFI_NOT_FOUND;
  }

  CopyRect (ResultField, Field);

  while (TRUE) {
    Field = VA_ARG (Args, RECT *);
    if (Field == NULL) {
      break;
    }

    ResultField->left   = MIN (ResultField->left  , Field->left);
    ResultField->top    = MIN (ResultField->top   , Field->top);
    ResultField->right  = MAX (ResultField->right , Field->right);
    ResultField->bottom = MAX (ResultField->bottom, Field->bottom);
  }

  return EFI_SUCCESS;
}

/**
 Get formatted string which follow the input line width

 @param [in]  InputString       The input string
 @param [in]  LineWidth         Line width which output string should follow
 @param [out] LineNum           Line number of formatted string
 @param [out] LineOffsetArray   Array of input string offset for each line of output formatted string
 @param [out] OutputString      Pointer to output formatted string

 @retval EFI_SUCCESS            Get screen resolution successfully
 @retval EFI_INVALID_PARAMETER  InputString, LineNum or OutputString is NULL or LineWidth is zero

**/
STATIC
EFI_STATUS
GetLineByWidth (
  IN      CHAR16                      *InputString,
  IN      UINT32                      LineWidth,
  OUT     UINT32                      *LineNum,
  OUT     UINT32                      **LineOffsetArray,
  OUT     CHAR16                      **OutputString
  )
{
  BOOLEAN                             IsWidth;
  BOOLEAN                             Finished;
  BOOLEAN                             AdjustStr;
  UINT32                              AdjCount;
  UINT32                              CharCount;
  UINT32                              TempCharCount;
  UINT32                              MaxLineNum;
  UINT32                              BufferSize;
  CHAR16                              *StringBuffer;
  UINT32                              InputStrOffset;
  UINT32                              OutputStrOffset;
  UINT32                              StrWidth;
  UINT32                              IncrementValue;
  CHAR16                              Character;
  UINT32                              *OffsetArray;
  UINT32                              OffsetArrayCount;

  if (InputString == NULL || LineNum == NULL || OutputString == NULL || LineOffsetArray == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (LineWidth < 1) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Try to calculate the maximun using memory size
  //
  MaxLineNum    = (UINT32) StrSize (InputString) / LineWidth;
  BufferSize    = MaxLineNum * 4 + (UINT32) StrSize (InputString);
  *OutputString = AllocateZeroPool (BufferSize);
  if (*OutputString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  IsWidth          = FALSE;
  Finished         = FALSE;
  InputStrOffset   = 0;
  OutputStrOffset  = 0;
  *LineNum         = 0;
  IncrementValue   = 1;
  OffsetArrayCount = 10;
  OffsetArray      = AllocateZeroPool (OffsetArrayCount * sizeof(UINT32));
  if (OffsetArray == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  while (!Finished) {
    if (IsWidth) {
      (*OutputString)[OutputStrOffset] = WIDE_CHAR;
      OutputStrOffset++;
    }

    StrWidth  = 0;
    CharCount = 0;
    AdjustStr = FALSE;

    do {
      //
      // Advance to the null-terminator or to the first width directive
      //
      Character = InputString[InputStrOffset + CharCount];

      while ((Character != CHAR_CARRIAGE_RETURN)&&
             (Character != NARROW_CHAR) &&
             (Character != WIDE_CHAR) &&
             (Character != 0) &&
             (StrWidth < LineWidth)) {
        CharCount++;
        StrWidth += IncrementValue;
        if (IncrementValue == 1 && ConsoleLibGetGlyphWidth (Character) == 2) {
          //
          // If character belongs to wide glyph, set the right glyph offset.
          //
          StrWidth++;
        }

        Character = InputString[InputStrOffset + CharCount];
      }

      if (StrWidth > LineWidth) {
        CharCount--;
        StrWidth = StrWidth - IncrementValue;
        AdjustStr = TRUE;
        break;
      } else if (StrWidth == LineWidth) {
        break;
      }
      //
      // We encountered a narrow directive - strip it from the size calculation since it doesn't get printed
      // and also set the flag that determines what we increment by.(if narrow, increment by 1, if wide increment by 2)
      //
      if (InputString[InputStrOffset + CharCount] == NARROW_CHAR) {
        //
        // Skip to the next character
        //
        CharCount++;
        IncrementValue = 1;
        IsWidth = FALSE;
      } else if (InputString[InputStrOffset + CharCount] == WIDE_CHAR) {
        //
        // Skip to the next character
        //
        CharCount++;
        IncrementValue = 2;
        IsWidth = TRUE;
      }
    } while (InputString[InputStrOffset + CharCount] != 0 && InputString[InputStrOffset + CharCount] != CHAR_CARRIAGE_RETURN);

    //
    // If space,narrow and wide character before end of the string, increase index to skip these characters
    //
    AdjCount      = 0;
    while (InputString[InputStrOffset + CharCount + AdjCount] != 0) {
      if ((InputString[InputStrOffset + CharCount + AdjCount] != WIDE_CHAR) &&
          (InputString[InputStrOffset + CharCount + AdjCount] != NARROW_CHAR) &&
          (InputString[InputStrOffset + CharCount + AdjCount] != CHAR_SPACE)) {
        break;
      }
      AdjCount++;
    }

    if (InputString[InputStrOffset + CharCount + AdjCount] == 0) {
      Finished = TRUE;
    } else {
      if (StrWidth == LineWidth || AdjustStr) {
        //
        // Rewind the string from the maximum size until we see a space to break the line
        //
        if (!IsWidth) {
          TempCharCount = CharCount;

          for (; (InputString[InputStrOffset + CharCount] != CHAR_SPACE) && (CharCount != 0); CharCount--, StrWidth = StrWidth - IncrementValue)
            ;
          if (CharCount == 0) {
            CharCount = TempCharCount;
          }
        }
      }
    }

    if (BufferSize < (UINT32) (OutputStrOffset + (CharCount + 2) * sizeof (CHAR16))) {
       BufferSize  *= 2;
       StringBuffer = AllocateZeroPool (BufferSize);
       if (StringBuffer == NULL) {
         return EFI_OUT_OF_RESOURCES;
       }
       CopyMem (StringBuffer, *OutputString, OutputStrOffset * sizeof (CHAR16));
       SafeFreePool ((VOID **) OutputString);
       *OutputString = StringBuffer;
    }

    CopyMem (&(*OutputString)[OutputStrOffset], &InputString[InputStrOffset], CharCount * sizeof (CHAR16));

    //
    // Increase offset to add null for end of one line
    //
    OutputStrOffset += (CharCount + 1);
    //
    // if not last string, we need increment the index to the first non-space character
    //
    if (!Finished) {
      while (InputString[InputStrOffset + CharCount] != 0) {
        if ((InputString[InputStrOffset + CharCount] != WIDE_CHAR) &&
            (InputString[InputStrOffset + CharCount] != NARROW_CHAR) &&
            (InputString[InputStrOffset + CharCount] != CHAR_SPACE) &&
            (InputString[InputStrOffset + CharCount] != CHAR_CARRIAGE_RETURN)) {
          break;
        }
        CharCount++;
      }
    }

    if (*LineNum >= OffsetArrayCount) {
      OffsetArray = ReallocatePool (
                      OffsetArrayCount * sizeof(UINT32),
                      (OffsetArrayCount + 10) * sizeof(UINT32),
                      OffsetArray
                      );
      if (OffsetArray == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      OffsetArrayCount += 10;
    }
    OffsetArray[*LineNum] = InputStrOffset;
    InputStrOffset        = InputStrOffset + CharCount;
    (*LineNum)++;
  }

  *LineOffsetArray = AllocateCopyPool ((*LineNum) * sizeof(UINT32), OffsetArray);
  FreePool (OffsetArray);
  if (*LineOffsetArray == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

/**
 Get result filed data from input source field data and input control info list

 @param [in,out] ResultField        Output field data
 @param [in]  SourceField           Input source field data
 @param [in]  UsedMenu              Used contorl info list
 @param [in]  MenuCount             Number of Used contorl info

 @retval EFI_SUCCESS                Get result filed data successfully
 @retval EFI_INVALID_PARAMETER      ResultField, SourceField or UsedMenu is NULL or MenuCount is zero

**/
EFI_STATUS
GetClearField (
  IN OUT RECT                                    *ResultField,
  IN     RECT                                    *SourceField,
  IN     H2O_CONTROL_INFO                        *UsedMenu,
  IN     UINT32                                  MenuCount,
  IN     UINT32                                  BorderLineWidth
  )
{
  RECT                                           OutField;
  RECT                                           MenuField;
  UINT32                                         Index;
  INT32                                          CutUsedHight;

  if (ResultField == NULL || SourceField == NULL || UsedMenu == NULL || MenuCount == 0) {
    return EFI_INVALID_PARAMETER;
  }

  CopyRect (&OutField, SourceField);
  OutField.left   += BorderLineWidth;
  OutField.top    += BorderLineWidth;
  OutField.right  -= BorderLineWidth;
  OutField.bottom -= BorderLineWidth;
  if (UsedMenu == NULL || MenuCount == 0) {
    CopyRect (ResultField, &OutField);
    return EFI_SUCCESS;
  }

  SetRectEmpty (&MenuField);
  for (Index = 0; Index < MenuCount; Index ++) {
    if (UsedMenu[Index].ControlField.left   >= (UsedMenu[Index].ParentPanel->PanelField.left + UsedMenu[Index].ParentPanel->PanelRelField.left  ) &&
        UsedMenu[Index].ControlField.right  <= (UsedMenu[Index].ParentPanel->PanelField.left + UsedMenu[Index].ParentPanel->PanelRelField.right ) &&
        UsedMenu[Index].ControlField.top    >= (UsedMenu[Index].ParentPanel->PanelField.top  + UsedMenu[Index].ParentPanel->PanelRelField.top   ) &&
        UsedMenu[Index].ControlField.bottom <= (UsedMenu[Index].ParentPanel->PanelField.top  + UsedMenu[Index].ParentPanel->PanelRelField.bottom)
        ) {
      CombineField (&MenuField, &MenuField, &UsedMenu[Index].ControlField, NULL);
    }
  }

  CutUsedHight = MenuField.bottom - MenuField.top + 1;

  if (OutField.top + CutUsedHight <= OutField.bottom) {
    OutField.top += CutUsedHight;
  }

  CopyRect (ResultField, &OutField);

  return EFI_SUCCESS;
}

/**
 Get string filed data from input panel field data and input string

 @param [in,out] ResultField         Output string field data
 @param [in]     PanelField          Input panel field data
 @param [in,out] LimitInField        Limit ield data
 @param [in]     Text                Pointer to input string

 @retval EFI_SUCCESS                Get string filed data successfully
 @retval Other                      Fail by calling GetLineByWidth ()

**/
EFI_STATUS
GetStringField (
  IN OUT RECT                                *ResultField,
  IN     RECT                                *PanelField,
  IN OUT RECT                                *LimitInField OPTIONAL,
  IN     CHAR16                              *Text
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     Y;
  UINT32                                     X;
  UINT32                                     LineWidth;
  UINT32                                     SeparateStringNum;
  UINT32                                     *SeparateStringOffset;
  CHAR16                                     *SeparateStrings;
  CHAR16                                     *CurrentString;
  UINT32                                     Index;
  UINT32                                     MaxStringWidth;
  UINT32                                     StringWidth;

  //
  // Get Text Row, Col and Width
  //
  // |--------------------------> Org LineWidth <-------------------------------------|
  //  --------------------------------------------------------------------------------
  // |     ? char     |---------> UserLineWidth <--------|      (? + 1) chars         |
  //  --------------------------------------------------------------------------------
  // ^(Field->Left) ^ (LimitInField->Left)           ^ (LimitInField->Right)       ^(Field->Right)
  //
  Y         = PanelField->top;
  X         = PanelField->left;
  LineWidth = PanelField->right - PanelField->left;

  if (LimitInField != NULL) {
    Y += LimitInField->top;
    X += LimitInField->left;

    if (LimitInField->right > LimitInField->left) {
      LineWidth = LimitInField->right - LimitInField->left;
    } else {
      LineWidth -= LimitInField->left;
    }
  }

  //
  // Get string list (Segment Text with LineWidth and count number of lines)
  //
  SeparateStrings = NULL;
  Status = GetLineByWidth (
             Text,
             LineWidth,
             &SeparateStringNum,
             &SeparateStringOffset,
             &SeparateStrings
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (LimitInField != NULL) {
    //
    // Set LimitInField
    //
    CurrentString  = SeparateStrings;
    MaxStringWidth = 0;
    for (Index = 0; Index < SeparateStringNum; Index ++) {
      StringWidth    = ((UINT32) GetStringWidth (CurrentString) / sizeof (CHAR16)) - 1;
      CurrentString += (StrLen (CurrentString) + 1);
      MaxStringWidth = MAX (MaxStringWidth, StringWidth);
    }

    LimitInField->right  = MaxStringWidth;
    LimitInField->bottom = LimitInField->top + SeparateStringNum - 1;
  }

  ResultField->left   = X;
  ResultField->right  = X + LineWidth - 1;
  ResultField->top    = Y;
  ResultField->bottom = Y + SeparateStringNum - 1;

  SafeFreePool ((VOID **) &SeparateStrings);
  SafeFreePool ((VOID **) &SeparateStringOffset);

  return EFI_SUCCESS;
}

/**
 Get cut control data

 @param[in] Control                 Control info
 @param[in] LimitField              Limit field data
 @param[out] ResultControlField     Result field data
 @param[out] ResultControlString    Result control string

 @retval EFI_SUCCESS                Get control field data successfully
 @retval EFI_INVALID_PARAMETER      Control, LimitField, ResultControlField or ResultControlString is NULL
 @retval EFI_NOT_FOUND              ControlField is in LimitField
 @retval EFI_OUT_OF_RESOURCES       Get formatted string fail by call GetLineByWidth () function

**/
STATIC
EFI_STATUS
GetCutControl (
  IN     H2O_CONTROL_INFO                    *Control,
  IN     RECT                                *LimitField,
  OUT    RECT                                *ResultControlField,
  OUT    CHAR16                              **ResultControlString
  )
{
  EFI_STATUS                                 Status;

  CHAR16                                     *TempString;
  UINT32                                     LimitWidth;
  UINT32                                     LimitHieght;
  CHAR16                                     *SeparateStrings;
  UINT32                                     *SeparateStringOffset;
  UINT32                                     SeparateStringNum;

  if (Control == NULL || LimitField == NULL || ResultControlField == NULL || ResultControlString == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Control->ControlField.left   >= LimitField->left  &&
      Control->ControlField.right  <= LimitField->right &&
      Control->ControlField.top    >= LimitField->top   &&
      Control->ControlField.bottom <= LimitField->bottom
      ) {
    //
    // Cut this control is not unnecessary
    //
    return EFI_NOT_FOUND;
  }

  LimitWidth  = LimitField->right - LimitField->left - 2;
  LimitHieght = LimitField->bottom - LimitField->top + 1;

  Status = GetLineByWidth (
             Control->Text.String,
             LimitWidth,
             &SeparateStringNum,
             &SeparateStringOffset,
             &SeparateStrings
             );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  TempString = AllocateCopyPool (StrSize (Control->Text.String), Control->Text.String);
  if (TempString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (SeparateStringNum > LimitHieght) {
    TempString[SeparateStringOffset[LimitHieght] - 1] = '\0';
  }
  CopyMem (ResultControlField, LimitField, sizeof (RECT));
  ResultControlField->right -= 2;
  *ResultControlString = TempString;

  SafeFreePool ((VOID **) &SeparateStringOffset);

  return EFI_SUCCESS;
}

/**
 Clear field

 @param [in] Attribute             Attribute for background and foreground colors for the OutputString ()
 @param [in] ...                   field data followed by following field data. A NULL terminates the list.

 @return The number of character is cleared

**/
UINT32
ClearField (
  IN  CONST UINT32                          Attribute,
  ...
  )
{
  VA_LIST                                    Args;
  RECT                                       *Field;
  UINT32                                     Count;

  INT32                                      Index;
  INT32                                      ClearWidth;
  CHAR16                                     *ClearString;
  INT32                                      ScreenRow;
  INT32                                      ScreenColumn;

  //
  // Set Field Attribute
  //
  DEConOutSetAttribute (mDEPrivate, (UINTN)Attribute);
  ScreenRow    = (INT32) (PcdGet32 (PcdDisplayEngineLocalTextRow) - 1);
  ScreenColumn = (INT32) (PcdGet32 (PcdDisplayEngineLocalTextColumn) - 1);
  //
  // Clear all Fields
  //
  VA_START (Args, Attribute);
  Count = 0;
  while (TRUE) {
    //
    // Get one of Field
    //
    Field = VA_ARG(Args, RECT *);
    if (Field == NULL) {
      break;
    }

    //
    // Clear one of Field
    //
    ClearWidth = Field->right - Field->left + 1;
    ClearString = AllocateZeroPool (sizeof (CHAR16) * (ClearWidth + 1));
    if (ClearString == NULL) {
      return 0;
    }
    for (Index = 0; Index < ClearWidth; Index ++) {
      ClearString[Index] = ' ';
    }
    for (Index = Field->top; Index <= Field->bottom; Index ++) {
      Count += ClearWidth;
      if ((Index == ScreenRow) && (Field->left + ClearWidth - 1 == ScreenColumn)) {
        ClearString[ClearWidth - 1] = CHAR_NULL;
      }

      DisplayString (Field->left, Index, ClearString);
    }
    SafeFreePool ((VOID **) &ClearString);
  }
  VA_END (Args);

  return Count;
}

/**
 Clear field difference between border

 @param [in] Attribute             Attribute for background and foreground colors for the OutputString ()
 @param [in] ...                   field data followed by following field data. A NULL terminates the list.

 @return The number of character is cleared

**/
UINT32
ClearContentField (
  IN     UINT32                              Attribute,
  ...
  )
{
  VA_LIST                                    Args;
  RECT                                       *Field;
  UINT32                                     Count;

  INT32                                      Index;
  INT32                                      ClearWidth;
  CHAR16                                     *ClearString;

  //
  // Set Field Attribute
  //
  DEConOutSetAttribute (mDEPrivate, Attribute);

  //
  // Clear all Fields
  //
  VA_START (Args, Attribute);
  Count = 0;
  while (TRUE) {
    //
    // Get one of Field
    //
    Field = VA_ARG(Args, RECT *);
    if (Field == NULL) {
      break;
    }

    //
    // Clear one of Field
    //
    ASSERT (Field->right > Field->left);
    if (Field->right <= Field->left) {
      continue;
    }
    ClearWidth  = Field->right - Field->left - 1;
    ClearString = AllocateZeroPool (sizeof (CHAR16) * (ClearWidth + 1));
    if (ClearString == NULL) {
      return 0;
    }
    for (Index = 0; Index < ClearWidth; Index ++) {
      ClearString[Index] = ' ';
    }
    for (Index = Field->top + 1; Index <= Field->bottom - 1; Index ++) {
      Count += ClearWidth;
      DisplayString (Field->left + 1, Index, ClearString);
    }
    SafeFreePool ((VOID **) &ClearString);
  }
  VA_END (Args);

  return Count;
}

/**
 Clear string in a region

 @param [in] Attribute             Attribute for background and foreground colors for the OutputString ()
 @param [in] Col                   Start column of clear string
 @param [in] Row                   Start row of clear string
 @param [in] LineWidth             String width to be clear

 @return The number of character is cleared

**/
UINT32
ClearString (
  IN     UINT32                          Attribute,
  IN     UINT32                          Col,
  IN     UINT32                          Row,
  IN     UINT32                          LineWidth
  )
{
  RECT                                   Field;

  Field.left   = Col;
  Field.right  = Col + LineWidth - 1;
  Field.top    = Row;
  Field.bottom = Row;

  //
  // Clear all Fields
  //
  return ClearField (Attribute, &Field, NULL);
}

/**
 Display border line

 @param [in] Attribute             Attribute for background and foreground colors for the OutputString ()
 @param [in] ...                   field data followed by following field data. A NULL terminates the list.

 @return The number of field is displayed

**/
UINT32
DisplayBorderLine (
  IN     UINT32                          Attribute,
  ...
  )
{
  VA_LIST                                Args;
  RECT                                   *Field;
  INT32                                  Width;
  INT32                                  Height;
  INT32                                  X;
  INT32                                  Y;
  UINT32                                 Count;
  CHAR16                                 Character[2];
  CHAR16                                 *String;
  UINT32                                 StringCount;

  //
  // Set Border Line Attribute
  //
  DEConOutSetAttribute (mDEPrivate, Attribute);

  //
  // Display Border Line for all Fields
  //
  Character[1] = CHAR_NULL;
  VA_START (Args, Attribute);
  Count = 0;
  while (TRUE) {
    //
    // Get one of Field
    //
    Field = VA_ARG( Args, RECT *);
    if (Field == NULL) {
      break;
    }
    Count ++;

    //
    // Display Border Line for one of Field
    //
    Width  = Field->right  - Field->left;
    Height = Field->bottom - Field->top;
    String = AllocateZeroPool (Width * sizeof(CHAR16));
    if (String == NULL) {
      return 0;
    }
    StringCount = 0;

    if (Width < 0 || Height < 0) {
      continue;
    } else if (Width == 0 && Height >= 1) {
      Character[0] = BOXDRAW_DOWN_HORIZONTAL;
      DisplayString (Field->left, Field->top, Character);
      Character[0] = BOXDRAW_UP_HORIZONTAL;
      DisplayString (Field->left, Field->bottom, Character);
    } else if (Width >= 1 && Height == 0) {
      Character[0] = BOXDRAW_VERTICAL_RIGHT;
      DisplayString (Field->left, Field->top, Character);
      Character[0] = BOXDRAW_VERTICAL_LEFT;
      DisplayString (Field->right, Field->top, Character);
    } else if (Width >= 1 && Height >= 1) {
      Character[0] = BOXDRAW_DOWN_RIGHT;
      DisplayString (Field->left, Field->top, Character);
      Character[0] = BOXDRAW_UP_RIGHT;
      DisplayString (Field->left, Field->bottom, Character);
      Character[0] = BOXDRAW_DOWN_LEFT;
      DisplayString (Field->right, Field->top, Character);
      Character[0] = BOXDRAW_UP_LEFT;
      DisplayString (Field->right, Field->bottom, Character);
    }

    if (Width > 1) {
      for (X = Field->left + 1; X <= Field->right - 1; X++) {
        String[StringCount++] = BOXDRAW_HORIZONTAL;
      }
      DisplayString (Field->left + 1, Field->top   , String);
      DisplayString (Field->left + 1, Field->bottom, String);
    }
    if (Height > 1) {
      for (Y = Field->top + 1; Y <= Field->bottom - 1; Y++) {
        Character[0] = BOXDRAW_VERTICAL;
        DisplayString (Field->left, Y, Character);
        DisplayString (Field->right, Y, Character);
      }
    }
    SafeFreePool ((VOID **) &String);
  }
  VA_END (Args);

  return Count;
}

/**
 Display panel

 @param [in] Panel                   Target panel info

 @retval EFI_SUCCESS                Display panel successfully

**/
EFI_STATUS
DisplayPanel (
  IN H2O_PANEL_INFO                         *Panel,
  IN UINT32                                  PseudoClass
  )
{
  INT32                                      Index;
  INT32                                      ControlMinY;
  INT32                                      ControlMaxY;
  INT32                                      TotalControlHeight;
  INT32                                      ScrollBarHeight;
  INT32                                      ScrollBarStartY;
  INT32                                      ScrollBarEndY;
  INT32                                      SelectStartY;
  INT32                                      SelectEndY;
  UINT32                                     Attribute;

  Attribute = 0;
  GetPanelColorAttribute (Panel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, PseudoClass, &Attribute);

  //
  // Clear Field and Display Border Line
  //
  if (Panel->PanelType == H2O_PANEL_TYPE_QUESTION) {
    if ((GetBorderWidth(Panel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL)) > 0) {
      ClearField (Attribute, &Panel->PanelField, NULL);
      DisplayBorderLine (Attribute, &Panel->PanelField, NULL);
    }

    //
    // Display Scroll Bar
    //
    if (Panel->WithScrollBarVertical) {
      for (Index = 0, ControlMinY = 0, ControlMaxY = 0; Index < (INT32) Panel->ControlList.Count; Index ++) {
        ControlMinY = MIN (ControlMinY, Panel->ControlList.ControlArray[Index].ControlField.top);
        ControlMaxY = MAX (ControlMaxY, Panel->ControlList.ControlArray[Index].ControlField.bottom);
      }
      TotalControlHeight = ControlMaxY - ControlMinY;
      //
      // Get Scroll Bar Location
      //
      ScrollBarStartY = Panel->PanelField.top;
      ScrollBarEndY   = Panel->PanelField.bottom;
      if ((GetBorderWidth(Panel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL)) > 0) {
        ScrollBarStartY += 1;
        ScrollBarEndY   -= 1;
      }
      if (Panel->PanelType == H2O_PANEL_TYPE_QUESTION) {
        //
        // Skip Title
        //
        ScrollBarStartY += 2;
      }
      ScrollBarHeight = ScrollBarEndY - ScrollBarStartY;
      //
      // Get Select Scroll Bar Location
      //
      SelectStartY = Panel->PanelRelField.top * ScrollBarHeight / TotalControlHeight;
      SelectEndY = Panel->PanelRelField.bottom * ScrollBarHeight / TotalControlHeight;
      for (Index = 0; Index < ScrollBarHeight; Index ++) {
        //
        // Set Scroll Bar Attribute
        //
        if (Index >= SelectStartY && Index <= SelectEndY) {
          DEConOutSetAttribute (mDEPrivate, EFI_BACKGROUND_CYAN);
        } else {
          DEConOutSetAttribute (mDEPrivate, EFI_BACKGROUND_BLACK);
        }
        //
        // Display Scroll Bar
        //
        DisplayString (Panel->PanelField.right - 2, ScrollBarStartY + Index, L"  ");
      }
    }
  }
  return EFI_SUCCESS;
}

/**
 Display control info list

 @param [in] IsCentered              Flag to decide to display in center
 @param [in] ButtonStartEndChar      Flag to decide to display string within []
 @param [in] ControlCount            Control list count
 @param [in] ControlArray            Control list

 @retval EFI_SUCCESS                Display control successfully

**/
EFI_STATUS
DisplayControls (
  IN CONST BOOLEAN                           IsCentered,
  IN       BOOLEAN                           ButtonStartEndChar,
  IN       UINT32                            ControlCount,
  IN       H2O_CONTROL_INFO                  *ControlArray
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     Attribute;
  RECT                                       ClearControlField;
  UINT32                                     SeparateStringIndex;
  UINT32                                     SeparateStringNum;
  UINT32                                     *SeparateStringOffset;
  CHAR16                                     *SeparateStrings;
  CHAR16                                     *TempString;
  UINT32                                     StringWidth;
  UINT32                                     FieldWidth;
  UINT32                                     FieldHeight;
  UINT32                                     CenteredStartX;
  INT32                                      CenteredStartY;
  UINT32                                     Index;
  CHAR16                                     *String;
  H2O_CONTROL_INFO                           Control;
  INT32                                      BorderLineWidth;

  if (ControlCount == 0 || ControlArray == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  BorderLineWidth = GetBorderWidth (ControlArray[0].ParentPanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);

  for (Index = 0; Index < ControlCount; Index++) {
    CopyMem (&Control, &ControlArray[Index], sizeof (H2O_CONTROL_INFO));

    //
    // Calculate the absolute position of each control field
    //
    OffsetRect (&Control.ControlField, Control.ParentPanel->PanelField.left    , Control.ParentPanel->PanelField.top);
    OffsetRect (&Control.ControlField, -Control.ParentPanel->PanelRelField.left, -Control.ParentPanel->PanelRelField.top);
    OffsetRect (&Control.ControlField, BorderLineWidth                         , BorderLineWidth);

    Attribute = 0;
    GetPanelColorAttribute (Control.ParentPanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, Control.ControlStyle.PseudoClass, &Attribute);

    //
    // Separate string by filed width
    //
    FieldWidth = Control.ControlField.right  - Control.ControlField.left + 1;
    String     = CatSPrint (NULL, (ButtonStartEndChar ? L"[%s]" : L"%s"), Control.Text.String);

    Status = GetLineByWidth (
               String,
               FieldWidth,
               &SeparateStringNum,
               &SeparateStringOffset,
               &SeparateStrings
               );
    SafeFreePool ((VOID **) &String);
    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Display Control
    //
    DEConOutSetAttribute (mDEPrivate, Attribute);

    TempString     = SeparateStrings;
    CenteredStartX = 0;
    CenteredStartY = 0;
    FieldHeight    = Control.ControlField.bottom - Control.ControlField.top  + 1;
    if (SeparateStringNum > FieldHeight) {
      SeparateStringNum = FieldHeight;
    }

    if (IsCentered) {
      if (FieldHeight > 1 && (FieldHeight - SeparateStringNum) > 2) {
        CenteredStartY = (FieldHeight - SeparateStringNum) / 2 - 1;

        CopyRect (&ClearControlField, &Control.ControlField);
        ClearControlField.bottom = ClearControlField.top + CenteredStartY - 1;
        ClearField (Attribute, &ClearControlField, NULL);
      }
    }

    for (SeparateStringIndex = 0; SeparateStringIndex < SeparateStringNum; SeparateStringIndex ++) {
      if (IsCentered) {
        CenteredStartX = Control.ControlField.left + ((FieldWidth - (UINT32)StrLen (TempString)) / 2) - 1;

        CopyRect (&ClearControlField, &Control.ControlField);
        ClearControlField.right   = Control.ControlField.left + CenteredStartX - 1;
        ClearControlField.top    += CenteredStartY + SeparateStringIndex;
        ClearControlField.bottom  = ClearControlField.top;

        if (ClearControlField.right >= ClearControlField.left) {
          ClearField (Attribute, &ClearControlField, NULL);
        }
      }

      //
      // Print one of separate string
      //
      DisplayString (
        Control.ControlField.left + CenteredStartX,
        Control.ControlField.top + CenteredStartY + SeparateStringIndex,
        TempString
        );
      StringWidth = ((UINT32) GetStringWidth (TempString) / sizeof (CHAR16)) - 1;
      TempString += StrLen (TempString) + 1;
      if (TempString[0] == 0x000a) {
        TempString++;
      }

      CopyRect (&ClearControlField, &Control.ControlField);
      ClearControlField.left   += (StringWidth > 0) ?  (CenteredStartX + StringWidth) : CenteredStartX;
      ClearControlField.top    += CenteredStartY + SeparateStringIndex;
      ClearControlField.bottom  = ClearControlField.top;
      if (ClearControlField.right >= ClearControlField.left) {
        ClearField (Attribute, &ClearControlField, NULL);
      }
    }

    //
    // Clear remaining field
    //
    CopyRect (&ClearControlField, &Control.ControlField);
    ClearControlField.top += CenteredStartY + SeparateStringIndex;
    if (ClearControlField.bottom >= ClearControlField.top) {
      ClearField (Attribute, &ClearControlField, NULL);
    }

    SafeFreePool((VOID **) &SeparateStrings);
    SafeFreePool((VOID **) &SeparateStringOffset);
  }

  return EFI_SUCCESS;
}

/**
 Display normal control info list

 @param [in] ControlCount            Control list count
 @param [in] ControlArray            Control list

 @retval EFI_SUCCESS                Display control successfully
 @retval EFI_INVALID_PARAMETER      ControlCount is zero or ControlArray is NULL

**/
EFI_STATUS
DisplayNormalControls (
  IN     UINT32                              ControlCount,
  IN     H2O_CONTROL_INFO                    *ControlArray
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     Count;
  UINT32                                     Index;
  UINT32                                     Start;
  BOOLEAN                                    SetStart;

  if (ControlCount == 0 || ControlArray == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ControlArray->ParentPanel->PanelType == H2O_PANEL_TYPE_SETUP_PAGE) {
    for (Index = 0; Index < ControlCount; Index++) {
      DisplaySetupPageControl (ControlArray[0].ParentPanel, &ControlArray[Index], FALSE);
    }

    return EFI_SUCCESS;
  }


  //
  // Check array is not empty
  //
  Count = 0;
  Index = 0;
  Start = 0;
  SetStart = FALSE;
  while (ControlArray[Index].Text.String != NULL) {
    if (ControlArray[Index].ControlField.left   >= ControlArray[Index].ParentPanel->PanelRelField.left  &&
        ControlArray[Index].ControlField.right  <= ControlArray[Index].ParentPanel->PanelRelField.right &&
        ControlArray[Index].ControlField.top    >= ControlArray[Index].ParentPanel->PanelRelField.top   &&
        ControlArray[Index].ControlField.bottom <= ControlArray[Index].ParentPanel->PanelRelField.bottom) {
      if (!SetStart) {
        SetStart = TRUE;
        Start = Index;
      }
      Count++;
    }
    Index++;

    if (ControlCount <= Index) {
      break;
    }
  }

  if (Count == 0) {
    return EFI_NOT_FOUND;
  }

  Status = EFI_NOT_FOUND;
  if (SetStart) {
    Status = DisplayControls (
               FALSE,
               FALSE,
               Count,
               &ControlArray[Start]
               );
  }
  return Status;
}

/**
 Display button control info list

 @param [in] ButtonStartEndChar     Flag to decide to display string within []
 @param [in] ControlCount           Control list count
 @param [in] ControlArray           Control list

 @retval EFI_SUCCESS                Display control successfully
 @retval EFI_INVALID_PARAMETER      ControlCount is zero or ControlArray is NULL

**/
EFI_STATUS
DisplayButtonControls (
  IN     BOOLEAN                             ButtonStartEndChar,
  IN     UINT32                              ControlCount,
  IN     H2O_CONTROL_INFO                    *ControlArray
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     Count;
  UINT32                                     Index;
  UINT32                                     Start;
  BOOLEAN                                    SetStart;


  if (ControlCount == 0 || ControlArray == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check array is not empty
  //
  Count = 0;
  Index = 0;
  Start = 0;
  SetStart = FALSE;
  while (ControlArray[Index].Text.String != NULL) {
    if (
      ControlArray[Index].ControlField.left   >= ControlArray[Index].ParentPanel->PanelRelField.left &&
      ControlArray[Index].ControlField.right  <= ControlArray[Index].ParentPanel->PanelRelField.right &&
      ControlArray[Index].ControlField.top    >= ControlArray[Index].ParentPanel->PanelRelField.top &&
      ControlArray[Index].ControlField.bottom <= ControlArray[Index].ParentPanel->PanelRelField.bottom
      ) {
      if (!SetStart) {
        SetStart = TRUE;
        Start = Index;
      }
      Count ++;
    }
    Index ++;
    if (ControlCount <= Index) {
      break;
    }
  }

  if (Count == 0) {
    return EFI_NOT_FOUND;
  }

  Status = EFI_NOT_FOUND;
  if (SetStart) {
    //
    // Display buttons
    //
    Status = DisplayControls (
               FALSE,
               ButtonStartEndChar,
               Count,
               ControlArray
               );
  }
  return Status;
}

/**
 Display centered control info list

 @param [in] ButtonStartEndChar     Flag to decide to display string within []
 @param [in] ControlCount           Control list count
 @param [in] ControlArray           Control list

 @retval EFI_SUCCESS                Display control successfully
 @retval EFI_INVALID_PARAMETER      ControlCount is zero or ControlArray is NULL

**/
EFI_STATUS
DisplayCenteredControl (
  IN     BOOLEAN                             ButtonStartEndChar,
  IN     UINT32                              ControlCount,
  IN     H2O_CONTROL_INFO                    *ControlArray
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     Count;
  UINT32                                     Index;
  UINT32                                     Start;
  BOOLEAN                                    SetStart;


  if (ControlCount == 0 || ControlArray == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check array is not empty
  //
  Count = 0;
  Index = 0;
  Start = 0;
  SetStart = FALSE;
  while (ControlArray[Index].Text.String != NULL) {
    if (
      ControlArray[Index].ControlField.left   >= ControlArray[Index].ParentPanel->PanelRelField.left &&
      ControlArray[Index].ControlField.right  <= ControlArray[Index].ParentPanel->PanelRelField.right &&
      ControlArray[Index].ControlField.top    >= ControlArray[Index].ParentPanel->PanelRelField.top &&
      ControlArray[Index].ControlField.bottom <= ControlArray[Index].ParentPanel->PanelRelField.bottom
      ) {
      if (!SetStart) {
        SetStart = TRUE;
        Start = Index;
      }
      Count ++;
    }
    Index ++;
    if (ControlCount <= Index) {
      break;
    }
  }

  if (Count == 0) {
    return EFI_NOT_FOUND;
  }

  Status = EFI_NOT_FOUND;
  if (SetStart) {
    //
    // Display buttons
    //
    Status = DisplayControls (
               TRUE,
               ButtonStartEndChar,
               Count,
               ControlArray
               );
  }
  return Status;
}

/**
 Display control in highlight

 @param [in] ButtonStartEndChar     Flag to decide to display string within []
 @param [in] Control                Control info

 @retval EFI_SUCCESS                Display control in highlight successfully
 @retval EFI_INVALID_PARAMETER      Control is NULL
 @retval Other                      Display control fail

**/
EFI_STATUS
DisplayHighLightControl (
  IN BOOLEAN                                 ButtonStartEndChar,
  IN H2O_CONTROL_INFO                        *Control
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     TempPseudoClass;
  CHAR16                                     *TempString;
  RECT                                       TempField;

  if (Control == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Control->ParentPanel->PanelType == H2O_PANEL_TYPE_SETUP_PAGE) {
    return DisplaySetupPageControl (Control->ParentPanel, Control, TRUE);
  }

  Status     = EFI_NOT_FOUND;
  TempString = NULL;

  if (Control->ParentPanel->PanelType == H2O_PANEL_TYPE_HELP_TEXT){
    TempString = Control->Text.String;
    CopyRect (&TempField, &Control->ControlField);
    //
    // Cut Control
    // Cut: If content exceed control, it cut content
    //
    Status = GetCutControl (Control, &Control->ParentPanel->PanelRelField, &Control->ControlField, &Control->Text.String);
    if (Status == EFI_SUCCESS) {
      //
      // Really cut
      //
      Control->ParentPanel->WithScrollBarVertical = TRUE;
      DisplayPanel (Control->ParentPanel, H2O_STYLE_PSEUDO_CLASS_NORMAL);
    } else {
      TempString = NULL;
      Control->ParentPanel->WithScrollBarVertical = FALSE;
    }
  }

  if (Control->ControlField.left   >= Control->ParentPanel->PanelRelField.left  &&
      Control->ControlField.right  <= Control->ParentPanel->PanelRelField.right &&
      Control->ControlField.top    >= Control->ParentPanel->PanelRelField.top   &&
      Control->ControlField.bottom <= Control->ParentPanel->PanelRelField.bottom
      ) {
    TempPseudoClass = Control->ControlStyle.PseudoClass;
    //
    // Highlight
    //
    if (Control->Editable) {
      Control->ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_FOCUS;
    } else if (Control->Selectable) {
      Control->ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_FOCUS;
    } else {
      Control->ControlStyle.PseudoClass = H2O_STYLE_PSEUDO_CLASS_HIGHLIGHT;
    }
    Status = DisplayControls (
               FALSE,
               ButtonStartEndChar,
               1,
               Control
               );
    Control->ControlStyle.PseudoClass = TempPseudoClass;

    if (TempString != NULL) {
      SafeFreePool ((VOID **) &Control->Text.String);
      Control->Text.String = TempString;
      CopyRect (&Control->ControlField, &TempField);
    }
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/**
 Display control in specific PseudoClass

 @param [in] PseudoClass            Displayed controls PseudoClass
 @param [in] ControlArray           Controls info

 @retval EFI_SUCCESS                Display control in highlight successfully
 @retval EFI_INVALID_PARAMETER      Control is NULL
 @retval Other                      Display control fail

**/
EFI_STATUS
DisplayPseudoClassControl (
  IN     UINT32                              PseudoClass,
  IN     UINT32                              ControlCount,
  IN     H2O_CONTROL_INFO                    *ControlArray
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     Count;
  UINT32                                     Index;
  UINT32                                     Start;
  BOOLEAN                                    SetStart;
  UINT32                                     TempPseudoClass;


  if (ControlCount == 0 || ControlArray == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check array is not empty
  //
  Count = 0;
  Index = 0;
  Start = 0;
  SetStart = FALSE;
  while (ControlArray[Index].Text.String != NULL) {
    if (
      ControlArray[Index].ControlField.left   >= ControlArray[Index].ParentPanel->PanelRelField.left  &&
      ControlArray[Index].ControlField.right  <= ControlArray[Index].ParentPanel->PanelRelField.right &&
      ControlArray[Index].ControlField.top    >= ControlArray[Index].ParentPanel->PanelRelField.top   &&
      ControlArray[Index].ControlField.bottom <= ControlArray[Index].ParentPanel->PanelRelField.bottom
      ) {
      if (!SetStart) {
        SetStart = TRUE;
        Start = Index;
      }
      //
      // Change PseudoClass
      //
      TempPseudoClass = ControlArray[Index].ControlStyle.PseudoClass;
      ControlArray[Index].ControlStyle.PseudoClass = PseudoClass;

      Status = EFI_NOT_FOUND;
      Status = DisplayControls (
                 FALSE,
                 FALSE,
                 1,
                 &ControlArray[Index]
                 );
      ControlArray[Index].ControlStyle.PseudoClass = TempPseudoClass;
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Count ++;
    }
    Index ++;
    if (ControlCount <= Index) {
      break;
    }
  }

  if (Count == 0) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

/**
 Process page up for panel info

 @param [in] Panel                  Panel info

 @retval EFI_SUCCESS                Process page up successfully

**/
EFI_STATUS
PanelPageUp (
  IN     H2O_PANEL_INFO                      *Panel
  )
{
  INT32                                      MoveHeight;


  MoveHeight = Panel->PanelField.bottom - Panel->PanelField.top;
  if ((GetBorderWidth(Panel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL)) > 0) {
    MoveHeight -= (BORDER_TOP_WIDTH_DEFLULT + BORDER_BOTTOM_WIDTH_DEFLULT);
  }

  if (Panel->PanelRelField.top < MoveHeight) {
    Panel->PanelRelField.top    = 0;
    Panel->PanelRelField.bottom = MoveHeight;
  } else {
    Panel->PanelRelField.top    -= MoveHeight;
    Panel->PanelRelField.bottom -= MoveHeight;
  }

  return EFI_SUCCESS;
}

/**
 Process page down for panel info

 @param [in] Panel                  Panel info

 @retval EFI_SUCCESS                Process page down successfully

**/
EFI_STATUS
PanelPageDown (
  IN     H2O_PANEL_INFO                      *Panel
  )
{
  INT32                                      MoveHeight;
  UINT32                                     Index;
  INT32                                      ControlMaxY;


  MoveHeight = Panel->PanelField.bottom - Panel->PanelField.top;
  if ((GetBorderWidth(Panel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL)) > 0) {
    MoveHeight -= (BORDER_TOP_WIDTH_DEFLULT + BORDER_BOTTOM_WIDTH_DEFLULT);
  }

  ControlMaxY = 0;
  for (Index = 0; Index < Panel->ControlList.Count; Index ++) {
    ControlMaxY = MAX (ControlMaxY, Panel->ControlList.ControlArray[Index].ControlField.bottom);
  }

  if (ControlMaxY < MoveHeight) {
    Panel->PanelRelField.top    = 0;
    Panel->PanelRelField.bottom = MoveHeight;
  } else {
    Panel->PanelRelField.top    += MoveHeight;
    Panel->PanelRelField.bottom += MoveHeight;
    if (Panel->PanelRelField.bottom > ControlMaxY) {
      Panel->PanelRelField.top    = ControlMaxY - MoveHeight;
      Panel->PanelRelField.bottom = ControlMaxY;
    }
  }

  return EFI_SUCCESS;
}

/**
 Update control info to make sure the field of control is within page

 @param [in] Control                Control info

 @retval EFI_SUCCESS                Update control info successfully
 @retval EFI_INVALID_PARAMETER      Control is NULL or Control->ParentPanel is NULL
 @retval EFI_NOT_FOUND              No need to update control info

**/
EFI_STATUS
EnsureControlInPanel (
  IN     H2O_CONTROL_INFO                    *Control
  )
{
  if (Control == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Control->ParentPanel == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Control->ControlField.left   < Control->ParentPanel->PanelRelField.left  ||
      Control->ControlField.right  > Control->ParentPanel->PanelRelField.right ||
      Control->ControlField.top    < Control->ParentPanel->PanelRelField.top   ||
      Control->ControlField.bottom > Control->ParentPanel->PanelRelField.bottom
      ) {
    //
    // Change panel
    //
    Control->ParentPanel->PanelRelField.left   = Control->ControlField.left;
    Control->ParentPanel->PanelRelField.top    = Control->ControlField.top;
    Control->ParentPanel->PanelRelField.right  = Control->ParentPanel->PanelRelField.left + (Control->ParentPanel->PanelField.right - Control->ParentPanel->PanelField.left);
    Control->ParentPanel->PanelRelField.bottom = Control->ParentPanel->PanelRelField.top  + (Control->ParentPanel->PanelField.bottom - Control->ParentPanel->PanelField.top);
    if ((GetBorderWidth(Control->ParentPanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL)) > 0) {
      Control->ParentPanel->PanelRelField.right  -= (BORDER_TOP_WIDTH_DEFLULT + BORDER_BOTTOM_WIDTH_DEFLULT);
      Control->ParentPanel->PanelRelField.bottom -= (BORDER_TOP_WIDTH_DEFLULT + BORDER_BOTTOM_WIDTH_DEFLULT);
    }
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

BOOLEAN
CheckPressControls (
  IN     BOOLEAN                              Keyboard,
  IN     EFI_INPUT_KEY                        *Key,
  IN     INT32                                RealMouseX,
  IN     INT32                                RealMouseY,
  IN     UINT32                               ControlCount,
  IN     H2O_CONTROL_INFO                     *ControlArray,
  IN OUT H2O_CONTROL_INFO                     **SelectedControl
  )
{
  INT32                                       MouseX;
  INT32                                       MouseY;
  H2O_CONTROL_INFO                            *CurrentControl;
  UINT32                                      Index;
  UINT32                                      SelectedIndex;
  BOOLEAN                                     Found;
  RECT                                        ControlAbsField;
  INT32                                       BorderLineWidth;
  EFI_STATUS                                  Status;

  if (Key == NULL) {
    return FALSE;
  }

  if (Key->ScanCode == SCAN_ESC) {
    *SelectedControl = NULL;
    return TRUE;
  }

  Found          = FALSE;
  CurrentControl = *SelectedControl;

  if (!Keyboard) {
    if (ControlCount == 0 || Key->UnicodeChar != CHAR_CARRIAGE_RETURN) {
      return Found;
    }

    Status = TransferToTextModePosition (mDEPrivate, RealMouseX, RealMouseY, (UINT32 *) &MouseX, (UINT32 *) &MouseY);
    if (EFI_ERROR (Status)) {
      return Found;
    }

    BorderLineWidth  = GetBorderWidth (ControlArray[0].ParentPanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);
    CopyRect (&ControlAbsField, &ControlArray[0].ParentPanel->PanelField);
    InflateRect (&ControlAbsField, -BorderLineWidth, -BorderLineWidth);
    if (!IsPointOnField (&ControlAbsField, MouseX, MouseY)) {
      return FALSE;
    }

    for (Index = 0; Index < ControlCount; Index++) {
      Status = GetControlAbsField (
                 &ControlArray[Index].ParentPanel->PanelField,
                 BorderLineWidth,
                 &ControlArray[Index].ControlField,
                 &ControlAbsField
                 );
      if (EFI_ERROR (Status)) {
        continue;
      }

      if (IsPointOnField (&ControlAbsField, MouseX, MouseY)) {
        //
        // Control has been pressed
        //
        CurrentControl = &ControlArray[Index];
        Found          = TRUE;
        break;
      }
    }
  } else {
    //
    // Keyboard Event
    //
    switch (Key->UnicodeChar) {

    case CHAR_CARRIAGE_RETURN:
      CurrentControl = *SelectedControl;
      Found          = TRUE;
      break;

    default:
      if (Key->ScanCode == SCAN_LEFT || Key->ScanCode == SCAN_UP) {
        for (Index = 0; Index < ControlCount; Index ++) {
          if ((UINTN)(UINTN *)CurrentControl == (UINTN)(UINTN *)&ControlArray[Index]) {
            SelectedIndex  = (Index > 0) ? (Index - 1) : (ControlCount - 1);
            CurrentControl = &ControlArray[SelectedIndex];
            Found          = TRUE;
            break;
          }
        }
      } else if (Key->ScanCode == SCAN_RIGHT || Key->ScanCode == SCAN_DOWN) {
        for (Index = 0; Index < ControlCount; Index ++) {
          if ((UINTN)(UINTN *)CurrentControl == (UINTN)(UINTN *)&ControlArray[Index]) {
            SelectedIndex  = (Index < (ControlCount - 1)) ? (Index + 1) : 0;
            CurrentControl = &ControlArray[SelectedIndex];
            Found          = TRUE;
            break;
          }
        }
      }
      break;
    }
  }

  *SelectedControl = CurrentControl;

  return Found;
}

EFI_STATUS
CalculateRequireSize (
  IN  CHAR16                                 *DisplayString,
  IN  UINT32                                 LimitLineWidth,
  OUT UINT32                                 *RequireWidth,
  OUT UINT32                                 *RequireHeight
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     SeparateStringNum;
  UINT32                                     *SeparateStringOffset;
  CHAR16                                     *SeparateString;
  CHAR16                                     *CurrentString;
  UINT32                                     Index;
  UINT32                                     StringWidth;
  UINT32                                     MaxStringWidth;

  if (DisplayString == NULL || RequireWidth == NULL || RequireHeight == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetLineByWidth (
             DisplayString,
             LimitLineWidth,
             &SeparateStringNum,
             &SeparateStringOffset,
             &SeparateString
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CurrentString  = SeparateString;
  MaxStringWidth = 0;
  for (Index = 0; Index < SeparateStringNum; Index++) {
    StringWidth    = ((UINT32) GetStringWidth (CurrentString) / sizeof (CHAR16)) - 1;
    CurrentString += (StrLen (CurrentString) + 1);
    MaxStringWidth = MAX (MaxStringWidth, StringWidth);
  }

  *RequireWidth  = MaxStringWidth;
  *RequireHeight = SeparateStringNum;

  SafeFreePool ((VOID **) &SeparateString);
  SafeFreePool ((VOID **) &SeparateStringOffset);

  return EFI_SUCCESS;
}

/**
 Init control list

 @param [in] PromptControls             Control list

**/
VOID
InitControlList (
 IN     H2O_CONTROL_LIST                    *ControlList
 )
{
  ZeroMem (ControlList, sizeof (H2O_CONTROL_LIST));
}

EFI_STATUS
FreeControlList (
  IN H2O_CONTROL_LIST                       *ControlList
  )
{
  UINT32                                     Index;

  if (ControlList != NULL && ControlList->ControlArray != NULL) {
    for (Index = 0; Index < ControlList->Count; Index ++) {
      SafeFreePool ((VOID **) &ControlList->ControlArray[Index].Text.String);
      SafeFreePool ((VOID **) &ControlList->ControlArray[Index].ValueStrInfo.String);
    }
    ControlList->Count = 0;
    FreePool (ControlList->ControlArray);
    ControlList->ControlArray = NULL;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DisplaySetupPageControl (
  IN H2O_PANEL_INFO                          *SetupPagePanel,
  IN H2O_CONTROL_INFO                        *Control,
  IN BOOLEAN                                 IsHighlight
  )
{
  EFI_STATUS                                 Status;
  UINT32                                     Attribute;
  UINT32                                     HighlightAttribute;
  UINT32                                     StartIndex;
  UINT32                                     EndIndex;
  UINT32                                     ControlHeight;
  UINT32                                     TextStringNum;
  UINT32                                     *TextStringOffset;
  CHAR16                                     *TextStrings;
  UINT32                                     ValueStringNum;
  UINT32                                     *ValueStringOffset;
  CHAR16                                     *ValueStrings;
  CHAR16                                     *TextStr;
  CHAR16                                     *ValueStr;
  UINT32                                     Index;
  INT32                                      BorderLineWidth;
  RECT                                       AbsField;
  RECT                                       RelField;
  RECT                                       TextField;
  RECT                                       ValueStrField;
  RECT                                       Field;
  UINTN                                      Width;
  UINT8                                      SequenceIndex;

  if (Control->ControlField.top    > SetupPagePanel->PanelRelField.bottom ||
      Control->ControlField.bottom < SetupPagePanel->PanelRelField.top) {
    return EFI_ABORTED;
  }

  GetPanelColorAttribute (SetupPagePanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, Control->ControlStyle.PseudoClass, &Attribute);
  GetPanelColorAttribute (SetupPagePanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_HIGHLIGHT , &HighlightAttribute);
  BorderLineWidth = GetBorderWidth (SetupPagePanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);

  //
  // Calculate the absolute position of each control field
  //
  CopyRect (&AbsField, &Control->ControlField);
  OffsetRect (
    &AbsField,
    SetupPagePanel->PanelField.left + BorderLineWidth - SetupPagePanel->PanelRelField.left,
    SetupPagePanel->PanelField.top + BorderLineWidth - SetupPagePanel->PanelRelField.top
    );

  CopyRect (&RelField, &Control->ControlField);
  ControlHeight = RelField.bottom - RelField.top + 1;
  StartIndex = (UINT32) ((SetupPagePanel->PanelRelField.top > RelField.top) ?
                         (SetupPagePanel->PanelRelField.top - RelField.top) :
                         0
                         );
  EndIndex   = (UINT32) ((RelField.bottom > SetupPagePanel->PanelRelField.bottom) ?
                         ControlHeight - 1 - (RelField.bottom - SetupPagePanel->PanelRelField.bottom) :
                         ControlHeight - 1
                         );

  //
  // Display Text string
  //
  CopyRect (&TextField, &Control->Text.StringField);
  OffsetRect (
    &TextField,
    AbsField.left - Control->ControlField.left,
    AbsField.top - Control->ControlField.top
    );

  Status = GetLineByWidth (
             Control->Text.String,
             Control->Text.StringField.right - Control->Text.StringField.left + 1,
             &TextStringNum,
             &TextStringOffset,
             &TextStrings
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TextStr = TextStrings;
  Index   = 0;
  while (Index < StartIndex) {
    TextStr += StrLen (TextStr) + 1;
    Index++;
  }

  for (Index = StartIndex; Index <= EndIndex; Index++) {
    if (Index <= TextStringNum - 1) {
      DEConOutSetAttribute (mDEPrivate, IsHighlight ? HighlightAttribute : Attribute);
      DisplayString (
        TextField.left,
        TextField.top + Index,
        TextStr
        );
      Width = GetStringDisplayWidth (TextStr);
      TextStr += StrLen (TextStr) + 1;
    } else {
      Width = 0;
    }

    SetRect (
      &Field,
      TextField.left + (INT32) Width,
      TextField.top + Index,
      TextField.left + (Control->Text.StringField.right - Control->Text.StringField.left),
      TextField.top + Index
      );
    ClearField (Attribute, &Field, NULL);
  }
  SafeFreePool((VOID **) &TextStrings);
  SafeFreePool((VOID **) &TextStringOffset);

  if (Control->ValueStrInfo.String == NULL) {
    return EFI_SUCCESS;
  }

  //
  // Display Value string
  //
  CopyRect (&ValueStrField, &Control->ValueStrInfo.StringField);
  OffsetRect (
    &ValueStrField,
    AbsField.left - Control->ControlField.left,
    AbsField.top - Control->ControlField.top
    );

  if (PROMPT_VALUE_SEPARATOR_WIDTH != 0) {
    for (Index = StartIndex; Index <= EndIndex; Index++) {
      SetRect (
        &Field,
        ValueStrField.left - PROMPT_VALUE_SEPARATOR_WIDTH,
        ValueStrField.top + Index,
        ValueStrField.left - PROMPT_VALUE_SEPARATOR_WIDTH,
        ValueStrField.top + Index
        );
      ClearField (Attribute, &Field, NULL);
    }
  }

  ValueStrings = NULL;
  ValueStringOffset = NULL;

  switch (Control->Operand) {

  case EFI_IFR_TIME_OP:
  case EFI_IFR_DATE_OP:
    DEConOutSetAttribute (mDEPrivate, IsHighlight ? HighlightAttribute : Attribute);
    DateTimeOpCodeDisplayValueStr (
      Control,
      &ValueStrField,
      Attribute,
      HighlightAttribute,
      IsHighlight
      );
    break;

  case EFI_IFR_ORDERED_LIST_OP:
  default:
    Status = GetLineByWidth (
               Control->ValueStrInfo.String,
               Control->ValueStrInfo.StringField.right - Control->ValueStrInfo.StringField.left + 1,
               &ValueStringNum,
               &ValueStringOffset,
               &ValueStrings
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    ValueStr      = ValueStrings;
    Index         = 0;
    SequenceIndex = 0;
    while (Index < StartIndex) {
      if (Control->Operand == EFI_IFR_ORDERED_LIST_OP && StrStr (ValueStr, L"\n") != NULL) {
        SequenceIndex++;
      }
      ValueStr += StrLen (ValueStr) + 1;
      Index++;
    }

    for (Index = StartIndex; Index <= EndIndex; Index++) {
      if (Index <= ValueStringNum - 1) {
        if (Control->Operand == EFI_IFR_ORDERED_LIST_OP) {
          DEConOutSetAttribute (mDEPrivate, IsHighlight && (SequenceIndex == Control->Sequence) ? HighlightAttribute : Attribute);
        } else {
          DEConOutSetAttribute (mDEPrivate, IsHighlight ? HighlightAttribute : Attribute);
        }
        DisplayString (
          ValueStrField.left,
          ValueStrField.top + Index,
          ValueStr
          );
        Width = GetStringDisplayWidth (ValueStr);
        if (Control->Operand == EFI_IFR_ORDERED_LIST_OP && StrStr (ValueStr, L"\n") != NULL) {
          Width--;
          SequenceIndex++;
        }
        ValueStr += StrLen (ValueStr) + 1;
      } else {
        Width = 0;
      }

      SetRect (
        &Field,
        ValueStrField.left + (INT32) Width,
        ValueStrField.top + Index,
        ValueStrField.left + (Control->ValueStrInfo.StringField.right - Control->ValueStrInfo.StringField.left),
        ValueStrField.top + Index
        );
      ClearField (Attribute, &Field, NULL);
    }
  }

  SafeFreePool((VOID **) &ValueStrings);
  SafeFreePool((VOID **) &ValueStringOffset);

  return EFI_SUCCESS;
}

EFI_STATUS
DisplaySetupPageControls (
  IN H2O_PANEL_INFO                          *SetupPagePanel
  )
{
  UINT32                                     Attribute;
  INT32                                      BorderLineWidth;
  UINT32                                     ControlIndex;
  UINT32                                     ControlCount;
  H2O_CONTROL_INFO                           *ControlArray;
  RECT                                       AbsField;

  if (SetupPagePanel == NULL || SetupPagePanel->PanelType != H2O_PANEL_TYPE_SETUP_PAGE) {
    return EFI_INVALID_PARAMETER;
  }

  ControlArray = SetupPagePanel->ControlList.ControlArray;
  ControlCount = SetupPagePanel->ControlList.Count;

  for (ControlIndex = 0; ControlIndex < ControlCount; ControlIndex++) {
    if (IsCurrentHighlight (mDEPrivate, ControlArray[ControlIndex].PageId, ControlArray[ControlIndex].QuestionId, ControlArray[ControlIndex].IfrOpCode)) {
      DisplaySetupPageControl (SetupPagePanel, &ControlArray[ControlIndex], TRUE);
    } else {
      DisplaySetupPageControl (SetupPagePanel, &ControlArray[ControlIndex], FALSE);
    }
  }

  if (ControlCount == 0 ||
      ControlArray[ControlCount - 1].ControlField.bottom < SetupPagePanel->PanelRelField.bottom) {
    BorderLineWidth = GetBorderWidth (SetupPagePanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);
    //
    // Clean remainding region
    //
    if (ControlCount == 0) {
      CopyRect (&AbsField, &SetupPagePanel->PanelRelField);
    } else {
      CopyRect (&AbsField, &ControlArray[ControlCount - 1].ControlField);
      AbsField.top    = AbsField.bottom + 1;
      AbsField.bottom = SetupPagePanel->PanelRelField.bottom;
    }
    OffsetRect (&AbsField, SetupPagePanel->PanelField.left    , SetupPagePanel->PanelField.top);
    OffsetRect (&AbsField, -SetupPagePanel->PanelRelField.left, -SetupPagePanel->PanelRelField.top);
    OffsetRect (&AbsField, BorderLineWidth                    , BorderLineWidth);
    GetPanelColorAttribute (SetupPagePanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL, &Attribute);
    ClearField (Attribute, &AbsField, NULL);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SetSetupPageControlsField (
  IN H2O_PANEL_INFO                           *SetupPagePanel
  )
{
  RECT                                        PanelRelField;
  UINT32                                      Index;
  UINT32                                      Count;
  H2O_CONTROL_INFO                            *ControlArray;
  H2O_CONTROL_INFO                            *Control;
  INT32                                       CurrentX;
  INT32                                       CurrentY;
  UINT32                                      SetupPageWidth;
  UINT32                                      PromptWidth;
  UINT32                                      ValueWidth;
  CHAR16                                      *SeparateStrings;
  UINT32                                      *SeparateStringOffset;
  EFI_STATUS                                  Status;
  INT32                                       TextStrWidth;
  INT32                                       TextStrHeight;
  INT32                                       ValueStrWidth;
  INT32                                       ValueStrHeight;
  INT32                                       BorderLineWidth;

  if (SetupPagePanel == NULL || SetupPagePanel->PanelType != H2O_PANEL_TYPE_SETUP_PAGE) {
    return EFI_INVALID_PARAMETER;
  }

  CopyRect (&PanelRelField , &SetupPagePanel->PanelField);
  OffsetRect (&PanelRelField , -PanelRelField.left, -PanelRelField.top);
  BorderLineWidth = GetBorderWidth (SetupPagePanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);
  if (BorderLineWidth > 0) {
    PanelRelField.bottom -= BorderLineWidth * 2;
    PanelRelField.right  -= BorderLineWidth * 2;
  }

  SetupPageWidth = (UINT32) (PanelRelField.right - PanelRelField.left + 1);
  PromptWidth    = (SetupPageWidth - PROMPT_VALUE_SEPARATOR_WIDTH) / 2;
  ValueWidth     = SetupPageWidth - PromptWidth;
  ControlArray   = SetupPagePanel->ControlList.ControlArray;
  Count          = SetupPagePanel->ControlList.Count;

  CurrentX = 0;
  CurrentY = 0;
  for (Index = 0; Index < Count; Index++) {
    Control = &ControlArray[Index];

    if (Control->ValueStrInfo.String == NULL) {
      TextStrWidth   = SetupPageWidth;
      ValueStrHeight = 0;
    } else {
      TextStrWidth   = PromptWidth;
      ValueStrWidth  = ValueWidth;
    }

    Status = GetLineByWidth (Control->Text.String, TextStrWidth, &TextStrHeight, &SeparateStringOffset, &SeparateStrings);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    FreePool ((VOID *) SeparateStringOffset);
    FreePool ((VOID *) SeparateStrings);

    SetRect (
      &Control->Text.StringField,
      CurrentX,
      CurrentY,
      CurrentX + TextStrWidth - 1,
      CurrentY + TextStrHeight - 1
      );

    ValueStrHeight = 0;
    if (Control->ValueStrInfo.String != NULL) {
      ValueStrWidth = ValueWidth;
      Status = GetLineByWidth (
                 Control->ValueStrInfo.String,
                 ValueStrWidth,
                 &ValueStrHeight,
                 &SeparateStringOffset,
                 &SeparateStrings
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      FreePool ((VOID *) SeparateStringOffset);
      FreePool ((VOID *) SeparateStrings);

      SetRect (
        &Control->ValueStrInfo.StringField,
        CurrentX + PromptWidth + PROMPT_VALUE_SEPARATOR_WIDTH,
        CurrentY,
        CurrentX + SetupPageWidth - 1,
        CurrentY + ValueStrHeight - 1
        );
    }

    SetRect (
      &Control->ControlField,
      CurrentX,
      CurrentY,
      CurrentX + SetupPageWidth - 1,
      CurrentY + MAX (TextStrHeight, ValueStrHeight) - 1
      );

    CurrentY += MAX (TextStrHeight, ValueStrHeight);
  }

  return EFI_SUCCESS;
}

