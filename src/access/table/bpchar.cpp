//
// Created by 白杰 on 2025/5/30.
//

#include "bpchar.h"
#include "system.h"

int varlenaExtractBpchar(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    // 这里可以优化，直接转换成char*类型，然后强制转换为varattrib_1b类型
    uint32_t firstByte = group[0];
    int			padding = 0;
    int			result	= 0;

    if (VARATT_IS_1B(firstByte)) {
        // 存储在磁盘上的，第一个字节已经是va_header了，转换为char*类型，强制转换为varattrib_1b结构体类型
        // 第一个字节自动就是*varattrib_1b->va_header，强制转换是按顺序填入到类型中，因为va_header定义为uint8_t，所以第一个字节就直接是va_header了
        // 下面拿长度是没问题的
        int varSize = VARSIZE_1B(firstByte);
        printf(" \n varSize: %u \n", varSize);
        printf(" \n VARSIZE_1B \n");
        if (varSize > group.size()) {
            throw std::out_of_range("varSize exceeds group size");
        }
        fieldData.assign(group.begin(), group.begin() + varSize);
//        for (int i = 0; i < fieldData.size(); ++i) {
//            printf(" %x ", fieldData[i]);
//        }
        if (nextColumnAttalign == "4" && VARATT_IS_1B(group[varSize])) {
            uint32_t dataLen = TYPEALIGN(1, *lpOff + varSize) - *lpOff;
            group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
            *lpOff += dataLen;
        } else {
            uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + varSize) - *lpOff;
            group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
            *lpOff += dataLen;
        }
        // 这里需要转为Char*类型
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(fieldData.data());
        CopyAppendEncode(buffer + 1, varSize - 1);
        return 0;
    } else if (VARATT_IS_4B(firstByte)) {
        printf("\nVARATT_IS_4B\n");
        if (VARATT_IS_4B_U(firstByte)) {
            printf("\nVARATT_IS_4B_U");
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

            const auto* buffer = reinterpret_cast<const unsigned char*>(fieldData.data());
            CopyAppendEncode(buffer + 4, vaHeaderSize - 4);
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
                return 0;
            }
            result = CopyAppendEncode(decompress_tmp_buff, decompressed_len);
            printf("res: %d", result);
//            unsigned int* out_size = padding + len;
            free(decompress_tmp_buff);
            return result;
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
}

/* Append given string to current COPY line */


/*
 * Append given string to current COPY line and encode special symbols
 * like \r, \n, \t and \\.
 */
int
CopyAppendEncode(const unsigned char *str, int orig_len)
{
    int			curr_offset = 0;
    int			len = orig_len;
    char	   *tmp_buff = (char *)malloc(2 * orig_len + 1);

    if (tmp_buff == NULL)
    {
        perror("malloc");
        exit(1);
    }

    while (len > 0)
    {
        /*
         * Since we are working with potentially corrupted data we can
         * encounter \0 as well.
         */
        if (*str == '\0')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = '0';
            curr_offset += 2;
        }
        else if (*str == '\r')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = 'r';
            curr_offset += 2;
        }
        else if (*str == '\n')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = 'n';
            curr_offset += 2;
        }
        else if (*str == '\t')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = 'r';
            curr_offset += 2;
        }
        else if (*str == '\\')
        {
            tmp_buff[curr_offset] = '\\';
            tmp_buff[curr_offset + 1] = '\\';
            curr_offset += 2;
        }
        else
        {
            /* It's a regular symbol. */
            tmp_buff[curr_offset] = *str;
            curr_offset++;
        }

        str++;
        len--;
    }

    tmp_buff[curr_offset] = '\0';
    CopyAppend(tmp_buff);
    printf(" \n tmp_buff: %s \n", tmp_buff);
    free(tmp_buff);

    return 0;
}


