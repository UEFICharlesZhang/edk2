

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include "tcm_hash.h"
#include "tcm_ecc.h"
#include <Protocol/NzGmProtocol.h>


void *realloc(void *Buffer, UINTN Size)
{
  void  *NewBuffer;

  NewBuffer = AllocatePool(Size);
  CopyMem(NewBuffer, Buffer, Size);
  FreePool(Buffer);

  return NewBuffer;
}


EFI_NZGM_PROTOCOL gEfiNzGmProtocol = {
  tcm_sch_starts,
  tcm_sch_update,
  tcm_sch_finish,
  tcm_hmac,
  tcm_ecc_init,
  tcm_ecc_encrypt
};  


EFI_STATUS
EFIAPI
NzGmInit (
	IN EFI_HANDLE       ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
	)
{
  EFI_STATUS  Status;
  
  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiNzGmProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &gEfiNzGmProtocol
                  );
  return Status;                
}

