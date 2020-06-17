/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Protocol/GraphicsOutput.h>

//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);
EFI_STATUS
EFIAPI
SetDifferentReslution()
{
  EFI_STATUS Status;
  UINTN GopHandleCount;
  EFI_HANDLE *GopHandleBuffer;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
  UINTN Index;
  UINT32 ModeNumber, MaxGopMode;
  UINTN                                 SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info;

  Status = gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiGraphicsOutputProtocolGuid,
      NULL,
      &GopHandleCount,
      &GopHandleBuffer);
  if (!EFI_ERROR(Status))
  {
    //
    // Add all the child handles as possible Console Device
    //
    for (Index = 0; Index < GopHandleCount; Index++)
    {
      Status = gBS->HandleProtocol(GopHandleBuffer[Index],
                                   &gEfiGraphicsOutputProtocolGuid, (VOID **)&GraphicsOutput);
      if (EFI_ERROR(Status))
      {
        continue;
      }
      Print (L"Gop :%d\n",Index);
      MaxGopMode = GraphicsOutput->Mode->MaxMode;
      for (ModeNumber = 0; ModeNumber < MaxGopMode; ModeNumber++)
      {
        Status = GraphicsOutput->SetMode (GraphicsOutput, ModeNumber);
        Status = GraphicsOutput->QueryMode(
            GraphicsOutput,
            ModeNumber,
            &SizeOfInfo,
            &Info);
        Print (L"  Mode :%d\n",ModeNumber);
        Print (L"    Version:%x\n",Info->Version);
        Print (L"    HorizontalResolution:%d\n",Info->HorizontalResolution);
        Print (L"    VerticalResolution:%d\n",Info->VerticalResolution);
        gBS->Stall(3*1000*1000);
      }
    }
  }
  return Status;
}
/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32 Index;

  Index = 0;

  //
  // Three PCD type (FeatureFlag, UINT32 and String) are used as the sample.
  //
  if (FeaturePcdGet (PcdHelloWorldPrintEnable)) {
    for (Index = 0; Index < PcdGet32 (PcdHelloWorldPrintTimes); Index ++) {
      //
      // Use UefiLib Print API to print string to UEFI console
      //
      Print ((CHAR16*)PcdGetPtr (PcdHelloWorldPrintString));
    }
  }

  SetDifferentReslution();

  return EFI_SUCCESS;
}
