// COMP1521 18s1 Assignment 2
// Implementation of heap management system

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myHeap.h"

// minimum total space for heap
#define MIN_HEAP  4096
// minimum amount of space for a free Chunk (excludes Header)
#define MIN_CHUNK 32

#define ALLOC     0x55555555
#define FREE      0xAAAAAAAA

typedef unsigned int uint;   // counters, bit-strings, ...

typedef void *Addr;          // addresses

typedef struct {             // headers for Chunks
   uint  status;             // status (ALLOC or FREE)
   uint  size;               // #bytes, including header
} Header;

static Addr  heapMem;        // space allocated for Heap
static int   heapSize;       // number of bytes in heapMem
static Addr *freeList;       // array of pointers to free chunks
static int   freeElems;      // number of elements in freeList[]
static int   nFree;          // number of free chunks

int initHeap(int size)
{
    //the value of N is also rounded up to the nearest multiple of 4
    //e.g. 5001 would be rounded up to 5004
    if (size % 4 != 0){                 // Rounding size up to the nearest multiple of 4
      size = size + (4-size%4);         
    }
    
    //check size!! if N is less than the minimum heap size 4069, then N is set to the minimum heap size
    //compare the size to the MIN_HEAP
    if (size < MIN_HEAP){
        size = MIN_HEAP;
    }
    //allocates a region of memory of size N bytes using malloc()
    heapMem = malloc(size);
    if( heapMem ==  NULL){ //If malloc fails
        return -1;
    }

    //using memset making it zero all entire heapMem
    memset(heapMem,0,size);

    //allocate freList array;
    freeList = malloc(size/MIN_CHUNK);
    //point freeList[0] at the first chunk of heapMem
    freeList[0] = heapMem;
    //create pointer p;
    Header *p;
    p = heapMem;

    p -> status = FREE;
    p -> size = size;


    heapSize = size;
    freeElems = 1; //whole heapMem is free and its just 1
    nFree = 1;

    return 0;

}    
void freeHeap()
{
   free(heapMem);
   free(freeList);
}

// allocate a chunk of memory
void *myMalloc(int size)
{
   //check the size 
   if(size %4 != 0){
      size = size + (4-size%4);
    }
   int fsize = size + sizeof(Header) + MIN_CHUNK;
   Addr    curr;
   Header *chunk;
   Addr s; 
   

   int found = -1;
   int min = heapSize + 1;
   for (int j = 0; j < nFree; j++) { //using for loop to find the smallest size in freeList
      Header *curr = (Header*)freeList[j];
      chunk = (Header *)curr;
      if (chunk->size >= size + sizeof(Header) && chunk->size < min) {
         found = j; //locate the loacation of found
         min = chunk->size; //min change when found the smaller size
      }
   }
    //in case cannot find the free chunk return NULL to out of this function
   if (found == -1) {
    return NULL;
    }

   chunk = (Header *)freeList[found];   

    //this part is for alocate the whole chunk

   if (chunk->size <= fsize) {
      chunk->status = ALLOC; 
    //want to change only status in header, size still the same 
     
      s = (Addr)((char *)chunk + sizeof(Header));
      // update freeList[]
      for (int update = found; update < nFree; update++) {
         freeList[update] = freeList[update + 1];
      }
      nFree--;

    //if the free chunk is larger than this, then split it into two chunks
    //the lower chunk allocated for the request
    //the upper chunk being a new free chunk
   }else if (chunk->size > fsize) {
     
      int free_size = chunk->size;
   
      chunk->size = size + sizeof(Header);
      chunk->status = ALLOC;
   
      int alloc_size = chunk->size;
    
      s = (Addr)((char *)chunk + sizeof(Header));
   
      curr = (Addr)((char *)freeList[found] + chunk->size);
      chunk = (Header *)curr;
      chunk->status = FREE;
      
      chunk->size = free_size - alloc_size;
 
      freeList[found] = curr;
   } else {
      return NULL;
   }
   // returns a pointer to the first usable byte of data in the chunk
   return s;
}
// free a chunk of memory
void myFree(void *block)
{

    Addr heapBlock = (Addr)(char *) block - sizeof(Header);
    Header * startingPoint = (Header *) heapBlock;
    Header * chunk;

    if (block < heapMem || block >= heapMem + heapSize || block == NULL)
    {
        fprintf(stderr, "Attempt to free unallocated chunk\n");
        exit(1);
    }
   
    
    int f = nFree - 1;
    int g = nFree;
    if (startingPoint -> status == ALLOC)
    {
        // insert freed chunk into sorted array
        while (f >= 0  && freeList[f] > heapBlock)
        {
            freeList[g] = freeList[f];
            f--; 
            g--;
        }
        freeList[g] = heapBlock;
        startingPoint -> status = FREE;
        nFree ++;
    }
    else
    {
        fprintf(stderr, "Attempt to free unallocated chunk\n");
        exit(1);
    }
    Header * prevC; // ptr to prev chunk
    Header * nextC; // ptr to next chunk
    // if a chunk has free neighbours
    if (nFree > 1)
    {
        f = 0;
        g = 1;
        while(f < nFree)
        {
            Addr curr = freeList[f];
            chunk = (Header *) curr;
            nextC = (Header *) freeList[g];
            
            Addr currNext = (Addr)(char *) freeList[f] + chunk -> size;
            Addr next = freeList[g];
            Addr prev;
            // neighbour is free chunk
            if (currNext == next) // merge chunk with next
            {
                // merge
                chunk -> size += nextC -> size;
                // update freeList[]
                memset(freeList[g], 0, nextC -> size);
                nFree --;
                for (int k = g; k < nFree; k ++)
                {
                    freeList[k] = freeList[k + 1];
                }
            }
            if (f != 0) // merge chunk with prev
            {
                prevC = (Header *) freeList[f - 1];
                prev = (Addr)(char *) freeList[f - 1] + prevC -> size;
                if (prev == curr)
                {
                    prevC -> size += chunk -> size;
                    // update freeList[]
                    memset(freeList[f], 0, chunk -> size);
                    nFree --;
                    for (int k = f; k < nFree; k ++)
                    {
                        freeList[k] = freeList[k + 1];
                    }
                }
            }
            f ++; 
            g ++;
        }
    }
}

// convert pointer to offset in heapMem
int  heapOffset(void *p)
{
   Addr heapTop = (Addr)((char *)heapMem + heapSize);
   if (p == NULL || p < heapMem || p >= heapTop)
      return -1;
   else
      return p - heapMem;
}

// dump contents of heap (for testing/debugging)
void dumpHeap()
{
   Addr    curr;
   Header *chunk;
   Addr    endHeap = (Addr)((char *)heapMem + heapSize);
   int     onRow = 0;

   curr = heapMem;
   while (curr < endHeap) {
      char stat;
      chunk = (Header *)curr;
      switch (chunk->status) {
      case FREE:  stat = 'F'; break;
      case ALLOC: stat = 'A'; break;
      default:    fprintf(stderr,"Corrupted heap %08x\n",chunk->status); exit(1); break;
      }
      printf("+%05d (%c,%5d) ", heapOffset(curr), stat, chunk->size);
      onRow++;
      if (onRow%5 == 0) printf("\n");
      curr = (Addr)((char *)curr + chunk->size);
   }
   if (onRow > 0) printf("\n");
}
