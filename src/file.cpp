//
// Created by lukas on 08.10.21.
//

#include <bits/stat.h>
#include "file.h"

/// Create a new file.
/// \param name Path of the file. Can't exceed NAME_LENGTH.
/// \param size Size of data block.
/// \param data Data to be stored.
/// \param st_uid User identification.
/// \param st_gid Group identification.
/// \param st_mode Permissions for file access.
/// \throws EINVAL If the length of the file path exceeds NAME_LENGTH from myfs-structs.h.
File::File(char *name, size_t size, char *data, uid_t st_uid, gid_t st_gid, mode_t st_mode) {
    this->nameSize = std::strlen(name);
    if (this->nameSize > NAME_LENGTH) throw std::system_error(EINVAL, std::generic_category(), "File name too long");
    this->name = new char[this->nameSize + 1];
    std::memcpy(this->name, name, this->nameSize + 1);

    this->size = size;
    this->data = new char[this->size];
    std::memcpy(this->data, data, size);

    this->st_uid = st_uid;
    this->st_gid = st_gid;
    this->st_mode = st_mode;
    setATime();
    setMTime();
    setCTime();
}

File::~File() {
    delete[] data;
    delete[] name;
}

/// Copy constructor for the file class.
/// \param other Another file to copy from.
File::File(const File &other) {
    nameSize = other.nameSize;
    size = other.size;
    st_uid = other.st_uid;
    st_gid = other.st_gid;
    st_mode = other.st_mode;
    atime = other.atime;
    mtime = other.mtime;
    ctime = other.ctime;
    open = other.open;

    name = new char[nameSize];
    std::memcpy(name, other.name, nameSize);

    data = new char[size];
    std::memcpy(data, other.data, size);
}

/// Change the path to the file.
/// \param name New path to file.
/// \returns 0 on success, -EINVAL If the length of the file path exceeds NAME_LENGTH from myfs-structs.h.
int File::setName(char *name) {
    size_t nameSize = std::strlen(name);
    if (nameSize > NAME_LENGTH) return -EINVAL;
    delete[] this->name;
    this->nameSize = nameSize;
    this->name = new char[this->nameSize];
    std::memcpy(this->name, name, this->nameSize + 1);
    setMTime();
    return 0;
}

/// Change the size of the data block.
/// \param size New data size.
/// \returns 0 on success
int File::setSize(size_t size) {
    this->size = size;
    std::realloc(this->data, this->size);
    setMTime();
    return 0;
}

/// Change the user identification.
/// \param st_uid New user id.
/// \returns 0 on success
int File::setUserID(uid_t st_uid) {
    this->st_uid = st_uid;
    setCTime();
    return 0;
}

/// Change the group identification.
/// \param st_gid New group id.
/// \returns 0 on success
int File::setGroupID(gid_t st_gid) {
    this->st_gid = st_gid;
    setCTime();
    return 0;
}

/// Change the permissions for file access.
/// \param st_mode New permissions.
/// \returns 0 on success
int File::setMode(mode_t st_mode) {
    this->st_mode = st_mode;
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
    if (!this->open) return -EINVAL;
    this->open = false;
    return 0;
}

/// Get the file path.
/// \param [out] name pointer containing file name
/// \returns 0 on success
int File::getName(char* name) {
    name = new char [this->nameSize + 1];
    std::strcpy(name, this->name);
    return 0;
}

/// Get the data size.
/// \param [out] size pointer containing file size
/// \returns 0 on success
int File::getSize(size_t* size) {
    *size = this->size;
    return 0;
}

/// Get the user id.
/// \param [out] st_uid pointer containing user identification
/// \returns 0 on success
int File::getUserID(uid_t* st_uid) {
    *st_uid = this->st_uid;
    return 0;
}

/// Get the group id.
/// \param [out] st_gid pointer containing group identification
/// \returns 0 on success
int File::getGroupID(gid_t* st_gid) {
    *st_gid = this->st_gid;
    return 0;
}

/// Get the permissions for file access.
/// \param [out] st_mode pointer containing user mode
/// \returns 0 on success
int File::getMode(mode_t* st_mode) {
    *st_mode = this-> st_mode;
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
/// \param [out] open pointer containing whether file is open (true = open, false = closed)
/// \returns 0 on success
int File::isOpen(bool* open) {
    *open = this->open;
    return 0;
}

/// Append a new data block to the existing one.
/// \param size Size of the new data.
/// \param data Pointer to the new data.
/// \returns 0 on success
int File::append(size_t size, char* data) {
    size_t oldSize = this->size;
    setSize(this->size + size);
    std::memcpy(this->data + oldSize, data, size);
    setMTime();
    return 0;
}

/// Override a part of the data block with new data.
/// \param size Size of the new data.
/// \param data Pointer to the new data.
/// \param offset Offset to the location to write the data to.
int File::write(size_t size, const char* data, off_t offset) {
    if (offset > this->size) return -EINVAL;
    if (size + offset > this->size) {
        setSize(size + offset);
    }
    std::memcpy(this->data + offset, data, size);
    setMTime();
    return 0;
}

/// Get a pointer to the data with offset.
/// \param offset Offset from the beginning of the data.
/// \return Pointer to the specified part of the data.
/// \returns 0 on success, -EINVAL If offset is greater than existing data size.
int File::getData(off_t offset, char* data) {
    if (offset > this->size) return -EINVAL;
    *data = *(this->data + offset);
    setATime();
    return 0;
}
/// Get the metadata of a file (user & group id, modification times, permissions, ...).
/// \param [out] statbuf Structure containing the meta data, for details type "man 2 stat" in a terminal.
/// \returns 0 on success
int File::getMetadata(struct stat *statbuf) {
    getUserID(statbuf->st_uid);
    getGroupID(statbuf->st_gid);
    getATime(statbuf->st_atime);
    getMTime(statbuf->st_mtime);
    getMode(statbuf->st_mode);
    getSize(statbuf->st_size);
    statbuf->st_nlink = 1;  //Set amount of hard links to file to 1 for now.
    return 0;
}
