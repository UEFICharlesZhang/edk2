/** @file

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

#ifndef _H2O_HII_H
#define _H2O_HII_H

///
/// GUIDed opcodes defined for H2O implementation.
///
#define H2O_IFR_EXT_GUID \
  { \
    0x38237648, 0x09cc, 0x47c4, {0x8b, 0x5f, 0xb0, 0x9f, 0x06, 0x89, 0x0d, 0xf7} \
  }

#pragma pack(1)

//
// For H2O_VALUE
//
#define H2O_VALUE_TYPE_PTR                   0x0000
#define H2O_VALUE_TYPE_BOOLEAN               0x0001
#define H2O_VALUE_TYPE_UINT32                0x0002
#define H2O_VALUE_TYPE_INT32                 0x0003
#define H2O_VALUE_TYPE_PERCENTAGE            0x0004

typedef union {
  BOOLEAN                                    Bool;
  INT32                                      Int;
  UINT32                                     Uint;
} H2O_VALUE_TYPE_VALUE;

typedef struct {
  UINT32                                     Type;
  H2O_VALUE_TYPE_VALUE                       Value;
} H2O_VALUE;

///
/// H2O implementation extension opcodes, new extension can be added here later.
///
#define H2O_IFR_EXT_HOTKEY            0x0000
#define H2O_IFR_EXT_LAYOUT            0x0001
#define H2O_IFR_EXT_STYLE             0x0002
#define H2O_IFR_EXT_HELPIMAGE         0x0003
#define H2O_IFR_EXT_PROPERTY          0x0004
#define H2O_IFR_EXT_STYLE_REF         0x0005
#define H2O_IFR_EXT_TEXT              0x0006
#define H2O_IFR_EXT_LINK              0x0007

//
// Link definitions
//
typedef struct _H2O_IFR_GUID_LINK {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_LINK
  ///
  UINT16              Function;
  UINT32              IncludeItemCount;
  UINT32              ExcludeItemCount;
  // EFI_GUID            ItemGuid[];
} H2O_IFR_GUID_LINK;

///
/// HotKey opcode.
///
typedef struct _H2O_IFR_GUID_HOTKEY {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_HOTKEY
  ///
  UINT16              Function;
  UINT8               HotKeyOffset;
} H2O_IFR_GUID_HOTKEY;

///
/// Layout opcode.
///
typedef struct _H2O_IFR_GUID_LAYOUT {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_LAYOUT
  ///
  UINT16              Function;
  UINT16              LayoutId;
} H2O_IFR_GUID_LAYOUT;

///
/// Style opcode.
///
typedef struct _H2O_IFR_GUID_STYLE {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_STYLE
  ///
  UINT16              Function;
  UINT8               Type;
  UINT8               ClassNameOffset;
  UINT8               PseudoClassOffset;
//CHAR8               Selector[1];
} H2O_IFR_GUID_STYLE;

///
/// Style type
///
#define H2O_IFR_STYLE_TYPE_LAYOUT      0x00
#define H2O_IFR_STYLE_TYPE_PANEL       0x01
#define H2O_IFR_STYLE_TYPE_FORM        0x02
#define H2O_IFR_STYLE_TYPE_FORMSET     0x03
#define H2O_IFR_STYLE_TYPE_ACTION      0x04
#define H2O_IFR_STYLE_TYPE_CHECKBOX    0x05
#define H2O_IFR_STYLE_TYPE_DATE        0x06
#define H2O_IFR_STYLE_TYPE_GOTO        0x07
#define H2O_IFR_STYLE_TYPE_ONEOF       0x08
#define H2O_IFR_STYLE_TYPE_NUMERIC     0x09
#define H2O_IFR_STYLE_TYPE_ORDEREDLIST 0x0a
#define H2O_IFR_STYLE_TYPE_PASSWORD    0x0b
#define H2O_IFR_STYLE_TYPE_STRING      0x0c
#define H2O_IFR_STYLE_TYPE_TIME        0x0d
#define H2O_IFR_STYLE_TYPE_TEXT        0x0e
#define H2O_IFR_STYLE_TYPE_SUBTITLE    0x0f
#define H2O_IFR_STYLE_TYPE_RESETBUTTON 0x10
#define H2O_IFR_STYLE_TYPE_SHEET       0xff

///
/// HelpImage opcode.
///
typedef struct _H2O_IFR_GUID_HELPIMAGE {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_HELPIMAGE
  ///
  UINT16              Function;
  EFI_IMAGE_ID        HelpImage;
} H2O_IFR_GUID_HELPIMAGE;

///
/// Property opcode.
///
typedef struct _H2O_IFR_GUID_PROPERTY {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_PROPERTY
  ///
  UINT16              Function;
  UINT8               IdentifierOffset;
  UINT8               ValueOffset;
//CHAR8               Property[1];
} H2O_IFR_GUID_PROPERTY;

///
/// Property value type
///
#define H2O_PROPERTY_VALUE_TYPE_ANIMATION    0x0001
#define H2O_PROPERTY_VALUE_TYPE_BOOLEAN      0x0002
#define H2O_PROPERTY_VALUE_TYPE_COLOR        0x0003
#define H2O_PROPERTY_VALUE_TYPE_COORD        0x0004
#define H2O_PROPERTY_VALUE_TYPE_ENUM         0x0005
#define H2O_PROPERTY_VALUE_TYPE_ENUMS        0x0006
#define H2O_PROPERTY_VALUE_TYPE_GUID         0x0007
#define H2O_PROPERTY_VALUE_TYPE_ID           0x0008
#define H2O_PROPERTY_VALUE_TYPE_IMAGE        0x0009
#define H2O_PROPERTY_VALUE_TYPE_KEYWORD      0x000A
#define H2O_PROPERTY_VALUE_TYPE_KEYWORDS     0x000B
#define H2O_PROPERTY_VALUE_TYPE_SIZE         0x000C
#define H2O_PROPERTY_VALUE_TYPE_STRING       0x000D

#define H2O_PROPERTY_VALUE_TYPE_OVERRIDE     0xFFFF

///
/// Style Ref opcode.
///
typedef struct _H2O_IFR_GUID_STYLE_REF {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_STYLE_REF
  ///
  UINT16              Function;
  UINT8               Type;
  UINT8               ClassNameOffset;
  UINT8               PseudoClassOffset;
//CHAR8               Selector[1];
} H2O_IFR_GUID_STYLE_REF;

///
/// Extended Text opcode.
///
typedef struct _H2O_IFR_GUID_TEXT {
  EFI_IFR_OP_HEADER   Header;
  ///
  /// H2O_IFR_EXT_GUID.
  ///
  EFI_GUID            Guid;
  ///
  /// H2O_IFR_EXT_TEXT
  ///
  UINT16              Function;
  EFI_STRING_ID       Text;
} H2O_IFR_GUID_TEXT;


typedef UINT16 H2O_PANEL_ID;
typedef UINT16 H2O_LAYOUT_ID;

//
// Hii Layout Package
//
#define H2O_HII_PACKAGE_LAYOUTS    0xe0

typedef struct _H2O_LAYOUT_PACKAGE_HDR {
  EFI_HII_PACKAGE_HEADER   Header;
  UINT32                   HdrSize;
  UINT32                   LayoutBlockOffset;
} H2O_LAYOUT_PACKAGE_HDR;

typedef struct _H2O_HII_LAYOUT_BLOCK {
  UINT8                    BlockType;
  UINT8                    BlockSize;
//UINT8                    BlockData[1];
} H2O_HII_LAYOUT_BLOCK;

#define H2O_HII_LIBT_LAYOUT_BEGIN  0x01
#define H2O_HII_LIBT_LAYOUT_END    0x02
#define H2O_HII_LIBT_PANEL_BEGIN   0x03
#define H2O_HII_LIBT_PANEL_END     0x04
#define H2O_HII_LIBT_STYLE_BEGIN   0x05
#define H2O_HII_LIBT_STYLE_END     0x06
#define H2O_HII_LIBT_PROPERTY      0x07
#define H2O_HII_LIBT_LAYOUT_DUP    0x08
#define H2O_HII_LIBT_PANEL_DUP     0x09
#define H2O_HII_LIBT_EXT2          0x10
#define H2O_HII_LIBT_EXT4          0x11
#define H2O_HII_LIBT_FORMAT        0x12
#define H2O_HII_LIBT_ENUM          0x13
#define H2O_HII_LIBT_KEYWORD       0x14

typedef struct _H2O_HII_LIBT_LAYOUT_BEGIN_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_LAYOUT_ID            LayoutId;
} H2O_HII_LIBT_LAYOUT_BEGIN_BLOCK;

typedef struct _H2O_HII_LIBT_LAYOUT_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_LAYOUT_END_BLOCK;

typedef UINT32 H2O_PANEL_TYPE;

#define H2O_PANEL_TYPE_MENU       0x0000
#define H2O_PANEL_TYPE_NAVIGATION 0x0001
#define H2O_PANEL_TYPE_FORM       0x0002
#define H2O_PANEL_TYPE_INFO       0x0003
#define H2O_PANEL_TYPE_TITLE      0x0004
#define H2O_PANEL_TYPE_HELP       0x0005
#define H2O_PANEL_TYPE_FORM2      0x0006
#define H2O_PANEL_TYPE_GUID       0xffff

typedef struct _H2O_HII_LIBT_PANEL_BEGIN_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_PANEL_ID             PanelId;
  H2O_PANEL_TYPE           PanelType;
  EFI_GUID                 PanelGuid;
} H2O_HII_LIBT_PANEL_BEGIN_BLOCK;

typedef struct _H2O_HII_LIBT_PANEL_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_PANEL_END_BLOCK;

typedef struct _H2O_HII_LIBT_STYLE_BEGIN_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT32                   Type;
  UINT8                    ClassNameOffset;
  UINT8                    PseudoClassOffset;
//CHAR8                    Selector[1];
} H2O_HII_LIBT_STYLE_BEGIN_BLOCK;

typedef struct _H2O_HII_LIBT_STYLE_END_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
} H2O_HII_LIBT_STYLE_END_BLOCK;

typedef struct _H2O_HII_LIBT_PROPERTY_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT32                   ValueType;
  H2O_VALUE                H2OValue;
  UINT8                    IdentifierOffset;
  UINT8                    ValueOffset;
//CHAR8                    Property[1];
} H2O_HII_LIBT_PROPERTY_BLOCK;

typedef struct _H2O_HII_LIBT_FORMAT_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT32                   StyleType;
  UINT32                   ValueType;
  UINT8                    IdentifierOffset;
} H2O_HII_LIBT_FORMAT_BLOCK;

typedef struct _H2O_HII_LIBT_ENUM_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT32                   ValueNumber;
  UINT8                    ValueOffset;
} H2O_HII_LIBT_ENUM_BLOCK;

typedef struct _H2O_HII_LIBT_KEYWORD_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT8                    ValueOffset;
} H2O_HII_LIBT_KEYWORD_BLOCK;

typedef struct _H2O_HII_LIBT_LAYOUT_DUP_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_LAYOUT_ID            LayoutId;
  H2O_LAYOUT_ID            OldLayoutId;
} H2O_HII_LIBT_LAYOUT_DUP_BLOCK;

typedef struct _H2O_HII_LIBT_PANEL_DUP_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  H2O_PANEL_ID             PanelId;
  H2O_LAYOUT_ID            OldLayoutId;
  H2O_PANEL_ID             OldPanelId;
} H2O_HII_LIBT_PANEL_DUP_BLOCK;

typedef struct _H2O_HII_LIBT_EXT2_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT8                    BlockType;
  UINT16                   BlockSize;
//UINT8                    BlockData[1];
} H2O_HII_LIBT_EXT2_BLOCK;

typedef struct _H2O_HII_LIBT_EXT4_BLOCK {
  H2O_HII_LAYOUT_BLOCK     Header;
  UINT8                    BlockType;
  UINT32                   BlockSize;
//UINT8                    BlockData[1];
} H2O_HII_LIBT_EXT4_BLOCK;


#pragma pack()

extern EFI_GUID gH2OIfrExtGuid;

#endif

