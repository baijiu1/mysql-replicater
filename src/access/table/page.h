//
// Created by 白杰 on 2024/12/16.
//

#ifndef MYSQL_REPLICATER_PAGE_H
#define MYSQL_REPLICATER_PAGE_H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>

using namespace std;

#define FLEXIBLE_ARRAY_MEMBER	/* empty */
const long osPagesize = sysconf(_SC_PAGE_SIZE);
#define _PAGESIZE osPagesize

#define MAXIMUM_ALIGNOF 4
#define TYPEALIGN(ALIGNVAL,LEN) \
    (((uintptr_t)(LEN) + ((ALIGNVAL) - 1)) & ~((uintptr_t)((ALIGNVAL) - 1)))

#define MAXALIGN(LEN) TYPEALIGN(MAXIMUM_ALIGNOF, (LEN))

/*
 * lp_flags has these possible states.	An UNUSED line pointer is available
 * for immediate re-use, the other states are not.
 */
#define LP_UNUSED 0   /* unused (should always have lp_len=0) */
#define LP_NORMAL 1   /* used (should always have lp_len>0) */
#define LP_REDIRECT 2 /* HOT redirect (should have lp_len=0) */
#define LP_DEAD 3     /* dead, may or may not have storage */

#define LP_INDEX_FROZEN 2 /* index tuple's xmin is frozen (used for multi-version btree index only) */

/*
pd_flags
*/
#define PD_HAS_FREE_LINES	0x0001	/* are there any unused line pointers? */
#define PD_PAGE_FULL		0x0002	/* not enough free space for new tuple? */
#define PD_ALL_VISIBLE		0x0004	/* all tuples on page are visible to
									 * everyone */

#define PD_VALID_FLAG_BITS	0x0007	/* OR of all valid pd_flags bits */


#define LocalTransactionId unsigned int
#define LWLockMode int
#define LOCKMODE int
#define BlockNumber unsigned int
//#define Oid unsigned int
#define ForkNumber int
//#define bool unsigned char


typedef struct {
    uint32_t xlogid;  /* high bits */
    uint32_t xrecoff; /* low bits */
} PageXLogRecPtr;

typedef struct ItemIdData {
    uint32_t lp_off; /* offset to tuple (from start of page) */
    uint32_t lp_flags;     /* state of item pointer, see below */
    uint32_t lp_len;      /* byte length of tuple */
} ItemIdData;

typedef ItemIdData* ItemId;
typedef uint16_t LocationIndex;
typedef uint64_t TransactionId;

typedef uint32_t ShortTransactionId;
typedef uint32_t CommandId;
typedef uint16_t OffsetNumber;
typedef unsigned char bits8;

typedef struct HeapTupleFields
{
    TransactionId t_xmin;		/* inserting xact ID */
    TransactionId t_xmax;		/* deleting or locking xact ID */

    union
    {
        CommandId	t_cid;		/* inserting or deleting command ID, or both */
        TransactionId t_xvac;	/* old-style VACUUM FULL xact ID */
    }			t_field3;
} HeapTupleFields;

typedef struct DatumTupleFields
{
    int32_t 		datum_len_;		/* varlena header (do not touch directly!) */

    int32_t		datum_typmod;	/* -1, or identifier of a record type */

    unsigned int			datum_typeid;	/* composite type OID, or RECORDOID */

    /*
     * datum_typeid cannot be a domain over composite, only plain composite,
     * even if the datum is meant as a value of a domain-over-composite type.
     * This is in line with the general principle that CoerceToDomain does not
     * change the physical representation of the base type value.
     *
     * Note: field ordering is chosen with thought that Oid might someday
     * widen to 64 bits.
     */
} DatumTupleFields;

typedef struct BlockIdData
{
    uint16_t 		bi_hi;
    uint16_t		bi_lo;
} BlockIdData;

typedef struct ItemPointerData
{
    BlockIdData ip_blkid;
    OffsetNumber ip_posid;
}

/* If compiler understands packed and aligned pragmas, use those */
#if defined(pg_attribute_packed) && defined(pg_attribute_aligned)
pg_attribute_packed()
			pg_attribute_aligned(2)
#endif
        ItemPointerData;






