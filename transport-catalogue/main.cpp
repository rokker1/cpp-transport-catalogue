#include "json.h"
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <transport_catalogue.pb.h>
#include <fstream>
#include <iostream>
#include <string_view>
using namespace std::literals;


void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}
int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     PrintUsage();
    //     return 1;
    // }

    // const std::string_view mode(argv[1]);

    // if (mode == "make_base"sv) {
        {
        // make base here
        // json::Document doc = json::Load(std::cin);
        std::ifstream input_base_requests("s14_1_opentest_3_make_base.json");
        json::Document doc = json::Load(input_base_requests);
        json_reader::JsonReader reader(doc);

        catalogue::TransportCatalogue cat;
        catalogue::TransportRouter transport_router(
            reader.ReadRoutingSettings(doc), 
            cat);
        // заполнение справочника и роутера
        reader.Fill(cat, transport_router);
        //graph::Router<BusRouteWeight> router(transport_router.GetRouteGraph<BusRouteWeight>());

        //renderer::MapRenderer renderer(reader.GetRenderSettings(), cat.GetBusesSorted());

        // ---- serialization moment! ++++
        Serialize::Serializer serializer_2000(
            cat,
            transport_router.GetRoutingSettings(),
            reader.GetRenderSettings(),
            reader.ReadSerializeSettings(doc)
        );
        serializer_2000.Save();
        }


    // } else if (mode == "process_requests"sv) {
        {
        // process requests here
        // json::Document doc = json::Load(std::cin);
        std::ifstream input_base_requests("s14_1_opentest_3_process_requests.json");
        json::Document doc = json::Load(input_base_requests);
        json_reader::JsonReader reader(doc);

        Serialize::Deserializer deserializer(reader.ReadSerializeSettings(doc));

        catalogue::TransportCatalogue cat = deserializer.GetTransportCatalogue();

        catalogue::TransportRouter transport_router(
            deserializer.GetRoutingSettings(),
            cat
        );
        renderer::MapRenderer renderer(deserializer.GetRenderSettings(), cat.GetBusesSorted());
        graph::Router<BusRouteWeight> router(transport_router.GetRouteGraph<BusRouteWeight>());

        RequestHandler handler(cat, renderer, router, transport_router);

        json::Document result = reader.ProcessStatRequests(handler);
        std::ofstream output_result("s14_1_opentest_3_answer_my.json");
        json::Print(result, output_result);
        //json::Print(result, std::cout);
        }


    // } else {
    //     PrintUsage();
    //     return 1;
    // }
}
// int main() {
//     {
      
      
//         json::Document doc = json::Load(std::cin);

//         catalogue::TransportCatalogue cat;
//         json_reader::JsonReader reader(doc);

//         catalogue::TransportRouter transport_router(
//             reader.ReadRoutingSettings(doc), 
//             cat);

//         // заполнение справочника и роутера
//         reader.Fill(cat, transport_router);

//         graph::Router<BusRouteWeight> router(transport_router.GetRouteGraph<BusRouteWeight>());

//         renderer::MapRenderer renderer(reader.GetRenderSettings(), cat.GetBusesSorted());

//         // ---- serialization moment! ++++
//         Serialize::Serializer serializer_2000(
//             cat,
//             transport_router.GetRoutingSettings(),
//             renderer.GetRenderSettings(),
//             reader.ReadSerializeSettings(doc)
//         );
//         serializer_2000.SaveTo("debug_serialize.bin");
//         Serialize::Deserializer deserializer("debug_serialize.bin");
//         catalogue::TransportCatalogue second_catalogue(std::move(deserializer.GetTransportCatalogue()));
//         catalogue::RoutingSettings routing_settings = deserializer.GetRoutingSettings();
//         renderer::RenderSettings render_settings = deserializer.GetRenderSettings();

//         RequestHandler handler(cat, renderer, router, transport_router);
//         // ------------------------------------------ stage 2
//         json::Document result = reader.ProcessStatRequests(handler);
//         json::Print(result, std::cout);
     
//     }
//     return 0;
// }