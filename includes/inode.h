//
// cReATEd bY LuKaS On 08.10.21.
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
    Superblock* s_block;

    int setATime();
    int setMTime();
    int setCTime();

    int getBlockList(off_t size, off_t offset, std::vector<index_t> *blockList);
public:
    Inode(Superblock* s_block, const char *name, uid_t uid, gid_t gid, mode_t mode);
    ~Inode();

    int setName(const char *name);
    int setSize(off_t size);
    int setUserID(uid_t uid);
    int setGroupID(gid_t gid);
    int setMode(mode_t mode);
    int setOpen();
    int setClose();
    int write(off_t size, const char *data, off_t offset);

    int getName(const char *name);
    int getSize(off_t *size);
    int getUserID(uid_t *uid);
    int getGroupID(gid_t *gid);
    int getMode(mode_t *mode);
    int getATime(std::time_t *atime);
    int getMTime(std::time_t *mtime);
    int getCTime(std::time_t *ctime);
    bool isOpen();
    int getData(off_t offset, char *data, off_t size);
    int getMetadata(struct stat *statbuf);
};

