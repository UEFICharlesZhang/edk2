/** @file
  Console Redirection Service Manager Protocol definition.

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CR_SRV_MANAGER_PROTOCOL_H_
#define _CR_SRV_MANAGER_PROTOCOL_H_

#include <Protocol/TerminalEscCode.h>
#include <Protocol/SimpleTextOut.h>

#define CR_SRV_MANAGER_PROTOCOL_GUID \
  { \
    0xb188d5e9, 0x44c9, 0x41f7, 0xae, 0xc6, 0x60, 0xf3, 0x6d, 0x12, 0x1a, 0x34 \
  }

typedef struct {
  UINT8  PortNumber;
} ISA_DEVICE;

typedef struct {
  UINTN   Bus;
  UINTN   Dev;
  UINTN   Fun;
} PCI_DEVICE;

typedef union {
  ISA_DEVICE  IsaDevice;
  PCI_DEVICE  PciDevice;
} SERIAL_DEVICE;

typedef struct {
  UINT8            Type;
  UINT64           BaudRate;
  UINT32           DataBits;
  UINT32           Parity;
  UINT32           StopBits;
  SERIAL_DEVICE    Device;
} CR_SRV_SERIAL_DEVICE;

typedef struct {
  CR_SRV_SERIAL_DEVICE  SerialDev;
} CR_SRV_INFO; 


typedef struct _CR_SRV_MANAGER_PROTOCOL CR_SRV_MANAGER_PROTOCOL;

//
// CrSrvManager definition
//
typedef 
EFI_STATUS
(EFIAPI *CR_SRV_MANAGER_ENTRY) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  EFI_HANDLE                 NotifiedTerminal,
  IN  EFI_EVENT                  ConInTimerEvent
  );

typedef 
EFI_STATUS
(EFIAPI *CR_SRV_NOTIFY_FUNCTION) (
  IN  CR_SRV_MANAGER_PROTOCOL   *This,
  IN  VOID                      *Context
  );

typedef
EFI_STATUS
(EFIAPI *CR_SRV_REGISTER_NOTIFY) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  CHAR16                     *ServiceName,
  IN  CR_SRV_NOTIFY_FUNCTION     CrSrvNotifyFunc,
  OUT EFI_HANDLE                 *NotifyHandle
  );

typedef 
VOID
(EFIAPI *CR_SRV_BDS_CALLBACK) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This
  );


//
// CrConInSrv definition
//
typedef 
EFI_STATUS
(EFIAPI *CR_SRV_READ_KEY) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN OUT EFI_INPUT_KEY           *Key
  );


//
// CrWinConOutSrv definition
//
typedef
EFI_STATUS
(EFIAPI *CR_SRV_WIN_CREATE) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  INT16                      X,
  IN  INT16                      Y,
  IN  UINT16                     CWidth,
  IN  UINT16                     CHeight,
  IN  BOOLEAN                    Border,
  IN  CHAR16                     *Title,
  OUT VOID                       **AWin
  );

typedef
EFI_STATUS
(EFIAPI *CR_SRV_WIN_DESTROY) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  VOID                       *Win
  );

typedef
EFI_STATUS
(EFIAPI *CR_SRV_WIN_SHOW) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  VOID                       *Win
  );

typedef
VOID
(EFIAPI *CR_SRV_WIN_CLEARLINE) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  VOID                       *Win,
  IN  UINTN                      Attrib,
  IN  UINT16                     LineNumber
  );

typedef
VOID
(EFIAPI *CR_SRV_WIN_PRINTCHAR) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  VOID                       *Win,
  IN  CHAR16                     Ch
  );

typedef
EFI_STATUS
(EFIAPI *CR_SRV_WIN_PRINTAT) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  VOID                       *Win,
  IN  UINT16                     X,
  IN  UINT16                     Y,
  IN  CHAR16                     *Format,
  ...  
  );

typedef
VOID
(EFIAPI *CR_SRV_WIN_TEXTOUT) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  VOID                       *Win,
  IN  UINT16                     X,
  IN  UINT16                     Y,
  IN  CHAR16                     *Str
  );


typedef
VOID 
(EFIAPI *CR_SRV_WIN_GETCURSOR) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  VOID                       *Win,
  IN  UINT16                     *CursorX,
  IN  UINT16                     *CursorY
  );

typedef
VOID
(EFIAPI *CR_SRV_WIN_SETCURSOR) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  VOID                       *Win,
  IN  UINT16                     CursorX,
  IN  UINT16                     CursorY
  );

typedef
VOID
(EFIAPI *CR_SRV_WIN_SETCONOUT) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  VOID                       *Win,  
  IN  BOOLEAN                    CRedirection
  );

typedef 
VOID
(EFIAPI *CR_SRV_DISABLE_TERMINAL_CONIN) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  BOOLEAN                    Setting
  );


typedef 
EFI_STATUS
(EFIAPI *CR_SRV_READ_DATA) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN OUT UINTN                   *BufferSize,
  IN OUT CHAR8                   *Buffer
  );

typedef 
EFI_STATUS
(EFIAPI *CR_SRV_WRITE_DATA) (
  IN  CR_SRV_MANAGER_PROTOCOL    *This,
  IN  UINTN                      *BufferSize,
  IN  CHAR8                      *Buffer
  );


typedef struct _CR_SRV_MANAGER_PROTOCOL { 
  // Service manager
  CR_SRV_MANAGER_ENTRY          CrSrvManagerEntry;
  CR_SRV_REGISTER_NOTIFY        CrSrvRegNotify;
  CR_SRV_BDS_CALLBACK           CrSrvBdsCallback;

  // ConIn service
  CR_SRV_READ_KEY               CrSMReadKey;

  // Window services
  CR_SRV_WIN_CREATE             CrSrvWinCreate;
  CR_SRV_WIN_DESTROY            CrSrvWinDestroy;
  CR_SRV_WIN_SHOW               CrSrvWinShow;
  CR_SRV_WIN_CLEARLINE          CrSrvWinClearLine;
  CR_SRV_WIN_PRINTCHAR          CrSrvWinPrintChar;
  CR_SRV_WIN_PRINTAT            CrSrvWinPrintAt;
  CR_SRV_WIN_TEXTOUT            CrSrvWinTextOut;
  CR_SRV_WIN_GETCURSOR          CrSrvWinGetCursor;
  CR_SRV_WIN_SETCURSOR          CrSrvWinSetCursor;
  CR_SRV_WIN_SETCONOUT          CrSrvWinSetConOut;

  // Data Transfer service
  CR_SRV_DISABLE_TERMINAL_CONIN  CrSrvDisableTermConIn;
  CR_SRV_READ_DATA               CrSrvReadData;
  CR_SRV_WRITE_DATA              CrSrvWriteData;
} CR_SRV_MANAGER_PROTOCOL;

extern EFI_GUID gCrSrvManagerProtocolGuid;

#endif // _CR_SRV_MANAGER_PROTOCOL_H_

