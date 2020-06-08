/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _IHISI_OEM_EXTRA_DATA_COMMUNICATION_H_
#define _IHISI_OEM_EXTRA_DATA_COMMUNICATION_H_

#include "IhisiRegistration.h"

typedef
EFI_STATUS
(EFIAPI *OEM_EXT_FUNCTION) (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  );

//
// Oem extra communiction typedef struct
//
typedef struct {
  UINT8                                 SubFunID;
  OEM_EXT_FUNCTION                      OemExtCommunicationSubFun;
} OEM_EXT_COMMUNICAT_FUNCTION;

//
// Oem extra read/write typedef struct
//
typedef struct {
  UINT8                                 SubFunID;
  IHISI_FUNCTION                        OemExtSubFun;
} OEM_EXT_READ_WRITE_FUNCTION;

//
// OEM Extra Data Communication Function Prototype
//

/**
  AH=41h, OEM Extra Data Communication

  01h = VBIOS
  02h~03h = Reserved
  04h~0Ch = User Define
  0Dh~4Fh = Reserved
  50h = OA 3.0 Read/Write
  51h = OA 3.0 Erase (Reset to default)
  52h = OA 3.0 Populate Header
  53h = OA 3.0 De-Populate Header
  54h = Logo Update (Write)
  55h = Check BIOS sign by System BIOS
  56~FFh = Reserved

  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
OemExtraDataCommunication (
  VOID
  );

/**
  AH=42h, OEM Extra Data Write.

  Function 41h and 42h (or 47h) are pairs. The function 41h has to be called before calling
  into function 42h.

  @retval EFI_SUCCESS            OEM Extra Data Write successful.
  @return Other                  OEM Extra Data Write failed.
**/
EFI_STATUS
OemExtraDataWrite (
  VOID
  );

/**
  Function to read OA3.0 data.

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
OemExtraDataRead (
  VOID
  );


//
// OA 3.0 Function Prototype
//

/**
  AH=41h, OEM Extra Data Communication type 50h to read/write OA3.0.

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS  Read or write OA3.0 successful.
**/
EFI_STATUS
OemExtraOa30ReadWriteFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  );

/**
  AH=41h, OEM Extra Data Communication type 51h to erase OA3.0 (reset to default).

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS  Erase OA3.0 successful.
**/
EFI_STATUS
OemExtraOa30EraseFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  );

/**
  AH=41h, OEM Extra Data Communication type 52h to populate header.

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS   populate header successful.
**/
EFI_STATUS
OemExtraOa30PopulateHeaderFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  );

/**
  AH=41h, OEM Extra Data Communication type 53h to de-populate header.

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS   populate header successful.
**/
EFI_STATUS
OemExtraOa30DePopulateHeaderFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  );

//
// Oem Extra Data Communication CsService Functions
//
EFI_STATUS
SmmCsSvcIhisiOemExtCommunication(
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
);


//BLD_Charles_035+s
EFI_STATUS
SubFunc_NovoDataWrite(
  VOID
  );

EFI_STATUS
SubFunc_NovoDataRead(
  VOID
  );
//BLD_Charles_035+e

#endif
