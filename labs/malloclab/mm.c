/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* the macro rely heavily on the fact sizeof(void **) == 4, any modification to the code will crash the code*/
#define WSIZE 4 // The size of single word as well as header or footer in bytes
#define DSIZE 8 // The size of double word
#define CHUNKSIZE (1 << 12) // The minimal size needed for heap extension
#define MINBSIZE (4 * WSIZE) // The minimal block size for allocated or free block, which is two word for header and footer and other two for next fptr and previous fptr
#define MAX_LIST_SIZE 8 // the maximal list size

#define MIN(x, y) ((x) < (y)? (x):(y))
#define MAX(x, y) ((x) > (y)? (x):(y))
#define PACK(size, alloc, prev_alloc) (size) | (alloc) | ((prev_alloc) << 1)// pack the size and alloc into a int

/* Read or write a word at address p */
#define GET(p) (*(uint32_t *)(p))
#define PUT(p, val) (*(uint32_t *)(p) = (val))

/* Unpack (read) the size or alloc from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PALLOC(p) ((GET(p) >> 1) & 0x1) 

/* Given the block pointer bp, compute address of its header and footer */
#define HDRP(bp) ((char *) (bp) - WSIZE)
#define FTRP(bp) ((char *) (bp) + GET_SIZE(HDRP(bp)) - DSIZE)


/* Given a free block ptr fptr, compute address of next fbptr in the correspondint list */
#define NEXT_FREEP(fptr) (*((void **) (fptr)))
/* Given a free block ptr fptr, compute the previous fptr address in the list */
#define PREV_FREEP(fptr) (*((void **) (fptr) + 1))
/* Put the address of next free ptr into address p */
#define PUT_N_FREEA(fptr, nfptr) (*((void **) (fptr)) = (nfptr))
/* Put the address of previous free ptr in to adddres p */
#define PUT_P_FREEA(fptr, pfptr) (*((void **) (fptr) + 1) = (pfptr))


/* Given a block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) // this is only vaild if previous block is sure to be a free block

/* Given a mallocated block, alter its header to change the status of whether previous block is allocated */
#define PUT_P_ALLOC(bp) (PUT(HDRP(bp), GET(HDRP(bp)) | 0x2))
#define PUT_P_FREE(bp) (PUT(HDRP(bp), GET(HDRP(bp)) & ~0x2))


static void **free_lists_p = NULL;
static char *prologue = NULL; // always pointered to the Prologue
static char *endlogue = NULL;   // always pointered to the Endlogue

static void *extend_heap(uint32_t size);  // extend the heap size and coalesce then place the newly crteated free block into proper list
static void *coalesce(void *fbp); // coalesce the block pointer and the result free block is unattached (but prev free block and next block is NULL)
static void place(void *bp, uint32_t asize); // place the header of a mallocated block with size asize at block ptr bp
static void place_free(void *fbp); // place an unattached free block into appropriate free block list
static void *remove_free(void *fbp); // remove a free block from its list reside in, return an unattached free block
static uint32_t compute_index(uint32_t size); // Given a free block ptr, compute the index of the free lists it belongs to
static void *find_fit(uint32_t size); // find first free block in the list fits the size

/* test suit */
static void check_no_unattached_free_block(void);
static void check_no_adjacent_free_block(void);
static void check_correct_prev_alloc_flag(void);
static void check_minimal_block_size(void);

static void check_no_unattached_free_block(void) {
    void *bp = prologue;
    uint32_t is_alloc;
    uint32_t size;
    int flag;
    while ((size = GET_SIZE(HDRP(bp))) > 0) {
        flag = 0;
        if (!(is_alloc = GET_ALLOC(HDRP(bp)))) {
            uint32_t index = compute_index(size);
            void *fbp = free_lists_p[index];
            while (fbp != NULL) {
                if (fbp == bp) {
                    flag = 1;
                }
                fbp = NEXT_FREEP(fbp);
            }
            if (!flag) {
                printf("Error: unattached free block detected!");
                return;
            }
        }
        bp = NEXT_BLKP(bp);
    }
}

static void check_no_adjacent_free_block(void) {
    void *pbp = prologue;
    void *bp = NEXT_BLKP(pbp);
    uint32_t p_alloc, n_alloc, alloc;
    void *nbp;
    while (GET_SIZE(HDRP(bp)) > 0) {
        nbp = NEXT_BLKP(bp);
        alloc = GET_ALLOC(HDRP(bp));
        if (!alloc) {
            p_alloc = GET_ALLOC(HDRP(pbp));
            n_alloc = GET_ALLOC(HDRP(nbp));
            if (!p_alloc || !n_alloc || !GET_PALLOC(HDRP(bp))) {
                printf("Error: adjacent free block is detected!");
                return;
            }
        }
        pbp = bp;
        bp = NEXT_BLKP(pbp);
    }
}

