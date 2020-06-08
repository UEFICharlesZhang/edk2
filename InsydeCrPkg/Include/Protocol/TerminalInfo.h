/** @file
  Serial Terminal Info protocol definition
  
;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _TERMINAL_INFO_
#define _TERMINAL_INFO_

#include <TerminalMessage.h>

//
// Serial Terminal Info protocol
//
#define SERIAL_TERMINAL_INFO_PROTOCOL_GUID \
  { 0x52F48D16, 0x28DC, 0x49D1, {0xb2, 0xA3, 0x8A, 0x37, 0x65, 0xB7, 0x8D, 0x2A} }

typedef struct _SERIAL_TERMINAL_INFO_PROTOCOL {
  UINT8                       DeviceType;
  BOOLEAN                     DoLogin;
  BOOLEAN                     DoAutoRefresh;
  UINT16                      Columns;
  UINT16                      Rows;
  TERMINAL_MESSAGE_CALLBACK   Notify;
  VOID                        *Context;
  EFI_IPv4_ADDRESS            StationAddress;
  UINT16                      StationPort;
  EFI_IPv4_ADDRESS            RemoteAddress;                                
  UINT16                      RemotePort;
  EFI_MAC_ADDRESS             RemoteHwMac;
} SERIAL_TERMINAL_INFO_PROTOCOL;

extern EFI_GUID gSerialTerminalInfoProtocolGuid;

#endif // _TERMINAL_INFO_