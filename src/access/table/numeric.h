//
// Created by 白杰 on 2024/12/31.
//

#ifndef MYSQL_REPLICATER_NUMERIC_H
#define MYSQL_REPLICATER_NUMERIC_H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <stdint.h>
#include "page.h"

using namespace std;

#define FLEXIBLE_ARRAY_MEMBER	/* empty */
/*-------------------------------------------------------------------------
 *
 * varatt.h
 *	  variable-length datatypes (TOAST support)
 *
 *
 * Portions Copyright (c) 1996-2024, PostgreSQL Global Development Group
 * Portions Copyright (c) 1995, Regents of the University of California
 *
 * src/include/varatt.h
 *
 *-------------------------------------------------------------------------
 */

/*
 * struct varatt_external is a traditional "TOAST pointer", that is, the
 * information needed to fetch a Datum stored out-of-line in a TOAST table.
 * The data is compressed if and only if the external size stored in
 * va_extinfo is less than va_rawsize - VARHDRSZ.
 *
 * This struct must not contain any padding, because we sometimes compare
 * these pointers using memcmp.
 *
 * Note that this information is stored unaligned within actual tuples, so
 * you need to memcpy from the tuple into a local struct variable before
 * you can look at these fields!  (The reason we use memcmp is to avoid
 * having to do that just to detect equality of two TOAST pointers...)
 */
typedef struct varatt_external
{
    uint32_t		va_rawsize;		/* Original data size (includes header) */
    uint32_t		va_extinfo;		/* External saved size (without header) and
								 * compression method */
    uint32_t			va_valueid;		/* Unique ID of value within TOAST table */
    uint32_t			va_toastrelid;	/* RelID of TOAST table containing it */
}			varatt_external;



/*
 * These macros define the "saved size" portion of va_extinfo.  Its remaining
 * two high-order bits identify the compression method.
 */
#define VARLENA_EXTSIZE_BITS	30
#define VARLENA_EXTSIZE_MASK	((1U << VARLENA_EXTSIZE_BITS) - 1)

/*
 * struct varatt_indirect is a "TOAST pointer" representing an out-of-line
 * Datum that's stored in memory, not in an external toast relation.
 * The creator of such a Datum is entirely responsible that the referenced
 * storage survives for as long as referencing pointer Datums can exist.
 *
 * Note that just as for struct varatt_external, this struct is stored
 * unaligned within any containing tuple.
 */
typedef struct varatt_indirect
{
    struct varlena *pointer;	/* Pointer to in-memory varlena */
}			varatt_indirect;

/*
 * struct varatt_expanded is a "TOAST pointer" representing an out-of-line
 * Datum that is stored in memory, in some type-specific, not necessarily
 * physically contiguous format that is convenient for computation not
 * storage.  APIs for this, in particular the definition of struct
 * ExpandedObjectHeader, are in src/include/utils/expandeddatum.h.
 *
 * Note that just as for struct varatt_external, this struct is stored
 * unaligned within any containing tuple.
 */
typedef struct ExpandedObjectHeader ExpandedObjectHeader;

typedef struct varatt_expanded
{
    ExpandedObjectHeader *eohptr;
} varatt_expanded;

/*
 * Type tag for the various sorts of "TOAST pointer" datums.  The peculiar
 * value for VARTAG_ONDISK comes from a requirement for on-disk compatibility
 * with a previous notion that the tag field was the pointer datum's length.
 */
typedef enum vartag_external
{
    VARTAG_INDIRECT = 1,
    VARTAG_EXPANDED_RO = 2,
    VARTAG_EXPANDED_RW = 3,
    VARTAG_ONDISK = 18
} vartag_external;

/* this test relies on the specific tag values above */
#define VARTAG_IS_EXPANDED(tag) \
	(((tag) & ~1) == VARTAG_EXPANDED_RO)

#define VARTAG_SIZE(tag) \
	((tag) == VARTAG_INDIRECT ? sizeof(varatt_indirect) : \
	 VARTAG_IS_EXPANDED(tag) ? sizeof(varatt_expanded) : \
	 (tag) == VARTAG_ONDISK ? sizeof(varatt_external) : \
	 (AssertMacro(false), 0))

/*
 * These structs describe the header of a varlena object that may have been
 * TOASTed.  Generally, don't reference these structs directly, but use the
 * macros below.
 *
 * We use separate structs for the aligned and unaligned cases because the
 * compiler might otherwise think it could generate code that assumes
 * alignment while touching fields of a 1-byte-header varlena.
 */
