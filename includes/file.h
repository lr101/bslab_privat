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
    unsigned short nameSize;    ///< Current path length excluding null character
    size_t size;                ///< Size of data block
    int userID;                 ///< User identifier
    int mode;                   ///< Access authorizations
    std::time_t atime;          ///< Time of last access
    std::time_t mtime;          ///< Time of last change
    std::time_t ctime;          ///< Time of last status change
    char* data;                 ///< File content
    bool open = false;          ///< True if file is open

public:
    File(char* name, size_t size, char* data, int userID, int mode);
    ~File();
    File(const File&);

    std::time_t setAtime();
    std::time_t setMtime();
    std::time_t setCtime();
};

#endif //MYFS_FILE_H