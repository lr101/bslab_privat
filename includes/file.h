//
// Created by lukas on 08.10.21.
//

#pragma once

#include <ctime>
#include <cstring>
#include <errno.h>
#include <system_error>

#include "myfs-structs.h"

class File {
    char* name;                 ///< Path to file
    size_t nameSize;            ///< Current path length excluding null character
    size_t size;                ///< Size of data block
    uid_t st_uid;               ///< User identifier
    gid_t st_gid;               ///< Group identifier
    mode_t st_mode;             ///< Permissions for file access
    std::time_t atime;          ///< Time of last access
    std::time_t mtime;          ///< Time of last change
    std::time_t ctime;          ///< Time of last status change
    char* data;                 ///< File content
    bool open = false;          ///< True if file is open

public:
    File(char* name, size_t size, char* data, uid_t st_uid, gid_t st_gid, mode_t st_mode);
    ~File();
    File(const File&);

    int setName(char*);
    int setSize(size_t);
    int setUserID(uid_t);
    int setGroupID(gid_t);
    int setMode(mode_t);
    int setATime();
    int setMTime();
    int setCTime();
    int setOpen();
    int setClose();
    int append(size_t size, char* data);
    int write(size_t size, char* data, off_t offset);

    int getName(char*);
    int getSize(size_t*);
    int getUserID(uid_t*);
    int getGroupID(gid_t*);
    int getMode(mode_t*);
    int getATime(std::time_t*);
    int getMTime(std::time_t*);
    int getCTime(std::time_t*);
    int isOpen(bool*);
    int getData(off_t offset, char*);
    int getMetadata(struct stat *statbuf);
};
