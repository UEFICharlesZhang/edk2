/*++
Copyright (c) 2011 Byosoft Corporation. All rights reserved.

Module Name:

  SmiVariable.c

Abstract:

 Provides Access to flash backup Services through SMI
--*/

#include <FlashUpdate.h>

#define UPDATE_HEADER   1
#define UPDATE_BIOS     2
#define UPDATE_ALL      3

#define  SIZE_64KB            0x00010000
#define  UBOOT_START_ADDRESS  0
#define  BIOS_START_ADDRESS   0x100000

EFI_DEVICE_PATH_PROTOCOL  *gAppPath;


EFI_STATUS
ShowCopyRightsAndWarning ()
{
  UINTN   Columns, Rows;
  UINTN   StringLen;
  UINTN   Index;
  CHAR16  *String[] = {
    L"************************************************************\n",
    L"*                  Byosoft Flash Update                    *\n",
    L"*        Copyright(C) 2006-2016, Byosoft Co.,Ltd.          *\n",
    L"*                  All rights reserved                     *\n",
    L"************************************************************\n"
  };

  // gST->ConOut->SetAttribute (gST->ConOut, EFI_BACKGROUND_BLACK | EFI_YELLOW);
  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &Columns, &Rows);
  StringLen = StrLen(String[0]);
  Columns   = (Columns - StringLen) / 2;
  Rows      = 0;
  //
  // Show Byosoft copyrights!
  //
  gST->ConOut->ClearScreen (gST->ConOut);
  for (Index = 0; Index < 5; Index ++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, Columns, Rows ++);
    Print(String[Index]);
  }
  Print(L"\n");

  // gST->ConOut->SetAttribute (gST->ConOut, EFI_BACKGROUND_BLACK | EFI_WHITE | EFI_BRIGHT);

  return EFI_SUCCESS;
}


void
PrintUsage()
{
  CHAR16  *DefaultName = L"ByoShellFlash64.efi";

  Print(L"  Byosoft Flash Update Utility Build Date: %a\n",__DATE__);

  Print(L"  Copyright (C) 2006 - 2016, Byosoft Co.,Ltd. All rights reserved.\n");

  Print(L"Calling convention:\n");

  Print(L"  %s [switches] <File Path>\n", DefaultName);

  Print(L"You can use following switches:\n");

  Print(L"  -h Update bios header space(uboot header).\n");
  Print(L"  -b Update bios space.\n\r");
  Print(L"  -a Update all flash space(header & bios).\n");

  Print(L"  -help or -? Show help message, <File Path> will be ignored.\n");
  Print(L"Example:\n");

  Print(L"  %s -b bios.fd\n",DefaultName);
  Print(L"  Means update bios space with file bios.fd\n");
}

EFI_STATUS
UpdateSpi (
  IN UINTN                     Address,
  IN UINT8                     *Buffer, 
  IN UINTN                     Size,
  IN CHAR16                    *String
)
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       Count;
  VOID        *TempBuffer = NULL;

  TempBuffer = AllocatePool(SIZE_64KB);
  Status = EFI_SUCCESS;
  if ((Size % SIZE_64KB) == 0) {
    Count = (Size / SIZE_64KB);
  } else {
    Count = (Size / SIZE_64KB) + 1;
  }
  Print(L"%s%02d%%%",String, 1);
  for (Index = 0; Index < Count; Index ++) {
    if (TempBuffer != NULL) {
      Read (Address + Index * SIZE_64KB, TempBuffer, SIZE_64KB);
      if (CompareMem(TempBuffer, Buffer + Index * SIZE_64KB, SIZE_64KB) == 0) {
        Print(L"\r%s %02d%%",String, ((Index + 1) * 100) / Count);
        continue;
      }
    }
    Status = EraseWrite (
               Address + Index * SIZE_64KB, 
               Buffer + Index * SIZE_64KB,
               SIZE_64KB
               );
    if (EFI_ERROR(Status)) {
      Print(L"\r%s Fail!\n",String);
      goto ProExit;
    }
    Print(L"\r%s %02d%%",String, ((Index + 1) * 100) / Count);
  }
  Print(L"\r%s Success!\n",String);
  
ProExit:
  if (TempBuffer != NULL) {
    FreePool (TempBuffer);
  }
  return Status;
}

EFI_STATUS
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS                Status;
  UINT8                     UpdateMode;
  UINT8                     *Buffer = NULL;
  UINTN                     FileSize;

  if (Argc != 3) {
    PrintUsage ();
    Status = EFI_INVALID_PARAMETER;
    goto ProExit;
  }

  UpdateMode = 0xFF;
  if (StrCmp (Argv[1], L"-h") == 0) {
    UpdateMode = UPDATE_HEADER;
  } else if (StrCmp (Argv[1], L"-b") == 0) {
    UpdateMode = UPDATE_BIOS;
  } else if (StrCmp (Argv[1], L"-a") == 0) {
    UpdateMode = UPDATE_ALL;
  }
  if (UpdateMode == 0xFF) {
    PrintUsage ();
    Status = EFI_INVALID_PARAMETER;
    goto ProExit;
  }

  ShowCopyRightsAndWarning ();
  Print(L"  Reading file........... ");
  Status = ReadFile (Argv[2], &Buffer, &FileSize);
  Print(L"%r!\n", Status);
  if (EFI_ERROR(Status)) {
    goto ProExit;
  }
  if ((FileSize <= BIOS_START_ADDRESS) && (UpdateMode != UPDATE_HEADER)) {
    Print(L"File %s size(%x) shoud be greater than 1M!\n",Argv[2], FileSize);
    goto ProExit;
  }

  switch (UpdateMode) {
    case UPDATE_HEADER:
      Status = UpdateSpi (
                 UBOOT_START_ADDRESS, 
                 Buffer, 
                 BIOS_START_ADDRESS,
                 L"  Updating bios header..."
                 );
      break;

    case UPDATE_BIOS:
      DEBUG((EFI_D_INFO, "update BIOS address:%x,size:%x\n", BIOS_START_ADDRESS, FileSize - BIOS_START_ADDRESS));
      Status = UpdateSpi (
                 BIOS_START_ADDRESS, 
                 Buffer + BIOS_START_ADDRESS, 
                 FileSize - BIOS_START_ADDRESS,
                 L"  Updating bios space ..."
                 );
      break;

    case UPDATE_ALL:
      Status = UpdateSpi (
                 UBOOT_START_ADDRESS, 
                 Buffer, 
                 BIOS_START_ADDRESS,
                 L"  Updating bios header..."
                 );
      if (!EFI_ERROR(Status)) {
        Status = UpdateSpi (
                   BIOS_START_ADDRESS, 
                   Buffer + BIOS_START_ADDRESS, 
                   FileSize - BIOS_START_ADDRESS,
                   L"  Updating bios space ..."
                   );
      }
      break;

  default:
    Status = EFI_INVALID_PARAMETER;
    break;
  }

ProExit:
  if (Buffer) {
    FreePool (Buffer);
  }

  if (!EFI_ERROR (Status)) {
    Print(L"\nBIOS has been updated, system will reboot now!\n");
    // stall 2s
    gBS->Stall (2000 * 1000);
    gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
  }

  return Status;
}



