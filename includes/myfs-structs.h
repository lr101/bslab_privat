//
//  myfs-structs.h
//  myfs
//
//  Created by Oliver Waldhorst on 07.09.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#ifndef myfs_structs_h
#define myfs_structs_h

#include "inode.h"
#include "blockdevice.h"

#define NAME_LENGTH 255
#define BLOCK_SIZE 512
#define NUM_DIR_ENTRIES 64
#define NUM_OPEN_FILES 64
#define NUM_FS_BLOCKS 62500
#define INDEX_SUPERBLOCK 0

struct InodePointer {
    Inode* inode;
    BlockDevice* blockDevice;

    uint32_t blockNo;
};

#endif /* myfs_structs_h */
