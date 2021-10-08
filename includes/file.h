//
// Created by user on 08.10.21.
//

#ifndef MYFS_FILE_H
#define MYFS_FILE_H
#include <ctime>

class File {
    //File attributes
private:
    char* name; //Including file path
    unsigned int name_size;
    size_t size;
    int userID;
    int mode;
    std::time_t atime;
    std::time_t mtime;
    std::time_t ctime;
    char* data;
    bool open = false; //TODO brauen wir das?
    static void deepCopy(char* from, char* to, size_t pSize);

public:
    File(char* name, size_t size, char* data, int userID, int mode);
    ~File();
    //TODO File methods:
    void copy();

};

#endif //MYFS_FILE_H