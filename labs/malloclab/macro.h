#define WSIZE (sizeof(unsigned int)) // The size of single word as well as header or footer in bytes
#define DSIZE (2 * WSIZE) // The size of double word
#define CHUNKSIZE (1 << 12) // The minimal size needed for heap extension
#define MINBLOCKSIZE (4 * WSIZE)

#define MIN(x, y) ((x) < (y)? (x):(y))
#define MAX(x, y) ((x) > (y)? (x):(y))
#define PACK(size, alloc, prev_free) (size) | (alloc) | ((prev_free) << 1)// pack the size and alloc into a int

/* Read or write a word at address p */
#define GET(P) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Unpack (read) the size or alloc from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PFREE(p) (GET(p) & 0x2)

/* Given the block pointer bp, compute address of its header and footer */
#define HDRP(bp) ((char *) (bp) - WSIZE)
#define FTRP(bp) ((char *) (bp) + GET_SIZE(HDRP(bp)) - DSIZE)


/* Given a free block ptr fbp, compute address of next fbptr in the correspondint list */
#define NEXT_FREEP(fptr) (*((void **) (fptr)))
/* Put the address of next free ptr into address p */
#define PUT_FREEA(fptr, nfptr) (*((void **) (fptr)) = (nfptr))


/* Given a block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

