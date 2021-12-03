//
// Created by lukas on 11.11.2021.
//

#include "superblock.h"

Superblock::Superblock(size_t size, size_t i_node_num, BlockDevice* blockDevice) {
    this->size = size;
    size_t i_map_size = calImapSize(i_node_num);
    size_t d_map_size = calDmapSize(size - i_node_num - i_map_index);
    this->i_map_index = SUPERBLOCK_SIZE + d_map_size;
    this->i_node_index = this->i_map_index + i_map_size;
    this->data_index  = this->i_node_index + i_node_num;
    this->blockDevice = blockDevice;
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
    (void) std::memcpy(ip->inode, buf, sizeof(Inode));
    return 0;
}

int Superblock::addDBlock(index_t *blockIndex) {
    index_t freeDBlock = getFreeDataBlockNo();
    *blockIndex = freeDBlock + getDataIndex();
    return toggleDMapIndex(freeDBlock);
}

int Superblock::removeDBlock(index_t blockIndex) {
    if (blockIndex < getDataIndex()) return -EINVAL;
    int ret = 0;
    blockIndex -= getDataIndex();
    char buf[BLOCK_SIZE];
    ret = blockDevice->read(getIMapIndex(), buf);
    if (*buf & (1 << blockIndex)) {
        *buf ^= (1 << blockIndex);
    } else {
        ret = -EINVAL;  //data block isn't used yet
    }
    blockDevice->write(getIMapIndex(), buf);
    return ret;
}

int Superblock::setInodeDataPointer(Inode* inode, int pointerIndex) {
    index_t freeDataBlock = this->getFreeDataBlockNo();
    int ret = this->toggleDMapIndex(freeDataBlock);
    ret += inode->setBlockPointer(pointerIndex, freeDataBlock + this->data_index);
    return ret;
}


index_t Superblock::setIndirectPointer(index_t dataBlockNo, off_t byteOffset) {
    index_t address = this->getFreeDataBlockNo() + this->data_index;
    char buf [BLOCK_SIZE] = {};
    (void) blockDevice->read(dataBlockNo, buf);
    char addrBuf [BYTES_PER_ADDRESS] = {};
    (void) std::memcpy(&address, addrBuf, BYTES_PER_ADDRESS);
    for (int i = 0; i < BYTES_PER_ADDRESS; i++) { buf[byteOffset + i] = addrBuf[i];}
    (void) blockDevice->write(dataBlockNo, buf);
    return address;
}

index_t Superblock::getIndirectPointer( index_t dataBlockNo, off_t byteOffset) {
    index_t address;
    char buf [BLOCK_SIZE] = {};
    (void) blockDevice->read(dataBlockNo, buf);
    char addrBuf [BYTES_PER_ADDRESS] = {};
    for (int i = 0; i < BYTES_PER_ADDRESS; i++) { addrBuf[i] = buf[byteOffset + i];}
    (void) std::memcpy(addrBuf, &address, BYTES_PER_ADDRESS);
    return address;
}

///
/// \param dataBlockNo the relative Block number that is toggled: therefore the bit changes from 1 -> 0 or 0 -> 1
/// \param blockDevice blockDevice pointer
/// \return error code
int Superblock::toggleDMapIndex(index_t dataBlockNo) {
    int dMapIndex = dataBlockNo / (BYTE_SIZE * BLOCK_SIZE) + D_MAP_INDEX;
    int dMapByteIndex = (BYTE_SIZE * BLOCK_SIZE) - dataBlockNo % N_BLOCK_PTR;
    char buf [BLOCK_SIZE] = {};
    (void) blockDevice->read(dMapIndex, buf);
    *buf ^=  1UL << dMapByteIndex;
    (void) blockDevice->write(dMapIndex, buf);
    return 0;
}

index_t Superblock::getFreeDataBlockNo() {
    char buf [BLOCK_SIZE] = {};
    for (int dMapIndex = D_MAP_INDEX; dMapIndex < this->i_map_index; dMapIndex++) {
        (void) blockDevice->read(dMapIndex, buf);
        for (int i = 0; i < BLOCK_SIZE * BYTE_SIZE; i++) {
            if (((*buf >> i) & 1) == 0) {
                return dMapIndex * BLOCK_SIZE * BYTE_SIZE + i;
            }
        }
    }
    return -1;
}



BlockDevice* Superblock::getBlockDevice() {
    return this->blockDevice;
}
int Superblock::getFreeInodeIndex() {
    char buf [BLOCK_SIZE] = {};
    this->blockDevice->read(this->getIMapIndex(), buf);
    for (int indexBit = 0; indexBit < NUM_DIR_ENTRIES; indexBit++) {
        if (((*buf >> indexBit) & 1) == 0) {
            int ret = this->flipBitInNode(indexBit, buf);
            return (ret < 0 ? ret : indexBit);
        }
    }
    return -EINVAL;
}

int Superblock::flipBitInNode(int index, char* buf) {
    *buf |= (1 << index);
    return this->blockDevice->write(this->getIMapIndex(), buf);
}
