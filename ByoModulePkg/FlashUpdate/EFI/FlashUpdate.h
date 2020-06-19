
#include <Uefi.h>

#include <Pi/PiDxeCis.h>

#include <Library/UefiApplicationEntryPoint.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/EfiShellParameters.h>
#include <Protocol/EfiShellInterface.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>

#include <Guid/FileInfo.h>
#include <SpiCommon.h>

EFI_STATUS
ReadFile (
  IN     CHAR16  *FileName,
  IN OUT UINT8   **FileBuffer,
  OUT    UINTN   *FileSize
);

EFI_STATUS
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  );
