/** @file
  GwFormSetManager.h

  Interface of Gw Formset Manager Protocol.
**/

#ifndef __GW_FORMSET_MANAGER_H__
#define __GW_FORMSET_MANAGER_H__

//
// Device Manager Setup Protocol GUID
//
#define EFI_GW_FORMSET_MANAGER_PROTOCOL_GUID \
  { 0x65e4992f, 0xd77c, 0x494d, { 0x9a, 0xd1, 0x68, 0x77, 0x5b, 0xb9, 0x1a, 0xa1 } }
extern EFI_GUID gEfiGwFormsetManagerProtocolGuid;

#define EFI_GW_SETUP_ENTERING_GUID \
  { 0x71202EEE, 0x5F53, 0x40d9, {0xAB, 0x3D, 0x9E, 0x0C, 0x26, 0xD9, 0x66, 0x57}}
extern EFI_GUID gEfiGwSetupEnteringGuid;

typedef struct _EFI_GW_FORMSET_MANAGER_PROTOCOL EFI_GW_FORMSET_MANAGER_PROTOCOL;

typedef struct {
  UINTN                           Signature;
  LIST_ENTRY                   Link;
  EFI_GUID                       Guid;
  EFI_HII_HANDLE             HiiHandle;
  EFI_STRING_ID              FormSetTitle;
  EFI_FORM_ID                 FirstFormId;
} GW_BROWSER_FORMSET;

#define GW_FORM_BROWSER_FORMSET_SIGNATURE  SIGNATURE_32 ('B', 'F', 'B', 'L')
#define GW_FORM_BROWSER_FORMSET_FROM_LINK(a)  CR (a, GW_BROWSER_FORMSET, Link, GW_FORM_BROWSER_FORMSET_SIGNATURE)

typedef EFI_STATUS
(*INSERT_GW_FORMSET) (
  IN EFI_GW_FORMSET_MANAGER_PROTOCOL    *This,
  IN EFI_GUID    *FormsetGuid
);

typedef EFI_STATUS
(*REMOVE_GW_FORMSET) (
  IN EFI_GW_FORMSET_MANAGER_PROTOCOL    *This,
  IN EFI_GUID    *FormsetGuid
);

typedef EFI_STATUS
( *RUN_GW_FORMSET) (
  IN EFI_GW_FORMSET_MANAGER_PROTOCOL    *This,
  IN EFI_GUID    *FormsetGuid
);

typedef EFI_STATUS
( *SETUP_CHECK_PASSWORD) (
  IN CHAR16 *Title,
  OUT CHAR16 *Password
);

typedef BOOLEAN
( *SETUP_CHECK_FORMSET) (
  IN EFI_GUID    *FormsetGuid
);

typedef struct _EFI_GW_FORMSET_MANAGER_PROTOCOL {
  LIST_ENTRY    GwFormSetList;
  INSERT_GW_FORMSET    Insert;
  REMOVE_GW_FORMSET    Remove;
  RUN_GW_FORMSET       Run;
  SETUP_CHECK_PASSWORD  CheckPassword;
  SETUP_CHECK_FORMSET   CheckFormset;
} EFI_GW_FORMSET_MANAGER_PROTOCOL;

#endif