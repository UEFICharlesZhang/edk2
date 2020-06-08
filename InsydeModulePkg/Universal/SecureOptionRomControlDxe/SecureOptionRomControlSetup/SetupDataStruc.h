/** @file
  Header file for Secure Option Rom Control page

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _SETUPDATASTRUC_H_
#define _SETUPDATASTRUC_H_

#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/SecureOptionRomControl.h>


#define CONFIG_VARSTORE_ID                  0x1033
#define SETTING_VARSTORE_ID                 0x1034

//
// Labels definition
//
#define SECURE_OPTION_ROM_POLICY_LABEL      0x3611
#define SECURE_OPTION_ROM_POLICY_END_LABEL  0x3612

//
// Keys definition
//
#define KEY_OPROM_POLICY_CONTROL            0x3681
#define KEY_OPROM_POLICY_INSERT             0x3682
#define KEY_OPROM_POLICY_DELETE             0x3683
#define KEY_LOAD_DEFAULT                    0x3684

#pragma pack(1)

typedef struct {
  UINT8         OptionRomPolicy;
} SECURE_OPROM_CONTROL_CONFIGURATION;


#define KEY_OPTION_ROM_POLICY_BASE          0x3500
#define MAX_OPTION_ROM_POLICY               0x100  

typedef struct {
  UINT8         OptionRomPolicySetting[MAX_OPTION_ROM_POLICY];
} OPTION_ROM_POLICY_SETTING;

#pragma pack()

#endif
