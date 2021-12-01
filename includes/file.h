//
// Created by lukas on 08.10.21.
//

#ifndef file_h
#define file_h

#include <ctime>
#include <cstring>
#include <errno.h>
#include <system_error>
#include <sys/stat.h>

#include "myfs-structs.h"

class File {
    std::string name;           ///< Path to file
    off_t size;                 ///< Size of data block
    uid_t uid;                  ///< User identifier
    gid_t gid;                  ///< Group identifier
    mode_t mode;                ///< Permissions for file access
    std::time_t atime;          ///< Time of last access
    std::time_t mtime;          ///< Time of last change
    std::time_t ctime;          ///< Time of last status change
    char* data;                 ///< File content
    bool open = false;          ///< True if file is open

    int setATime();
    int setMTime();
    int setCTime();
public:
    File(std::string *name, uid_t uid, gid_t gid, mode_t mode);
    ~File();
    File(const File&);


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
};

#endif /* file_h */