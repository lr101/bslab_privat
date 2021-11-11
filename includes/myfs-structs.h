//
//  myfs-structs.h
//  myfs
//
//  Created by Oliver Waldhorst on 07.09.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#ifndef myfs_structs_h
#define myfs_structs_h

#define NAME_LENGTH 255
#define BLOCK_SIZE 512
#define NUM_DIR_ENTRIES 64
#define NUM_OPEN_FILES 64
#define NUM_FS_BLOCKS 62500
typedef unsigned int index_t;

// TODO: Add structures of your file system here

struct s_block {
    size_t size;
    index_t d_map_index;
    index_t i_map_index;
    index_t inode_index;
    index_t data_index;
};

#endif /* myfs_structs_h */
