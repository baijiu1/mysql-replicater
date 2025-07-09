//
// Created by 白杰 on 2025/5/30.
//

#ifndef MYSQL_REPLICATER_BPCHAR_H
#define MYSQL_REPLICATER_BPCHAR_H
#include <iostream>
#include <vector>
#include "numeric.h"
#include "page.h"

using namespace std;
#define TYPEALIGN_DOWN(ALIGNVAL, LEN) \
    (((uintptr_t)(LEN) + ((ALIGNVAL) - 1)) & ~((uintptr_t)((ALIGNVAL) - 1)))

#define IS_4BYTE_ALIGNED(x) (((x) & 0x3) == 0)

int varlenaExtractBpchar(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);



int
CopyAppendEncode(const unsigned char *str, int orig_len);

#endif //MYSQL_REPLICATER_BPCHAR_H
