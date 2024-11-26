//
// Created by 白杰 on 2024/4/22.
//

#ifndef MYSQL_REPLICATER_READ_H
#define MYSQL_REPLICATER_READ_H

extern class Read;

class Read {

    int Close();
    int Connect();
public:
    char packet_body[120];
    int Init();
    Read();

};

#endif //MYSQL_REPLICATER_READ_H

