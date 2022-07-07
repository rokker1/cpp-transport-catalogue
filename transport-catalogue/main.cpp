#include "json.h"
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

int main() {
    {
      json::Document doc = json::Load(std::cin);

      catalogue::TransportCatalogue cat;
      json_reader::JsonReader reader(doc, cat);

      catalogue::TransportRouter transport_router(
        reader.ReadRoutingSettings(doc), 
        cat);

      graph::Router<BusRouteWeight> router(cat.GetRouteGraph<BusRouteWeight>());

      renderer::MapRenderer renderer(reader.GetRenderSettings(), cat.GetBusesSorted());

      RequestHandler handler(cat, renderer, router);

      json::Document result = reader.ProcessStatRequests(handler);
      json::Print(result, std::cout);

      
    }

    return 0;
}