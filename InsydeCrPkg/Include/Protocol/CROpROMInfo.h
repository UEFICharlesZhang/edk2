/** @file
  CROpROMInfo Protocol Definitions

;******************************************************************************
;* Copyright (c) 2012 - 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CONSOLE_REDIRECTION_OPROM_INFO_H_
#define _CONSOLE_REDIRECTION_OPROM_INFO_H_

#include <Uefi.h>

#define CONSOLE_REDIRECTION_OPROM_INFO_PROTOCOL_GUID \
  { \
    0xF773D226, 0xBD93, 0x4767, 0xB7, 0x50, 0x01, 0xDE, 0xE8, 0x9C, 0x66, 0x0A \
  }

typedef struct _EFI_CONSOLE_REDIRECTION_OPROM_INFO_PROTOCOL  EFI_CONSOLE_REDIRECTION_OPROM_INFO_PROTOCOL;


#define CR_OPROM_INFO_SIGNATURE   SIGNATURE_32('C','R','O','I')

//
// Bios Post Phase
//
#define BIOS_PHASE_INIT          0
#define BIOS_PHASE_OPROM         1
#define BIOS_PHASE_SCU           2
#define BIOS_PHASE_OS            3
#define BIOS_PHASE_DOS_RUNING    4
#define BIOS_PHASE_OPROM_RUNING  5

#define BIOS_PHASE                          0x047

#define BDA_EBDA_OFFSET                     0x40E

#define EBDA_DATA(Offset, type)             (*(type*)(UINTN)(((*(UINT16*)(UINTN)0x40e) << 4) + (Offset)))

#define CR_SERIAL_NONE_FC 0
#define CR_SERIAL_HW_FC   1
#define CR_SERIAL_SW_FC   2

#define XON  0x11
#define XOFF 0x13

#define CR_MEM_MANAGER_SIGNATURE  SIGNATURE_32 ('$', 'C', 'E', 'I') 


#pragma pack(1)                                                              
typedef struct {
  UINT32    Signature;
  UINT8     SwFlowControlState;
} CR_OPROM_INFO;
#pragma pack()

typedef 
CR_OPROM_INFO*
(EFIAPI *CR_GET_COMMON_AREA) (
  IN   EFI_CONSOLE_REDIRECTION_OPROM_INFO_PROTOCOL    *This
  );


typedef struct _EFI_CONSOLE_REDIRECTION_OPROM_INFO_PROTOCOL {
  BOOLEAN             CrOpROMInstalled;
  CR_GET_COMMON_AREA  CrGetCommonArea;
} EFI_CONSOLE_REDIRECTION_OPROM_INFO_PROTOCOL;

extern EFI_GUID gCROpROMInfoProtocolGuid;

#endif
