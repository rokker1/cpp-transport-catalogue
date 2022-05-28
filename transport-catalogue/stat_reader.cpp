#include "stat_reader.h"

namespace transport
{
namespace stat_reader
{

void Stat_reader::Add_request(std::istream& is) {
    string str;
    getline(is, str);

    size_t space_pos = str.find(" ");
    string_view operation = str.substr(0, space_pos);

    size_t req_begin = str.find_first_not_of(" ", space_pos);
    string req_text = str.substr(req_begin);
    
    if(operation == "Bus") {
        requests___.emplace_back(std::move(Request{RequestType::GET_BUS_INFO, req_text, ""}));
    } else if(operation == "Stop") {
        requests___.emplace_back(std::move(Request{RequestType::GET_STOP_INFO, "", req_text}));
    } else {
        return;
    }
}

const std::deque<Request>& Stat_reader::Get_all_reqs() {
    return requests___;
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

