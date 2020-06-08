## @file
# H2O Console Redirection package project build description file.
#
#******************************************************************************
#* Copyright (c) 2013 - 2014, Insyde Software Corporation. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************
#

[Defines]

[LibraryClasses]
!if gInsydeTokenSpaceGuid.PcdH2OConsoleRedirectionSupported == 1
 CrVfrConfigLib|InsydeCrPkg/Library/CrVfrConfigLib/CrVfrConfigLib.inf
 CrBdsLib|InsydeCrPkg/Library/CrBdsLib/CrBdsLib.inf
 CrConfigDefaultLib|InsydeCrPkg/Library/CrConfigDefaultLib/CrConfigDefaultLib.inf
!endif 
[LibraryClasses.common.PEIM]

[LibraryClasses.common.DXE_DRIVER]

[LibraryClasses.common.DXE_SMM_DRIVER]

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

[LibraryClasses.common.UEFI_DRIVER]

[LibraryClasses.common.UEFI_APPLICATION]

[PcdsFeatureFlag]
  gInsydeCrTokenSpaceGuid.PcdH2OCRPciSerialSupported|TRUE
  gInsydeCrTokenSpaceGuid.PcdH2OCRUsbSerialSupported|FALSE
  gInsydeCrTokenSpaceGuid.PcdH2OCRTelnetSupported|FALSE
  gInsydeCrTokenSpaceGuid.PcdH2OCRSrvManagerSupported|FALSE
  gInsydeCrTokenSpaceGuid.PcdH2OAdvanceConsoleRedirectionSupported|FALSE

!if gInsydeCrTokenSpaceGuid.PcdH2OAdvanceConsoleRedirectionSupported == TRUE
  gInsydeCrTokenSpaceGuid.PcdH2OCRUsbSerialSupported|TRUE
  gInsydeCrTokenSpaceGuid.PcdH2OCRSrvManagerSupported|TRUE
  gInsydeCrTokenSpaceGuid.PcdH2OCRTelnetSupported|TRUE
!endif
  
[PcdsFixedAtBuild]

[PcdsFixedAtBuild.IPF]

[PcdsPatchableInModule]

[Components.$(PEI_ARCH)] 

[Components.$(DXE_ARCH)]
!if gInsydeTokenSpaceGuid.PcdH2OConsoleRedirectionSupported == 1 
  InsydeCrPkg/CrHookDxe/CrHookDxe.inf
  InsydeCrPkg/CrPolicyDxe/CrPolicyDxe.inf
  InsydeCrPkg/IsaSerialDxe/IsaSerialDxe.inf
!if gInsydeCrTokenSpaceGuid.PcdH2OCRTelnetSupported == 1  
  InsydeCrPkg/AdvanceTerminalDxe/AdvanceTerminalDxe.inf
  InsydeCrPkg/SolDxe/SolDxe.inf
!else
  InsydeCrPkg/TerminalDxe/TerminalDxe.inf
!endif 
!if gInsydeCrTokenSpaceGuid.PcdH2OCRPciSerialSupported == 1 
  InsydeCrPkg/PciSerialDxe/PciSerialDxe.inf
!endif
!if gInsydeCrTokenSpaceGuid.PcdH2OCRSrvManagerSupported == 1
  InsydeCrPkg/CrSrvManagerDxe/CrSrvManagerDxe.inf  
  InsydeCrPkg/CrBiosFlashDxe/CrBiosFlashDxe.inf
  InsydeCrPkg/CrFileTransferDxe/CrFileTransferDxe.inf
  InsydeCrPkg/FileSelectUIDxe/FileSelectUIDxe.inf
!endif
!if gInsydeCrTokenSpaceGuid.PcdH2OCRUsbSerialSupported == 1   
  InsydeCrPkg/UsbSerialDxe/UsbSerialDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2ODdtSupported != 1
  InsydeCrPkg/CrDdtCableDxe/CrDdtCableDxe.inf
!endif
  InsydeCrPkg/CrPl2303Dxe/CrPl2303Dxe.inf
!endif
!if gInsydeTokenSpaceGuid.PcdH2ONetworkIscsiSupported != 1 && gInsydeCrTokenSpaceGuid.PcdH2OCRTelnetSupported == 1
  InsydeNetworkPkg/Drivers/TcpDxe/TcpDxe.inf
!endif  
!endif


