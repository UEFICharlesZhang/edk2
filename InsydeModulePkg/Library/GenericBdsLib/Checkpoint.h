/** @file
  Header file of Checkpoint related functions

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
#ifndef _INTERNAL_CHECKPOINT_H_
#define _INTERNAL_CHECKPOINT_H_

/**
  Initalize H2O_BDS_CP_CON_IN_CONNECT_AFTER_PROTOCOL
  data and trigger gH2OBdsCpConInConnectAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConInConnectAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConInConnectAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConInConnectAfter (
  VOID
  );

/**
  Initalize H2O_BDS_CP_CON_IN_CONNECT_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpConInConnectBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConInConnectBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConInConnectBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConInConnectBefore (
  VOID
  );

/**
  Initalize H2O_BDS_CP_CON_OUT_CONNECT_AFTER_PROTOCOL
  data and trigger gH2OBdsCpConOutConnectAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConOutConnectAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConOutConnectAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConOutConnectAfter (
  VOID
  );

/**
  Initalize H2O_BDS_CP_CON_OUT_CONNECT_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpConOutConnectBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConOutConnectBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConOutConnectBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConOutConnectBefore (
  VOID
  );

#endif