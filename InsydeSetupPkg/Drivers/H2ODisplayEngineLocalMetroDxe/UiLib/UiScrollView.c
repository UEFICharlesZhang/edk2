/** @file
  UI Common Controls

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "UiControls.h"

STATIC UI_SCROLL_VIEW_CLASS     *mScrollViewClass = NULL;
#define CURRENT_CLASS           mScrollViewClass

VOID
UiScrollViewSetPosition (
  UI_CONTROL                    *Control,
  CONST RECT                    *Pos
  )
{
  UI_VERTICAL_LAYOUT *This;
  UI_CONTROL   *Item;
  RECT   Rc;
  RECT   ItemRc;
  SIZE   Size;
  UINT32       Top;
  UINT32       Expand;
  SIZE   Available;
  SIZE   ItemSize;
  BOOLEAN EnableScroll;
  INT32   ScrollBarWidth;

  Size.cx  = 0;
  Size.cy  = 0;
  Expand   = 0;

  This = (UI_VERTICAL_LAYOUT *) Control;

  ShowSetPositionDbgInfo (Control, Pos);
  GetWindowRect(Control->Wnd, &Rc);
  MapWindowPoints(HWND_DESKTOP, GetParent(Control->Wnd), (LPPOINT) &Rc, 2);
  if (!EqualRect (&Rc, Pos)) {
    ShowSetPositionDbgInfo (Control, Pos);
    MoveWindow (Control->Wnd, Pos->left, Pos->top, Pos->right - Pos->left, Pos->bottom - Pos->top, TRUE);
    CONTROL_CLASS_INVALIDATE (This);
    Control->NeedUpdateChildPos = TRUE;
  }

  if (Control->ItemCount == 0) {
    return ;
  }

  if ((GetWindowLongPtr (Control->Items[0]->Wnd, GWL_STYLE) & WS_VISIBLE) == 0) {
    return ;
  }

  if (!Control->NeedUpdateChildPos) {
    return ;
  }
  Control->NeedUpdateChildPos = FALSE;


  CopyRect (&Rc, Pos);
  OffsetRect (&Rc, -Rc.left, -Rc.top);

  Rc.left   += Control->Padding.left;
  Rc.top    += Control->Padding.top;
  Rc.right  -= Control->Padding.right;
  Rc.bottom -= Control->Padding.bottom;

  if (Rc.left > Rc.right) {
    Rc.left = Rc.right;
  }
  if (Rc.top > Rc.bottom) {
    Rc.top = Rc.bottom;
  }

  Available.cx = Rc.right - Rc.left;
  Available.cy = Rc.bottom - Rc.top;

  Item = Control->Items[0];


  Top = Rc.top;
  EnableScroll = FALSE;

  Size = CONTROL_CLASS (Item)->EstimateSize (Item, Available);

  if (Size.cy == 0) {
    ItemSize.cy = Available.cy;
  } else if (Size.cy == MATCH_PARENT) {
    ItemSize.cy = Rc.bottom - Top;
  } else {
    ItemSize.cy = Size.cy;
  }

  if ((Size.cx == 0) || (Size.cx == MATCH_PARENT)) {
    ItemSize.cx = Available.cx;
  } else {
    ItemSize.cx = Size.cx;
  }

  if (Control->VScrollBar != NULL) {
    if ((UINT32)ItemSize.cy > (UINT32) Available.cy) {
      EnableScroll = TRUE;
    }
  }

  if (Control->VScrollBar == NULL || !EnableScroll) {
    ItemRc.left   = Rc.left;
    ItemRc.top    = Top;
    ItemRc.right  = ItemRc.left + ItemSize.cx;
    ItemRc.bottom = ItemRc.top  + ItemSize.cy;
    mSetPositionLevel++;
    CONTROL_CLASS (Item)->SetPosition (Item, &ItemRc);
    mSetPositionLevel--;
    UiSetAttribute (Control->VScrollBar, L"visible", L"false");

    return ;
  }

  UiSetAttribute (Control->VScrollBar, L"visible", L"true");
  ScrollBarWidth = Control->VScrollBar->Control.FixedSize.cx;
  Available.cx -= ScrollBarWidth;
  Size = CONTROL_CLASS (Item)->EstimateSize (Item, Available);
  if (Size.cy == 0) {
    ItemSize.cy = Available.cy;
  } else if (Size.cy == MATCH_PARENT) {
    ItemSize.cy = Rc.bottom - Top;
  } else {
    ItemSize.cy = Size.cy;
  }

  if ((Size.cx == 0) || (Size.cx == MATCH_PARENT)) {
    ItemSize.cx = Available.cx;
  } else {
    ItemSize.cx = Size.cx;
  }

  Top -= Control->ScrollPos.y;
  Control->VScrollBar->Range = ItemSize.cy - Available.cy;

  //
  // child item
  //
  ItemRc.left   = Rc.left;
  ItemRc.top    = Top;
  ItemRc.right  = ItemRc.left + ItemSize.cx;
  ItemRc.bottom = ItemRc.top  + ItemSize.cy;
  mSetPositionLevel++;
  CONTROL_CLASS (Item)->SetPosition (Item, &ItemRc);
  mSetPositionLevel--;

  //
  // scrollbar
  //
  ItemRc.left   = Rc.right - ScrollBarWidth;
  ItemRc.top    = 0;
  ItemRc.right  = ItemRc.left + ScrollBarWidth;
  ItemRc.bottom = Rc.bottom;
  CONTROL_CLASS (Control->VScrollBar)->SetPosition ((UI_CONTROL *)Control->VScrollBar, &ItemRc);
}


BOOLEAN
EFIAPI
UiScrollViewSetAttribute (
  UI_CONTROL  *Control,
  CHAR16      *Name,
  CHAR16      *Value
  )
{

  UI_SCROLL_VIEW *This;

  This = (UI_SCROLL_VIEW *) Control;

  if (StrCmp (Name, L"scrollbarThumbVertical") == 0) {
    if (Control->VScrollBar != NULL) {
      UiSetAttribute (Control->VScrollBar, L"thumb", Value);
    }
  } else if (StrCmp (Name, L"scrollbarTrackVertical") == 0) {
    if (Control->VScrollBar != NULL) {
      UiSetAttribute (Control->VScrollBar, L"track", Value);
    }
  } else {
    return PARENT_CLASS_SET_ATTRIBUTE (CURRENT_CLASS, Control, Name, Value);
  }

  CONTROL_CLASS_INVALIDATE (This);

  return TRUE;
}



LRESULT
EFIAPI
UiScrollViewKeyDown (
  UI_SCROLL_VIEW                *This,
  WPARAM                        WParam,
  LPARAM                        LParam
  )
{
  UI_CONTROL                    *Control;

  Control = (UI_CONTROL *) This;

  switch (WParam) {

  case VK_DOWN:
    SCROLLBAR_CLASS(Control->VScrollBar)->SetScrollPos (Control->VScrollBar, Control->ScrollPos.y + 19);
    break;

  case VK_UP:
    SCROLLBAR_CLASS(Control->VScrollBar)->SetScrollPos (Control->VScrollBar, Control->ScrollPos.y - 20);
    break;

  default:
    break;
  }

  return 0;
}

LRESULT
EFIAPI
UiScrollViewProc (
  HWND   Wnd,
  UINT32 Msg,
  WPARAM WParam,
  LPARAM LParam
  )
{
  UI_SCROLL_VIEW                *This;
  UI_CONTROL                    *Control;

  This = (UI_SCROLL_VIEW *) GetWindowLongPtr (Wnd, 0);
  if (This == NULL && Msg != WM_CREATE && Msg != WM_NCCALCSIZE) {
    ASSERT (FALSE);
    return 0;
  }
  Control = (UI_CONTROL *)This;

  switch (Msg) {

  case WM_CREATE:
    This = (UI_SCROLL_VIEW *) AllocateZeroPool (sizeof (*CURRENT_CLASS));
    if (This != NULL) {
      CONTROL_CLASS (This) = (UI_CONTROL_CLASS *) GetClassLongPtr (Wnd, 0);
      SetWindowLongPtr (Wnd, 0, (INTN)This);
      SendMessage (Wnd, UI_NOTIFY_CREATE, WParam, LParam);
    }
    break;

  case UI_NOTIFY_CREATE:
    PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam, LParam);
    SetWindowLongPtr (
      Control->Wnd,
      GWL_EXSTYLE, GetWindowLongPtr (Control->Wnd, GWL_EXSTYLE) & ~WS_EX_NOACTIVATE
      );
    UiSetAttribute (Control, L"tabstop", L"true");
    break;

  case WM_NCHITTEST:
    if (Control->VScrollBar != NULL) {
      return HTCLIENT;
    }
    return HTTRANSPARENT;

  case WM_KEYDOWN:
    UiScrollViewKeyDown (This, WParam, LParam);
    break;

  default:
    return PARENT_CLASS_WNDPROC (CURRENT_CLASS, Wnd, Msg, WParam, LParam);
  }

  return 0;
}

UI_SCROLL_VIEW_CLASS  *
EFIAPI
GetScrollViewClass (
  VOID
  )
{
  if (CURRENT_CLASS != NULL) {
    return CURRENT_CLASS;
  }

  InitUiClass ((UI_CONTROL_CLASS **)&CURRENT_CLASS, sizeof (*CURRENT_CLASS), L"ScrollView", (UI_CONTROL_CLASS *)GetControlClass());
  if (CURRENT_CLASS == NULL) {
    return NULL;
  }
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->WndProc      = UiScrollViewProc;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetAttribute = UiScrollViewSetAttribute;
  ((UI_CONTROL_CLASS *)CURRENT_CLASS)->SetPosition  = UiScrollViewSetPosition;

  return CURRENT_CLASS;
}

