/** @file
  Header file for PCI device Lookup Table

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _PCI_DEV_LOOK_UP_TABLE_H_
#define _PCI_DEV_LOOK_UP_TABLE_H_


EFI_STATUS
GetVerdorIdShortName (
  IN UINT16  VendorId,
  OUT CHAR16 **ShortString
  );

EFI_STATUS
GetVerdorIdLongName (
  IN UINT16  VendorId,
  OUT CHAR16 **LongString
  );

#endif
