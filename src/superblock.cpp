//
// Created by lukas on 11.11.2021.
//

#include <iostream>
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

int Superblock::loadINodes(std::vector<InodePointer*>* iNodes) {
    char buf [BLOCK_SIZE] = {};
    int ret = this->blockDevice->read(this->getIMapIndex(), buf);
    for (int indexBit = 0; indexBit < NUM_DIR_ENTRIES && ret == 0; indexBit++) {
        if (((buf[indexBit >> BYTE_BITS] >> (indexBit % BYTE_SIZE)) & 1 ) == 1) {
            auto ip = new struct InodePointer();
            ret += this->getINode(this->getINodeIndex() + indexBit, ip);
            ip->inode->setSuperblock(this);
            iNodes->push_back(ip);
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
int Superblock::getINode(index_t blockNo, InodePointer* ip) {
    ip->inode = (Inode*) malloc(sizeof(Inode));
    ip->blockNo = blockNo;
    ip->blockDevice = blockDevice;
    char buf [BLOCK_SIZE] = {};
    (void) blockDevice->read(blockNo, buf);
    (void) std::memcpy(ip->inode, buf, sizeof(Inode));
    return 0;
}


int Superblock::addBlocks(Inode* inode, off_t numNewBlocks, index_t startBlockIndex) {
    index_t endBlockIndex = startBlockIndex + numNewBlocks;
    index_t indirectAddress = -1;
    int ret = 0;

    for (index_t i = startBlockIndex; i < endBlockIndex; i++) {
        if (i < DIR_BLOCK) {
            ret += setInodeDataPointer(inode, i);
        } else if (i - DIR_BLOCK < N_IND_BLOCKS_PTR) {

            index_t relativeIndex = i - DIR_BLOCK;
            index_t indexInodePointer = (relativeIndex >> BLOCK_PTR_BITS) + DIR_BLOCK;

            if (relativeIndex % N_BLOCK_PTR == 0) ret += setInodeDataPointer(inode, indexInodePointer);

            (void) setIndirectPointer(inode->getBlock(indexInodePointer), (relativeIndex % N_BLOCK_PTR));

        } else if (i - DIR_BLOCK - N_IND_BLOCKS_PTR < DIND_BLOCK * pow(N_BLOCK_PTR, 2)) {

            index_t relativeIndex = i - DIR_BLOCK - N_IND_BLOCKS_PTR;
            index_t indexInodePointer = (relativeIndex >>  BLOCK_PTR_BITS) + DIR_BLOCK + IND_BLOCK;
            index_t indirectPointerIndex = relativeIndex >>  BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK;

            if (relativeIndex % (N_BLOCK_PTR * N_BLOCK_PTR) == 0)  ret += setInodeDataPointer(inode, indexInodePointer);
            if (relativeIndex % N_BLOCK_PTR == 0) indirectAddress = setIndirectPointer(inode->getBlock(indexInodePointer), indirectPointerIndex);

            (void)setIndirectPointer(indirectAddress, (relativeIndex % N_BLOCK_PTR));

        } else {
            ret = -EINVAL;
        }
    }
    return ret;
}

int Superblock::rmBlocks(Inode* inode, off_t numRmvBlocks, index_t endBlockIndex) {
    index_t startBlockIndex = endBlockIndex - numRmvBlocks;
    index_t indirectAddress = -1;
    int ret = 0;

    for (index_t i = startBlockIndex; i < endBlockIndex; i++) {
        if (i < DIR_BLOCK) {
            ret += toggleDMapIndex(inode->getBlock(i));

        } else if (i - DIR_BLOCK < N_IND_BLOCKS_PTR) {

            index_t relativeIndex = i - DIR_BLOCK;
            index_t indexInodePointer = (relativeIndex >> BLOCK_PTR_BITS) + DIR_BLOCK;

            if (relativeIndex % N_BLOCK_PTR == 0) ret += toggleDMapIndex(inode->getBlock(indexInodePointer));
            ret += toggleDMapIndex(getIndirectPointer(inode->getBlock(indexInodePointer), (relativeIndex % N_BLOCK_PTR)));

        } else if (i - DIR_BLOCK - N_IND_BLOCKS_PTR < DIND_BLOCK * pow(N_BLOCK_PTR,2)) {
            index_t relativeIndex = i - DIR_BLOCK - N_IND_BLOCKS_PTR;
            index_t indexInodePointer = (relativeIndex >>  BLOCK_PTR_BITS) + DIR_BLOCK + IND_BLOCK;
            index_t indirectPointerIndex = relativeIndex >>  BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK;
            if (relativeIndex % (N_BLOCK_PTR * N_BLOCK_PTR) == 0)  ret += toggleDMapIndex(inode->getBlock(indexInodePointer));
            if (relativeIndex % N_BLOCK_PTR == 0) {
                indirectAddress = getIndirectPointer(inode->getBlock(indexInodePointer), indirectPointerIndex);
                ret += toggleDMapIndex(indirectAddress);
            }
            ret += toggleDMapIndex(getIndirectPointer(indirectAddress, (relativeIndex % N_BLOCK_PTR)));

        } else {
            ret = -EINVAL;
        }
    }
    return ret;
}

int Superblock::setInodeDataPointer(Inode* inode, int pointerIndex) {
    index_t freeDataBlock = this->getFreeDataBlockNo();
    return inode->setBlockPointer(pointerIndex, freeDataBlock + this->data_index);
}


index_t Superblock::setIndirectPointer(index_t dataBlockNo, off_t offset) {
    index_t address = this->getFreeDataBlockNo() + this->data_index;
    char buf [BLOCK_SIZE] = {};
    (void) blockDevice->read(dataBlockNo, buf);
    buf[offset * BYTES_PER_ADDRESS] |= address;
    (void) blockDevice->write(dataBlockNo, buf);
    return address;
}

index_t Superblock::getIndirectPointer( index_t dataBlockNo, off_t offset) {
    index_t address;
    char buf [BLOCK_SIZE] = {};
    (void) blockDevice->read(dataBlockNo, buf);
    (void) std::memcpy(&address,buf + (offset * BYTES_PER_ADDRESS), BYTES_PER_ADDRESS );
    return address;
}

///
/// \param dataBlockNo the relative Block number that is toggled: therefore the bit changes from 1 -> 0 or 0 -> 1
/// \param blockDevice blockDevice pointer
/// \return error code
int Superblock::toggleDMapIndex(index_t dataBlockNo) {
    dataBlockNo -= this->data_index;
    int dMapIndex = dataBlockNo / (BYTE_SIZE * BLOCK_SIZE) + D_MAP_INDEX;
    int dMapByteIndex = dataBlockNo % (BYTE_SIZE * BLOCK_SIZE);
    char buf [BLOCK_SIZE] = {};
    (void) blockDevice->read(dMapIndex, buf);
    flipBitInBuf(dMapByteIndex, buf, dMapIndex);
    return 0;
}
index_t Superblock::getFreeDataBlockNo() {
    char buf [BLOCK_SIZE] = {};
    for (int dMapIndex = D_MAP_INDEX; dMapIndex < this->i_map_index; dMapIndex++) {
        (void) blockDevice->read(dMapIndex, buf);
        for (int i = 0; i < BLOCK_SIZE * BYTE_SIZE; i++) {
            if (((buf[i >> BYTE_BITS] >> (i % BYTE_SIZE)) & 1 ) == 0) {
                flipBitInBuf(i, buf, dMapIndex);
                return i + (D_MAP_INDEX - 1) * BYTE_SIZE * BLOCK_SIZE;
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
        if (((buf[indexBit >> BYTE_BITS] >> (indexBit % BYTE_SIZE)) & 1 ) == 0) {
            int ret = this->flipBitInBuf(indexBit, buf, this->i_map_index);
            return (ret < 0 ? ret : indexBit);
        }
    }
    return -EINVAL;
}

int Superblock::flipBitInBuf(int index, char* buf, int mapIndex) {
    buf[index >> BYTE_BITS] ^= 1 <<  (index % BYTE_SIZE);
    return this->blockDevice->write(mapIndex,  buf);
}

int Superblock::setBlockDevice(BlockDevice *pBlockDevice) {
    this->blockDevice = pBlockDevice;
    return 0;
}
