/** @file
 HII Extension Library implementation that uses DXE protocols and services.

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

#include "InternalHiiExLib.h"

extern GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 mHiiDefaultTypeToWidth[];

UINTN
EFIAPI
InternalHiiOpCodeHandlePosition (
  IN VOID  *OpCodeHandle
  );

UINT8 *
EFIAPI
InternalHiiCreateOpCodeExtended (
  IN VOID   *OpCodeHandle,
  IN VOID   *OpCodeTemplate,
  IN UINT8  OpCode,
  IN UINTN  OpCodeSize,
  IN UINTN  ExtensionSize,
  IN UINT8  Scope
  );

UINT8 *
EFIAPI
InternalHiiAppendOpCodes (
  IN VOID  *OpCodeHandle,
  IN VOID  *RawOpCodeHandle
  );

UINT8 *
EFIAPI
InternalHiiOpCodeHandleBuffer (
  IN VOID  *OpCodeHandle
  );

/**
 Create H2O_IFR_GUID_TEXT opcode.

 @param[in] OpCodeHandle              Handle to the buffer of opcodes.
 @param[in] PreviousOpCodeHeader      Pointer to the opcode header of previous opcode
 @param[in] TextTwo                   String ID for Text Two

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateGuidTextOpCode (
  IN VOID                                  *OpCodeHandle,
  IN EFI_IFR_OP_HEADER                     *PreviousOpCodeHeader,
  IN EFI_STRING_ID                         TextTwo
  )
{
  H2O_IFR_GUID_TEXT                        *GuidText;

  GuidText = (H2O_IFR_GUID_TEXT *) HiiCreateGuidOpCode (
                                     OpCodeHandle,
                                     &gH2OIfrExtGuid,
                                     NULL,
                                     sizeof (H2O_IFR_GUID_TEXT)
                                     );
  if (GuidText == NULL) {
    return NULL;
  }

  GuidText->Function = H2O_IFR_EXT_TEXT;
  GuidText->Text     = TextTwo;

  if (PreviousOpCodeHeader->Scope == 0) {
    PreviousOpCodeHeader->Scope = 1;
    HiiCreateEndOpCode (OpCodeHandle);
  }

  return (UINT8 *) GuidText;
}

UINT8 *
HiiCreateRefreshOpCode (
  IN VOID                                  *OpCodeHandle,
  IN EFI_IFR_OP_HEADER                     *PreviousOpCodeHeader,
  IN UINT8                                 RefreshInterval
  )
{
  EFI_IFR_REFRESH                          OpCode;
  UINT8                                    *OpCodePtr;

  OpCode.RefreshInterval = RefreshInterval;

  OpCodePtr = (UINT8 *)InternalHiiCreateOpCodeExtended (
                         OpCodeHandle,
                         &OpCode,
                         EFI_IFR_REFRESH_OP,
                         sizeof (OpCode),
                         0,
                         0
                         );
  if (OpCodePtr == NULL) {
    return NULL;
  }

  if (PreviousOpCodeHeader->Scope == 0) {
    PreviousOpCodeHeader->Scope = 1;
    HiiCreateEndOpCode (OpCodeHandle);
  }

  return (UINT8 *) OpCodePtr;

}

/**
 Create EFI_IFR_ACTION_OP opcode with text two.
 If string ID of text two is zero, it will create EFI_IFR_ACTION_OP opcode without text two.

 @param[in] OpCodeHandle    Handle to the buffer of opcodes.
 @param[in] QuestionId      Question ID
 @param[in] Prompt          String ID for Prompt
 @param[in] Help            String ID for Help
 @param[in] TextTwo         String ID for Text Two
 @param[in] QuestionFlags   Flags in Question Header
 @param[in] QuestionConfig  String ID for configuration

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
EFIAPI
HiiCreateActionOpCodeEx (
  IN VOID                                  *OpCodeHandle,
  IN EFI_QUESTION_ID                       QuestionId,
  IN EFI_STRING_ID                         Prompt,
  IN EFI_STRING_ID                         Help,
  IN EFI_STRING_ID                         TextTwo,
  IN UINT8                                 QuestionFlags,
  IN EFI_STRING_ID                         QuestionConfig
  )
{
  EFI_IFR_ACTION                           *Action;

  Action = (EFI_IFR_ACTION *) HiiCreateActionOpCode (
                                OpCodeHandle,
                                QuestionId,
                                Prompt,
                                Help,
                                QuestionFlags,
                                QuestionConfig
                                );
  if (Action == NULL || TextTwo == 0) {
    return (UINT8 *) Action;
  }

  HiiCreateGuidTextOpCode (OpCodeHandle, &Action->Header, TextTwo);

  return (UINT8 *) Action;
}

/**
 Create EFI_IFR_ACTION_OP opcode with text two and scope.
 If string ID of text two is zero, it will create EFI_IFR_ACTION_OP opcode without text two.

 @param[in] OpCodeHandle    Handle to the buffer of opcodes.
 @param[in] QuestionId      Question ID
 @param[in] Prompt          String ID for Prompt
 @param[in] Help            String ID for Help
 @param[in] TextTwo         String ID for Text Two
 @param[in] QuestionFlags   Flags in Question Header
 @param[in] QuestionConfig  String ID for configuration
 @param[in] Scope               Scope

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
EFIAPI
HiiCreateActionOpCodeExWithScope (
  IN VOID                                  *OpCodeHandle,
  IN EFI_QUESTION_ID                       QuestionId,
  IN EFI_STRING_ID                         Prompt,
  IN EFI_STRING_ID                         Help,
  IN EFI_STRING_ID                         TextTwo,
  IN UINT8                                 QuestionFlags,
  IN EFI_STRING_ID                         QuestionConfig,
  IN UINT8                                 Scope
  )
{
  EFI_IFR_ACTION                           *Action;

  Action = (EFI_IFR_ACTION *) HiiCreateActionOpCode (
                                OpCodeHandle,
                                QuestionId,
                                Prompt,
                                Help,
                                QuestionFlags,
                                QuestionConfig
                                );
  if (Action == NULL || TextTwo == 0) {
    return (UINT8 *) Action;
  }

  Action->Header.Scope = Scope;
  HiiCreateGuidTextOpCode (OpCodeHandle, &Action->Header, TextTwo);

  return (UINT8 *) Action;
}

/**
 Create EFI_IFR_ACTION_OP opcode with text two and refresh opcode.
 If string ID of text two is zero, it will create EFI_IFR_ACTION_OP opcode without text two.
 If RefreshInterval is zero, it will create EFI_IFR_ACTION_OP opcode without refresh.

 @param[in] OpCodeHandle    Handle to the buffer of opcodes.
 @param[in] QuestionId      Question ID
 @param[in] Prompt          String ID for Prompt
 @param[in] Help            String ID for Help
 @param[in] TextTwo         String ID for Text Two
 @param[in] QuestionFlags   Flags in Question Header
 @param[in] QuestionConfig  String ID for configuration
 @param[in] RefreshInterval  RefreshInterval

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
EFIAPI
HiiCreateActionOpCodeExWithRefresh (
  IN VOID                                  *OpCodeHandle,
  IN EFI_QUESTION_ID                       QuestionId,
  IN EFI_STRING_ID                         Prompt,
  IN EFI_STRING_ID                         Help,
  IN EFI_STRING_ID                         TextTwo,
  IN UINT8                                 QuestionFlags,
  IN EFI_STRING_ID                         QuestionConfig,
  IN UINT8                                 RefreshInterval
  )
{
  EFI_IFR_ACTION                           *Action;
  UINT8                                    Scope;

  if (RefreshInterval == 0) {
    Scope = 0;
  } else {
    Scope = 1;
  }

  Action = (EFI_IFR_ACTION *) HiiCreateActionOpCodeExWithScope (
                                OpCodeHandle,
                                QuestionId,
                                Prompt,
                                Help,
                                TextTwo,
                                QuestionFlags,
                                QuestionConfig,
                                Scope
                                );
  if (Action == NULL || Scope == 0) {
    return (UINT8 *) Action;
  }

  HiiCreateRefreshOpCode (OpCodeHandle, &Action->Header, RefreshInterval);
  HiiCreateEndOpCode (OpCodeHandle);

  return (UINT8 *) Action;
}

/**
 Create EFI_IFR_PASSWORD_OP opcode with text two.
 If string ID of text two is zero, it will create EFI_IFR_PASSWORD_OP opcode without text two.

 @param[in] OpCodeHandle          Handle to the buffer of opcodes.
 @param[in] QuestionId            Question ID
 @param[in] VarStoreId            Storage ID
 @param[in] VarOffset             Offset in Storage
 @param[in] Prompt                String ID for Prompt
 @param[in] Help                  String ID for Help
 @param[in] TextTwo               String ID for Text Two
 @param[in] QuestionFlags         Flags in Question Header
 @param[in] Minimum               Numeric minimum value
 @param[in] Maximum               Numeric maximum value

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
EFIAPI
HiiCreatePasswordOpCodeEx (
  IN VOID                                  *OpCodeHandle,
  IN EFI_QUESTION_ID                       QuestionId,
  IN EFI_VARSTORE_ID                       VarStoreId,
  IN UINT16                                VarOffset,
  IN EFI_STRING_ID                         Prompt,
  IN EFI_STRING_ID                         Help,
  IN EFI_STRING_ID                         TextTwo,
  IN UINT8                                 QuestionFlags,
  IN UINT16                                MinSize,
  IN UINT16                                MaxSize
  )
{
  EFI_IFR_PASSWORD                         Password;
  UINT8                                    *OpCodePtr;

  ZeroMem (&Password, sizeof (Password));
  Password.Header.OpCode                   = EFI_IFR_PASSWORD_OP;
  Password.Header.Length                   = sizeof (Password);
  Password.Question.QuestionId             = QuestionId;
  Password.Question.VarStoreId             = VarStoreId;
  Password.Question.VarStoreInfo.VarOffset = VarOffset;
  Password.Question.Header.Prompt          = Prompt;
  Password.Question.Header.Help            = Help;
  Password.Question.Flags                  = QuestionFlags;
  Password.MinSize                         = MinSize;
  Password.MaxSize                         = MaxSize;

  OpCodePtr = HiiCreateRawOpCodes (OpCodeHandle, (UINT8 *) &Password, sizeof (Password));
  if (OpCodePtr == NULL || TextTwo == 0) {
    return OpCodePtr;
  }

  HiiCreateGuidTextOpCode (OpCodeHandle, &((EFI_IFR_PASSWORD *) OpCodePtr)->Header, TextTwo);

  return OpCodePtr;
}

/**
 Create EFI_IFR_ONE_OF_OPTION_OP opcode with ext op.

 If OpCodeHandle is NULL, then ASSERT().
 If Type is invalid, then ASSERT().
 If Flags is invalid, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.
 @param[in] StringId      StringId for the option
 @param[in] Flags         Flags for the option
 @param[in] Type          Type for the option
 @param[in] Value         Value for the option
 @param[in] ExtendedOpCodeHandle   Handle for a buffer of Extended opcodes.

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
EFIAPI
HiiCreateOneOfOptionOpCodeWithExtOp (
  IN VOID    *OpCodeHandle,
  IN UINT16  StringId,
  IN UINT8   Flags,
  IN UINT8   Type,
  IN UINT64  Value,
  IN VOID    *ExtendedOpCodeHandle
  )
{
  EFI_IFR_ONE_OF_OPTION    OpCode;
  UINTN                    Position;

  ASSERT (ExtendedOpCodeHandle != NULL);
  ASSERT (Type < EFI_IFR_TYPE_OTHER);

  ZeroMem (&OpCode, sizeof (OpCode));
  OpCode.Option = StringId;
  OpCode.Flags  = (UINT8) (Flags & (EFI_IFR_OPTION_DEFAULT));
  OpCode.Type   = Type;
  CopyMem (&OpCode.Value, &Value, mHiiDefaultTypeToWidth[Type]);

  Position = InternalHiiOpCodeHandlePosition (OpCodeHandle);
  InternalHiiCreateOpCodeExtended (OpCodeHandle, &OpCode, EFI_IFR_ONE_OF_OPTION_OP, sizeof (OpCode), 0, 1);

  InternalHiiAppendOpCodes (OpCodeHandle, ExtendedOpCodeHandle);
  HiiCreateEndOpCode (OpCodeHandle);
  return InternalHiiOpCodeHandleBuffer (OpCodeHandle) + Position;
}

/**
 Create EFI_IFR_IMAGE_OP opcode.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.
 @param[in] ImageId      Image ID for Image

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
EFIAPI
HiiCreateImageOpCode (
  IN VOID           *OpCodeHandle,
  IN EFI_IMAGE_ID   ImageId
  )
{
  EFI_IFR_IMAGE     OpCode;

  ZeroMem (&OpCode, sizeof (OpCode));
  OpCode.Id = ImageId;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_IMAGE_OP,
           sizeof (OpCode),
           0,
           0
           );
}

/**
 Create EFI_IFR_TRUE opcode.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateTrueOpCode (
  IN VOID                    *OpCodeHandle
  )
{
  EFI_IFR_TRUE               OpCode;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_TRUE_OP,
           sizeof (OpCode),
           0,
           0
           );
}

/**
 Create EFI_IFR_FALSE opcode.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateFalseOpCode (
  IN VOID                    *OpCodeHandle
  )
{
  EFI_IFR_FALSE              OpCode;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_FALSE_OP,
           sizeof (OpCode),
           0,
           0
           );
}

/**
 Create EFI_IFR_AND opcode.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateAndOpCode (
  IN VOID                    *OpCodeHandle
  )
{
  EFI_IFR_AND                OpCode;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_AND_OP,
           sizeof (OpCode),
           0,
           0
           );
}

/**
 Create EFI_IFR_OR opcode.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateOrOpCode (
  IN VOID                    *OpCodeHandle
  )
{
  EFI_IFR_OR                 OpCode;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_OR_OP,
           sizeof (OpCode),
           0,
           0
           );
}

/**
 Create EFI_IFR_NOT opcode.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateNotOpCode (
  IN VOID                    *OpCodeHandle
  )
{
  EFI_IFR_NOT                OpCode;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_NOT_OP,
           sizeof (OpCode),
           0,
           0
           );
}

/**
 Create EFI_IFR_EQ_ID_VAL opcode.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.
 @param[in] QuestionId  Question ID
 @param[in]  Value         Value for the equal

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateIdEqualOpCodeEx (
  IN VOID                    *OpCodeHandle,
  IN EFI_QUESTION_ID         QuestionId,
  IN UINT16                  Value
  )
{
  EFI_IFR_EQ_ID_VAL          OpCode;

  OpCode.QuestionId = QuestionId;
  OpCode.Value = Value;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_EQ_ID_VAL_OP,
           sizeof (OpCode),
           0,
           0
           );
}

/**
 Create EFI_IFR_EQ_ID_VAL_LIST opcode.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.
 @param[in] QuestionId  Question ID
 @param[in]  ListLength  Numer of value
 @param[in]  ValueList    List of value

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateIdEqualListOpCodeEx (
  IN VOID                    *OpCodeHandle,
  IN EFI_QUESTION_ID         QuestionId,
  IN UINT8                   ListLength,
  IN UINT16                  *ValueList
  )
{
  EFI_IFR_EQ_ID_VAL_LIST     *OpCode;
  UINTN                      OpCodeSize;
  UINT8                      *OpCodePtr;
  UINT8                      Index;

  OpCodeSize = sizeof (EFI_IFR_EQ_ID_VAL_LIST) + sizeof (UINT16) * (ListLength - 1);
  OpCode = AllocatePool (OpCodeSize);
  if (OpCode == NULL) {
    return NULL;
  }
  
  OpCode->QuestionId = QuestionId;
  OpCode->ListLength = ListLength;
  for (Index = 0; Index < ListLength; Index++) {
    OpCode->ValueList[Index] = ValueList[Index];  
  }

  OpCodePtr = InternalHiiCreateOpCodeExtended (
             OpCodeHandle,
             OpCode,
             EFI_IFR_EQ_ID_VAL_LIST_OP,
             OpCodeSize,
             0,
             0
             );
  FreePool (OpCode);

  return OpCodePtr;
}

/**
 Create EFI_IFR_SUPPRESS_IF opcode with scope.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.
 @param[in] Scope              Scope

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateSuppressIfOpCodeEx (
  IN VOID                    *OpCodeHandle,
  IN UINT8                   Scope
  )
{
  EFI_IFR_SUPPRESS_IF        OpCode;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_SUPPRESS_IF_OP,
           sizeof (OpCode),
           0,
           Scope
           );
}

/**
 Create EFI_IFR_GRAY_OUT_IF opcode with scope.

 If OpCodeHandle is NULL, then ASSERT().

 @param[in] OpCodeHandle  Handle to the buffer of opcodes.
 @param[in] Scope              Scope

 @retval NULL   There is not enough space left in Buffer to add the opcode.
 @retval Other  A pointer to the created opcode.
**/
UINT8 *
HiiCreateGrayOutIfOpCodeEx (
  IN VOID                    *OpCodeHandle,
  IN UINT8                   Scope
  )
{
  EFI_IFR_GRAY_OUT_IF        OpCode;

  return InternalHiiCreateOpCodeExtended (
           OpCodeHandle,
           &OpCode,
           EFI_IFR_GRAY_OUT_IF_OP,
           sizeof (OpCode),
           0,
           Scope
           );
}

