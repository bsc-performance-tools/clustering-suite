#include "smp_superscalar_sort.h"

/*
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2008 Barcelona Supercomputing Center
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * NOTE: This code is an adaptation from the `cilksort' implementation (from
 * Matteo Frigo at MIT) for the SMP Superscalar framework.  It has been
 * reordered and cleaned up a bit from the original to ease the reading of
 * code.
 */

/*
 * The original cilksort is based on multisort, an idea from the paper: S. G.
 * Akl and N. Santoro, "Optimal Parallel Merging and Sorting Without Memory
 * Conflicts", IEEE Trans. Comp., Vol. C-36 No. 11, Nov. 1987
 *
 * SMP Superscalar does not support recursive tasks, but the dependency
 * tracking is dynamic and automatic so we play on that card.
 *
 * At a certain array size, direct quicksort is used.  So, in the first pass,
 * many parallel quicksort tasks operate on independent slices of the original
 * array.  Then each slice is merged, but also in a parallel way.  The original
 * cilkmerge code did the following:
 *
 *       cilkmerge(A[1..n], B[1..m], C[1..(n+m)]) =
 *          find the median of A \union B using binary
 *          search.  The binary search gives a pair
 *          (ma, mb) such that ma + mb = (n + m)/2
 *          and all elements in A[1..ma] are smaller than
 *          B[mb..m], and all the B[1..mb] are smaller
 *          than all elements in A[ma..n].
 *
 *          spawn cilkmerge(A[1..ma], B[1..mb], C[1..(n+m)/2])
 *          spawn cilkmerge(A[ma..m], B[mb..n], C[(n+m)/2 .. (n+m)])
 *          sync
 *
 * This recursion has been divided in three steps:
 *
 *      1. Split A and B (using the median finding technique).  This is a task
 *         that depends on A and B and generates two smaller chunks from A, two
 *         from B and two from C.
 *      2. Perform the two recursive merges (this is NOT a task).
 *      3. Join the two merged chunks into C again.  This is an empty task
 *         because C is already sorted after the parallel merge (if the
 *         partitioning was correct).
 *
 * Of course, there is a sequential merge for a certain block size, which is
 * also a task just like the quicksort.  Depending on the values chosen for
 * QUICKSIZE and MERGESIZE, the task dependency graph varies.  With a
 * QUICKSIZE = 2 * MERGESIZE we would have as many parallel sequential merge
 * tasks as quicksort tasks:
 *
 *                       quick   quick   quick   quick
 *                          \     /         \     /
 *                           split           split
 *                          /     \         /     \
 *                       merge   merge    merge   merge
 *                           \    /         \    /
 *                            join           join
 *                                \         /
 *                                 \       /
 *                                  \     /
 *                                   split
 *                                  /     \
 *                                 /       \
 *                                /         \
 *                           split           split
 *                          /     \         /     \
 *                       merge   merge   merge   merge
 *                           \    /         \    /
 *                            join           join
 *                                \         /
 *                                 \       /
 *                                  \     /
 *                                    join
 *
 * And the pattern repeats as the array to sort grows, i.e. the number of
 * quicksort tasks is larger.
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef float  ELM;

/* MERGESIZE must be >= 2 */
#define KILO 1024
#ifndef MERGESIZE
#define MERGESIZE (256*KILO)
//#define MERGESIZE (4*KILO)
#endif
#ifndef QUICKSIZE
#define QUICKSIZE (512*KILO)
//#define QUICKSIZE (8*KILO)
#endif
#define INSERTIONSIZE 20

#define swap(T, a, b) \
{ \
  T tmp;\
  tmp = a;\
  a = b;\
  b = tmp;\
}



/**********************  ARRAY RANDOMIZATION FUNCTIONS  **********************/

unsigned long  rand_nxt = 0;


static inline unsigned long my_rand(void)
{
        rand_nxt = rand_nxt * 1103515245 + 12345;
        return rand_nxt;
}


static inline void my_srand(unsigned long seed)
{
        rand_nxt = seed;
}


