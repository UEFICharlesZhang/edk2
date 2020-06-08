/** @file
FAT  Directory Manage functions

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
//
// This file contains 'Framework Code' and is licensed as such
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.
//
/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the Software 
License Agreement which accompanies this distribution.


Module Name:

  DirectoryManage.c

Abstract:

  Functions for performing directory entry io

Revision History

--*/

#include "FatPeim.h"


/**
  Set the File's cluster info in its directory entry.
  
  @param[in]  File        The open's file entry.
  
  @retval    none.
  
**/
STATIC
VOID
FatSetDirEntCluster (
  IN PEI_FAT_FILE    *File
  )
{
  UINTN Cluster;
  Cluster = File->StartingCluster;

  File->FileDir.FileClusterHigh  = (UINT16) (Cluster >> 16);
  File->FileDir.FileCluster      = (UINT16) Cluster;
}

/**
  Get the FAT entry of the volume, which is identified with the Index.

  @param[in]  PrivateData     Pointer to the PEF_FAT_PRIVATE_DATA structure
  @param[in]  Volume           FAT file system volume.
  @param[in]  Index             The index of the FAT entry of the volume.
  
  @retval    none.
  
**/
STATIC
VOID *
FatLoadFatEntry (
  IN     PEI_FAT_PRIVATE_DATA   *PrivateData,
  IN     PEI_FAT_VOLUME         *Volume,
  IN     UINTN                  Index
  )
{
  UINTN       PosInByte;
  EFI_STATUS  Status;
  
  if (Index > (Volume->MaxCluster + 1)) {
    Volume->FatEntryBuffer = (UINT32) - 1;
    return &Volume->FatEntryBuffer;
  }
  
  //
  // Compute buffer position needed
  //
  switch (Volume->FatType) {
  case Fat12:
    PosInByte            = FAT_POS_FAT12 (Index);
    Volume->FatEntrySize = FAT_POS_FAT12 (1);
    break;

  case Fat16:
    PosInByte            = FAT_POS_FAT16 (Index);
    Volume->FatEntrySize = FAT_POS_FAT16 (1);
    break;

  default:
    PosInByte            = FAT_POS_FAT32 (Index);
    Volume->FatEntrySize = FAT_POS_FAT32 (1);
  }
  
  //
  // Set the position and read the buffer
  //
  Volume->FatEntryPos = Volume->FatPos + PosInByte;

  Status = FatReadDisk (
             PrivateData, 
             Volume->BlockDeviceNo, 
             Volume->FatEntryPos, 
             Volume->FatEntrySize, 
             &Volume->FatEntryBuffer);

  if (EFI_ERROR (Status)) {
    Volume->FatEntryBuffer = (UINT32) -1;
  }

  return &Volume->FatEntryBuffer;
}

/**
  Get the FAT entry value of the volume, which is identified with the Index.

  @param[in]  PrivateData     Pointer to the PEF_FAT_PRIVATE_DATA structure
  @param[in]  Volume           FAT file system volume.
  @param[in]  Index             The index of the FAT entry of the volume.
  
  @retval    The value of the FAT entry..
  
**/
STATIC
UINTN
FatGetFatEntry (
  IN  PEI_FAT_PRIVATE_DATA       *PrivateData,
  IN  PEI_FAT_VOLUME             *Volume,
  IN  UINTN                      Index
  )
{
  VOID    *Value;
  UINT8   *E12;
  UINT16  *E16;
  UINT32  *E32;
  UINTN   Accum;

  Value = FatLoadFatEntry (PrivateData, Volume, Index);

  if (Index > (Volume->MaxCluster + 1)) {
    return (UINTN) -1;
  }

  switch (Volume->FatType) {
  case Fat12:
    E12   = Value;
    Accum = E12[0] | (E12[1] << 8);
    Accum = FAT_ODD_CLUSTER_FAT12 (Index) ? (Accum >> 4) : (Accum & FAT_CLUSTER_MASK_FAT12);
    Accum = Accum | ((Accum >= FAT_CLUSTER_SPECIAL_FAT12) ? FAT_CLUSTER_SPECIAL_EXT : 0);
    break;

  case Fat16:
    E16   = Value;
    Accum = *E16;
    Accum = Accum | ((Accum >= FAT_CLUSTER_SPECIAL_FAT16) ? FAT_CLUSTER_SPECIAL_EXT : 0);
    break;

  default:
    E32   = Value;
    Accum = *E32 & FAT_CLUSTER_MASK_FAT32;
    Accum = Accum | ((Accum >= FAT_CLUSTER_SPECIAL_FAT32) ? FAT_CLUSTER_SPECIAL_EXT : 0);
  }

  return Accum;
}

