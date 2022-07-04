#include "json.h"
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
int main() {
    {
      json::Document doc = json::Load(std::cin);

      catalogue::TransportCatalogue cat;
      json_reader::JsonReader reader(doc, cat);

      graph::Router<double> router(cat.GetRouteGraph<double>());

      renderer::MapRenderer renderer(reader.GetRenderSettings(), cat.GetBusesSorted());

      RequestHandler handler(cat, renderer);

      json::Document result = reader.ProcessStatRequests(handler);
      json::Print(result, std::cout);

      
    }

    return 0;
}