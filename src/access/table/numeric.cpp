//
// Created by 白杰 on 2024/12/31.
//

#include "numeric.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstdint>
#include "system.h"


using namespace std;


int varlenaExtractNumeric(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    // 这里可以优化，直接转换成char*类型，然后强制转换为varattrib_1b类型
    uint8_t firstByte = group[0];
    int			padding = 0;
    int			result	= 0;
    printf("firstByte: %x group[0]: %x  group[1]: %x group[2]: %x ", firstByte, group[0], group[1], group[2]);


    if (VARATT_IS_1B(firstByte)) {
        // 存储在磁盘上的，第一个字节已经是va_header了，转换为char*类型，强制转换为varattrib_1b结构体类型
        // 第一个字节自动就是*varattrib_1b->va_header，强制转换是按顺序填入到类型中，因为va_header定义为uint8_t，所以第一个字节就直接是va_header了
        // 下面拿长度是没问题的
        printf("\n VARATT_IS_1B \n");
        printf("firstByte: %x group[0]: %x  group[1]: %x group[2]: %x ", firstByte, group[0], group[1], group[2]);
        int varSize = VARSIZE_1B(firstByte);
//        int varSize1 = ((firstByte >> 1) & 0x7F);
//        printf("varSize: %d\n", varSize);
//        printf("varSize: %x", ((firstByte >> 1) & 0x7F));
        if (varSize > group.size()) {
            throw std::out_of_range("varSize exceeds group size");
        }
        fieldData.assign(group.begin(), group.begin() + varSize);
//        for (int i = 0; i < fieldData.size(); ++i) {
//            printf(" %x ", fieldData[i]);
//        }
        // 如果下一个不是short varlena类型，那么就要对齐
        if (nextColumnAttalign == "4" && group.size() > varSize && group[varSize] != 0 && VARATT_IS_1B(group[varSize])) {
            uint32_t dataLen = TYPEALIGN(1, *lpOff + varSize) - *lpOff;
            group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
            *lpOff += dataLen;
        } else {
//            printf("TYPEALIGN(stoi(nextColumnAttalign), *lpOff + varSize): %lu", TYPEALIGN(stoi(nextColumnAttalign), *lpOff + varSize));
            uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + varSize) - *lpOff;
            printf("dataLen: %u groupSize: %zu", dataLen, group.size());
            if (dataLen > group.size()) {
                group.erase(group.begin(), group.begin() + varSize); // 移除已取出的字节
            } else {
                group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
            }
            *lpOff += dataLen;
        }
        // 这里需要转为Char*类型
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(fieldData.data());
        CopyAppendNumericValue(buffer + 1, varSize - 1);
        return 0;
    } else if (VARATT_IS_4B(firstByte)) {
        if (VARATT_IS_4B_U(firstByte)) {
            // 未压缩数据varattrib_4b，最大1G，4个字节长度
            fieldData.assign(group.begin(), group.begin() + 4);
            uint32_t vaHeader = convertBigToLittleEndian32(fieldData, 0);
            fieldData.clear();
            uint32_t vaHeaderSize = VARSIZE_4B(vaHeader);
            fieldData.assign(group.begin(), group.begin() + vaHeaderSize);
            if (group.size() < vaHeaderSize) {
                group.erase(group.begin(), group.begin() + group.size()); // 移除已取出的字节
            } else {
                uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + vaHeaderSize) - *lpOff;
                group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
                *lpOff += dataLen;
            }
//            group.erase(group.begin(), group.begin() + MAXALIGN(vaHeaderSize)); // 移除已取出的字节
            const auto* buffer = reinterpret_cast<const unsigned char*>(fieldData.data());
            CopyAppendNumericValue(buffer + 4, vaHeaderSize - 4);
        } else if (VARATT_IS_4B_C(firstByte)) {
            printf("\n4B_C\n");
            // 压缩数据varattrib_4b，最大1G，用于判断是否压缩
            /*
             * xxxxxx10 4-byte length word, aligned, *compressed* data (up to 1G)
             */
            fieldData.assign(group.begin(), group.begin() + 4);
            uint32_t vaHeader = convertBigToLittleEndian32(fieldData, 0);

            fieldData.clear();
            int						decompress_ret;
            int32_t					len = VARSIZE_4B(vaHeader);

            int32_t					decompressed_len = 0;
            unsigned char *decompress_tmp_buff;
            fieldData.assign(group.begin(), group.begin() + len);
            group.erase(group.begin(), group.begin() + MAXALIGN(len)); // 移除已取出的字节
//            char *buffer;
            if (fieldData.empty()) {
                printf("\n ERROR 0000 \n");
            }
            string bufferStr(fieldData.begin(), fieldData.end());
            const char* buffer = bufferStr.c_str();
#if PG_VERSION_NUM >= 140000
            unsigned int cmid;
#endif

#if PG_VERSION_NUM >= 140000
            decompressed_len = VARDATA_COMPRESSED_GET_EXTSIZE(buffer);
#else
            decompressed_len = VARRAWSIZE_4B_C(buffer);
#endif

            if ((decompress_tmp_buff = (unsigned char*)malloc(decompressed_len)) == NULL)
            {
                perror("malloc");
                exit(1);
            }
#if PG_VERSION_NUM >= 140000
            cmid = VARDATA_COMPRESSED_GET_COMPRESS_METHOD(buffer);
            switch(cmid)
            {
                case TOAST_PGLZ_COMPRESSION_ID:
                    decompress_ret = pglz_decompress(VARDATA_4B_C(buffer), len - 2 * sizeof(uint32_t),
                                                     decompress_tmp_buff, decompressed_len, true);
                    break;
#ifdef USE_LZ4
                    case TOAST_LZ4_COMPRESSION_ID:
				    decompress_ret = LZ4_decompress_safe(VARDATA_4B_C(buffer), decompress_tmp_buff,
													 len - 2 * sizeof(uint32), decompressed_len);
				    break;
#endif
                default:
                    decompress_ret = -1;
                    break;
            }
#else /* PG_VERSION_NUM < 140000 */
            decompress_ret = pglz_decompress(VARDATA_4B_C(buffer), len - 2 * sizeof(uint32),
                                             decompress_tmp_buff, decompressed_len
#if PG_VERSION_NUM >= 120000
                    , true
#endif
            );
#endif /* PG_VERSION_NUM >= 140000 */
            if ((decompress_ret != decompressed_len) || (decompress_ret < 0))
            {
                printf("WARNING: Corrupted toast data, unable to decompress.\n");
//                CopyAppend("(inline compressed, corrupted)");
//                *out_size = padding + len;
                free(decompress_tmp_buff);
//                return 0;
            }

            result = CopyAppendNumericValue(decompress_tmp_buff, decompressed_len);
            printf("res: %d", result);
//            unsigned int* out_size = padding + len;
            free(decompress_tmp_buff);
//            return result;
        }
    } else if (VARATT_IS_1B_E(firstByte)) {
        // TOAST
        printf("\n1B_E\n");
        printf("\n暂时没有适配1B_E的解析\n");

//        fieldData.assign(group.begin(), group.begin() + 4);
//        uint64_t vaHeader = convertBigToLittleEndian64(fieldData, 0);
//        fieldData.clear();
//        unsigned int					len = VARSIZE_EXTERNAL(vaHeader);
//
//        fieldData.assign(group.begin(), group.begin() + len);
//        group.erase(group.begin(), group.begin() + len); // 移除已取出的字节
//        const auto* buffer = reinterpret_cast<const unsigned char*>(fieldData.data());
//        // buff_size: numeric的总长度
////        if (len > buff_size)
////            return -1;
//
//        if (blockOptions & BLOCK_DECODE_TOAST)
//        {
//            result = ReadStringFromToast(buffer, len, &len, &CopyAppendNumericValue);
//        }
//        else if (VARATT_IS_EXTERNAL_ONDISK(buffer))
//        {
//            varatt_external toast_ptr;
//            VARATT_EXTERNAL_GET_POINTER(toast_ptr, buffer);
//            if (VARATT_EXTERNAL_IS_COMPRESSED(toast_ptr))
//            {
//#if PG_VERSION_NUM >= 140000
//                switch (VARATT_EXTERNAL_GET_COMPRESS_METHOD(toast_ptr))
//                {
//                    case TOAST_PGLZ_COMPRESSION_ID:
//#endif
//                        CopyAppend("(TOASTED,pglz)");
//#if PG_VERSION_NUM >= 140000
//                        break;
//                    case TOAST_LZ4_COMPRESSION_ID:
//                        CopyAppend("(TOASTED,lz4)");
//                        break;
//                    default:
//                        CopyAppend("(TOASTED,unknown)");
//                        break;
//                }
//#endif
//            }
//            else
//                CopyAppend("(TOASTED,uncompressed)");
//        }
//            /* If tag is indirect or expanded, it was stored in memory. */
//        else
//            CopyAppend("(TOASTED IN MEMORY)");
//
//        *out_size = padding + len;
//        return result;

    }
};

