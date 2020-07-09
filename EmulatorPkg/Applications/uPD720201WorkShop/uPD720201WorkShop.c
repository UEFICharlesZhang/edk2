/**
 *
 * Copyright (c) 2018, Great Wall Co,.Ltd. All rights reserved.
**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <IndustryStandard/Pci.h>
#include <Protocol/PciIo.h>
#include <Library/TimerLib.h>
#include "uPD720201FwDownload.h"
#include "uPD720201FwUpdate.h"
#include "uPD720201Register.h"

EFI_PCI_IO_PROTOCOL *uPD720201Array[16];
UINTN mCount = 0;

EFI_STATUS
EFIAPI
ListuPD720201(
  )
{
  EFI_STATUS Status;
  UINTN Index;
  EFI_PCI_IO_PROTOCOL *Instance;
  EFI_HANDLE *HandleBuffer;
  UINTN NumberOfHandles;
  PCI_TYPE00 PciConfigHeader;

  UINT32 Data = 0;
  UINT16 ExCtrlSts = 0;
  UINT32 RomParameter;

  Status = gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiPciIoProtocolGuid,
      NULL,
      &NumberOfHandles,
      &HandleBuffer);

  // DEBUG((EFI_D_INFO, "USBPortControl LocateHandleBuffer:%r!\n", Status));

  if (EFI_ERROR(Status))
  {
    return Status;
  }

  for (Index = 0; Index < NumberOfHandles; Index++)
  {
    //
    // Get the device path on this handle
    //
    Status = gBS->HandleProtocol(
        HandleBuffer[Index],
        &gEfiPciIoProtocolGuid,
        (VOID **)&Instance);
    // DEBUG((EFI_D_INFO, "USBPortControl HandleProtocol:%r!\n", Status));

    if (EFI_ERROR(Status))
    {
      // DEBUG((EFI_D_INFO, "USBPortControl fail to handle gEfiPciIoProtocolGuid:%r!\n"));
      return Status;
    }
    //
    // PCI specification states you should check VendorId and Device Id.
    //
    Status = Instance->Pci.Read(
        Instance,
        EfiPciIoWidthUint32,
        0,
        sizeof(PciConfigHeader) / sizeof(UINT32),
        &PciConfigHeader);
    if ((PciConfigHeader.Hdr.VendorId == 0x1912) && (PciConfigHeader.Hdr.DeviceId == 0x0014))
    {
      uPD720201Array[mCount] = Instance;
      mCount++;
    }
  }
  Print(L" xHCI Controller List:\n");
  for (Index = 0; Index < mCount; Index++)
  {
    uPD720201Array[Index]->Pci.Read(uPD720201Array[Index], EfiPciIoWidthUint32, 0x6C, 1, &Data);
    uPD720201Array[Index]->Pci.Read(uPD720201Array[Index], EfiPciIoWidthUint16, PCICNF0F6, 1, &ExCtrlSts);
    ExCtrlSts &= BIT15;
    if (ExCtrlSts)
    {
      uPD720201Array[Index]->Pci.Read(uPD720201Array[Index], EfiPciIoWidthUint32, PCICNF0EC, 1, &RomParameter);
      Print(L"  %d  FW ver:%04X-%04X Rom:0x%08X\n", Index, Data >> 15, Data >> 7, RomParameter);
    }
    else
    {
      Print(L"  %d  FW ver:%04X-%04X No External Rom\n", Index, Data >> 15, Data >> 7);
    }
  }
  DEBUG((EFI_D_INFO, "\n"));
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
ReadFile(
    IN VOID *Buffer,
    IN UINTN *BufferSize,
    IN CHAR16 *OutputName)
{
  EFI_STATUS Status;
  SHELL_FILE_HANDLE FileHandle;
  UINT64 FileSize;
  Status = ShellOpenFileByName(OutputName, &FileHandle, EFI_FILE_MODE_READ, 0);
  DEBUG((EFI_D_INFO, "%a %s Status = %r!\n", __FUNCTION__, OutputName, Status));
  if (EFI_ERROR(Status))
  {
    return Status;
  }
  Status = ShellGetFileSize(FileHandle, &FileSize);
  DEBUG((EFI_D_INFO, "%a ShellGetFileSize =%X Status = %r!\n", __FUNCTION__, FileSize, Status));
  if (EFI_ERROR(Status))
  {
    return Status;
  }
  Status = ShellReadFile(FileHandle, (UINTN *)&FileSize, Buffer);
  DEBUG((EFI_D_INFO, "%a ShellReadFile :%P Status = %r!\n", __FUNCTION__, Buffer, Status));
  if (EFI_ERROR(Status))
  {
    return Status;
  }
  ShellCloseFile(&FileHandle);
  *BufferSize = FileSize;
  return Status;
}
/**
  Write the buffer context to file

  @param  Buffer            Context Buffer.
  @param  BufferSize        Buffer Length been written to
  @param  OutputFileName    the file name that save the buffer context.

  @retval EFI_SUCCESS       Success.
  @retval Others            Fail.

**/
EFI_STATUS
EFIAPI
WriteFile (
  IN VOID     *Buffer,
  IN UINTN    BufferSize,
  IN CHAR16   *OutputName
  )
{
  EFI_STATUS                Status;
  SHELL_FILE_HANDLE         FileHandle;

  Status = ShellOpenFileByName(OutputName, &FileHandle, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0);
  DEBUG((EFI_D_INFO, "%a %s Status = %r!\n", __FUNCTION__, OutputName, Status));
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = ShellWriteFile(FileHandle, &BufferSize, Buffer);
  ShellCloseFile(&FileHandle);

  return Status;
}
EFI_STATUS
EFIAPI
FwDownload(
  UINT32 ControllerNum,
  UINT32 *Buffer, 
  UINT32 BufferSize
  )
{
  EFI_STATUS Status;
  DEBUG((EFI_D_INFO, "%a, ControllerNum:%d, Buffer:%p,BufferSize:%d!\n", __FUNCTION__, ControllerNum, Buffer,BufferSize));
  Status = uPD720201FwDownload(uPD720201Array[ControllerNum], Buffer, BufferSize);
  return Status;
}
EFI_STATUS
EFIAPI
ReadSpiRom(
  UINT32 ControllerNum,
  UINT32 *Buffer, 
  UINT32 *BufferSize
  )
{
  EFI_STATUS Status;
  Status = uPD720201ReadSpiRom(uPD720201Array[ControllerNum], Buffer, BufferSize);
  return Status;
}
EFI_STATUS
EFIAPI
WriteSpiRom(
  UINT32 ControllerNum,
  UINT32 *Buffer, 
  UINT32 BufferSize
  )
{
  EFI_STATUS Status;
  DEBUG((EFI_D_INFO, "%a, ControllerNum:%d, Buffer:%p,BufferSize:%d!\n", __FUNCTION__, ControllerNum, Buffer,BufferSize));
  Status = uPD720201WriteSpiRom(uPD720201Array[ControllerNum],Buffer,BufferSize);
  return Status;
}
EFI_STATUS
EFIAPI
EraseSpiRom(UINT32 ControllerNum)
{
  EFI_STATUS Status;
  DEBUG((EFI_D_INFO, "%a, ControllerNum:%d!\n", __FUNCTION__, ControllerNum));
  Status = uPD720201EraseSpiRom(uPD720201Array[ControllerNum]);
  return Status;  
}
/**
  Print Help message

**/
VOID
EFIAPI
PrintHelp (
  IN  VOID
  )
{
  Print(L" -------  uPD720201 WorkShop by Charles Zhang  -------\n");
  Print(L" Copyright (c) 2020 GreatWall All rigths reserved.<BR>\n");
  Print(L"\n");

  Print(L"Help:\n");
  Print(L" Erase External rom: uPD720201WorkShop  -e   [xHCI Controller Number] \n");
  Print(L" Read  External rom: uPD720201WorkShop  -r   [xHCI Controller Number] [File name] \n");
  Print(L" Write External rom: uPD720201WorkShop  -w   [xHCI Controller Number] [File name] \n");
  Print(L" FW download       : uPD720201WorkShop  -d   [xHCI Controller Number] [File name] \n");
  Print(L"\n");
}
/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param[in] Argc     The number of items in Argv.
  @param[in] Argv     Array of pointers to strings.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS Status;
  UINT32 *Buffer;
  UINT32 BufferSize;
  UINT32 ControllerNum;
  
  Status = ShellInitialize ();

  Buffer = AllocatePool(512*1024); //512K for read buffer
  if (Argc < 2) {
    PrintHelp();
    ListuPD720201();
    return EFI_INVALID_PARAMETER;
  }
  ListuPD720201();

  if (StrCmp(Argv[1], L"-r") == 0)
  {
    if (Argc != 4)
    {
      Print(L"Incorrect parameter count.\n");
      return EFI_INVALID_PARAMETER;
    }
    ControllerNum = (UINT32)StrHexToUintn(Argv[2]);
    Print(L"Read ControllerNum:%d\n", ControllerNum);
    if(ControllerNum > mCount)
    {
      return EFI_INVALID_PARAMETER;
    }
    Status = ReadSpiRom(ControllerNum, Buffer, &BufferSize);
    Print(L"Save Buffer to file:%s\n", Argv[3]);
    Status = WriteFile (Buffer, BufferSize, Argv[3]);
    if (!EFI_ERROR(Status))
    {
      Print(L"Read External Rom Successful!\n");
    }else
    {
      Print(L"Read External Rom Fail!\n");
    }
    return Status;
  }

  if (StrCmp(Argv[1], L"-d") == 0)
  {
    if (Argc != 4)
    {
      Print(L"Incorrect parameter count.\n");
      return EFI_INVALID_PARAMETER;
    }
    ControllerNum = (UINT32)StrHexToUintn(Argv[2]);
    Print(L"FWdownload to Controller:%d\n", ControllerNum);
    if (ControllerNum > mCount)
    {
      return EFI_INVALID_PARAMETER;
    }
    Print(L"Load file       :%s\n", Argv[3]);
    Status = ReadFile(Buffer, (UINTN *)&BufferSize, Argv[3]);
    if (EFI_ERROR(Status))
    {
      return Status;
    }
    Status = FwDownload(ControllerNum, Buffer, BufferSize);
    if (!EFI_ERROR(Status))
    {
      Print(L"Read External Rom Successful!\n");
    }else
    {
      Print(L"Read External Rom Fail!\n");
    }    
    return Status;
  }
  if (StrCmp(Argv[1], L"-w") == 0)
  {
    if (Argc != 4)
    {
      Print(L"Incorrect parameter count.\n");
      return EFI_INVALID_PARAMETER;
    }
    ControllerNum = (UINT32)StrHexToUintn(Argv[2]);
    Print(L"Write Controller:%d\n", ControllerNum);
    if (ControllerNum > mCount)
    {
      return EFI_INVALID_PARAMETER;
    }
    Print(L"Load file       :%s\n", Argv[3]);
    Status = ReadFile(Buffer, (UINTN *)&BufferSize, Argv[3]);
    if (EFI_ERROR(Status))
    {
      return Status;
    }
    Status = WriteSpiRom(ControllerNum, Buffer, BufferSize);
    if (!EFI_ERROR(Status))
    {
      Print(L"Write External Rom Successful!\n");
    }else
    {
      Print(L"Write External Rom Fail!\n");
    }
    return Status;
  }
  if (StrCmp(Argv[1], L"-e") == 0)
  {
    if (Argc != 3)
    {
      Print(L"Incorrect parameter count.\n");
      return EFI_INVALID_PARAMETER;
    }
    ControllerNum = (UINT32)StrHexToUintn(Argv[2]);
    Print(L"Erase Controller:%d\n", ControllerNum);
    if (ControllerNum > mCount)
    {
      return EFI_INVALID_PARAMETER;
    }
    Status = EraseSpiRom(ControllerNum);
    if (!EFI_ERROR(Status))
    {
      Print(L"Write External Rom Successful!\n");
    }else
    {
      Print(L"Write External Rom Fail!\n");
    }
    return Status;
  }
  return EFI_SUCCESS;
}