typedef union
{
    struct						/* Normal varlena (4-byte length) */
    {
        uint32_t		va_header;
        char		va_data[FLEXIBLE_ARRAY_MEMBER];
    }			va_4byte;
    struct						/* Compressed-in-line format */
    {
        uint32_t		va_header;
        uint32_t		va_tcinfo;	/* Original data size (excludes header) and
								 * compression method; see va_extinfo */
        char		va_data[FLEXIBLE_ARRAY_MEMBER]; /* Compressed data */
    }			va_compressed;
} varattrib_4b;

typedef struct
{
    uint8_t 		va_header;
    char		va_data[FLEXIBLE_ARRAY_MEMBER]; /* Data begins here */
} varattrib_1b;

/* TOAST pointers are a subset of varattrib_1b with an identifying tag byte */
typedef struct
{
    uint8_t		va_header;		/* Always 0x80 or 0x01 */
    uint8_t		va_tag;			/* Type of datum */
    char		va_data[FLEXIBLE_ARRAY_MEMBER]; /* Type-specific data */
} varattrib_1b_e;

static bool copyStringInitDone = false;
#define MaxAllocSize	((int) 0x3fffffff) /* 1 gigabyte - 1 */
#define PG_VERSION_NUM 160004

typedef struct StringInfoData
{
    char	   *data;
    int			len;
    int			maxlen;
    int			cursor;
} StringInfoData;

typedef StringInfoData *StringInfo;
static StringInfoData copyString;
typedef uint16_t NumericDigit;

struct NumericShort
{
    uint16_t          n_header;               /* Sign + display scale + weight */
    NumericDigit n_data[FLEXIBLE_ARRAY_MEMBER]; /* Digits */
};

struct NumericLong
{
    uint16_t          n_sign_dscale;  /* Sign + display scale */
    uint16_t           n_weight;               /* Weight of 1st digit  */
    NumericDigit n_data[FLEXIBLE_ARRAY_MEMBER]; /* Digits */
};

union NumericChoice
{
    uint16_t          n_header;               /* Header word */
    struct NumericLong n_long;      /* Long form (4-byte header) */
    struct NumericShort n_short;    /* Short form (2-byte header) */
};

struct NumericData
{
    union NumericChoice choice; /* choice of format */
};

struct NumericData;
typedef struct NumericData *Numeric;

typedef struct NumericVar
{
    int			ndigits;		/* # of digits in digits[] - can be 0! */
    int			weight;			/* weight of first digit */
    int			sign;			/* NUMERIC_POS, _NEG, _NAN, _PINF, or _NINF */
    int			dscale;			/* display scale */
    NumericDigit *buf;			/* start of palloc'd space for digits[] */
    NumericDigit *digits;		/* base-NBASE digits */
} NumericVar;



#define NBASE          10000
#define HALF_NBASE     5000
#define DEC_DIGITS     4                       /* decimal digits per NBASE digit */
#define MUL_GUARD_DIGITS       2       /* these are measured in NBASE digits */
#define DIV_GUARD_DIGITS       4

typedef uint16_t NumericDigit;

/*
 * Interpretation of high bits.
 */

#define NUMERIC_SIGN_MASK      0xC000
#define NUMERIC_POS                    0x0000
#define NUMERIC_NEG                    0x4000
#define NUMERIC_SHORT          0x8000
#define NUMERIC_SPECIAL                0xC000

#define NUMERIC_FLAGBITS(n) ((n)->choice.n_header & NUMERIC_SIGN_MASK)
#define NUMERIC_IS_SHORT(n)            (NUMERIC_FLAGBITS(n) == NUMERIC_SHORT)
#define NUMERIC_IS_SPECIAL(n)  (NUMERIC_FLAGBITS(n) == NUMERIC_SPECIAL)

#define NUMERIC_HDRSZ  (VARHDRSZ + sizeof(uint16) + sizeof(int16))
#define NUMERIC_HDRSZ_SHORT (VARHDRSZ + sizeof(uint16))
typedef enum ToastCompressionId:uint32_t
{
    TOAST_PGLZ_COMPRESSION_ID = 0,
    TOAST_LZ4_COMPRESSION_ID = 1,
    TOAST_INVALID_COMPRESSION_ID = 2
} ToastCompressionId;

#define VARATT_IS_1B_E(PTR) ((((varattrib_1b *) (PTR))->va_header) == 0x01)
#define VARATT_IS_EXTERNAL(PTR) VARATT_IS_1B_E(PTR)
#define VARATT_IS_EXTERNAL_ONDISK(PTR) \
	(VARATT_IS_EXTERNAL(PTR) && VARTAG_EXTERNAL(PTR) == VARTAG_ONDISK)


