/** @file
  This library abstract how to access TPM2 hardware device.

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

#include <IndustryStandard/Tpm20.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/Tpm2PtpCommLib.h>

#ifdef EFI_DEBUG
/**
  Prints command or response buffer for debugging purposes.

  @param[in] Buffer     Buffer to print.
  @param[in] BufferSize Buffer data length.
**/
VOID
EFIAPI
PtpPrintBuffer(IN UINT8 *Buffer, IN UINT32 BufferSize)
{
  UINT32 Index;

  DEBUG ((DEBUG_INFO, "Buffer Address: 0x%08x, Size: 0x%08x, Value:\n", Buffer, BufferSize));
  for(Index = 0; Index < BufferSize; Index++){
    DEBUG ((DEBUG_INFO, "%02x ", *(Buffer + Index)));
   if((Index+1) % 16 == 0) DEBUG ((DEBUG_INFO, "\n"));
  }
  DEBUG ((DEBUG_INFO, "\n"));
}
#endif // EFI_DEBUG

/**
  Copy data from MMIO region to system memory by using 8-bit access.

  Copy data from MMIO region specified by starting address StartAddress
  to system memory specified by Buffer by using 8-bit access. The total
  number of byte to be copied is specified by Length. Buffer is returned.

  If Length is greater than (MAX_ADDRESS - StartAddress + 1), then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().


  @param  StartAddress    The starting address for the MMIO region to be copied from.
  @param  Length          The size in bytes of the copy.
  @param  Buffer          The pointer to a system memory buffer receiving the data read.

  @return Buffer

**/
UINT8 *
EFIAPI
MmioReadBuffer8 (
  IN  UINTN       StartAddress,
  IN  UINTN       Length,
  OUT UINT8       *Buffer
  )
{
  UINT8   *ReturnBuffer;

  ASSERT ((Length - 1) <=  (MAX_ADDRESS - StartAddress));
  ASSERT ((Length - 1) <=  (MAX_ADDRESS - (UINTN) Buffer));

  ReturnBuffer = Buffer;

  while (Length-- > 0) {
    *(Buffer++) = MmioRead8 (StartAddress++);
  }

  return ReturnBuffer;
}

/**
  Copy data from system memory to the MMIO region by using 8-bit access.

  Copy data from system memory specified by Buffer to the MMIO region specified 
  by starting address StartAddress by using 8-bit access. The total number 
  of byte to be copied is specified by Length. Buffer is returned.
  
  If Length is greater than (MAX_ADDRESS - StartAddress + 1), then ASSERT(). 
  If Length is greater than (MAX_ADDRESS -Buffer + 1), then ASSERT().


  @param  StartAddress    The starting address for the MMIO region to be copied to.
  @param  Length          The size, in bytes, of Buffer.
  @param  Buffer          The pointer to a system memory buffer containing the data to write.

  @return Buffer

**/
UINT8 *
EFIAPI
MmioWriteBuffer8 (
  IN  UINTN         StartAddress,
  IN  UINTN         Length,
  IN  CONST UINT8   *Buffer
  )
{
  VOID* ReturnBuffer;

  ASSERT ((Length - 1) <=  (MAX_ADDRESS - StartAddress));
  ASSERT ((Length - 1) <=  (MAX_ADDRESS - (UINTN) Buffer));
 
  ReturnBuffer = (UINT8 *) Buffer;
  
  while (Length-- != 0) {
     MmioWrite8 (StartAddress++, *(Buffer++));
  }

  return ReturnBuffer;
 
}

/**
  Check whether CRB interface activated.

  @retval TRUE        CRB is active.
  @retval FALSE       CRB is not active.
**/
BOOLEAN
EFIAPI
IsCrbInterfaceActive (
  VOID
  )
{
  UINT8       InterfaceType;

  InterfaceType = MmioRead8 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_TPM_INTERFACE_ID) & 0xF;
  if (InterfaceType == B_TPM_INTERFACE_CRB) {
    return TRUE;
  }
  return FALSE;
}