void
enlargeStringInfo(StringInfo str, int needed)
{
    int		newlen;
    int		limit;
    char	   *old_data;

    limit = MaxAllocSize;

    /*
     * Guard against out-of-range "needed" values.  Without this, we can get
     * an overflow or infinite loop in the following.
     */
    if (needed < 0)				/* should not happen */
    {
        printf("Error: invalid string enlargement request size: %d", needed);
        exit(1);
    }

    if (((int) needed) >= (limit - (int) str->len))
    {
        printf("Error: cannot enlarge string buffer containing %d bytes by %d more bytes.",
               str->len, needed);
        exit(1);
    }

    needed += str->len + 1;		/* total space required now */

    /* Because of the above test, we now have needed <= limit */

    if (needed <= str->maxlen)
        return;					/* got enough space already */

    /*
     * We don't want to allocate just a little more space with each append;
     * for efficiency, double the buffer size each time it overflows.
     * Actually, we might need to more than double it if 'needed' is big...
     */
    newlen = 2 * str->maxlen;
    while (needed > newlen)
        newlen = 2 * newlen;

    /*
     * Clamp to the limit in case we went past it.  Note we are assuming here
     * that limit <= INT_MAX/2, else the above loop could overflow.  We will
     * still have newlen >= needed.
     */
    if (newlen > limit)
        newlen = limit;

    old_data = str->data;
    str->data = (char *) realloc(str->data, (int) newlen);
    if (str->data == NULL)
    {
        free(old_data);
        printf("Error: realloc() failed!\n");
        exit(1);
    }

    str->maxlen = newlen;
}


