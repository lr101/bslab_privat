//
// Created by Oliver Waldhorst on 20.03.20.
// Copyright © 2017-2020 Oliver Waldhorst. All rights reserved.

#include "myondiskfs.h"

// For documentation of FUSE methods see https://libfuse.github.io/doxygen/structfuse__operations.html

#undef DEBUG

// TODO: Comment lines to reduce debug messages
#define DEBUG
#define DEBUG_METHODS
#define DEBUG_RETURN_VALUES

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "macros.h"
#include "myfs.h"
#include "myfs-info.h"
#include "blockdevice.h"
#include "inode.h"

/// @brief Constructor of the on-disk file system class.
///
/// You may add your own constructor code here.
MyOnDiskFS::MyOnDiskFS() : MyFS() {
    // create a block device object
    this->blockDevice= new BlockDevice(BLOCK_SIZE);
    // TODO: [PART 2] Add your constructor code here

}

/// @brief Destructor of the on-disk file system class.
///
/// You may add your own destructor code here.
MyOnDiskFS::~MyOnDiskFS() {
    // free block device object
    delete this->blockDevice;
    for (auto const& item : files) {
        delete item.second;
    }

}

/// @brief Create a new file.
///
/// Create a new file with given name and permissions.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] mode Permissions for file access.
/// \param [in] dev Can be ignored.
/// \return 0 on success, -ERRNO on failure.



int MyOnDiskFS::fuseMknod(const char *path, mode_t mode, dev_t dev) {
    LOGM();
    LOGF("Attributes: path=%s, mode=%u", path, mode);
    char buf[BLOCK_SIZE];
    if (this->files.find(path) == this->files.end() && this->files.size() < NUM_DIR_ENTRIES) {
        std::string newName = path;
        try {
            Inode * inode = new Inode(&newName, getuid(), getgid(), mode);
            char* buf = new char[BLOCK_SIZE];
            this->blockDevice->read(this->s_block->getIMapIndex(), buf);
            int index = this->s_block->getFreeInodeIndex(buf);
            buf = this->s_block->flipBitInNode(index,buf);
            auto *ip = new struct InodePointer();
            ip->inode = inode;
            ip->blockDevice = this->blockDevice;
            ip->blockNo = this->s_block->getINodeIndex() + index;
            this->files[path] = ip;

            // write inode to blockdevice
            int ret = writeInode(ip);
            if (ret != 0) { RETURN(ret);}
        } catch (const std::exception &e) {
            LOGF("Error creating new file: %s", e.what());
            RETURN(-EINVAL);
        }
    } else if (this->files.size() >= NUM_DIR_ENTRIES) {
        RETURN(-ENOSPC);
    } else {
        RETURN(-EEXIST);
    }

    RETURN(0);
}



/// @brief Delete a file.
///
/// Delete a file with given name from the file system.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseUnlink(const char *path) {
    LOGM();
    LOGF("Attributes: path=%s", path);

    auto itPath = this->files.find(path);

    if (itPath != this->files.end()) {
        this->files.erase(itPath);
        // TODO: free inode's block
        RETURN(0);
    } else {
        RETURN(-ENOENT);
    }
}

/// @brief Rename a file.
///
/// Rename the file with with a given name to a new name.
/// Note that if a file with the new name already exists it is replaced (i.e., removed
/// before renaming the file.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] newpath  New name of the file, starting with "/".
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseRename(const char *path, const char *newpath) {
    LOGM();
    LOGF("Attributes: path=%s, newpath=%s", path, newpath);

    auto itPath = this->files.find(path);

    if (itPath != this->files.end()) {
        auto const value = std::move(itPath->second);
        this->files.erase(itPath);
        std::string newPathString = std::string(newpath);
        this->files.insert({newPathString, std::move(value)});
        InodePointer* ip = this->files.find(newpath)->second;
        ip->inode->setName(&newPathString); // TODO: exception handling
        RETURN(writeInode(ip));
    } else {
        RETURN(-ENOENT);
    }
}

