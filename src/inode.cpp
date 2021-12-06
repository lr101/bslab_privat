//
// cReATEd bY LuKaS On 08.10.21.
//

#include "inode.h"

/// Create a new file.
/// \param [in] name Path of the file. Can't exceed NAME_LENGTH.
/// \param [in] uid User identification.
/// \param [in] gid Group identification.
/// \param [in] mode Permissions for file access.
/// \throws EINVAL If the length of the file path exceeds NAME_LENGTH from myfs-structs.h.
Inode::Inode(Superblock* s_block, const char *name, uid_t uid, gid_t gid, mode_t mode) {
    int nameLength = std::strlen(name) + 1;
    if (nameLength > NAME_LENGTH) throw std::system_error(EINVAL, std::generic_category());
    this->name = new char[nameLength];
    std::memcpy(this->name, name, nameLength);
    //setSize(0);
    this->uid = uid;
    this->gid = gid;
    this->mode = mode;
    this->s_block = s_block;
    setATime();
    setMTime();
    setCTime();
}

Inode::~Inode() {
    setSize(0);
    delete this->name;
}

/// Change the path to the file.
/// \param name [in] New path to file.
/// \returns 0 on success, -EINVAL If the length of the file path exceeds NAME_LENGTH from myfs-structs.h.
int Inode::setName(const char *name) {
    int nameLength = std::strlen(name) + 1;
    if (nameLength > NAME_LENGTH) return -EINVAL;
    this->name = new char[nameLength];
    std::memcpy(this->name, name, nameLength);
    setMTime();
    return 0;
}

/// Change the size of the data block.
/// \param size [in] New data size.
/// \returns 0 on success
int Inode::setSize(off_t size) {
    if (size < 0) return -EINVAL;
    int ret = 0;
    if (this->size > size) {
        std::vector<index_t> *blockList = new std::vector<index_t>();
        ret += getBlockList(this->size, this->size - size, blockList);
        for (auto &tempBlock : *blockList) {
            ret += this->s_block->removeDBlock(tempBlock);
        }
        delete blockList;
    } else {
        while (size > this->size && ret == 0) {
            ret += appendBlock();
        }
    }
    this->size = size;
    setMTime();
    return ret;
}