/**
   Set the FAT entry value of the volume, which is identified with the Index.

  @param[in]  PrivateData     Pointer to the PEF_FAT_PRIVATE_DATA structure
  @param[in]  Volume           FAT file system volume.
  @param[in]  Index             The index of the FAT entry of the volume.
  @param[in]  Value             The new value of the FAT entry.
  
  @retval       EFI_SUCCESS   Set the new FAT entry value sucessfully
  @retval       EFI_VOLUME_CORRUPTED   The FAT type of the volume is error.
  @retval       other   An error occurred when operation the FAT entries.
  
**/
STATIC
EFI_STATUS
FatSetFatEntry (
  IN PEI_FAT_PRIVATE_DATA        *PrivateData,
  IN PEI_FAT_VOLUME              *Volume,
  IN UINTN                       Index,
  IN UINTN                       Value
  )
{
  VOID        *Pos;
  UINT8       *E12;
  UINT16      *E16;
  UINT32      *E32;
  UINTN       Accum;
  EFI_STATUS  Status;

  if (Index < FAT_MIN_CLUSTER) {
    return EFI_VOLUME_CORRUPTED;
  }

  //
  // Make sure the entry is in memory
  //
  Pos = FatLoadFatEntry (PrivateData, Volume, Index);

  //
  // Update the value
  //
  switch (Volume->FatType) {
  case Fat12:
    E12   = Pos;
    Accum = E12[0] | (E12[1] << 8);
    Value = Value & FAT_CLUSTER_MASK_FAT12;

    if (FAT_ODD_CLUSTER_FAT12 (Index)) {
      Accum = (Value << 4) | (Accum & 0xF);
    } else {
      Accum = Value | (Accum & FAT_CLUSTER_UNMASK_FAT12);
    }

    E12[0]  = (UINT8) (Accum & 0xFF);
    E12[1]  = (UINT8) (Accum >> 8);
    break;

  case Fat16:
    E16   = Pos;
    *E16  = (UINT16) Value;
    break;

  default:
    E32   = Pos;
    *E32  = (*E32 & FAT_CLUSTER_UNMASK_FAT32) | (UINT32) (Value & FAT_CLUSTER_MASK_FAT32);
  }

  Status = FatWriteDisk (
             PrivateData,
             Volume->BlockDeviceNo,
             Volume->FatEntryPos,
             Volume->FatEntrySize,
             &Volume->FatEntryBuffer
             );

  return Status;
}

/**
  Allocate a free cluster and return the cluster index.

  @param[in]  PrivateData     Pointer to the PEF_FAT_PRIVATE_DATA structure
  @param[in]  Volume           FAT file system volume.
  
  @retval       The index of the free cluster
  
**/
STATIC
UINTN
FatAllocateCluster (
  IN PEI_FAT_PRIVATE_DATA        *PrivateData,
  IN PEI_FAT_VOLUME              *Volume
  )
{
  UINTN         Cluster;
  UINTN         Index;
  static UINTN  LastIndex = FAT_MIN_CLUSTER;

  Cluster = (UINTN) FAT_CLUSTER_LAST;

  //
  // Start looking at FatFreePos for the next unallocated cluster
  //
  if (LastIndex > Volume->MaxCluster) {
    LastIndex = FAT_MIN_CLUSTER;
  }

  for (Index = LastIndex + 1; Index < Volume->MaxCluster + 1; Index++) {

    Cluster = FatGetFatEntry (PrivateData, Volume, Index);
    if (Cluster == FAT_CLUSTER_FREE) {
      LastIndex = Index;
      return Index;
    }
  }
  
  return Cluster;
}

