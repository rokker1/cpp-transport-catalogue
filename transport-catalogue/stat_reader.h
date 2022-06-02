#pragma once

#include <iostream>
#include "transport_catalogue.h"

namespace transport
{
namespace stat_reader
{

using namespace catalogue;

enum class RequestType {
    GET_BUS_INFO,
    GET_STOP_INFO,
    MAX_REQUEST_TYPES,
};

struct Request
{
    RequestType type;
    string bus;
    string stop;
};
class Stat_reader {
public:
    void AddRequest(std::istream& is);

    const std::deque<Request>& GetAllRequests();
private:
    std::deque<Request> requests_;
};

std::ostream& operator<<(std::ostream& out, const BusInfo& info);
std::ostream& operator<<(std::ostream& out, const StopInfo& info);
    
} // namespace stat_reader

} // namespace transport