void scramble_array (ELM *arr, unsigned long size)
{
        unsigned long  i;
        unsigned long  j;

        for (i = 0; i < size; ++i)
        {
                j = my_rand();
                j = j % size;
                swap(ELM, arr[i], arr[j]);
        }
}


void fill_array (ELM *arr, unsigned long size)
{
        unsigned long  i;

        my_srand(1);
        /* first, fill with integers 1..size */
        for (i = 0; i < size; ++i)
                arr[i] = i;

        /* then, scramble randomly */
        scramble_array(arr, size);
}


/*******************  ARRAY CHUNK REPRESENTANT ALLOCATION  *******************/

/*
 * This structure represents a slice or chunk of array.  It is essentially used
 * to generate dependencies between tasks so they are executed in the correct
 * order, but as parallelized as possible.
 *
 * In order to avoid the runtime to perform memory copies and renames on these
 * allocated structures, we have to pass them around as void pointers.  This is
 * due to some limitations of the runtime.  And then we use the filed "repr" as
 * the representant of the structure.
 *
 * If there was a way to tell the runtime to NOT manage the structures, the
 * integer field would not be necessary, as the memory address of the structure
 * would serve to generate the dependencies.
 *
 * Remember that each chunk is unique, even if both fields point to the same
 * places of the large array; because the chunk is semantically different.
 */
struct chunk
{
        ELM  *low;
        ELM  *high;
        int   repr;
};


/*
 * An amortized allocation policy is used because many chunk structures will be
 * requested.  What's more, once a chunk structure has been allocated, its
 * address must NOT change; otherwise the runtime will get confused, leading to
 * a sorting failure.
 *
 * Chunks are grouped in blocks of 4096 structures (big, but not too big).  So
 * we can request page-aligned allocation, and we would probably get a few
 * contiguous pages; no to mention the amortized cost for the next 4095
 * allocations.
 *
 * As we can't move the blocks, the use of realloc() for a single block of
 * structures is forbidden.  The way to solve this is to have an array of
 * blocks that keeps them accessible.  Actually, this is not really necessary;
 * we could drop the old block pointer because we only need the current block
 * to provide the next structure.  But you never know when you'd be able to
 * optimise the memory here.
 */
struct chunk  **chunk_block_pool;
int             chunk_count, chunk_block_max;


void init_chunk_pool (void)
{
        chunk_block_pool = malloc(32 * sizeof(struct chunk *));
        chunk_count      = 0;
        chunk_block_max  = 32;
}


struct chunk *new_chunk (void)
{
        struct chunk  *block;

        if ((chunk_count & 4095) == 0)
        {
                if ((chunk_count >> 12) == chunk_block_max)
                {
                        chunk_block_max += 32;
                        chunk_block_pool = realloc(chunk_block_pool,
                                                   chunk_block_max * sizeof(struct chunk *));
                        if (chunk_block_pool == NULL)
                        {
                                perror("Out of memory for blocks array");
                                exit(EXIT_FAILURE);
                        }
                }
                chunk_block_pool[chunk_count >> 12] = valloc(4096 * sizeof(struct chunk));
                if (chunk_block_pool[chunk_count >> 12] == NULL)
                {
                        perror("Out of memory for blocks");
                        exit(EXIT_FAILURE);
                }
        }

        block = chunk_block_pool[chunk_count >> 12];
        return &block[(chunk_count++) & 4095];
}



/***********************  SEQUENTIAL SORTING FUNCTIONS  ***********************/

/*
 * Insertion sort.  Used to sort very small array slices or chunks.
 */
void insertion_sort (ELM *low, ELM *high)
{
        ELM  *p, *q;
        ELM   a, b;

        for (q = low + 1; q <= high; ++q)
        {
                a = q[0];
                for (p = q - 1; p >= low && (b = p[0]) > a; p--)
                        p[1] = b;
                p[1] = a;
        }
}


/*
 * Find the median element of the three values.  That is, not the biggest, nor
 * the smallest, but the value in the middle.
 */
