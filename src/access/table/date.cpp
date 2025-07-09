//
// Created by 白杰 on 2025/5/31.
//

#include "date.h"
/* Decode a date type */

void
j2date(int jd, int *year, int *month, int *day)
{
    unsigned int julian;
    unsigned int quad;
    unsigned int extra;
    int			y;

    julian = jd;
    julian += 32044;
    quad = julian / 146097;
    extra = (julian - quad * 146097) * 4 + 3;
    julian += 60 + quad * 3 + extra / 146097;
    quad = julian / 1461;
    julian -= quad * 1461;
    y = julian * 4 / 1461;
    julian = ((y != 0) ? ((julian + 305) % 365) : ((julian + 306) % 366))
             + 123;
    y += quad * 4;
    *year = y - 4800;
    quad = julian * 2141 / 65536;
    *day = julian - 7834 * quad / 256;
    *month = (quad + 10) % MONTHS_PER_YEAR + 1;
}


int
decode_date(const char *buffer, unsigned int buff_size)
{
    const char *new_buffer = (const char *) INTALIGN(buffer);
    unsigned int delta = (unsigned int) ((uintptr_t) new_buffer - (uintptr_t) buffer);
    int32_t 		d,
            jd,
            year,
            month,
            day;

    if (buff_size < delta)
        return -1;

    buff_size -= delta;
    buffer = new_buffer;

    if (buff_size < sizeof(int32_t))
        return -2;

//    *out_size = sizeof(int32_t) + delta;

    d = *(int32_t *) buffer;
    if (d == PG_INT32_MIN)
    {
        CopyAppend("-infinity");
        return 0;
    }
    if (d == PG_INT32_MAX)
    {
        CopyAppend("infinity");
        return 0;
    }

    jd = d + POSTGRES_EPOCH_JDATE;
    j2date(jd, &year, &month, &day);

    CopyAppendFmt("%04d-%02d-%02d%s", (year <= 0) ? -year + 1 : year, month, day, (year <= 0) ? " BC" : "");

    return 0;
}

/* Decode a time type */
int
decode_time(const char *buffer, unsigned int buff_size)
{
    const char *new_buffer = (const char *) LONGALIGN(buffer);
    unsigned int delta = (unsigned int) ((uintptr_t) new_buffer - (uintptr_t) buffer);
    int64_t 		timestamp,
            timestamp_sec;

    if (buff_size < delta)
        return -1;

    buff_size -= delta;
    buffer = new_buffer;

    if (buff_size < sizeof(int64_t))
        return -2;

    timestamp = *(int64_t *) buffer;
    timestamp_sec = timestamp / 1000000;
//    *out_size = sizeof(int64_t) + delta;

    CopyAppendFmt("%02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d.%06" INT64_MODIFIER "d",
                  timestamp_sec / 60 / 60, (timestamp_sec / 60) % 60, timestamp_sec % 60,
                  timestamp % 1000000);

    return 0;
}

/* Decode a timetz type */
int
decode_timetz(const char *buffer, unsigned int buff_size)
{
    const char *new_buffer = (const char *) LONGALIGN(buffer);
    unsigned int delta = (unsigned int) ((uintptr_t) new_buffer - (uintptr_t) buffer);
    int64_t 		timestamp,
            timestamp_sec;
    int32_t 		tz_sec,
            tz_min;

    if (buff_size < delta)
        return -1;

    buff_size -= delta;
    buffer = new_buffer;

    if (buff_size < (sizeof(int64_t) + sizeof(int32_t)))
        return -2;

    timestamp = *(int64_t *) buffer;
    tz_sec = *(int32_t *) (buffer + sizeof(int64_t));
    timestamp_sec = timestamp / 1000000;
    tz_min = -(tz_sec / 60);
//    *out_size = sizeof(int64) + sizeof(int32) + delta;

    CopyAppendFmt("%02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d.%06" INT64_MODIFIER "d%c%02d:%02d",
                  timestamp_sec / 60 / 60, (timestamp_sec / 60) % 60, timestamp_sec % 60,
                  timestamp % 1000000, (tz_min > 0 ? '+' : '-'), abs(tz_min / 60), abs(tz_min % 60));

    return 0;
}

/* Decode a timestamp type and timestampz type */
int
decode_timestamp_internal(const char *buffer, unsigned int buff_size, bool with_timezone)
{
    const char *new_buffer = (const char *) LONGALIGN(buffer);
    unsigned int delta = (unsigned int) ((uintptr_t) new_buffer - (uintptr_t) buffer);
    int64_t		timestamp,
            timestamp_sec;
    int32_t		jd,
            year,
            month,
            day;

    if (buff_size < delta)
        return -1;

    buff_size -= delta;
    buffer = new_buffer;

    if (buff_size < sizeof(int64_t))
        return -2;

//    *out_size = sizeof(int64_t) + delta;
    timestamp = *(int64_t *) buffer;

    if (timestamp == DT_NOBEGIN)
    {
        CopyAppend("-infinity");
        return 0;
    }
    if (timestamp == DT_NOEND)
    {
        CopyAppend("infinity");
        return 0;
    }

    jd = timestamp / USECS_PER_DAY;
    if (jd != 0)
        timestamp -= jd * USECS_PER_DAY;

    if (timestamp < INT64CONST(0))
    {
        timestamp += USECS_PER_DAY;
        jd -= 1;
    }

    /* add offset to go from J2000 back to standard Julian date */
    jd += POSTGRES_EPOCH_JDATE;

    j2date(jd, &year, &month, &day);
    timestamp_sec = timestamp / 1000000;

    CopyAppendFmt("%04d-%02d-%02d %02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d:%02" INT64_MODIFIER "d.%06" INT64_MODIFIER "d%s%s",
                  (year <= 0) ? -year + 1 : year, month, day,
                  timestamp_sec / 60 / 60, (timestamp_sec / 60) % 60, timestamp_sec % 60,
                  timestamp % 1000000,
                  with_timezone ? "+00" : "",
                  (year <= 0) ? " BC" : "");

    return 0;
}