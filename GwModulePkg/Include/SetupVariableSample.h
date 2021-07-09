/*++

Copyright (c) 2010 - 2015, GreatWall Corporation.<BR>
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced,
stored in a retrieval system, or transmitted in any form or by any
means without the express written consent of GreatWall Corporation.

File Name:

Abstract:
  Platform configuration setup.

Revision History:


--*/

#ifndef _SETUP_VARIABLE_SAMPLE_H
#define _SETUP_VARIABLE_SAMPLE_H

#define PLATFORM_SETUP_VARIABLE_GUID \
  { \
     0xEC87D643, 0xEBA4, 0x4BB5, {0xA1, 0xE5, 0x3F, 0x3E, 0x36, 0xB2, 0x0D, 0xA9} \
  }

#pragma pack(1)
// NOTE: When you add anything to this structure,
//   you MUST add it to the very bottom!!!!
//   You must make sure the structure size is able to divide by 32!
typedef struct {
  //
  // Keyboard
  //
  UINT8         Numlock;
  //
  // Boot from Network
  //
  UINT8         BootNetwork;
  //
  // Boot USB
  //
  UINT8         BootUsb;
  //
  // Language Select
  //
  UINT8         LanguageSelect;
  //
  // SATA Type
  //
  UINT16         SataType;
  //
  // Memory
  //
  UINT8         MemoryMode;
  UINT16        MemorySpeed;
} SYSTEM_CONFIGURATION;
#pragma pack()

#ifndef VFRCOMPILE
  extern EFI_GUID    gPlatformSetupVariableGuid;
  #define PLATFORM_SETUP_VARIABLE_NAME    L"Setup"
  #define PLATFORM_SETUP_VARIABLE_FLAG    (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)
#endif

#ifndef SETUP_DATA
  #define SETUP_DATA SYSTEM_CONFIGURATION  
#endif

#ifndef SETUP_DATA_VARSTORE
  #define SETUP_DATA_VARSTORE \
    efivarstore SETUP_DATA, varid = 1, attribute = 0x7, name  = Setup, guid  = PLATFORM_SETUP_VARIABLE_GUID;
#endif

#endif //End,#ifndef _SETUP_VARIABLE
