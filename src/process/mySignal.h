//
// Created by 白杰 on 2024/5/21.
//

#ifndef MYSQL_REPLICATER_SIGNAL_H
#define MYSQL_REPLICATER_SIGNAL_H

void handleSignalINT(int signum);
void handleSignalABRT(int signum);
int registerSignal();


#endif //MYSQL_REPLICATER_SIGNAL_H
