

#ifndef  __SM3_HASH_H__
#define  __SM3_HASH_H__
//---------------------------------------------------------------

// #include <uefi.h>

EFI_STATUS 
EFIAPI 
CalcSM3Hash (
  CONST UINT8 *Data, 
  UINTN DataLength, 
  UINT8 *Hash
  );



//---------------------------------------------------------------
#endif