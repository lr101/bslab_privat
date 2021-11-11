//
// Created by lukas on 11.11.2021.
//
#pragma once
#include <math.h>

#include "myfs-structs.h"

#define BYTE_SIZE 8
#define SUPERBLOCK_SIZE

class Superblock {
    size_t size; //blocks
    index_t d_map_index;
    index_t i_map_index;
    index_t inode_index;
    index_t data_index;
public:
    Superblock(size_t size, size_t inode_num);

    size_t cal_imap_size(size_t inode_num);

    size_t cal_dmap_size (size_t size)
};

