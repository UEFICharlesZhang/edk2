/** @file
  Setup Mouse Protocol implementation

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "SetupMouse.h"

extern PRIVATE_MOUSE_DATA          *mPrivate;

/**
 SetupMouseScreenBltWorker Blt function to process keyboard display

 @param [in]   Private          Setup mouse private data
 @param [in]   GopEntry         Gop Entry
 @param [in]   BltOperation     The operation to perform
 @param [in]   SourceX          The X coordinate of the source for BltOperation
 @param [in]   SourceY          The Y coordinate of the source for BltOperation
 @param [in]   DestinationX     The X coordinate of the destination for BltOperation
 @param [in]   DestinationY     The Y coordinate of the destination for BltOperation
 @param [in]   Width            The width of a rectangle in the blt rectangle in pixels
 @param [in]   Height           The height of a rectangle in the blt rectangle in pixels
 @param [in]   Delta            Not used for EfiBltVideoFill and EfiBltVideoToVideo operation.
                                If a  of 0 is used, the entire BltBuffer will be operated on.
                                If a subrectangle of the BltBuffer is used, then  represents
                                the number of bytes in a row of the BltBuffer.

 @return SetupMouseCursorBlt function status

**/
EFI_STATUS
SetupMouseScreenBltWorker (
  IN  PRIVATE_MOUSE_DATA                 *Private,
  IN  GOP_ENTRY                          *GopEntry,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL      *BltBuffer, OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION  BltOperation,
  IN  UINTN                              SourceX,
  IN  UINTN                              SourceY,
  IN  UINTN                              DestinationX,
  IN  UINTN                              DestinationY,
  IN  UINTN                              Width,
  IN  UINTN                              Height,
  IN  UINTN                              Delta
  )
{
  EFI_STATUS                      Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *Gop;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *ScreenFrameBuffer;
  EFI_TPL                         OriginalTPL;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *VScreen;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *VScreenSrc;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *Blt;
  INTN                            Y;
  UINT32                          FillPixel;
  UINT32                          VerticalResolution;
  UINT32                          HorizontalResolution;
  RECT                            InvalidateRc;
  UINTN                           CopyBytes;

  if ((BltOperation < 0) || (BltOperation >= EfiGraphicsOutputBltOperationMax)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }

  //
  // We need to fill the Virtual Screen buffer with the blt data.
  // The virtual screen is upside down, as the first row is the bootom row of
  // the image.
  //
  Gop                  = GopEntry->GraphicsOutput;
  VerticalResolution   = Gop->Mode->Info->VerticalResolution;
  HorizontalResolution = Gop->Mode->Info->HorizontalResolution;
  ScreenFrameBuffer    = GopEntry->Screen.Image;

  if (BltOperation == EfiBltVideoToBltBuffer) {

    //
    // Video to BltBuffer: Source is Video, destination is BltBuffer
    //
    if (SourceY + Height > VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
    // We would not want a timer based event (Cursor, ...) to come in while we are
    // doing this operation.
    //
    CopyBytes = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

    OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);

    VScreenSrc = ScreenFrameBuffer + SourceY * HorizontalResolution + SourceX;
    Blt        = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)BltBuffer + DestinationY * Delta + DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

    Y = Height;
    while ((--Y) >= 0) {
      CopyMem (Blt, VScreenSrc, CopyBytes);
      VScreenSrc += HorizontalResolution;
      Blt = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)Blt + Delta);
    }

    gBS->RestoreTPL (OriginalTPL);

    return EFI_SUCCESS;
  }

  //
  // BltBuffer to Video: Source is BltBuffer, destination is Video
  //
  if (DestinationY + Height > VerticalResolution) {
    return EFI_INVALID_PARAMETER;
  }

  if (DestinationX + Width > HorizontalResolution) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  CopyBytes = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

  OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);

  switch ((UINTN) BltOperation) {

  case EfiBltBufferToVideo:
    Blt     = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *) BltBuffer + SourceY * Delta + SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    VScreen = ScreenFrameBuffer + DestinationY * HorizontalResolution + DestinationX;

    Y = Height;
    while ((--Y) >= 0) {
      CopyMem (VScreen, Blt, CopyBytes);
      Blt = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)Blt + Delta);
      VScreen += HorizontalResolution;
    }
    break;

  case EfiBltVideoToVideo:
    if (DestinationY <= SourceY) {

      VScreenSrc = ScreenFrameBuffer + SourceY * HorizontalResolution      + SourceX;
      VScreen    = ScreenFrameBuffer + DestinationY * HorizontalResolution + DestinationX;

      Y = Height;
      while ((--Y) >= 0) {
        CopyMem (VScreen, VScreenSrc, CopyBytes);
        VScreenSrc += HorizontalResolution;
        VScreen += HorizontalResolution;
      }
    } else {
      VScreenSrc = ScreenFrameBuffer + (SourceY + Height - 1) * HorizontalResolution      + SourceX;
      VScreen    = ScreenFrameBuffer + (DestinationY + Height - 1) * HorizontalResolution + DestinationX;

      Y = Height;
      while ((--Y) >= 0) {
        CopyMem (VScreen, VScreenSrc, CopyBytes);
        VScreenSrc -= HorizontalResolution;
        VScreen -= HorizontalResolution;
      }
    }
    break;

  case EfiBltVideoFill:
    FillPixel = *(UINT32 *)BltBuffer;
    Blt       = GopEntry->FillLine;

    Y = Width;
    while ((--Y) >= 0) {
      *(UINT32 *)Blt = FillPixel;
      Blt++;
    }

    Blt = GopEntry->FillLine;
    VScreen = ScreenFrameBuffer + DestinationY * HorizontalResolution + DestinationX;

    Y = Height;
    while ((--Y) >= 0) {
      CopyMem (VScreen, Blt, CopyBytes);
      VScreen += HorizontalResolution;
    }
    break;
  }

  SetRect (
    &InvalidateRc,
    (INT32)DestinationX,
    (INT32)DestinationY,
    (INT32)(DestinationX + Width),
    (INT32)(DestinationY + Height)
    );
  UnionRect (&GopEntry->InvalidateRc, &GopEntry->InvalidateRc, &InvalidateRc);
  Status = RenderImage (Private, GopEntry, FALSE);

  gBS->RestoreTPL (OriginalTPL);

  return Status;
}

