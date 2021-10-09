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

    void setName(char*);
    void setSize(size_t);
    void setUserID(int);
    void setMode(int);
    void setATime();
    void setMTime();
    void setCTime();
    void setOpen();
    void setClose();

    char* getName();
    size_t getSize();
    int getUserID();
    int getMode();
    std::time_t getATime();
    std::time_t getMTime();
    std::time_t getCTime();
    bool isOpen();
};

#endif //MYFS_FILE_H