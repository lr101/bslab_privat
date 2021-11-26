//
// Created by lukas on 08.10.21.
//

#pragma once

#include <ctime>
#include <cstring>
#include <errno.h>
#include <system_error>
#include <sys/stat.h>
#include <vector>

#include "myfs-structs.h"
#include "superblock.h"

#define DIR_BLOCK 4
#define IND_BLOCK 4
#define DIND_BLOCK 2
#define N_BLOCKS (DIR_BLOCK + IND_BLOCK + DIND_BLOCK)
#define N_BLOCK_PTR (BLOCK_SIZE / sizeof(index_t))
#define BLOCK_PTR_BITS 7    //7 bits to address 0 to 127
#define BLOCK_PTR_BIT_MASK (N_BLOCK_PTR - 1)

class Inode {
    size_t size;                ///< Size in bytes of the entire data part
    uid_t uid;                  ///< User identifier
    gid_t gid;                  ///< Group identifier
    mode_t mode;                ///< Permissions for file access
    time_t atime;               ///< Time of last access
    time_t mtime;               ///< Time of last change
    time_t ctime;               ///< Time of last status change
    bool open = false;          ///< True if file is open
    char* name;                 ///< Name of file
    index_t block[N_BLOCKS];    ///< Block List, pointer to either blocks or more pointer

    int setATime();
    int setMTime();
    int setCTime();

    int getBlockList(InodePointer*, off_t, off_t, std::vector<index_t>*);
public:
    Inode(std::string *name, uid_t uid, gid_t gid, mode_t mode);
    ~Inode();
    Inode(const Inode&);


    int setName(InodePointer*, std::string*);
    int setSize(InodePointer*, off_t);
    int setUserID(InodePointer*, uid_t);
    int setGroupID(InodePointer*, gid_t);
    int setMode(InodePointer*, mode_t);
    int setOpen();
    int setClose();
    int write(InodePointer*, off_t, const char*, off_t);

    int getName(std::string*);
    int getSize(off_t*);
    int getUserID(uid_t*);
    int getGroupID(gid_t*);
    int getMode(mode_t*);
    int getATime(std::time_t*);
    int getMTime(std::time_t*);
    int getCTime(std::time_t*);
    bool isOpen();
    int getData(InodePointer*, off_t, char*, off_t);
    int getMetadata(struct stat*);
};

