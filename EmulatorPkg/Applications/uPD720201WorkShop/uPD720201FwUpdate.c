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
#include "uPD720201FwUpdate.h"
#include "uPD720201Register.h"


EFI_STATUS
EFIAPI
CheckCurrentVersion(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT32 ControlRegister;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, 0x6C, 1, &ControlRegister);
  DEBUG((EFI_D_INFO, "%a, Version:%x-%x\n", __FUNCTION__, ControlRegister >> 15, ControlRegister >> 7));
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
WriteFWData0(IN EFI_PCI_IO_PROTOCOL *PciIo, UINT32 Data)
{
  DEBUG((EFI_D_INFO, "%a, write data:%08x\n",__FUNCTION__,Data));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint32, PCICNF0F8, 1, &Data);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
WriteFWData1(IN EFI_PCI_IO_PROTOCOL *PciIo, UINT32 Data)
{
  DEBUG((EFI_D_INFO, "%a, write data:%08x\n",__FUNCTION__,Data));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint32, PCICNF0FC, 1, &Data);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
ReadFWData0(IN EFI_PCI_IO_PROTOCOL *PciIo, UINT32 *Data)
{
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, PCICNF0F8, 1, Data);
  DEBUG((EFI_D_INFO, "%a:%08x\n",__FUNCTION__,*Data));
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
ReadFWData1(IN EFI_PCI_IO_PROTOCOL *PciIo, UINT32 *Data)
{
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, PCICNF0FC, 1, Data);
  DEBUG((EFI_D_INFO, "%a:%08x\n",__FUNCTION__,*Data));
  return EFI_SUCCESS;
}
BOOLEAN
CheckExternalRom(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  ControlRegister &= PCICNF0F6_ROMEXIST;
  if (ControlRegister)
  {
    DEBUG((EFI_D_INFO, "%a, External ROM attached!\n", __FUNCTION__));
    return TRUE;
  }
  else
  {
    DEBUG((EFI_D_INFO, "%a, NO External ROM !\n", __FUNCTION__));
    return FALSE;
  }
}
EFI_STATUS
EFIAPI
SetExternalROMAccessEnable(IN EFI_PCI_IO_PROTOCOL *PciIo,BOOLEAN Flag)
{
  UINT16 ControlRegister;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  if (Flag == 1)
  {
    ControlRegister |= PCICNF0F6_ROMENABLE;
  }
  else
  {
    ControlRegister &= ~PCICNF0F6_ROMENABLE;
  }
  DEBUG((EFI_D_INFO, "%a, ControlRegister:%x\n",__FUNCTION__,ControlRegister));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  return EFI_SUCCESS;
}
UINT32
ConfirmResult(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 0;
  UINT16 ResuleCode = 0;

  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  ResuleCode = (ControlRegister >> PCICNF0F6_RESULT_B) & 0x07;
  DEBUG((EFI_D_INFO, "%a, ResuleCode:%x\n",__FUNCTION__,ResuleCode));
  return (UINT32)ResuleCode;
}
EFI_STATUS
EFIAPI
PrepareRom(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT32 ControlRegister;
  UINT32 RomParameter;

  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, PCICNF0EC, 1, &ControlRegister);
  DEBUG((EFI_D_INFO, "%a, ROM Information:%x\n",__FUNCTION__,ControlRegister));
  switch (ControlRegister)
  {
  case 0x00C22210:
  case 0x00C22211:
    RomParameter = 0x500;
    break;
  case 0x00202010:
  case 0x00202011:
    RomParameter = 0x750;
    break;
  case 0x00202012:
  case 0x00202013:
    RomParameter = 0x760;
    break;
  case 0x00BF0048:
  case 0x00BF0049:
    RomParameter = 0x10791;
    break;
  default:
    RomParameter = 0x700;
    break;
  }
  DEBUG((EFI_D_INFO, "%a, RomParameter:%x\n",__FUNCTION__,RomParameter));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint32, PCICNF0F0, 1, &RomParameter);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
