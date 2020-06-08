/** @file
  SIO Library.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/SioLib.h>

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
  )
{  
  if (DeviceProtocolTable != NULL) {
    while (DeviceProtocolTable->Device != NULL_ID) {
      if ((DeviceProtocolTable->Device == Device) && (DeviceProtocolTable->Instance == DeviceInstance)) {
        *Ldn = DeviceProtocolTable->Ldn;
        return;
      }
      DeviceProtocolTable++;
    }
  }
  return;
}

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
  )
{
  SioWrite8 (
    SioResourceFunction->SioCfgPort,
    Index,
    SioResourceFunction->ScriptFlag
    );
  SioWrite8 (
    SioResourceFunction->SioCfgPort + 1,
    Data,
    SioResourceFunction->ScriptFlag
    );
}

/**
  Read data to the specific register.  

  @param[in]  Index    SIO index port.

  @retval     UINT8    Read value from SIO. 
**/
UINT8
IDR8 (
  IN UINT8                      Index,
  IN EFI_SIO_RESOURCE_FUNCTION  *SioResourceFunction
  )  
{  
  SioWrite8 (
    SioResourceFunction->SioCfgPort, 
    Index,
    FALSE
    );

  return SioRead8(SioResourceFunction->SioCfgPort + 1);
}

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
  )
{
  IoWrite8 ((UINTN)Port, Value);
  
  if (ScriptFlag == TRUE) {
    S3BootScriptSaveIoWrite (
      EfiBootScriptWidthUint8,
      Port,
      1,      
      &Value    
      );  
  }  
}

/**
  Writes an I/O port.   

  @param[in]  Port     I/O Port to write.

  @retval     Value    Get data from SIO.
**/
UINT8
SioRead8 (
  IN UINT16  Port
  )
{
  return IoRead8 ((UINTN)Port);
}

/**

  Check physical existence of keyboard.

  @retval  True or false    Keyboard existence return true, otherwise funs return false
**/
BOOLEAN
KbcExistence (
  )
{
  if (IoRead8 (DATA_PORT) == DECODE_NOT_OPEN) return FALSE;

  WriteValueToKbcPort (DATA_PORT, ECHO_COMMAND, 0);

  while(!(IoRead8(STATUS_PORT) & BIT0));

  if (IoRead8(DATA_PORT) == ECHO) return TRUE;
  
  return FALSE;
}

/**

  Check physical existence of MOUSE. 

  @retval  True or false    Mouse existence return true, otherwise funs return false
**/
BOOLEAN
MouseExistence (
  )
{
  BOOLEAN  Result;
  
  Result = FALSE;

  if (IoRead8 (DATA_PORT) == DECODE_NOT_OPEN) return FALSE;

  WriteValueToKbcPort (COMMAND_PORT, NEXT_TO_MOUSE, 0);
  WriteValueToKbcPort (DATA_PORT, WRAP_MODE, 8000);
  WriteValueToKbcPort (COMMAND_PORT, NEXT_TO_MOUSE, 0);
  WriteValueToKbcPort (DATA_PORT, TEST_VALUE, 0);

  while(!(IoRead8(STATUS_PORT) & BIT0));

  if (IoRead8(DATA_PORT) == TEST_VALUE) Result = TRUE;

  WriteValueToKbcPort (COMMAND_PORT, NEXT_TO_MOUSE, 0);
  WriteValueToKbcPort (DATA_PORT, NORMAL_MODE, 8000);

  while(!(IoRead8(STATUS_PORT) & BIT0));
  IoRead8(DATA_PORT);
  
  return Result;
}

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
  )
{
  while (IoRead8 (STATUS_PORT) & BIT0) {
    IoRead8 (DATA_PORT);
  }

  while (IoRead8 (STATUS_PORT) & BIT1);

  IoWrite8 (Port, Value);

  //
  // Stall Microseconds
  //
  while (StallMicroseconds) {
    while (!(IoRead8 (0x61) & 0x10));
    while (IoRead8 (0x61) & 0x10);
    StallMicroseconds--;
  }

  while (IoRead8 (STATUS_PORT) & BIT1);
}

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
  ) 
{
  BOOLEAN                       Found;
  SIO_EXTENSIVE_TABLE_MAINTYPE  *Table; 

  Found = FALSE;
  
  if (*PcdDmiPointer != NULL) {
    while (*(UINT8*)(*PcdDmiPointer) != 0xFF) {
      if (*(UINT8*)(*PcdDmiPointer) == MAIN_TYPE) {   
        Table = (SIO_EXTENSIVE_TABLE_MAINTYPE*)*PcdDmiPointer;
        if ((Device == Table->Device) && (DeviceInstance == Table->DeviceInstance)) {
          Found = TRUE;        
          break;                  
        }  
      }        
      *PcdDmiPointer += *(UINT8*)(*PcdDmiPointer + 1);    
    }
  }    
  if (!Found) {  
    *PcdDmiPointer = NULL;
  }    
}

