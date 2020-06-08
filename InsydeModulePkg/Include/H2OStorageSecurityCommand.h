/** @file
  H2O-specific Storage Security Command Protocol definition
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

#ifndef _H2O_STORAGE_SECURITY_COMMAND_H_
#define _H2O_STORAGE_SECURITY_COMMAND_H_

#include <Protocol/StorageSecurityCommand.h>
#include <IndustryStandard/Atapi.h>

#define H2O_SECURITY_PROTOCOL_ID             0x80

//
// Control parameter
//
//
// Password protection
//
#define SET_USER_PASSWORD                    0
#define SET_MASTER_PASSWORD                  1
//
// Write protection
//
#define SOFTWARE_WP                          0 // temporary write-protect
#define POWER_ON_WP                          1

//
// Identifier parameter
//
//
// Write protection
//
//
// MMC spec 4.5 and above supports enable protection on BPs seperately.
// Since BOOT_WP can only be set once for a power cycle, please make sure the protection on BPs is expected of you.
//
#define BP_WP_SEL_BOTH                       0
#define BP_WP_SEL_SEPARETE                   1

typedef struct {
  UINT32     HeaderSize;
  //
  // Control parameter
  // For password protection, Control describes which password type is going to be set.
  // For write protection, Control describes the write protection type.
  // For RPMB access, Control is reserved (zero).  
  //
  UINTN      Control;
  //
  // Identifier parameter
  // For password protection, Identifier describes master password identifier.
  // For write protection, Identifier describes the address of write-protect group if any in SOFTWARE_WP cases.
  // For write protection, Identifier describes Boot WP selection in POWER_ON_WP cases.
  // For RPMB access, Identifier is reserved (zero).  
  //
  UINTN      Identifier;
  //
  // Describes the size of Data (in bytes) and does not include the size of the header.
  // For password protection, MessageLength describes the length of password (Data)
  // For write protection, it's reserved.
  // For RPMB access, Data is the length of RPMB-key.
  //
  UINTN      MessageLength;
  //
  // Designates an array of bytes that is MessageLength in size.
  // For password protection, Data is the password string.
  // For write protection, Data is not used.
  // For RPMB access, Data is the RPMB-key.
  //
  //  UINT8     Data[1];
  //
} H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER;

typedef enum {
  //
  // Password protection
  //
  SEND_PWD_SET = 1,
  SEND_PWD_UNLOCK,
  SEND_PWD_CLEAR,
  SEND_PWD_FREEZE,
  SEND_PWD_RESET_SECURITY_STATUS,
  //
  // Write protection
  //
  SEND_WP_ENABLE,
  SEND_WP_DISABLE,
  //
  // RPMB access
  //
  SEND_RPMB_WR_REQ,
  SEND_RPMB_KEY_PRG,
  H2O_SSC_SEND_OP_MAX
} H2O_STORAGE_SECURITY_COMMAND_SEND_OPERATION; // SecurityProtocolSpecificData

typedef enum {
  RCV_IDENTIFY_DATA = 1,
  RCV_MAX_PWD_LEN,
  RCV_WP_GRP_SIZE,
  RCV_IS_RPMB_PROGRAMMED,
  H2O_SSC_RCV_OP_MAX
} H2O_STORAGE_SECURITY_COMMAND_RECEIVE_OPERATION; // SecurityProtocolSpecificData

#endif
