#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
#include "geo.h"


void ReadInput(std::istream& in, transport::catalogue::Transport_catalogue& catalogue) {
       
    using namespace transport::catalogue;
    using namespace transport::input_reader;
    using namespace transport::stat_reader; 
    Input_reader reader;
    int query_count;
    std::string str;

    in >> query_count;
    getline(in, str);
    for(int i = 0; i < query_count; ++i) {
        reader.AddInputQuery(in);
    }

    auto stop_queries = reader.GetStopInputQueries();
    for(const auto& [name, coordinates, _] : stop_queries) {
        catalogue.AddStop(name, coordinates.lat, coordinates.lng);
    }
    for(const auto& [name, _, distances] : stop_queries) {
        catalogue.AddDistances(name, distances);
    }
    //transport_catalogue.PrintIntervals();

    auto bus_queries = reader.GetBusInputQueries();
    for(const auto& [name, stops, type] : bus_queries) {
        catalogue.AddBus(name, stops, type);
    }
}

int main () {
    using namespace std;
    using namespace transport::catalogue;
    using namespace transport::input_reader;
    using namespace transport::stat_reader;
    Transport_catalogue transport_catalogue;

    ReadInput(std::cin, transport_catalogue);
    ProcessRequests(std::cin, std::cout, transport_catalogue);
  /* std::string str;
    Stat_reader request_reader;
    int request_count = 0;
    cin >> request_count;
    getline(cin, str);
    for(int i = 0; i < request_count; ++i) {
        request_reader.AddRequest(cin);
    }

    for(const auto& request : request_reader.GetAllRequests())
    {
        if(request.type == RequestType::GET_BUS_INFO) {
            auto info = transport_catalogue.GetBusInfo(request.bus);
            std::cout << "Bus " << request.bus << ": " << info << endl;
        } else if(request.type == RequestType::GET_STOP_INFO) {
            auto info = transport_catalogue.GetStopInfo(request.stop);
            std::cout << "Stop " << request.stop << ": " << info << std::endl;
        } else {
            ;
        }
    }*/
    //cout << "ok" << endl;
}