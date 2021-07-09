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

#ifndef _FORMSET_CONFIGURATION_H
#define _FORMSET_CONFIGURATION_H

#define GW_FORMSET_CLASS                       0
#define GW_FORMSET_SUB_CLASS                   0
#define ROOT_FORM_ID                        1
#define KEY_UNASSIGN_GROUP            0xFFFF

//
// Form ID, Must be Unique.
//
#define FORM_ID_SYSTEM_INFO                 0x1000

//
// Question ID, Must be Unique.
//
#define EXIT_KEY_SAVE                       0x2000
#define EXIT_KEY_DISCARD                    0x2001
#define EXIT_KEY_DEFAULT                    0x2002

#define  SEC_KEY_ADMIN_PD            0x3101
#define  SEC_KEY_POWER_ON_PD     0x3102
#define  SEC_KEY_CLEAR_USER_PD   0x3103


#endif // End,#ifndef _FORMSET_CONFIGURATION_H