typedef struct HeapTupleHeader {
    union {
        HeapTupleFields t_heap;
        DatumTupleFields t_datum;
    } t_choice;

    ItemPointerData t_ctid; /* current TID of this or newer tuple */

    /* Fields below here must match MinimalTupleData! */

    uint16_t t_infomask2; /* number of attributes + various flags */

    uint16_t t_infomask; /* various flag bits, see below */

    uint8_t t_hoff; /* sizeof header incl. bitmap, padding */

    /* ^ - 23 bytes - ^ */

    bits8 t_bits[FLEXIBLE_ARRAY_MEMBER]; /* bitmap of NULLs -- VARIABLE LENGTH */

    /* MORE DATA FOLLOWS AT END OF STRUCT */
} HeapTupleHeaderData;
typedef HeapTupleHeader* HeapTupleHeaders;

typedef struct {
    /* XXX LSN is member of *any* block, not only page-organized ones */
    PageXLogRecPtr pd_lsn;    /* LSN: next byte after last byte of xlog
                               * record for last change to this page */
    uint16_t pd_checksum;       /* checksum */
    uint16_t pd_flags;          /* flag bits, see below */
    LocationIndex pd_lower;   /* offset to start of free space */
    LocationIndex pd_upper;   /* offset to end of free space */
    LocationIndex pd_special; /* offset to start of special space */
    uint16_t pd_pagesize_version;
    ShortTransactionId pd_prune_xid;           /* oldest prunable XID, or zero if none */
#ifdef OPENGAUSS
    TransactionId pd_xid_base;                 /* base value for transaction IDs on page */
    TransactionId pd_multi_base;               /* base value for multixact IDs on page */
#endif
//    ItemIdData pd_linp[FLEXIBLE_ARRAY_MEMBER]; /* beginning of line pointer array */
    vector<ItemIdData> pd_linp; /* beginning of line pointer array */

} HeapPageHeaderData;

typedef HeapPageHeaderData *HeapPageHeader;


typedef struct TableStateData {
    string tableName;
    string tableOID;

} TableStateData;

typedef TableStateData *TableState;

typedef struct HeapTupleData
{
    uint32_t    t_len;   /* length of *t_data */
    ItemPointerData  t_self;   /* SelfItemPointer */
    unsigned int     t_tableOid;  /* table the tuple came from */
#define FIELDNO_HEAPTUPLEDATA_DATA 3
    vector<HeapTupleHeaderData> t_data; /* -> tuple data */
//    HeapTupleHeaders  t_header;   /* -> tuple header */

} HeapTupleData;

typedef HeapTupleData *HeapTuple;


int getHeapHeader();


typedef struct FilenodeMapEntry {
    uint32_t relid;        /* 对应表的 pg_class.oid 值 */
    uint32_t relfilenode;  /* 表的 relfilenode 值 */
} FilenodeMapEntry;

#define NAMEDATALEN 64