/**
  Count the number of clusters given a size

  @param[in]  Volume           FAT file system volume.
  @param[in]  Size               The size in bytes.
  
  @retval       The number of the clusters.
  
**/
STATIC
UINTN
FatSizeToClusters (
  IN PEI_FAT_VOLUME   *Volume,
  IN UINTN            Size
  )
{
  UINTN Clusters;

  Clusters = Size / Volume->ClusterSize;
  if ((Size % Volume->ClusterSize) > 0) {
    Clusters += 1;
  }

  return Clusters;
}

/**
  This function reads the next vaild entry in the parent directory.
  If no more items were found, the function returns EFI_NOT_FOUND.
  
  @param[in]  PrivateData       Global memory map for accessing global variables
  @param[in][out]  ParentDir         The parent directory

  @retval EFI_SUCCESS           The next sub file is successfully retrieved.
  @retval EFI_INVALID_PARAMETER The ParentDir is not a directory.
  @retval EFI_NOT_FOUND         No more sub file found
  @retval EFI_DEVICE_ERROR      Something error while accessing media.

**/
EFI_STATUS
FatFindVaildDirEnt (
  IN      PEI_FAT_PRIVATE_DATA  *PrivateData,
  IN OUT  PEI_FAT_FILE          *ParentDir
  )
{
  EFI_STATUS          Status;
  UINTN               Dummy;
  UINTN               DataSize;
  FAT_DIRECTORY_ENTRY DirEntryBuffer;
  FAT_DIRECTORY_ENTRY *DirEntry;
  UINT64              CurrentPos;

  Status                     = EFI_SUCCESS;
  DirEntry                   = (FAT_DIRECTORY_ENTRY *) &DirEntryBuffer;
  CurrentPos                 = ParentDir->CurrentPos;
  ParentDir->FileLastCluster = ParentDir->CurrentCluster;
  
  DivU64x32Remainder (CurrentPos, FILE_DESCRIPTION_BLOCK_SIZE, &Dummy);
  if (Dummy) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Pick a valid directory entry
  //
  while (1) {
    //
    // Read one entry
    //
    DataSize  = FILE_DESCRIPTION_BLOCK_SIZE;
    Status    = FatReadFile (PrivateData, ParentDir, &DataSize, &DirEntryBuffer);
    if (EFI_ERROR (Status)) {
      return EFI_DEVICE_ERROR;
    }
    
    if (DataSize < FILE_DESCRIPTION_BLOCK_SIZE) {
      return EFI_NOT_FOUND;
    }
    //
    // Record CurrentCluster in FileLastCluster, because CurrentCluster will set (-1) when end of directory.
    //
    if (ParentDir->FileLastCluster != ParentDir->CurrentCluster && ParentDir->CurrentCluster != FAT_CLUSTER_LAST) {
      ParentDir->FileLastCluster = ParentDir->CurrentCluster;
    }
    
    if (((UINT8) DirEntry->FileName[0]) == EMPTY_ENTRY_MARK || ((UINT8) DirEntry->FileName[0]) == DELETE_ENTRY_MARK) {
      //
      // We get a valid directory entry, then handle it
      // CurrentPos should back to vaild address because FatReadFile() should change CurrentPos to next entry
      //
      ParentDir->CurrentPos -= FILE_DESCRIPTION_BLOCK_SIZE;
      return EFI_SUCCESS;
    }
    
    if (ParentDir->CurrentCluster == FAT_CLUSTER_LAST) {
      //
      // Exist directory area have no valid directory entry.
      // When CurrentCluster=FAT_CLUSTER_LAST,  FatReadFile() should not  change CurrentPos to next entry
      // so we need add it.
      //
      ParentDir->FileSize   = ParentDir->CurrentPos + FILE_DESCRIPTION_BLOCK_SIZE;
      ParentDir->CurrentPos = ParentDir->FileSize;
      return EFI_OUT_OF_RESOURCES;
    }     
  }  
}