int Inode::appendBlock() {
    int ret = 0;
    // blockAmount is the amount of current blocks + 1 to compare via shift and bit masks if new indirect pointer are
    // needed
    index_t blockAmount = getBlockAmount(this->size);
    if (blockAmount < DIR_BLOCK) {
        ret += s_block->addDBlock(&this->block[blockAmount]);
        this->size += BLOCK_SIZE;
    } else if ((blockAmount -= DIR_BLOCK) < IND_BLOCK * N_BLOCK_PTR) {
        // Check if the necessary data block in this->block is already in use
        if (getBlockAmount(this->size) - DIR_BLOCK >> BLOCK_PTR_BITS < blockAmount + 1 >> BLOCK_PTR_BITS || blockAmount == 0) {//TODO if fehlerhaft
            // If not, create it
            ret += s_block->addDBlock(&this->block[(blockAmount >> BLOCK_PTR_BITS) + DIR_BLOCK]);
        }
        char *buf = new char[BLOCK_SIZE];
        // Read the necessary data block from this->block into buf
        ret += s_block->getBlockDevice()->read(this->block[(blockAmount >> BLOCK_PTR_BITS) + DIR_BLOCK], buf);
        // Write the address of a new data block
        ret += s_block->addDBlock((index_t *) &buf[(blockAmount & BLOCK_PTR_BIT_MASK) * sizeof(index_t)]);
        // Write the new buf to blockdevice
        ret += s_block->getBlockDevice()->write(this->block[(blockAmount >> BLOCK_PTR_BITS) + DIR_BLOCK], buf);
        delete[] buf;
        this->size += BLOCK_SIZE;
    } else if ((blockAmount -= IND_BLOCK * N_BLOCK_PTR) < DIND_BLOCK * N_BLOCK_PTR * N_BLOCK_PTR) {
        // Check if the necessary double indirect pointer exists
        if (getBlockAmount(this->size) - IND_BLOCK * N_BLOCK_PTR - DIR_BLOCK >> BLOCK_PTR_BITS * 2 < blockAmount >> BLOCK_PTR_BITS * 2) {
            // If not, create it
            ret += s_block->addDBlock(&this->block[(blockAmount >> BLOCK_PTR_BITS * 2) + DIR_BLOCK + IND_BLOCK]);
        }
        // Check if the necessary indirect pointer exists
        if (getBlockAmount(this->size) - IND_BLOCK * N_BLOCK_PTR - DIR_BLOCK >> BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK < blockAmount >> BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK) {
            char *buf = new char[BLOCK_SIZE];
            // Read block from double indirect pointer
            ret += s_block->getBlockDevice()->read(this->block[(blockAmount >> BLOCK_PTR_BITS * 2) + DIR_BLOCK + IND_BLOCK], buf);
            // Write address of new data block for indirect pointer to buf
            ret += s_block->addDBlock((index_t *) &buf[(blockAmount >> BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK) * sizeof(index_t)]);
            // Write new buf to blockdevice
            ret += s_block->getBlockDevice()->write(this->block[(blockAmount >> BLOCK_PTR_BITS * 2) + DIR_BLOCK + IND_BLOCK], buf);
            delete[] buf;
        }
        char *buf = new char[BLOCK_SIZE];
        // Read block from double indirect pointer
        ret += s_block->getBlockDevice()->read(this->block[(blockAmount >> BLOCK_PTR_BITS * 2) + DIR_BLOCK + IND_BLOCK], buf);
        // Read block from indirect pointer
        ret += s_block->getBlockDevice()->read((index_t) buf[blockAmount >> BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK], buf);
        // Write address to new direct pointer
        ret += s_block->addDBlock((index_t *) &buf[blockAmount & BLOCK_PTR_BIT_MASK]);
        // Write new buf to blockdevice
        ret += s_block->getBlockDevice()->write(this->block[(blockAmount >> BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK)], buf);
        delete[] buf;
        this->size += BLOCK_SIZE;
    } else {
        ret = -ENOMEM;
    }
    return ret;
}

index_t Inode::getBlockAmount(off_t bytes) {
    return (bytes >> 9) + (bytes & BLOCK_SIZE - 1 ? 1 : 0); //9 -> 512 = 2^9; 512 - 1 -> 511 = 0x1FF
}

/// Change the user identification.
/// \param uid [in] New user id.
/// \returns 0 on success
int Inode::setUserID(uid_t uid) {
    this->uid = uid;
    setCTime();
    return 0;
}

/// Change the group identification.
/// \param gid [in] New group id.
/// \returns 0 on success
int Inode::setGroupID(gid_t gid) {
    this->gid = gid;
    setCTime();
    return 0;
}

/// Change the permissions for file access.
/// \param mode [in] New permissions.
/// \returns 0 on success
int Inode::setMode(mode_t mode) {
    this->mode = mode;
    setCTime();
    return 0;
}

/// Update the time of last access to current time.
/// \returns 0 on success
int Inode::setATime() {
    this->atime = std::time(nullptr);
    return 0;
}

/// Update the time of last change to current time.
/// \returns 0 on success
int Inode::setMTime() {
    this->mtime = std::time(nullptr);
    return 0;
}

/// Update the time of last status change to current time.
/// \returns 0 on success
int Inode::setCTime() {
    this->ctime = std::time(nullptr);
    return 0;
}

/// Open the file.
/// \returns 0 on success, -EINVAL If the file is already open.
int Inode::setOpen() {
    if (this->open) return -EINVAL;
    this->open = true;
    setATime();
    return 0;
}

/// Close the file.
/// \returns 0 on success, -EINVAL If the file is already closed.
int Inode::setClose() {
    if (!this->open) return -EBADF;
    this->open = false;
    return 0;
}

/// Get the file path.
/// \param [out] name pointer containing file name
/// \returns 0 on success
int Inode::getName(char *name) {
    std::strcpy(name, this->name);
    return 0;
}

