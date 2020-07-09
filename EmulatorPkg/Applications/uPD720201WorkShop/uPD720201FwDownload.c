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
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <IndustryStandard/Pci.h>
#include <Protocol/PciIo.h>
#include "uPD720201FwDownload.h"
#include <Library/TimerLib.h>
#include "uPD720201Register.h"


EFI_STATUS
EFIAPI
FWDownloadControl(IN EFI_PCI_IO_PROTOCOL *PciIo, BOOLEAN Flag)
{
  UINT16 ControlRegister;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F4, 1, &ControlRegister);
  ControlRegister &= ~PCICNF0F4_FWDOWNLOADENABLE;
  ControlRegister |= Flag;
  DEBUG((EFI_D_INFO, "%a, write data:%x\n",__FUNCTION__,ControlRegister));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCICNF0F4, 1, &ControlRegister);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
WaitF4SetData0to0(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 0;
  UINT32 TimeOut = GENERIC_TIMEOUT;
  do
  {
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F4, 1, &ControlRegister);
    DEBUG((EFI_D_INFO, "%a, ControlRegister :%04x\n", __FUNCTION__, ControlRegister));
    ControlRegister &= PCICNF0F4_SETDATA0;
    MicroSecondDelay(1000);
    TimeOut--;
  } while ((ControlRegister != 0) && (TimeOut > 0));
  if (TimeOut == 0)
  {
    return EFI_TIMEOUT;
  }
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
WaitF4SetData1to0(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 0;
  UINT32 TimeOut = GENERIC_TIMEOUT;

  do
  {
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F4, 1, &ControlRegister);
    DEBUG((EFI_D_INFO, "%a, ControlRegister :%04x\n", __FUNCTION__, ControlRegister));
    ControlRegister &= PCICNF0F4_SETDATA1;
    MicroSecondDelay(1000);
    TimeOut--;
  } while ((ControlRegister != 0) && (TimeOut > 0));
  if (TimeOut == 0)
  {
    return EFI_TIMEOUT;
  }
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
WriteF8Data0(IN EFI_PCI_IO_PROTOCOL *PciIo, UINT32 Data)
{
  DEBUG((EFI_D_INFO, "%a, write data:%08x\n",__FUNCTION__,Data));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint32, PCICNF0F8, 1, &Data);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
WriteFCData1(IN EFI_PCI_IO_PROTOCOL *PciIo, UINT32 Data)
{
  DEBUG((EFI_D_INFO, "%a, write data:%08x\n",__FUNCTION__,Data));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint32, PCICNF0FC, 1, &Data);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
SetF4Data0(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 0;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F4, 1, &ControlRegister);
  ControlRegister |= PCICNF0F4_SETDATA0;
  DEBUG((EFI_D_INFO, "%a:%x\n",__FUNCTION__,ControlRegister));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCICNF0F4, 1, &ControlRegister);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
SetF4Data1(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 0;
  PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F4, 1, &ControlRegister);
  ControlRegister |= PCICNF0F4_SETDATA1;
  DEBUG((EFI_D_INFO, "%a:%x\n",__FUNCTION__,ControlRegister));
  PciIo->Pci.Write(PciIo, EfiPciIoWidthUint16, PCICNF0F4, 1, &ControlRegister);
  return EFI_SUCCESS;
}
EFI_STATUS
EFIAPI
ConfirmF4Result(IN EFI_PCI_IO_PROTOCOL *PciIo)
{
  UINT16 ControlRegister = 0;
  UINT16 ResuleCode = 0;
  UINT32 TimeOut = GENERIC_TIMEOUT;

  do
  {
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCICNF0F4, 1, &ControlRegister);
    ResuleCode = (ControlRegister >> PCICNF0F4_RESULT_B) & 0x07;
    DEBUG((EFI_D_INFO, "%a, ResuleCode:%x\n", __FUNCTION__, ResuleCode));
    TimeOut--;
    MicroSecondDelay(1000);
  } while ((ResuleCode != 1) && (TimeOut > 0));
   if (ResuleCode == 1)
  {
    return EFI_SUCCESS;
  }
  else
  {
    return EFI_DEVICE_ERROR;
  }
}
EFI_STATUS
EFIAPI
uPD720201FwDownload(
  IN EFI_PCI_IO_PROTOCOL          *PciIo,
  IN UINT32 *Buffer, 
  IN UINT32 BufferSize
  )
{
  UINT32 i = 0;
  EFI_STATUS Status;
  SET_DATA set_data_page = SET_DATA_PAGE0;

  //Step1
  FWDownloadControl(PciIo, TRUE);

  // Write all the dword
  // Write all the dwords, one pair at a time
  for (i = 0; i < BufferSize/4; i++)
  {
    DEBUG((EFI_D_INFO, "%a, Index:%d, data:0x%08X\n", __FUNCTION__, i, Buffer[i]));
    switch (set_data_page)
    {
    case SET_DATA_PAGE0:
      //Step7
      WaitF4SetData0to0(PciIo);

      //Step8
      WriteF8Data0(PciIo, Buffer[i]);
      SetF4Data0(PciIo);

      set_data_page = SET_DATA_PAGE1;
      break;
    case SET_DATA_PAGE1:
      //Step9
      WaitF4SetData1to0(PciIo);

      //Step10
      WriteFCData1(PciIo, Buffer[i]);
      SetF4Data1(PciIo);

      set_data_page = SET_DATA_PAGE0;
      break;
    default:
      break;
    }
    DEBUG((EFI_D_INFO, "\n"));
  }
  //Repeat End

  //Step12
  FWDownloadControl(PciIo, FALSE);

  //Step13
  Status = ConfirmF4Result(PciIo);
  DEBUG((EFI_D_INFO, "%a, FW download complete with status :%r\n", __FUNCTION__, Status));

  return Status;
}