/**
 Get formset information (title string ID, tilte help string ID and root form ID) from Hii database

 @param[in]      HiiHandle           Hii handle
 @param[in, out] FormSetGuid         Pointer to formset GUID
 @param[out]     FormSetTitleStrId   Pointer to formset title string ID
 @param[out]     FormSetHelpStrId    Pointer to formset help string ID
 @param[out]     RootFormId          Pointer to root Form ID

 @retval EFI_SUCCESS            Successfully get formset information
 @retval EFI_INVALID_PARAMETER  Input pointer is NULL or form ID is zero
 @retval EFI_OUT_OF_RESOURCES   Fail to allocate pool
 @retval EFI_NOT_FOUND          Fail to export package list or get matched formset
 @retval Other                  Fail to locate Hii database protocol or export package list
**/
EFI_STATUS
GetFormSetInfo (
  IN     EFI_HII_HANDLE             HiiHandle,
  IN OUT EFI_GUID                   *FormSetGuid,
  OUT    EFI_STRING_ID              *FormSetTitleStrId,
  OUT    EFI_STRING_ID              *FormSetHelpStrId,
  OUT    EFI_FORM_ID                *RootFormId
  )
{
  EFI_HII_DATABASE_PROTOCOL     *HiiDatabase;
  EFI_STATUS                   Status;
  UINTN                        BufferSize;
  EFI_HII_PACKAGE_LIST_HEADER  *HiiPackageList;
  UINT8                        *Package;
  UINT8                        *OpCodeData;
  UINT32                       Offset;
  UINT32                       Offset2;
  UINT32                       PackageListLength;
  EFI_HII_PACKAGE_HEADER       PackageHeader;
  BOOLEAN                      Found;
  EFI_IFR_FORM_SET             *FormSet;
  EFI_GUID                     ZeroGuid;


  if (HiiHandle == NULL || FormSetGuid == NULL || FormSetTitleStrId == NULL || FormSetHelpStrId == NULL || RootFormId == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get HII PackageList
  //
  BufferSize     = 0;
  HiiPackageList = NULL;
  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return EFI_NOT_FOUND;
  }

  HiiPackageList = AllocatePool (BufferSize);
  if (HiiPackageList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get Form package from this HII package List
  //
  Offset  = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  Offset2 = 0;
  PackageListLength = ReadUnaligned32 (&HiiPackageList->PackageLength);
  if (PackageListLength > BufferSize) {
    PackageListLength = (UINT32)BufferSize;
  }

  ZeroMem (&ZeroGuid, sizeof(EFI_GUID));
  FormSet = NULL;
  Found   = FALSE;
  while (Offset < PackageListLength) {
    Package = ((UINT8 *) HiiPackageList) + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
      //
      // Search FormSet Opcode in this Form Package
      //
      Offset2 = sizeof (EFI_HII_PACKAGE_HEADER);
      while (Offset2 < PackageHeader.Length) {
        OpCodeData = Package + Offset2;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
          FormSet = (EFI_IFR_FORM_SET *) OpCodeData;

          if (CompareGuid (FormSetGuid, &ZeroGuid) || CompareGuid (FormSetGuid, (EFI_GUID *)(VOID *)&FormSet->Guid)) {
            CopyGuid (FormSetGuid, (EFI_GUID *)(VOID *)&FormSet->Guid);
            *FormSetTitleStrId = FormSet->FormSetTitle;
            *FormSetHelpStrId  = FormSet->Help;
          } else {
            FormSet = NULL;
          }
        }

        if (FormSet != NULL && (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_OP)) {
          *RootFormId = ((EFI_IFR_FORM *) OpCodeData)->FormId;
          Found = TRUE;
          break;
        }

        //
        // Go to next opcode
        //
        Offset2 += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
        if (((EFI_IFR_OP_HEADER *) OpCodeData)->Length < sizeof (EFI_IFR_OP_HEADER)) {
          break;
        }
      }
    }

    if (Found) {
      break;
    }

    //
    // Go to next package
    //
    Offset += PackageHeader.Length;
    if (PackageHeader.Length == 0) {
      break;
    }
  }

  FreePool (HiiPackageList);

  return Found ? EFI_SUCCESS : EFI_NOT_FOUND;
}

