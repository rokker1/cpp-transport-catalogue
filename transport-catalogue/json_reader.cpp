#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using namespace std::literals;

namespace json_reader {
void JsonReader::ProcessBaseRequests(json::Document document, catalogue::TransportCatalogue& catalogue) {
    assert(document.GetRoot().IsMap());
    assert(document.GetRoot().AsMap().count("base_requests"));
    ReadBaseRequests(document);
    FillCatalogueFromReader(catalogue);
}

void JsonReader::ReadBaseRequests(json::Document document) {
    json::Array base_requests = document.GetRoot().AsMap().at("base_requests").AsArray();

    // цикл по каждому отдельному запросу на добавление
    for(const json::Node& base_request : base_requests) {
        assert(base_request.IsMap());
        assert(base_request.AsMap().count("type") != 0);
        if(base_request.AsMap().at("type").AsString() == "Stop") {
            AddStopBaseRequest(base_request);
        } else if(base_request.AsMap().at("type").AsString() == "Bus") {
            AddBusBaseRequest(base_request);
        } else {
            throw std::exception();
        }
    }
}

void JsonReader::AddStopBaseRequest(const json::Node& request) {
    AddStopRequest add_stop_request{};
    add_stop_request.cordinates.lat = request.AsMap().at("latitude").AsDouble();
    add_stop_request.cordinates.lng = request.AsMap().at("longitude").AsDouble();
    add_stop_request.name = request.AsMap().at("name").AsString();
    const json::Dict& road_distances = request.AsMap().at("road_distances").AsMap();
    for(const auto& road_distance : road_distances) {
        add_stop_request.distances[road_distance.first] = road_distance.second.AsInt();
    }
    add_stop_requests_.push_back(std::move(add_stop_request));
}
void JsonReader::AddBusBaseRequest(const json::Node& request) {
    AddBusRequest add_bus_request{};
    add_bus_request.name = request.AsMap().at("name").AsString();
    add_bus_request.is_roundtrip = request.AsMap().at("is_roundtrip").AsBool();
    const json::Array& stops = request.AsMap().at("stops").AsArray();
    for(const auto& stop : stops) {
        add_bus_request.stops.push_back(std::move(stop.AsString()));
    }
    add_bus_requests_.push_back(std::move(add_bus_request));
}

void JsonReader::FillCatalogueFromReader(catalogue::TransportCatalogue& catalogue) {
    for(const auto& [name, coordinates, _] : add_stop_requests_) {
        catalogue.AddStop(name, coordinates);
    }
    for(const auto& [name_from, _, distances] : add_stop_requests_) {
        const Stop* stop_from = catalogue.FindStop(name_from);
        for(const auto& [name_to, distance] : distances) {
            const Stop* stop_to = catalogue.FindStop(name_to);
            catalogue.SetDistance({stop_from, stop_to}, distance);
        }
    }
    for(const auto& [name, stops, type] : add_bus_requests_) {
        BusType bus_type;
        if(type) {
            bus_type = BusType::CYCLED;
        } else {
            bus_type = BusType::ORDINARY;
        }
        catalogue.AddBus(name, stops, bus_type);
    }
}

renderer::RenderSettings JsonReader::ReadRenderSettingsFromJSON(const json::Document& document) const {
    renderer::RenderSettings settings;
    assert(document.GetRoot().IsMap());
    assert(document.GetRoot().AsMap().count("render_settings") != 0);
    const json::Node& json_settings = document.GetRoot().AsMap().at("render_settings").AsMap();
    
    settings.bus_label_font_size = json_settings.AsMap().at("bus_label_font_size").AsInt();
    settings.bus_label_offset.x = json_settings.AsMap().at("bus_label_offset").AsArray()[0].AsDouble();
    settings.bus_label_offset.y = json_settings.AsMap().at("bus_label_offset").AsArray()[1].AsDouble();
    
    for(const json::Node& color : json_settings.AsMap().at("color_palette").AsArray()) {
        settings.color_palette.push_back(ReadColor(color));
    }

    settings.height = json_settings.AsMap().at("height").AsDouble();
    settings.line_width = json_settings.AsMap().at("line_width").AsDouble();
    settings.padding = json_settings.AsMap().at("padding").AsDouble();
    settings.stop_label_font_size = json_settings.AsMap().at("stop_label_font_size").AsInt();
    settings.stop_label_offset.x = json_settings.AsMap().at("stop_label_offset").AsArray()[0].AsDouble();
    settings.stop_label_offset.y = json_settings.AsMap().at("stop_label_offset").AsArray()[1].AsDouble();
    settings.stop_radius = json_settings.AsMap().at("stop_radius").AsDouble();
    settings.underlayer_color = ReadColor(json_settings.AsMap().at("underlayer_color"));
    settings.underlayer_width = json_settings.AsMap().at("underlayer_width").AsDouble();
    settings.width = json_settings.AsMap().at("width").AsDouble();

    return settings;
}

svg::Color JsonReader::ReadColor(const json::Node& node) const {
    svg::Color color;
    if(std::holds_alternative<json::Array>(node.GetValue())) {
        switch (node.AsArray().size())
        {
        case 3:
            color = svg::Rgb{
                static_cast<unsigned char>(node.AsArray()[0].AsInt()),
                static_cast<unsigned char>(node.AsArray()[1].AsInt()),
                static_cast<unsigned char>(node.AsArray()[2].AsInt())
            };
            break;
        case 4:
            color = svg::Rgba{
                static_cast<unsigned char>(node.AsArray()[0].AsInt()),
                static_cast<unsigned char>(node.AsArray()[1].AsInt()),
                static_cast<unsigned char>(node.AsArray()[2].AsInt()),
                node.AsArray()[3].AsDouble()
            };
            break;
        default:
            break;
        };
    } else if (std::holds_alternative<std::string>(node.GetValue())) {
        color = node.AsString();
    } else {
        throw std::bad_variant_access();
    }
    return color;
}
JsonReader::JsonReader(json::Document document, catalogue::TransportCatalogue& catalogue) 
    : document_(document), catalogue_(catalogue)
{
    ProcessBaseRequests(document_, catalogue_); 
}

renderer::RenderSettings JsonReader::GetRenderSettings() const {
    return ReadRenderSettingsFromJSON(document_);
}
json::Document JsonReader::ProcessStatRequests(RequestHandler& handler) {
    json::Array answers_array{};
    
    json::Array stat_requests = document_.GetRoot().AsMap().at("stat_requests").AsArray();

    for(const auto& stat_request : stat_requests) {
        std::string_view request_type = stat_request.AsMap().at("type").AsString();
        if(request_type == "Bus"sv) {

            std::optional<BusStat> bus_stat = handler.GetBusStat(stat_request.AsMap().at("name").AsString());
            int id = stat_request.AsMap().at("id").AsInt();
            answers_array.push_back(std::move(ConvertBusStatToJsonDict(id, bus_stat)));

        } else if(request_type == "Stop"sv) {
            
            int id = stat_request.AsMap().at("id").AsInt();
            std::optional<StopInfo> stop_info = handler.GetStopInfo(stat_request.AsMap().at("name").AsString());
            answers_array.push_back(std::move(ConvertStopInfoToJsonDict(id, stop_info)));

        } else if(request_type == "Map"sv) {
            
            int id = stat_request.AsMap().at("id").AsInt();
            std::string map_as_string = std::move(handler.RenderMapToString());
            answers_array.push_back(std::move(ConvertMapToJsonDict(id, map_as_string)));
        
        } else {
            throw std::logic_error("bad stat request");
        }
    }

    json::Document document{answers_array};
    return document;
}


json::Node JsonReader::ConvertBusStatToJsonDict(int id, std::optional<BusStat> bus_stat) {

    if(bus_stat.has_value()) {
        // автобус существует
        auto stat = bus_stat.value();
        json::Node answer{json::Dict{
                {"request_id", id},
                {"curvature", stat.curvative},
                {"unique_stop_count", static_cast<int>(stat.unique_stops_count)},
                {"stop_count", static_cast<int>(stat.stops_count)},
                {"route_length", static_cast<int>(stat.length)},
            }};
        return answer;
    } else {
        // запрос несуществующего автобуса
        json::Node answer{json::Dict{
            {"request_id", id},
            {"error_message", "not found"s},
        }};
        return answer;
    }
}
json::Node JsonReader::ConvertStopInfoToJsonDict(int id, std::optional<StopInfo> stop_info) {
    if(stop_info.has_value()) {
        // остановка существует
        auto info = stop_info.value();
        
        json::Array buses;
        for(const auto bus : info.buses_) {
            buses.push_back(std::move(std::string(bus)));
        }
        json::Node answer{json::Dict{
                {"request_id", id},
                {"buses", buses}
            }};
        return answer;
    } else {
        // запрос несуществующего остановки
        json::Node answer{json::Dict{
            {"request_id", id},
            {"error_message", "not found"s},
        }};
        return answer;
    }
}
json::Node JsonReader::ConvertMapToJsonDict(int id, std::string map_as_string) {
    json::Node answer{json::Dict{
        {"request_id", id},
        {"map", std::move(map_as_string)},
    }};
    return answer;
}

}
