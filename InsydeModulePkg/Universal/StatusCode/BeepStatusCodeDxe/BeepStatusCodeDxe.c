/** @file

  Beep Status Code Dxe implementation.

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

#include <BeepStatusCodeDxe.h>

EFI_RSC_HANDLER_PROTOCOL              *mRscHandlerProtocol   = NULL;
EFI_EVENT                             mExitBootServicesEvent = NULL;
STATUS_CODE_BEEP_ENTRY                *mStatusCodeBeepList;
BEEP_TYPE                             *mBeepTypeList;

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

  BeepTypeList = (BEEP_TYPE *)mBeepTypeList;

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
*/
EFI_STATUS
EFIAPI
BeepStatusCode (
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue,
  IN UINT32                         Instance    OPTIONAL,
  IN EFI_GUID                       * CallerId  OPTIONAL,
  IN EFI_STATUS_CODE_DATA           * Data      OPTIONAL  
  ) 
{
  UINTN                                 Index;
  STATUS_CODE_BEEP_ENTRY                *BeepEntry;
  BEEP_TYPE                             *BeepType;
  UINT8                                 BeepTypeIndex;

  if (mStatusCodeBeepList == NULL || mBeepTypeList== NULL) {
    return EFI_UNSUPPORTED;
  }
  
  Index = 0;
  while (mStatusCodeBeepList[Index].CodeType != 0) {  
    if ((mStatusCodeBeepList[Index].CodeType == CodeType) && (mStatusCodeBeepList[Index].CodeValue == CodeValue)) {
      BeepEntry = (STATUS_CODE_BEEP_ENTRY *)&(mStatusCodeBeepList[Index]);
      BeepTypeIndex = GetBeepType (BeepEntry->BeepTypeId);
      if (BeepTypeIndex == END_BEEP_TYPE_INDEX) {
        //
        // Can not find the match beep type.
        //
        return EFI_UNSUPPORTED;
      }
      BeepType  = (BEEP_TYPE *)&(mBeepTypeList[BeepTypeIndex]);

      //
      // Beep Handler to locate all Speaker Interface protocol instance.
      //
      BeepHandler (BeepEntry, BeepType);
      break;
    }
    
    Index++;
  }
      
  return EFI_SUCCESS;
}

