#include <sys/defs.h>
#include <sys/mman.h>
#define NALLOC 4096

typedef long Align;                      /* for alignment to long boundary */

typedef union header {                      /* block header */
    struct {
        union header *ptr;                   /* next block if on free list */
        size_t size;                         /* size of this block */
    } s;

    Align x;                               /* force alignment of blocks */

} Header;


static Header base = {0};                /* empty list to get started */
static Header* freeptr = NULL;           /* start of free list */

/* free: put block ap in free list */
void free(void *ap) {
    Header *bp, *p;
    bp = (Header *)ap - 1; /* point to block header */

    for (p = freeptr; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) {
            break; /* freed block at start or end of arena */
        }
    }

    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else {
        bp->s.ptr = p->s.ptr;
    }

    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else {
        p->s.ptr = bp;
    }

    freeptr = p;
}


static Header *morecore(size_t nunits) {
    char *cp;
    Header *up;

    if (nunits < NALLOC)
        nunits = NALLOC;
    cp = mmap(0, nunits * sizeof(Header), 0x007);

    if (cp == NULL) {
        return NULL;
    }
    up = (Header *)cp;
    up->s.size = nunits;
    free((void *)(up + 1));

    return freeptr;
}

/* Malloc: general-purpose storage allocator */
void *malloc (size_t nbytes) {
    Header*  p;
    Header*  prevptr;
    size_t   nunits;
    void*    result;
    int     is_allocating;

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;

    prevptr = freeptr;
    if (prevptr == NULL)  {                     /* no free list yet */
        base.s.ptr  = &base;
        freeptr     = &base;
        prevptr     = &base;
        base.s.size = 0;
    }

    is_allocating = 1;
    for (p = prevptr->s.ptr; is_allocating != 0; p = p->s.ptr) {
        if (p->s.size >= nunits) {              /* big enough */
            if (p->s.size == nunits) {          /* exactly */
                prevptr->s.ptr = p->s.ptr;
            } else {                            /* allocate tail end */
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }

            freeptr = prevptr;
            result = p+1;
            is_allocating = 0;                  /* we are done */
        }

        if (p == freeptr) {                     /* wrapped around free list */
            p = morecore(nunits);
            if (p == NULL) {
                result = NULL;                   /* none left */
                is_allocating = 0;
            }
        }
        prevptr = p;
    }                                           /* for */

    return result;
}
