//
// Created by clanat on 09.07.18.
//

#ifndef NEON_BACKEND_MEMORYHANDLER_H
#define NEON_BACKEND_MEMORYHANDLER_H

#include <cstdint>
#include <memory>

#include <Models.pb.h>

#define ADDR_IS_PLAYER_ONLINE 0x00B4B424

namespace neon {
    class MemoryService {
    public:
        const bool isPlayerOnline() const { return *(bool *)ADDR_IS_PLAYER_ONLINE; }
        const void* Read(uint32_t address, size_t size) const;
        void Write(uintptr_t address, const void* data, size_t size) const;

        void HandleMemoryQuery(const MemoryQuery &query, MemoryQuery::Result *result) const;
        void HandleMemoryBatchQuery(const MemoryBatchQuery &batchQuery, MemoryBatchQuery::Result *result) const;
    };
}


#endif //NEON_BACKEND_MEMORYHANDLER_H