// pg_class
typedef struct {
    /* oid */
    uint32_t			oid;
    /* class name */
    string 	relname;
    /* OID of namespace containing this class */
    uint32_t			relnamespace ;
    /* OID of entry in pg_type for relation's implicit row type, if any */
    uint32_t			reltype ;
    /* OID of entry in pg_type for underlying composite type, if any */
    uint32_t			reloftype ;
    /* class owner */
    uint32_t			relowner ;
    /* access method; 0 if not a table / index */
    uint32_t			relam ;
    /* identifier of physical storage file */
    /* relfilenode == 0 means it is a "mapped" relation, see relmapper.c */
    uint32_t			relfilenode;
    /* identifier of table space for relation (0 means default for database) */
    uint32_t			reltablespace ;
    /* # of blocks (not always up-to-date) */
    uint32_t 		relpages ;
    /* # of tuples (not always up-to-date; -1 means "unknown") */
    float 		reltuples ;
    /* # of all-visible blocks (not always up-to-date) */
    uint32_t		relallvisible ;
    /* OID of toast table; 0 if none */
    uint32_t			reltoastrelid ;
    /* T if has (or has had) any indexes */
    bool		relhasindex ;
    /* T if shared across databases */
    bool		relisshared ;
    /* see RELPERSISTENCE_xxx constants below */
    char		relpersistence ;
    /* see RELKIND_xxx constants below */
    char		relkind ;
    /* number of user attributes */
    uint16_t		relnatts ;	/* genbki.pl will fill this in */
    /*
     * Class pg_attribute must contain exactly "relnatts" user attributes
     * (with attnums ranging from 1 to relnatts) for this class.  It may also
     * contain entries with negative attnums for system attributes.
     */
    /* # of CHECK constraints for class */
    uint16_t		relchecks ;
    /* has (or has had) any rules */
    bool		relhasrules ;
    /* has (or has had) any TRIGGERs */
    bool		relhastriggers ;
    /* has (or has had) child tables or indexes */
    bool		relhassubclass ;
    /* row security is enabled or not */
    bool		relrowsecurity ;
    /* row security forced for owners or not */
    bool		relforcerowsecurity ;
    /* matview currently holds query results */
    bool		relispopulated ;
    /* see REPLICA_IDENTITY_xxx constants */
    char		relreplident ;
    /* is relation a partition? */
    bool		relispartition ;
    /* link to original rel during table rewrite; otherwise 0 */
    uint32_t			relrewrite ;
    /* all Xids < this are frozen in this rel */
    uint64_t relfrozenxid ;	/* FirstNormalTransactionId */
    /* all multixacts in this rel are >= this; it is really a MultiXactId */
    uint64_t relminmxid ;	/* FirstMultiXactId */
#ifdef CATALOG_VARLEN			/* variable-length fields start here */
    /* NOTE: These fields are not present in a relcache entry's rd_rel field. */
	/* access permissions */
	aclitem		relacl[1] BKI_DEFAULT(_null_);
	/* access-method-specific options */
	text		reloptions[1] BKI_DEFAULT(_null_);
	/* partition bound node tree */
	pg_node_tree relpartbound BKI_DEFAULT(_null_);
#endif
} FormData_pg_class;

typedef FormData_pg_class *Form_pg_class;



