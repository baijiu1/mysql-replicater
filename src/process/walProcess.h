//
// Created by 白杰 on 2024/6/25.
//

#ifndef MYSQL_REPLICATER_WALPROCESS_H
#define MYSQL_REPLICATER_WALPROCESS_H

#include <iostream>
#include <cstdint>
#include <cstdio>

using namespace std;

void startWalProcess(int threadId, vector<uint8_t> data);
int startXLogLongPageHeaderDataProcess(const vector<uint8_t>& data);
int startXLogPageHeaderDataProcess(const vector<uint8_t>& data);



int startWalParser(vector<uint8_t> data);
/*
 * Each page of XLOG file has a header like this:
 * 每一个事务日志文件的page都有头部信息,结构如下:
 */
//可作为WAL版本信息
#define XLOG_PAGE_MAGIC 0xD098  /* can be used as WAL version indicator */
typedef uint64_t XLogRecPtr;
#define InvalidXLogRecPtr   0
#define XLogRecPtrIsInvalid(r)  ((r) == InvalidXLogRecPtr)
#define LSN_FORMAT_ARGS(lsn) (AssertVariableIsOfTypeMacro((lsn), XLogRecPtr), (uint32) ((lsn) >> 32)), ((uint32) (lsn))
typedef uint64_t XLogSegNo;
typedef uint32_t TimeLineID;
typedef uint16_t RepOriginId;

typedef struct XLogPageHeaderData
{
    //WAL版本信息,PG V11.1 --> 0xD98
    uint16_t      xlp_magic;      /* magic value for correctness checks */
    //标记位(详见下面说明)
    uint16_t      xlp_info;       /* flag bits, see below */
    //page中第一个XLOG Record的TimeLineID,类型为uint32_t
    TimeLineID  xlp_tli;        /* TimeLineID of first record on page */
    //page的XLOG地址(在事务日志中的偏移),类型为uint64
    XLogRecPtr  xlp_pageaddr;   /* XLOG address of this page */

    /*
     * When there is not enough space on current page for whole record, we
     * continue on the next page.  xlp_rem_len is the number of bytes
     * remaining from a previous page.
     * 如果当前页的空间不足以存储整个XLOG Record,在下一个页面中存储余下的数据
     * xlp_rem_len表示上一页XLOG Record剩余部分的大小
     *
     * Note that xl_rem_len includes backup-block data; that is, it tracks
     * xl_tot_len not xl_len in the initial header.  Also note that the
     * continuation data isn't necessarily aligned.
     * 注意xl_rem_len包含backup-block data(full-page-write);
     * 也就是说在初始的头部信息中跟踪的是xl_tot_len而不是xl_len.
     * 另外要注意的是剩余的数据不需要对齐.
     */
    //上一页空间不够存储XLOG Record,该Record在本页继续存储占用的空间大小
    uint32_t      xlp_rem_len;    /* total len of remaining data for record */
} XLogPageHeaderData;

#define SizeOfXLogShortPHD  MAXALIGN(sizeof(XLogPageHeaderData))

typedef XLogPageHeaderData *XLogPageHeader;

/*
 * When the XLP_LONG_HEADER flag is set, we store additional fields in the
 * page header.  (This is ordinarily done just in the first page of an
 * XLOG file.)  The additional fields serve to identify the file accurately.
 * 如设置了XLP_LONG_HEADER标记,在page header中存储额外的字段.
 * (通常在每个事务日志文件也就是segment file的的第一个page中存在).
 * 附加字段用于准确识别文件。
 */
typedef struct XLogLongPageHeaderData
{
    //标准的头部域字段
    XLogPageHeaderData std;     /* standard header fields */
    //pg_control中的系统标识码
    uint64_t      xlp_sysid;      /* system identifier from pg_control */
    //交叉检查
    uint32_t      xlp_seg_size;   /* just as a cross-check */
    //交叉检查
    uint32_t      xlp_xlog_blcksz;    /* just as a cross-check */
} XLogLongPageHeaderData;

#define SizeOfXLogLongPHD   MAXALIGN(sizeof(XLogLongPageHeaderData))
//指针
typedef XLogLongPageHeaderData *XLogLongPageHeader;

