//
// Created by clanat on 11.07.18.
//

#ifndef NEON_BACKEND_FUNCTIONSERVICE_H
#define NEON_BACKEND_FUNCTIONSERVICE_H

#include "Models.pb.h"

namespace neon {

    class FunctionService {
    public:
        void HandleFunctionQuery(const FunctionQuery& query, FunctionQuery::Result* result) const;
    private:
        WoWObjectsResult GetObjects() const;
        uintptr_t GetObjectPointer(uint64_t guid) const;
        void CopyToResult(void *bytes, size_t size, FunctionQuery::Result *result) const;
        uint64_t GetActivePlayerGuid() const;
        uint32_t GetMapId() const;
        bool ClickToMove(uint32_t action, float *point, uint64_t targetGuid, float precision) const;
    };
}


#endif //NEON_BACKEND_FUNCTIONSERVICE_H