#define SET_VARSIZE_4B(PTR,len) \
	(((uint32_t ) (len)) << 2)
#define SET_VARSIZE_4B_C(PTR,len) \
	((((uint32_t) (len)) << 2) | 0x02)
#define SET_VARSIZE_1B(PTR,len) \
	((((uint8_t) (len)) << 1) | 0x01)
#define SET_VARTAG_1B_E(PTR,tag) \
	(va_header = 0x01, \
	 (va_tag = (tag))

#define VARATT_IS_4B(Len) \
	((Len & 0x01) == 0x00)
#define VARATT_IS_4B_U(Len) \
	((Len & 0x03) == 0x00)
#define VARATT_IS_4B_C(Len) \
	((Len & 0x03) == 0x02)
#define VARATT_IS_1B(Len) \
	((Len & 0x01) == 0x01)
#define VARATT_IS_1B_E(Len) \
	(Len == 0x01)



#define NUMERIC_SHORT_SIGN_MASK			0x2000
#define NUMERIC_SHORT_DSCALE_MASK		0x1F80
#define NUMERIC_SHORT_DSCALE_SHIFT		7
#define NUMERIC_SHORT_DSCALE_MAX		\
	(NUMERIC_SHORT_DSCALE_MASK >> NUMERIC_SHORT_DSCALE_SHIFT)
#define NUMERIC_SHORT_WEIGHT_SIGN_MASK	0x0040
#define NUMERIC_SHORT_WEIGHT_MASK		0x003F
#define NUMERIC_SHORT_WEIGHT_MAX		NUMERIC_SHORT_WEIGHT_MASK
#define NUMERIC_SHORT_WEIGHT_MIN		(-(NUMERIC_SHORT_WEIGHT_MASK+1))
//#define SET_VARSIZE_1B(PTR,len) \
//	(((varattrib_1b *) (PTR))->va_header = (((uint8) (len)) << 1) | 0x01)
#define SET_VARSIZE(PTR, len)				SET_VARSIZE_4B(PTR, len)

#define SET_VARSIZE_SHORT(PTR, len)			SET_VARSIZE_1B(PTR, len)
#define SET_VARSIZE_COMPRESSED(PTR, len)	SET_VARSIZE_4B_C(PTR, len)
#define MINS_PER_HOUR	60
#define SECS_PER_YEAR	(36525 * 864)	/* avoid floating-point computation */
#define SECS_PER_DAY	86400
#define SECS_PER_HOUR	3600
#define SECS_PER_MINUTE 60
#define POSTGRES_EPOCH_JDATE	2451545
#define DTERR_BAD_FORMAT 000
#define DTERR_TZDISP_OVERFLOW 002
#define MAX_TZDISP_HOUR		15
#define MINS_PER_HOUR	60


#define USECS_PER_SEC	INT64CONST(1000000)
#define USECS_PER_DAY	INT64CONST(86400000000)
#define pg_restrict __restrict


#define NBASE		10000
#define HALF_NBASE	5000
#define DEC_DIGITS	4			/* decimal digits per NBASE digit */
#define MUL_GUARD_DIGITS	2	/* these are measured in NBASE digits */
#define DIV_GUARD_DIGITS	4
#define PG_INT32_MAX	(0x7FFFFFFF)
#define digitbuf_alloc(ndigits)  \
	((NumericDigit *) malloc((ndigits) * sizeof(NumericDigit)))
#define digitbuf_free(buf)	\
	do { \
		 if ((buf) != NULL) \
			 free(buf); \
	} while (0)
#define VARHDRSZ		((uint32_t) sizeof(uint32_t))

#define init_var(v)		memset(v, 0, sizeof(NumericVar))

#define NUMERIC_DIGITS(num) (NUMERIC_HEADER_IS_SHORT(num) ? \
	(num)->choice.n_short.n_data : (num)->choice.n_long.n_data)
#define NUMERIC_NDIGITS(num) \
	((VARSIZE(num) - NUMERIC_HEADER_SIZE(num)) / sizeof(NumericDigit))
#define NUMERIC_CAN_BE_SHORT(scale,weight) \
	((scale) <= NUMERIC_SHORT_DSCALE_MAX && \
	(weight) <= NUMERIC_SHORT_WEIGHT_MAX && \
	(weight) >= NUMERIC_SHORT_WEIGHT_MIN)
