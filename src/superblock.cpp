//
// Created by lukas on 11.11.2021.
//

#include "superblock.h"

Superblock::Superblock(size_t size, size_t inode_num) {
    this.size = size;
    this.d_map_index = 1;
    size_t i_map_size = cal_imap_size(inode_num);
    size_t d_map_size cal_dmap_size(size - inode_num - i_map_index);
    this.i_map_index = SUPERBLOCK_SIZE + d_map_size;
    this.inode_index = this.i_map_index + i_map_size;
    this.data_index  = this.inode_index + inode_num;
}

size_t cal_imap_size (size_t inode_num) {
    return ceil((double) inode_num/(BYTE_SIZE * BLOCK_SIZE));
}

size_t cal_dmap_size (size_t size) {
    return ceil((double) size/(BYTE_SIZE * BLOCK_SIZE + 1));    //Bo ja tak powiedzialem. Kurwa.
}