/// Get the data size.
/// \param [out] size pointer containing file size
/// \returns 0 on success
int Inode::getSize(off_t* size) {
    *size = this->size;
    return 0;
}

/// Get the user id.
/// \param [out] uid pointer containing user identification
/// \returns 0 on success
int Inode::getUserID(uid_t* uid) {
    *uid = this->uid;
    return 0;
}

/// Get the group id.
/// \param [out] gid pointer containing group identification
/// \returns 0 on success
int Inode::getGroupID(gid_t* gid) {
    *gid = this->gid;
    return 0;
}

/// Get the permissions for file access.
/// \param [out] mode pointer containing user mode
/// \returns 0 on success
int Inode::getMode(mode_t* mode) {
    *mode = this-> mode;
    return 0;
}

/// Get the time of last access.
/// \param [out] atime pointer containing time of last access
/// \returns 0 on success
int Inode::getATime(std::time_t* atime) {
    *atime = this->atime;
    return 0;
}

/// Get the time of last change.
/// \param [out] mtime pointer containing time of last change
/// \returns 0 on success
int Inode::getMTime(std::time_t* mtime) {
    *mtime = this->mtime;
    return 0;
}

/// Get the time of last status change.
/// \param [out] ctime pointer containing last status change
/// \returns 0 on success
int Inode::getCTime(std::time_t* ctime) {
    *ctime = this->ctime;
    return 0;
}

/// Get whether the file is open.
/// \returns Boolean value if file was already opened.
bool Inode::isOpen() {
    return this->open;
}

/// Override a part of the data block with new data.
/// \param size [in] Size of the new data.
/// \param data [in] Pointer to the new data.
/// \param offset [in] Offset to the location to write the data to.
/// \returns 0 on success, -EINVAL If size negative, -EBADF If file is not open
int Inode::write(off_t size, const char* data, off_t offset) {
    if (!this->open) return -EBADF;
    if (size < 0) return -EINVAL;
    if (size + offset > this->size) {
        setSize(size + offset);
    }
    std::vector<index_t> *blockList = new std::vector<index_t>();
    int ret = getBlockList(size, offset, blockList);
    if (ret < 0) return ret;
    int growingOffset = 0;
    for (auto &tempBlock : *blockList) {
        char* buffer = new char[BLOCK_SIZE];
        this->s_block->getBlockDevice()->read(tempBlock, buffer);
        if (tempBlock == *blockList->begin()) {
            std::memcpy(buffer, data, BLOCK_SIZE - (offset & BLOCK_PTR_BIT_MASK));
            growingOffset += BLOCK_SIZE - (offset & BLOCK_PTR_BIT_MASK);
        } else if (tempBlock == *blockList->end()) {
            std::memcpy(buffer, data + growingOffset, size + offset & BLOCK_PTR_BIT_MASK);
        } else {
            std::memcpy(buffer, data + growingOffset, BLOCK_SIZE);
            growingOffset += BLOCK_SIZE;
        }
    }
    setMTime();
    delete blockList;
    return size;
}

/// Get a pointer to the data with offset.
/// \param offset [in] Offset from the beginning of the data.
/// \param data [out] Pointer to the requested data.
/// \returns 0 on success, -EINVAL If offset is greater than existing data size, -EBADF If file is not open
int Inode::getData(off_t offset, char *data, off_t size) {
    if (!this->open) { return -EBADF; }
    if (offset > this->size / BLOCK_SIZE) return -EINVAL;

    std::vector<index_t> *dataBlocks = new std::vector<index_t>();
    this->getBlockList(size, offset, dataBlocks);
    char *requiredData = this->collectDataFromBlocks(offset, size, dataBlocks);
    memcpy(data, requiredData, size);
    setATime();
    delete dataBlocks;
    return 0;
}
/// Get the metadata of a file (user & group id, modification times, permissions, ...).
/// \param [out] statbuf Structure containing the meta data, for details type "man 2 stat" in a terminal.
/// \returns 0 on success
int Inode::getMetadata(struct stat *statbuf) {
    getUserID(&statbuf->st_uid);
    getGroupID(&statbuf->st_gid);
    getATime(&statbuf->st_atime);
    getMTime(&statbuf->st_mtime);
    getMode(&statbuf->st_mode);
    getSize(&statbuf->st_size);
    statbuf->st_nlink = 1;  //Set amount of hard links to file to 1 for now.
    return 0;
}

