
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#define SPI_CFG_PORT_ADDR                0x1fffff00
/*
 * SPI registers
 */
#define REG_SPI_CFG		0x00
#define REG_ERR_EN		0x04
#define REG_ERR_LOG		0x08
#define REG_WP_REG		0x0C
#define REG_CS_DELAY		0x10
#define REG_FLASH_CAP		0x14
#define REG_FLUSH_REG		0x18
#define REG_CMD_PORT		0x20
#define REG_ADDR_PORT		0x24
#define REG_HD_PORT		0x28
#define REG_LD_PORT		0x2C
#define REG_CS0_CFG		0x30
#define REG_CS1_CFG		0x34
#define REG_CS2_CFG		0x38
#define REG_CS3_CFG		0x3C
#define REG_CLK_CTRL_SET	0x40
#define REG_CLK_CTRL_CLR	0x44


/*
 * the codes of the different commands
 */
#define CMD_WRDI		0x4
#define CMD_RDID		0x9F
#define CMD_RDSR		0x5
#define CMD_WREN		0x6
#define CMD_ERASE		0xD8
#define CMD_RDAR		0x65
#define CMD_P4E			0x20

/*
 * the offset of the command register
 */
#define RW_NUM_SHIFT		0
#define DUMMY_SHIFT		4
#define ADDR_MODE1_SHIFT	5
#define ADDR_MODE0_SHIFT	6
#define CMD_SHIFT		8
#define FLASH_SEL_SHIFT		16

/*
 * the codes of the different commands
 */
#define CMD_WRDI		0x4
#define CMD_RDID		0x9F
#define CMD_RDSR		0x5
#define CMD_WREN		0x6
#define CMD_ERASE		0xD8
#define CMD_RDAR		0x65
#define CMD_P4E			0x20

/*
 * the bytes of reading/writing
 */
#define RW_NUM_1		1
#define RW_NUM_2		2
#define RW_NUM_4		4
#define RW_NUM_8		8


EFI_STATUS
EraseWrite (
  IN  UINT32                 Offset,
  IN  UINT8                  *Buffer,
  IN  UINT32                 Length
);

EFI_STATUS
Read (
  IN UINT32                  Offset,
  IN OUT UINT8               *Buffer,
  IN UINT32                  Length
);