static inline ELM simple_median (ELM a, ELM b, ELM c)
{
        if (a < b)
        {
                if (b < c)
                        return b;
                else
                {
                        if (a < c)
                                return c;
                        else
                                return a;
                }
        }
        else
        {
                if (b > c)
                        return b;
                else
                {
                        if (a > c)
                                return c;
                        else
                                return a;
                }
        }
}


/*
 * Choose the pivot to partition for quicksort.  Simple approach for now; a
 * better median-finding may be preferable
 */
static inline ELM choose_pivot (ELM *low, ELM *high)
{
        return simple_median(*low, *high, low[(high - low) / 2]);
}


/*
 * Sort a slice of array by choosing a pivot and sort the array in two
 * partitions: the elements smaller than the pivot and the elements larger than
 * the pivot (in that order).
 *
 * This is the element exchange performed as part of the quicksort algorithm.
 */
ELM *partition_sort (ELM *low, ELM *high)
{
        ELM   pivot;
        ELM   h, l;
        ELM  *curr_low = low;
        ELM  *curr_high = high;

        pivot = choose_pivot(low, high);

        while (1)
        {
                while ((h = *curr_high) > pivot)
                        curr_high--;

                while ((l = *curr_low) < pivot)
                        curr_low++;

                if (curr_low >= curr_high)
                        break;

                *curr_high-- = l;
                *curr_low++ = h;
        }

        /*
         * I don't know if this is really necessary.  The problem is that the
         * pivot is not always the first element, and the partition may be
         * trivial.  However, if the partition is trivial, then *high is the
         * largest element, whence the following code.
         */
        if (curr_high < high)
                return curr_high;
        else
                return curr_high - 1;
}


/*
 * Sequential quicksort, tail-recursive.  Almost unrecognizable :-)
 */
void quicksort (ELM *low, ELM *high)
{
        ELM  *p;

        while (high - low >= INSERTIONSIZE)
        {
                p = partition_sort(low, high);
                quicksort(low, p);
                low = p + 1;
        }

        insertion_sort(low, high);
}


/*
 * Sequential merge of two array slices/chunks.  Linear cost but slightly
 * optimized.
 */
void merge (ELM *low1, ELM *high1, ELM *low2, ELM *high2, ELM *lowdest)
{
        ELM  a1, a2;

        /*
         * The following 'if' statement is not necessary for the correctness of
         * the algorithm, and is in fact subsumed by the rest of the function.
         * However, it is a few percent faster.  Here is why.
         *
         * The merging loop below has something like
         *
         *   if (a1 < a2) {
         *        *dest++ = a1;
         *        ++low1;
         *        if (end of array) break;
         *        a1 = *low1;
         *   }
         *
         * Now, a1 is needed immediately in the next iteration and there is no
         * way to mask the latency of the load.  A better approach is to load
         * a1 *before* the end-of-array check; the problem is that we may be
         * speculatively loading an element out of range.  While this is
         * probably not a problem in practice, yet I don't feel comfortable
         * with an incorrect algorithm.  Therefore, I use the 'fast' loop on
         * the array (except for the last element) and the 'slow' loop for the
         * rest, saving both performance and correctness.
         */
        if (low1 < high1 && low2 < high2)
        {
                a1 = *low1;
                a2 = *low2;
                for (;;)
                        if (a1 < a2)
                        {
                                *lowdest++ = a1;
                                a1 = *++low1;
                                if (low1 >= high1)
                                        break;
                        }
                        else
                        {
                                *lowdest++ = a2;
                                a2 = *++low2;
                                if (low2 >= high2)
                                        break;
                        }
        }

        if (low1 <= high1 && low2 <= high2)
        {
                a1 = *low1;
                a2 = *low2;
                for (;;)
                        if (a1 < a2)
                        {
                                *lowdest++ = a1;
                                ++low1;
                                if (low1 > high1)
                                        break;
                                a1 = *low1;
                        }
                        else
                        {
                                *lowdest++ = a2;
                                ++low2;
                                if (low2 > high2)
                                        break;
                                a2 = *low2;
                        }
        }

        if (low1 > high1)
                memcpy(lowdest, low2, sizeof(ELM) * (high2 - low2 + 1));
        else
                memcpy(lowdest, low1, sizeof(ELM) * (high1 - low1 + 1));
}


