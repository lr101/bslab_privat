//
//  test-myfs.cpp
//  testing
//
//  Created by Oliver Waldhorst on 15.12.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include "../catch/catch.hpp"


#include "tools.hpp"
#include "myfs.h"
#include "inode.h"
#include "superblock.h"
#include "blockdevice.h"
#include <cstring>
#include <ctime>

std::string cmpName = "TEST";
uid_t cmpUid = 1;
gid_t cmpGid = 2;
mode_t cmpMode = 3;
std::string testName;
gid_t testGid;
uid_t testUid;
mode_t testMode;

Superblock* s_block = new Superblock(62500, 64, new BlockDevice(512));
Inode* inode = new Inode(s_block, cmpName.c_str(), cmpUid, cmpGid, cmpMode);

// TODO: Implement your helper functions here!
TEST_CASE("test setSize()") {
    REQUIRE(sizeof(bool) == 0);
}