// pg_attribute
typedef struct {
    uint32_t			attrelid;	/* OID of relation containing
												 * this attribute */
    string 	attname;		/* name of attribute */

/*
 * atttypid is the OID of the instance in Catalog Class pg_type that
 * defines the data type of this attribute (e.g. int4).  Information in
 * that instance is redundant with the attlen, attbyval, and attalign
 * attributes of this instance, so they had better match or Postgres will
 * fail.  In an entry for a dropped column, this field is set to zero
 * since the pg_type entry may no longer exist; but we rely on attlen,
 * attbyval, and attalign to still tell us how large the values in the
 * table are.
 */
    uint32_t			atttypid ;

/*
 * attlen is a copy of the typlen field from pg_type for this attribute.
 * See atttypid comments above.
 */
    uint16_t 		attlen;

/*
 * attnum is the "attribute number" for the attribute:	A value that
 * uniquely identifies this attribute within its class. For user
 * attributes, Attribute numbers are greater than 0 and not greater than
 * the number of attributes in the class. I.e. if the Class pg_class says
 * that Class XYZ has 10 attributes, then the user attribute numbers in
 * Class pg_attribute must be 1-10.
 *
 * System attributes have attribute numbers less than 0 that are unique
 * within the class, but not constrained to any particular range.
 *
 * Note that (attnum - 1) is often used as the index to an array.
 */
    uint16_t		attnum;

/*
 * fastgetattr() uses attcacheoff to cache byte offsets of attributes in
 * heap tuples.  The value actually stored in pg_attribute (-1) indicates
 * no cached value.  But when we copy these tuples into a tuple
 * descriptor, we may then update attcacheoff in the copies. This speeds
 * up the attribute walking process.
 */
    uint32_t		attcacheoff ;

/*
 * atttypmod records type-specific data supplied at table creation time
 * (for example, the max length of a varchar field).  It is passed to
 * type-specific input and output functions as the third argument. The
 * value will generally be -1 for types that do not need typmod.
 */
    uint32_t		atttypmod ;

/*
 * attndims is the declared number of dimensions, if an array type,
 * otherwise zero.
 */
    uint16_t		attndims;

/*
 * attbyval is a copy of the typbyval field from pg_type for this
 * attribute.  See atttypid comments above.
 */
bool		attbyval;

/*
 * attalign is a copy of the typalign field from pg_type for this
 * attribute.  See atttypid comments above.
 */
char		attalign;

/*----------
 * attstorage tells for VARLENA attributes, what the heap access
 * methods can do to it if a given tuple doesn't fit into a page.
 * Possible values are as for pg_type.typstorage (see TYPSTORAGE macros).
 *----------
 */
char		attstorage;

/*
 * attcompression sets the current compression method of the attribute.
 * Typically this is InvalidCompressionMethod ('\0') to specify use of the
 * current default setting (see default_toast_compression).  Otherwise,
 * 'p' selects pglz compression, while 'l' selects LZ4 compression.
 * However, this field is ignored whenever attstorage does not allow
 * compression.
 */
char		attcompression ;

/* This flag represents the "NOT NULL" constraint */
bool		attnotnull;

/* Has DEFAULT value or not */
bool		atthasdef ;

/* Has a missing value or not */
bool		atthasmissing ;

/* One of the ATTRIBUTE_IDENTITY_* constants below, or '\0' */
char		attidentity ;

/* One of the ATTRIBUTE_GENERATED_* constants below, or '\0' */
char		attgenerated ;

/* Is dropped (ie, logically invisible) or not */
bool		attisdropped ;

/*
 * This flag specifies whether this column has ever had a local
 * definition.  It is set for normal non-inherited columns, but also for
 * columns that are inherited from parents if also explicitly listed in
 * CREATE TABLE INHERITS.  It is also set when inheritance is removed from
 * a table with ALTER TABLE NO INHERIT.  If the flag is set, the column is
 * not dropped by a parent's DROP COLUMN even if this causes the column's
 * attinhcount to become zero.
 */
bool		attislocal ;

/* Number of times inherited from direct parent relation(s) */
    uint16_t		attinhcount ;

/* attribute's collation, if any */
    uint32_t			attcollation ;

#ifdef CATALOG_VARLEN			/* variable-length/nullable fields start here */
/* NOTE: The following fields are not present in tuple descriptors. */

	/*
	 * attstattarget is the target number of statistics datapoints to collect
	 * during VACUUM ANALYZE of this column.  A zero here indicates that we do
	 * not wish to collect any stats about this column. A null value here
	 * indicates that no value has been explicitly set for this column, so
	 * ANALYZE should use the default setting.
	 *
	 * int16 is sufficient for the current max value (MAX_STATISTICS_TARGET).
	 */
	int16		attstattarget BKI_DEFAULT(_null_) BKI_FORCE_NULL;

	/* Column-level access permissions */
	aclitem		attacl[1] BKI_DEFAULT(_null_);

	/* Column-level options */
	text		attoptions[1] BKI_DEFAULT(_null_);

	/* Column-level FDW options */
	text		attfdwoptions[1] BKI_DEFAULT(_null_);

	/*
	 * Missing value for added columns. This is a one element array which lets
	 * us store a value of the attribute type here.
	 */
	anyarray	attmissingval BKI_DEFAULT(_null_);
#endif
} FormData_pg_attribute;

typedef FormData_pg_attribute *Form_pg_attribute;


typedef struct NullableDatum
{
#define FIELDNO_NULLABLE_DATUM_DATUM 0
    uintptr_t		value;
#define FIELDNO_NULLABLE_DATUM_ISNULL 1
    bool		isnull;
    /* due to alignment padding this could be used for flags for free */
} NullableDatum;

typedef struct FormExtraData_pg_attribute
{
    NullableDatum attstattarget;
    NullableDatum attoptions;
} FormExtraData_pg_attribute;



