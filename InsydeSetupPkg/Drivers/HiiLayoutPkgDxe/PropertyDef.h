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

#ifndef _PROPERTY_DEF_H_
#define _PROPERTY_DEF_H_

#define SCU_DEFAULT_LAYOUT_ID                  0x0002
#define SCU_LMDE_LAYOUT_ID                     0x0003
#define SCU_RTDE_LAYOUT_ID                     0x0004
#define SCU_RBDE_LAYOUT_ID                     0x0005
#define SCU_LCDE_LAYOUT_ID                     0x0006

#define FRONT_PAGE_LMDE_LAYOUT_ID              0x0008
#define COMMON_PAGE_LMDE_LAYOUT_ID             0x0009
#define SCU_OEM_LAYOUT_ID                      0x0010
#define BOOT_MANAGER_LTDE_LAYOUT_ID            0x0011
#define BOOT_FROM_FILE_LTDE_LAYOUT_ID          0x0012

#define SCREEN_PANEL_ID                        0x0001
#define OWNER_DRAW_PANEL_ID                    0x0002
#define TITLE_PANEL_ID                         0x0003
#define SETUP_MENU_PANEL_ID                    0x0004
#define SETUP_PAGE_PANEL_ID                    0x0005
#define HELP_TEXT_PANEL_ID                     0x0006
#define HOTKEY_PANEL_ID                        0x0007
#define QUESTION_PANEL_ID                      0x0008
#define SETUP_PAGE_2_PANEL_ID                  0x0009

#define SETUP_UTILITY_FORMSET_CLASS_GUID {0x9f85453e, 0x2f03, 0x4989, 0xad, 0x3b, 0x4a, 0x84, 0x07, 0x91, 0xaf, 0x3a}

#define ADVANCED_VFR_ID                        0x0002
#define EVENT_LOG_FORM_ID                      0x0003

#define H2O_PANEL_TYPE_SCREEN                  0x00000000 // default
#define H2O_PANEL_TYPE_HOTKEY                  0x00000001
#define H2O_PANEL_TYPE_HELP_TEXT               0x00000002
#define H2O_PANEL_TYPE_SETUP_MENU              0x00000004
#define H2O_PANEL_TYPE_SETUP_PAGE              0x00000008
#define H2O_PANEL_TYPE_FORM_TITLE              0x00000010
#define H2O_PANEL_TYPE_QUESTION                0x00000020
#define H2O_PANEL_TYPE_OWNER_DRAW              0x00000040
#define H2O_PANEL_TYPE_SETUP_PAGE2             0x00000080

#define OEM_TEXT_GRAY                          rgb0xFF808285
#define OEM_BK_GRAY                            rgb0xFFE6E7E8
#define OEM_ORANGE                             rgb0xFFE45620
#define OEM_BLUE                               rgb0xFF19BEEC

#define H2O_CONTROL_MAIN_PAGE_ONE_OF_GUID \
{ \
  0xd197cfcd, 0x825f, 0x4182, 0xa6, 0x8a, 0xda, 0x7f, 0x0a, 0x1a, 0x17, 0xa3 \
}

#define RESOLUTION_VERTICAL                    0x00000000
#define RESOLUTION_HORIZONTAL                  0x00000001

#define DISPLAY_NONE                           0x00000000
#define DISPLAY_LEFT                           0x00000001
#define DISPLAY_TOP                            0x00000002
#define DISPLAY_RIGHT                          0x00000004
#define DISPLAY_BOTTOM                         0x00000008

#endif

