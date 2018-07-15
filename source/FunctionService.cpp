//
// Created by clanat on 11.07.18.
//

#include <functional>

#include "FunctionService.h"

using namespace neon;

#define ADDR_IS_PLAYER_ONLINE
#define ADDR_GET_OBJ_PTR 0x00468460
#define ADDR_GET_ACTIVE_PLR_GUID 0x00468550
#define ADDR_GET_MAP_ID 0x00468580
#define ADDR_CLICK_TO_MOVE 0x00611130

#define ADDR_OBJ_MGR 0x00B41414

#define FIRST_OBJ_OFFSET 0xAC
#define NEXT_OBJ_OFFSET 0x3C

#define OBJ_TYPE_OFFSET 0x14
#define OBJ_GUID_OFFSET 0X30

void FunctionService::HandleFunctionQuery(const FunctionQuery &query, FunctionQuery::Result *result) const {
    switch (query.kind()) {
        case FunctionQuery::OBJECT_POINTER: {
            auto guid = *(uint64_t *)query.args()[0].c_str();
            auto pointer = GetObjectPointer(guid);
            CopyToResult(&pointer, sizeof(pointer), result);
            break;
        }
        case FunctionQuery::ACTIVE_PLAYER_POINTER: {
            auto guid = GetActivePlayerGuid();
            auto pointer = GetObjectPointer(guid);
            CopyToResult(&pointer, sizeof(pointer), result);
            break;
        }
        case FunctionQuery::ACTIVE_PLAYER_GUID: {
            auto guid = GetActivePlayerGuid();
            CopyToResult(&guid, sizeof(guid), result);
            break;
        }
        case FunctionQuery::MAP_ID: {
            auto mapId = GetMapId();
            CopyToResult(&mapId, sizeof(mapId), result);
            break;
        }
        case FunctionQuery::ENUM_VISIBLE_OBJECTS: {
            result->set_data(GetObjects().SerializeAsString());
            break;
        }
        case FunctionQuery::CLICK_TO_MOVE: {
            uint32_t action;
            float point[3];
            uint64_t targetGuid;
            float precision;
            memcpy(&action, query.args()[0].c_str(), sizeof(uint32_t));
            memcpy(&point[0], query.args()[1].c_str(), sizeof(float) * 3);
            memcpy(&targetGuid, query.args()[2].c_str(), sizeof(uint64_t));
            memcpy(&precision, query.args()[3].c_str(), sizeof(float));

            auto returnValue = ClickToMove(action, point, targetGuid, precision);
            CopyToResult(&returnValue, sizeof(result), result);
            break;
        }
        default:
            throw std::runtime_error("Unknown FunctionQuery kind");

    }
}

WoWObjectsResult FunctionService::GetObjects() const {
    WoWObjectsResult result;

    const auto nextObjectPtr = [](uintptr_t objectPointer) -> uintptr_t {
        auto ptr = *(uintptr_t *)(objectPointer + NEXT_OBJ_OFFSET);
        auto isValid = objectPointer != 0 && (objectPointer & 1) == 0;
        return isValid ? ptr : 0;
    };

    auto managerPtr = *(uintptr_t *)(ADDR_OBJ_MGR);
    auto objectPtr = *(uintptr_t *)(managerPtr + FIRST_OBJ_OFFSET);

    while (objectPtr != 0) {
        result.mutable_pointers()->Add(objectPtr);
        result.mutable_guids()->Add(*(uint64_t *)(objectPtr + OBJ_GUID_OFFSET));
        result.mutable_types()->Add(*(uint8_t *)(objectPtr + OBJ_TYPE_OFFSET));
        objectPtr = nextObjectPtr(objectPtr);
    }

    return result;
}

uintptr_t FunctionService::GetObjectPointer(uint64_t guid) const {
    typedef void * (*func_t)(uint64_t);
    auto func = (func_t)(void *)ADDR_GET_OBJ_PTR;
    return (uintptr_t)func(guid);
}

uint64_t FunctionService::GetActivePlayerGuid() const {
    typedef uint64_t (*func_t)();
    auto func = (func_t)(void *)ADDR_GET_ACTIVE_PLR_GUID;
    return func();
}

uint32_t FunctionService::GetMapId() const {
    typedef uint32_t (*func_t)();
    auto func = (func_t)(void *)ADDR_GET_MAP_ID;
    return func();
}

bool FunctionService::ClickToMove(uint32_t action, float *point, uint64_t targetGuid, float precision) const {
    typedef bool (*func_t)(uint32_t, uint64_t*, float*, float) __attribute__ ((stdcall));

    auto playerPtr = (void *)GetObjectPointer(GetActivePlayerGuid());
    if (playerPtr == nullptr) {
        return false;
    }

    auto func = (func_t)(void *)ADDR_CLICK_TO_MOVE;
    asm volatile ("movl %0, %%ecx" : : "m" (playerPtr));    // thiscall
    return func(action, &targetGuid, point, precision);
}

void FunctionService::CopyToResult(void *bytes, size_t size, FunctionQuery::Result *result) const {
    if (size == 0) {
        return;
    }

    char data[size];
    memcpy(data, bytes, size);
    result->set_data(data);
}