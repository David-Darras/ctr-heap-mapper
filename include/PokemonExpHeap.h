#ifndef CTR_HEAP_MAPPER_POKEMON_EXP_HEAP_H
#define CTR_HEAP_MAPPER_POKEMON_EXP_HEAP_H
#include "types.h"

struct HeapTable // at 0x08000000
{
    struct Entry
    {
        u32 wrapper;
        u32 isUsed : 1;
        u32 isStatic : 1;
        u32  : 30;
    } entries[560];
};

struct ExpHeapWrapper
{
    u32 vtable;
    u32 id;
    u32 allocCount;
    u32 ptr;
    s32 physicalLock;
    u32 ownerThreadId;
    s32 recursionCount;
    u32 expHeap;
};

#endif // CTR_HEAP_MAPPER_POKEMON_EXP_HEAP_H
