//
// Created by clanat on 08.07.18.
//

#ifndef NEON_BACKEND_APISERVICE_H
#define NEON_BACKEND_APISERVICE_H

#include <string>
#include <iostream>
#include <memory>

#include "zmq.hpp"
#include "Models.pb.h"

namespace neon {
    class MemoryService;
    class FunctionService;

    class APIService {
    public:
        APIService(const char* address, MemoryService& memoryService, FunctionService& functionService);
        void Update();
    private:
        const char* _address;
        zmq::context_t _context;
        zmq::socket_t _socket;
        zmq::pollitem_t _pollItem;

        MemoryService& _memoryService;
        FunctionService& _functionService;

        void HandleEvents();
        void HandleData(const void *data, const size_t size);
        void HandleDataUnsafe(const void *data, const size_t size);

        void SendError(const std::string &message);
        void Send(const Response &response);
    };
}


#endif //NEON_BACKEND_APISERVICE_H
