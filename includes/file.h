//
// Created by lukas on 08.10.21.
//

#ifndef MYFS_FILE_H
#define MYFS_FILE_H

#include <ctime>
#include <cstring>
#include <errno.h>
#include <system_error>

#include "myfs-structs.h"

class File {
    char* name;                 ///< Path to file
    unsigned short nameSize;
    size_t size;
    int userID;
    int mode;
    std::time_t atime;
    std::time_t mtime;
    std::time_t ctime;
    char* data;
    bool open = false;

public:
    File(char* name, size_t size, char* data, int userID, int mode);
    ~File();
    File(const File&);
    File& operator=(const File&);

};

#endif //MYFS_FILE_H