void
#if PG_VERSION_NUM < 160000
appendBinaryStringInfo(StringInfo str, const char *data, int datalen)
#else
appendBinaryStringInfo(StringInfo str, const void *data, int datalen)
#endif
{
    assert(str != NULL);

    /* Make more room if needed */
    enlargeStringInfo(str, datalen);

    /* OK, append the data */
    memcpy(str->data + str->len, data, datalen);
    str->len += datalen;

    /*
     * Keep a trailing null in place, even though it's probably useless for
     * binary data.  (Some callers are dealing with text but call this because
     * their input isn't null-terminated.)
     */
    str->data[str->len] = '\0';
}

void
appendStringInfoString(StringInfo str, const char *s)
{
    appendBinaryStringInfo(str, s, strlen(s));
}

void
resetStringInfo(StringInfo str)
{
    str->data[0] = '\0';
    str->len = 0;
    str->cursor = 0;
}

void
initStringInfo(StringInfo str)
{
    printf("\ninit StringInfo!\n");
    int			size = 1024;	/* initial default buffer size */

    str->data = (char *) malloc(size);
    str->maxlen = size;
    resetStringInfo(str);
}



/* Append given string to current COPY line */
void
CopyAppend(const char *str)
{
    if (!copyStringInitDone)
    {
        initStringInfo(&copyString);
        copyStringInitDone = true;
    }

    /* Caller probably wanted just to init copyString */
    if (str == NULL)
        return;

    if (copyString.data[0] != '\0')
        appendStringInfoString(&copyString, "\t");

    appendStringInfoString(&copyString, str);
    printf(" \n str1: %s \n", str);
}

