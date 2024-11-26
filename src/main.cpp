#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include "common/util.h"
#include <csignal>
#include "process/mySignal.h"
#include <fstream>


using namespace std;

int MysqlReplicater_Admin_Int_module() {

};



int main() {
    // detect exception signal
    registerSignal();
//    Util * util = new Util();
    Util * util;
//    util->Init();

//
//    MysqlReplicater_Admin_Int_module();
//    if (ret == 1) {
//        raise(SIGINT);
//    }

    // insert to target db
//    util->createPostgreSQL();


   util->openFile("wal");


}


