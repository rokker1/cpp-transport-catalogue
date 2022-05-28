#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
#include "geo.h"

int main () {
    using namespace std;
    using namespace transport::catalogue;
    using namespace transport::input_reader;
    using namespace transport::stat_reader;
    int query_count;
    string str;

    cin >> query_count;
    getline(cin, str);

    Transport_catalogue transport_catalogue;
    Input_reader input_reader;
    for(int i = 0; i < query_count; ++i) {
        input_reader.Add_input_query(cin);
    }

    auto stop_queries = input_reader.Get_add_stop_queries();
    for(const auto& [name, coordinates, _] : stop_queries) {
        transport_catalogue.AddStop(name, coordinates.lat, coordinates.lng);
    }
    for(const auto& [name, _, distances] : stop_queries) {
        transport_catalogue.AddDistances(name, distances);
    }
    //transport_catalogue.PrintIntervals();

    auto bus_queries = input_reader.Get_add_bus_queries();
    for(const auto& [name, stops, type] : bus_queries) {
        transport_catalogue.AddBus(name, stops, type);
    }


    Stat_reader request_reader;
    int request_count = 0;
    cin >> request_count;
    getline(cin, str);
    for(int i = 0; i < request_count; ++i) {
        request_reader.Add_request(cin);
    }

    for(const auto& request : request_reader.Get_all_reqs())
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
    }
    //cout << "ok" << endl;
}