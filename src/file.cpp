//
// Created by lukas on 08.10.21.
//

#include <file.h>
#include <cstring>
#include "myfs-structs.h"


//TODO Constructor

File::File(char *name, size_t size, char *data, int userID, int mode) {
    //initialize name_size
    this->name_size = std::strlen(name);

    //initialize name (deep copy)
    if (this->name_size <= NAME_LENGTH) {
        deepCopy(name, this->name, name_size + 1);
        name[name_size] = '\0';
    } else {
        //TODO ERROR
    }

    //initialize size
    this->size = size; //TODO as bytes?

    //initialize data
    deepCopy(data, this->data, size);

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

void File::deepCopy(char *from, char *to, size_t pSize) {
    to = new char[pSize];
    for(int i = 0; i < pSize; i++, from++, to++) {
        *to = *from;
    }
}


//TODO file methods here
void File::copy() {
    //TODO
}