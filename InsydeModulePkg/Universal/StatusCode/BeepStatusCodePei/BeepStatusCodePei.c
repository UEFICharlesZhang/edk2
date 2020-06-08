/** @file
  Implementation of Beep Status Code Pei module.

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

#include <BeepStatusCodePei.h>


EFI_PEI_NOTIFY_DESCRIPTOR mRscHandlerPpiNotify = {
                            (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
                            &gEfiPeiRscHandlerPpiGuid,
                            RscHandlerPpiNotifyCallback
                            };


/**
  This routine is the notification function for Report Status Code Handler Ppi.

  @param  PeiServices           Indirect reference to the PEI Services Table.
  @param  NotifyDescriptor      Address of the notification descriptor data structure. Type
                                EFI_PEI_NOTIFY_DESCRIPTOR is defined above.
  @param  Ppi                   Address of the PPI that was installed.

  @retval EFI_STATUS            

**/
EFI_STATUS
EFIAPI
RscHandlerPpiNotifyCallback (
  IN CONST EFI_PEI_SERVICES         **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR      *NotifyDescriptor,
  IN VOID                           *Ppi
  )
{
  EFI_STATUS                        Status;
  EFI_PEI_RSC_HANDLER_PPI           *RscHandlerPpi;

  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gEfiPeiRscHandlerPpiGuid,
                             0,
                             NULL,
                             (VOID **) &RscHandlerPpi
                             );
  if (!EFI_ERROR(Status)) {
    //
    // Register the worker function to ReportStatusCodeRouter
    //
    Status = RscHandlerPpi->Register (PeiBeepStatusCode);
  } 

  return Status;
}

/**

  This routine is to return the index number of Beep Type table.

  @param  BeepTypeId  The Id of the Beep Type table.    

  @retval Index number
            - 0xFF : The input Id is not available.
            - Other: The index number of the Beep Type table.

**/
UINT8
EFIAPI
GetBeepType (
  IN UINT8       BeepTypeId
  )
{
  BEEP_TYPE                               *BeepTypeList;
  UINT8                                   Index;

  BeepTypeList = (BEEP_TYPE *)PcdGetPtr (PcdBeepTypeList);

  Index = 0;
  while (BeepTypeList[Index].Index != 0xFF) {
    if (BeepTypeList[Index].Index == BeepTypeId) {
      return Index;
    }

    Index++;
  }

  //
  // Return Index as 0xFF to stand for no matched Beep Type. 
  //
  Index = END_BEEP_TYPE_INDEX;
  
  return Index;
  
}

/**
  This function is to handle Beep Status Code.

  @param  PeiServices      Indirect reference to the PEI Services Table.
  @param  CodeType         Indicates the type of status code being reported.
  @param  CodeValue        Describes the current status of a hardware or
                           software entity. This includes information about the class and
                           subclass that is used to classify the entity as well as an operation.
                           For progress codes, the operation is the current activity.
                           For error codes, it is the exception.For debug codes,it is not defined at this time.
  @param  Instance         The enumeration of a hardware or software entity within
                           the system. A system may contain multiple entities that match a class/subclass
                           pairing. The instance differentiates between them. An instance of 0 indicates
                           that instance information is unavailable, not meaningful, or not relevant.
                           Valid instance numbers start with 1.
  @param  CallerId         This optional parameter may be used to identify the caller.
                           This parameter allows the status code driver to apply different rules to
                           different callers.
  @param  Data             This optional parameter may be used to pass additional data.

  @retval EFI_SUCCESS      Function completes successfully.  
          EFI_UNSUPPORTED  Can not find the match beep type.
**/
EFI_STATUS
EFIAPI
PeiBeepStatusCode (
  IN CONST EFI_PEI_SERVICES         **PeiServices,
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue, 
  IN UINT32                         Instance  OPTIONAL,
  IN CONST EFI_GUID                 *CallerId OPTIONAL,
  IN CONST EFI_STATUS_CODE_DATA     *Data     OPTIONAL
  ) 
{ 
  UINTN                                 Index;
  STATUS_CODE_BEEP_ENTRY                *StatusCodeBeepList;
  BEEP_TYPE                             *BeepTypeList;
  EFI_STATUS                            Status;
  STATUS_CODE_BEEP_ENTRY                *BeepEntry;
  BEEP_TYPE                             *BeepType;
  UINT8                                 BeepTypeIndex;
  
  StatusCodeBeepList = NULL;
  BeepTypeList = NULL;
  Status = OemSvcUpdateStatusCodeBeep (&StatusCodeBeepList, &BeepTypeList);
  if (Status != EFI_MEDIA_CHANGED) {
    //
    // Can not get any OEM tables, use default tables.
    //
    StatusCodeBeepList = (STATUS_CODE_BEEP_ENTRY *)PcdGetPtr(PcdStatusCodeBeepList);
    BeepTypeList = (BEEP_TYPE *)PcdGetPtr (PcdBeepTypeList);
  }
  
  Index = 0;
  while (StatusCodeBeepList[Index].CodeType != 0) {  
    if ((StatusCodeBeepList[Index].CodeType == CodeType) && (StatusCodeBeepList[Index].CodeValue == CodeValue)) {
      BeepEntry = (STATUS_CODE_BEEP_ENTRY *)&(StatusCodeBeepList[Index]);
      BeepTypeIndex = GetBeepType (BeepEntry->BeepTypeId);
      if (BeepTypeIndex == END_BEEP_TYPE_INDEX) {
        //
        // Can not find the match beep type.
        //
        return EFI_UNSUPPORTED;
      }
      BeepType  = (BEEP_TYPE *)&(BeepTypeList[BeepTypeIndex]);
      //
      // Beep Handler to locate all Speaker Interface ppi instance.
      //
      BeepHandler (PeiServices, BeepEntry, BeepType);
      
    }
    
    Index++;
  }

  return EFI_SUCCESS;
}

