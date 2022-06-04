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
    std::string bus;
    std::string stop;
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
void PrintBusInfo(std::string_view bus, const BusInfo& info, std::ostream& os);
void PrintStopInfo(std::string_view stop, const StopInfo& info, std::ostream& os);    
void ProcessRequests(std::istream& is, std::ostream& os, Transport_catalogue& catalogue);
} // namespace stat_reader

} // namespace transport
