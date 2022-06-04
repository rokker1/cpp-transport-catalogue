#include "stat_reader.h"

namespace transport
{
namespace stat_reader
{

void Stat_reader::AddRequest(std::istream& is) {
    std::string str;
    getline(is, str);

    size_t space_pos = str.find(" ");
    std::string_view operation = str.substr(0, space_pos);

    size_t req_begin = str.find_first_not_of(" ", space_pos);
    std::string req_text = str.substr(req_begin);
    
    if(operation == "Bus") {
        requests_.emplace_back(std::move(Request{RequestType::GET_BUS_INFO, req_text, ""}));
    } else if(operation == "Stop") {
        requests_.emplace_back(std::move(Request{RequestType::GET_STOP_INFO, "", req_text}));
    } else {
        return;
    }
}

std::istream& operator>>(std::istream& is, Request& request) {
    std::string str;
    getline(is, str);

    size_t space_pos = str.find(" ");
    std::string_view operation = str.substr(0, space_pos);

    size_t req_begin = str.find_first_not_of(" ", space_pos);
    std::string req_text = str.substr(req_begin);
    
    if(operation == "Bus") {
        request = Request{RequestType::GET_BUS_INFO, req_text, ""};
        return is;
    } else if(operation == "Stop") {
        request = Request{RequestType::GET_STOP_INFO, "", req_text};
        return is;
    } else {
        return is;
    }
}

const std::deque<Request>& Stat_reader::GetAllRequests() {
    return requests_;
}

void PrintBusInfo(std::string_view bus, const BusInfo& info, std::ostream& os) {
    os << "Bus " << bus << ": " << info << std::endl;
}
void PrintStopInfo(std::string_view stop, const StopInfo& info, std::ostream& os) {
    os << "Stop " << stop << ": " << info << std::endl;
}
void ProcessRequests(std::istream& is, std::ostream& os, Transport_catalogue& catalogue) {
    int request_count;
    Request request;

    is >> request_count;
    std::string nullstring;
    getline(is, nullstring);
    for (int i = 0; i < request_count; ++i)
    {
        is >> request;
        switch (request.type)
        {
        case RequestType::GET_BUS_INFO:
            PrintBusInfo(request.bus, catalogue.GetBusInfo(request.bus), os);
            break;
        case RequestType::GET_STOP_INFO:
            PrintStopInfo(request.stop, catalogue.GetStopInfo(request.stop), os);
            break;

        default:
            break;
        }
    }
}

std::ostream& operator<<(std::ostream& out, const BusInfo& info) {
    if(info.IsEmpty()) {
        out << "not found";
    } else {
        out << info.stops_count << " stops on route, " 
        << info.unique_stops_count << " unique stops, " 
        << info.length << " route length, "
        << info.curvative << " curvature";
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const StopInfo& info) {
    if(!info.IsExsists) {
        out << "not found";
    } else if (!info.buses_.empty()) {
        out << "buses";
        for(const auto& s : info.buses_) {
            out << " ";
            out << s;
        }
    } else {
        out << "no buses";
    }
    return out;
}        
} // namespace stat_reader
} // namespace transport