static void check_correct_prev_alloc_flag() {
    void *pbp = prologue;
    void *bp = NEXT_BLKP(pbp);
    uint32_t size;
    while ((size = GET_SIZE(HDRP(bp))) > 0) {
        if (GET_PALLOC(HDRP(bp)) != GET_ALLOC(HDRP(pbp))) {
            printf("A block with inconsitent prev_alloc flag is detected!");
            return;
        }
        pbp = bp;
        bp = NEXT_BLKP(pbp);
    }
}

static void check_minimal_block_size() {
    void *bp = prologue;
    uint32_t size;
    while ((size = GET_SIZE(HDRP(bp))) > 0) {
        if (size < MINBSIZE || (bp && 0x7) != 0) {
            printf("Detected a block with size less then minimal block size");
            return;
        }
        bp = NEXT_BLKP(bp);
    }
}

/* place a mallocated block at the unattached free block fbp, 
 * it will mallocated the whole block if remaining size is smaller then minimal free block size, 
 * a new free block maybe created, and it will be placed into appropriate list
 */
static void place(void *fbp, uint32_t asize) {
    uint32_t csize = GET_SIZE(HDRP(fbp));
    void *n_bp;
    if (csize - asize < MINBSIZE) {
        PUT(HDRP(fbp), PACK(csize, 1, 1));
        n_bp = NEXT_BLKP(fbp);
        PUT_P_ALLOC(n_bp); // change the header of the next block, which is mallocated, to show previous block is mallocated
    } else {
        PUT(HDRP(fbp), PACK(asize, 1, 1));
        n_bp = NEXT_BLKP(fbp);
        PUT(HDRP(n_bp), PACK(csize - asize, 0, 1));
        PUT(FTRP(n_bp), PACK(csize - asize, 0, 1));
        PUT_N_FREEA(n_bp, NULL);
        PUT_P_FREEA(n_bp, NULL);
        place_free(n_bp);
        PUT_P_ALLOC(NEXT_BLKP(n_bp));
    }
}

/* find the first free block in the list fits the size, the NULL is returned if there is no fits */
static void *find_fit(uint32_t size) {
    uint32_t start_index = compute_index(size);
    for (int i = start_index; i < MAX_LIST_SIZE; ++i) {
        void *fbp = free_lists_p[i];
        while (fbp != NULL) {
            if (GET_SIZE(HDRP(fbp)) >= size) {
                return fbp;
            }
            fbp = NEXT_FREEP(fbp);
        }
    }
    return NULL;
}
/* coalesce a unattached free block, return a coalesced unattached free block (ptr)*/
static void *coalesce(void *fbp) {
    void *n_bp = NEXT_BLKP(fbp); // the block ptr of the next block 
    uint32_t prev_alloc = GET_PALLOC(HDRP(fbp));
    uint32_t next_alloc = GET_ALLOC(HDRP(n_bp));
    uint32_t size = GET_SIZE(HDRP(fbp));
    void *p_fbp; // will be assigned to the block ptr of previous block if the previous block is free
    if (!prev_alloc && !next_alloc) {
        p_fbp = PREV_BLKP(fbp);
        /* remove both free blocks from free lists */
        remove_free(p_fbp);
        remove_free(n_bp);

        /* update the header and the footer of the newly coalesced free block */
        size += (GET_SIZE(HDRP(p_fbp)) + GET_SIZE(HDRP(n_bp)));
        fbp = p_fbp;
        PUT(HDRP(fbp), PACK(size, 0, 1));
        PUT(FTRP(fbp), PACK(size, 0, 1));
    }

    if (!prev_alloc && next_alloc) {
        p_fbp = PREV_BLKP(fbp);
        /* remove the previous free block from list */
        remove_free(p_fbp);

        /* update the header and the footer of the newly coalesced free block */
        size += (GET_SIZE(HDRP(p_fbp)));
        fbp = p_fbp;
        PUT(HDRP(fbp), PACK(size, 0, 1));
        PUT(FTRP(fbp), PACK(size, 0, 1));
    }

    if (prev_alloc && !next_alloc) {
        /* remove the next free block from list */
        remove_free(n_bp);
        
        /* update the header and the footer of newly coalesced free block */
        size += (GET_SIZE(HDRP(n_bp)));
        PUT(HDRP(fbp), PACK(size, 0, 1));
        PUT(FTRP(fbp), PACK(size, 0, 1));
    }

    /* put the free block into unattached state */
    PUT_P_FREEA(fbp, NULL);
    PUT_N_FREEA(fbp, NULL);

    /* update the new next block status field, which is assumed to be mallocated */
    n_bp = NEXT_BLKP(fbp);
    PUT_P_FREE(n_bp);

    return fbp;
}

