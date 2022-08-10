#include "json.h"
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>
#include <fstream>
#include <iostream>
#include <string_view>
using namespace std::literals;


void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}
int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        // make base here

    } else if (mode == "process_requests"sv) {

        // process requests here

    } else {
        PrintUsage();
        return 1;
    }
}
// int main() {
//     {
      
      
//       json::Document doc = json::Load(std::cin);

//       catalogue::TransportCatalogue cat;
//       json_reader::JsonReader reader(doc);

//       catalogue::TransportRouter transport_router(
//         reader.ReadRoutingSettings(doc), 
//         cat);

//       // заполнение справочника и роутера
//       reader.Fill(cat, transport_router);

//       graph::Router<BusRouteWeight> router(transport_router.GetRouteGraph<BusRouteWeight>());

//       renderer::MapRenderer renderer(reader.GetRenderSettings(), cat.GetBusesSorted());

//       RequestHandler handler(cat, renderer, router, transport_router);

//       json::Document result = reader.ProcessStatRequests(handler);
//       json::Print(result, std::cout);
     
//     }
//     return 0;
// }