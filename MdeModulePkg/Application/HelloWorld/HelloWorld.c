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
#include <Library/PrintLib.h>

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Library/UefiRuntimeServicesTableLib.h>


//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);

VOID
DumpBootOption(void)
{
  EFI_STATUS Status;

  CHAR16 OptionName[22];

  UINT8 *Variable;
  UINT8 *VariablePtr;
  UINTN VariableSize;
  CHAR16 *Description;
  CHAR16 *BIOS_NAME;

  BIOS_NAME = AllocateCopyPool(StrSize(L"GW_BIOS"),L"GW_BIOS");

  for (UINT16 OptionNumber = 0; OptionNumber < 0xFF; OptionNumber++)
  {
    UnicodeSPrint(OptionName, sizeof(OptionName), L"Boot%04x", OptionNumber);

    Status = GetVariable2(
        OptionName,
        &gEfiGlobalVariableGuid,
        (VOID **)&Variable,
        &VariableSize);

    if(!EFI_ERROR (Status)){
    VariablePtr = Variable;

    //
    // Skip the option attribute
    //
    VariablePtr += sizeof(UINT32);

    //
    // Skip the option's device path size
    //
    VariablePtr += sizeof(UINT16);

    //
    // Find the option's description string
    //
    Description = (CHAR16 *)VariablePtr;

    Print(L"Get Boot %d Name:%s \n", OptionNumber, Description);
    if (CompareMem(Description, BIOS_NAME, sizeof(BIOS_NAME)) == 0)
    {
      Print(L"Found BIOS Flash flag\n");
      Status = gRT->SetVariable(OptionName, &gEfiGlobalVariableGuid, 0, 0, NULL);
      Print(L"Delete BIOS Flash flag status:%r\n", Status);
    }
  }
}
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

  DumpBootOption();

  return EFI_SUCCESS;
}