/* place a detached free block into the head of proper free list */
static void place_free(void *fbp) {
    uint32_t index = compute_index(GET_SIZE(HDRP(fbp)));
    void *n_fbp = free_lists_p[index];
    free_lists_p[0] = fbp;
    if (n_fbp != NULL) {
        PUT_N_FREEA(fbp, n_fbp);
        PUT_P_FREEA(n_fbp, fbp);
    }
}
/* Given a free block ptr, compute the index of the free list it belongs to */
/* TODO: optimise using b search */
static uint32_t compute_index(uint32_t size) {

    for (int i = 0; i < MAX_LIST_SIZE - 1; i++) {
        uint32_t upper = 1 << (i + 4);
        uint32_t lower = 1 << (i + 6);
        if (size >= lower && size < upper) {
            return i;
        }
    }

    return (MAX_LIST_SIZE - 1);
}
/* remove the free block from the doubly linked list, the original fbp is returned*/
static void *remove_free(void *fbp) {
    void *prev_fbp = PREV_FREEP(fbp);
    void *next_fbp = NEXT_FREEP(fbp);

    if (prev_fbp != NULL && next_fbp != NULL) {
        PUT_P_FREEA(next_fbp, prev_fbp);
        PUT_N_FREEA(prev_fbp, next_fbp);
    } else if (prev_fbp != NULL && next_fbp == NULL) { // the second conditional trivially holds true, write here for clarity
        PUT_N_FREEA(prev_fbp, NULL);
    } else if (next_fbp != NULL && prev_fbp == NULL) { // same as the last one
        PUT_P_FREEA(next_fbp, NULL);
        uint32_t index = compute_index(GET_SIZE(HDRP(fbp)));
        free_lists_p[index] = next_fbp;
    } else {
        uint32_t index = compute_index(GET_SIZE(HDRP(fbp)));
        free_lists_p[index] = NULL;
    }

    PUT_P_FREEA(fbp, NULL);
    PUT_N_FREEA(fbp, NULL);
    return fbp;
}

/* extend the heap by at least words number of words, 
 * and then create a new free block (which will not be placed into appropriate list) of newly crteated heap, 
 * the block ptr of this newly created unattached free block is returned
 * the size is assumed to has satisfied alignment requiremen if needed
 */
static void *extend_heap(uint32_t size) {
    void *bp;
    uint32_t prev_alloc = GET_PALLOC(endlogue); // the flag whether the block before endlogue is allocated

    if ((long)(bp = mem_sbrk(size)) == -1) return NULL;

    /* initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0, prev_alloc)); // Free block header
    PUT(FTRP(bp), PACK(size, 0, prev_alloc)); // Free block footer
    PUT_N_FREEA(bp, NULL);             // The next free block is NULL
    PUT_P_FREEA(bp, NULL);
    endlogue = NEXT_BLKP(bp);
    PUT(HDRP(endlogue), PACK(0, 1, 0)); /* New epilogue header */
    bp = coalesce(bp);
    return bp;
}
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    /* initialize free list header */
    if ((free_lists_p = (void **) mem_sbrk(ALIGN(MAX_LIST_SIZE * sizeof(void *)))) == (void **)-1)
        return -1;
    for (int i = 0; i < MAX_LIST_SIZE; ++i) {
        free_lists_p[i] = NULL;
    }

    /* Create the intial empty heap */
    if ((prologue = mem_sbrk(4 * WSIZE)) == (void *)-1) 
        return -1;
    
    PUT(prologue, 0);
    prologue += 2 * WSIZE;
    PUT(HDRP(prologue), PACK(DSIZE, 1, 0)); // prologue
    endlogue = NEXT_BLKP(prologue);
    PUT(HDRP(endlogue), PACK(0, 1, 1)); // endlogue header

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    void *fbp;
    if ((fbp = extend_heap(CHUNKSIZE)) == NULL)
        return -1;
    place_free(fbp);

    /* test before return */
    check_correct_prev_alloc_flag();
    check_minimal_block_size();
    check_no_adjacent_free_block();
    check_no_unattached_free_block();

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    uint32_t newsize = MAX(ALIGN(size + sizeof(uint32_t)), MINBSIZE);
    void *fbp = find_fit(newsize);
    if (fbp != NULL) {
        remove_free(fbp);
        place(fbp, newsize);
    } else {
        if ((fbp = extend_heap(MAX(CHUNKSIZE, newsize)))== NULL) {
            return NULL;
        }
        place(fbp, newsize);
    }

    /* test before return */
    check_correct_prev_alloc_flag();
    check_minimal_block_size();
    check_no_adjacent_free_block();
    check_no_unattached_free_block();

    return fbp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    uint32_t prev_alloc = GET_PALLOC(HDRP(ptr));
    uint32_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0, prev_alloc));
    PUT(FTRP(ptr), PACK(size, 0, prev_alloc));
    PUT_P_FREEA(ptr, NULL);
    PUT_N_FREEA(ptr, NULL);
    ptr = coalesce(ptr);
    place_free(ptr);

    /* test before return */
    check_correct_prev_alloc_flag();
    check_minimal_block_size();
    check_no_adjacent_free_block();
    check_no_unattached_free_block();

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    if (ptr == NULL || ALIGN(sizeof(uint32_t) + size) <= GET_SIZE(HDRP(ptr))) {
        return ptr;
    }
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = GET_SIZE(HDRP(oldptr)) - sizeof(uint32_t);
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);

    /* test before return */
    check_correct_prev_alloc_flag();
    check_minimal_block_size();
    check_no_adjacent_free_block();
    check_no_unattached_free_block();

    return newptr;
}


