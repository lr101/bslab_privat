//
// Created by lukas on 08.10.21.
//

#include "file.h"

/// Create a new file.
/// \param name Path of the file. Can't exceed NAME_LENGTH.
/// \param size Size of data block.
/// \param data Data to be stored.
/// \param userID User identification.
/// \param mode Permissions for file access.
/// \throws EINVAL If the length of the file path exceeds NAME_LENGTH from myfs-structs.h.
File::File(char *name, size_t size, char *data, int userID, int mode) {
    this->nameSize = std::strlen(name);
    if (this->nameSize > NAME_LENGTH) throw std::system_error(EINVAL, std::generic_category(), "File name too long");
    this->name = new char[this->nameSize + 1];
    std::memcpy(this->name, name, this->nameSize + 1);

    this->size = size;
    this->data = new char[this->size];
    std::memcpy(this->data, data, size);

    this->userID = userID;
    this->mode = mode;
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
    userID = other.userID;
    mode = other.mode;
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
/// \throws EINVAL If the length of the file path exceeds NAME_LENGTH from myfs-structs.h.
void File::setName(char *name) {
    size_t nameSize = std::strlen(name);
    if (nameSize > NAME_LENGTH) throw std::system_error(EINVAL, std::generic_category(), "File name too long");
    delete[] name;
    this->nameSize = nameSize;
    this->name = new char[this->nameSize];
    std::memcpy(this->name, name, this->nameSize + 1);
}

/// Change the size of the data block.
/// \param size New data size.
void File::setSize(size_t size) {
    this->size = size;
    std::realloc(this->data, this->size);
}

/// Change the user identification.
/// \param userID New user id.
void File::setUserID(int userID) {
    this->userID = userID;
}

/// Change the permissions for file access.
/// \param mode New permissions.
void File::setMode(int mode) {
    this->mode = mode;
}

/// Update the time of last access to current time.
void File::setATime() {
    this->atime = std::time(nullptr);
}

/// Update the time of last change to current time.
void File::setMTime() {
    this->mtime = std::time(nullptr);
}

/// Update the time of last status change to current time.
void File::setCTime() {
    this->ctime = std::time(nullptr);
}

/// Open the file.
/// \throws EINVAL If the file is already open.
void File::setOpen() {
    if (this->open) throw std::system_error(EINVAL, std::generic_category(), "File already opened");
    this->open = true;
}

/// Close the file.
/// \throws EINVAL If the file is already closed.
void File::setClose() {
    if (!this->open) throw std::system_error(EINVAL, std::generic_category(), "File already closed");
    this->open = false;
}

/// Get the file path.
/// \return Pointer to name.
char* File::getName() {
    return name;
}

/// Get the data size.
/// \return Size of data block.
size_t File::getSize() {
    return size;
}

/// Get the user id.
/// \return User identification.
int File::getUserID() {
    return userID;
}

/// Get the permissions for file access.
/// \return File permissions.
int File::getMode() {
    return mode;
}

/// Get the time of last access.
/// \return Last access.
std::time_t File::getATime() {
    return atime;
}

/// Get the time of last change.
/// \return Last change.
std::time_t File::getMTime() {
    return mtime;
}

/// Get the time of last status change.
/// \return Last status change.
std::time_t File::getCTime() {
    return ctime;
}

/// Get whether the file is open.
/// \return True if file is open, otherwise false.
bool File::isOpen() {
    return open;
}

/// Append a new data block to the existing one.
/// \param size Size of the new data.
/// \param data Pointer to the new data.
void File::append(size_t size, char* data) {
    size_t oldSize = this->size;
    setSize(size);
    std::memcpy(this->data + oldSize, data, size);
}

/// Insert a new data block at any point of the existing data.
/// \param size Size of the new data.
/// \param data Pointer to the new data.
/// \param offset Point at which new data is to be inserted.
/// \throws EINVAL If offset is greater than existing data size.
void File::insert(size_t size, char* data, off_t offset) {
    if (offset > this->size) throw std::system_error(EINVAL, std::generic_category(), "Offset can't be greater than data size");
    setSize(this->size + size);
    std::memcpy(this->data + offset + size, this->data + offset, size);
    std::memcpy(this->data + offset, data, size);
}

/// Get a pointer to the data with offset.
/// \param offset Offset from the beginning of the data.
/// \return Pointer to the specified part of the data.
/// \throws EINVAL If offset is greater than existing data size.
char* File::getData(off_t offset) {
    if (offset > this->size) throw std::system_error(EINVAL, std::generic_category(), "Offset can't be greater than data size");
    return this->data + offset;
}
