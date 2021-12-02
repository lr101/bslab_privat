//
// Created by lukas on 11.11.2021.
//

#ifndef superblock_h
#define superblock_h

#include <cmath>
#include "inode.h"
#include "myfs-structs.h"
#include "blockdevice.h"

#define BYTE_SIZE 8
#define SUPERBLOCK_SIZE 1
#define D_MAP_INDEX 1
#define BYTES_PER_ADDRESS sizeof(index_t)

class Inode;
class BlockDevice;

typedef uint32_t index_t;

struct InodePointer {
    Inode* inode;
    BlockDevice* blockDevice;

    index_t blockNo;
};

class Superblock {

    size_t size; //blocks
    index_t i_map_index;
    index_t i_node_index;
    index_t data_index;
    BlockDevice* blockDevice;

    static size_t calImapSize(size_t i_node_num);
    static size_t calDmapSize (size_t size);
private:
    int getINode(index_t, struct InodePointer*, BlockDevice* blockDevice);

public:
    Superblock(size_t size, size_t i_node_num, BlockDevice* blockDevice);
    ~Superblock();


    size_t getSize();
    index_t getDMapIndex();
    index_t getIMapIndex();
    index_t getINodeIndex();
    index_t getDataIndex();
    BlockDevice* getBlockDevice();

    int loadINodes(BlockDevice*, struct InodePointer*);

    int addBlocks(Inode* , off_t );

    int rmBlocks(Inode*, off_t);

    int toggleDMapIndex(index_t );

    index_t getFreeDataBlockNo();

    index_t setIndirectPointer(index_t, off_t );

    index_t getIndirectPointer( index_t, off_t);

    int setInodeDataPointer(Inode*, int pointerIndex);

    int getFreeInodeIndex();
    int flipBitInNode (int index, char* buf);
};

#endif /* superblock_h */