#define NUMERIC_FLAGBITS(n) ((n)->choice.n_header & NUMERIC_SIGN_MASK)
#define NUMERIC_IS_SHORT(n)		(NUMERIC_FLAGBITS(n) == NUMERIC_SHORT)
#define NUMERIC_IS_SPECIAL(n)	(NUMERIC_FLAGBITS(n) == NUMERIC_SPECIAL)

#define NUMERIC_HDRSZ	(VARHDRSZ + sizeof(uint16_t) + sizeof(uint16_t))
#define NUMERIC_HDRSZ_SHORT (VARHDRSZ + sizeof(uint16_t))

#define NUMERIC_DSCALE_MASK			0x3FFF
#define NUMERIC_DSCALE_MAX			NUMERIC_DSCALE_MASK
#define FLEXIBLE_ARRAY_MEMBER	/* empty */

#define VARDATA_4B(PTR)		(((varattrib_4b *) (PTR))->va_4byte.va_data)
#define VARDATA_4B_C(PTR)	(((varattrib_4b *) (PTR))->va_compressed.va_data)
#define VARDATA_1B(PTR)		(((varattrib_1b *) (PTR))->va_data)
#define VARDATA_1B_E(PTR)	(((varattrib_1b_e *) (PTR))->va_data)

#define AssertMacro(condition)	((void)true)
#define VARTAG_1B_E(PTR) (((varattrib_1b_e *) (PTR))->va_tag)

#define VARTAG_EXTERNAL(PTR) VARTAG_1B_E(PTR)
#define VARTAG_IS_EXPANDED(tag) (((tag) & ~1) == VARTAG_EXPANDED_RO)

#define VARTAG_SIZE(tag) ((tag) == VARTAG_INDIRECT ? sizeof(varatt_indirect) : VARTAG_IS_EXPANDED(tag) ? sizeof(varatt_expanded) : (tag) == VARTAG_ONDISK ? sizeof(varatt_external) : (AssertMacro(false), 0))

#define VARHDRSZ_EXTERNAL offsetof(varattrib_1b_e, va_data)

#define VARSIZE_EXTERNAL(PTR)				(VARHDRSZ_EXTERNAL + VARTAG_SIZE(VARTAG_EXTERNAL(PTR)))

// 长度操作
#define VARSIZE_4B(Len) \
	((Len >> 2) & 0x3FFFFFFF)

#define VARSIZE_1B(Len) \
    ((Len >> 1) & 0x7F)

#define VARLENA_EXTSIZE_BITS 30


// 压缩用
#define VARHDRSZ_COMPRESSED		offsetof(varattrib_4b, va_compressed.va_data)
#define VARDATA_COMPRESSED_GET_EXTSIZE(PTR) \
	(((varattrib_4b *) (PTR))->va_compressed.va_tcinfo & VARLENA_EXTSIZE_MASK)
#define VARDATA_COMPRESSED_GET_COMPRESS_METHOD(PTR) \
	(((varattrib_4b *) (PTR))->va_compressed.va_tcinfo >> VARLENA_EXTSIZE_BITS)


/*
 * If the flag bits are NUMERIC_SHORT or NUMERIC_SPECIAL, we want the short
 * header; otherwise, we want the long one.  Instead of testing against each
 * value, we can just look at the high bit, for a slight efficiency gain.
 */
#define NUMERIC_HEADER_IS_SHORT(n)     (((n)->choice.n_header & 0x8000) != 0)
#define NUMERIC_HEADER_SIZE(n) \
       (sizeof(uint16_t) + \
        (NUMERIC_HEADER_IS_SHORT(n) ? 0 : sizeof(int16_t )))

/*
 * Definitions for special values (NaN, positive infinity, negative infinity).
 *
 * The two bits after the NUMERIC_SPECIAL bits are 00 for NaN, 01 for positive
 * infinity, 11 for negative infinity.  (This makes the sign bit match where
 * it is in a short-format value, though we make no use of that at present.)
 * We could mask off the remaining bits before testing the active bits, but
 * currently those bits must be zeroes, so masking would just add cycles.
 */
#define NUMERIC_EXT_SIGN_MASK  0xF000  /* high bits plus NaN/Inf flag bits */
#define NUMERIC_NAN                            0xC000
#define NUMERIC_PINF                   0xD000
#define NUMERIC_NINF                   0xF000
#define NUMERIC_INF_SIGN_MASK  0x2000

