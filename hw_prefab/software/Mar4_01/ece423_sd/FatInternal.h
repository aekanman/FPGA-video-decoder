// --------------------------------------------------------------------
// Copyright (c) 2007 by Terasic Technologies Inc. 
// --------------------------------------------------------------------
//
// Permission:
//
//   Terasic grants permission to use and modify this code for use
//   in synthesis for all Terasic Development Boards and Altera Development 
//   Kits made by Terasic.  Other use of this code, including the selling 
//   ,duplication, or modification of any portion is strictly prohibited.
//
// Disclaimer:
//
//   This VHDL/Verilog or C/C++ source code is intended as a design reference
//   which illustrates how these types of functions can be implemented.
//   It is the user's responsibility to verify their design for
//   consistency and functionality through the use of formal
//   verification methods.  Terasic provides no warranty regarding the use 
//   or functionality of this code.
//
// --------------------------------------------------------------------
//           
//                     Terasic Technologies Inc
//                     356 Fu-Shin E. Rd Sec. 1. JhuBei City,
//                     HsinChu County, Taiwan
//                     302
//
//                     web: http://www.terasic.com/
//                     email: support@terasic.com
//
// --------------------------------------------------------------------

#ifndef FATINTERNAL_H_
#define FATINTERNAL_H_

#include "hw_sd.h"
#include "FatFileSystem.h"

#define MY_SECTER_SIZE 512

typedef void *DISK_HANDLE;
typedef bool (*READ_BLOCK512_FUNC)(DISK_HANDLE DiskHandle, alt_u32 PysicalSelector, alt_u8 szBuf[512]);
typedef bool (*WRITE_BLOCK512_FUNC)(DISK_HANDLE DiskHandle, alt_u32 PysicalSelector, alt_u8 szBuf[512]);

typedef bool (*READ_BLOCKS_FUNC)(DISK_HANDLE DiskHandle, alt_u32 PysicalSelector, alt_u32 n, char* buf);

#define UNUSED_DIR_ENTRY           0xE5
#define REMAINED_UNUSED_DIR_ENTRY  0x00


typedef enum{
    PARTITION_FAT16=6,  // limited to 2G
    PARTITION_FAT32=11,
}PARTITION_TYPE;

typedef struct{
    // Hardware Access Function
    DISK_HANDLE     DiskHandle;
    READ_BLOCK512_FUNC ReadBlock512;
    WRITE_BLOCK512_FUNC WriteBlock512;
    READ_BLOCKS_FUNC	ReadBlocks;
    
    //    
    alt_u32 Partition_Type;
    bool bMount;
    //
    alt_u32 PartitionStartSecter;
    alt_u32 BPB_BytsPerSec;
    alt_u32 BPB_SecPerCluster;
    alt_u32 BPB_RsvdSecCnt;
    alt_u32 BPB_NumFATs;
    alt_u32 BPB_RootEntCnt;
    alt_u32 BPB_FATSz;
    //
    alt_u32 nBytesPerCluster;
    alt_u32 FatEntrySecter;
    alt_u32 RootDirectoryEntrySecter;
    alt_u32 DataEntrySecter;
    
    // temp data for secter
    alt_u8  Secter_Data[MY_SECTER_SIZE];
    alt_u32 Secter_Index; 
    //
#ifdef FAT_READONLY    
    char *szFatTable; // read into memory for speedup
#endif
        
    
}VOLUME_INFO;


typedef struct{
    bool IsOpened;
    unsigned int OpenAttribute;
    unsigned int SeekPos;
    unsigned int Cluster;
    unsigned int ClusterSeq; // zero base
    //FAT_DIRECTORY Directory;
    FILE_CONTEXT Directory;
    // 
    FAT_HANDLE Fat;
}FAT_FILE_INFO;


typedef enum{
    CLUSTER_UNUSED,
    CLUSTER_RESERVED,
    CLUSTER_BAD,
    CLUSTER_LAST_INFILE,
    CLUSTER_NEXT_INFILE
}CLUSTER_TYPE;

typedef struct{
    char Name[8];
    char Extension[3];
    char Attribute;
    char reserved[2]; 
    unsigned short CreateTime;
    unsigned short CreateDate;
    unsigned short LastAccessDate;
    unsigned short FirstLogicalClusterHi; // not used in FAT12/FAT16
    unsigned short LastWriteTime;
    unsigned short LastWriteDate;
    unsigned short FirstLogicalCluster;
    unsigned int FileSize;
}fat_packed FAT_DIRECTORY;  //fat_packed

typedef struct{
    char LDIR_Ord;
    char LDIR_Name1[10];
    char LDIR_Attr;  // must be ATTR_LONG_NAME (0x0F)
    char LDIR_Type;
    char LDIR_Chksum;
    char LDIR_Name2[12];
    char LDIR_FstClusLO[2]; // must be zero
    char LDIR_Name3[4]; 
}fat_packed FAT_LONG_DIRECTORY;  //fat_packed


#define ATTR_IBUTE_VOLUME_BIT       (0x01)
#define ATTRIBUTE_DIRECTORY_BIT    (0x01 << 1)
#define ATTRIBUTE_SYSTEM_BIT       (0x01 << 2)
#define ATTRIBUTE_READONLY_BIT     (0x01 << 3)
#define ATTRIBUTE_ARCHIVE_BIT      (0x01 << 4)
#define ATTRIBUTE__BIT             (0x01 << 5)

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME (ATTR_READ_ONLY |ATTR_HIDDEN |ATTR_SYSTEM |ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK (ATTR_READ_ONLY |ATTR_HIDDEN |ATTR_SYSTEM |ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)

#define LAST_LONG_ENTRY         0x40

//
FAT_HANDLE fatMountSdcard(void);
FAT_HANDLE fatMountUsbDisk(DEVICE_HANDLE hUsbDisk);
//FAT_HANDLE fatMountUsbDisk(DEVICE_HANDLE hDevice);


// internal function
CLUSTER_TYPE fat16ClusterType(alt_u32 Fat);
CLUSTER_TYPE fat32ClusterType(alt_u32 Fat);
alt_u32 fatNextCluster(VOLUME_INFO *pVol, alt_u32 ThisCluster);
alt_u32 fatFindUnusedCluster(VOLUME_INFO *pVol);
void fatDumpDate(unsigned short Date);
void fatDumpTime(unsigned short Date);
bool fatIsValidDir(FAT_DIRECTORY *pDir);
bool fatIsLastDir(FAT_DIRECTORY *pDir);
bool fatDelClusterList(VOLUME_INFO *pVol, alt_u32 StartCluster);

//===============
// add for v2.0
bool fatReadSecter(VOLUME_INFO *pVol, alt_u32 nSecter);
bool fatReadMultiSecter(VOLUME_INFO *pVol, alt_u32 nSecter, alt_u32 n, char* buf);

FAT_DIRECTORY* fatFindDirectory(VOLUME_INFO *pVol, alt_u32 nDirectoryIndex);
alt_u32 fatFindUnusedDirectory(VOLUME_INFO *pVol);
bool fatIsUnusedDir(FAT_DIRECTORY *pDir);
bool fatCreateFileDirectory(VOLUME_INFO *pVol, char *pFilename);
//===============

// debug tool
void fatDump(FAT_DIRECTORY *pDirectory);



#endif /*FATINTERNAL_H_*/
