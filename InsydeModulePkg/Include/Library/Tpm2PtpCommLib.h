/** @file
  Implements TPM2 PTP ( Platform TPM Profile ) Device Library
  
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

#ifndef _TPM2_PTP_COMM_LIB_H_
#define _TPM2_PTP_COMM_LIB_H_

#include <Uefi.h>

//
// Default Timeout values
//
#define PTP_HCI_TIMEOUT_A                   750  * 1000  // 750 ms
#define PTP_HCI_TIMEOUT_B                   2000 * 1000  //   2 s
#define PTP_HCI_TIMEOUT_C                   200  * 1000  // 200 ms
#define PTP_HCI_TIMEOUT_D                   30   * 1000  // 30  ms
#define PTP_HCI_POLLING_PERIOD              140          // Poll register every 140 microsecondss

#define PTP_HCI_RESPONSE_HEADER_SIZE        12           // TPM2_RESPONSE_HEADER size 

#define TPM_LOCALITY_BUFFER_SIZE            0x10000      // Locality Buffer Size

//
// PTP HCI Control Area
//
#define R_PTP_LOCALITY_STATE                0x00
#define R_TPM_LOCALITY_CONTROL              0X08
#define R_TPM_LOCALITY_STATUS               0x0C
#define R_TPM_INTERFACE_ID                  0x30
#define R_CRB_CONTROL_EXT                   0x38
#define R_CRB_CONTROL_REQ                   0x40
#define R_CRB_CONTROL_STS                   0x44
#define R_CRB_CONTROL_CANCEL                0x48
#define R_CRB_CONTROL_START                 0x4C
#define R_CRB_CONTROL_INT                   0x50
#define R_CRB_CONTROL_CMD_SIZE              0x58
#define R_CRB_CONTROL_CMD_LOW               0x5C
#define R_CRB_CONTROL_CMD_HIGH              0x60
#define R_CRB_CONTROL_RESPONSE_SIZE         0x64
#define R_CRB_CONTROL_RESPONSE_ADDR         0x68
#define R_CRB_DATA_BUFFER                   0x80

//
// R_CRB_CONTROL_STS Bits
//
#define B_CRB_CONTROL_STS_TPM_STATUS             0x00000001 // BIT0
#define B_CRB_CONTROL_STS_TPM_IDLE               0x00000002 // BIT1

//
// R_CRB_CONTROL_REQ Bits
//
#define B_R_CRB_CONTROL_REQ_COMMAND_READY        0x00000001 // BIT0
#define B_R_CRB_CONTROL_REQ_GO_IDLE              0x00000002 // BIT1

//
// R_CRB_CONTROL_START Bits
//
#define B_CRB_CONTROL_START                      0x00000001 // BIT0

//
// R_TPM_INTERFACE_ID Bits
//
#define B_TPM_INTERFACE_FIFO                     0x0000
#define B_TPM_INTERFACE_CRB                      0x0001
#define B_TPM_INTERFACE_TIS                      0xFFFF
//
// R_CRB_CONTROL_CANCEL Bits
//
#define B_CRB_CONTROL_CANCEL                     0x00000001 // BIT0

//
// R_TPM_LOCALITY_STATUS Bits
//
#define B_CRB_LOCALITY_STS_GRANTED               0x00000001 // BIT0
#define B_CRB_LOCALITY_STS_BEEN_SEIZED           0x00000002 // BIT1

//
// R_TPM_LOCALITY_CONTROL Bits
//
#define B_CRB_LOCALITY_CTL_REQUEST_ACCESS        0x00000001 // BIT0
#define B_CRB_LOCALITY_CTL_RELINQUISH            0x00000002 // BIT1
#define B_CRB_LOCALITY_CTL_SEIZE                 0x00000004 // BIT2

//
// R_PTP_LOCALITY_STATE Bits
//
#define B_CRB_LOCALITY_STATE_TPM_ESTABLISHED     0x00000001 // BIT0
#define B_CRB_LOCALITY_STATE_LOCALITY_ASSIGNED   0x00000002 // BIT1
#define B_CRB_LOCALITY_STATE_REGISTER_VALID      0x00000080 // BIT7

//
// R_PTP_LOCALITY_STATE Mask Values
//
#define V_CRB_LOCALITY_STATE_ACTIVE_LOC_MASK     0x0000001C // Bits [4:2]

//
// Value written to R_PTP_HCI_CMD and CA_START
// to indicate that a command is available for processing
//
#define V_PTP_HCI_COMMAND_AVAILABLE_START        0x00000001
#define V_PTP_HCI_COMMAND_AVAILABLE_CMD          0x00000000
#define V_PTP_HCI_BUFFER_ADDRESS_RDY             0x00000003

//
// Ignore bit setting mask for WaitRegisterBits
//
#define V_PTP_HCI_IGNORE_BITS                    0x00000000

//
// All bits clear mask for WaitRegisterBits
//
#define V_PTP_HCI_ALL_BITS_CLEAR                 0xFFFFFFFF
#define V_PTP_HCI_START_CLEAR                    0x00000001

//
// Max CRB command/reponse buffer length
//
#define S_PTP_HCI_CRB_LENGTH                     3968 // 0xFED40080:0xFED40FFF = 3968 Bytes

/**
  Check whether CRB interface activated.

  @retval TRUE        CRB is active.
  @retval FALSE       CRB is not active.
**/
BOOLEAN
EFIAPI
IsCrbInterfaceActive (
  VOID
  );

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
  );

/**
  Sends command to TPM for execution.

  @param[in] Buffer    Buffer for TPM command data.
  @param[in] DataLength   TPM command data length.

  @retval EFI_SUCCESS     Operation completed successfully.
  @retval EFI_TIMEOUT     The register can't run into the expected status in time.
**/
EFI_STATUS
EFIAPI
PtpTpm2Send (
  IN     UINT8      *Buffer,
  IN     UINT32     DataLength
  );

/**
  Receives response data of last command from TPM.

  @param[out] Buffer            Buffer for response data.
  @param[out] RespSize          Response data length.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_TIMEOUT           The register can't run into the expected status in time.
  @retval EFI_DEVICE_ERROR      Unexpected device status.
  @retval EFI_BUFFER_TOO_SMALL  Response data is too long.
**/
EFI_STATUS
EFIAPI
PtpTpm2Receive (
  OUT     UINT8     *Buffer,
  OUT     UINT32    *RespSize
  );

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
  IN     UINT32            InputParameterBlockSize,
  IN     UINT8             *InputParameterBlock,
  IN OUT UINT32            *OutputParameterBlockSize,
  IN     UINT8             *OutputParameterBlock
  );

#endif //_TPM2_PTP_COMM_LIB_H_
