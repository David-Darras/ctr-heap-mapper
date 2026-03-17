#ifndef CTR_HEAP_MAPPER_EXP_HEAP_H
#define CTR_HEAP_MAPPER_EXP_HEAP_H

#include "types.h"

struct MemoryBlockHeader
{
    u16 signature; // DU, RF
    u8 id;
    u8 alignment : 7; // default: 4
    u8 isRearAlloc : 1; // default: front
    u32 payloadSize; // without the header

    u32 prev;
    u32 next;

    bool isUsed() const { return signature == 0x5544; } // DU (USED)
    bool isFree() const { return signature == 0x4652; } // RF (FREE)
    bool isValid() const { return isUsed() || isFree(); }
};

struct ExpHeap
{
    u32 vtable;

    struct Node
    {
        u32 parent;
        u32 children;
        u32 zeroFill : 1;
        u32  : 31;
    } node;

    u8 reserved0[8];

    struct Core
    {
        u32 signature; // HPXE

        struct
        {
            u32 prev;
            u32 next;
        } siblings;

        struct
        {
            u32 head;
            u32 tail;
            u16 count;
            u16 offset; // = sizeof(signature) = 4
        } subHeaps;

        u32 start;
        u32 end;
        u32 reserved1;

        struct
        {
            u32 head;
            u32 tail;
        } freeBlocks, usedBlocks;

        u16 id;
        bool isBestFit; // default: first fit
        u8 reserved2;
        u32 reserved3;
    } core;

    u32 allocCount;

    bool isValid() const { return core.signature == 0x45585048; } // HPXE
    u32 getTotalSize() const { return core.end - core.start; }
};

#endif // CTR_HEAP_MAPPER_EXP_HEAP_H
