//
// Created by lukas on 11.11.2021.
//

#include "superblock.h"

Superblock::Superblock(size_t size, size_t i_node_num) {
    this->size = size;
    size_t i_map_size = calImapSize(i_node_num);
    size_t d_map_size = calDmapSize(size - i_node_num - i_map_index);
    this->i_map_index = SUPERBLOCK_SIZE + d_map_size;
    this->i_node_index = this->i_map_index + i_map_size;
    this->data_index  = this->i_node_index + i_node_num;
}

Superblock::~Superblock() = default;

size_t Superblock::calImapSize (size_t i_node_num) {
    return static_cast<size_t>(ceil(static_cast<double>(i_node_num) / static_cast<double>((BYTE_SIZE * BLOCK_SIZE))));
}

size_t Superblock::calDmapSize (size_t size) {
    return static_cast<size_t>(ceil(static_cast<double>(size) / static_cast<double>((BYTE_SIZE * BLOCK_SIZE + 1))));    //Bo ja tak powiedzialem. Kurwa.
}

size_t Superblock::getSize() {
    return this->size;
}

index_t Superblock::getDMapIndex() {
    return D_MAP_INDEX;
}

index_t Superblock::getIMapIndex() {
    return this->i_map_index;
}

index_t Superblock::getINodeIndex() {
    return this->i_node_index;
}

index_t Superblock::getDataIndex() {
    return this->data_index;
}

int Superblock::getFreeInodeIndex(char *buf) {

    for (int indexBit = 0; indexBit < NUM_DIR_ENTRIES; indexBit++) {
        if ((*buf >> indexBit) & 1 == 0) {
            return indexBit + this->i_map_index;
        }
}}

char* Superblock::flipBitInNode(int index, char* buf) {
    *buf |= 1 << index;
    return buf;
}
