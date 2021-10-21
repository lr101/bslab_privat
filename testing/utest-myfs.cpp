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
#include "file.h"
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

//Test set-Name
TEST_CASE("T-f1.0") {
    printf("Testcase f-1.0: Test constructor\n");
    //init
    File* file = new File(&cmpName, cmpUid, cmpGid, cmpMode);
    std::time_t cmpTime = std::time(nullptr);
    int retName;
    int retUser;
    int retGroup;
    int retMode;
    int retSize;
    //get values
    retName = file->getName(&testName);
    retUser = file->getUserID(&testUid);
    retGroup = file->getGroupID(&testGid);
    retMode = file->getMode(&testMode);
    //test with input values. GET Methods work due to returning without edit
    REQUIRE(strcmp(testName.c_str(), cmpName.c_str()) == 0);
    REQUIRE(cmpUid == testUid);
    REQUIRE(cmpGid == testGid);
    REQUIRE(testMode == cmpMode);
    //test ret-values
    REQUIRE(retName == 0);
    REQUIRE(retUser == 0);
    REQUIRE(retGroup == 0);
    REQUIRE(retMode == 0);
    //check data size in constructor
    off_t size;
    retSize = file->getSize(&size);
    REQUIRE(size == 0);
    REQUIRE(retSize == 0);
    //check if time is in threshold: +0.0001s
    std::time_t testMtime;
    std::time_t testCtime;
    std::time_t testAtime;
    int retM;
    int retC;
    int retA;
    retM = file->getMTime(&testMtime);
    retA = file->getMTime(&testAtime);
    retC = file->getMTime(&testCtime);
    REQUIRE(std::difftime(cmpTime, testMtime) < 0.0001);
    REQUIRE(std::difftime(cmpTime, testAtime) < 0.0001);
    REQUIRE(std::difftime(cmpTime, testCtime) < 0.0001);
    REQUIRE(retA == 0);
    REQUIRE(retC == 0);
    REQUIRE(retM == 0);
}


