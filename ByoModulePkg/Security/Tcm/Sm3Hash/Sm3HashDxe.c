

// #include <uefi.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/TcmHashSm3.h>
#include "SM3HashLib.h"




STATIC TCM_HASH_SM3_PROTOCOL gHashTm3Protocol = {
  CalcSM3Hash,
};



/**
  The driver's entry point.

  It publishes EFI TCM Protocol.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.  
  @param[in] SystemTable  A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
Sm3HashDxeEntry (
  IN    EFI_HANDLE       ImageHandle,
  IN    EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS  Status;
  
  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gTcmHashSm3ProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &gHashTm3Protocol
                  );
  return Status;
}  