/*
 * Dicotomic search of val within the slice [low, high].  Returns index which
 * contains greatest element <= val.  If val is less than all elements, returns
 * low - 1.
 *
 * This is the key to parallel merges.
 */
ELM *binary_search (ELM val, ELM *low, ELM *high)
{
        ELM  *mid;

        while (low != high)
        {
                mid = low + ((high - low + 1) >> 1);
                if (val <= *mid)
                        high = mid - 1;
                else
                        low = mid;
        }

        if (*low > val)
                return low - 1;
        else
                return low;
}



/******************************  TASK FUNCTIONS  ******************************/
/*
 * These functions constitute the glue between the runtime and the sequential
 * sorting and merging methods.  Their dependencies ensure the correct order of
 * execution, while preserving as much parallelization as possible.
 *
 * All the void pointer parameters which name ends with x are casted to chunk
 * structure pointers with the trailing x removed from the name.
 */


#pragma css task input (ckx) inout(ckrep)
void task_quicksort (void *ckx, int *ckrep)
{
    struct chunk  *ck = (struct chunk *) ckx;

    quicksort(ck->low, ck->high);
}


#pragma css task input(srcAx, srcBx, destx, Arep, Brep) inout(destrep)
void task_merge (void *srcAx, void *srcBx, void *destx,
                 int  *Arep,  int  *Brep,  int  *destrep)
{
    struct chunk  *srcA = (struct chunk *) srcAx;
    struct chunk  *srcB = (struct chunk *) srcBx;
    struct chunk  *dest = (struct chunk *) destx;

    merge(srcA->low, srcA->high, srcB->low, srcB->high, dest->low);
}


/*
 * Split sorted chunks.  This task calculates the proper locations where the
 * chunks can be split to be merged in parallel safely.  For each chunk, two
 * more are generated: the left and the right side of the split.
 *
 * Read the initial comment (at the beginning of this file) to understand the
 * logic of the splitting mechanism.
 */
#pragma css task input(srcAx, srcBx, destx, Aleftx, Bleftx, destleftx) \
                 input(Arightx, Brightx, destrightx) \
                 input(sArep, sBrep, destrep) \
                 output(sALrep, sBLrep, Ldrep, sARrep, sBRrep, Rdrep)
void task_split_chunks (void *srcAx,   void *srcBx,   void *destx,
                        void *Aleftx,  void *Bleftx,  void *destleftx,
                        void *Arightx, void *Brightx, void *destrightx,
                        int  *sArep,   int  *sBrep,   int  *destrep,
                        int  *sALrep,  int  *sBLrep,  int  *Ldrep,
                        int  *sARrep,  int  *sBRrep,  int  *Rdrep)
{
        struct chunk  *srcA      = (struct chunk *) srcAx;
        struct chunk  *srcB      = (struct chunk *) srcBx;
        struct chunk  *dest      = (struct chunk *) destx;
        struct chunk  *Aleft     = (struct chunk *) Aleftx;
        struct chunk  *Bleft     = (struct chunk *) Bleftx;
        struct chunk  *destleft  = (struct chunk *) destleftx;
        struct chunk  *Aright    = (struct chunk *) Arightx;
        struct chunk  *Bright    = (struct chunk *) Brightx;
        struct chunk  *destright = (struct chunk *) destrightx;
        ELM  *splitA, *splitB;
        long  splitsize;

        Aleft->low = srcA->low;  Aright->high = srcA->high;
        Bleft->low = srcB->low;  Bright->high = srcB->high;
        /*
         * We want to take the middle element from the larger of the two
         * arrays.
         */
        if (srcB->high - srcB->low > srcA->high - srcA->low)
        {
                /* srcB is larger than srcA */
                splitB = ((srcB->high - srcB->low + 1) / 2) + srcB->low;
                splitA = binary_search(*splitB, srcA->low, srcA->high);

                Bleft->high = splitB - 1; Bright->low = splitB;
                Aleft->high = splitA;     Aright->low = splitA + 1;
        }
        else
        {
                /* srcA is larger than srcB */
                splitA = ((srcA->high - srcA->low + 1) / 2) + srcA->low;
                splitB = binary_search(*splitA, srcB->low, srcB->high);

                Aleft->high = splitA - 1; Aright->low = splitA;
                Bleft->high = splitB;     Bright->low = splitB + 1;
        }

        splitsize       = splitA - srcA->low + splitB - srcB->low;
        destleft->low   = dest->low;
        destleft->high  = dest->low + splitsize;
        destright->low  = dest->low + splitsize + 1;
        destright->high = dest->high;
}


