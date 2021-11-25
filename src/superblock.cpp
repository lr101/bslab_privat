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

/// Load INodes from BlockDevice
///
/// This function is called when the file system is mounted and a container does already exits.
/// All Inodes from the container are loaded into the inmemory file system
/// \param
/// \return 0 on success -ERRNO on failure.

int Superblock::loadINodes(BlockDevice* blockDevice, InodePointer* ip) {
    char buf [BLOCK_SIZE] = {};
    int ret  = blockDevice->read(this->getIMapIndex(), buf);
    if (ret >= 0) {
        for (int indexBit = 0; indexBit < NUM_DIR_ENTRIES; indexBit++) {
            if (((*buf >> indexBit) & 1) == 1) {

                ret += this->getINode(this->getINodeIndex() + indexBit, ip, blockDevice);

                if (ret >= 0) {
                    std::string path;
                    ret += ip->inode->getName(&path);
                }
            }
        }
    }
    return ret;
}

/// Load read INode from a given block into an INodePointer
///
/// This function reads an INode from a given blockNo into an INodePointer from the container
///
/// \param blockNo [in] block number in BLockDevice
/// \param ip [out] struct InodePointer filled with the blockNo, Inode Object, BlockDevice Pointer
/// \return 0 on success
int Superblock::getINode(index_t blockNo, InodePointer* ip, BlockDevice* blockDevice) {
    ip->inode = (Inode*) malloc(sizeof(Inode));
    ip->blockNo = blockNo;
    ip->blockDevice = blockDevice;
    char buf [BLOCK_SIZE] = {};
    (void) blockDevice->read(blockNo, buf);
    std::memcpy(ip->inode, buf, sizeof(Inode));
    return 0;
}

