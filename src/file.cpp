//
// Created by lukas on 08.10.21.
//

#include "file.h"

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

void File::setName(char *name) {
    unsigned short nameSize = std::strlen(name);
    if (nameSize > NAME_LENGTH) throw std::system_error(EINVAL, std::generic_category(), "File name too long");
    delete[] name;
    this->nameSize = nameSize;
    this->name = new char[this->nameSize];
    std::memcpy(this->name, name, this->nameSize + 1);
}

void File::setSize(size_t size) {
    this->size = size;
    std::realloc(this->data, this->size);
}

void File::setUserID(int userID) {
    this->userID = userID;
}

void File::setMode(int mode) {
    this->mode = mode;
}

void File::setATime() {
    this->atime = std::time(nullptr);
}

void File::setMTime() {
    this->mtime = std::time(nullptr);
}

void File::setCTime() {
    this->ctime = std::time(nullptr);
}

void File::setOpen() {
    if (this->open) throw std::system_error(EINVAL, std::generic_category(), "File already opened");
    this->open = true;
}

void File::setClose() {
    if (!this->open) throw std::system_error(EINVAL, std::generic_category(), "File already closed");
    this->open = false;
}

char* File::getName() {
    return name;
}

size_t File::getSize() {
    return size;
}

int File::getUserID() {
    return userID;
}

int File::getMode() {
    return mode;
}

std::time_t File::getATime() {
    return atime;
}

std::time_t File::getMTime() {
    return mtime;
}

std::time_t File::getCTime() {
    return ctime;
}

bool File::isOpen() {
    return open;
}

void File::append(size_t size, char* data) {
    size_t oldSize = this->size;
    setSize(size);
    std::memcpy(this->data + oldSize, data, size);
}

void File::insert(size_t size, char* data, off_t offset) {
    if (offset > this->size) throw std::system_error(EINVAL, std::generic_category(), "Offset needs to be smaller or equal to data size");
    setSize(this->size + size);
    std::memcpy(this->data + offset + size, this->data + offset, size);
    std::memcpy(this->data + offset, data, size);
}

char* File::getData(off_t offset) {
    return this->data + offset;
}
