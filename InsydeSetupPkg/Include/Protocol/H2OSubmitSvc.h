/** @file
  H2O Submit Service Protocol Header

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

#ifndef _H2O_SUBMIT_SVC_PROTOCOL_H_
#define _H2O_SUBMIT_SVC_PROTOCOL_H_

#include <Protocol/FormBrowserEx.h>

#define H2O_SUBMIT_SVC_PROTOCOL_GUID \
{ \
  0x61DECF9E, 0x25D1, 0x425C, 0x9A, 0x83, 0x54, 0x8E, 0x34, 0x38, 0x5D, 0x87 \
} 

typedef struct _H2O_SUBMIT_SVC_PROTOCOL H2O_SUBMIT_SVC_PROTOCOL;

/**
 Execute the Submit service function.
 
 @param[in]  This                A pointer to the H2O_SUBMIT_SVC_PROTOCOL instance.  
 @param[out] Request             A pointer to the request from the Submit service functions.
                                 Related definition can refer to "Browser actions" of FormBrowserEx.h.
 @param[out] ShowSubmitDialog    A pointer to the value if needing to show the original submit dialog.
 
 @retval EFI_SUCCESS             Execute the Submit service functions successfully.
 @retval EFI_Status              Otherwise.                  
*/
typedef
EFI_STATUS
(EFIAPI *H2O_EXECUTE_SVC) (
  IN    H2O_SUBMIT_SVC_PROTOCOL             *This,
  OUT   UINT32                              *Request,
  OUT   BOOLEAN                             *ShowSubmitDialog
  );


struct _H2O_SUBMIT_SVC_PROTOCOL {
  H2O_EXECUTE_SVC                 ExecuteSvc;  
};

extern GUID gH2OSubmitSvcProtocolGuid;

#endif
