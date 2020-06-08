/** @file
   Implement H2O display engine event related functions.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "H2ODisplayEngineLibInternal.h"

/**
  Send H2O display engine change question event

  @param[in] PageId              The page identifier
  @param[in] QuestionId          The question identifier
  @param[in] HiiValue            A pointer to question HII value

  @retval EFI_SUCCESS            Send change question event successfully
  @retval EFI_INVALID_PARAMETER  HiiValue is NULL
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendChangeQNotify (
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_HII_VALUE                            *HiiValue
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_CHANGE_Q             ChangeQNotify;

  if (HiiValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&ChangeQNotify, sizeof (ChangeQNotify));
  ChangeQNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_CHANGE_Q);
  ChangeQNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGE_Q;
  ChangeQNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
  ChangeQNotify.PageId     = PageId;
  ChangeQNotify.QuestionId = QuestionId;
  CopyMem (&ChangeQNotify.HiiValue, HiiValue, sizeof (EFI_HII_VALUE));

  return FBProtocol->Notify (FBProtocol, &ChangeQNotify.Hdr);
}

/**
  Send H2O display engine changing question event

  @param[in] BodyHiiValue        The body HII value
  @param[in] ButtonHiiValue      Press button HII value

  @retval EFI_SUCCESS            Send changing question event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendChangingQNotify (
  IN EFI_HII_VALUE                            *BodyHiiValue,
  IN EFI_HII_VALUE                            *ButtonHiiValue
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_CHANGING_Q           ChangingQNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&ChangingQNotify, sizeof (ChangingQNotify));
  ChangingQNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_CHANGING_Q);
  ChangingQNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
  ChangingQNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_CHANGING_Q;
  //
  // FBDialog will store which question is current question, so PageId and QuestionId can be 0.
  //
  ChangingQNotify.PageId     = 0;
  ChangingQNotify.QuestionId = 0;

  if (BodyHiiValue != NULL) {
    CopyMem (&ChangingQNotify.BodyHiiValue, BodyHiiValue, sizeof (EFI_HII_VALUE));
  }
  if (ButtonHiiValue != NULL) {
    CopyMem (&ChangingQNotify.ButtonHiiValue, ButtonHiiValue, sizeof (EFI_HII_VALUE));
  }

  return FBProtocol->Notify (FBProtocol, &ChangingQNotify.Hdr);
}

/**
  Send H2O display engine shut down dialog event
**/
EFI_STATUS
SendShutDNotify (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_SHUT_D               ShutDNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&ShutDNotify, sizeof (ShutDNotify));
  ShutDNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_SHUT_D);
  ShutDNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_BROADCAST;
  ShutDNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_SHUT_D;

  return FBProtocol->Notify (FBProtocol, &ShutDNotify.Hdr);
}

