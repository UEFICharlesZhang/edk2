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
#include <Protocol/SimpleNetwork.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/TimerLib.h>

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
SendSnpPackage(void)
{
  EFI_STATUS Status;

  EFI_HANDLE *HandleBuffer;
  UINTN NumberOfHandles;
  UINTN Index;
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;

  UINTN BuffSize;
  UINT8 Data[42] = {/*tar mac */0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,/*src mac */0x1, 0x2, 0x3, 0x4, 0x5, 0x6,/*Protocol */0x08,0x06, 
  0x00, 0x01,  0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
  /*src mac */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, /*src ip */0xc0,0xa8,1,1,
  /*tar mac */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*tar ip */0xc0,0xa8,1,2};

  Print(L"SendSnpPackage start!\n");

  // Locate all the Firmware Volume protocols.
  Status = gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiSimpleNetworkProtocolGuid,
      NULL,
      &NumberOfHandles,
      &HandleBuffer);
  Print(L"  LocateHandleBuffer status:%r\n", Status);

  if (EFI_ERROR(Status))
  {
    return Status;
  }

  // Looking for FV with ACPI storage file
  for (Index = 0; Index < NumberOfHandles; Index++)
  {
    //
    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    //
    Status = gBS->HandleProtocol(
        HandleBuffer[Index],
        &gEfiSimpleNetworkProtocolGuid,
        (VOID **)&Snp);
    Print(L"    HandleProtocol index %d status:%r\n", Index, Status);

    if (EFI_ERROR(Status))
    {
      return Status;
    }

    BuffSize = sizeof(Data);
    Status = Snp->Transmit(Snp, 0, BuffSize, Data, NULL,NULL,NULL);
    Print(L"    Snp.Transmit status:%r\n", Status);
  }
  Print(L"SendSnpPackage End!\n\n");

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
  while(1)
  {
    SendSnpPackage();
    MicroSecondDelay(1000 * 1000);
  }
  return EFI_SUCCESS;
}
