//
// Created by lukas on 11.11.2021.
//
#pragma once

#include <cmath>
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

    static size_t calImapSize(size_t i_node_num);
    static size_t calDmapSize (size_t size);

public:
    Superblock(size_t size, size_t i_node_num);
    ~Superblock();

    size_t getSize();
    index_t getDMapIndex();
    index_t getIMapIndex();
    index_t getINodeIndex();
    index_t getDataIndex();
};

