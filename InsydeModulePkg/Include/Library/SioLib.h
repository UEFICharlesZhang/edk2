/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SIO_LIBRARY_H_
#define _SIO_LIBRARY_H_

#include <Sio/SioResource.h>
#include <Pi/PiS3BootScript.h>
#include <Library/IoLib.h>
#include <Library/S3BootScriptLib.h>

#define DATA_PORT        0x60
#define COMMAND_PORT     0x64
#define STATUS_PORT      0x64

#define DECODE_NOT_OPEN  0xFF
#define WRAP_MODE        0xEE
#define TEST_VALUE       0x05
#define NORMAL_MODE      0xEC
#define NEXT_TO_MOUSE    0xD4
#define ECHO_COMMAND     0xEE
#define ECHO             0xEE

/**
  Write data to the specific register.  

  @param[in]  Index    SIO index port.
  @param[in]  Data     The 8 bits value write to index port.
**/
VOID
IDW8 (
  IN UINT8                      Index,
  IN UINT8                      Data,
  IN EFI_SIO_RESOURCE_FUNCTION  *SioResourceFunction
  );

/**
  Read data to the specific register.  

  @param[in]  Index    SIO index port.

  @retval     UINT8    Read value from SIO. 
**/
UINT8
IDR8 (
  IN UINT8                      Index,
  IN EFI_SIO_RESOURCE_FUNCTION  *SioResourceFunction
  );

/**
  Writes an I/O port.  

  @param[in]  Port     I/O Port to write.
  @param[in]  Value    The 8 bits value write to port.
**/
VOID
SioWrite8 (
  IN UINT16   Port,
  IN UINT8    Value,
  IN BOOLEAN  ScriptFlag
  );

/**
  Writes an I/O port.   

  @param[in]  Port     I/O Port to write.

  @retval     Value    Get data from SIO.
**/
UINT8
SioRead8 (
  IN UINT16  Port
  );

/**

  Check physical existence of keyboard.

  @retval  True or false    Keyboard existence return true, otherwise funs return false
**/
BOOLEAN
KbcExistence (
  );

/**

  Check physical existence of MOUSE. 

  @retval  True or false    Mouse existence return true, otherwise funs return false
**/
BOOLEAN
MouseExistence (
  );

/**

  Write Data or Command to specified IoPort 60h or 64h.  

  @param[in]  Port                 IoPort Index.
  @param[in]  Value                Data value or KBC command.
  @param[in]  StallMicroseconds    Stall Microseconds.
**/
VOID
WriteValueToKbcPort (
  IN  UINT8   Port,
  IN  UINT8   Value,
  IN  UINT32  StallMicroseconds
  );
  
/**
  Pass in informatioin to find LDN of SIO device.  

  @param[in]       *DeviceProtocolTable  DeviceProtocolFunction table pointer.
  @param[in]       Devicee               What kind of the device.
  @param[in]       DeviceInstance        SIO instance.
  @param[out]      LDN                   LDN of SIO device.
**/
VOID
FindDeviceLdn (
  IN  EFI_INSTALL_DEVICE_FUNCTION *DeviceProtocolTable,
  IN  UINT8                       Device,
  IN  UINT8                       DeviceInstance,
  OUT UINT8                       *Ldn
  );

/**
  Pass in informatioin to find extensive resource from PCD data.  

  @param[in, out]  *TableList            PCD data pointer.
  @param[in]       TypeInstance          SIO instance.
  @param[in]       Device                What kind of the device.
  @param[in]       DeviceInstance        Device instance.
**/
VOID
FindExtensiveDevice (
  UINT8                    **PcdDmiPointer,
  UINT8                    TypeInstance,
  UINT8                    Device,
  UINT8                    DeviceInstance
  ); 

/**
  Pass in informatioin to find device resource from PCD data.  

  @param[in, out]  *TableList            PCD data pointer.
  @param[in]       TypeInstance          SIO instance.
  @param[in]       Device                What kind of the device.
  @param[in]       DeviceInstance        Device instance.
**/
SIO_DEVICE_LIST_TABLE*
FindSioDevice (
  SIO_DEVICE_LIST_TABLE    *PcdPointer,
  UINT8                    TypeInstance,
  UINT8                    Device,
  UINT8                    DeviceInstance
  ); 

/**
  Word to chage place for high byte, low byte
   
  @param[in] Value    The number need to chage.

  @retval    Value    Function complete successfully. 
**/
UINT16
UpsideDown (
  IN UINT16                           Value
  );

/**
  Convert a single number to character.
  It assumes the input Char is in the scope of L'0' ~ L'9' and L'A' ~ L'F'.
  
  @param[in] Num    The input char which need to change to a hex number.
**/
UINT16
UintToChar (
  IN UINT8                           Num
  );

/**
  Decode character in ASCII to hexadecimal

  @param[in] Asc           Character in ASCII

  @retval Value in hexadecimal.
**/
UINT8
AsciiToHexadecimal (
  IN UINT8                        Asc
  );

#endif