/**
 SetupMouseCursorBlt Blt function to process keyboard display

 @param [in]   This             Pointer to Graphics Output protocol instance
 @param [in]   BltBuffer        The data to transfer to screen
 @param [in]   BltOperation     The operation to perform
 @param [in]   SourceX          The X coordinate of the source for BltOperation
 @param [in]   SourceY          The Y coordinate of the source for BltOperation
 @param [in]   DestinationX     The X coordinate of the destination for BltOperation
 @param [in]   DestinationY     The Y coordinate of the destination for BltOperation
 @param [in]   Width            The width of a rectangle in the blt rectangle in pixels
 @param [in]   Height           The height of a rectangle in the blt rectangle in pixels
 @param [in]   Delta            Not used for EfiBltVideoFill and EfiBltVideoToVideo operation.
                                If a  of 0 is used, the entire BltBuffer will be operated on.
                                If a subrectangle of the BltBuffer is used, then  represents
                                the number of bytes in a row of the BltBuffer.

 @return SetupMouseCursorBlt function status

**/
EFI_STATUS
EFIAPI
SetupMouseScreenBlt (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL            *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *BltBuffer, OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION       BltOperation,
  IN  UINTN                                   SourceX,
  IN  UINTN                                   SourceY,
  IN  UINTN                                   DestinationX,
  IN  UINTN                                   DestinationY,
  IN  UINTN                                   Width,
  IN  UINTN                                   Height,
  IN  UINTN                                   Delta         OPTIONAL
  )
{
  PRIVATE_MOUSE_DATA            *Private;
  LIST_ENTRY                    *Node;
  GOP_ENTRY                     *GopEntry;

  Private  = mPrivate;
  GopEntry = NULL;

  Node = GetFirstNode (&Private->GopList);
  while (!IsNull (&Private->GopList, Node)) {
    GopEntry = GOP_ENTRY_FROM_THIS (Node);
    if (GopEntry->GraphicsOutput == This) {
      break;
    }
    Node = GetNextNode (&Private->GopList, Node);
  }

  ASSERT (!IsNull (&Private->GopList, Node));
  if (IsNull (&Private->GopList, Node) || GopEntry == NULL) {
    return EFI_UNSUPPORTED;
  }

  return SetupMouseScreenBltWorker (
           Private,
           GopEntry,
           BltBuffer,
           BltOperation,
           SourceX,
           SourceY,
           DestinationX,
           DestinationY,
           Width,
           Height,
           Delta
           );
}