/**
  Send H2O display engine select question event

  @param[in] PageId              The page identifier
  @param[in] QuestionId          The question identifier
  @param[in] IfrOpCode           A pointer to question IFR opcode

  @retval EFI_SUCCESS            Send select question event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendSelectQNotify (
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_SELECT_Q             SelectQNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&SelectQNotify, sizeof (H2O_DISPLAY_ENGINE_EVT_SELECT_Q));
  SelectQNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_SELECT_Q);
  SelectQNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_Q;
  SelectQNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
  SelectQNotify.PageId     = PageId;
  SelectQNotify.QuestionId = QuestionId;
  SelectQNotify.IfrOpCode  = IfrOpCode;

  return FBProtocol->Notify (FBProtocol, &SelectQNotify.Hdr);
}

/**
  Send H2O display engine open question event

  @param[in] PageId              The page identifier
  @param[in] QuestionId          The question identifier
  @param[in] IfrOpCode           A pointer to question IFR opcode

  @retval EFI_SUCCESS            Send open question event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendOpenQNotify (
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_OPEN_Q               OpenQNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&OpenQNotify, sizeof (OpenQNotify));
  OpenQNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_OPEN_Q);
  OpenQNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_OPEN_Q;
  OpenQNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
  OpenQNotify.PageId     = PageId;
  OpenQNotify.QuestionId = QuestionId;
  OpenQNotify.IfrOpCode  = IfrOpCode;

  return FBProtocol->Notify (FBProtocol, &OpenQNotify.Hdr);
}

/**
  Send H2O display engine select page event

  @param[in] PageId              The page identifier

  @retval EFI_SUCCESS            Send select page event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendSelectPNotify (
  IN H2O_PAGE_ID                              PageId
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_SELECT_P             SelectPNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&SelectPNotify, sizeof (SelectPNotify));
  SelectPNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_SELECT_P);
  SelectPNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_SELECT_P;
  SelectPNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;
  SelectPNotify.PageId     = PageId;

  return FBProtocol->Notify (FBProtocol, &SelectPNotify.Hdr);
}

/**
  Send H2O display engine load default event

  @retval EFI_SUCCESS            Send load default event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendDefaultNotify (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_DEFAULT              DefaultNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&DefaultNotify, sizeof (DefaultNotify));
  DefaultNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_DEFAULT);
  DefaultNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_DEFAULT;
  DefaultNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;

  return FBProtocol->Notify (FBProtocol, &DefaultNotify.Hdr);
}

/**
  Send H2O display engine submit and exit event

  @retval EFI_SUCCESS            Send submit and exit event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendSubmitExitNotify (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_SUBMIT_EXIT          SubmitExitNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&SubmitExitNotify, sizeof (SubmitExitNotify));
  SubmitExitNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_SUBMIT_EXIT);
  SubmitExitNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT_EXIT;
  SubmitExitNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;

  return FBProtocol->Notify (FBProtocol, &SubmitExitNotify.Hdr);
}

/**
  Send H2O display engine discard and exit event

  @retval EFI_SUCCESS            Send discard and exit event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendDiscardExitNotify (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_DISCARD_EXIT         DiscardExitNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&DiscardExitNotify, sizeof (DiscardExitNotify));
  DiscardExitNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_DISCARD_EXIT);
  DiscardExitNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD_EXIT;
  DiscardExitNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;

  return FBProtocol->Notify (FBProtocol, &DiscardExitNotify.Hdr);
}

/**
  Send H2O display engine submit event

  @retval EFI_SUCCESS            Send submit event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendSubmitNotify (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_SUBMIT               SubmitNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&SubmitNotify, sizeof (SubmitNotify));
  SubmitNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_SUBMIT);
  SubmitNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_SUBMIT;
  SubmitNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;

  return FBProtocol->Notify (FBProtocol, &SubmitNotify.Hdr);
}

/**
  Send H2O display engine discard event

  @retval EFI_SUCCESS            Send discard event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendDiscardNotify (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_DISCARD              DiscardNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&DiscardNotify, sizeof (DiscardNotify));
  DiscardNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_DISCARD);
  DiscardNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_DISCARD;
  DiscardNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;

  return FBProtocol->Notify (FBProtocol, &DiscardNotify.Hdr);
}

/**
  Send H2O display engine refresh event

  @retval EFI_SUCCESS            Send refresh event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendRefreshNotify (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_REFRESH              RefreshNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&RefreshNotify, sizeof (RefreshNotify));
  RefreshNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_REFRESH);
  RefreshNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_REFRESH;
  RefreshNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;

  return FBProtocol->Notify (FBProtocol, &RefreshNotify.Hdr);
}

/**
  Send H2O display engine show help event

  @retval EFI_SUCCESS            Send show help event successfully
  @retval Other                  Fail to locate procotol or return fail from calling notify event function
**/
EFI_STATUS
SendShowHelpNotify (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_DISPLAY_ENGINE_EVT_SHOW_HELP            ShowHelpNotify;

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (&ShowHelpNotify, sizeof (ShowHelpNotify));
  ShowHelpNotify.Hdr.Size   = sizeof (H2O_DISPLAY_ENGINE_EVT_SHOW_HELP);
  ShowHelpNotify.Hdr.Type   = H2O_DISPLAY_ENGINE_EVT_TYPE_SHOW_HELP;
  ShowHelpNotify.Hdr.Target = H2O_DISPLAY_ENGINE_EVT_TARGET_FORM_BROWSER;

  return FBProtocol->Notify (FBProtocol, &ShowHelpNotify.Hdr);
}

