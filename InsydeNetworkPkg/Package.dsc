## @file
# Network Package Description file
#
#******************************************************************************
#* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************
#

[Defines]

[PcdsFeatureFlag]
  
[LibraryClasses]

[PcdsDynamicExDefault]
  #
  # PXE procedure cancel hot key definition
  # Key struct definition {KeyCode, ScanCode, [Next hot key], 0x00, 0x00[END]}
  #
  gInsydeTokenSpaceGuid.PcdPxeCancelHotKeyDefinition|{ \
    0x00, 0x17, \ # ESC
    0x00, 0x00  \ # List end
    }
  gInsydeTokenSpaceGuid.PcdPxeCancelHotKeyString|"[ESC]"

[Components.$(DXE_ARCH)]
!if gInsydeTokenSpaceGuid.PcdH2ONetworkSupported
  InsydeNetworkPkg/Drivers/NetworkLockerDxe/NetworkLockerDxe.inf
  InsydeNetworkPkg/Drivers/DhcpDummyDxe/DhcpDummyDxe.inf
  InsydeNetworkPkg/Drivers/PxeDummyDxe/PxeDummyDxe.inf
  InsydeNetworkPkg/Drivers/DpcDxe/DpcDxe.inf
  InsydeNetworkPkg/Drivers/MnpDxe/MnpDxe.inf
  InsydeNetworkPkg/Drivers/ArpDxe/ArpDxe.inf
  InsydeNetworkPkg/Drivers/SnpDxe/SnpDxe.inf
  InsydeNetworkPkg/Drivers/Ip4Dxe/Ip4Dxe.inf
  InsydeNetworkPkg/Drivers/Ip4ConfigDxe/Ip4ConfigDxe.inf
  InsydeNetworkPkg/Drivers/Udp4Dxe/Udp4Dxe.inf
  InsydeNetworkPkg/Drivers/Dhcp4Dxe/Dhcp4Dxe.inf
  InsydeNetworkPkg/Drivers/Mtftp4Dxe/Mtftp4Dxe.inf
!if gInsydeTokenSpaceGuid.PcdH2ONetworkIpv6Supported
  InsydeNetworkPkg/Drivers/Ip6Dxe/Ip6Dxe.inf
  InsydeNetworkPkg/Drivers/Udp6Dxe/Udp6Dxe.inf
  InsydeNetworkPkg/Drivers/Dhcp6Dxe/Dhcp6Dxe.inf
  InsydeNetworkPkg/Drivers/Mtftp6Dxe/Mtftp6Dxe.inf
!endif
  InsydeNetworkPkg/Drivers/UefiPxeBcDxe/UefiPxeBcDxe.inf

!if gInsydeTokenSpaceGuid.PcdH2ONetworkIscsiSupported
  InsydeNetworkPkg/Drivers/TcpDxe/TcpDxe.inf
  InsydeNetworkPkg/Drivers/IScsiDxe/IScsiDxe.inf
!endif
!endif