/**
 Copy image to blend image

 @param [in]   Private          Setup mouse private data
 @param [in]   GopEntry         Gop Entry
 @param [in]   Rc               Sync rectangle
 @param [out]  ImageIsSame      Screen Image is same as GOP

 @retval N/A

**/
VOID
SyncScreenImage (
  IN  PRIVATE_MOUSE_DATA  *Private,
  IN  GOP_ENTRY           *GopEntry,
  IN  RECT                *Rc,
  OUT BOOLEAN             *ImageIsSame
  )
{
  INTN                          X;
  INTN                          Y;
  INT32                         Width;
  INT32                         Height;
  UINT32                        *BlendBlt;
  UINT32                        *CheckBlt;
  UINT32                        *ScreenBlt;
  UINTN                         HorizontalResolution;
  BOOLEAN                       IsSame;
  UINT32                        *BlendBuffer;
  UINT32                        *CheckBuffer;
  UINT32                        *ScreenBuffer;


  ASSERT (ImageIsSame != NULL);

  IsSame = TRUE;
  HorizontalResolution = GopEntry->GraphicsOutput->Mode->Info->HorizontalResolution;
  Width  = Rc->right - Rc->left;
  Height = Rc->bottom - Rc->top;

  //
  // read image from gop
  //
  GopEntry->OriginalBlt (
              GopEntry->GraphicsOutput,
              GopEntry->CheckBuffer,
              EfiBltVideoToBltBuffer,
              Rc->left,
              Rc->top,
              Rc->left,
              Rc->top,
              Width,
              Height,
              GopEntry->BytesPerScanLine
              );

  *ImageIsSame = TRUE;
  BlendBuffer  = (UINT32 *) GopEntry->BlendBuffer  + Rc->top * HorizontalResolution + Rc->left;
  CheckBuffer  = (UINT32 *) GopEntry->CheckBuffer  + Rc->top * HorizontalResolution + Rc->left;
  ScreenBuffer = (UINT32 *) GopEntry->Screen.Image + Rc->top * HorizontalResolution + Rc->left;


  Y = Height;
  while ((--Y) >= 0) {
    BlendBlt = BlendBuffer;
    CheckBlt = CheckBuffer;
    ScreenBlt = ScreenBuffer;

    X = Width;
    while ((--X) >= 0) {
      if (*BlendBlt != *CheckBlt) {
        *ScreenBlt = *CheckBlt;
        IsSame = FALSE;
      }
      BlendBlt++;
      CheckBlt++;
      ScreenBlt++;
    }
    BlendBuffer += HorizontalResolution;
    CheckBuffer += HorizontalResolution;
    ScreenBuffer += HorizontalResolution;
  }

  *ImageIsSame = IsSame;
}


/**
 Copy image to blend image

 @param [in]   Private          Setup mouse private data
 @param [in]   GopEntry         Gop Entry
 @param [in]   ImageInfo        Source image information
 @param [in]   UpdateRc         Need update region
 @param [in]   Transparent      Whether have transparent color

 @retval N/A

**/
VOID
UpdateImage (
  IN PRIVATE_MOUSE_DATA   *Private,
  IN GOP_ENTRY            *GopEntry,
  IN IMAGE_INFO           *ImageInfo,
  IN RECT                 *UpdateRc,
  IN BOOLEAN              Transparent
  )
{
  INTN                            X;
  INTN                            Y;
  INT32                           Width;
  INT32                           Height;
  INT32                           ImageWidth;
  INT32                           ImageHeight;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *ImageBlt;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *BlendBlt;
  RECT                            *ImageRc;
  UINTN                           HorizontalResolution;
  UINTN                           CopyBytes;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *ImageBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   *BlendBuffer;



  Width       = UpdateRc->right - UpdateRc->left;
  Height      = UpdateRc->bottom - UpdateRc->top;
  ImageWidth  = ImageInfo->ImageRc.right - ImageInfo->ImageRc.left;
  ImageHeight = ImageInfo->ImageRc.bottom - ImageInfo->ImageRc.top;

  ImageRc     = &ImageInfo->ImageRc;
  HorizontalResolution = GopEntry->GraphicsOutput->Mode->Info->HorizontalResolution;

  ASSERT (Width  <= ImageWidth);
  ASSERT (Height <= ImageHeight);

  ImageBuffer = ImageInfo->Image + (UpdateRc->top - ImageRc->top) * ImageWidth + (UpdateRc->left - ImageRc->left);
  BlendBuffer = GopEntry->BlendBuffer + UpdateRc->top * HorizontalResolution + UpdateRc->left;
  if (!Transparent) {

    CopyBytes = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

    Y = Height;
    while ((--Y) >= 0) {
      CopyMem (BlendBuffer, ImageBuffer, CopyBytes);
      ImageBuffer += ImageWidth;
      BlendBuffer += HorizontalResolution;
    }
  } else {

    ImageBuffer = ImageInfo->Image + (UpdateRc->top - ImageRc->top) * ImageWidth + (UpdateRc->left - ImageRc->left);
    BlendBuffer = GopEntry->BlendBuffer + UpdateRc->top * HorizontalResolution + UpdateRc->left;

    Y = Height;
    while ((--Y) >= 0) {
      ImageBlt = ImageBuffer;
      BlendBlt = BlendBuffer;

      X = Width;
      while ((--X) >= 0) {
        BlendPixel (BlendBlt,  ImageBlt);
        *(UINT32 *)BlendBlt = (0x80000000 | (*(UINT32 *)BlendBlt & 0x00FFFFFF));
        ImageBlt++;
        BlendBlt++;

      }
      ImageBuffer += ImageWidth;
      BlendBuffer += HorizontalResolution;
    }
  }
}

