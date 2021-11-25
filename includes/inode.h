//
// Created by lukas on 08.10.21.
//

#pragma once

#include <ctime>
#include <cstring>
#include <errno.h>
#include <system_error>
#include <sys/stat.h>

#include "myfs-structs.h"
#include "blockdevice.h"

#define N_BLOCKS 15;

/**
 * TODO:
 * name size check
 * (or check is last char is '\0')
 * store name size? no?
 *
 * block list
 * 12? indirekt pointer, 3? indirekt
 * more level 2 indirekt pointer or one level 3 indirekt pointer
 *
 */

class Inode {
    size_t blocks;              ///< Amount of used blocks
    size_t bytes;               ///< Amount of used bytes in the last block
    uid_t uid;                  ///< User identifier
    gid_t gid;                  ///< Group identifier
    mode_t mode;                ///< Permissions for file access
    time_t atime;               ///< Time of last access
    time_t mtime;               ///< Time of last change
    time_t ctime;               ///< Time of last status change
    bool open = false;          ///< True if file is open
    char* name;                 ///< Name of file
    size_t block[N_BLOCKS];     ///< Block List, pointer to either blocks or more pointer

    int setATime();
    int setMTime();
    int setCTime();
public:
    Inode(std::string *name, uid_t uid, gid_t gid, mode_t mode);
    ~Inode();
    Inode(const Inode&);


    int setName(std::string*);
    int setSize(off_t);
    int setUserID(uid_t);
    int setGroupID(gid_t);
    int setMode(mode_t);
    int setOpen();
    int setClose();
    int write(off_t, const char*, off_t);

    int getName(std::string*);
    int getSize(off_t*);
    int getUserID(uid_t*);
    int getGroupID(gid_t*);
    int getMode(mode_t*);
    int getATime(std::time_t*);
    int getMTime(std::time_t*);
    int getCTime(std::time_t*);
    bool isOpen();
    int getData(off_t, char*, off_t);
    int getMetadata(struct stat*);

    int saveToBlockDevice(InodePointer*);
};

