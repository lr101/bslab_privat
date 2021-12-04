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
    std::memcpy(this->name, name, nameLength);
    this->size = 0;
    setSize(0);
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
}

/// Change the path to the file.
/// \param name [in] New path to file.
/// \returns 0 on success, -EINVAL If the length of the file path exceeds NAME_LENGTH from myfs-structs.h.
int Inode::setName(const char *name) {
    int nameLength = std::strlen(name) + 1;
    if (nameLength > NAME_LENGTH) return -EINVAL;
    std::memcpy(this->name, name, nameLength);
    setMTime();
    return 0;
}

/// Change the size of the data block.
/// \param size [in] New data size.
/// \returns 0 on success
int Inode::setSize(off_t size) {
    if (size < 0) return -EINVAL;
    if (this->size > size) {
         off_t numRmvBlocks = (((this->size - size) / 4096) + 1) << BYTE_BITS;
         this->s_block->rmBlocks(this, numRmvBlocks, ((this->size  / 4096) + 1) << BYTE_BITS);
    } else if (this->size < size){
        off_t numNewBlocks = (((size - this->size) / 4096) + 1) << BYTE_BITS;
        this->s_block->addBlocks(this, numNewBlocks, this->size / BLOCK_SIZE);
    }
    this->size = size;
    setMTime();
    return 0;
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
    std::vector<index_t> blockList;
    int ret = getBlockList(size, offset, &blockList);
    if (ret < 0) return ret;
    int growingOffset = 0;
    offset = offset % BLOCK_SIZE;

    for (const auto& tempBlock : blockList) {
        char buf [BLOCK_SIZE] = {};
        if (tempBlock == blockList.front()) {
            this->s_block->getBlockDevice()->read(tempBlock, buf);
            std::memcpy(&(buf[offset]), data, BLOCK_SIZE - offset);
            growingOffset += BLOCK_SIZE - offset;
        } else if (tempBlock == blockList.back()) {
            std::memcpy(buf, data + growingOffset, size - growingOffset);
        } else {
            std::memcpy(buf, data + growingOffset, BLOCK_SIZE);
            growingOffset += BLOCK_SIZE;
        }
        this->s_block->getBlockDevice()->write(tempBlock, buf);
    }
    setMTime();
    return size;
}

/// Get a pointer to the data with offset.
/// \param offset [in] Offset from the beginning of the data.
/// \param data [out] Pointer to the requested data.
/// \returns 0 on success, -EINVAL If offset is greater than existing data size, -EBADF If file is not open
int Inode::getData(off_t offset, char *data, off_t size) {
    if (!this->open) { return -EBADF; }
    if (offset > this->size / BLOCK_SIZE) return -EINVAL;

    std::vector<index_t> blockList;
    int ret = getBlockList((size < this->size ? size : this->size), offset, &blockList);
    if (ret < 0) return ret;
    int growingOffset = 0;
    offset = offset % BLOCK_SIZE;

    for (const auto& tempBlock : blockList) {
        char buf [BLOCK_SIZE] = {};
        this->s_block->getBlockDevice()->read(tempBlock, buf);

        if (tempBlock == blockList.front()) {
            std::memcpy(data,&(buf[offset]), BLOCK_SIZE - offset);
            growingOffset += BLOCK_SIZE - offset;
        } else if (tempBlock == blockList.back()) {
            std::memcpy(&data[growingOffset], buf, size - growingOffset);
        } else {
            std::memcpy( &data[growingOffset], buf, BLOCK_SIZE);
            growingOffset += BLOCK_SIZE;
        }
    }

    setATime();
    return size;
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
    if (size < 0 || size + offset > ((this->size >> 12) + 1) << 12) return -EINVAL;
    index_t startBlockIndex = offset / BLOCK_SIZE;
    index_t endBlockIndex = startBlockIndex + ceil(size * 1.0 / BLOCK_SIZE);
    int ret = 0;

    for (index_t i = startBlockIndex; i < endBlockIndex; i++) {
        int currentBlockIndex = i;

        if (currentBlockIndex < DIR_BLOCK) {

            blockList->push_back(this->block[currentBlockIndex]);

        } else if ((currentBlockIndex -= DIR_BLOCK) < IND_BLOCK * N_BLOCK_PTR) {
            index_t directP = this->s_block->getIndirectPointer(this->block[(currentBlockIndex >> BLOCK_PTR_BITS) + DIR_BLOCK], currentBlockIndex % N_BLOCK_PTR);
            blockList->push_back(directP);

        } else if ((currentBlockIndex -= IND_BLOCK * N_BLOCK_PTR) < DIND_BLOCK * N_BLOCK_PTR * N_BLOCK_PTR) {

            index_t indirectP = this->s_block->getIndirectPointer(this->block[(currentBlockIndex >>  BLOCK_PTR_BITS) + DIR_BLOCK + IND_BLOCK], currentBlockIndex >>  BLOCK_PTR_BITS & BLOCK_PTR_BIT_MASK);
            index_t directP =   this->s_block->getIndirectPointer(indirectP, (currentBlockIndex & BLOCK_PTR_BIT_MASK));
            blockList->push_back(directP);

        } else {
            ret = -EINVAL;
        }
    }

    return ret;
}

size_t Inode::getBlock(int index) {
    if (index < 0 && index >= N_BLOCKS) return -EINVAL;
    return this->block[index];
}

int Inode::setBlockPointer(int index, index_t blockNo) {
    if (index < 0 && index >= N_BLOCKS) return -EINVAL;
    this->block[index] = blockNo;
    return 0;
}

int Inode::setSuperblock(Superblock* s_block) {
    this->s_block = s_block;
    return 0;
}