/* When record crosses page boundary, set this flag in new page's header */
//如果XLOG Record跨越page边界,在新page header中设置该标志位
#define XLP_FIRST_IS_CONTRECORD     0x0001
//该标志位标明是"long"页头
/* This flag indicates a "long" page header */
#define XLP_LONG_HEADER             0x0002
/* This flag indicates backup blocks starting in this page are optional */
//该标志位标明从该页起始的backup blocks是可选的(不一定存在)
#define XLP_BKP_REMOVABLE           0x0004
//xlp_info中所有定义的标志位(用于page header的有效性检查)
/* All defined flag bits in xlp_info (used for validity checking of header) */
#define XLP_ALL_FLAGS               0x0007

#define XLogPageHeaderSize(hdr)     \
    (((hdr)->xlp_info & XLP_LONG_HEADER) ? SizeOfXLogLongPHD : SizeOfXLogShortPHD)

/*
 * The overall layout of an XLOG record is:
 *      Fixed-size header (XLogRecord struct)
 *      XLogRecordBlockHeader struct
 *      XLogRecordBlockHeader struct
 *      ...
 *      XLogRecordDataHeader[Short|Long] struct
 *      block data
 *      block data
 *      ...
 *      main data
 * XLOG record的整体布局如下:
 *         固定大小的头部(XLogRecord 结构体)
 *        XLogRecordBlockHeader 结构体
 *        XLogRecordBlockHeader 结构体
 *        ...
 *        XLogRecordDataHeader[Short|Long] 结构体
 *        block data
 *        block data
 *        ...
 *        main data
 *
 * There can be zero or more XLogRecordBlockHeaders, and 0 or more bytes of
 * rmgr-specific data not associated with a block.  XLogRecord structs
 * always start on MAXALIGN boundaries in the WAL files, but the rest of
 * the fields are not aligned.
 * 其中,XLogRecordBlockHeaders可能有0或者多个,与block无关的0或多个字节的rmgr-specific数据
 * XLogRecord通常在WAL文件的MAXALIGN边界起写入,但后续的字段并没有对齐
 *
 * The XLogRecordBlockHeader, XLogRecordDataHeaderShort and
 * XLogRecordDataHeaderLong structs all begin with a single 'id' byte. It's
 * used to distinguish between block references, and the main data structs.
 * XLogRecordBlockHeader/XLogRecordDataHeaderShort/XLogRecordDataHeaderLong开头是占用1个字节的"id".
 * 用于区分block依赖和main data结构体.
 */
//typedef unsigned int Oid;
//typedef Oid regproc;
//typedef regproc RegProcedure;

typedef uint32_t TransactionId;

typedef uint32_t LocalTransactionId;

typedef uint32_t SubTransactionId;

#define InvalidSubTransactionId     ((SubTransactionId) 0)
#define TopSubTransactionId         ((SubTransactionId) 1)

/* MultiXactId must be equivalent to TransactionId, to fit in t_xmax */
typedef TransactionId MultiXactId;

typedef uint32_t MultiXactOffset;

typedef uint32_t CommandId;
typedef uint8_t RmgrId;
typedef uint32_t pg_crc32c;
typedef struct XLogRecord
{
    //record的大小
    uint32_t      xl_tot_len;     /* total len of entire record */
    //xact id
    TransactionId xl_xid;       /* xact id */
    //指向log中的前一条记录
    XLogRecPtr  xl_prev;        /* ptr to previous record in log */
    //标识位,详见下面的说明
    uint8_t       xl_info;        /* flag bits, see below */
    //该记录的资源管理器
    RmgrId      xl_rmid;        /* resource manager for this record */
    /* 2 bytes of padding here, initialize to zero */
    //2个字节的crc校验位,初始化为0
    pg_crc32c   xl_crc;         /* CRC for this record */

    /* XLogRecordBlockHeaders and XLogRecordDataHeader follow, no padding */
    //接下来是XLogRecordBlockHeaders和XLogRecordDataHeader
} XLogRecord;
//宏定义:XLogRecord大小
#define SizeOfXLogRecord    (offsetof(XLogRecord, xl_crc) + sizeof(pg_crc32c))

