/** @file
   Insyde Console Information protocol definition
   
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

#ifndef _CONSOLE_INFO_H_
#define _CONSOLE_INFO_H_

#include <Uefi.h>

#define INSYDE_CONSOLE_INFO_PROTOCOL_GUID \
  { 0xea237d6a, 0x6b64, 0x419e, { 0xa0, 0xf2, 0x5e, 0x78, 0x9a, 0x8e, 0xde, 0xd2 } }

typedef struct _INSYDE_CONSOLE_INFO_PROTOCOL  INSYDE_CONSOLE_INFO_PROTOCOL;

typedef struct _INSYDE_SCREEN_BUFFER_INFO {
  UINT16                        Columns;
  UINT16                        Rows;
  CHAR16                        *CharBuf;
  UINT8                         *AttrBuf;
  EFI_SIMPLE_TEXT_OUTPUT_MODE   *ModeInfo;
} INSYDE_SCREEN_BUFFER_INFO;

typedef
VOID
(EFIAPI *CONSOLE_DFC) (
  UINTN   Param1,
  UINTN   Param2,
  UINTN   Param3,
  UINTN   Param4
  );

typedef
EFI_STATUS
(EFIAPI *QUEUE_CONSOLE_DFC) (
  EFI_TPL       Tpl,
  CONSOLE_DFC   Dfc,
  UINTN         Param1,
  UINTN         Param2,
  UINTN         Param3,
  UINTN         Param4
  );

typedef
BOOLEAN
(EFIAPI *IS_CONSOLE_EXEC) (
  VOID
  );

typedef
EFI_STATUS
(EFIAPI *GET_CONSOLE_INFO) (
  IN  INSYDE_CONSOLE_INFO_PROTOCOL  *This,
  OUT INSYDE_SCREEN_BUFFER_INFO     *BufInfo 
  );

struct _INSYDE_CONSOLE_INFO_PROTOCOL {
  QUEUE_CONSOLE_DFC     QueueDfc;
  IS_CONSOLE_EXEC       IsConsoleExec;
  GET_CONSOLE_INFO      GetConsoleInfo;
};

extern EFI_GUID gInsydeConsoleInfoProtocolGuid;

#endif