/*
 * Join the split (and merged) chunks.  This task does not need to do anything
 * because the parallel merges knew exactly where to place the elements; so they
 * are sorted correctly.
 *
 * However, this task acts as a kind of "dynamic" barrier by inducing
 * dependencies.  It is funny how simple this task is, but how important for
 * proper execution.
 */
#pragma css task input(leftrep, rightrep) output(destrep)
void task_join_chunks (int *leftrep, int *rightrep, int *destrep)
{
        /*
         * Only representants needed, because they are the ones that generate
         * dependency information.
         */
}



/********************  RECURSIVE TASK SUBMITTER FUNCTIONS  ********************/
/*
 * Contrary to the Cilk approach, all recursion steps are performed in a single
 * thread at the beginning of the execution.  The main reason for this is the
 * lack of recursive tasks support from SMP Superscalar, at the moment.
 *
 * Therefore, these functions only serve as task submitters.  Once tasks are
 * created, let the runtime handle them appropriately.
 */


void smpss_merge (struct chunk *srcA, struct chunk *srcB, struct chunk *dest,
                  long size)
{
        struct chunk  *Aleft, *Aright, *Bleft, *Bright, *destleft, *destright;

        if (size < MERGESIZE)
        {
                /* Sequential merge when block size reached */
                task_merge(srcA, srcB, dest,
                           &srcA->repr, &srcB->repr, &dest->repr);
                return;
        }

        size /= 2;

        /*
         * Basic approach: Split the source chunks, split the destination chunk
         * appropriately, and merge the two lower halves independently from the
         * two upper halves (i.e. in parallel).
         *
         * Chunk splitting generates new, smaller, chunks.
         */

        Aleft    = new_chunk();   Aright    = new_chunk();
        Bleft    = new_chunk();   Bright    = new_chunk();
        destleft = new_chunk();   destright = new_chunk();

        task_split_chunks(srcA,   srcB,   dest,
                          Aleft,  Bleft,  destleft,
                          Aright, Bright, destright,
                          &srcA->repr,   &srcB->repr,   &dest->repr,
                          &Aleft->repr,  &Bleft->repr,  &destleft->repr,
                          &Aright->repr, &Bright->repr, &destright->repr);

        smpss_merge(Aleft,  Bleft,  destleft,  size);
        smpss_merge(Aright, Bright, destright, size);

        task_join_chunks(&destleft->repr, &destright->repr, &dest->repr);
}