/// @brief Get file meta data.
///
/// Get the metadata of a file (user & group id, modification times, permissions, ...).
/// \param [in] path Name of the file, starting with "/".
/// \param [out] statbuf Structure containing the meta data, for details type "man 2 stat" in a terminal.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseGetattr(const char *path, struct stat *statbuf) {
    LOGM();
    LOGF("Attributes: path=%s", path);

    // GNU's definitions of the attributes (http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
    // 		st_uid: 	The user ID of the file’s owner.
    //		st_gid: 	The group ID of the file.
    //		st_atime: 	This is the last access time for the file.
    //		st_mtime: 	This is the time of the last modification to the contents of the file.
    //		st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and
    //		            the file permission bits (see Permission Bits).
    //		st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have
    //	             	entries for this file. If the count is ever decremented to zero, then the file itself is
    //	             	discarded as soon as no process still holds it open. Symbolic links are not counted in the
    //	             	total.
    //		st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field
    //		            isn’t usually meaningful. For symbolic links this specifies the length of the file name the link
    //		            refers to.

    auto itPath = this->files.find(path);
    int ret = 0;

    if (strcmp(path, "/") == 0) {
        statbuf->st_uid = getuid();
        statbuf->st_gid = getgid();
        statbuf->st_atime = time(NULL);
        statbuf->st_mtime = time(NULL);
        statbuf->st_ctime = time(NULL);
        statbuf->st_mode = S_IFDIR | 0755;
        statbuf->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
    } else if (itPath != this->files.end()) {
        ret = itPath->second->inode->getMetadata(statbuf);
    } else
        ret = -ENOENT;
    if (ret >= 0) {
        LOGF("Return Attribute: userID=%d", statbuf->st_uid);
        LOGF("Return Attribute: groupID=%d", statbuf->st_gid);
        LOGF("Return Attribute: aTime=%ld", statbuf->st_atime);
        LOGF("Return Attribute: cTime=%ld", statbuf->st_ctime);
        LOGF("Return Attribute: mTime=%ld", statbuf->st_mtime);
        LOGF("Return Attribute: mode=%d", statbuf->st_mode);
        LOGF("Return Attribute: nLink=%lu", statbuf->st_nlink);
    }
    RETURN(ret);
}

/// @brief Change file permissions.
///
/// Set new permissions for a file.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] mode New mode of the file.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseChmod(const char *path, mode_t mode) {
    LOGM();
    LOGF("Attributes: path=%s, mode=%d", path, mode);

    auto itPath = this->files.find(path);

    if (itPath != this->files.end()) {
        itPath->second->inode->setMode(mode); //TODO: exception handling
        RETURN(writeInode(itPath->second));
    } else {
        RETURN(-ENOENT);
    }
}

/// @brief Change the owner of a file.
///
/// Change the user and group identifier in the meta data of a file.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] uid New user id.
/// \param [in] gid New group id.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseChown(const char *path, uid_t uid, gid_t gid) {
    LOGM();
    LOGF("Attributes: path=%s, userID=%u, groupID=%u", path, uid, gid);

    auto itPath = this->files.find(path);
    int ret;

    if (itPath != this->files.end()) {
        ret = itPath->second->inode->setUserID(uid);
        ret |= itPath->second->inode->setGroupID(gid);
    } else {
        ret = -ENOENT;
    }
    if (ret == 0) {
        ret = writeInode(itPath->second);
    }

    RETURN(ret);
}

/// @brief Open a file.
///
/// Open a file for reading or writing. This includes checking the permissions of the current user and incrementing the
/// open file count.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [out] fileInfo Can be ignored in Part 1
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("Attributes: path=%s", path);

    auto itPath = this->files.find(path);
    int ret;

    if (itPath != this->files.end()) {
        ret = itPath->second->inode->setOpen();
    } else {
        ret = -ENOENT;
    }
    if (ret == 0) { ret = writeInode(itPath->second); }

    RETURN(ret);
}

/// @brief Read from a file.
///
/// Read a given number of bytes from a file starting form a given position.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// Note that the file content is an array of bytes, not a string. I.e., it is not (!) necessarily terminated by '\0'
/// and may contain an arbitrary number of '\0'at any position. Thus, you should not use strlen(), strcpy(), strcmp(),
/// ... on both the file content and buf, but explicitly store the length of the file and all buffers somewhere and use
/// memcpy(), memcmp(), ... to process the content.
/// \param [in] path Name of the file, starting with "/".
/// \param [out] buf The data read from the file is stored in this array. You can assume that the size of buffer is at
/// least 'size'
/// \param [in] size Number of bytes to read
/// \param [in] offset Starting position in the file, i.e., number of the first byte to read relative to the first byte of
/// the file
/// \param [in] fileInfo Can be ignored in Part 1
/// \return The Number of bytes read on success. This may be less than size if the file does not contain sufficient bytes.
/// -ERRNO on failure.
int MyOnDiskFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("Attributes: path=%s, offset=%lu, size=%lu, fileInfo=%s", path, offset, size, "Ignored in Part1");

    auto itPath = this->files.find(path);

    if (itPath != this->files.end()) {
        RETURN(itPath->second->inode->getData(offset, buf, size));
    } else {
        RETURN(-ENOENT);
    }
}