typedef struct {
uint32_t 			oid;			/* oid */

/* type name */
string 	typname;

/* OID of namespace containing this type */
    uint32_t			typnamespace;

/* type owner */
    uint32_t			typowner;

/*
 * For a fixed-size type, typlen is the number of bytes we use to
 * represent a value of this type, e.g. 4 for an int4.  But for a
 * variable-length type, typlen is negative.  We use -1 to indicate a
 * "varlena" type (one that has a length word), -2 to indicate a
 * null-terminated C string.
 */
uint16_t 		typlen ;

/*
 * typbyval determines whether internal Postgres routines pass a value of
 * this type by value or by reference.  typbyval had better be false if
 * the length is not 1, 2, or 4 (or 8 on 8-byte-Datum machines).
 * Variable-length types are always passed by reference. Note that
 * typbyval can be false even if the length would allow pass-by-value; for
 * example, type macaddr8 is pass-by-ref even when Datum is 8 bytes.
 */
bool		typbyval ;

/*
 * typtype is 'b' for a base type, 'c' for a composite type (e.g., a
 * table's rowtype), 'd' for a domain, 'e' for an enum type, 'p' for a
 * pseudo-type, or 'r' for a range type. (Use the TYPTYPE macros below.)
 *
 * If typtype is 'c', typrelid is the OID of the class' entry in pg_class.
 */
char		typtype ;

/*
 * typcategory and typispreferred help the parser distinguish preferred
 * and non-preferred coercions.  The category can be any single ASCII
 * character (but not \0).  The categories used for built-in types are
 * identified by the TYPCATEGORY macros below.
 */

/* arbitrary type classification */
char		typcategory ;

/* is type "preferred" within its category? */
bool		typispreferred ;

/*
 * If typisdefined is false, the entry is only a placeholder (forward
 * reference).  We know the type's name and owner, but not yet anything
 * else about it.
 */
bool		typisdefined ;

/* delimiter for arrays of this type */
char		typdelim ;

/* associated pg_class OID if a composite type, else 0 */
    uint32_t			typrelid ;

/*
 * Type-specific subscripting handler.  If typsubscript is 0, it means
 * that this type doesn't support subscripting.  Note that various parts
 * of the system deem types to be "true" array types only if their
 * typsubscript is array_subscript_handler.
 */
    uint32_t		typsubscript ;

/*
 * If typelem is not 0 then it identifies another row in pg_type, defining
 * the type yielded by subscripting.  This should be 0 if typsubscript is
 * 0.  However, it can be 0 when typsubscript isn't 0, if the handler
 * doesn't need typelem to determine the subscripting result type.  Note
 * that a typelem dependency is considered to imply physical containment
 * of the element type in this type; so DDL changes on the element type
 * might be restricted by the presence of this type.
 */
    uint32_t			typelem ;

/*
 * If there is a "true" array type having this type as element type,
 * typarray links to it.  Zero if no associated "true" array type.
 */
    uint32_t			typarray ;

/*
 * I/O conversion procedures for the datatype.
 */

/* text format (required) */
    uint32_t		typinput ;
    uint32_t		typoutput;

/* binary format (optional) */
    uint32_t		typreceive ;
    uint32_t		typsend ;

/*
 * I/O functions for optional type modifiers.
 */
    uint32_t		typmodin ;
    uint32_t		typmodout;

/*
 * Custom ANALYZE procedure for the datatype (0 selects the default).
 */
    uint32_t		typanalyze ;

/* ----------------
 * typalign is the alignment required when storing a value of this
 * type.  It applies to storage on disk as well as most
 * representations of the value inside Postgres.  When multiple values
 * are stored consecutively, such as in the representation of a
 * complete row on disk, padding is inserted before a datum of this
 * type so that it begins on the specified boundary.  The alignment
 * reference is the beginning of the first datum in the sequence.
 *
 * 'c' = CHAR alignment, ie no alignment needed.
 * 's' = SHORT alignment (2 bytes on most machines).
 * 'i' = INT alignment (4 bytes on most machines).
 * 'd' = DOUBLE alignment (8 bytes on many machines, but by no means all).
 * (Use the TYPALIGN macros below for these.)
 *
 * See include/access/tupmacs.h for the macros that compute these
 * alignment requirements.  Note also that we allow the nominal alignment
 * to be violated when storing "packed" varlenas; the TOAST mechanism
 * takes care of hiding that from most code.
 *
 * NOTE: for types used in system tables, it is critical that the
 * size and alignment defined in pg_type agree with the way that the
 * compiler will lay out the field in a struct representing a table row.
 * ----------------
 */
char		typalign;

/* ----------------
 * typstorage tells if the type is prepared for toasting and what
 * the default strategy for attributes of this type should be.
 *
 * 'p' PLAIN	  type not prepared for toasting
 * 'e' EXTERNAL   external storage possible, don't try to compress
 * 'x' EXTENDED   try to compress and store external if required
 * 'm' MAIN		  like 'x' but try to keep in main tuple
 * (Use the TYPSTORAGE macros below for these.)
 *
 * Note that 'm' fields can also be moved out to secondary storage,
 * but only as a last resort ('e' and 'x' fields are moved first).
 * ----------------
 */
char		typstorage ;

/*
 * This flag represents a "NOT NULL" constraint against this datatype.
 *
 * If true, the attnotnull column for a corresponding table column using
 * this datatype will always enforce the NOT NULL constraint.
 *
 * Used primarily for domain types.
 */
bool		typnotnull ;

/*
 * Domains use typbasetype to show the base (or domain) type that the
 * domain is based on.  Zero if the type is not a domain.
 */
    uint32_t			typbasetype ;

/*
 * Domains use typtypmod to record the typmod to be applied to their base
 * type (-1 if base type does not use a typmod).  -1 if this type is not a
 * domain.
 */
    uint32_t		typtypmod ;

/*
 * typndims is the declared number of dimensions for an array domain type
 * (i.e., typbasetype is an array type).  Otherwise zero.
 */
    uint32_t		typndims ;

/*
 * Collation: 0 if type cannot use collations, nonzero (typically
 * DEFAULT_COLLATION_OID) for collatable base types, possibly some other
 * OID for domains over collatable types
 */
    uint32_t			typcollation ;

#ifdef CATALOG_VARLEN			/* variable-length fields start here */

/*
	 * If typdefaultbin is not NULL, it is the nodeToString representation of
	 * a default expression for the type.  Currently this is only used for
	 * domains.
	 */
	pg_node_tree typdefaultbin BKI_DEFAULT(_null_) BKI_ARRAY_DEFAULT(_null_);

	/*
	 * typdefault is NULL if the type has no associated default value. If
	 * typdefaultbin is not NULL, typdefault must contain a human-readable
	 * version of the default expression represented by typdefaultbin. If
	 * typdefaultbin is NULL and typdefault is not, then typdefault is the
	 * external representation of the type's default value, which may be fed
	 * to the type's input converter to produce a constant.
	 */
	text		typdefault BKI_DEFAULT(_null_) BKI_ARRAY_DEFAULT(_null_);

	/*
	 * Access permissions
	 */
	aclitem		typacl[1] BKI_DEFAULT(_null_);
#endif
} FormData_pg_type;

