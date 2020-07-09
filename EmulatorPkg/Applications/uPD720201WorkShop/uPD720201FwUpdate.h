/**
 *
 * Copyright (c) 2018, Great Wall Co,.Ltd. All rights reserved.
**/
EFI_STATUS
EFIAPI
uPD720201ReadSpiRom (
  IN EFI_PCI_IO_PROTOCOL          *PciIo,
  UINT32 *Buffer, 
  UINT32 *BufferSize
  );
EFI_STATUS
EFIAPI
uPD720201WriteSpiRom(
  IN EFI_PCI_IO_PROTOCOL          *PciIo,
  IN UINT32 *FwBuffer, 
  IN UINTN BufferSize
  );
EFI_STATUS
EFIAPI
uPD720201EraseSpiRom(
  IN EFI_PCI_IO_PROTOCOL          *PciIo
  );