void smpss_sort (struct chunk *arr, struct chunk *tmp, long size)
{
        /*
         * Divide the input in four parts of the same size (A, B, C, D)
         * Then:
         *   1) recursively sort A, B, C, and D
         *   2) merge A and B into tmp1, and C and D into tmp2
         *   3) merge tmp1 and tmp2 into the original array
         *
         * Parallel quicksort tasks work at blocks with QUICKSIZE elements.
         * Parallel sequential merge tasks work at blocks with MERGESIZE
         * elements.
         */
        long           quarter = size / 4;
        struct chunk  *A, *B, *C, *D, *tA, *tB, *tC, *tD, *tAB, *tCD;

        if (size < QUICKSIZE)
        {
                /* Quicksort when block size reached */
                task_quicksort(arr, &arr->repr);
                return;
        }

        A = new_chunk();   tA = new_chunk();
        B = new_chunk();   tB = new_chunk();
        C = new_chunk();   tC = new_chunk();
        D = new_chunk();   tD = new_chunk();

         A->low = arr->low;             A->high =  A->low + quarter - 1;
        tA->low = tmp->low;            tA->high = tA->low + quarter - 1;
         B->low =  A->low + quarter;    B->high =  B->low + quarter - 1;
        tB->low = tA->low + quarter;   tB->high = tB->low + quarter - 1;
         C->low =  B->low + quarter;    C->high =  C->low + quarter - 1;
        tC->low = tB->low + quarter;   tC->high = tC->low + quarter - 1;
         D->low =  C->low + quarter;    D->high = arr->high;
        tD->low = tC->low + quarter;   tD->high = tmp->high;

        /* Temporary merge chunks (tA + tB and tC + tD) */
        tAB       = new_chunk();   tCD       = new_chunk();
        tAB->low  = tA->low;       tCD->low  = tC->low;
        tAB->high = tB->high;      tCD->high = tD->high;

        smpss_sort(A, tA, quarter);
        smpss_sort(B, tB, quarter);
        smpss_sort(C, tC, quarter);
        smpss_sort(D, tD, size - 3 * quarter);

        smpss_merge(A, B, tAB, quarter);
        smpss_merge(C, D, tCD, quarter);

        smpss_merge(tAB, tCD, arr, size / 2);
}



/**************************  MAIN HELPER FUNCTIONS  **************************/

int usage (char *argv0)
{
        fprintf(stderr, "SMP Superscalar version of the Multisort algorithm.\n\n"
                        "Usage: %s [-n <size>] [-b <type>] [-h]\n\n"
                        "\t-n  Define the size of the array to be sorted.\n"
                        "\t-b  Use a predefined benchmark size:\n"
                        "\t\t1  Small.\n"
                        "\t\t2  Medium.\n"
                        "\t\t3  Large.\n"
                        "\t-h  This help.\n\n", argv0);

     return -1;
}


int numeric_value (char *str, long *val)
{
        char *endptr;

        *val = strtol(str, &endptr, 0);
        return str != endptr && *endptr == '\0';
}


static inline float elapsed_time (struct timeval *start, struct timeval *end)
{
        return (end->tv_sec - start->tv_sec) + (end->tv_usec - start->tv_usec) * 1.0e-6f;
}


int psort (float *x, int length) {

//        int             success;
//        long            size, benchmark, i;
        ELM            *array, *tmp;
        struct timeval  start_time, end_time;
        struct chunk    array_chunk, tmp_chunk;

        // Default size (2^25 elements)
//        size      = 32 * 1024 * 1024;
//        benchmark = 0;

        // Parse command line arguments
/*        switch (argc)
        {
        case 1:
                break;
        case 2:
                if (strcmp(argv[1], "-h") != 0)
                        fprintf(stderr, "ERROR: Unknown argument\n\n");
                return usage(argv[0]);
        case 3:
                if (strcmp(argv[1], "-n") == 0)
                {
                        if (!numeric_value(argv[2], &size))
                        {
                                fprintf(stderr, "ERROR: Invalid size\n\n");
                                return usage(argv[0]);
                        }
                }
                else if (strcmp(argv[1], "-b") == 0)
                {
                        if (!numeric_value(argv[2], &benchmark))
                        {
                                fprintf(stderr, "ERROR: Invalid benchmark\n\n");
                                return usage(argv[0]);
                        }
                }
                else
                {
                        fprintf(stderr, "ERROR: Unknown arguments\n\n");
                        return usage(argv[0]);
                }
                break;
        default:
                fprintf(stderr, "ERROR: Too many arguments\n\n");
                return usage(argv[0]);
        }

        // Change size if benchmark requested
        switch (benchmark)
        {
        case 0:
                break;
        case 1:  // short benchmark options -- a little work
                size = 10000;
                break;
        case 2:  // standard benchmark options
                size = 3000000;
                break;
        case 3:  // long benchmark options -- a lot of work
                size = 4100000;
                break;
        default:
                fprintf(stderr, "ERROR: Unknown benchmark value\n\n");
                return usage(argv[0]);
        }
*/
        array = x;
        tmp   = valloc(length * sizeof(ELM));

        init_chunk_pool();

        array_chunk.low  = array;
        array_chunk.high = array + length - 1;
        tmp_chunk.low    = tmp;
        tmp_chunk.high   = tmp + length - 1;

//        fill_array(array, size);

//#pragma css start
//        gettimeofday(&start_time, NULL);
        smpss_sort(&array_chunk, &tmp_chunk, length);
#pragma css barrier
//        gettimeofday(&end_time, NULL);
//#pragma css finish
//        printf("time: %f\n\n", elapsed_time(&start_time, &end_time));

/*        success = 1;
        for (i = 0; i < size; ++i)
                if (array[i] != i)
                        success = 0;

        if (success)
                printf("number of elements = %ld\n\n", size);
        else
                printf("SORTING FAILURE\n");
*/
//        free(array);
        free(tmp);
        return 0;
}