/**
  Pass in informatioin to find device resource from PCD data.  

  @param[in, out]  *TableList            PCD data pointer.
  @param[in]       TypeInstance          SIO instance.
  @param[in]       Device                What kind of the device.
  @param[in]       DeviceInstance        Device instance.

  @retval          PcdPointer            If found device, it'll return pointer or else it return NULL. 
**/
SIO_DEVICE_LIST_TABLE*
FindSioDevice (
  SIO_DEVICE_LIST_TABLE  *PcdPointer,
  UINT8                  TypeInstance,
  UINT8                  Device,
  UINT8                  DeviceInstance
  ) 
{
  if (PcdPointer != NULL) {
    //
    // Calculate the number of non-zero entries in the table
    //
    while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
      if ((PcdPointer->TypeInstance == TypeInstance) && (PcdPointer->Device == Device) &&
        (PcdPointer->DeviceInstance == DeviceInstance)) {
        return PcdPointer;
      }
      PcdPointer++;
    }
  }
  PcdPointer = NULL;
  return PcdPointer;
}

/**
  Word to chage place for high byte, low byte
   
  @param[in] Value    The number need to chage.

  @retval    Value    Function complete successfully. 
**/
UINT16
UpsideDown (
  IN UINT16  Value
  )
{
  UINT16     High;
  UINT8      Low;

  High = Value & 0xFF;
  Low = Value >> 8;

  Value = (High << 8) + Low;

  return Value;
}

/**
  Convert a single number to character.
  It assumes the input Char is in the scope of L'0' ~ L'9' and L'A' ~ L'F'.
  
  @param[in] Num    The input char which need to change to a hex number.
**/
UINT16
UintToChar (
  IN UINT8  Num
  )
{
  UINT8     Index;
  UINT16    Value;
  UINT8     TempValue;

  Value = 0;

  for (Index = 0; Index < (sizeof(UINT16)/sizeof(UINT8)); Index++) {
    TempValue = Num & 0xF;
    if ((TempValue >= 0) && (TempValue <= 9)) {
      Value = (Value << 8) + ((UINT16) (TempValue + L'0'));
    }

    if ((TempValue >= 0xA) && (TempValue <= 0xF)) {
      Value = (Value << 8) + ((UINT16) (TempValue + L'A' - 0xA ));
    }
    Num = Num >> 4;
  }
  
  return Value;
}

/**
  Decode character in ASCII to hexadecimal

  @param[in] Asc           Character in ASCII

  @retval Value in hexadecimal.
**/
UINT8
AsciiToHexadecimal (
  IN UINT8  Asc
  )
{
  if (Asc >= '0' && Asc <= '9') {
    return Asc - '0';
  } else if (Asc >= 'A' && Asc <= 'F') {
    return Asc - 'A' + 0xA;
  } else if (Asc >= 'a' && Asc <= 'f') {
    return Asc - 'a' + 0xA;
  }

  return 0;
}