int Inode::getBlockList(off_t size, off_t offset, std::vector<index_t>* blockList) {
    if (size < 0 || size + offset > this->size) return -EINVAL;
    index_t startBlockIndex = getBlockAmount(offset) - 1;
    index_t endBlockIndex = getBlockAmount(size + offset) - 1;
    int ret = 0;

    for (index_t i = startBlockIndex; i <= endBlockIndex; i++) {
        index_t* realBlockAddr = new index_t;
        ret += getBlock(i, realBlockAddr);
        blockList->push_back(*realBlockAddr);
        delete realBlockAddr;
    }

    return ret;
}

int Inode::getBlock(index_t blockIndex, index_t* realBlockAddr) {
    int ret = 0;
    if (blockIndex < 0) {
        ret = -EINVAL;
    } else if (blockIndex < DIR_BLOCK) {
        *realBlockAddr = this->block[blockIndex];
    } else if ((blockIndex -= DIR_BLOCK) < IND_BLOCK * N_BLOCK_PTR) {
        char *buffer = new char[BLOCK_SIZE];
        ret += s_block->getBlockDevice()->read(this->block[(blockIndex >> BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK) + DIR_BLOCK], buffer);
        *realBlockAddr = (index_t) buffer[blockIndex & BLOCK_PTR_BIT_MASK];
        delete[] buffer;
    } else if ((blockIndex -= IND_BLOCK * N_BLOCK_PTR) < DIND_BLOCK * N_BLOCK_PTR * N_BLOCK_PTR) {
        char *buffer = new char[BLOCK_SIZE];
        ret += s_block->getBlockDevice()->read(this->block[(blockIndex >> BLOCK_PTR_BITS * 2 & BLOCK_PTR_BIT_MASK) + DIR_BLOCK + IND_BLOCK], buffer);
        ret += s_block->getBlockDevice()->read((index_t) buffer[blockIndex >> BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK], buffer);
        *realBlockAddr = (index_t) buffer[blockIndex & BLOCK_PTR_BIT_MASK];
        delete[] buffer;
    } else {
        ret = -EINVAL;
    }
    return ret;
}

int Inode::setBlockPointer(int index, index_t blockNo) {
    if (index >= 0 && index < N_BLOCKS) return -EINVAL;
    this->block[index] = blockNo;
    return 0;
}

char *Inode::collectDataFromBlocks(off_t offset, off_t size, std::vector<uint32_t>* dataBlocks) {
    uint32_t startBlockIndex = offset / BLOCK_SIZE;
    uint32_t endBlockIndex = (size + offset) / BLOCK_SIZE;
    uint32_t numberOfBlocks = endBlockIndex - startBlockIndex;
    char *requiredData = new char[size-offset];
    for (int i = 0; i < numberOfBlocks; i++) {
        char *buffer = new char[BLOCK_SIZE];
        s_block->getBlockDevice()->read(dataBlocks->at(i), buffer);

        if (i == 0) {
            uint32_t startBlockByte = offset % BLOCK_SIZE;
            for (int bufferIndex = startBlockByte; bufferIndex < size || bufferIndex < BLOCK_SIZE; i++) {
                *requiredData += buffer[bufferIndex];
            }
        } else if (i == numberOfBlocks - 1) {
            uint32_t endBlockByte = (size + offset) % BLOCK_SIZE;
            for (int bufferIndex = 0; bufferIndex < endBlockByte; i++) {
                *requiredData += buffer[bufferIndex];
            }
        } else { *requiredData += *buffer; }

        delete[] buffer;
    }
    return requiredData;
}
