/**
 *
 * Copyright (c) 2018, Great Wall Co,.Ltd. All rights reserved.
**/
//@@---------------------- Registers for firmware downloading

#define PCICNF0F4  0xF4  // control and status bits
   /*
   0 = FW download enable (1), RW
   1 = FW download lock (1) or unlock (0), need 0 to perform download, RW(Write Once)
   6:4 = Result code, RO -- processing (0), success (1), error (2),
   8 = Set Data 0, RW
   9 = Set Data 1, RW
   16:31 = (used for serial EEPROM read/write.  31 = serial EEPROM present.)
   */
#define PCICNF0F4_FWDOWNLOADENABLE  (0x0001)
#define PCICNF0F4_FWDOWNLOADLOCK    (0x0002)
#define PCICNF0F4_SETDATA0          (0x0100)
#define PCICNF0F4_SETDATA1          (0x0200)
#define PCICNF0F4_RESULT            (0x0070)
#define PCICNF0F4_FWDOWNLOADENABLE_B    (0)
#define PCICNF0F4_FWDOWNLOADLOCK_B      (1)
#define PCICNF0F4_SETDATA0_B            (8)
#define PCICNF0F4_SETDATA1_B            (9)
#define PCICNF0F4_RESULT_B              (4)

//@@---------------------- Registers for firmware Update

#define PCICNF0F6  0xF6  // FW Control and Status Register (Offset Address: F6h)
#define PCICNF0F6_ROMENABLE    BIT0
#define PCICNF0F6_ROMERASE     BIT1
#define PCICNF0F6_ROMSETDATA0  BIT8
#define PCICNF0F6_ROMSETDATA1  BIT9
#define PCICNF0F6_ROMGETDATA0  BIT10
#define PCICNF0F6_ROMGETDATA1  BIT11
#define PCICNF0F6_ROMEXIST     BIT15

#define PCICNF0F6_RESULT_B              (4)

#define PCICNF0EC  0xEC  // ROM infor
#define PCICNF0F0  0xF0  // ROM param

//@@---------------------- DATA0 & DATA1

#define PCICNF0F8  0xF8  // data 0 dword
#define PCICNF0FC  0xFC  // data 1 dword


//@@---------------------- Other define

#define GENERIC_TIMEOUT  0x64  // 100

//#define SINGLE_PAGE

typedef enum {
    SET_DATA_PAGE0,
    SET_DATA_PAGE1
} SET_DATA;