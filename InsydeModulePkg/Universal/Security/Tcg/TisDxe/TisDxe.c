/** @file
  Functions to access discrete TPM2 module.

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

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/Tpm2TisCommLib.h>
#include <Library/Tpm2PtpCommLib.h>

#include <Protocol/Tpm2Interface.h>
#include <Protocol/SmmBase2.h>

TPM2_INTERFACE_PROTOCOL                   mTisInterface;
TPM2_INTERFACE_PROTOCOL                   mSmmTisInterface;

EFI_STATUS
EFIAPI
TisDxeCommand (
  IN     TPM2_INTERFACE_PROTOCOL          *This,
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
EFIAPI
TisDxeRequestUseTpm (
  IN  TPM2_INTERFACE_PROTOCOL             *This
  )
{
  return TisPcRequestUseTpm ((TIS_PC_REGISTERS_PTR)(UINTN) PcdGet64 (PcdTpmBaseAddress));
}

EFI_STATUS
EFIAPI
TisDxeSend (
  IN  TPM2_INTERFACE_PROTOCOL             *This,
  IN  UINT8                               *BufferIn,
  IN  UINT32                              SizeIn
  )
{
  return TisSend (
           (TIS_PC_REGISTERS_PTR)(UINTN) PcdGet64 (PcdTpmBaseAddress),
           BufferIn,
           SizeIn
           );
}

EFI_STATUS
EFIAPI
TisDxeReceive (
  IN     TPM2_INTERFACE_PROTOCOL          *This,
  IN OUT UINT8                            *BufferOut,
  IN OUT UINT32                           *SizeOut
  )
{
  UINT32                                  Timeout;

  Timeout = TIS_TIMEOUT_B;
  if (This == &mSmmTisInterface) {
    //
    // To improve performance shorten timeout duration in SMM
    //
    Timeout = TIS_SMM_TIMEOUT;
  }

  return TisReceive (
           (TIS_PC_REGISTERS_PTR)(UINTN) PcdGet64 (PcdTpmBaseAddress),
           Timeout,
           BufferOut,
           SizeOut
           );
}

EFI_STATUS
EFIAPI
PtpDxeCommand (
  IN     TPM2_INTERFACE_PROTOCOL          *This,
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
EFIAPI
PtpDxeRequestUseTpm (
  IN  TPM2_INTERFACE_PROTOCOL             *This
  )
{
  return PtpTpm2RequestUseTpm ();
}

EFI_STATUS
EFIAPI
PtpDxeSend (
  IN  TPM2_INTERFACE_PROTOCOL             *This,
  IN  UINT8                               *BufferIn,
  IN  UINT32                              SizeIn
  )
{
  return PtpTpm2Send (
           BufferIn, 
           SizeIn
           );
}

EFI_STATUS
EFIAPI
PtpDxeReceive (
  IN     TPM2_INTERFACE_PROTOCOL          *This,
  IN OUT UINT8                            *BufferOut,
  IN OUT UINT32                           *SizeOut
  )
{
  return PtpTpm2Receive (
           BufferOut,
           SizeOut
           );
}

/**
  The driver's entry point. It publishes TPM2 Interface Protocol.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
DriverEntry (
  IN    EFI_HANDLE                        ImageHandle,
  IN    EFI_SYSTEM_TABLE                  *SystemTable
  )
{
  EFI_SMM_BASE2_PROTOCOL                  *SmmBase;
  BOOLEAN                                 InSmm;
  EFI_STATUS                              Status;
  EFI_SMM_SYSTEM_TABLE2                   *Smst;
  EFI_HANDLE                              Handle;
  VOID                                    *Interface;

  if (!TisPcPresenceCheck ((TIS_PC_REGISTERS_PTR)(UINTN) PcdGet64 (PcdTpmBaseAddress))) {
    DEBUG ((EFI_D_ERROR, "TPM not detected!\n"));
    return EFI_SUCCESS;
  }

  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **)&SmmBase
                  );
  if (!EFI_ERROR (Status)) {
    SmmBase->InSmm (SmmBase, &InSmm);
  } else {
    InSmm = FALSE;
  }

  if (!InSmm) {
    //
    // In boot service
    //
    if (IsCrbInterfaceActive () && FeaturePcdGet (PcdH2OTpm2PtpSupported)) {
      mTisInterface.SummitCommand  = PtpDxeCommand;
      mTisInterface.RequestUse     = PtpDxeRequestUseTpm;
      mTisInterface.Send           = PtpDxeSend;
      mTisInterface.Receive        = PtpDxeReceive;
    } else {
      mTisInterface.SummitCommand  = TisDxeCommand;
      mTisInterface.RequestUse     = TisDxeRequestUseTpm;
      mTisInterface.Send           = TisDxeSend;
      mTisInterface.Receive        = TisDxeReceive;
    }

    Status = gBS->LocateProtocol (
                    &gTpm2InterfaceGuid,
                    NULL,
                    (VOID **)&Interface
                    );
    if (!EFI_ERROR (Status)) {
      //
      // TPM2 Interface Protocol already exists.
      // Return EFI_SUCCESS here to handle the case that fTPM driver executed prior to this driver.
      //
      return EFI_SUCCESS;
    }
    
    Handle = NULL;
    Status = gBS->InstallProtocolInterface (
                     &Handle,
                     &gTpm2InterfaceGuid,
                     EFI_NATIVE_INTERFACE,
                     &mTisInterface
                     );
    ASSERT_EFI_ERROR (Status);
  } else {
    //
    // In Smm
    //
    SmmBase->GetSmstLocation (SmmBase, &Smst);

    if (IsCrbInterfaceActive () && FeaturePcdGet (PcdH2OTpm2PtpSupported)) {
      mSmmTisInterface.SummitCommand  = PtpDxeCommand;
      mSmmTisInterface.RequestUse     = PtpDxeRequestUseTpm;
      mSmmTisInterface.Send           = PtpDxeSend;
      mSmmTisInterface.Receive        = PtpDxeReceive;
    } else {
      mSmmTisInterface.SummitCommand  = TisDxeCommand;
      mSmmTisInterface.RequestUse     = TisDxeRequestUseTpm;
      mSmmTisInterface.Send           = TisDxeSend;
      mSmmTisInterface.Receive        = TisDxeReceive;
    }

    Status = Smst->SmmLocateProtocol (
                     &gTpm2InterfaceGuid,
                     NULL,
                     (VOID **)&Interface
                     );
    if (!EFI_ERROR (Status)) {
      //
      // TPM2 Interface Protocol already exists.
      // Return EFI_SUCCESS here to handle the case that fTPM driver executed prior to this driver.
      //
      return EFI_SUCCESS;
    }
    
    Handle = NULL;
    Status = Smst->SmmInstallProtocolInterface (
                     &Handle,
                     &gTpm2InterfaceGuid,
                     EFI_NATIVE_INTERFACE,
                     &mSmmTisInterface
                     );
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}
