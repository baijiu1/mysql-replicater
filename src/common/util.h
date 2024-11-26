//
// Created by 白杰 on 2024/4/22.
//

#ifndef MYSQL_REPLICATER_UTIL_H
#define MYSQL_REPLICATER_UTIL_H

#endif //MYSQL_REPLICATER_UTIL_H
#include <iostream>
#include <stdio.h>
#include <string>

#define walPageThread 10;
using namespace std;

class Util {

public:
    Util();
    int Init();
    bool OpenCfgFile(char * filename);
    bool WriteResumeFile();
    int createPostgreSQL();
    bool openFile(const string& filename);

};
