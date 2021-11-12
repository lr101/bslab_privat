//
// Created by lukas on 11.11.2021.
//
#pragma once

#include <math.h>
#include <cstdint>

#include "myfs-structs.h"

#define BYTE_SIZE 8
#define SUPERBLOCK_SIZE 1
#define D_MAP_INDEX 1

typedef uint32_t index_t;

class Superblock {

    size_t size; //blocks
    index_t i_map_index;
    index_t i_node_index;
    index_t data_index;

public:
    Superblock(size_t size, size_t i_node_num);
    ~Superblock();

    size_t cal_imap_size(size_t i_node_num);
    size_t cal_dmap_size (size_t size);
};

