/** @file
  Entry point and initial functions for H2O local Metro display engine driver

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#ifndef _H2O_DIALOGS_H_
#define _H2O_DIALOGS_H_

extern BOOLEAN                  mTitleVisible;
extern H2O_FORM_BROWSER_D       *mFbDialog;

extern CHAR16 *mTimeDialogChilds;
extern CHAR16 *mDateDialogChilds;
extern CHAR16 *mStringDialogChilds;
extern CHAR16 *mOneOfDialogChilds;
extern CHAR16 *mConfirmPageDialogChilds;
extern CHAR16 *mPasswordDialogChilds;
extern CHAR16 *mOrderedListDialogChilds;
extern CHAR16 *mOneOfDialogWithoutSendFormChilds;
extern CHAR16 *mHelpDialogChilds;
extern CHAR16 *mNumericDialogWithoutSendFormChilds;

UINT32
GetButtonWidthByStr (
  VOID
  );


LRESULT
H2ONumericDialogProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );

LRESULT
H2ONumericDialogWithoutSendFormProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );

VOID
FreeDialogEvent (
  IN H2O_FORM_BROWSER_D         **DialogEvt
  );

LRESULT
WINAPI
DialogCallback (
  HWND Wnd,
  UINT Msg,
  WPARAM WParam,
  LPARAM LParam
  );

LRESULT
H2OOrderedListInputProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );

LRESULT
H2OOneOfInputProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );

LRESULT
H2OPasswordDialogProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );


INTN
H2ODateTimeDialogProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );

INTN
H2OPopupDialogProc (
  HWND         Wnd,
  UINT         Msg,
  WPARAM       WParam,
  LPARAM       lParam
  );

LRESULT
H2OHelpDialogProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );

LRESULT
H2OStringDialogProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );

LRESULT
H2OConfirmPageDialogProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );


LRESULT
H2OConfirmPasswordProc (
  HWND                          Wnd,
  UINT                          Msg,
  WPARAM                        WParam,
  LPARAM                        LParam
  );

#endif