/**
  Sets TPM_LOC_CTRL register to a defined value to indicate that a command is
  available for processing.

  @retval EFI_SUCCESS   Register successfully written.
  @retval TBD

**/
EFI_STATUS
EFIAPI
PtpTpm2RequestUseTpm (
  VOID
  )
{

  UINT32      LocalityState;
  UINT32      ControlStatus;
  UINT32      WaitTime;

  //
  // Make sure TPM is not in fatal error state
  //
  ControlStatus = MmioRead32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_STS);

  if (( ControlStatus & B_CRB_CONTROL_STS_TPM_STATUS ) != 0 ) {
    DEBUG ((DEBUG_INFO, "ControlStatus = 0x%x\n", ControlStatus));
  }

  LocalityState = MmioRead32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_PTP_LOCALITY_STATE);

  if((((LocalityState & V_CRB_LOCALITY_STATE_ACTIVE_LOC_MASK) >> 2) == 0) &&
      ((LocalityState & B_CRB_LOCALITY_STATE_LOCALITY_ASSIGNED) != 0)) {
    DEBUG ((DEBUG_INFO, "Locality 0 already assigned\n"));
    return EFI_SUCCESS;
  } else {
    DEBUG ((DEBUG_INFO, "Requesting Locality\n"));
  }

  //
  // Request access to locality
  //
  MmioWrite32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_TPM_LOCALITY_CONTROL,
               B_CRB_LOCALITY_CTL_REQUEST_ACCESS);
  //
  // Wait for assignment of locality
  //
  LocalityState = 0;
  WaitTime = 0;
  while ((WaitTime < PTP_HCI_TIMEOUT_A) &&
         ((LocalityState & B_CRB_LOCALITY_STATE_REGISTER_VALID) != 0) &&
         ((LocalityState & B_CRB_LOCALITY_STATE_LOCALITY_ASSIGNED) != 0) &&
         (((LocalityState & V_CRB_LOCALITY_STATE_ACTIVE_LOC_MASK) >> 2) == 0)) {

    LocalityState = MmioRead32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_PTP_LOCALITY_STATE);

    MicroSecondDelay (PTP_HCI_POLLING_PERIOD);
    WaitTime += PTP_HCI_POLLING_PERIOD;
  }
  if (WaitTime >= PTP_HCI_TIMEOUT_A) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Sends command to TPM for execution.

  @param[in] Buffer    Buffer for TPM command data.
  @param[in] DataLength   TPM command data length.

  @retval EFI_SUCCESS     Operation completed successfully.
  @retval EFI_TIMEOUT     The register can't run into the expected status in time.
**/
EFI_STATUS
EFIAPI
PtpTpm2Send(
  IN     UINT8      *Buffer,
  IN     UINT32     DataLength
  )
{
  UINT32      ControlStatus;
  UINT32      WaitTime;

  //
  // Make sure TPM is not in fatal error state
  //
  ControlStatus = MmioRead32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_STS);

  if (( ControlStatus & B_CRB_CONTROL_STS_TPM_STATUS ) != 0 ) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Wait for tpm to go Idle and finish previous command
  //
  WaitTime = 0;
  while ((WaitTime < PTP_HCI_TIMEOUT_C) &&
          ((ControlStatus & B_CRB_CONTROL_STS_TPM_IDLE) != B_CRB_CONTROL_STS_TPM_IDLE)) {

    ControlStatus = MmioRead32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_STS);

    MicroSecondDelay (PTP_HCI_POLLING_PERIOD);
    WaitTime += PTP_HCI_POLLING_PERIOD;
  }
  if (WaitTime >= PTP_HCI_TIMEOUT_C) {
    DEBUG ((DEBUG_INFO, "TPM Already in command ready state \n"));
  }

  //
  // Request TPM to come out of idle
  //
  MmioWrite32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_REQ, B_R_CRB_CONTROL_REQ_COMMAND_READY);

  //
  // Wait for tpm to clear tpmidle
  //
   WaitTime = 0;
  while ((WaitTime < PTP_HCI_TIMEOUT_C) &&
          ((ControlStatus & B_CRB_CONTROL_STS_TPM_IDLE) != 0)) {

    ControlStatus = MmioRead32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_STS);

    MicroSecondDelay (PTP_HCI_POLLING_PERIOD);
    WaitTime += PTP_HCI_POLLING_PERIOD;
  }
  if (WaitTime >= PTP_HCI_TIMEOUT_C) {
    return EFI_TIMEOUT;
  }

  MmioWriteBuffer8 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_DATA_BUFFER, DataLength, (UINT8*)Buffer);

  //
  // Trigger Command processing by writing to start register
  //
  MmioWrite32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_START, B_CRB_CONTROL_START);

  return EFI_SUCCESS;
}