/*
 * The high 4 bits in xl_info may be used freely by rmgr. The
 * XLR_SPECIAL_REL_UPDATE and XLR_CHECK_CONSISTENCY bits can be passed by
 * XLogInsert caller. The rest are set internally by XLogInsert.
 * xl_info的高4位由rmgr自由使用.
 * XLR_SPECIAL_REL_UPDATE和XLR_CHECK_CONSISTENCY由XLogInsert函数的调用者传入.
 * 其余由XLogInsert内部使用.
 */
#define XLR_INFO_MASK           0x0F
#define XLR_RMGR_INFO_MASK      0xF0

/*
 * If a WAL record modifies any relation files, in ways not covered by the
 * usual block references, this flag is set. This is not used for anything
 * by PostgreSQL itself, but it allows external tools that read WAL and keep
 * track of modified blocks to recognize such special record types.
 * 如果WAL记录使用特殊的方式(不涉及通常块引用)更新了关系的存储文件,设置此标记.
 * PostgreSQL本身并不使用这种方法，但它允许外部工具读取WAL并跟踪修改后的块，
 *   以识别这种特殊的记录类型。
 */
#define XLR_SPECIAL_REL_UPDATE  0x01

/*
 * Enforces consistency checks of replayed WAL at recovery. If enabled,
 * each record will log a full-page write for each block modified by the
 * record and will reuse it afterwards for consistency checks. The caller
 * of XLogInsert can use this value if necessary, but if
 * wal_consistency_checking is enabled for a rmgr this is set unconditionally.
 * 在恢复时强制执行一致性检查.
 * 如启用此功能,每个记录将为记录修改的每个块记录一个完整的页面写操作，并在以后重用它进行一致性检查。
 * 在需要时,XLogInsert的调用者可使用此标记,但如果rmgr启用了wal_consistency_checking,
 *   则会无条件执行一致性检查.
 */
#define XLR_CHECK_CONSISTENCY   0x02

/*
 * Header info for block data appended to an XLOG record.
 * 追加到XLOG record中block data的头部信息
 *
 * 'data_length' is the length of the rmgr-specific payload data associated
 * with this block. It does not include the possible full page image, nor
 * XLogRecordBlockHeader struct itself.
 * 'data_length'是与此块关联的rmgr特定payload data的长度。
 * 它不包括可能的full page image，也不包括XLogRecordBlockHeader结构体本身。
 *
 * Note that we don't attempt to align the XLogRecordBlockHeader struct!
 * So, the struct must be copied to aligned local storage before use.
 * 注意:我们不打算尝试对齐XLogRecordBlockHeader结构体!
 * 因此,在使用前,XLogRecordBlockHeader必须拷贝到对齐的本地存储中.
 */
typedef struct XLogRecordBlockHeader
{
    //块引用ID
    uint8_t       id;             /* block reference ID */
    //在关系中使用的fork和flags
    uint8_t       fork_flags;     /* fork within the relation, and flags */
    //payload字节大小
    uint16_t      data_length;    /* number of payload bytes (not including page
                                 * image) */

    /* If BKPBLOCK_HAS_IMAGE, an XLogRecordBlockImageHeader struct follows */
/* If BKPBLOCK_SAME_REL is not set, a RelFileNode follows */
/* BlockNumber follows */
    //如BKPBLOCK_HAS_IMAGE,后续为XLogRecordBlockImageHeader结构体
    //如BKPBLOCK_SAME_REL没有设置,则为RelFileNode
    //后续为BlockNumber
} XLogRecordBlockHeader;

#define SizeOfXLogRecordBlockHeader (offsetof(XLogRecordBlockHeader, data_length) + sizeof(uint16))

/*
 * XLogRecordDataHeaderShort/Long are used for the "main data" portion of
 * the record. If the length of the data is less than 256 bytes, the short
 * form is used, with a single byte to hold the length. Otherwise the long
 * form is used.
 * XLogRecordDataHeaderShort/Long用于记录的“main data”部分。
 * 如果数据的长度小于256字节，则使用短格式，用一个字节保存长度。
 * 否则使用长形式。
 *
 * (These structs are currently not used in the code, they are here just for
 * documentation purposes).
 * (这些结构体不会再代码中使用,在这里是为了文档记录的目的)
 */
typedef struct XLogRecordDataHeaderShort
{
    uint8_t       id;             /* XLR_BLOCK_ID_DATA_SHORT */
    uint8_t       data_length;    /* number of payload bytes */
}           XLogRecordDataHeaderShort;

