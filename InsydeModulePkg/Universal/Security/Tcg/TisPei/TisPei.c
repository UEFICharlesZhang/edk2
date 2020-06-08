/** @file
  Functions to access discrete TPM module.

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiPei.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/PerformanceLib.h>
#include <Library/BaseLib.h>
#include <Library/Tpm2TisCommLib.h>
#include <Library/Tpm2PtpCommLib.h>

#include <Ppi/Tpm2InterfacePei.h>

EFI_STATUS
TisPeiCommand (
  IN     EFI_PEI_SERVICES                 **PeiServices,
  IN     PEI_TPM2_INTERFACE_PPI           *This,
  IN     UINT8                            *BufferIn,
  IN     UINT32                           SizeIn,
  IN OUT UINT8                            *BufferOut,
  IN OUT UINT32                           *SizeOut
  )
{
  return TisTpmCommand (
           (TIS_PC_REGISTERS_PTR)(UINTN) PcdGet64 (PcdTpmBaseAddress),
           TIS_TIMEOUT_B,
           BufferIn,
           SizeIn,
           BufferOut,
           SizeOut
           );
}

EFI_STATUS
TisPeiRequestUseTpm (
  IN     EFI_PEI_SERVICES                 **PeiServices,
  IN     PEI_TPM2_INTERFACE_PPI           *This
  )
{
  return TisPcRequestUseTpm ((TIS_PC_REGISTERS_PTR)(UINTN) PcdGet64 (PcdTpmBaseAddress));
}

EFI_STATUS
TisPeiSend (
  IN     EFI_PEI_SERVICES                 **PeiServices,
  IN     PEI_TPM2_INTERFACE_PPI           *This,
  IN     UINT8                            *BufferIn,
  IN     UINT32                           SizeIn
  )
{
  return TisSend (
           (TIS_PC_REGISTERS_PTR)(UINTN) PcdGet64 (PcdTpmBaseAddress),
           BufferIn,
           SizeIn
           );
}

EFI_STATUS
TisPeiReceive (
  IN     EFI_PEI_SERVICES                 **PeiServices,
  IN     PEI_TPM2_INTERFACE_PPI           *This,
  IN OUT UINT8                            *BufferOut,
  IN OUT UINT32                           *SizeOut
  )
{
  return TisReceive (
           (TIS_PC_REGISTERS_PTR)(UINTN) PcdGet64 (PcdTpmBaseAddress),
           TIS_TIMEOUT_B,
           BufferOut,
           SizeOut
           );
}

PEI_TPM2_INTERFACE_PPI         mPeiTisTpm2InterfacePpi = {
  TisPeiCommand,
  TisPeiRequestUseTpm,
  TisPeiSend,
  TisPeiReceive
};

EFI_PEI_PPI_DESCRIPTOR   mInstallPeiTisTpm2InterfacePpi = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gPeiTpm2InterfacePpiGuid,
  &mPeiTisTpm2InterfacePpi
};

EFI_STATUS
PtpPeiCommand (
  IN     EFI_PEI_SERVICES                 **PeiServices,
  IN     PEI_TPM2_INTERFACE_PPI           *This,
  IN     UINT8                            *BufferIn,
  IN     UINT32                           SizeIn,
  IN OUT UINT8                            *BufferOut,
  IN OUT UINT32                           *SizeOut
  )
{
  return PtpTpm2SubmitCommand (
           SizeIn,
           BufferIn,
           SizeOut,
           BufferOut
           );
}

EFI_STATUS
PtpPeiRequestUseTpm (
  IN     EFI_PEI_SERVICES                 **PeiServices,
  IN     PEI_TPM2_INTERFACE_PPI           *This
  )
{
  return PtpTpm2RequestUseTpm ();
}

EFI_STATUS
PtpPeiSend (
  IN     EFI_PEI_SERVICES                 **PeiServices,
  IN     PEI_TPM2_INTERFACE_PPI           *This,
  IN     UINT8                            *BufferIn,
  IN     UINT32                           SizeIn
  )
{
  return PtpTpm2Send (
           BufferIn,
           SizeIn
           );
}

EFI_STATUS
PtpPeiReceive (
  IN     EFI_PEI_SERVICES                 **PeiServices,
  IN     PEI_TPM2_INTERFACE_PPI           *This,
  IN OUT UINT8                            *BufferOut,
  IN OUT UINT32                           *SizeOut
  )
{
  return PtpTpm2Receive (
           BufferOut,
           SizeOut
           );
}

PEI_TPM2_INTERFACE_PPI         mPeiPtpTpm2InterfacePpi = {
  PtpPeiCommand,
  PtpPeiRequestUseTpm,
  PtpPeiSend,
  PtpPeiReceive
};

EFI_PEI_PPI_DESCRIPTOR   mInstallPeiPtpTpm2InterfacePpi = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gPeiTpm2InterfacePpiGuid,
  &mPeiPtpTpm2InterfacePpi
};

/**
  Entry point of this module.

  @param[in] FileHandle   Handle of the file being invoked.
  @param[in] PeiServices  Describes the list of possible PEI Services.

  @return Status.

**/
EFI_STATUS
EFIAPI
PeimEntry (
  IN       EFI_PEI_FILE_HANDLE            FileHandle,
  IN CONST EFI_PEI_SERVICES               **PeiServices
  )
{
  EFI_STATUS                              Status;
  EFI_PEI_PPI_DESCRIPTOR                  *PpiDescriptor;
  VOID                                    *Ppi;

  if (!TisPcPresenceCheck ((TIS_PC_REGISTERS_PTR)(UINTN) PcdGet64 (PcdTpmBaseAddress))) {
    DEBUG ((EFI_D_ERROR, "TPM not detected!\n"));
    return EFI_SUCCESS;
  }

  Ppi           = NULL;
  PpiDescriptor = NULL;
  Status = PeiServicesLocatePpi (
             &gPeiTpm2InterfacePpiGuid,
             0,
             &PpiDescriptor,
             (VOID **) &Ppi
             );
  if (!EFI_ERROR (Status)) {
    //
    // TPM2 Interface PPI already exists.
    // Return EFI_SUCCESS here to handle the case that fTPM driver executed prior to this driver.
    //
    return EFI_SUCCESS;
  }
  
  //
  // Install PPI
  //
  if (IsCrbInterfaceActive () && FeaturePcdGet (PcdH2OTpm2PtpSupported)) {
    Status = PeiServicesInstallPpi (&mInstallPeiPtpTpm2InterfacePpi);
  } else {
    Status = PeiServicesInstallPpi (&mInstallPeiTisTpm2InterfacePpi);
  }
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