/**
  Receives response data of last command from TPM.

  @param[out] Buffer        Buffer for response data.
  @param[out] RespSize          Response data length.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_TIMEOUT           The register can't run into the expected status in time.
  @retval EFI_DEVICE_ERROR      Unexpected device status.
  @retval EFI_BUFFER_TOO_SMALL  Response data is too long.
**/
EFI_STATUS
EFIAPI
PtpTpm2Receive(
  OUT     UINT8     *Buffer,
  OUT     UINT32    *RespSize
  )
{
  EFI_STATUS  Status;
  UINT32      ControlStatus;
  UINT32      WaitTime;
  UINT16      Data16;
  UINT32      Data32;
  
  Status = EFI_SUCCESS;
  DEBUG ((DEBUG_INFO, "TPM_CRB: Tpm2PtpReceive start\n"));

  //
  // Wait for command completion
  //
  WaitTime = 0;
  ControlStatus = 0;
  ControlStatus = MmioRead32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_START);

  while ((WaitTime < PTP_HCI_TIMEOUT_B) &&
        ((ControlStatus & B_CRB_CONTROL_START) != 0)) {

    ControlStatus = MmioRead32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_START);

    MicroSecondDelay (PTP_HCI_POLLING_PERIOD);
    WaitTime += PTP_HCI_POLLING_PERIOD;
  }
  if (WaitTime >= PTP_HCI_TIMEOUT_B) {
    return EFI_TIMEOUT;
  }
  //
  // Read the response data header
  //
  MmioReadBuffer8 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_DATA_BUFFER, PTP_HCI_RESPONSE_HEADER_SIZE, (UINT8 *)Buffer);

  //
  // Check the reponse data header (tag, parasize and returncode)
  //
  CopyMem (&Data16, Buffer, sizeof (UINT16));
  DEBUG ((DEBUG_INFO, "TPM2_RESPONSE_HEADER.tag = 0x%04x\n", SwapBytes16(Data16)));

  //
  // TPM Rev 2.0 Part 2 - 6.9 TPM_ST (Structure Tags)
  // TPM_ST_RSP_COMMAND - Used in a response that has an error in the tag.
  //
  if (SwapBytes16(Data16) == TPM_ST_RSP_COMMAND) {
    DEBUG ((DEBUG_ERROR, "TPM2_RESPONSE_HEADER.tag = TPM_ST_RSP_COMMAND - Error in response!\n"));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  CopyMem(&Data32, (Buffer + 2), sizeof(UINT32));
  DEBUG ((DEBUG_INFO, "TPM2_RESPONSE_HEADER.paramSize = 0x%08x\n", SwapBytes32(Data32)));

  *RespSize = SwapBytes32(Data32);

  if(*RespSize == sizeof(TPM2_RESPONSE_HEADER)) {
    Status = EFI_SUCCESS;
    goto Exit;
  }
  if(*RespSize < sizeof(TPM2_RESPONSE_HEADER)) {
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }
  if(*RespSize > S_PTP_HCI_CRB_LENGTH) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Exit;
  }

  //
  // Read the entire response data header
  //
  MmioReadBuffer8 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_DATA_BUFFER, *RespSize,  (UINT8 *)Buffer);

  Exit:

    return Status;
}

/**
  Sends formatted command to TPM for execution and returns formatted response data.

  @param[in]  InputBuffer       Buffer for the input data.
  @param[in]  InputBufferSize   Size of the input buffer.
  @param[out] ReturnBuffer      Buffer for the output data.
  @param[out] ReturnBufferSize  Size of the output buffer.

  @retval EFI_SUCCESS  Operation completed successfully.
  @retval EFI_TIMEOUT  The register can't run into the expected status in time.
**/
EFI_STATUS
EFIAPI
PtpTpm2SubmitCommand (
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN OUT UINT32        *OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  )
{
  EFI_STATUS  Status;
  
  DEBUG ((DEBUG_INFO, "TPM_CRB: SubmitCommand start\n"));

  if(InputParameterBlock == NULL || OutputParameterBlock == NULL || InputParameterBlockSize == 0) {
    DEBUG ((DEBUG_ERROR, "Buffer == NULL or InputBufferSize == 0\n"));
    return EFI_INVALID_PARAMETER;
  }
  
  PtpTpm2RequestUseTpm ();
  
  //
  // Send the command to TPM
  //
  #ifdef EFI_DEBUG
  DEBUG ((DEBUG_INFO, "Buffer Dump: Command \n"));
  PtpPrintBuffer ( InputParameterBlock, InputParameterBlockSize);
  #endif // EFI_DEBUG
  Status = PtpTpm2Send (InputParameterBlock, InputParameterBlockSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Tpm2PtpSend EFI_ERROR = %r\n", Status));
    return Status;
  }

  //
  // Receive the response data from TPM
  //
  Status = PtpTpm2Receive (OutputParameterBlock, OutputParameterBlockSize);
  #ifdef EFI_DEBUG
  DEBUG ((DEBUG_INFO, "Buffer Dump: Response \n"));
  PtpPrintBuffer ( OutputParameterBlock, *OutputParameterBlockSize);
  #endif // EFI_DEBUG
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Tpm2PtpReceive EFI_ERROR = %r\n", Status));
    if (Status == EFI_TIMEOUT) {
      //
      // Cancel the command
      //
      MmioWrite32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_CANCEL, B_CRB_CONTROL_CANCEL);
      MicroSecondDelay (PTP_HCI_TIMEOUT_B);
    }
  }

  MmioWrite32 ((UINTN) PcdGet64 (PcdTpmBaseAddress) + R_CRB_CONTROL_REQ, B_R_CRB_CONTROL_REQ_GO_IDLE);

  return Status;
}
