/** @file
  Header file for check memory consistency realted functions

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _MEMORY_CHECK_H_
#define _MEMORY_CHECK_H_
#include "InternalBdsLib.h"

typedef struct {
  UINTN                     Signature;
  LIST_ENTRY                Link;
  EFI_PHYSICAL_ADDRESS      MemStart;
  EFI_PHYSICAL_ADDRESS      MemEnd;
} SKIP_MEMORY_RANGE;

#pragma pack (1)
typedef struct {
  EFI_PHYSICAL_ADDRESS      MemStart;
  EFI_PHYSICAL_ADDRESS      MemEnd;
} MEMORY_RANGE;
#pragma pack ()

#define SKIP_MEMORY_RANGE_SIGNATURE SIGNATURE_32 ('S', 'M', 'R', 'S')
#define SKIP_MEMORY_RANGE_FROM_THIS(a)           CR(a, SKIP_MEMORY_RANGE, Link, SKIP_MEMORY_RANGE_SIGNATURE)
#define MAX_ADJUST_MEMORY_TIMES                  3

#endif
