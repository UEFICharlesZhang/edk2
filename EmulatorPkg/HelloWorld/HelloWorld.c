/** @file
  This is a test application that demonstrates how to use the sorting functions.

  Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Protocol/UserManager.h>
#include <Protocol/UserCredential2.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>//For CompareGuid
#include <Library/MemoryAllocationLib.h>//For allocatepool



EFI_USER_MANAGER_PROTOCOL *mUserManager           = NULL;
// {D71B80A1-C0CF-4A19-BAE6-C2509088FE3D}
EFI_GUID Pwd_Credential_Guid = { 0x78b9ec8b, 0xc000, 0x46c5, { 0xac, 0x93, 0x24, 0xa0, 0xc1, 0xbb, 0x0, 0xce }};

//
// Internal user profile entry.
//
typedef struct {
  UINTN   MaxProfileSize;
  UINTN   UserProfileSize;
  CHAR16  UserVarName[9];
  UINT8   *ProfileInfo;
} USER_PROFILE_ENTRY;

/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param  Argc             Argument count
  @param  Argv             The parsed arguments

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS Status;
  EFI_USER_PROFILE_HANDLE User  = NULL;
  UINTN HandleCount;
  EFI_HANDLE *HandleBuf;
  UINTN       Index;
  EFI_USER_CREDENTIAL2_PROTOCOL *UserCredential;
  EFI_USER_PROFILE_HANDLE UserToEnroll  = NULL;

  Status = gBS->LocateProtocol(
      &gEfiUserManagerProtocolGuid,
      NULL,
      (VOID **)&mUserManager);
  Print(L"LocateProtocol gEfiUserManagerProtocolGuid :%r!\n", Status);

  Status = mUserManager->Identify(mUserManager,&User);
  Print(L"Identify Status :%r!,Handle:0x%X\n", Status,User);
  
  Status = mUserManager->Current(mUserManager,&User);
  Print(L"Current Status :%r!,Handle:0x%X\n", Status,User);

  // 
  // Try to find all the user credential provider driver.
  //
  HandleCount = 0;
  HandleBuf = NULL;
  Status = gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiUserCredential2ProtocolGuid,
      NULL,
      &HandleCount,
      &HandleBuf);
  Print(L"LocateProtocol gEfiUserCredential2ProtocolGuid :%r!\n", Status);

  if (!EFI_ERROR(Status))
  {
    Print(L"gEfiUserCredential2ProtocolGuid HandleCount:0x%x!\n", HandleCount);

    for (Index = 0; Index < HandleCount; Index++)
    {
      Status = gBS->HandleProtocol(
          HandleBuf[Index],
          &gEfiUserCredential2ProtocolGuid,
          (VOID **)&UserCredential);
      if (EFI_ERROR(Status))
      {
        FreePool(HandleBuf);
        return Status;
      }
      //This is a pwd credential
      if (CompareGuid(&UserCredential->Identifier, &Pwd_Credential_Guid))
      {
        //Create a new user first
        Status = mUserManager->Create(mUserManager, &UserToEnroll);
        Print(L"Create Status :%r!,Handle:0x%X\n", Status, UserToEnroll);

        if (EFI_ERROR(Status))
        {
          Print(L"Create User Fail:%r\n", Status);
        }
        else
        {
          Status = UserCredential->Enroll(UserCredential, UserToEnroll);
          Print(L"UserCredential Enroll :%r!\n", Status);
        }
      }
    }
  }

  return EFI_SUCCESS;
}
