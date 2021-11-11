//
// Created by lukas on 11.11.2021.
//

#include "superblock.h"

Superblock::Superblock(size_t size, size_t i_node_num) {
    this->size = size;
    size_t i_map_size = cal_imap_size(i_node_num);
    size_t d_map_size = cal_dmap_size(size - i_node_num - i_map_index);
    this->i_map_index = SUPERBLOCK_SIZE + d_map_size;
    this->i_node_index = this->i_map_index + i_map_size;
    this->data_index  = this->i_node_index + i_node_num;
}

size_t Superblock::cal_imap_size (size_t i_node_num) {
    return ceil((double) i_node_num/(BYTE_SIZE * BLOCK_SIZE));
}

size_t Superblock::cal_dmap_size (size_t size) {
    return ceil((double) size/(BYTE_SIZE * BLOCK_SIZE + 1));    //Bo ja tak powiedzialem. Kurwa.
}

