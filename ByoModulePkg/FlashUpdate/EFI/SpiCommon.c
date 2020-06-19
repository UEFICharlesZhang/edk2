
#include "SpiCommon.h"

STATIC UINTN  mNorFlashControlBase = SPI_CFG_PORT_ADDR;

static inline void cmd_enable(void)
{
  *(volatile UINT32 *)(mNorFlashControlBase + REG_LD_PORT);
}

static inline void wait_wip(void)
{
  volatile UINT32  status;
  UINT32  cmd;

  cmd = (CMD_RDSR << CMD_SHIFT) | RW_NUM_1;

  /*
   * test the WIP bit of status register until it becomes 0
   */
  do {
    *(volatile UINT32 *)(mNorFlashControlBase + REG_CMD_PORT) = cmd;
    cmd_enable();
    status = *(volatile UINT32 *)(mNorFlashControlBase + REG_LD_PORT);
  } while (status & 0x1);
}

static inline void write_enable(void)
{
  volatile UINT32  status;
  UINT32  cmd;

  /*
   * test the WEL bit of status register until it becomes 1
   */
  do {
    cmd = (CMD_WREN << CMD_SHIFT);
    *(volatile UINT32 *)(mNorFlashControlBase + REG_CMD_PORT) = cmd;
    cmd_enable();

    cmd = (CMD_RDSR << CMD_SHIFT) | RW_NUM_1;
    *(volatile UINT32 *)(mNorFlashControlBase + REG_CMD_PORT) = cmd;
    cmd_enable();

    status = *(volatile UINT32 *)(mNorFlashControlBase + REG_LD_PORT);
  } while (!(status & 0x2));
}

static inline void write_disable(void)
{
  volatile UINT32  status;
  UINT32  cmd;

  /*
   * test the WEL bit of status register until it becomes 0
   */
  do {
    cmd = (CMD_WRDI << CMD_SHIFT);
    *(volatile UINT32 *)(mNorFlashControlBase + REG_CMD_PORT) = cmd;
    cmd_enable();

    cmd = (CMD_RDSR << CMD_SHIFT) | RW_NUM_1;
    *(volatile UINT32 *)(mNorFlashControlBase + REG_CMD_PORT) = cmd;
    cmd_enable();

    status = *(volatile UINT32 *)(mNorFlashControlBase + REG_LD_PORT);
  } while (status & 0x2);
}


static inline void SPI_Erase_Sector(UINTN BlockAddress)
{
  UINT32  cmd;

  wait_wip();
  write_enable();
  wait_wip();

  *(volatile UINT32 *)(mNorFlashControlBase + REG_ADDR_PORT) = (UINT32)BlockAddress;

  cmd = (CMD_ERASE << CMD_SHIFT) | (0x1 << ADDR_MODE0_SHIFT);
  *(volatile UINT32 *)(mNorFlashControlBase + REG_CMD_PORT) = cmd;
  cmd_enable();

  wait_wip();
}

static inline void SPI_Write_Word(UINTN Address, UINT32 Value)
{
  if (Value == 0xffffffff)
    return;

  wait_wip();
  write_enable();
  wait_wip();

  *(volatile UINT32 *)Address = Value;

  wait_wip();
}


EFI_STATUS
NorFlashPlatformWrite (
  IN UINTN            Address,
  IN VOID             *Buffer,
  IN UINT32           BufferSizeInBytes
  )
{
  UINT32 Index = 0;

  for(Index = 0; Index < BufferSizeInBytes; Index += 4) {
    SPI_Write_Word(Address + Index, *(UINT32 *)((UINTN)Buffer + Index));
  }

  return EFI_SUCCESS;
}


EFI_STATUS
Erase (
  IN UINT32                  Offset,
  IN UINT32                  Length
) {
  EFI_STATUS     Status;
  UINT32         Index;
  UINT32         Count;

  Status = EFI_SUCCESS;
  if ((Length % SIZE_64KB) == 0) {
    Count = Length / SIZE_64KB;
    for (Index = 0; Index < Count; Index ++) {
      DEBUG((EFI_D_ERROR, "Erase:%x,Length:%x\n", Offset,Length));
      SPI_Erase_Sector (Offset);
      Offset += SIZE_64KB;
    }
  } else {
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

EFI_STATUS
Write (
  IN  UINT32                 Offset,
  IN  UINT8                  *Buffer,
  IN  UINT32                 Length
) {
  EFI_STATUS     Status;
  
  if (Length % 4 || Offset % 4) {
    DEBUG ((EFI_D_ERROR, "Error input for spi program!!Need check!!\n"));
    return EFI_INVALID_PARAMETER;
  }
  DEBUG((EFI_D_ERROR, "Write:%x,Length:%x\n", Offset,Length));
  Status = NorFlashPlatformWrite(Offset, Buffer, Length);

  return Status;
}

EFI_STATUS
Read (
  IN UINT32                  Offset,
  IN OUT UINT8               *Buffer,
  IN UINT32                  Length
) {

  CopyMem (Buffer, (UINT8*)(UINTN)(Offset), Length);

  return EFI_SUCCESS;
}

EFI_STATUS
EraseWrite (
  IN  UINT32                 Offset,
  IN  UINT8                  *Buffer,
  IN  UINT32                 Length
) {
  EFI_STATUS      Status;
  UINTN           Index;
  UINTN           Count;

  Status = EFI_SUCCESS;
  Count = Length / SIZE_64KB;
  for (Index = 0; Index < Count; Index ++) {
    Status = Erase (Offset + Index * SIZE_64KB, SIZE_64KB);
    if (EFI_ERROR(Status)) {
      break;
    }
    Status = Write (Offset + Index * SIZE_64KB, Buffer + Index * SIZE_64KB, SIZE_64KB);
    if (EFI_ERROR(Status)) {
      break;
    }
  }

  return Status;
}





