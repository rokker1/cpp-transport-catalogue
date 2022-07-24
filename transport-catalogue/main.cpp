#include "json.h"
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

int main() {
    {
      json::Document doc = json::Load(std::cin);

      catalogue::TransportCatalogue cat;
      json_reader::JsonReader reader(doc);

      catalogue::TransportRouter transport_router(
        reader.ReadRoutingSettings(doc), 
        cat);

      // заполнение справочника и роутера
      reader.Fill(cat, transport_router);

      graph::Router<BusRouteWeight> router(transport_router.GetRouteGraph<BusRouteWeight>());

      renderer::MapRenderer renderer(reader.GetRenderSettings(), cat.GetBusesSorted());

      RequestHandler handler(cat, renderer, router, transport_router);

      json::Document result = reader.ProcessStatRequests(handler);
      json::Print(result, std::cout);
     
    }
    return 0;
}