static int
CopyAppendNumeric(const NumericVar *var, Numeric Numeric)
{

    int			sign;
    int			weight;
    int			dscale;
    int			ndigits;
    int			i;
    char	   *str;
    char	   *cp;
    char	   *endcp;
    int			d;
    bool		putit;
    NumericDigit d1;
    NumericDigit dig;
    NumericDigit *digits;

    sign = var->sign;
    weight = var->weight;
    dscale = var->dscale;
    digits = Numeric->choice.n_short.n_data;
//    uint8_t 		num_size = VARSIZE_1B(Numeric->vl_len_);
    int num_size = 1;
    if (num_size == NUMERIC_HEADER_SIZE(Numeric))
    {
        /* No digits - compressed zero. */
//        CopyAppendFmt("%d", 0);
//        free(num);
        return 0;
    }
    else
    {
        ndigits = num_size / sizeof(NumericDigit);
        i = (weight + 1) * DEC_DIGITS;
        if (i <= 0)
            i = 1;

        str = (char *) malloc(i + dscale + DEC_DIGITS + 2);
        cp = str;

        /*
         * Output a dash for negative values
         */
        if (sign == NUMERIC_NEG)
            *cp++ = '-';

        /*
         * Output all digits before the decimal point
         */
        if (weight < 0)
        {
            d = weight + 1;
            *cp++ = '0';
        }
        else
        {
            for (d = 0; d <= weight; d++)
            {
                dig = (d < ndigits) ? digits[d] : 0;

                /*
                 * In the first digit, suppress extra leading decimal
                 * zeroes
                 */
                putit = (d > 0);
                d1 = dig / 1000;
                dig -= d1 * 1000;
                putit |= (d1 > 0);
                if (putit)
                    *cp++ = d1 + '0';
                d1 = dig / 100;
                dig -= d1 * 100;
                putit |= (d1 > 0);
                if (putit)
                    *cp++ = d1 + '0';
                d1 = dig / 10;
                dig -= d1 * 10;
                putit |= (d1 > 0);
                if (putit)
                    *cp++ = d1 + '0';
                *cp++ = dig + '0';
            }
        }

        /*
         * If requested, output a decimal point and all the digits that
         * follow it. We initially put out a multiple of DEC_DIGITS
         * digits, then truncate if needed.
         */
        if (dscale > 0)
        {
            *cp++ = '.';
            endcp = cp + dscale;
            for (i = 0; i < dscale; d++, i += DEC_DIGITS)
            {
                dig = (d >= 0 && d < ndigits) ? digits[d] : 0;
                d1 = dig / 1000;
                dig -= d1 * 1000;
                *cp++ = d1 + '0';
                d1 = dig / 100;
                dig -= d1 * 100;
                *cp++ = d1 + '0';
                d1 = dig / 10;
                dig -= d1 * 10;
                *cp++ = d1 + '0';
                *cp++ = dig + '0';
            }
            cp = endcp;
        }

        *cp = '\0';
        CopyAppend(str);
        free(str);
        return 0;
    }

}

