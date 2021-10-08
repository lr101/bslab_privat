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
    int groupID;
    int mode;
    std::time_t atime;
    std::time_t mtime;
    std::time_t ctime;
    char* data;
    bool open = false;
    static void deepCopy(char* from, char* to, size_t pSize);

public:
    File(char* name, size_t size, char* data, int userID, int groupID, int mode);
    ~File();
    //TODO File methods that we probably will use; need to be implemented in file.ccp
    //TODO return int value is status code (0 on success, -ERRORCODE on Error)

    int rename(char* name);
    int getMetadata(struct stat* meta); //look up via: $man 2 stat
    int setUserID(int userID);
    int setGroupID(int groupID);
    int setOpen(bool open);
    int getData(/*TODO params */);
    int setData(/*TODO params */);
    int setDataSize(/*TODO params */);


};


#endif //MYFS_FILE_H