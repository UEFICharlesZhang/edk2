/** @file

  Header file of Beep Status Code implementation.

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

#ifndef _BEEP_STATUS_CODE_H_
#define _BEEP_STATUS_CODE_H_

#include <Pi/PiStatusCode.h>

#define BEEP_NONE                             0x00
#define BEEP_LONG                             0x01
#define BEEP_SHORT                            0x02

#define END_BEEP_TYPE_INDEX                   0xFF

#define BEEP_LONG_DURATION                    1000000
#define BEEP_LONG_TIME_INTERVAL               250000
#define BEEP_SHORT_DURATION                   250000
#define BEEP_SHORT_TIME_INTERVAL              250000

#define SMM_COMM_UNREGISTER_HANDLER           0x01


#pragma pack(1)

typedef struct {
  UINT32                         BeepDuration;
  UINT32                         TimerInterval;
} BEEP_STRUCTURE;

typedef struct {
  UINT8                          Index;
  UINT8                          SoundType[9];  
} BEEP_TYPE;


typedef struct {
  EFI_STATUS_CODE_TYPE           CodeType;
  EFI_STATUS_CODE_VALUE          CodeValue;
  UINT8                          BeepTypeId;
  UINT8                          BeepLoop;
  UINT8                          Reserved[2];  
} STATUS_CODE_BEEP_ENTRY;

typedef struct {
  UINTN                          Function;
  EFI_STATUS                     ReturnStatus;
  BOOLEAN                        UnregisterService;             
} SMM_BEEP_STATUS_CODE_COMMUNICATE;

#pragma pack()

extern EFI_GUID gH2OBeepStatusCodeCommunicationGuid;

#endif

