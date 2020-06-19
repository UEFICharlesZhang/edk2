
#include <FlashUpdate.h>

EFI_STATUS 
ReadFileInFS (
  IN      CHAR16  *FileName,
  OUT     UINT8   **FileData,
  IN OUT  UINTN   *BufferSize
  )
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *ptSFS;
  EFI_FILE_PROTOCOL               *ptRootFile;
  EFI_FILE_PROTOCOL               *ptFile;
  EFI_FILE_INFO                   *FileInfo;
  UINTN                           FileInfoSize;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                           NumberOfHandles;
  UINTN                           Index;
  UINT8                           *Buffer;

  Status     = EFI_SUCCESS;
  ptRootFile = NULL;
  ptFile     = NULL;
  FileInfo   = NULL;
  HandleBuffer = NULL;	

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  for (Index = 0; Index < NumberOfHandles; Index ++) {
    Status = gBS->HandleProtocol(
               HandleBuffer[Index], 
               &gEfiSimpleFileSystemProtocolGuid, 
               (VOID **)&ptSFS
               );
    if (!EFI_ERROR (Status)) {
      Status = ptSFS->OpenVolume(ptSFS, &ptRootFile);
      if (!EFI_ERROR (Status)) {
        Status = ptRootFile->Open(
                   ptRootFile, 
                   &ptFile, 
                   FileName, 
                   EFI_FILE_MODE_READ, 
                   0
                   );
        if (!EFI_ERROR (Status)) {
          break;
        } else {
          if (ptRootFile != NULL){
            ptRootFile->Close(ptRootFile);
            ptRootFile = NULL;						
          }
        }
      }
    }
  }

  if (HandleBuffer != NULL) {
    FreePool(HandleBuffer);
    HandleBuffer = NULL;
  }

  if (!EFI_ERROR (Status)) {
    FileInfo = NULL;
    FileInfoSize = 0;
    Status = ptFile->GetInfo (
                       ptFile,
                       &gEfiFileInfoGuid,
                       &FileInfoSize,
                       FileInfo
                       );
    if(Status == EFI_BUFFER_TOO_SMALL){
      FileInfo = AllocatePool(FileInfoSize);
      if(FileInfo == NULL){
        Status = EFI_OUT_OF_RESOURCES;
      } else {
        Status = ptFile->GetInfo(
                           ptFile,
                           &gEfiFileInfoGuid,
                           &FileInfoSize,
                           FileInfo
                           );
      }
    }
  }
  if (EFI_ERROR (Status)) {
    goto ProcExit;
  }

  if(FileInfo->Attribute & EFI_FILE_DIRECTORY){
    Status = EFI_INVALID_PARAMETER;
    goto ProcExit;
  }

  Buffer = AllocatePool((UINTN)FileInfo->FileSize);
  if (Buffer == NULL) {
    goto ProcExit;
  }

  Status = ptFile->Read(ptFile, &FileInfo->FileSize, Buffer);
  if (EFI_ERROR (Status)) {
    FreePool (Buffer);
    goto ProcExit;
  }
  *FileData = Buffer;
  *BufferSize = FileInfo->FileSize;

ProcExit:
  if (ptFile != NULL){
    ptFile->Close(ptFile);
  }  
  if (ptRootFile != NULL){
    ptRootFile->Close(ptRootFile);
  }
  if (FileInfo != NULL) {
    FreePool (FileInfo);
  }

  return Status;
}

EFI_STATUS
ReadFile (
  IN     CHAR16  *FileName,
  IN OUT UINT8   **FileBuffer,
  OUT    UINTN   *FileSize
)
{
  EFI_STATUS  Status;

  Status = ReadFileInFS (FileName, FileBuffer, FileSize);

  return Status;
}
