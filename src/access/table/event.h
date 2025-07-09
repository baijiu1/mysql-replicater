//
// Created by 白杰 on 2024/12/29.
//

#ifndef MYSQL_REPLICATER_EVENT_H
#define MYSQL_REPLICATER_EVENT_H

#include <string>
#include <cstdio>
#include <vector>
#include <map>
#include "system.h"

using namespace std;


typedef enum blockSwitches
{
    BLOCK_ABSOLUTE = 0x00000001,		/* -a: Absolute(vs Relative) addressing */
    BLOCK_BINARY = 0x00000002,			/* -b: Binary dump of block */
    BLOCK_FORMAT = 0x00000004,			/* -f: Formatted dump of blocks / control file */
    BLOCK_FORCED = 0x00000008,			/* -S: Block size forced */
    BLOCK_NO_INTR = 0x00000010,			/* -d: Dump straight blocks */
    BLOCK_RANGE = 0x00000020,			/* -R: Specific block range to dump */
    BLOCK_CHECKSUMS = 0x00000040,		/* -k: verify block checksums */
    BLOCK_DECODE = 0x00000080,			/* -D: Try to decode tuples */
    BLOCK_DECODE_TOAST = 0x00000100,	/* -t: Try to decode TOAST values */
    BLOCK_IGNORE_OLD = 0x00000200		/* -o: Decode old values */
} blockSwitches;
unsigned int blockOptions;



#define IS_4BYTE_ALIGNED(x) (((x) & 0x3) == 0)


typedef int (*typeEvent)(vector<uint8_t>&, vector<uint8_t>&, uint32_t *, string);

map<string, typeEvent> typeEventFuncMap;
int InitEventFuncMap();
int parserTypeForName(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForSmallint(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForBigint(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForInt(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForOid(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForNumeric(vector<uint8_t>&, vector<uint8_t>&, uint32_t *, string);
int parserTypeForFloat(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForDouble(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForChar(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForVarchar(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForText(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForBytea(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForBool(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForDate(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForTime(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForTimestamp(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForBpchar(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForTimestampWithTimeZone(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForInterval(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForUuid(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForInet(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForCidr(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForMacaddr(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForJson(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForJsonb(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForHstore(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForInt4range(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForNumrange(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForTsrange(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForTstzrange(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForDaterange(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);
int parserTypeForDefault(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);





#endif //MYSQL_REPLICATER_EVENT_H