/**
  Beep Handler for Status code.
  
  @param  *BeepEntry       Pointer to the Beep statuc code entry.
  @param  *BeepType        Pointer to the Beep type.

  @retval EFI_SUCCESS     

**/
EFI_STATUS
EFIAPI
BeepHandler (
  IN STATUS_CODE_BEEP_ENTRY     *BeepEntry,
  IN BEEP_TYPE                  *BeepType
  )
{
  EFI_STATUS                              Status;
  EFI_SPEAKER_IF_PROTOCOL                 *SpeakerProtocol;
  UINT8                                   BeepDataCount;
  UINT8                                   BeepLoopIndex;
  UINT32                                  BeepDuration;
  UINT32                                  TimerInterval;
  EFI_HANDLE                              *HandleBuffer;
  UINTN                                   NumberOfHandles;
  UINTN                                   Index;  
  
  //
  // Locate Speaker Interface protocol.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSpeakerInterfaceProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }  

  for (Index = 0; Index < NumberOfHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiSpeakerInterfaceProtocolGuid,
                    (VOID **)&SpeakerProtocol
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

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
        
        Status = SpeakerProtocol->GenerateBeep (
                                    SpeakerProtocol,
                                    0x1, 
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
 
  return EFI_SUCCESS;
}

/**
  Notification function for ReportStatusCode Handler Protocol.

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               The pointer to the notification function's context,
                                which is implementation-dependent.                        
  
  @retval VOID                

**/
VOID
EFIAPI
RscHandlerProtocolCallback (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  )
{
  EFI_STATUS                        Status;

  Status = gBS->LocateProtocol (
                  &gEfiRscHandlerProtocolGuid,
                  NULL,
                  (VOID **) &mRscHandlerProtocol
                  );
  ASSERT_EFI_ERROR (Status);
  
  //
  // Register the worker function to ReportStatusCodeRouter
  //
  Status = mRscHandlerProtocol->Register (BeepStatusCode, TPL_HIGH_LEVEL);  
  
  ASSERT_EFI_ERROR (Status);

  return;
}

/**
  Unregister status code callback functions which only available at boot time. 

  @param  Event         Event whose notification function is being invoked.
  @param  Context       Pointer to the notification function's context, which is
                        always zero in current implementation.

**/
VOID
EFIAPI
UnregisterBootTimeHandlers (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  UINTN                                  CommSize;
  EFI_STATUS                             Status;
  EFI_SMM_COMMUNICATE_HEADER             *SmmCommBufferHeader;
  EFI_SMM_COMMUNICATION_PROTOCOL         *Communication;
  SMM_BEEP_STATUS_CODE_COMMUNICATE       *SmmBscCommunicateData;
  UINT8                                  SmmBscBuffer[SMM_BSC_COMMUNICATION_BUFFER_SIZE];
  
  if (mRscHandlerProtocol != NULL) {
    Status = mRscHandlerProtocol->Unregister (BeepStatusCode);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "Unregister BeepStatusCode() in DXE: %r\n", Status));
    }
  }

  //
  // Unregister SMM handler for Beep Status Code.
  //
  Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **)&Communication);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "BeepStatusCodeDxe locate SMM Communication protocol: %r\n", Status));
    return;
  }

  //
  // Use to notify SMM driver that the Beep service is not available after exiting boot services.
  //
  SmmCommBufferHeader = (EFI_SMM_COMMUNICATE_HEADER*)SmmBscBuffer;
  SmmBscCommunicateData = (SMM_BEEP_STATUS_CODE_COMMUNICATE *)SmmCommBufferHeader->Data;

  SmmBscCommunicateData->Function = SMM_COMM_UNREGISTER_HANDLER;
  SmmBscCommunicateData->UnregisterService = TRUE;
  SmmBscCommunicateData->ReturnStatus = EFI_UNSUPPORTED;
  
  CopyGuid (&SmmCommBufferHeader->HeaderGuid, &gH2OBeepStatusCodeCommunicationGuid);
  SmmCommBufferHeader->MessageLength = sizeof(SMM_BEEP_STATUS_CODE_COMMUNICATE);
  CommSize = OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data)  + sizeof(SMM_BEEP_STATUS_CODE_COMMUNICATE);
  
  Status = Communication->Communicate (Communication, SmmBscBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "BeepStatusCodeDxe Unregister SMM Handler: %r\n", Status));
    return;
  }

}

/**
 
 Entry point of Beep Status Code Driver.
 The Beep Status Code driver will handle all Beep events during DXE and BDS phase.

 @param  ImageHandle       The firmware allocated handle for the EFI image.
 @param  SystemTable       A pointer to the EFI System Table. 
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BeepStatusCodeDxeEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  ) 
{
  EFI_STATUS                                Status;
  EFI_EVENT                                 RscHandlerProtocolEvent;

  Status = gBS->LocateProtocol (
                  &gEfiRscHandlerProtocolGuid,
                  NULL,
                  (VOID **) &mRscHandlerProtocol
                  );
  if (EFI_ERROR (Status)) {
    //
    // Create callback for registering the Beep Status Code worker function.
    //
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    RscHandlerProtocolCallback,
                    NULL,
                    &gEfiRscHandlerProtocolGuid,
                    &RscHandlerProtocolEvent
                    );
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "Create event to register BeepStatusCode(): %r\n", Status));
    }
  } else {
    //
    // Register the worker function to ReportStatusCodeRouter.
    //
    Status = mRscHandlerProtocol->Register (BeepStatusCode, TPL_HIGH_LEVEL);    
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Get Beep related tables.
  //
  mStatusCodeBeepList = NULL;
  mBeepTypeList = NULL;
  Status = OemSvcUpdateStatusCodeBeep (&mStatusCodeBeepList, &mBeepTypeList);
  if (Status != EFI_MEDIA_CHANGED) {
    //
    // Can not get any OEM tables, use default tables.
    //
    mStatusCodeBeepList = (STATUS_CODE_BEEP_ENTRY *)PcdGetPtr(PcdStatusCodeBeepList);
    mBeepTypeList = (BEEP_TYPE *)PcdGetPtr (PcdBeepTypeList);
  }

  //
  // Create event callback to unregister the Beep Status Code worker function.
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  UnregisterBootTimeHandlers,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &mExitBootServicesEvent
                  );
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "Create event to unregister BeepStatusCode(): %r\n", Status));
  }

  return Status;
}
