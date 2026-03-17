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
    MemoryBlockHeader* prev;
    MemoryBlockHeader* next;

    bool isUsed() const
    {
        return signature == 0x5544; // DU (USED)
    }

    bool isFree() const
    {
        return signature == 0x4652; // RF (FREE)
    }

    bool isValid() const
    {
        return isUsed() || isFree();
    }

    void* getPayload() const
    {
        return (void*)((uptr)this + sizeof(MemoryBlockHeader));
    }
};

struct ExpHeap
{
    void* vtable;

    struct Node
    {
        Node* parent;
        void* children;
        u32 zeroFill : 1;
        u32  : 31;
    } node;

    u8 reserved0[8];

    struct Core
    {
        u32 signature; // HPXE

        struct
        {
            Core* prev;
            Core* next;
        } siblings;

        struct
        {
            Core* head;
            Core* tail;
            u16 count;
            u16 offset; // = sizeof(signature) = 4
        } subHeaps; // intrusive linked list

        void* start;
        void* end;
        u32 reserved1;

        struct
        {
            MemoryBlockHeader* head;
            MemoryBlockHeader* tail;
        } freeBlocks, usedBlocks;

        u16 id;
        bool isBestFit; // default: first fit
        u8 reserved2;
        u32 reserved3;
    } core;

    u32 allocCount;

    u32 isValid() const
    {
        return core.signature == 0x45585048; // HPXE (Expandable Heap)
    }

    u32 getTotalSize() const
    {
        return (u32)((uptr)core.end - (uptr)core.start);
    }
};

#endif // CTR_HEAP_MAPPER_EXP_HEAP_H