PrepareHostController(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT32 ControlRegister = 1;
  UINT32 FwMagicCode = 0x53524F4D;
  UINT32 TimeOut = GENERIC_TIMEOUT;

  if(CheckExternalRom(PciIo))
  {
    PrepareRom(PciIo);
    WriteFWData0(PciIo, FwMagicCode);
    MicroSecondDelay(1000);
    SetExternalROMAccessEnable(PciIo,TRUE);
  }else
  {
    DEBUG((EFI_D_INFO, "%a, No ExternalRom\n", __FUNCTION__));
    return EFI_UNSUPPORTED;
  }

  while ((ControlRegister != 0) && (TimeOut > 0))
  {
    ControlRegister = ConfirmResult(PciIo);
    TimeOut--;
  }
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
WaitSetData0to0(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 1;
  UINT32 TimeOut = GENERIC_TIMEOUT;

  while ((ControlRegister != 0) && (TimeOut > 0))
  {
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
    ControlRegister &= PCICNF0F6_ROMSETDATA0;
    MicroSecondDelay(1000);
    TimeOut--;
  }
  DEBUG((EFI_D_INFO, "%a, ControlRegister:%x\n",__FUNCTION__,ControlRegister));
  if (TimeOut == 0)
  {
    return EFI_TIMEOUT;
  }
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
WaitSetData1to0(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 1;
  UINT32 TimeOut = GENERIC_TIMEOUT;

  while ((ControlRegister != 0) && (TimeOut > 0))
  {
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
    ControlRegister &= PCICNF0F6_ROMSETDATA1;
    MicroSecondDelay(1000);
    TimeOut--;
  }
  DEBUG((EFI_D_INFO, "%a, ControlRegister:%x\n",__FUNCTION__,ControlRegister));
  if (TimeOut == 0)
  {
    return EFI_TIMEOUT;
  }
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
SetData0(IN EFI_PCI_IO_PROTOCOL *PciIo, BOOLEAN Flag)
{
  UINT16 ControlRegister = 0;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  DEBUG((EFI_D_INFO, "%a, Orginal:%x\n",__FUNCTION__,ControlRegister));
  ControlRegister &= ~PCICNF0F6_ROMSETDATA0;
  if(Flag != 0)
  ControlRegister |= PCICNF0F6_ROMSETDATA0;
  DEBUG((EFI_D_INFO, "%a, Target :%x\n",__FUNCTION__,ControlRegister));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
SetData1(IN EFI_PCI_IO_PROTOCOL *PciIo, BOOLEAN Flag)
{
  UINT16 ControlRegister = 0;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  DEBUG((EFI_D_INFO, "%a, Orginal:%x\n",__FUNCTION__,ControlRegister));
  ControlRegister &= ~PCICNF0F6_ROMSETDATA1;
  if(Flag != 0)
  ControlRegister |= PCICNF0F6_ROMSETDATA1;
  DEBUG((EFI_D_INFO, "%a, Target :%x\n",__FUNCTION__,ControlRegister));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
GetData0(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 0;
  UINT32 TimeOut = GENERIC_TIMEOUT;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  DEBUG((EFI_D_INFO, "%a, Orginal:%x\n", __FUNCTION__, ControlRegister));
  ControlRegister |= PCICNF0F6_ROMGETDATA0;
  DEBUG((EFI_D_INFO, "%a, Target :%x\n", __FUNCTION__, ControlRegister));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  do
  {
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
    ControlRegister &= PCICNF0F6_ROMGETDATA0;
    MicroSecondDelay(1000);
    TimeOut--;
  } while ((ControlRegister != 0) && (TimeOut > 0));
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
GetData1(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 0;
  UINT32 TimeOut = GENERIC_TIMEOUT;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  DEBUG((EFI_D_INFO, "%a, Orginal:%x\n",__FUNCTION__,ControlRegister));
  ControlRegister |= PCICNF0F6_ROMGETDATA1;
  DEBUG((EFI_D_INFO, "%a, Target :%x\n",__FUNCTION__,ControlRegister));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  do
  {
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
    ControlRegister &= PCICNF0F6_ROMGETDATA1;
    MicroSecondDelay(1000);
    TimeOut--;
  } while ((ControlRegister != 0) && (TimeOut > 0));
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
uPD720201WriteSpiRom(
  IN EFI_PCI_IO_PROTOCOL          *PciIo,
  IN UINT32 *FwBuffer, 
  IN UINTN BufferSize
  )
{
  UINT32 i = 0;
  UINT32 Result = 0;
  EFI_STATUS Status;
  SET_DATA set_data_page = SET_DATA_PAGE0;
  UINT32 TimeOut = GENERIC_TIMEOUT;
  //Step1
  Status = PrepareHostController(PciIo);
  if (Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_INFO, "%a, Not Support!\n", __FUNCTION__));
    return EFI_UNSUPPORTED;
  }
  DEBUG((EFI_D_INFO, "\n"));
  // Write all the dwords, one pair at a time
  for (i = 0; i < (BufferSize / 4); i++)
  {
    DEBUG((EFI_D_INFO, "%a, Update index:%d!\n", __FUNCTION__,i));
    switch (set_data_page)
    {
    case SET_DATA_PAGE0:
      //Step7
      WaitSetData0to0(PciIo);

      //Step8
      WriteFWData0(PciIo, FwBuffer[i]);
      SetData0(PciIo, 1);

      set_data_page = SET_DATA_PAGE1;
      break;
    case SET_DATA_PAGE1:
      //Step9
      WaitSetData1to0(PciIo);

      //Step10
      WriteFWData1(PciIo, FwBuffer[i]);
      SetData1(PciIo, 1);

      set_data_page = SET_DATA_PAGE0;
      break;
    default:
      break;
    }
    DEBUG((EFI_D_INFO, "\n"));
  }
  //Repeat End

  SetExternalROMAccessEnable(PciIo, FALSE);

  //Step13
  do
  {
    Result = ConfirmResult(PciIo);
    MicroSecondDelay(10 * 1000);
    TimeOut--;
  } while ((Result != 0x01) && (TimeOut > 0));

  if (Result == 0x01)
  {
    DEBUG((EFI_D_INFO, "%a, FW Update Success!\n", __FUNCTION__));
    return EFI_SUCCESS;
  }
  else
  {
    DEBUG((EFI_D_INFO, "%a, FW Update Failed!\n", __FUNCTION__));
    return EFI_DEVICE_ERROR;
  }
}

EFI_STATUS
EFIAPI
uPD720201ReadSpiRom (
  IN EFI_PCI_IO_PROTOCOL          *PciIo,
  UINT32 *Buffer, 
  UINT32 *BufferSize
  )
{
  UINT32 i;
  SET_DATA set_data_page = SET_DATA_PAGE0;
  PrepareHostController(PciIo);
  DEBUG((EFI_D_INFO, "\n"));

  for (i = 0; i < (512*1024 / 4); i++)
  {
    DEBUG((EFI_D_INFO, "%a, Read index:%d!\n", __FUNCTION__,i));
    switch (set_data_page)
    {
    case SET_DATA_PAGE0:
      GetData0(PciIo);
      ReadFWData0(PciIo,&Buffer[i]);

      set_data_page = SET_DATA_PAGE1;
      break;
    case SET_DATA_PAGE1:
      GetData1(PciIo);
      ReadFWData1(PciIo,&Buffer[i]);

      set_data_page = SET_DATA_PAGE0;
      break;
    default:
      break;
    }
    DEBUG((EFI_D_INFO, "\n"));
    if (Buffer[i] == 0xFFFFFFFF)
    {
      *BufferSize = i * 4;
      DEBUG((EFI_D_INFO, "%a, Total Read:%d Byte!\n", __FUNCTION__, *BufferSize));
      break;
    }
  }
  SetExternalROMAccessEnable(PciIo, FALSE);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
uPD720201EraseSpiRom(
IN EFI_PCI_IO_PROTOCOL          *PciIo
  )
{
  UINT32 ControlRegister;
  UINT32 FwMagicCode = 0x5A65726F;
  UINT32 TimeOut = GENERIC_TIMEOUT;

  if(CheckExternalRom(PciIo))
  {
    PrepareRom(PciIo);
    WriteFWData0(PciIo, FwMagicCode);
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
    ControlRegister |= PCICNF0F6_ROMERASE;
    PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
  }else
  {
    DEBUG((EFI_D_INFO, "%a, No ExternalRom\n", __FUNCTION__));
    return EFI_UNSUPPORTED;
  }
  while ((ControlRegister != 0) && (TimeOut > 0))
  {
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F6, 1, &ControlRegister);
    ControlRegister &= PCICNF0F6_ROMERASE;
    DEBUG((EFI_D_INFO, "%a, ControlRegister:%X!\n", __FUNCTION__, ControlRegister));
    MicroSecondDelay(10*1000);
    TimeOut--;
  }
  if (ControlRegister == 0)
  {
    return EFI_SUCCESS;
  }
  else
  {
    return EFI_TIMEOUT;
  }
}