#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include "common/util.h"
#include <csignal>
#include "process/mySignal.h"
#include "access/access.h"

using namespace std;


int main(int argc, char* argv[]) {
    Util * util = new Util();
    Util * util;
    util->Init();
    // create mysql binlog receive program
    createBinlogStartApp(argc, argv);
    MysqlReplicater_Admin_Int_module();
//    if (ret == 1) {
//        raise(SIGINT);
//    }
}