int CopyAppendNumericValue(const unsigned char *buffer, int num_size)
{
    struct NumericData *num = (struct NumericData *) malloc(num_size);

    if (num == NULL)
        return -2;

    memcpy((char *) num, buffer, num_size);

    if (NUMERIC_IS_SPECIAL(num))
    {
        int	result = -2;

        if (NUMERIC_IS_NINF(num))
        {
            CopyAppend("-Infinity");
            result = 0;
        }
        if (NUMERIC_IS_PINF(num))
        {
            CopyAppend("Infinity");
            result = 0;
        }
        if (NUMERIC_IS_NAN(num))
        {
            CopyAppend("NaN");
            result = 0;
        }

        free(num);

        return result;
    }
    else
    {
        int			sign;
        int			weight;
        int			dscale;
        int			ndigits;
        int			i;
        char	   *str;
        char	   *cp;
        char	   *endcp;
        int			d;
        bool		putit;
        NumericDigit d1;
        NumericDigit dig;
        NumericDigit *digits;

        sign = NUMERIC_SIGN(num);
        weight = NUMERIC_WEIGHT(num);
        dscale = NUMERIC_DSCALE(num);

        if (num_size == NUMERIC_HEADER_SIZE(num))
        {
            /* No digits - compressed zero. */
//            CopyAppendFmt("%d", 0);
            free(num);
            return 0;
        }
        else
        {
            ndigits = num_size / sizeof(NumericDigit);
            digits = (NumericDigit *) ((char *) num + NUMERIC_HEADER_SIZE(num));

            i = (weight + 1) * DEC_DIGITS;
            if (i <= 0)
                i = 1;

            str = (char*)malloc(i + dscale + DEC_DIGITS + 2);
            cp = str;
            /*
             * Output a dash for negative values
             */
            if (sign == NUMERIC_NEG)
                *cp++ = '-';

            /*
             * Output all digits before the decimal point
             */
            if (weight < 0)
            {
                d = weight + 1;
                *cp++ = '0';
            }
            else
            {
                for (d = 0; d <= weight; d++)
                {
                    dig = (d < ndigits) ? digits[d] : 0;
                    /*
                     * In the first digit, suppress extra leading decimal
                     * zeroes
                     */
                    putit = (d > 0);
                    d1 = dig / 1000;
                    dig -= d1 * 1000;
                    putit |= (d1 > 0);
                    if (putit)
                        *cp++ = d1 + '0';
                    d1 = dig / 100;
                    dig -= d1 * 100;
                    putit |= (d1 > 0);
                    if (putit)
                        *cp++ = d1 + '0';
                    d1 = dig / 10;
                    dig -= d1 * 10;
                    putit |= (d1 > 0);
                    if (putit)
                        *cp++ = d1 + '0';
                    *cp++ = dig + '0';
                }
            }

            /*
             * If requested, output a decimal point and all the digits that
             * follow it. We initially put out a multiple of DEC_DIGITS
             * digits, then truncate if needed.
             */
            if (dscale > 0)
            {
                *cp++ = '.';
                endcp = cp + dscale;
                for (i = 0; i < dscale; d++, i += DEC_DIGITS)
                {
                    dig = (d >= 0 && d < ndigits) ? digits[d] : 0;
                    d1 = dig / 1000;
                    dig -= d1 * 1000;
                    *cp++ = d1 + '0';
                    d1 = dig / 100;
                    dig -= d1 * 100;
                    *cp++ = d1 + '0';
                    d1 = dig / 10;
                    dig -= d1 * 10;
                    *cp++ = d1 + '0';
                    *cp++ = dig + '0';
                }
                cp = endcp;
            }
            *cp = '\0';
            CopyAppend(str);
            printf(" \n str: %s \n", str);
            free(str);
            free(num);
            return 0;
        }
    }
}

int parserNumericRelValue(Numeric Numeric, vector<uint8_t>& data, const string& type) {
    NumericVar	x;
//    x.ndigits = NUMERIC_NDIGITS(Numeric);
    x.weight = NUMERIC_WEIGHT(Numeric);
    x.sign = NUMERIC_SIGN(Numeric);
    x.dscale = NUMERIC_DSCALE(Numeric);
//    x.digits = NUMERIC_DIGITS(Numeric);
    CopyAppendNumeric(&x, Numeric);
    return 1;

}