/**
 Render Image to GOP

 @param [in]   Private          Setup mouse private data
 @param [in]   GopEntry         Gop Entry
 @param [in]   NeedSyncScreen   Whether need sync screen image

 @return EFI_STATUS return original BLT function status

**/
EFI_STATUS
RenderImage (
  IN PRIVATE_MOUSE_DATA   *Private,
  IN GOP_ENTRY            *GopEntry,
  IN BOOLEAN              NeedSyncScreen
  )
{

  RECT        UpdateRc;
  RECT        InvalidateRc;
  BOOLEAN     ImageIsSame;
  EFI_STATUS  Status;

  IntersectRect (&InvalidateRc, &GopEntry->InvalidateRc, &GopEntry->Screen.ImageRc);
  if (IsRectEmpty (&InvalidateRc)) {
    return EFI_SUCCESS;
  }

  if (NeedSyncScreen && Private->NeedSyncFrameBuffer) {
    SyncScreenImage (Private, GopEntry, &InvalidateRc, &ImageIsSame);
  }

  if (IntersectRect (&UpdateRc, &GopEntry->Screen.ImageRc, &InvalidateRc)) {
    UpdateImage (Private, GopEntry, &GopEntry->Screen, &UpdateRc, FALSE);
  }

  if (Private->Keyboard.Visible && IntersectRect (&UpdateRc, &Private->Keyboard.ImageRc, &InvalidateRc)) {
    UpdateImage (Private, GopEntry, &Private->Keyboard, &UpdateRc, TRUE);
  }

  if (Private->Cursor.Visible && IntersectRect (&UpdateRc, &Private->Cursor.ImageRc, &InvalidateRc)) {
    UpdateImage (Private, GopEntry, &Private->Cursor, &UpdateRc, TRUE);
  }

  Status = GopEntry->OriginalBlt (
                       GopEntry->GraphicsOutput,
                       GopEntry->BlendBuffer,
                       EfiBltBufferToVideo,
                       InvalidateRc.left,
                       InvalidateRc.top,
                       InvalidateRc.left,
                       InvalidateRc.top,
                       InvalidateRc.right - InvalidateRc.left,
                       InvalidateRc.bottom - InvalidateRc.top,
                       sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * GopEntry->GraphicsOutput->Mode->Info->HorizontalResolution
                       );

  SetRectEmpty (&GopEntry->InvalidateRc);

  return Status;
}

/**
 Render Image for All GOPs

 @param [in]   Private          Setup mouse private data

 @retval N/A

**/
VOID
RenderImageForAllGop (
  IN PRIVATE_MOUSE_DATA         *Private
  )

{
  GOP_ENTRY                     *GopEntry;
  LIST_ENTRY                    *Node;

  ASSERT_LOCKED (&Private->SetupMouseLock);

  Node = GetFirstNode (&Private->GopList);
  while (!IsNull (&Private->GopList, Node)) {
    GopEntry = GOP_ENTRY_FROM_THIS (Node);
    RenderImage (Private, GopEntry, TRUE);
    Node = GetNextNode (&Private->GopList, Node);
  }
}

