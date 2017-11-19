#ifndef _DEFS_H
#define _DEFS_H

#define NULL ((void*)0)

typedef unsigned long  uint64_t;
typedef          long   int64_t;
typedef unsigned int   uint32_t;
typedef          int    int32_t;
typedef unsigned short uint16_t;
typedef          short  int16_t;
typedef unsigned char   uint8_t;
typedef          char    int8_t;

typedef uint64_t size_t;
typedef int64_t ssize_t;

typedef uint64_t off_t;

typedef uint32_t pid_t;

typedef uint64_t mode_t;

/*
- Round up N to the nearest multiple of S
- https://stackoverflow.com/questions/1010922/question-about-round-up-macro
- ROUND_UP(121, 5) = 125, ROUND_UP(10, 5) = 10
*/
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

#define ROUND_DOWN(N, S) ((N / S) * S)

/* Kernbase: refer to linker.script */
#define KERNBASE 0xffffffff80000000

/* Virtual base address */
#define VIRTUAL_BASE 0xFFFFFFFF00000000

#endif