typedef FormData_pg_type *Form_pg_type;

// 整个表的属性
typedef struct {
    uint32_t oid;
    string tableName;
    uint32_t filenode;
    uint32_t columnNum; // 第几个列
    string columnName;  // 列名
    string columnTypeName;
    uint32_t columnTypeId; // 对应的到pg_type表
    uint32_t columnLength; // 对于固定长度数据类型（如 int、char），它表示字节数。对于变长数据类型（如 text），此字段为 -1。
    uint32_t columnTypeMod; // 数据类型的修饰符，通常用于表示长度限制（如 varchar(50) 中的 50）。
//    vector<uint8_t > columnNdims; // 列的数据维度数。对于普通标量类型，该值为 0；对于数组类型，则是数组的维度数。
//    vector<bool> columnByVal; // 如果列的数据类型按值存储，值为 true，否则为 false。例如，int4 是按值存储的。
//    vector<unsigned char> columnAlign; // 列的对齐方式。常见的值包括：	•	'c'：按字符对齐
    //    'd'：按双字对齐
    //    'i'：按整数对齐
    //    'm'：按更大数据类型对齐
    //    'p'：按指针对齐
    uint8_t columnStorage; // 列的数据存储方式。值包括：
    //    'p'：主存储方式
    //    'm'：压缩存储
    //    'x'：外部存储
    uint8_t columnCompression; // 列的压缩方式。值为 'p'（默认）和 'm'，表示是否进行数据压缩。
    bool columnNotNull; // 是否为 NOT NULL 列。如果列定义了 NOT NULL 约束，则该字段为 true，否则为 false。
    bool columnHasDefault; // 如果列有默认值，则为 true，否则为 false。
//    vector<bool> columnHasmissing; // 是否存在缺失值。如果列有缺失值（用于某些特定列类型，如 JSONB 等），则为 true。
//    vector<unsigned char> columnIdentity; // 列的身份属性类型，表示该列是否是身份列（'a'：自动生成、'd'：显式定义、's'：序列）。
//    vector<unsigned char> columnGenerated; // 列的生成类型。表示列是否是通过 GENERATED 语法定义的自动生成列。
    bool columnIsdropped; // 如果列已被删除，则为 true，否则为 false。此字段通常用于表示逻辑删除的列。
    bool columnIslocal; // 如果列是表的本地列而不是继承列，则为 true，否则为 false。
    uint16_t columnInhcount; // 列继承的次数。如果列是从父表继承的，这个值大于零。
    uint32_t columnCollation; // 列的排序规则 OID，指向 pg_collation 表中的 oid 字段。对于字符类型列，指定排序规则。
//    vector<uint8_t> columnStattarget; // 表示统计目标，通常与列的分析级别相关，用于 PostgreSQL 的查询优化器。


} tableStructData;

