/** @file

   The definition of Setup Change Variable.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _SETUP_CHANGE_VARIABLE_H_
#define _SETUP_CHANGE_VARIABLE_H_

extern EFI_GUID gH2OSetupChangeVariableGuid;

#define H2O_SETUP_CHANGE_VARIABLE_NAME        L"SetupChange"

#pragma pack(1)

typedef struct _H2O_SETUP_CHANGE_VARIABLE {
  UINT32                    TimeStamp;
  UINT16                    Size;
  UINT16                    Data[1];
} H2O_SETUP_CHANGE_VARIABLE;

#pragma pack()

#endif