/// @brief Write to a file.
///
/// Write a given number of bytes to a file starting at a given position.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// Note that the file content is an array of bytes, not a string. I.e., it is not (!) necessarily terminated by '\0'
/// and may contain an arbitrary number of '\0'at any position. Thus, you should not use strlen(), strcpy(), strcmp(),
/// ... on both the file content and buf, but explicitly store the length of the file and all buffers somewhere and use
/// memcpy(), memcmp(), ... to process the content.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] buf An array containing the bytes that should be written.
/// \param [in] size Number of bytes to write.
/// \param [in] offset Starting position in the file, i.e., number of the first byte to read relative to the first byte of
/// the file.
/// \param [in] fileInfo Can be ignored in Part 1 .
/// \return Number of bytes written on success, -ERRNO on failure.
int MyOnDiskFS::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("Attributes: path=%s, offset=%lu, size=%lu, fileInfo=%s", path, offset, size, "Ignored in Part1");
    int ret;

    auto curFile = files.find(path);
    if (curFile == files.end()) {RETURN(-ENOENT);}
    ret = curFile->second->inode->write(size, buf, offset);
    if (ret == 0) {
        // update Inode on disk. Why? MTime changes after write operation
        ret = writeInode(curFile->second);
    }
    RETURN(ret);
}

/// @brief Close a file.
///
/// \param [in] path Name of the file, starting with "/".
/// \param [in] File handel for the file set by fuseOpen.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseRelease(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("Attributes: path=%s, fileInfo=%s", path, "Ignored in Part1");
    auto curFile = files.find(path);
    if (curFile == files.end()) {RETURN(-ENOENT);}
    int ret = curFile->second->inode->setClose();
    if (ret == 0) {
        ret = writeInode(curFile->second);
    }
    RETURN(ret);
}

/// @brief Truncate a file.
///
/// Set the size of a file to the new size. If the new size is smaller than the old size, spare bytes are removed. If
/// the new size is larger than the old size, the new bytes may be random.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] newSize New size of the file.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseTruncate(const char *path, off_t newSize) {
    LOGM();
    LOGF("Attributes: path=%s, newSize=%ld", path, newSize);
    int ret;
    if (files.find(path) == files.end()) {ret = -EBADF;}
    if (ret == 0) { ret = files[path]->inode->setSize(newSize); }
    if (ret == 0) { ret = writeInode(files[path]); }
    RETURN(ret);
}

/// @brief Truncate a file.
///
/// Set the size of a file to the new size. If the new size is smaller than the old size, spare bytes are removed. If
/// the new size is larger than the old size, the new bytes may be random. This function is called for files that are
/// open.
/// You do not have to check file permissions, but can assume that it is always ok to access the file.
/// \param [in] path Name of the file, starting with "/".
/// \param [in] newSize New size of the file.
/// \param [in] fileInfo Can be ignored in Part 1.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseTruncate(const char *path, off_t newSize, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("Attributes: path=%s, newSize=%ld, fileInfo=%s", path, newSize, "Ignored in Part1");
    int ret;
    if (files.find(path) == files.end()) { ret =-EBADF; }
    if (ret == 0) { ret = files[path]->inode->setSize(newSize) };
    if (ret == 0) { ret = writeInode(files[path]); }
    RETURN(ret);
}