typedef tableStructData* tableStruct;


// 整个表的属性
typedef struct tableConstruct{
    string oid;
    string tableName;
    string relFilenode;
    string filenodePath;
    string tableSize;
    string columnNum; // 第几个列
    string columnName;  // 列名
    string columnTypeName;
    string columnTypeId; // 对应的到pg_type表
    string columnLength; // 对于固定长度数据类型（如 int、char），它表示字节数。对于变长数据类型（如 text），此字段为 -1。
    string columnTypeMod; // 数据类型的修饰符，通常用于表示长度限制（如 varchar(50) 中的 50）。
//    vector<uint8_t > columnNdims; // 列的数据维度数。对于普通标量类型，该值为 0；对于数组类型，则是数组的维度数。
//    vector<bool> columnByVal; // 如果列的数据类型按值存储，值为 true，否则为 false。例如，int4 是按值存储的。
//    vector<unsigned char> columnAlign; // 列的对齐方式。常见的值包括：	•	'c'：按字符对齐
    string columnAttalign;
    //    'd'：按双字对齐
    //    'i'：按整数对齐
    //    'm'：按更大数据类型对齐
    //    'p'：按指针对齐
    string columnStorage; // 列的数据存储方式。值包括：
    //    'p'：主存储方式
    //    'm'：压缩存储
    //    'x'：外部存储
    string columnCompression; // 列的压缩方式。值为 'p'（默认）和 'm'，表示是否进行数据压缩。
    string columnNotNull; // 是否为 NOT NULL 列。如果列定义了 NOT NULL 约束，则该字段为 true，否则为 false。
    string columnHasDefault; // 如果列有默认值，则为 true，否则为 false。
//    vector<bool> columnHasmissing; // 是否存在缺失值。如果列有缺失值（用于某些特定列类型，如 JSONB 等），则为 true。
//    vector<unsigned char> columnIdentity; // 列的身份属性类型，表示该列是否是身份列（'a'：自动生成、'd'：显式定义、's'：序列）。
//    vector<unsigned char> columnGenerated; // 列的生成类型。表示列是否是通过 GENERATED 语法定义的自动生成列。
    string columnIsdropped; // 如果列已被删除，则为 true，否则为 false。此字段通常用于表示逻辑删除的列。
    string columnIslocal; // 如果列是表的本地列而不是继承列，则为 true，否则为 false。
    string columnInhcount; // 列继承的次数。如果列是从父表继承的，这个值大于零。
    string columnCollation; // 列的排序规则 OID，指向 pg_collation 表中的 oid 字段。对于字符类型列，指定排序规则。
//    vector<uint8_t> columnStattarget; // 表示统计目标，通常与列的分析级别相关，用于 PostgreSQL 的查询优化器。


} tableConstructData;

//typedef tableConstructData* tableConstruct;

// 每行数据填充到t_data里，每行数据的t_xmin和t_xmax填充，一一对应
typedef struct finalTableStruct{
    vector<string > t_data;
    uint32_t t_xmin;
    uint32_t t_xmax;
    struct finalTableStruct *next;

} finalTableData;


#endif //MYSQL_REPLICATER_PAGE_H
