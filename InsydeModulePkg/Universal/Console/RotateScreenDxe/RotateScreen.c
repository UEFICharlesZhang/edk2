/** @file
  RotateScreen

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
#include <Uefi.h>

#include <IndustryStandard/Acpi.h>	//BLD_Quber_029+

#include <Guid/DebugMask.h>

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/SimpleTextInEx.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/AbsolutePointer.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/AcpiTable.h>		//BLD_Quber_029+
#include <Protocol/AcpiSupport.h>	//BLD_Quber_029+


#include "HookLib.h"

VOID *mShellImageCallbackReg = NULL;

typedef struct _ROTATE_SCREEN_GOP_ENTRY {
  UINT32                                    Signature;
  LIST_ENTRY                                Link;

  BOOLEAN                                   NeedFlip;


  EFI_GRAPHICS_OUTPUT_BLT_PIXEL             *GopBuffer;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION      Info;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE         Mode;

  EFI_HANDLE                                GopHandle;
  EFI_GRAPHICS_OUTPUT_PROTOCOL              *GraphicsOutput;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE   OrgQueryMode;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE     OrgSetMode;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT          OrgBlt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE         *OrgMode;

} ROTATE_SCREEN_GOP_ENTRY;

#define ROTATE_SCREEN_GOP_ENTRY_SIGNATURE    SIGNATURE_32 ('r', 's', 'g', 'e')
#define GOP_ENTYRY_FROM_LINK(_link)  CR (_link, ROTATE_SCREEN_GOP_ENTRY, Link, ROTATE_SCREEN_GOP_ENTRY_SIGNATURE)


LIST_ENTRY                      mGopEntryList;
VOID                            *mGopRegistration;

HOOK_DRIVER_BINDING_ENTRY       *mHookGraphicsConsoleEntry;

BOOLEAN                         mEnhanceVideoToVideo = TRUE;
//BLD_Quber_029-BOOLEAN                         mReadyToBoot;
//BLD_Quber_029	+S
BOOLEAN                         mReadyToBoot = FALSE;
BOOLEAN                         mIsBootToWindows = FALSE;
BOOLEAN                         mSkipSetMode = FALSE;
//BLD_Quber_029	+E

EFI_GUID                        gEfiShellProtocolGuid = {0x6302d008, 0x7f9b, 0x4f30, {0x87, 0xac, 0x60, 0xc9, 0xfe, 0xf5, 0xda, 0x4e}};
EFI_GUID                        gEfiShellEnvironment2Guid = {0x47c7b221, 0xc42a, 0x11d2, {0x8e, 0x57, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};


BOOLEAN
EFIAPI
IsRotateScreen (
  VOID
  )
{
  ROTATE_SCREEN_GOP_ENTRY       *Entry;
  LIST_ENTRY                    *Link;

  for (Link = GetFirstNode (&mGopEntryList);
       !IsNull (&mGopEntryList, Link);
       Link = GetNextNode (&mGopEntryList, Link)) {

    Entry = GOP_ENTYRY_FROM_LINK (Link);
    if (Entry->NeedFlip) {
      return TRUE;
    }
  }

  return FALSE;
}


ROTATE_SCREEN_GOP_ENTRY *
EFIAPI
FindGopEntryByHandle (
  IN  EFI_HANDLE                Handle
  )
{
  ROTATE_SCREEN_GOP_ENTRY                *GopEntry;
  LIST_ENTRY                             *Link;

  for (Link = GetFirstNode (&mGopEntryList);
       !IsNull (&mGopEntryList, Link);
       Link = GetNextNode (&mGopEntryList, Link)) {

    GopEntry = GOP_ENTYRY_FROM_LINK (Link);
    if (GopEntry->GopHandle == Handle) {
      return GopEntry;
    }
  }

  return NULL;
}


ROTATE_SCREEN_GOP_ENTRY *
FindGopEntry (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL       *GraphicsOutput
  )
{
  ROTATE_SCREEN_GOP_ENTRY                *GopEntry;
  LIST_ENTRY                             *Link;

  for (Link = GetFirstNode (&mGopEntryList);
       !IsNull (&mGopEntryList, Link);
       Link = GetNextNode (&mGopEntryList, Link)) {

    GopEntry = GOP_ENTYRY_FROM_LINK (Link);
    if (GopEntry->GraphicsOutput == GraphicsOutput) {
      return GopEntry;
    }
  }

  return NULL;
}

VOID
PatchGopMode (
  IN  ROTATE_SCREEN_GOP_ENTRY       *GopEntry
  )
{

  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;

  GraphicsOutput = GopEntry->GraphicsOutput;

  if (GopEntry->OrgMode == NULL && GraphicsOutput->Mode != NULL) {
    CopyMem (&GopEntry->Mode, GraphicsOutput->Mode, sizeof (EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
    CopyMem (&GopEntry->Info, GraphicsOutput->Mode->Info, sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));

    GopEntry->OrgMode               = GraphicsOutput->Mode;

    GopEntry->Mode.MaxMode    = GraphicsOutput->Mode->MaxMode * 2;
    GopEntry->Mode.Info       = &GopEntry->Info;
    GopEntry->Mode.SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

    GraphicsOutput->Mode      = &GopEntry->Mode;
  }
}

EFI_STATUS
EFIAPI
RotateScreenGraphicsConsoleDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                              Status;
  ROTATE_SCREEN_GOP_ENTRY                 *GopEntry;

  GopEntry = FindGopEntryByHandle (ControllerHandle);
  ASSERT (GopEntry != NULL);
  if (GopEntry != NULL) {
    PatchGopMode (GopEntry);
  }

  Status = mHookGraphicsConsoleEntry->Start (This, ControllerHandle, RemainingDevicePath);

  return Status;
}

EFI_STATUS
EFIAPI
RotateScreenGraphicsConsoleDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL           *This,
  IN EFI_HANDLE                            ControllerHandle,
  IN UINTN                                 NumberOfChildren,
  IN EFI_HANDLE                            *ChildHandleBuffer
  )
{
  EFI_STATUS                              Status;
  ROTATE_SCREEN_GOP_ENTRY                 *GopEntry;

  GopEntry = FindGopEntryByHandle (ControllerHandle);
  if (GopEntry != NULL) {
    GopEntry->GraphicsOutput->QueryMode = GopEntry->OrgQueryMode;
    GopEntry->GraphicsOutput->SetMode = GopEntry->OrgSetMode;
    GopEntry->GraphicsOutput->Blt = GopEntry->OrgBlt;
    GopEntry->GraphicsOutput->Mode = GopEntry->OrgMode;
    if (GopEntry->GopBuffer != NULL) {
      FreePool (GopEntry->GopBuffer);
    }
    RemoveEntryList (&GopEntry->Link);
    FreePool (GopEntry);
  }

  Status = mHookGraphicsConsoleEntry->Stop  (This, ControllerHandle, NumberOfChildren, ChildHandleBuffer);

  return Status;
}

EFI_STATUS
RotateScreenQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  )
{
  EFI_STATUS                    Status;
  ROTATE_SCREEN_GOP_ENTRY       *GopEntry;
  BOOLEAN                       NeedFlip;
  UINT32                        HorizontalResolution;
  UINT32                        TargetMode;

  GopEntry = FindGopEntry (This);
  if (GopEntry == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (This == NULL || Info == NULL || SizeOfInfo == NULL || ModeNumber >= This->Mode->MaxMode) {
    return EFI_INVALID_PARAMETER;
  }

  NeedFlip = FALSE;
  TargetMode = ModeNumber;
  if ((ModeNumber >= GopEntry->OrgMode->MaxMode) &&
      (ModeNumber < GopEntry->OrgMode->MaxMode * 2)) {
    ModeNumber -= GopEntry->OrgMode->MaxMode;
    NeedFlip = TRUE;
  }

  Status = GopEntry->OrgQueryMode (
                       This,
                       ModeNumber,
                       SizeOfInfo,
                       Info
                       );

  if (!EFI_ERROR (Status) && NeedFlip) {
    HorizontalResolution          = (*Info)->HorizontalResolution;
    (*Info)->HorizontalResolution = (*Info)->VerticalResolution;
    (*Info)->VerticalResolution   = HorizontalResolution;
    (*Info)->PixelsPerScanLine    = (*Info)->HorizontalResolution;
  }

  return Status;
}

//BLD_Quber_029	+S
EFI_STATUS
RotateScreenBltRotate90 (
  IN  ROTATE_SCREEN_GOP_ENTRY                 *GopEntry,
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
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *This;
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Src;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Dst;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *GopBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *SrcBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *DstBuffer;
  UINT32                        HorizontalResolution;
  UINT32                        VerticalResolution;
  INTN                          X;
  UINTN                         Y;

  This      = GopEntry->GraphicsOutput;
  GopBuffer = GopEntry->GopBuffer;

  HorizontalResolution = This->Mode->Info->HorizontalResolution;
  VerticalResolution   = This->Mode->Info->VerticalResolution;


  Status = EFI_UNSUPPORTED;

  switch (BltOperation) {

  case EfiBltVideoFill:
    This->Mode = GopEntry->OrgMode;
    CopyMem (
      GopBuffer,
      (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)BltBuffer + SourceY * Delta + SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
      sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
      );
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         BltOperation,
                         0,
                         0,
                         DestinationY,
                         HorizontalResolution - (Width + DestinationX),
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    break;

  case EfiBltVideoToBltBuffer:
    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         BltOperation,
                         SourceY,
                         HorizontalResolution - (Width + SourceX),
                         0,
                         0,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    if (EFI_ERROR (Status)) {
      return Status;
    }

    SrcBuffer = GopBuffer + (Width - 1) * Height;
    DstBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)BltBuffer + DestinationY * Delta + DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

    for (Y = 0; Y < Height; Y++) {
      Src = SrcBuffer;
      Dst = DstBuffer;

      X = Width;
      while (--X >= 0) {
        *Dst = *Src;
        Src -= Height;
        Dst++;
      }
      SrcBuffer++;
      DstBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)DstBuffer + Delta);
    }
    break;

  case EfiBltBufferToVideo:
    SrcBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)BltBuffer + SourceY * Delta + SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    DstBuffer = GopBuffer + (Width - 1) * Height;

    for (Y = 0; Y < Height; Y++) {
      Src = SrcBuffer;
      Dst = DstBuffer;

      X = Width;
      while (--X >= 0) {
        *Dst = *Src;
        Src++;
        Dst -= Height;
      }

      SrcBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)SrcBuffer + Delta);
      DstBuffer++;
    }

    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         BltOperation,
                         0,
                         0,
                         DestinationY,
                         HorizontalResolution - (Width + DestinationX),
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    break;

  case EfiBltVideoToVideo:
    //
    // For improve GOP VideoToVideo performance in rotate mode,
    // Use BltBufferToVideo to display
    //
    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         EfiBltVideoToBltBuffer,
                         SourceY,
                         HorizontalResolution - (Width + SourceX),
                         0,
                         0,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    if (EFI_ERROR (Status)) {
      return Status;
    }

    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         EfiBltBufferToVideo,
                         0,
                         0,
                         DestinationY,
                         HorizontalResolution - (Width + DestinationX),
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    break;

  default:
    ASSERT (FALSE);
  }

  return Status;
}

EFI_STATUS
RotateScreenBltRotate270 (
  IN  ROTATE_SCREEN_GOP_ENTRY                 *GopEntry,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           * BltBuffer, OPTIONAL
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
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *This;
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Src;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Dst;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *GopBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *SrcBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *DstBuffer;
  UINT32                        HorizontalResolution;
  UINT32                        VerticalResolution;
  INTN                          X;
  UINTN                         Y;

  This      = GopEntry->GraphicsOutput;
  GopBuffer = GopEntry->GopBuffer;

  HorizontalResolution = This->Mode->Info->HorizontalResolution;
  VerticalResolution   = This->Mode->Info->VerticalResolution;


  Status = EFI_UNSUPPORTED;

  switch (BltOperation) {

  case EfiBltVideoFill:
    This->Mode = GopEntry->OrgMode;
    CopyMem (
      GopBuffer,
      (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)BltBuffer + SourceY * Delta + SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)),
      sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
      );
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         BltOperation,
                         0,
                         0,
                         VerticalResolution - (Height + DestinationY),
                         DestinationX,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    break;

  case EfiBltVideoToBltBuffer:
    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         BltOperation,
                         VerticalResolution - (Height + SourceY),
                         SourceX,
                         0,
                         0,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    if (EFI_ERROR (Status)) {
      return Status;
    }

    SrcBuffer = GopBuffer + (Height - 1);
    DstBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)BltBuffer + DestinationY * Delta + DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

    for (Y = 0; Y < Height; Y++) {
      Src = SrcBuffer;
      Dst = DstBuffer;

      X = Width;
      while (--X >= 0) {
        *Dst = *Src;
        Src += Height;
        Dst++;
      }
      SrcBuffer--;
      DstBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)DstBuffer + Delta);
    }
    break;

  case EfiBltBufferToVideo:
    SrcBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)BltBuffer + SourceY * Delta + SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    DstBuffer = GopBuffer + (Height - 1);

    for (Y = 0; Y < Height; Y++) {
      Src = SrcBuffer;
      Dst = DstBuffer;

      X = Width;
      while (--X >= 0) {
        *Dst = *Src;
        Src++;
        Dst += Height;
      }

      SrcBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)SrcBuffer + Delta);
      DstBuffer--;
    }

    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         BltOperation,
                         0,
                         0,
                         VerticalResolution - (Height + DestinationY),
                         DestinationX,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    break;

  case EfiBltVideoToVideo:
    //
    // For improve GOP VideoToVideo performance in rotate mode,
    // Use BltBufferToVideo to display
    //
    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         EfiBltVideoToBltBuffer,
                         VerticalResolution - (Height + SourceY),
                         SourceX,
                         0,
                         0,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    if (EFI_ERROR (Status)) {
      return Status;
    }

    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         EfiBltBufferToVideo,
                         0,
                         0,
                         VerticalResolution - (Height + DestinationY),
                         DestinationX,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    break;

  default:
    ASSERT (FALSE);
  }

  return Status;
}
//BLD_Quber_029	+E

EFI_STATUS
RotateScreenBlt (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL            * This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           * BltBuffer, OPTIONAL
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

//BLD_Quber_029	-S
/*
  EFI_STATUS                    Status;
  ROTATE_SCREEN_GOP_ENTRY       *GopEntry;
  INTN                          X;
  UINTN                         Y;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Src;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Dst;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *GopBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *SrcBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *DstBuffer;
  UINT32                        HorizontalResolution;
  UINT32                        VerticalResolution;
  EFI_TPL                       OriginalTPL;

  GopEntry = FindGopEntry (This);
  if (GopEntry == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!GopEntry->NeedFlip) {

    OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);
    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                       This,
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
    This->Mode = &GopEntry->Mode;
    gBS->RestoreTPL (OriginalTPL);
    return Status;
  }

  if (BltOperation >= EfiGraphicsOutputBltOperationMax) {
    return EFI_UNSUPPORTED;
  }

  HorizontalResolution = This->Mode->Info->HorizontalResolution;
  VerticalResolution   = This->Mode->Info->VerticalResolution;

  if (HorizontalResolution == 0 || VerticalResolution == 0) {
    return EFI_NOT_READY;
  }

  if ((Width > HorizontalResolution) || (Height > VerticalResolution)) {
    return EFI_INVALID_PARAMETER;
  }

  if (GopEntry->GopBuffer == NULL) {
    GopEntry->GopBuffer = AllocateZeroPool (
                            HorizontalResolution * VerticalResolution *
                            sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                            );
    if (GopEntry->GopBuffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  //
  // check region
  //
  if (BltOperation == EfiBltVideoToBltBuffer || BltOperation == EfiBltVideoToVideo) {
    if ((SourceX >= HorizontalResolution) || (SourceY >= VerticalResolution)) {
      return EFI_INVALID_PARAMETER;
    }

    SourceY = VerticalResolution - (SourceY + Height);
    if (SourceY + Height > VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }

  if (BltOperation != EfiBltVideoToBltBuffer) {
    if ((DestinationX >= HorizontalResolution) || (DestinationY >= VerticalResolution)) {
      return EFI_INVALID_PARAMETER;
    }

    DestinationY =  VerticalResolution - (DestinationY + Height);
    if (DestinationY + Height > VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationX + Width > HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }

  if (Delta == 0) {
    Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }

  GopBuffer = GopEntry->GopBuffer;
  Status = EFI_SUCCESS;

  OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);

  if (BltOperation == EfiBltBufferToVideo) {
    SrcBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8*)BltBuffer + (SourceY + Height - 1) * Delta + SourceX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    DstBuffer = GopBuffer;

    for (Y = 0; Y < Height; Y++) {
      Src = SrcBuffer;
      Dst = DstBuffer;

      X = Width;
      while (--X >= 0) {
        *Dst = *Src;
        Src++;
        Dst += Height;
      }

      SrcBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)SrcBuffer - Delta);
      DstBuffer ++;
    }

    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         BltOperation,
                         0,
                         0,
                         DestinationY,
                         DestinationX,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;

  } else if (BltOperation == EfiBltVideoToBltBuffer) {

    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         BltOperation,
                         SourceY,
                         SourceX,
                         0,
                         0,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    SrcBuffer = GopBuffer;
    DstBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8*)BltBuffer + (DestinationY + Height - 1) * Delta  + DestinationX * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

    for (Y = 0; Y < Height; Y++) {
      Src = SrcBuffer;
      Dst = DstBuffer;

      X = Width;
      while (--X >= 0) {
        *Dst = *Src;
        Src += Height;
        Dst++;
      }
      SrcBuffer++;
      DstBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)((UINT8 *)DstBuffer - Delta);
    }

  } else if (BltOperation == EfiBltVideoToVideo) {

    //
    // For improve GOP VideoToVideo performance in rotate mode,
    // Use BltBufferToVide to display
    //
    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         EfiBltVideoToBltBuffer,
                         SourceY,
                         SourceX,
                         0,
                         0,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         GopBuffer,
                         EfiBltBufferToVideo,
                         0,
                         0,
                         DestinationY,
                         DestinationX,
                         Height,
                         Width,
                         0
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;

  } else if (BltOperation == EfiBltVideoFill) {

    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                         This,
                         BltBuffer,
                         BltOperation,
                         SourceY,
                         SourceX,
                         DestinationY,
                         DestinationX,
                         Height,
                         Width,
                         Delta
                         );
    ASSERT_EFI_ERROR (Status);
    This->Mode = &GopEntry->Mode;
  }

Done:
  gBS->RestoreTPL (OriginalTPL);
  */
  EFI_STATUS                    Status;
  ROTATE_SCREEN_GOP_ENTRY       *GopEntry;
  UINT32                        HorizontalResolution;
  UINT32                        VerticalResolution;
  EFI_TPL                       OriginalTPL;

  GopEntry = FindGopEntry (This);
  if (GopEntry == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!GopEntry->NeedFlip) {

    OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);
    This->Mode = GopEntry->OrgMode;
    Status = GopEntry->OrgBlt (
                       This,
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
    This->Mode = &GopEntry->Mode;
    gBS->RestoreTPL (OriginalTPL);
    return Status;
  }

  if (BltOperation >= EfiGraphicsOutputBltOperationMax) {
    return EFI_UNSUPPORTED;
  }

  HorizontalResolution = This->Mode->Info->HorizontalResolution;
  VerticalResolution   = This->Mode->Info->VerticalResolution;

  if (HorizontalResolution == 0 || VerticalResolution == 0) {
    return EFI_NOT_READY;
  }

  if ((Width > HorizontalResolution) || (Height > VerticalResolution)) {
    return EFI_INVALID_PARAMETER;
  }

  if (GopEntry->GopBuffer == NULL) {
    GopEntry->GopBuffer = AllocateZeroPool (
                            HorizontalResolution * VerticalResolution *
                            sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                            );
    if (GopEntry->GopBuffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  //
  // check range: source is video
  //
  if (BltOperation == EfiBltVideoToBltBuffer || BltOperation == EfiBltVideoToVideo) {
    if ((SourceX >= HorizontalResolution) || (SourceY >= VerticalResolution)) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceY + Height > VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // check range: destination is video
  //
  if (BltOperation != EfiBltVideoToBltBuffer) {
    if ((DestinationX >= HorizontalResolution) || (DestinationY >= VerticalResolution)) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationX + Width > HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationY + Height > VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }

  if (Delta == 0) {
    Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }

  Status = EFI_UNSUPPORTED;


  OriginalTPL = gBS->RaiseTPL (TPL_NOTIFY);

  if (PcdGetBool(PcdH2ORotateScreenIs90DegreeClockwise)) {
    Status =  RotateScreenBltRotate90 (
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
  } else {
    Status = RotateScreenBltRotate270 (
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

  gBS->RestoreTPL (OriginalTPL);
//BLD_Quber_029	-E
  return Status;
}


EFI_STATUS
RotateScreenSetMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL * This,
  IN  UINT32                       ModeNumber
  )
{
  EFI_STATUS                    Status;
  ROTATE_SCREEN_GOP_ENTRY       *GopEntry;
  UINT32                        TargetMode;
  BOOLEAN                       NeedFlip;

  GopEntry = FindGopEntry (This);
  if (GopEntry == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  NeedFlip = FALSE;
  TargetMode = ModeNumber;
  if ((ModeNumber >= GopEntry->OrgMode->MaxMode) &&
      (ModeNumber < GopEntry->OrgMode->MaxMode * 2)) {
    ModeNumber -= GopEntry->OrgMode->MaxMode;
    NeedFlip = TRUE;
  }

  This->Mode = GopEntry->OrgMode;
  This->Blt  = GopEntry->OrgBlt;
  //BLD_Quber_029	+S
  if (mSkipSetMode) {	
    Status = EFI_SUCCESS;
  } else {
  //BLD_Quber_029	+E
    Status = GopEntry->OrgSetMode (
                         This,
                         ModeNumber
                         );
  }//BLD_Quber_029+
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    This->Mode = &GopEntry->Mode;
    This->Blt  = RotateScreenBlt;
    return Status;
  }

  if (GopEntry->GopBuffer != NULL) {
    FreePool (GopEntry->GopBuffer);
  }
  GopEntry->GopBuffer = NULL;

  GopEntry->NeedFlip  = NeedFlip;
  GopEntry->Mode.Mode = This->Mode->Mode;
  GopEntry->Mode.FrameBufferSize = This->Mode->FrameBufferSize;

  if (NeedFlip) {
    GopEntry->Info.HorizontalResolution = This->Mode->Info->VerticalResolution;
    GopEntry->Info.VerticalResolution   = This->Mode->Info->HorizontalResolution;
    GopEntry->Info.PixelsPerScanLine    = GopEntry->Info.HorizontalResolution;
    GopEntry->Mode.Mode                 = ModeNumber + GopEntry->OrgMode->MaxMode;
  } else {
    CopyMem (&GopEntry->Info, This->Mode->Info, sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
  }

  GopEntry->OrgMode = This->Mode;
  This->Mode = &GopEntry->Mode;
  This->Blt  = RotateScreenBlt;
  return Status;
}

EFI_STATUS
RotateScreenHookGop (
  IN EFI_HANDLE                   GopHandle,
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput
  )
{
  ROTATE_SCREEN_GOP_ENTRY       *GopEntry;
  EFI_STATUS                    Status;

  if (mHookGraphicsConsoleEntry == NULL) {
    Status = HookDriverBindingByDriverName (
               L"Graphics Console Driver",
               NULL,
               RotateScreenGraphicsConsoleDriverBindingStart,
               RotateScreenGraphicsConsoleDriverBindingStop,
               &mHookGraphicsConsoleEntry
               );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (FindGopEntry (GraphicsOutput) != NULL) {
    return EFI_SUCCESS;
  }

  GopEntry = AllocateZeroPool (sizeof (ROTATE_SCREEN_GOP_ENTRY));
  if (GopEntry == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  GopEntry->Signature       = ROTATE_SCREEN_GOP_ENTRY_SIGNATURE;
  GopEntry->GopBuffer       = NULL;
  GopEntry->GraphicsOutput  = GraphicsOutput;
  GopEntry->GopHandle       = GopHandle;

  //
  // hook GraphicsOutput
  //
  GopEntry->OrgQueryMode    = GraphicsOutput->QueryMode;
  GopEntry->OrgSetMode      = GraphicsOutput->SetMode;
  GopEntry->OrgBlt          = GraphicsOutput->Blt;

  GraphicsOutput->QueryMode = RotateScreenQueryMode;
  GraphicsOutput->SetMode   = RotateScreenSetMode;
  GraphicsOutput->Blt       = RotateScreenBlt;

  InsertTailList (&mGopEntryList, &GopEntry->Link);

  return EFI_SUCCESS;
}

VOID
RotateScreenGopCallback (
  IN EFI_EVENT                  Event,
  IN VOID                       *Content
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    Handle;
  UINTN                         BufferSize;
  VOID                          *DevicePath;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;

  while (TRUE) {
    BufferSize = sizeof (EFI_HANDLE);
    Status = gBS->LocateHandle (
                    ByRegisterNotify,
                    NULL,
                    mGopRegistration,
                    &BufferSize,
                    &Handle
                    );
    if (EFI_ERROR (Status)) {
      break;
    }

    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiDevicePathProtocolGuid,
                    (VOID **) &DevicePath
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->HandleProtocol (
                    Handle,
                    &gEfiGraphicsOutputProtocolGuid,
                    (VOID **) &GraphicsOutput
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }
    RotateScreenHookGop (Handle, GraphicsOutput);
  }
}

EFI_STATUS
EFIAPI
RegisterRotateTouchCallback (
  VOID
  );


EFI_IMAGE_START                 ImageStart;


BOOLEAN
EFIAPI
IsWindows (
  EFI_HANDLE                    Handle
  );

//BLD_Quber_029	+S
EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE *
GetBgrtTable (
  UINTN                         *TableKey
  )
{
  EFI_STATUS                    Status;
  EFI_ACPI_SUPPORT_PROTOCOL     *AcpiSupport;
  UINTN                         Index;
  EFI_ACPI_DESCRIPTION_HEADER   *Table;
  UINTN                         Handle;
  EFI_ACPI_TABLE_VERSION        Version;

  AcpiSupport = NULL;
  Status = gBS->LocateProtocol (
                  &gEfiAcpiSupportProtocolGuid,
                  NULL,
                  (VOID **)&AcpiSupport
                  );
  if (EFI_ERROR(Status)) {
    return NULL;
  }

  Index = 0;
  while (1) {
    Table = NULL;
    Status = AcpiSupport->GetAcpiTable (AcpiSupport, Index, (VOID **) &Table, &Version, &Handle);
    if (Status == EFI_NOT_FOUND) {
      break;
    }

    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (Table->Signature == EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE_SIGNATURE) {
      *TableKey = Handle;
      return (EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE *)Table;
    }

    FreePool (Table);
    Index++;
  }

  return NULL;
}
//BLD_Quber_029	+E

EFI_STATUS
EFIAPI
RotateScreenStartImage (
  IN EFI_HANDLE  ImageHandle,
  OUT UINTN      *ExitDataSize,
  OUT CHAR16     **ExitData  OPTIONAL
  )
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  ROTATE_SCREEN_GOP_ENTRY       *GopEntry;
  LIST_ENTRY                    *Link;
  UINTN                         Mode;
  UINTN                         MaxMode;
  UINTN                         Columns;
  UINTN                         Rows;
  UINTN                         ExpectColumns;
  UINTN                         ExpectRows;
//BLD_Quber_029	-S
/*
  if (mReadyToBoot) {
    if (IsWindows (ImageHandle)) {
	*/
  EFI_ACPI_TABLE_PROTOCOL                   *AcpiTableProtocol;
  EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE *OldBgrtTable;
  EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE *NewBgrtTable;
  UINTN                                     AcpiTableKey;

  if (!mReadyToBoot) {
    goto Done;
  }

  if (!IsWindows (ImageHandle)) {
    goto Done;
  }
//BLD_Quber_029	-E
      Status = gBS->HandleProtocol (
                      gST->ConsoleOutHandle,
                      &gEfiGraphicsOutputProtocolGuid,
                      &GraphicsOutput
                      );
      if (EFI_ERROR (Status)) {
        goto Done;
      }

      for (Link = GetFirstNode (&mGopEntryList);
           !IsNull (&mGopEntryList, Link);
           Link = GetNextNode (&mGopEntryList, Link)) {

        GopEntry = GOP_ENTYRY_FROM_LINK (Link);
        if (GopEntry->GraphicsOutput->Mode->Info->VerticalResolution == GraphicsOutput->Mode->Info->VerticalResolution &&
            GopEntry->GraphicsOutput->Mode->Info->HorizontalResolution == GraphicsOutput->Mode->Info->HorizontalResolution &&
            !GopEntry->NeedFlip) {
          goto Done;
        }
      }


      ExpectColumns = GraphicsOutput->Mode->Info->VerticalResolution / EFI_GLYPH_WIDTH;
      ExpectRows    = GraphicsOutput->Mode->Info->HorizontalResolution /  EFI_GLYPH_HEIGHT;

      MaxMode = gST->ConOut->Mode->MaxMode;
      for (Mode = 0; Mode < MaxMode; Mode++) {
        Status = gST->ConOut->QueryMode (
                                gST->ConOut,
                                Mode,
                                &Columns,
                                &Rows
                                );
        if (EFI_ERROR (Status)) {
          continue;
        }

//BLD_Quber_029	-S
/*
        if (Columns == ExpectColumns && Rows == ExpectRows) {
          gST->ConOut->SetMode (gST->ConOut, Mode);
          break;
        }
      }
    }
  }
*/
if (Columns == ExpectColumns && Rows == ExpectRows) {
      OldBgrtTable = GetBgrtTable (&AcpiTableKey);
      mSkipSetMode = TRUE;
      gST->ConOut->SetMode (gST->ConOut, Mode);
      mSkipSetMode = FALSE;
      NewBgrtTable = GetBgrtTable (&AcpiTableKey);

      //
      // keep bgrt logo rotated image offset
      //
      if (OldBgrtTable != NULL && NewBgrtTable != NULL) {

        NewBgrtTable->ImageOffsetX = OldBgrtTable->ImageOffsetX;
        NewBgrtTable->ImageOffsetY = OldBgrtTable->ImageOffsetY;

        if (PcdGetBool(PcdH2ORotateScreenIs90DegreeClockwise)) {
          NewBgrtTable->Status |= (OldBgrtTable->Status | 0x02);
        } else {
          NewBgrtTable->Status |= (OldBgrtTable->Status | 0x06);
        }

        Status = gBS->LocateProtocol (
                        &gEfiAcpiTableProtocolGuid,
                        NULL,
                        (VOID**)&AcpiTableProtocol
                        );
        if (!EFI_ERROR (Status)) {

          Status = AcpiTableProtocol->UninstallAcpiTable (
                                        AcpiTableProtocol,
                                        AcpiTableKey
                                        );
          ASSERT_EFI_ERROR (Status);

          AcpiTableKey = 0;
          Status = AcpiTableProtocol->InstallAcpiTable (
                                        AcpiTableProtocol,
                                        NewBgrtTable,
                                        sizeof (EFI_ACPI_5_0_BOOT_GRAPHICS_RESOURCE_TABLE),
                                        &AcpiTableKey
                                        );
          ASSERT_EFI_ERROR (Status);
        }
      }
      if (OldBgrtTable != NULL) {
        FreePool (OldBgrtTable);
      }
      if (NewBgrtTable != NULL) {
        FreePool (NewBgrtTable);
      }
      break;
    }
  }
//BLD_Quber_029	-E
Done:

  Status = ImageStart (ImageHandle, ExitDataSize, ExitData);

  return Status;
}


VOID
EFIAPI
RotateScreenReadyToBootCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  gBS->CloseEvent (Event);

  mReadyToBoot = TRUE;
}

VOID
EFIAPI
ShellImageCallback (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  )
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;
  UINTN                         Mode;
  UINTN                         MaxMode;
  UINTN                         Columns;
  UINTN                         Rows;
  UINTN                         ExpectColumns;
  UINTN                         ExpectRows;
  ROTATE_SCREEN_GOP_ENTRY       *GopEntry;
  LIST_ENTRY                    *Link;
  UINTN                         NoHandles;
  UINTN                         Index;
  EFI_HANDLE                    *HandleBuffer;
  VOID                          *Protocol;

  if (!FeaturePcdGet (PcdH2ORotateShellSupported)) {
    return ;
  }

  Link = GetFirstNode (&mGopEntryList);
  if (IsNull (&mGopEntryList, Link)) {
    return ;
  }

  GopEntry = GOP_ENTYRY_FROM_LINK (Link);
  if (GopEntry->NeedFlip) {
    return ;
  }

  //
  // Locate the Handle that the AbsolutePointer interface is bound to
  //
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &NoHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }

  GraphicsOutput = NULL;
  for (Index = 0; Index < NoHandles; Index++) {

    //
    // for ConSplitter, no device path
    //
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiDevicePathProtocolGuid,
                    &Protocol
                    );
    if (!EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiSimpleTextOutProtocolGuid,
                    &Protocol
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiGraphicsOutputProtocolGuid,
                    &Protocol
                    );
    if (!EFI_ERROR (Status)) {
      GraphicsOutput = Protocol;
      break;
    }
  }

  FreePool (HandleBuffer);
  if (GraphicsOutput == NULL) {
    return ;
  }

  ExpectColumns = GraphicsOutput->Mode->Info->VerticalResolution / EFI_GLYPH_WIDTH;
  ExpectRows    = GraphicsOutput->Mode->Info->HorizontalResolution /  EFI_GLYPH_HEIGHT;

  MaxMode = gST->ConOut->Mode->MaxMode;
  for (Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode (
                            gST->ConOut,
                            Mode,
                            &Columns,
                            &Rows
                            );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (Columns == ExpectColumns && Rows == ExpectRows) {
      gST->ConOut->SetMode (gST->ConOut, Mode);
      break;
    }
  }
}


/*++

Routine Description:
  Install Driver to produce rotate screen

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:

  EFI_SUCCESS - RotateScreen init success

  Other       - No protocol installed, unload driver.

--*/
EFI_STATUS
EFIAPI
RotateScreenInstall (
  IN EFI_HANDLE                 ImageHandle,
  IN EFI_SYSTEM_TABLE           *SystemTable
  )
{
  EFI_STATUS                    Status;
  EFI_EVENT                     Event;
  EFI_TPL                       OrgTpl;
  VOID                          *Registration;

  HookDriverBindingLibConstructor (ImageHandle, SystemTable);
  RegisterRotateTouchCallback ();

  InitializeListHead (&mGopEntryList);
  mHookGraphicsConsoleEntry = NULL;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  RotateScreenGopCallback,
                  NULL,
                  &Event
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }

//BLD_Quber_029	+S
  PcdSetBool (
    PcdH2ORotateScreenIs90DegreeClockwise,
    PcdGetBool (PcdH2ORotateScreenIs90DegreeClockwise)
    );
//BLD_Quber_029	+E
  //
  // Register for protocol notifications on this event
  //
  mGopRegistration = NULL;
  Status = gBS->RegisterProtocolNotify (
                  &gEfiGraphicsOutputProtocolGuid,
                  Event,
                  &mGopRegistration
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (Event);
  }

  //
  // shell 2.0
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ShellImageCallback,
                  NULL,
                  &Event
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  Registration = NULL;
  Status = gBS->RegisterProtocolNotify (
                  &gEfiShellProtocolGuid,
                  Event,
                  &Registration
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (Event);
  }


  //
  // shell 1.0
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ShellImageCallback,
                  NULL,
                  &Event
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  Registration = NULL;
  Status = gBS->RegisterProtocolNotify (
                  &gEfiShellEnvironment2Guid,
                  Event,
                  &Registration
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (Event);
  }


  mReadyToBoot = FALSE;
  Status = EfiCreateEventReadyToBootEx (
             TPL_NOTIFY,
             RotateScreenReadyToBootCallback,
             NULL,
             &Event
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (Event);
  }

  ImageStart = gBS->StartImage;
  gBS->StartImage = RotateScreenStartImage;

  //
  // re-calculate Boot Services header crc
  //
  OrgTpl = gBS->RaiseTPL (TPL_NOTIFY);
  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (
        (UINT8 *) &gBS->Hdr,
        gBS->Hdr.HeaderSize,
        &gBS->Hdr.CRC32
        );
  gBS->RestoreTPL (OrgTpl);

  return Status;
}

