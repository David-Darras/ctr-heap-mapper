#ifndef CTR_HEAP_MAPPER_GATEWAYRAMDUMP_H
#define CTR_HEAP_MAPPER_GATEWAYRAMDUMP_H

#include "types.h"

struct RegionEntry
{
    u32 startAddress;
    u32 fileOffset;
    u32 size;
};

struct GatewayRAMDump
{
    u32 regionCount;
    u32 padding;
    // RegionEntry regions[];
};

#endif //CTR_HEAP_MAPPER_GATEWAYRAMDUMP_H