/*******************************  MAIN PROGRAM  *******************************/
/*
int main (int argc, char **argv)
{
        int             success;
        long            size, benchmark, i;
        ELM            *array, *tmp;
        struct timeval  start_time, end_time;
        struct chunk    array_chunk, tmp_chunk;

        // Default size (2^25 elements)
        size      = 32 * 1024 * 1024;
        benchmark = 0;

        // Parse command line arguments
        switch (argc)
        {
        case 1:
                break;
        case 2:
                if (strcmp(argv[1], "-h") != 0)
                        fprintf(stderr, "ERROR: Unknown argument\n\n");
                return usage(argv[0]);
        case 3:
                if (strcmp(argv[1], "-n") == 0)
                {
                        if (!numeric_value(argv[2], &size))
                        {
                                fprintf(stderr, "ERROR: Invalid size\n\n");
                                return usage(argv[0]);
                        }
                }
                else if (strcmp(argv[1], "-b") == 0)
                {
                        if (!numeric_value(argv[2], &benchmark))
                        {
                                fprintf(stderr, "ERROR: Invalid benchmark\n\n");
                                return usage(argv[0]);
                        }
                }
                else
                {
                        fprintf(stderr, "ERROR: Unknown arguments\n\n");
                        return usage(argv[0]);
                }
                break;
        default:
                fprintf(stderr, "ERROR: Too many arguments\n\n");
                return usage(argv[0]);
        }

        // Change size if benchmark requested
        switch (benchmark)
        {
        case 0:
                break;
        case 1:  // short benchmark options -- a little work
                size = 10000;
                break;
        case 2:  // standard benchmark options
                size = 3000000;
                break;
        case 3:  // long benchmark options -- a lot of work
                size = 4100000;
                break;
        default:
                fprintf(stderr, "ERROR: Unknown benchmark value\n\n");
                return usage(argv[0]);
        }

        array = valloc(size * sizeof(ELM));
        tmp   = valloc(size * sizeof(ELM));

        init_chunk_pool();

        array_chunk.low  = array;
        array_chunk.high = array + size - 1;
        tmp_chunk.low    = tmp;
        tmp_chunk.high   = tmp + size - 1;

        fill_array(array, size);

#pragma css start
        gettimeofday(&start_time, NULL);
        smpss_sort(&array_chunk, &tmp_chunk, size);
#pragma css barrier
        gettimeofday(&end_time, NULL);
#pragma css finish
        printf("time: %f\n\n", elapsed_time(&start_time, &end_time));

        success = 1;
        for (i = 0; i < size; ++i)
                if (array[i] != i)
                        success = 0;

        if (success)
                printf("number of elements = %ld\n\n", size);
        else
                printf("SORTING FAILURE\n");

        free(array);
        free(tmp);
        return 0;
}

*/
