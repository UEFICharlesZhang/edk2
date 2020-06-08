/** @file
  XML parser

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "UiControls.h"

#define TAG 1
#define CONTENT 2
#define QUOTE   3

#define MAX_ATTRIBS 128

STATIC
BOOLEAN
IsSpace (
  INT32 c
  )
{
  return ((c<=' ' && (c==' ' || (c<=13 && c>=9) || c == '\0')));
}

STATIC
VOID
ParserContent (
  CHAR16 *Str,
  SAX_PARSER_CONTENT_CALLBACK ContentCb,
  VOID   *Data
  )
{
  while (IsSpace (*Str)) {
    Str++;
  }

  if (*Str == '\0') {
    return ;
  }

  if (ContentCb != NULL) {
    (*ContentCb)(Data, Str);
  }
}


STATIC
VOID
ParseElement (
  IN OUT CHAR16                *Str,
  IN SAX_PARSER_START_CALLBACK StartCb,
  IN SAX_PARSER_END_CALLBACK   EndCb,
  IN OUT VOID                  *Data
  )
{
  CHAR16       *AttrNames[MAX_ATTRIBS];
  CHAR16       *AttrValues[MAX_ATTRIBS];
  UINTN        AttrNum;
  CHAR16       *TagName;
  BOOLEAN      IsStart;
  BOOLEAN      IsEnd;
  CHAR16       QuoteChar;

  AttrNum = 0;
  TagName = NULL;
  IsStart = FALSE;
  IsEnd   = FALSE;

  //
  // trim white space after the '<'
  //
  while (IsSpace (*Str)) {
    Str++;
  }

  //
  // start tag or end tag
  //
  if (*Str == L'/') {
    Str++;
    IsEnd = TRUE;
  } else {
    IsStart = TRUE;
  }

  //
  // commonts or preprocessor
  //
  if (*Str == '\0' || *Str == L'?' || *Str == '!') {
    return ;
  }


  TagName = Str;
  while (!IsSpace (*Str) && *Str != '/') {
    Str++;
  }

  if (*Str != '\0') {
    if (*Str == '/') {
      IsEnd = TRUE;
    }
    *Str++ = L'\0';
  }

  while (!IsEnd && *Str != '\0' && AttrNum < (MAX_ATTRIBS - 1)) {

    while (IsSpace (*Str)) {
      Str++;
    }

    if (*Str == '\0') {
      break;
    }

    if (*Str == '/') {
      IsEnd = TRUE;
      break;
    }

    AttrNames[AttrNum] = Str++;
    AttrValues[AttrNum] = NULL;
    while (!IsSpace (*Str) && *Str != '=') {
      Str++;
    }

    if (*Str != '\0') {
      *Str++ = '\0';
    }

    //
    // beginning of the value
    //
    while (*Str != '\0' && *Str != '\"' && *Str != '\'') {
      Str++;
    }

    if (*Str == '\0') {
      break;
    }

    QuoteChar = *Str;
    Str++;
    AttrValues[AttrNum++] = Str;
    while (*Str != '\0' && *Str != QuoteChar) {
      if (*Str == '\\' && *(Str + 1) == QuoteChar) {
        Str += 2;
        continue;
      }
      Str++;
    }
    if (*Str != '\0') {
      *Str++ = '\0';
    }
  }
  AttrNames[AttrNum] = NULL;
  AttrValues[AttrNum++] = NULL;

  if (IsStart && StartCb) {
    (*StartCb)(Data, TagName, AttrNames, AttrValues);
  }
  if (IsEnd && EndCb) {
    (*EndCb)(Data, TagName);
  }
}

BOOLEAN
EFIAPI
SaxParser (
  CHAR16           *XMLBuffer,
  SAX_PARSER_START_CALLBACK   StartCb,
  SAX_PARSER_CONTENT_CALLBACK ContentCb,
  SAX_PARSER_END_CALLBACK     EndCb,
  VOID             *Data
  )
{
  CHAR16 *Str;
  CHAR16 *Mark;
  UINTN  State;
  CHAR16 QuoteChar;

  Str   = XMLBuffer;
  Mark  = Str;
  State = CONTENT;
  QuoteChar = 0;

  while (*Str != '\0') {

    if (*Str == '<' && State == CONTENT) {
      *Str++ = '\0';
      ParserContent (Mark, ContentCb, Data);
      Mark = Str;
      State = TAG;
    } else if (*Str == '\\' && State == QUOTE) {
      if (*(Str + 1) == QuoteChar) {
        Str += 2;
        continue;
      }
      Str++;
    } else if (*Str == '\"' || *Str == '\'') {
      if (State == TAG) {
        State = QUOTE;
        QuoteChar = *Str;
      } else if (State == QUOTE && *Str == QuoteChar) {
        State = TAG;
      }
      Str++;
    } else if (*Str == '>' && State == TAG) {
      *Str++ = '\0';
      ParseElement (Mark, StartCb, EndCb, Data);
      Mark = Str;
      State = CONTENT;
    } else {
      Str++;
    }
  }
  return TRUE;
}


