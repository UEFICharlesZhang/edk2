/** @file
  This file for CR SOL reference
	
;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "Sol.h"

#define NUL                               0
#define SOL_TEXT_INPUT_DIALOG_HEIGHT      7
#define NARROW_CHAR         0xFFF0
#define WIDE_CHAR           0xFFF1

typedef BOOLEAN (*VALIDATE_FUNC)(CHAR16 code);

UINTN
PrintInternal (
  IN UINTN                            Column,
  IN UINTN                            Row,
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *Out,
  IN CHAR16                           *fmt,
  IN VA_LIST                          args
  )
{
  CHAR16  *Buffer;
  CHAR16  *BackupBuffer;
  UINTN   Index;
  UINTN   PreviousIndex;

  //
  // For now, allocate an arbitrarily long buffer
  //
  Buffer        = AllocateZeroPool (0x10000);
  BackupBuffer  = AllocateZeroPool (0x10000);
  ASSERT (Buffer);
  ASSERT (BackupBuffer);
  if (Buffer == NULL || BackupBuffer == NULL) {
    return 0;
  }

  if (Column != (UINTN) -1) {
    Out->SetCursorPosition (Out, Column, Row);
  }

  UnicodeVSPrint  (Buffer, 0x10000, fmt, args);

  Out->Mode->Attribute = Out->Mode->Attribute & 0x7f;
  Out->SetAttribute (Out, Out->Mode->Attribute);

  Index         = 0;
  PreviousIndex = 0;

  do {
    for (; (Buffer[Index] != NARROW_CHAR) && (Buffer[Index] != WIDE_CHAR) && (Buffer[Index] != 0); Index++) {
      BackupBuffer[Index] = Buffer[Index];
    }

    if (Buffer[Index] == 0) {
      break;
    }
    //
    // Null-terminate the temporary string
    //
    BackupBuffer[Index] = 0;

    //
    // Print this out, we are about to switch widths
    //
    Out->OutputString (Out, &BackupBuffer[PreviousIndex]);

    //
    // Preserve the current index + 1, since this is where we will start printing from next
    //
    PreviousIndex = Index + 1;

    //
    // We are at a narrow or wide character directive.  Set attributes and strip it and print it
    //
    if (Buffer[Index] == NARROW_CHAR) {
      //
      // Preserve bits 0 - 6 and zero out the rest
      //
      Out->Mode->Attribute = Out->Mode->Attribute & 0x7f;
      Out->SetAttribute (Out, Out->Mode->Attribute);
    } else {
      //
      // Must be wide, set bit 7 ON
      //
      Out->Mode->Attribute = Out->Mode->Attribute | EFI_WIDE_ATTRIBUTE;
      Out->SetAttribute (Out, Out->Mode->Attribute);
    }

    Index++;
  } while (Buffer[Index] != 0);

  //
  // We hit the end of the string - print it
  //
  Out->OutputString (Out, &BackupBuffer[PreviousIndex]);

  gBS->FreePool (Buffer);
  gBS->FreePool (BackupBuffer);

  return EFI_SUCCESS;
}


UINTN
PrintAt (
  IN UINTN     Column,
  IN UINTN     Row,
  IN CHAR16    *fmt,
  ...
  )
{
  VA_LIST args;

  VA_START (args, fmt);
  return PrintInternal (Column, Row, gST->ConOut, fmt, args);
};

VOID
DialogStrnCpy (
  CHAR16    *Dest,
  CHAR16    *Src,
  UINTN     Length
  )
{
  while (*Src && Length) {
    *(Dest++) = *(Src++);
    Length--;
  }

  *Dest = 0;
}


EFI_STATUS
ReadKeyStroke (
  OUT  EFI_INPUT_KEY           *Key
  )
{
  EFI_STATUS      Status;
  UINTN           Index;

  gBS->WaitForEvent (1, gST->ConIn->WaitForKey, &Index);
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, Key);

  return Status;
}


VOID
DrawHorizontalLine (
  IN  UINTN       x,
  IN  UINTN       y,
  IN  UINTN       Len
  )
{
  UINTN       i;

  for (i = 0; i < Len; i++, x++) {
    PrintAt( x, y, L"%c", BOXDRAW_HORIZONTAL);
  }
}

VOID
DrawVerticalLine (
  IN  UINTN       x,
  IN  UINTN       y,
  IN  UINTN       Len
  )
{
  UINTN       i;

  for (i = 0; i < Len; i++, y++) {
    PrintAt( x, y, L"%c", BOXDRAW_VERTICAL);
  }
}

VOID
ClearBoxArea (
  IN  UINTN       x,
  IN  UINTN       y,
  IN  UINTN       Width,
  IN  UINTN       Height,
  IN  UINTN       Attrib
  )
{
  UINTN         Row;
  UINTN         Column;
  CHAR16        *SpaceLine;
  EFI_STATUS    Status;

  //
  // allocate space as clean line
  //  
  Status = gBS->AllocatePool (EfiBootServicesData, (Width + 1) * sizeof(CHAR16), (VOID **)&SpaceLine);
  if (EFI_ERROR(Status)) {
    return;
  }

  //
  // Fill line with space char
  // 
  for (Column = 0; Column < Width; Column++) {
    SpaceLine[Column] = L' ';
  }

  //
  // add a null terminate
  // 
  SpaceLine[Column] = 0;

  //
  // set console attribute
  // 
  gST->ConOut->SetAttribute (gST->ConOut, Attrib);

  //
  // clean the box Row
  // 
  for (Row = 0; Row < Height; Row++) {
    PrintAt( x, y + Row, L"%s", SpaceLine);
  }

  //
  // free memory of clean line
  // 
  gBS->FreePool (SpaceLine);
}

VOID 
DrawInputDialogBox (
  IN  UINTN       x,
  IN  UINTN       y,
  IN  UINTN       Width,
  IN  UINTN       Height,
  IN  CHAR16      *Title
  )
{
  UINTN     BoxTop;
  UINTN     BoxBottom;
  UINTN     BoxLeft;
  UINTN     BoxRight;
  UINTN     TitleX;
  UINTN     TitleY; 
  UINTN     TitleLen;
  UINTN     Attrib;

  BoxLeft = x;
  BoxRight = x + Width - 1;
  BoxTop = y;
  BoxBottom = y + Height - 1;

  Attrib = EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLUE);
  
  //
  //2 Clear Box
  //
  ClearBoxArea (x, y, Width, Height, Attrib);
  
  //
  //2 Draw Horizontal line
  //
  DrawHorizontalLine( BoxLeft+1, BoxTop, Width - 2);
  DrawHorizontalLine( BoxLeft+1, BoxTop + 2, Width - 2);
  DrawHorizontalLine( BoxLeft+1, BoxBottom, Width - 2);

  //
  //2 Draw Vertical line
  // 
  DrawVerticalLine ( BoxLeft, BoxTop+1, Height - 2);
  DrawVerticalLine ( BoxRight, BoxTop+1, Height - 2);

  //
  //2 Draw box coner
  // 
  PrintAt (BoxLeft, BoxTop, L"%c", BOXDRAW_DOWN_RIGHT);
  PrintAt (BoxRight, BoxTop, L"%c", BOXDRAW_DOWN_LEFT);
  PrintAt (BoxLeft, BoxBottom, L"%c", BOXDRAW_UP_RIGHT);
  PrintAt (BoxRight, BoxBottom, L"%c", BOXDRAW_UP_LEFT);
  
  PrintAt (BoxLeft, BoxTop + 2, L"%c", BOXDRAW_VERTICAL_RIGHT);
  PrintAt (BoxRight, BoxTop + 2, L"%c", BOXDRAW_VERTICAL_LEFT);

  //
  //2 Print Title string
  // 
  TitleLen = StrLen (Title);
  TitleX = x + Width / 2 - TitleLen / 2;
  TitleY = y + 1;
  PrintAt (TitleX, TitleY, L"%s", Title);
  
}

EFI_STATUS
TextInputDialog (
  IN  CHAR16          *Title,
  IN  VALIDATE_FUNC   IsValidChar,
  IN  UINTN           FieldLen,
  IN  UINTN           BufferSize,
  OUT CHAR16          *Buffer
  )
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_TEXT_OUTPUT_MODE   *ConsoleMode;
  CHAR16                        DisStr[256];
  UINTN                         Offset;
  UINTN                         Columns;
  UINTN                         Rows;
  BOOLEAN                       IsDone;
  UINTN                         OldAttribute;
  BOOLEAN                       NeedUpdate;
  UINTN                         TitleLen;
  UINTN                         FieldX;
  UINTN                         FieldY;
  EFI_INPUT_KEY                 Key;
  UINTN                         BoxLeft;
  UINTN                         BoxRight;
  UINTN                         BoxTop;
  UINTN                         BoxBottom;
  UINTN                         DialogHeight;
  UINTN                         DialogWidth;
  UINTN                         FieldPos;
  UINTN                         BufferPos; 
  
  //
  // Variable initial
  //
  TitleLen = StrLen (Title);
  if (FieldLen > TitleLen) {
    DialogWidth = FieldLen + 4;
  }
  else {
    DialogWidth = TitleLen + 4;
  }
  
  DialogHeight = SOL_TEXT_INPUT_DIALOG_HEIGHT;

  //
  // Disable cursor
  //
  OldAttribute  = gST->ConOut->Mode->Attribute;
  gST->ConOut->EnableCursor (gST->ConOut, FALSE);

  //
  // Get screen size and calculate left, right, top and bottom of this box
  //
  ConsoleMode = gST->ConOut->Mode;
  gST->ConOut->QueryMode (gST->ConOut, ConsoleMode->Mode, &Columns, &Rows);
  
  BoxLeft = (Columns - DialogWidth) / 2;
  BoxRight = BoxLeft + DialogWidth - 1;
  BoxTop = (Rows - DialogHeight) / 2;
  BoxBottom = BoxTop + DialogHeight - 1;

  //
  // Print  dialog 
  //
  DrawInputDialogBox (BoxLeft, BoxTop, DialogWidth, DialogHeight, Title);

  //
  // Caculate the Field X, Y, Width
  //
  FieldX = (DialogWidth - FieldLen) / 2 + BoxLeft;
  FieldY = BoxTop + 4;

  //
  // Clear Input area
  //
  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_BLACK, EFI_LIGHTGRAY));
  for (FieldPos = FieldX; FieldPos < FieldX + FieldLen; FieldPos++) {
    PrintAt (FieldPos, FieldY, L" ");
  }

  //
  // Start to process user input
  //
  Buffer[0] = NUL;
  BufferPos = 0;
  NeedUpdate = FALSE;
  IsDone = FALSE;
  Key.UnicodeChar = 0;
  Key.ScanCode = 0;
  Status = EFI_UNSUPPORTED;
  
  while (!IsDone) {
    Status = ReadKeyStroke (&Key);

    switch (Key.UnicodeChar) {
      
    case CHAR_NULL:
      switch (Key.ScanCode) {

      case SCAN_ESC:
        gST->ConOut->SetAttribute (gST->ConOut, OldAttribute);
        Buffer[0] = NUL;
        IsDone = TRUE;

      default:
        break;
      }

      break;

    case CHAR_CARRIAGE_RETURN:
      IsDone = TRUE;
      break;

    case CHAR_BACKSPACE:
      if (BufferPos) {
        //
        // If not move back beyond string beginning, move all characters behind
        // the current position one character forward
        //
        BufferPos--;
        Buffer[BufferPos] = L'\0';
        NeedUpdate = TRUE;
      }
      break;
      
    default:
      if ((*IsValidChar)(Key.UnicodeChar)) {
      
        if (BufferPos < BufferSize - 1) {        
          Buffer[BufferPos] = Key.UnicodeChar;
          BufferPos++;
          Buffer[BufferPos] = NUL;
          NeedUpdate = TRUE;
        }
      }
    
      break;
    }
    
    //
    // Check if we need update add/remove character
    //
    if (NeedUpdate) {
    
      if (BufferPos > FieldLen) {
        Offset = BufferPos - FieldLen;
      }
      else {
        Offset = 0;
      }
	  
      DialogStrnCpy( DisStr, Buffer + Offset, FieldLen);

      if (StrLen(DisStr) < FieldLen) {
        StrCat(DisStr, L" ");
	  }
      
      gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_BLACK, EFI_LIGHTGRAY));
      PrintAt (FieldX, FieldY, L"%s", DisStr);

      gST->ConOut->SetAttribute (gST->ConOut, OldAttribute);

      NeedUpdate = FALSE;
    }
      
  }

  if (Buffer[0] == NUL) {
    return EFI_NOT_READY;
  }

  return EFI_SUCCESS;
}