#define NUMERIC_EXT_FLAGBITS(n)        ((n)->choice.n_header & NUMERIC_EXT_SIGN_MASK)
#define NUMERIC_IS_NAN(n)              ((n)->choice.n_header == NUMERIC_NAN)
#define NUMERIC_IS_PINF(n)             ((n)->choice.n_header == NUMERIC_PINF)
#define NUMERIC_IS_NINF(n)             ((n)->choice.n_header == NUMERIC_NINF)
#define NUMERIC_IS_INF(n) \
       (((n)->choice.n_header & ~NUMERIC_INF_SIGN_MASK) == NUMERIC_PINF)

/*
 * Short format definitions.
 */

#define NUMERIC_SHORT_SIGN_MASK                        0x2000
#define NUMERIC_SHORT_DSCALE_MASK              0x1F80
#define NUMERIC_SHORT_DSCALE_SHIFT             7
#define NUMERIC_SHORT_DSCALE_MAX               \
       (NUMERIC_SHORT_DSCALE_MASK >> NUMERIC_SHORT_DSCALE_SHIFT)
#define NUMERIC_SHORT_WEIGHT_SIGN_MASK 0x0040
#define NUMERIC_SHORT_WEIGHT_MASK              0x003F
#define NUMERIC_SHORT_WEIGHT_MAX               NUMERIC_SHORT_WEIGHT_MASK
#define NUMERIC_SHORT_WEIGHT_MIN               (-(NUMERIC_SHORT_WEIGHT_MASK+1))

/*
 * Extract sign, display scale, weight.  These macros extract field values
 * suitable for the NumericVar format from the Numeric (on-disk) format.
 *
 * Note that we don't trouble to ensure that dscale and weight read as zero
 * for an infinity; however, that doesn't matter since we never convert
 * "special" numerics to NumericVar form.  Only the constants defined below
 * (const_nan, etc) ever represent a non-finite value as a NumericVar.
 */

#define NUMERIC_DSCALE_MASK                    0x3FFF
#define NUMERIC_DSCALE_MAX                     NUMERIC_DSCALE_MASK

#define NUMERIC_SIGN(n) \
       (NUMERIC_IS_SHORT(n) ? \
               (((n)->choice.n_short.n_header & NUMERIC_SHORT_SIGN_MASK) ? \
                NUMERIC_NEG : NUMERIC_POS) : \
               (NUMERIC_IS_SPECIAL(n) ? \
                NUMERIC_EXT_FLAGBITS(n) : NUMERIC_FLAGBITS(n)))
#define NUMERIC_DSCALE(n)      (NUMERIC_HEADER_IS_SHORT((n)) ? \
       ((n)->choice.n_short.n_header & NUMERIC_SHORT_DSCALE_MASK) \
               >> NUMERIC_SHORT_DSCALE_SHIFT \
       : ((n)->choice.n_long.n_sign_dscale & NUMERIC_DSCALE_MASK))
#define NUMERIC_WEIGHT(n)      (NUMERIC_HEADER_IS_SHORT((n)) ? \
       (((n)->choice.n_short.n_header & NUMERIC_SHORT_WEIGHT_SIGN_MASK ? \
               ~NUMERIC_SHORT_WEIGHT_MASK : 0) \
        | ((n)->choice.n_short.n_header & NUMERIC_SHORT_WEIGHT_MASK)) \
       : ((n)->choice.n_long.n_weight))

#if __GNUC__ >= 3
#define likely(x)	__builtin_expect((x) != 0, 1)
#define unlikely(x) __builtin_expect((x) != 0, 0)
#else
#define likely(x)	((x) != 0)
#define unlikely(x) ((x) != 0)
#endif

/*
 * Min
 *		Return the minimum of two numbers.
 */
#define Min(x, y)		((x) < (y) ? (x) : (y))

int parserNumericRelValue(Numeric, vector<uint8_t>&, const string&);
int CopyAppendNumericValue(const unsigned char *, int);
 void
CopyAppend(const char *str);
void
initStringInfo(StringInfo str);
void
resetStringInfo(StringInfo str);
void
appendStringInfoString(StringInfo str, const char *s);

static char * get_str_from_var(const NumericVar *);
int32_t pglz_decompress(const char *source, int32_t slen, unsigned char *dest,
                int32_t rawsize, bool check_complete);

static int
ReadStringFromToast(const char *buffer,
                    unsigned int buff_size,
                    unsigned int* out_size,
                    int (*parse_value)(const unsigned char *, int));

int varlenaExtractNumeric(vector<uint8_t>&, vector<uint8_t >&, uint32_t *, string);

#endif //MYSQL_REPLICATER_NUMERIC_H
