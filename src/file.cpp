//
// Created by lukas on 08.10.21.
//

#include "file.h"

/// Create a new file.
/// \param [in] name Path of the file. Can't exceed NAME_LENGTH.
/// \param [in] uid User identification.
/// \param [in] gid Group identification.
/// \param [in] mode Permissions for file access.
/// \throws EINVAL If the length of the file path exceeds NAME_LENGTH from myfs-structs.h.
File::File(std::string *name, uid_t uid, gid_t gid, mode_t mode) {
    if (name->size() > NAME_LENGTH) throw std::system_error(EINVAL, std::generic_category());
    this->name = std::string(*name);
    this->size = 0;
    this->data = new char[0];
    this->uid = uid;
    this->gid = gid;
    this->mode = mode;
    setATime();
    setMTime();
    setCTime();
}

File::~File() {
    delete[] data;
}

/// Copy constructor for the file class.
/// \param other [in] Another file to copy from.
File::File(const File &other) {
    name = std::string(other.name);
    size = other.size;
    uid = other.uid;
    gid = other.gid;
    mode = other.mode;
    atime = other.atime;
    mtime = other.mtime;
    ctime = other.ctime;
    open = other.open;

    data = new char[size];
    std::memcpy(data, other.data, size);
}

/// Change the path to the file.
/// \param name [in] New path to file.
/// \returns 0 on success, -EINVAL If the length of the file path exceeds NAME_LENGTH from myfs-structs.h.
int File::setName(std::string *name) {
    if (name->size() > NAME_LENGTH) return -EINVAL;
    this->name = std::string(*name);
    setMTime();
    return 0;
}

/// Change the size of the data block.
/// \param size [in] New data size.
/// \returns 0 on success
int File::setSize(off_t size) {
    if (size < 0) return -EINVAL;
    this->size = size;
    this->data = (char*) std::realloc(this->data, this->size);
    setMTime();
    return 0;
}

/// Change the user identification.
/// \param uid [in] New user id.
/// \returns 0 on success
int File::setUserID(uid_t uid) {
    this->uid = uid;
    setCTime();
    return 0;
}

/// Change the group identification.
/// \param gid [in] New group id.
/// \returns 0 on success
int File::setGroupID(gid_t gid) {
    this->gid = gid;
    setCTime();
    return 0;
}

/// Change the permissions for file access.
/// \param mode [in] New permissions.
/// \returns 0 on success
int File::setMode(mode_t mode) {
    this->mode = mode;
    setCTime();
    return 0;
}

/// Update the time of last access to current time.
/// \returns 0 on success
int File::setATime() {
    this->atime = std::time(nullptr);
    return 0;
}

/// Update the time of last change to current time.
/// \returns 0 on success
int File::setMTime() {
    this->mtime = std::time(nullptr);
    return 0;
}

/// Update the time of last status change to current time.
/// \returns 0 on success
int File::setCTime() {
    this->ctime = std::time(nullptr);
    return 0;
}

/// Open the file.
/// \returns 0 on success, -EINVAL If the file is already open.
int File::setOpen() {
    if (this->open) return -EINVAL;
    this->open = true;
    setATime();
    return 0;
}

/// Close the file.
/// \returns 0 on success, -EINVAL If the file is already closed.
int File::setClose() {
    if (!this->open) return -EBADF;
    this->open = false;
    return 0;
}

/// Get the file path.
/// \param [out] name pointer containing file name
/// \returns 0 on success
int File::getName(std::string *name) {
    *name = std::string(this->name);
    return 0;
}

/// Get the data size.
/// \param [out] size pointer containing file size
/// \returns 0 on success
int File::getSize(off_t* size) {
    *size = this->size;
    return 0;
}

/// Get the user id.
/// \param [out] uid pointer containing user identification
/// \returns 0 on success
int File::getUserID(uid_t* uid) {
    *uid = this->uid;
    return 0;
}

/// Get the group id.
/// \param [out] gid pointer containing group identification
/// \returns 0 on success
int File::getGroupID(gid_t* gid) {
    *gid = this->gid;
    return 0;
}

/// Get the permissions for file access.
/// \param [out] mode pointer containing user mode
/// \returns 0 on success
int File::getMode(mode_t* mode) {
    *mode = this-> mode;
    return 0;
}

/// Get the time of last access.
/// \param [out] atime pointer containing time of last access
/// \returns 0 on success
int File::getATime(std::time_t* atime) {
    *atime = this->atime;
    return 0;
}

/// Get the time of last change.
/// \param [out] mtime pointer containing time of last change
/// \returns 0 on success
int File::getMTime(std::time_t* mtime) {
    *mtime = this->mtime;
    return 0;
}

/// Get the time of last status change.
/// \param [out] ctime pointer containing last status change
/// \returns 0 on success
int File::getCTime(std::time_t* ctime) {
    *ctime = this->ctime;
    return 0;
}

/// Get whether the file is open.
/// \returns Boolean value if file was already opened.
bool File::isOpen() {
    return this->open;
}

/// Override a part of the data block with new data.
/// \param size [in] Size of the new data.
/// \param data [in] Pointer to the new data.
/// \param offset [in] Offset to the location to write the data to.
/// \returns 0 on success, -EINVAL If size negative, -EBADF If file is not open
int File::write(off_t size, const char* data, off_t offset) {
    if (!this->open) {return -EBADF;}
    if (size < 0) return -EINVAL;
    if (size + offset > this->size) {
        setSize(size + offset);
    }
    std::memcpy(this->data + offset, data, size);
    setMTime();
    return size;
}

/// Get a pointer to the data with offset.
/// \param offset [in] Offset from the beginning of the data.
/// \param data [out] Pointer to the requested data.
/// \returns 0 on success, -EINVAL If offset is greater than existing data size, -EBADF If file is not open
int File::getData(off_t offset, char* data, off_t size) {
    if (!this->open) {return -EBADF;}
    if (offset > this->size) return -EINVAL;
    std::memcpy(data, this->data + offset, size);
    setATime();
    return size;
}
/// Get the metadata of a file (user & group id, modification times, permissions, ...).
/// \param [out] statbuf Structure containing the meta data, for details type "man 2 stat" in a terminal.
/// \returns 0 on success
int File::getMetadata(struct stat *statbuf) {
    getUserID(&statbuf->st_uid);
    getGroupID(&statbuf->st_gid);
    getATime(&statbuf->st_atime);
    getMTime(&statbuf->st_mtime);
    getMode(&statbuf->st_mode);
    getSize(&statbuf->st_size);
    statbuf->st_nlink = 1;  //Set amount of hard links to file to 1 for now.
    return 0;
}