/**
 Copy string from the string ID of source Hii handle to the string ID of destination Hii handle.
 If the string ID of destination Hii handle, it will create a new string ID.

 @param[in] SrcHiiHandle        Source Hii handle
 @param[in] SrcStringId         String ID of source Hii handle
 @param[in] DstHiiHandle        Destination Hii handle
 @param[in] DstStringId         Pointer to string ID of destination Hii handle

 @retval EFI_SUCCESS            Successfully copy string
 @retval EFI_INVALID_PARAMETER  Input pointer is NULL or source string ID is zero
 @retval EFI_NOT_FOUND          Get PlatformLangCodes variable fail
 @retval EFI_OUT_OF_RESOURCES   Allocate pool fail
 @retval Other                  Fail to get variable
**/
EFI_STATUS
CopyStringByStringId (
  IN     EFI_HII_HANDLE         SrcHiiHandle,
  IN     EFI_STRING_ID          SrcStringId,
  IN     EFI_HII_HANDLE         DstHiiHandle,
  IN OUT EFI_STRING_ID          *DstStringId
  )
{
  EFI_STATUS                    Status;
  UINTN                         PlatformLangCodesSize;
  UINT8                         *PlatformLangCodes;
  UINT8                         *PlatformLang;
  UINTN                         Index;
  CHAR16                        *SrcString;
  EFI_STRING_ID                 NewStringId;

  if (SrcHiiHandle == NULL || SrcStringId == 0 || DstHiiHandle == NULL || DstStringId == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PlatformLangCodes     = NULL;
  PlatformLangCodesSize = 0;
  Status = gRT->GetVariable (L"PlatformLangCodes", &gEfiGlobalVariableGuid, NULL, &PlatformLangCodesSize, (VOID *) PlatformLangCodes);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return EFI_NOT_FOUND;
  }
  PlatformLangCodes = AllocateZeroPool (PlatformLangCodesSize);
  if (PlatformLangCodes == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  Status = gRT->GetVariable (L"PlatformLangCodes", &gEfiGlobalVariableGuid, NULL, &PlatformLangCodesSize, (VOID *) PlatformLangCodes);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Index = 0;
  while (Index < PlatformLangCodesSize) {
    PlatformLang = &PlatformLangCodes[Index];

    while (PlatformLangCodes[Index] != ';' && PlatformLangCodes[Index] != 0) {
      Index++;
    }
    PlatformLangCodes[Index++] = 0;

    SrcString = HiiGetString (SrcHiiHandle, SrcStringId, PlatformLang);
    if (SrcString != NULL) {
      NewStringId = HiiSetString (DstHiiHandle, *DstStringId, SrcString, PlatformLang);
      if (NewStringId != 0) {
        *DstStringId = NewStringId;
      }
      FreePool (SrcString);
    }
  }

  FreePool (PlatformLangCodes);
  return EFI_SUCCESS;
}

/**
 Insert EFI_IFR_REF3_OP opcode in target formset

 @param[in] HiiHandle           Hii handle
 @param[in] FormSetGuid         Formset GUID
 @param[in] FormId              Form ID
 @param[in] LabelNumber         Label number
 @param[in] RefHiiHandle        Hii handle of EFI_IFR_REF3_OP opcode
 @param[in] FormSetGuid         Formset GUID of EFI_IFR_REF3_OP opcode
 @param[in] FormId              Form ID of EFI_IFR_REF3_OP opcode
 @param[in] RefQuestionId       Question ID of EFI_IFR_REF3_OP opcode

 @retval EFI_SUCCESS            Successfully Insert opcodes
 @retval EFI_INVALID_PARAMETER  Input pointer is NULL or form ID is zero
 @retval EFI_OUT_OF_RESOURCES   Fail to create opcode
 @retval Other                  Fail to update form data into Hii database or get formset info
**/
EFI_STATUS
HiiInsertRef3Opcode (
  IN EFI_HII_HANDLE                 HiiHandle,
  IN EFI_GUID                       *FormSetGuid,
  IN UINT16                         FormId,
  IN UINT16                         LabelNumber,
  IN EFI_HII_HANDLE                 RefHiiHandle,
  IN EFI_GUID                       *RefFormSetGuid,
  IN EFI_FORM_ID                    RefFormId,
  IN EFI_QUESTION_ID                RefQuestionId
  )
{
  EFI_STATUS                        Status;
  EFI_GUID                          GoToFormSetGuid;
  EFI_STRING_ID                     GoToFormSetTitleStrId;
  EFI_STRING_ID                     GoToFormSetHelpStrId;
  EFI_FORM_ID                       GoToFormId;
  EFI_STRING_ID                     NewTitleStrId;
  EFI_STRING_ID                     NewHelpStrId;
  VOID                              *StartOpCodeHandle;
  EFI_IFR_GUID_LABEL                *StartLabel;

  if (HiiHandle == NULL || RefHiiHandle == NULL || RefQuestionId == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Prepare all elements for creating EFI_IFR_REF3_OP opcode
  //
  if (RefFormSetGuid == NULL) {
    ZeroMem (&GoToFormSetGuid, sizeof(EFI_GUID));
  } else {
    CopyMem (&GoToFormSetGuid, RefFormSetGuid, sizeof(EFI_GUID));
  }

  Status = GetFormSetInfo (
             RefHiiHandle,
             &GoToFormSetGuid,
             &GoToFormSetTitleStrId,
             &GoToFormSetHelpStrId,
             &GoToFormId
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (RefFormId != 0) {
    GoToFormId = RefFormId;
  }

  NewTitleStrId = 0;
  NewHelpStrId  = 0;
  CopyStringByStringId (RefHiiHandle, GoToFormSetTitleStrId, HiiHandle, &NewTitleStrId);
  CopyStringByStringId (RefHiiHandle, GoToFormSetHelpStrId , HiiHandle, &NewHelpStrId);

  //
  // Insert EFI_IFR_REF3_OP opcode in target formset
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  if (StartOpCodeHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  if (StartLabel == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LabelNumber;

  HiiCreateGotoExOpCode (
    StartOpCodeHandle,
    GoToFormId,
    NewTitleStrId,
    NewHelpStrId,
    0,
    RefQuestionId,
    0,
    &GoToFormSetGuid,
    0
    );

  Status = HiiUpdateForm (
             HiiHandle,
             FormSetGuid,
             FormId,
             StartOpCodeHandle,
             NULL
             );

  if (StartOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle (StartOpCodeHandle);
  }

  return Status;
}

/**
 Clean all created opcodes between start and end labels.

 @param[in] HiiHandle           Hii handle
 @param[in] FormSetGuid         Formset GUID
 @param[in] FormId              Form ID
 @param[in] StartLabelNumber    Start label number
 @param[in] EndLabelNumber      End label number

 @retval EFI_SUCCESS            Successfully clean opcodes
 @retval EFI_INVALID_PARAMETER  Input pointer is NULL or form ID is zero
 @retval EFI_OUT_OF_RESOURCES   Fail to create opcode
 @retval Other                  Fail to update form data into Hii database
**/
EFI_STATUS
HiiCleanLabelOpcode (
  IN EFI_HII_HANDLE                 HiiHandle,
  IN EFI_GUID                       *FormSetGuid,
  IN UINT16                         FormId,
  IN UINT16                         StartLabelNumber,
  IN UINT16                         EndLabelNumber
  )
{
  VOID                              *StartOpCodeHandle;
  VOID                              *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL                *StartLabel;
  EFI_IFR_GUID_LABEL                *EndLabel;
  EFI_STATUS                        Status;

  if (HiiHandle == NULL || FormSetGuid == NULL || FormId == 0) {
    return EFI_INVALID_PARAMETER;
  }

  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  if (StartOpCodeHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  if (EndOpCodeHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  if (StartLabel == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = StartLabelNumber;

  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  if (EndLabel == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  EndLabel->ExtendOpCode   = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number         = EndLabelNumber;

  Status = HiiUpdateForm (
             HiiHandle,
             FormSetGuid,
             FormId,
             StartOpCodeHandle,
             EndOpCodeHandle
             );

  if (StartOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle (StartOpCodeHandle);
  }
  if (EndOpCodeHandle != NULL) {
    HiiFreeOpCodeHandle (EndOpCodeHandle);
  }

  return Status;
}
