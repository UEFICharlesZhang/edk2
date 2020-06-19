/** @file

Copyright (c) 2006 - 2011, Byosoft Corporation.<BR> 
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in 
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced, 
stored in a retrieval system, or transmitted in any form or by any 
means without the express written consent of Byosoft Corporation.

**/


#ifndef __TCM_UPDATE_STATUS_PROTOCOL_H__
#define __TCM_UPDATE_STATUS_PROTOCOL_H__

//-------------------------------------------------------------
// {07EEBB0A-21E9-49c9-AE29-F532E44ECDB8}
#define TCM_UPDATE_STATUS_PROTOCOL_GUID \
  { 0x7eebb0a, 0x21e9, 0x49c9, { 0xae, 0x29, 0xf5, 0x32, 0xe4, 0x4e, 0xcd, 0xb8 } }

#define TCM_STATUS_ENABLE        1
#define TCM_STATUS_DISABLE       2
#define TCM_STATUS_CLEAR         3


typedef
EFI_STATUS
(EFIAPI *TCM_UPDATE_STATUS)(
  IN UINTN  NewStatus
  );

typedef struct {
    TCM_UPDATE_STATUS  Update;
} TCM_UPDATE_STATUS_PROTOCOL;


extern EFI_GUID gTcmUpdateStatusProtocolGuid;

//-------------------------------------------------------------
#endif
