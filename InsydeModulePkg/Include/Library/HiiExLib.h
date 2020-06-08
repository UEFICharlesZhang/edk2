/** @file
  Public include file for the HII Extension Library

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;*****************************************************************************3*
*/

#ifndef __HII_EX_LIB_H__
#define __HII_EX_LIB_H__

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
  );

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
  );

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
  );

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
  IN UINT64                                MaxSize
  );

UINT8 *
EFIAPI
HiiCreateOneOfOptionOpCodeWithExtOp (
  IN VOID    *OpCodeHandle,
  IN UINT16  StringId,
  IN UINT8   Flags,
  IN UINT8   Type,
  IN UINT64  Value,
  IN VOID    *ExtendedOpCodeHandle
  );

UINT8 *
EFIAPI
HiiCreateImageOpCode (
  IN VOID           *OpCodeHandle,
  IN EFI_IMAGE_ID   ImageId
  );

UINT8 *
HiiCreateRefreshOpCode (
  IN VOID                    *OpCodeHandle,
  IN EFI_IFR_OP_HEADER       *PreviousOpCodeHeader,
  IN UINT8                   RefreshInterval
  );

UINT8 *
HiiCreateTrueOpCode (
  IN VOID                    *OpCodeHandle
  );

UINT8 *
HiiCreateFalseOpCode (
  IN VOID                    *OpCodeHandle
  );

UINT8 *
HiiCreateAndOpCode (
  IN VOID                    *OpCodeHandle
  );

UINT8 *
HiiCreateOrOpCode (
  IN VOID                    *OpCodeHandle
  );

UINT8 *
HiiCreateNotOpCode (
  IN VOID                    *OpCodeHandle
  );

UINT8 *
HiiCreateIdEqualOpCodeEx (
  IN VOID                    *OpCodeHandle,
  IN EFI_QUESTION_ID         QuestionId,
  IN UINT16                  Value
  );

UINT8 *
HiiCreateIdEqualListOpCodeEx (
  IN VOID                    *OpCodeHandle,
  IN EFI_QUESTION_ID         QuestionId,
  IN UINT8                   ListLength,
  IN UINT16                  *ValueList
  );

UINT8 *
HiiCreateSuppressIfOpCodeEx (
  IN VOID                    *OpCodeHandle,
  IN UINT8                   Scope
  );

UINT8 *
HiiCreateGrayOutIfOpCodeEx (
  IN VOID                    *OpCodeHandle,
  IN UINT8                   Scope
  );

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
  );

EFI_STATUS
HiiCleanLabelOpcode (
  IN EFI_HII_HANDLE                HiiHandle,
  IN EFI_GUID                      *FormSetGuid,
  IN UINT16                        FormId,
  IN UINT16                        StartLabelNumber,
  IN UINT16                        EndLabelNumber
  );

#endif
