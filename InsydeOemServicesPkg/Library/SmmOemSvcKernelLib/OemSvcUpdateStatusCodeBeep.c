/** @file
  This OEM Service is to change the related Beep tables.
  
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

#include <BeepStatusCode.h>

// Beep table for different kinds of Beep sounds.
// {BeepTypeId, Sound1, Sound2, Sound3, Sound4, Sound5, Sound6, Sound7, Sound8, Sound9}
//  
//  Sound type:
//  - BEEP_NONE : 0x00
//  - BEEP_LONG : 0x01
//  - BEEP_SHORT: 0x02
//
BEEP_TYPE mBeepTypeList[] = {
  {0x00, 0x1, 0x1, 0x1, 0x2, 0x2, 0x0, 0x0, 0x0, 0x0}, //  0: Long, Long, Long, Short, Short.
  {0x01, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, //  1: Short.
  {0x02, 0x2, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, //  2: Short, Long.
  {0x03, 0x1, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, //  3: Long, Short.
  {0x04, 0x2, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, //  4: Short, Short.
  {0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}  //  End Entry
  };

//
// Beep List for different kinds of Beep event for Status Code.
// {CodeType, CodeValue, BeepTypeId, BeepLoopCount, Reserved, Reserved}
//  
//  Code Type: Refer to PiStatusCode.h
//  - EFI_PROGRESS_CODE             0x00000001
//  - EFI_ERROR_CODE                0x00000002
//  - EFI_DEBUG_CODE                0x00000003
//
//  Code Value: Refer to PiStatusCode.h
//  - EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_PC_INIT             0x00051005
//  - EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_EC_NONE_DETECTED    0x00051009
//
//  BeepTypeId:
//  - Refer to the Index number of PcdBeepTypeList
//
//  BeepLoopCount:
//  - Indicate the loop count for this Beep event.
//
STATUS_CODE_BEEP_ENTRY mBeepList[] = {
  {EFI_PROGRESS_CODE, EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_PC_INIT,           0x0, 0x1, 0, 0},
  {EFI_ERROR_CODE,    EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_EC_NONE_DETECTED,  0x0, 0x1, 0, 0},
  {0x0, 0x0, 0x0, 0x0, 0x0, 0} // End Entry
  };

/**
  Change the Beep related table as OEM's feature.

  @param[out]  **StatusCodeBeepList  Follow the STATUS_CODE_BEEP_ENTRY structure to define the Beep entries.
  @param[out]  **BeepTypeList        Follow the BEEP_TYPE structure to define the Beep type

  @retval      EFI_MEDIA_CHANGED     The value of IN OUT parameter is changed.
                                     Return the OEM-defined Beep tables.
  @retval      EFI_UNSUPPORTED       There is no OEM tables returned. 
**/
EFI_STATUS
OemSvcUpdateStatusCodeBeep (
  OUT STATUS_CODE_BEEP_ENTRY           **StatusCodeBeepList,
  OUT BEEP_TYPE                        **BeepTypeList
  )
{
//
//  Sample code to let OEM to update the table.
//  OEM can modify the content base on some special policies.
//  
//  *StatusCodeBeepList = (STATUS_CODE_BEEP_ENTRY *)&mBeepList;
//  *BeepTypeList = (BEEP_TYPE *)&mBeepTypeList;     
//  return EFI_MEDIA_CHANGED;

  return EFI_UNSUPPORTED;
}