int32_t
pglz_decompress(const char *source, int32_t slen, unsigned char *dest,
                int32_t rawsize, bool check_complete) {
    const unsigned char *sp;
    const unsigned char *srcend;
    unsigned char *dp;
    unsigned char *destend;

    sp = (const unsigned char *) source;
    srcend = ((const unsigned char *) source) + slen;
    dp = (unsigned char *) dest;
    destend = dp + rawsize;

    while (sp < srcend && dp < destend) {

        /*
         * Read one control byte and process the next 8 items (or as many as
         * remain in the compressed input).
         */
        unsigned char ctrl = *sp++;
        int ctrlc;

        for (ctrlc = 0; ctrlc < 8 && sp < srcend && dp < destend; ctrlc++) {
            if (ctrl & 1) {
                /*
                 * Set control bit means we must read a match tag. The match
                 * is coded with two bytes. First byte uses lower nibble to
                 * code length - 3. Higher nibble contains upper 4 bits of the
                 * offset. The next following byte contains the lower 8 bits
                 * of the offset. If the length is coded as 18, another
                 * extension tag byte tells how much longer the match really
                 * was (0-255).
                 */
                int32_t len;
                int32_t off;

                len = (sp[0] & 0x0f) + 3;
                off = ((sp[0] & 0xf0) << 4) | sp[1];
                sp += 2;
                if (len == 18)
                    len += *sp++;

                /*
                 * Check for corrupt data: if we fell off the end of the
                 * source, or if we obtained off = 0, or if off is more than
                 * the distance back to the buffer start, we have problems.
                 * (We must check for off = 0, else we risk an infinite loop
                 * below in the face of corrupt data.  Likewise, the upper
                 * limit on off prevents accessing outside the buffer
                 * boundaries.)
                 */
                if (unlikely(sp > srcend || off == 0 ||
                             off > (dp - (unsigned char *) dest)))
                    return -1;

                /*
                 * Don't emit more data than requested.
                 */
                len = Min(len, destend - dp);

                /*
                 * Now we copy the bytes specified by the tag from OUTPUT to
                 * OUTPUT (copy len bytes from dp - off to dp).  The copied
                 * areas could overlap, so to avoid undefined behavior in
                 * memcpy(), be careful to copy only non-overlapping regions.
                 *
                 * Note that we cannot use memmove() instead, since while its
                 * behavior is well-defined, it's also not what we want.
                 */
                while (off < len) {
                    /*
                     * We can safely copy "off" bytes since that clearly
                     * results in non-overlapping source and destination.
                     */
                    memcpy(dp, dp - off, off);
                    len -= off;
                    dp += off;

                    /*----------
                     * This bit is less obvious: we can double "off" after
                     * each such step.  Consider this raw input:
                     *		112341234123412341234
                     * This will be encoded as 5 literal bytes "11234" and
                     * then a match tag with length 16 and offset 4.  After
                     * memcpy'ing the first 4 bytes, we will have emitted
                     *		112341234
                     * so we can double "off" to 8, then after the next step
                     * we have emitted
                     *		11234123412341234
                     * Then we can double "off" again, after which it is more
                     * than the remaining "len" so we fall out of this loop
                     * and finish with a non-overlapping copy of the
                     * remainder.  In general, a match tag with off < len
                     * implies that the decoded data has a repeat length of
                     * "off".  We can handle 1, 2, 4, etc repetitions of the
                     * repeated string per memcpy until we get to a situation
                     * where the final copy step is non-overlapping.
                     *
                     * (Another way to understand this is that we are keeping
                     * the copy source point dp - off the same throughout.)
                     *----------
                     */
                    off += off;
                }
                memcpy(dp, dp - off, len);
                dp += len;
            } else {
                /*
                 * An unset control bit means LITERAL BYTE. So we just copy
                 * one from INPUT to OUTPUT.
                 */
                *dp++ = *sp++;
            }

            /*
             * Advance the control bit
             */
            ctrl >>= 1;
        }
    }
    if (check_complete && (dp != destend || sp != srcend))
        return -1;

    /*
     * That's it.
     */
    return (unsigned char *) dp - dest;
}
//
//static int
//ReadStringFromToast(const char *buffer,
//                    unsigned int buff_size,
//                    unsigned int* out_size,
//                    int (*parse_value)(const unsigned char *, int))
//{
//    int		result = 0;
//
//    /* If toasted value is on disk, we'll try to restore it. */
//    if (VARATT_IS_EXTERNAL_ONDISK(buffer))
//    {
//        varatt_external toast_ptr;
//        char	   *toast_data = NULL;
//        /* Number of chunks the TOAST data is divided into */
//        int32		num_chunks;
//        /* Actual size of external TOASTed value */
//        int32		toast_ext_size;
//        /* Path to directory with TOAST realtion file */
//        char	   *toast_relation_path;
//        /* Filename of TOAST relation file */
//        char		toast_relation_filename[MAXPGPATH];
//        FILE	   *toast_rel_fp;
//        unsigned int block_options = 0;
//        unsigned int control_options = 0;
//
//        VARATT_EXTERNAL_GET_POINTER(toast_ptr, buffer);
//
//        /* Extract TOASTed value */
//#if PG_VERSION_NUM >= 140000
//        toast_ext_size = VARATT_EXTERNAL_GET_EXTSIZE(toast_ptr);
//#else
//        toast_ext_size = toast_ptr.va_extsize;
//#endif
//        num_chunks = (toast_ext_size - 1) / TOAST_MAX_CHUNK_SIZE + 1;
//
//        printf("  TOAST value. Raw size: %8d, external size: %8d, "
//               "value id: %6d, toast relation id: %6d, chunks: %6d\n",
//               toast_ptr.va_rawsize,
//               toast_ext_size,
//               toast_ptr.va_valueid,
//               toast_ptr.va_toastrelid,
//               num_chunks);
//
//        /* Open TOAST relation file */
//        toast_relation_path = strdup(fileName);
//        get_parent_directory(toast_relation_path);
//        sprintf(toast_relation_filename, "%s/%d",
//                *toast_relation_path ? toast_relation_path : ".",
//                toast_ptr.va_toastrelid);
//        toast_rel_fp = fopen(toast_relation_filename, "rb");
//        if (!toast_rel_fp) {
//            printf("Cannot open TOAST relation %s\n",
//                   toast_relation_filename);
//            result = -1;
//        }
//        else
//        {
//            unsigned int toast_relation_block_size = GetBlockSize(toast_rel_fp);
//            fseek(toast_rel_fp, 0, SEEK_SET);
//            toast_data = malloc(toast_ptr.va_rawsize);
//
//            result = DumpFileContents(block_options,
//                                      control_options,
//                                      toast_rel_fp,
//                                      toast_relation_block_size,
//                                      -1, /* no start block */
//                                      -1, /* no end block */
//                                      true, /* is toast relation */
//                                      toast_ptr.va_valueid,
//                                      toast_ext_size,
//                                      toast_data);
//
//            if (result == 0)
//            {
//                if (VARATT_EXTERNAL_IS_COMPRESSED(toast_ptr))
//                    result = DumpCompressedString(toast_data, toast_ext_size, parse_value);
//                else
//                    result = parse_value(toast_data, toast_ext_size);
//            }
//            else
//            {
//                printf("Error in TOAST file.\n");
//            }
//
//            free(toast_data);
//            fclose(toast_rel_fp);
//        }
//
//        free(toast_relation_path);
//    }
//        /* If tag is indirect or expanded, it was stored in memory. */
//    else
//    {
//        CopyAppend("(TOASTED IN MEMORY)");
//    }
//
//    return result;
//}