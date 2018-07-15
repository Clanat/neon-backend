//
// Created by clanat on 09.07.18.
//

#include "MemoryService.h"

#include <cstring>

using namespace neon;

const void* MemoryService::Read(uintptr_t address, size_t size) const {
    auto data = malloc(size);
    memcpy(data, (void *)address, size);
    return data;
}

void MemoryService::Write(uintptr_t address, const void* data, size_t size) const {
    memcpy((void *)address, data, size);
}

void MemoryService::HandleMemoryQuery(const MemoryQuery& query, MemoryQuery::Result* result) const {
    switch (query.kind()) {
        case MemoryQuery::READ: {
            const auto data = Read(query.address(), static_cast<size_t>(query.size()));
            result->set_data(data, static_cast<size_t>(query.size()));
            break;
        }
        case MemoryQuery::WRITE: {
            Write(query.address(), query.data().c_str(), static_cast<size_t>(query.size()));
            break;
        }
        default:
            break;
    }
}

void MemoryService::HandleMemoryBatchQuery(const MemoryBatchQuery& batchQuery, MemoryBatchQuery::Result* result) const {
    for (const auto &query: batchQuery.queries()) {
        HandleMemoryQuery(query, result->mutable_results()->Add());
    }
}