/// @brief Read a directory.
///
/// Read the content of the (only) directory.
/// You do not have to check file permissions, but can assume that it is always ok to access the directory.
/// \param [in] path Path of the directory. Should be "/" in our case.
/// \param [out] buf A buffer for storing the directory entries.
/// \param [in] filler A function for putting entries into the buffer.
/// \param [in] offset Can be ignored.
/// \param [in] fileInfo Can be ignored.
/// \return 0 on success, -ERRNO on failure.
int MyOnDiskFS::fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("Attributes: path=%s", path);

    filler( buf, ".", NULL, 0 ); // Current Directory
    filler( buf, "..", NULL, 0 ); // Parent Directory

    // If the user is trying to show the files/directories of the root directory show the following
    if ( strcmp( path, "/" ) == 0 ) {
        for (auto const& item : files) {
            filler(buf, (item.first).c_str()+1, NULL, 0);
            LOGF("Return Attribute:%s",item.first.c_str());
        }
        RETURN(0);
    } else {
        RETURN(-ENOTDIR);
    }
}

/// Initialize a file system.
///
/// This function is called when the file system is mounted. You may add some initializing code here.
/// \param [in] conn Can be ignored.
/// \return 0.
void* MyOnDiskFS::fuseInit(struct fuse_conn_info *conn) {
    // Open logfile
    this->logFile= fopen(((MyFsInfo *) fuse_get_context()->private_data)->logFile, "w+");
    if(this->logFile == NULL) {
        fprintf(stderr, "ERROR: Cannot open logfile %s\n", ((MyFsInfo *) fuse_get_context()->private_data)->logFile);
    } else {
        // turn of logfile buffering
        setvbuf(this->logFile, NULL, _IOLBF, 0);

        LOG("Starting logging...\n");

        LOG("Using on-disk mode");

        LOGF("Container file name: %s", ((MyFsInfo *) fuse_get_context()->private_data)->contFile);

        int ret= this->blockDevice->open(((MyFsInfo *) fuse_get_context()->private_data)->contFile);

        char buf [BLOCK_SIZE] = {};

        if(ret >= 0) {
            LOG("Container file does exist, reading");
            this->s_block = (Superblock*) malloc(sizeof(Superblock));
            ret = this->blockDevice->read(INDEX_SUPERBLOCK, buf);
            std::memcpy(this->s_block, buf, sizeof(Superblock));
            auto* ip = new InodePointer;
            ret += this->s_block->loadINodes(this->blockDevice, ip);

            std::string path;
            ret += ip->inode->getName(&path);

            files[path] = ip;
        } else if(ret == -ENOENT) {
            LOG("Container file does not exist, creating a new one");
            ret = this->blockDevice->create(((MyFsInfo *) fuse_get_context()->private_data)->contFile);
            if (ret >= 0) {
                this->s_block = new Superblock(NUM_FS_BLOCKS, NUM_DIR_ENTRIES);
                std::memcpy(buf, this->s_block, sizeof(*this->s_block));
                ret += this->blockDevice->write(INDEX_SUPERBLOCK, buf);
            }
        }

        if(ret < 0) {
            LOGF("ERROR: Access to container file failed with error %d", ret);
        } else {
            LOG("Created Superblock with the following block index's:");
            LOGF("DMapIndex: %u", this->s_block->getDMapIndex());
            LOGF("IMapIndex %u", this->s_block->getIMapIndex());
            LOGF("INodeIndex %u", this->s_block->getINodeIndex());
            LOGF("DataIndex: %u", this->s_block->getDataIndex());
            LOGF("Size: %lu", this->s_block->getSize());
        }
     }

    RETURN(0);
}

/// @brief Clean up a file system.
///
/// This function is called when the file system is unmounted. You may add some cleanup code here.
void MyOnDiskFS::fuseDestroy() {
    LOGM();
    LOG("Delete files from file map");
    delete this->s_block;
    for (auto const& item : files) {
        delete item.second;
    }
}

// TODO: [PART 2] You may add your own additional methods here!



/// Save inode on blockdevice
///
/// This function reads an inode and blockNo from an InodePointer and writes the Inode to its respective block
///
/// \param ip [in] struct InodePointer filled with the blockNo, Inode Object, BlockDevice Pointer
/// \return 0 on success
int MyOnDiskFS::writeInode(InodePointer* ip) {
    // todo: consider throwing error if file open
    int ret = this->blockDevice->write(ip->blockNo,(char *) ip->inode);
    return ret;
}



// DO NOT EDIT ANYTHING BELOW THIS LINE!!!

/// @brief Set the static instance of the file system.
///
/// Do not edit this method!
void MyOnDiskFS::SetInstance() {
    MyFS::_instance= new MyOnDiskFS();
}
