//
// Created by lukas on 08.10.21.
//

#include "file.h"

File::File(char *name, size_t size, char *data, int userID, int mode) {
    //initialize name_size
    this->nameSize = std::strlen(name);     //TODO Do we want to include '\0'

    //initialize name
    if (this->nameSize > NAME_LENGTH) throw std::system_error(EINVAL
                                                              , std::generic_category()
                                                              , "File name too long");
    this->name = new char[this->nameSize + 1];
    std::memcpy(this->name, name, this->nameSize + 1);

    //initialize size
    this->size = size;

    //initialize data
    std::memcpy(this->data, data, size);

    //initialize userID
    this->userID = userID;

    //initialize mode
    this->mode = mode;

    //initialize times
    this->atime = std::time(nullptr);
    this->mtime = std::time(nullptr);
    this->ctime = std::time(nullptr);
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
