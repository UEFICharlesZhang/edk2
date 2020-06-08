/** @file

  Beep Status Code Smm implementation.

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

#include <BeepStatusCodeSmm.h>

EFI_SMM_RSC_HANDLER_PROTOCOL         *mRscHandlerProtocol = NULL;
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

  @param[in] Protocol   Points to the protocol's unique identifier.
  @param[in] Interface  Points to the interface instance.
  @param[in] Handle     The handle on which the interface was installed.                         
  
  @retval EFI_STATUS                

**/
EFI_STATUS
EFIAPI
RscHandlerProtocolCallback (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  EFI_STATUS                        Status;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmRscHandlerProtocolGuid,
                    NULL,
                    (VOID **) &mRscHandlerProtocol
                    );
  ASSERT_EFI_ERROR (Status);
  
  //
  // Register the worker function to ReportStatusCodeRouter
  //
  Status = mRscHandlerProtocol->Register (BeepStatusCode);
  
  ASSERT_EFI_ERROR (Status);
  
  return Status;
}

/**
  This function is to unregister the Beep Status Code callback function in SMM.

  @param[in]     DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]     Context         Points to an optional handler context which was specified when the
                                 handler was registered.
  @param[in,out] CommBuffer      A pointer to a collection of data in memory that will
                                 be conveyed from a non-SMM environment into an SMM environment.
  @param[in,out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS            The interrupt was handled and quiesced. No other handlers 
                                 should still be called.
**/
EFI_STATUS
EFIAPI
UnregisterBootTimeHandlers (
  IN     EFI_HANDLE                     DispatchHandle,
  IN     CONST VOID                     *RegisterContext,
  IN OUT VOID                           *CommBuffer,
  IN OUT UINTN                          *CommBufferSize
  )
{  
  SMM_BEEP_STATUS_CODE_COMMUNICATE       *SmmBscCommunicateData;  
  EFI_STATUS                             Status = EFI_UNSUPPORTED;

  SmmBscCommunicateData = (SMM_BEEP_STATUS_CODE_COMMUNICATE *)CommBuffer;
  switch (SmmBscCommunicateData->Function) {
    case SMM_COMM_UNREGISTER_HANDLER:
      if (SmmBscCommunicateData->UnregisterService == FALSE) {
        SmmBscCommunicateData->ReturnStatus = EFI_UNSUPPORTED;
        break;
      }

      //
      // Unregister Status Code callback function.
      //
      if (mRscHandlerProtocol != NULL) {
        Status = mRscHandlerProtocol->Unregister (BeepStatusCode);
        if (EFI_ERROR (Status)) {
          DEBUG((EFI_D_ERROR, "Unregister BeepStatusCode() in SMM: %r\n", Status));
        }
      } 
      
      SmmBscCommunicateData->ReturnStatus = Status;
      break;
      
    default :
      SmmBscCommunicateData->ReturnStatus = EFI_UNSUPPORTED;
  }
  
  return EFI_SUCCESS;
}

/**

 Entry point of Beep Status Code Driver. 
 The Beep Status Code driver will handle all Beep events during SMM.

 @param  ImageHandle       The firmware allocated handle for the EFI image.
 @param  SystemTable       A pointer to the EFI System Table. 
   
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BeepStatusCodeSmmEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  ) 
{
  EFI_STATUS                                Status;
  EFI_EVENT                                 RscHandlerProtocolEvent;
  EFI_HANDLE                                DispatchHandle;

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmRscHandlerProtocolGuid,
                    NULL,
                    (VOID **) &mRscHandlerProtocol
                    );
  if (EFI_ERROR (Status)) {
    //
    // Create callback for registering the Beep Status Code worker function.
    //
    Status = gSmst->SmmRegisterProtocolNotify (
                      &gEfiSmmRscHandlerProtocolGuid,
                      RscHandlerProtocolCallback,
                      &RscHandlerProtocolEvent
                      );
  } else {
    //
    // Register the worker function to ReportStatusCodeRouter
    //
    Status = mRscHandlerProtocol->Register (BeepStatusCode);    
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
  // To unregister the Beep Status Code callback function in SMM after exiting Boot Service.
  //
  Status = gSmst->SmiHandlerRegister (
                    UnregisterBootTimeHandlers,
                    &gH2OBeepStatusCodeCommunicationGuid,
                    &DispatchHandle
                    );
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "Register UnregisterBootTimeHandlers(): %r\n", Status));
  }

  return Status;
}