/**
   Append a zero cluster to the current directory.
  
  @param[in]  PrivateData       Global memory map for accessing global variables
  @param[in]  Dir                  The parent directory

  @retval EFI_SUCCESS          Append a zero cluster to the directory successfully.
  @retval other                      An error occurred when appending the zero cluster.
  
**/
STATIC
EFI_STATUS  
FatExpandDir (
  IN PEI_FAT_PRIVATE_DATA  *PrivateData,
  IN PEI_FAT_FILE          *Dir
)
{
  EFI_STATUS  Status;
  UINT32      Position;
  
  Dir->CurrentPos = Dir->FileSize; 
  Position        = Dir->CurrentPos;

  //
  // Find vaild cluster
  //
  Status = FatGrowEof (PrivateData, Dir->Volume, Dir, Dir->FileSize + Dir->Volume->ClusterSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Dir->CurrentCluster = Dir->FileLastCluster;
  
  //
  // Clear all new directory area
  //
  Status = FatWriteZeroPool (PrivateData, Dir);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Dir->CurrentCluster = Dir->FileLastCluster;  
  Dir->CurrentPos     = Position;
  return EFI_SUCCESS;
}

/**
  Find the new directory entry position for the directory entry.

  @param[in]  PrivateData       Global memory map for accessing global variables
  @param[in]  Parent              The parent fat file
  @param[in]  NewDirEntry      The new Directory value
  @param[out] NewFile           The retrieved fat file point

  @retval EFI_SUCCESS          The new directory entry position is successfully found.
  @retval EFI_VOLUME_FULL    The directory has reach its maximum capacity.
  @retval other                     An error occurred when reading the directory entry.

**/
STATIC
EFI_STATUS
FatNewEntryPos (
  IN  PEI_FAT_PRIVATE_DATA  *PrivateData,  
  IN  PEI_FAT_FILE          *Parent,
  IN  FAT_DIRECTORY_ENTRY   *NewDirEntry,
  OUT PEI_FAT_FILE          *NewFile
  )
{

  EFI_STATUS    Status;
  UINT32        Dummy;

  //
  // Find first free space or delete space to insert directory entry.
  // 8.3 name space only one directory(32byte).
  //
  Status = FatFindVaildDirEnt (PrivateData, Parent);
  if (Status == EFI_OUT_OF_RESOURCES) {
    //
    // FAT12, FAT16
    //
    if (Parent->IsFixedRootDir) {
      return EFI_VOLUME_FULL;
    }
    // 
    // There are no vaild directory space. Allocate a new cluster for this directory.
    //    
    Status = FatExpandDir (PrivateData, Parent);
    if (EFI_ERROR(Status)) {
      return Status;
    }    
  } else if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // fill in the output parameter
  //
  DivU64x32Remainder (Parent->CurrentPos, Parent->Volume->ClusterSize, &Dummy);
  
  NewFile->CurrentCluster  = 0;
  NewFile->CurrentPos      = 0;
  NewFile->DirEntryOffset  = Dummy;
  NewFile->FileSize        = 0;
  NewFile->DirEntryCluster = Parent->CurrentCluster;
  NewFile->StartingCluster = 0;
  NewFile->FileLastCluster = 0;
  NewFile->Volume          = Parent->Volume;
  NewFile->IsRootDir       = FALSE;   
  
  return EFI_SUCCESS;
}

/**
  Grow the end of the open file base on the NewSizeInBytes.

  @param[in]  PrivateData        Pointer to the PEF_FAT_PRIVATE_DATA structure
  @param[in]  Volume              Pointer to the PEI_FAT_VOLUME structure
  @param[in]  File                   The open file.
  @param[in]  NewSizeInBytes  The new size in bytes of the open file.
  
  @retval EFI_SUCCESS                  The file is grown sucessfully.
  @retval EFI_UNSUPPORTED           The file size is larger than 4GB.
  @retval EFI_VOLUME_CORRUPTED  There are errors in the files' clusters.
  @retval EFI_VOLUME_FULL            The volume is full and can not grow the file.

**/
EFI_STATUS
FatGrowEof (
  IN PEI_FAT_PRIVATE_DATA  *PrivateData,
  IN PEI_FAT_VOLUME        *Volume,
  IN PEI_FAT_FILE          *File,
  IN UINT64                NewSizeInBytes
  ) 
{
  EFI_STATUS  Status;
  UINTN       Cluster;
  UINTN       CurSize;
  UINTN       NewSize;
  UINTN       LastCluster;
  UINTN       NewCluster;
  UINTN       ClusterCount;

  ClusterCount = 0;
  
  //
  // For FAT file system, the max file is 4GB.
  //
  if (NewSizeInBytes > 0x0FFFFFFFFL) {
    return EFI_UNSUPPORTED;
  }

  //
  // If the file is already large enough, do nothing
  //
  CurSize = FatSizeToClusters (Volume, File->FileSize);
  NewSize = FatSizeToClusters (Volume, (UINTN) NewSizeInBytes);
  if (CurSize < NewSize) {
    
    //
    // If we haven't found the files last cluster do it now
    //
    if ((File->StartingCluster != 0) && (File->FileLastCluster == 0)) {
      Cluster       = File->StartingCluster;
      ClusterCount  = 0;
      //
      // find last Cluster
      //
      while (!FAT_CLUSTER_END_OF_CHAIN (Cluster)) {
        if (Cluster == FAT_CLUSTER_FREE || Cluster >= FAT_CLUSTER_SPECIAL) {
          Status = EFI_VOLUME_CORRUPTED;
          goto Done;
        }

        ClusterCount++;
        File->FileLastCluster  = Cluster;
        Cluster                = FatGetFatEntry (PrivateData, Volume, Cluster);
      }

      if (ClusterCount != CurSize) {
        Status = EFI_VOLUME_CORRUPTED;
        goto Done;
      }
    }
    
    //
    // Loop until we've allocated enough space
    //
    LastCluster = File->FileLastCluster;
 
    while (CurSize < NewSize) {
      NewCluster = FatAllocateCluster (PrivateData, Volume);
      
      if (FAT_CLUSTER_END_OF_CHAIN (NewCluster)) {
        //
        // Volume is full 
        //
        if (LastCluster != FAT_CLUSTER_FREE) {
          FatSetFatEntry (PrivateData, Volume, LastCluster, (UINTN) FAT_CLUSTER_LAST);
          File->FileLastCluster = LastCluster;
        }
 
        Status = EFI_VOLUME_FULL;
        goto Done;
      }
 
      if (LastCluster != 0) {
        FatSetFatEntry (PrivateData, Volume, LastCluster, NewCluster);
      } else {
        File->StartingCluster = NewCluster;
        File->CurrentCluster  = NewCluster;
      }
 
      LastCluster = NewCluster;
      CurSize += 1;
    }
    
    //
    // Terminate the cluster list
    //
    FatSetFatEntry (PrivateData, Volume, LastCluster, (UINTN) FAT_CLUSTER_LAST);
    File->FileLastCluster = LastCluster;
  }

  File->FileSize = (UINT32) NewSizeInBytes;
  
  return EFI_SUCCESS;

Done:
  //
  // Shrink end of file is not implement now.
  //  FatShrinkEof();
  //
  return Status;
}

/**
  Save the directory entry to disk.

  @param[in]  PrivateData        Pointer to the PEF_FAT_PRIVATE_DATA structure
  @param[in]  Parent              The parent fat file  
  @param[in]  NewDirBuffer      The directory entry to be saved.
  
  @retval EFI_SUCCESS           Store the directory entry successfully.
  @retval other                      An error occurred when writing the directory entry.

**/
EFI_STATUS
FatStoreDirEnt (
  IN  PEI_FAT_PRIVATE_DATA  *PrivateData,
  IN  PEI_FAT_FILE          *Parent,
  IN  FAT_DIRECTORY_ENTRY   *NewDirBuffer
  )
{
  UINTN Size;

  //
  // Write directory entry
  //
  Size = FILE_DESCRIPTION_BLOCK_SIZE;
  return FatWriteFile (PrivateData, Parent, &Size, NewDirBuffer);
}

/**
  Create a new Fat Directory entry.

  @param[in]  PrivateData        Pointer to the PEF_FAT_PRIVATE_DATA structure
  @param[in]  Parent               Pointer to Parent Directory.
  @param[in]  FileName            
  @param[in]  Attributes          File's Attributes
  @param[out]  PEI_FAT_FILE   return File Entry
  
  @retval EFI_SUCCESS                  Create the directory sucessfully.
  @retval EFI_UNSUPPORTED           The file name is not 8.3 format.  
  @retval EFI_END_OF_MEDIA          There are no spaces to add directory.
  @retval EFI_DEVICE_ERROR           The device error.

**/
EFI_STATUS
FatCreateDirEnt (
  IN  PEI_FAT_PRIVATE_DATA  *PrivateData,  
  IN  PEI_FAT_FILE          *Parent,
  IN  CHAR16                *FileName,
  IN  UINT8                 Attributes,
  OUT PEI_FAT_FILE          *NewFile
  )
{
  EFI_STATUS          Status;
  CHAR16              *Pos;

  //
  // Generate an 8.3 name.
  // Only support 8.3 name. there are only one entry in count. 
  //
  if (!FatCheckIs8Dot3Name (FileName, NewFile->FileDir.FileName)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Copy file name to NewFile Handle
  //
  Pos = (UINT16 *) NewFile->FileName;
  SetMem ((UINT8 *) Pos, FAT_MAX_FILE_NAME_LENGTH, 0);
  CopyMem ((UINT8 *) Pos, (UINT8 *) FileName, StrSize (FileName));
  
  //
  // Determine the file's directory entry position 
  //
  Status = FatNewEntryPos (PrivateData, Parent, &NewFile->FileDir, NewFile);  
  if (EFI_ERROR (Status)) {
    return Status;
  } 

  //
  // Set DirEntry Fat time
  //
  FatGetCurrentFatTime (PrivateData, &NewFile->FileDir.FileCreateTime);
  CopyMem (&NewFile->FileDir.FileModificationTime, &NewFile->FileDir.FileCreateTime, sizeof (FAT_DATE_TIME));
  CopyMem (&NewFile->FileDir.FileLastAccess, &NewFile->FileDir.FileCreateTime.Date, sizeof (FAT_DATE));

  NewFile->FileDir.Attributes = Attributes;
  NewFile->Attributes         = Attributes;

  //
  // Create new directory entry
  //
  return FatStoreDirEnt (PrivateData, Parent, &NewFile->FileDir);
}

/**
  Set the File's cluster and size info in its directory entry.

  @param[in]  File        The open's file entry.
  
  @retval    none.
  
**/
VOID
FatUpdateDirEntClusterSizeInfo (
  IN PEI_FAT_FILE    *File
  )
{
  File->FileDir.FileSize = (UINT32) File->FileSize;
  FatSetDirEntCluster (File);
}

/**
  Flush the data associated with an open file.
  In this implementation, only last Mod/Access time is updated.
  
  @param[in]  PrivateData     Pointer to the PEF_FAT_PRIVATE_DATA structure
  @param[in]  File                The open's file entry.
  
  @retval    EFI_INVALID_PARAMETER.  File's Directory have some error.
  @retval    EFI_SUCCESS.                 Flush file successed.
  @retval    EFI_DEVICE_ERROR           Something error.
**/
EFI_STATUS
FatFileFlush (
  IN PEI_FAT_PRIVATE_DATA  *PrivateData, 
  IN PEI_FAT_FILE          *File
  )
{
  EFI_STATUS    Status;
  FAT_DATE_TIME FatNow;
  UINT64        ClusterPos;

  //
  // Update file access time
  //
  FatGetCurrentFatTime (PrivateData, &FatNow);      
  CopyMem ((UINT8*)&File->FileDir.FileLastAccess, (UINT8*) &FatNow.Date, sizeof (FAT_DATE));
  CopyMem ((UINT8*)&File->FileDir.FileModificationTime, (UINT8*) &FatNow, sizeof (FAT_DATE_TIME));

  //
  // update whole file's director entry
  //
  if (File->Volume->FatType != Fat32) {
    //
    // FAT12, FAT16
    //
    Status = FatWriteDisk (
               PrivateData,
               File->Volume->BlockDeviceNo,
               File->Volume->RootDirPos + File->DirEntryOffset,
               FILE_DESCRIPTION_BLOCK_SIZE,
               &File->FileDir
               );

  } else {
    //
    // FAT32
    //
    Status = FatGetClusterPos (
               PrivateData, 
               File->Volume, 
               File->DirEntryCluster,
               &ClusterPos
               );
    if (EFI_ERROR(Status)) {
      return Status;
    }
    
    Status = FatWriteDisk (
           PrivateData,
           File->Volume->BlockDeviceNo,
           ClusterPos + File->DirEntryOffset,
           FILE_DESCRIPTION_BLOCK_SIZE,
           &File->FileDir
           );
 
  }

  return Status;
}


