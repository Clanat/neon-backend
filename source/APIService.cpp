//
// Created by clanat on 08.07.18.
//

#include "APIService.h"
#include "glx_hook.h"
#include "zmq.hpp"
#include "MemoryService.h"
#include "FunctionService.h"

#include <exception>
#include <iostream>
#include <memory>

using namespace neon;

void glx_hook() {
    static neon::MemoryService memoryService;
    static neon::FunctionService functionService;

    // TODO: use current address
    static neon::APIService apiService("tcp://192.168.0.24:60000", memoryService, functionService);

    apiService.Update();
}

APIService::APIService(const char *address, MemoryService& memoryService, FunctionService& functionService):
        _address(address),
        _memoryService(memoryService),
        _functionService(functionService),
        _context(),
        _socket({_context, zmq::socket_type::rep}),
        _pollItem() {

    auto reconnectInterval = 10000;
    _socket.setsockopt(ZMQ_RECONNECT_IVL, &reconnectInterval);
    _socket.bind(_address);
    _pollItem.events = ZMQ_POLLIN;
    _pollItem.socket = _socket;
}

void APIService::Update() {
    if (zmq::poll(&_pollItem, 1, 1) > 0) {
        HandleEvents();
    }
}

void APIService::HandleEvents() {
    if ((_pollItem.events & ZMQ_POLLIN) == ZMQ_POLLIN) {
        unsigned char buffer[512];
        auto size = _socket.recv(buffer, 512);
        if (size < 0) {
            throw std::runtime_error("Could not receive message");
        }

        HandleData(&buffer[0], size);
    }
}

void APIService::HandleData(const void *data, const size_t size) {
    try {
        HandleDataUnsafe(data, size);
    }
    catch (const std::exception& exception) {
        SendError(exception.what());
    }
}

void APIService::HandleDataUnsafe(const void *data, const size_t size) {
    Request request;
    if (!request.ParseFromArray(data, size)) {
        throw std::runtime_error("Could not parse message");
    }

    if (!_memoryService.isPlayerOnline()) {
        SendError("Player is not online");
        return;
    }

    Response response;
    switch (request.kind()) {
        case Request::MEMORY_QUERY: {
            MemoryQuery query;
            if (!query.ParseFromString(request.data())) {
                throw std::runtime_error("Could not parse MemoryQuery");
            }

            MemoryQuery::Result result;
            _memoryService.HandleMemoryQuery(query, &result);
            response.set_data(result.SerializeAsString());
            response.set_kind(Response::MEMORY_QUERY);
            break;
        }
        case Request::MEMORY_BATCH_QUERY: {
            MemoryBatchQuery query;
            if (!query.ParseFromString(request.data())) {
                throw std::runtime_error("Could not parse MemoryBatchQuery");
            }
            MemoryBatchQuery::Result result;
            _memoryService.HandleMemoryBatchQuery(query, &result);
            response.set_data(result.SerializeAsString());
            response.set_kind(Response::MEMORY_BATCH_QUERY);
            break;
        }
        case Request::FUNCTION_QUERY: {
            FunctionQuery query;
            if (!query.ParseFromString(request.data())) {
                throw std::runtime_error("Could not parse MemoryBatchQuery");
            }
            FunctionQuery::Result result;
            _functionService.HandleFunctionQuery(query, &result);
            response.set_data(result.SerializeAsString());
            response.set_kind(Response::FUNCTION_QUERY);
            break;
        }
        default:
            throw std::runtime_error("Unknown request kind");
    }
    Send(response);
}

void APIService::SendError(const std::string& message) {
    Response::Error error;
    error.set_message(message);

    Response response;
    response.set_kind(Response::ERROR);
    response.set_data(error.SerializeAsString());
    Send(response);
}

void APIService::Send(const Response &response) {
    const auto dataSize = response.ByteSizeLong();
    void* data = malloc(dataSize);
    if (!response.SerializeToArray(data, dataSize)) {
        throw std::runtime_error("Could not serialize response");
    }

    if (_socket.send(data, dataSize) != dataSize) {
        throw std::runtime_error("Could not send response");
    }
}