/**
  Beep Handler for Status code.
  
  @param  PeiServices      Indirect reference to the PEI Services Table.
  @param  *BeepEntry       Pointer to the Beep statuc code entry.
  @param  *BeepType        Pointer to the Beep type.

  @retval EFI_SUCCESS     

**/
EFI_STATUS
EFIAPI
BeepHandler (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN STATUS_CODE_BEEP_ENTRY     *BeepEntry,
  IN BEEP_TYPE                  *BeepType
  )
{
  EFI_STATUS                              Status;
  EFI_STATUS                              PpiStatus;
  PEI_SPEAKER_IF_PPI                      *SpeakerPpi;
  UINT8                                   BeepDataCount;
  UINT8                                   BeepLoopIndex;
  UINT32                                  BeepDuration;
  UINT32                                  TimerInterval;
  UINTN                                   Index;
    
  //
  // Locate Speaker Interface Ppi.
  //
  Index = 0;
  do {
    PpiStatus = (*PeiServices)->LocatePpi (
                                  PeiServices,
                                  &gPeiSpeakerInterfacePpiGuid,
                                  Index,
                                  NULL,
                                  (VOID **)&SpeakerPpi
                                  );
    if (!EFI_ERROR (PpiStatus)) {
      for (BeepLoopIndex = 0; BeepLoopIndex < BeepEntry->BeepLoop; BeepLoopIndex++) {
        BeepDataCount = 0;
        while (BeepType->SoundType[BeepDataCount] != BEEP_NONE) {
          BeepDuration = 0;
          TimerInterval = 0;
          if (BeepType->SoundType[BeepDataCount] == BEEP_LONG) {
            BeepDuration  = BEEP_LONG_DURATION;
            TimerInterval = BEEP_LONG_TIME_INTERVAL;
          } else if (BeepType->SoundType[BeepDataCount] == BEEP_SHORT) {
            BeepDuration  = BEEP_SHORT_DURATION;
            TimerInterval = BEEP_SHORT_TIME_INTERVAL;
          } 
          
          Status = SpeakerPpi->GenerateBeep (
                                 (UINTN)0x1, 
                                 BeepDuration,
                                 TimerInterval 
                                 );
          if (EFI_ERROR (Status)) {
            return Status;
          }
          BeepDataCount++;
        }
      }
    }
    Index++;
  } while (!EFI_ERROR (PpiStatus));
  
  return EFI_SUCCESS;
}

/**
  Perform PEI Beep Status Code Init.

  @param [in] FileHandle           Handle of the file being invoked. 
  @param [in] PeiServices          Describes the list of possible PEI Services.

  @retval EFI Status            
**/
EFI_STATUS
EFIAPI
BeepStatusCodePeiEntryPoint (
  IN EFI_PEI_FILE_HANDLE        FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                            Status;
  EFI_PEI_RSC_HANDLER_PPI               *RscHandlerPpi;

  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gEfiPeiRscHandlerPpiGuid,
                             0,
                             NULL,
                             (VOID **) &RscHandlerPpi
                             );
  if (EFI_ERROR (Status)) {
    //
    // Register callback function for PEI_RSC_HANDLER_PPI.
    //
    Status = (**PeiServices).NotifyPpi (PeiServices, &mRscHandlerPpiNotify); 
    if (EFI_ERROR (Status)) {
      return Status;
    }
  } else {
    //
    // Register the worker function to ReportStatusCodeRouter
    //
    Status = RscHandlerPpi->Register (PeiBeepStatusCode);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  
  return Status;
}