#define SizeOfXLogRecordDataHeaderShort (sizeof(uint8) * 2)

typedef struct XLogRecordDataHeaderLong
{
    uint8_t       id;             /* XLR_BLOCK_ID_DATA_LONG */
    /* followed by uint32_t data_length, unaligned */
    //接下来是无符号32位整型的data_length(未对齐)
}           XLogRecordDataHeaderLong;

#define SizeOfXLogRecordDataHeaderLong (sizeof(uint8) + sizeof(uint32_t))

/*
 * Block IDs used to distinguish different kinds of record fragments. Block
 * references are numbered from 0 to XLR_MAX_BLOCK_ID. A rmgr is free to use
 * any ID number in that range (although you should stick to small numbers,
 * because the WAL machinery is optimized for that case). A couple of ID
 * numbers are reserved to denote the "main" data portion of the record.
 * 块id用于区分不同类型的记录片段。
 * 块引用编号从0到XLR_MAX_BLOCK_ID。
 * rmgr可以自由使用该范围内的任何ID号
 *   (尽管您应该坚持使用较小的数字，因为WAL机制针对这种情况进行了优化)。
 * 保留两个ID号来表示记录的“main”数据部分。
 *
 * The maximum is currently set at 32, quite arbitrarily. Most records only
 * need a handful of block references, but there are a few exceptions that
 * need more.
 * 目前的最大值是32，非常随意。
 * 大多数记录只需要少数块引用，但也有少数的例外，需要更多。
 */
#define XLR_MAX_BLOCK_ID            32

#define XLR_BLOCK_ID_DATA_SHORT     255
#define XLR_BLOCK_ID_DATA_LONG      254
#define XLR_BLOCK_ID_ORIGIN         253

//#endif                          /* XLOGRECORD_H */

/*
 * We don't store the whole fixed part (HeapTupleHeaderData) of an inserted
 * or updated tuple in WAL; we can save a few bytes by reconstructing the
 * fields that are available elsewhere in the WAL record, or perhaps just
 * plain needn't be reconstructed.  These are the fields we must store.
 * NOTE: t_hoff could be recomputed, but we may as well store it because
 * it will come for free due to alignment considerations.
 * PG不会在WAL中存储插入/更新的元组的全部固定部分(HeapTupleHeaderData);
 *   我们可以通过重新构造在WAL记录中可用的一些字段来节省一些空间,或者直接扁平化处理。
 * 这些都是我们必须存储的字段。
 * 注意:t_hoff可以重新计算，但我们也需要存储它，因为出于对齐的考虑,会被析构。
 */
typedef struct xl_heap_header
{
    uint16_t      t_infomask2;//t_infomask2标记
    uint16_t      t_infomask;//t_infomask标记
    uint8_t       t_hoff;//t_hoff
} xl_heap_header;
//HeapHeader的大小

#define SizeOfHeapHeader    (offsetof(xl_heap_header, t_hoff) + sizeof(uint8))

//7)    xl_heap_insert结构体定义
/*
 * xl_heap_insert/xl_heap_multi_insert flag values, 8 bits are available.
 */
/* PD_ALL_VISIBLE was cleared */
#define XLH_INSERT_ALL_VISIBLE_CLEARED          (1<<0)
#define XLH_INSERT_LAST_IN_MULTI                (1<<1)
#define XLH_INSERT_IS_SPECULATIVE               (1<<2)
#define XLH_INSERT_CONTAINS_NEW_TUPLE           (1<<3)

/* This is what we need to know about insert */
//这是在插入时需要获知的信息
typedef uint16_t OffsetNumber;
typedef struct xl_heap_insert
{
    //已成功插入的元组的偏移
    OffsetNumber offnum;        /* inserted tuple's offset */
    uint8_t       flags;            //标记

    /* xl_heap_header & TUPLE DATA in backup block 0 */
    //xl_heap_header & TUPLE DATA在备份块0中
} xl_heap_insert;
//xl_heap_insert大小
#define SizeOfHeapInsert    (offsetof(xl_heap_insert, flags) + sizeof(uint8))


#endif //MYSQL_REPLICATER_WALPROCESS_H
