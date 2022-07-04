#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using namespace std::literals;

namespace json_reader {
void JsonReader::ProcessBaseRequests(json::Document document, catalogue::TransportCatalogue& catalogue) {
    assert(document.GetRoot().IsDict());
    assert(document.GetRoot().AsDict().count("base_requests"));
    SetRoutingSettings(ReadRoutingSettings(document), catalogue);
    ReadBaseRequests(document);
    FillCatalogueFromReader(catalogue);
    
}

void JsonReader::ReadBaseRequests(json::Document document) {
    json::Array base_requests = document.GetRoot().AsDict().at("base_requests").AsArray();

    // цикл по каждому отдельному запросу на добавление
    for(const json::Node& base_request : base_requests) {
        assert(base_request.IsDict());
        assert(base_request.AsDict().count("type") != 0);
        if(base_request.AsDict().at("type").AsString() == "Stop") {
            AddStopBaseRequest(base_request);
        } else if(base_request.AsDict().at("type").AsString() == "Bus") {
            AddBusBaseRequest(base_request);
        } else {
            throw std::exception();
        }
    }
}

void JsonReader::AddStopBaseRequest(const json::Node& request) {
    AddStopRequest add_stop_request{};
    add_stop_request.cordinates.lat = request.AsDict().at("latitude").AsDouble();
    add_stop_request.cordinates.lng = request.AsDict().at("longitude").AsDouble();
    add_stop_request.name = request.AsDict().at("name").AsString();
    const json::Dict& road_distances = request.AsDict().at("road_distances").AsDict();
    for(const auto& road_distance : road_distances) {
        add_stop_request.distances[road_distance.first] = road_distance.second.AsInt();
    }
    add_stop_requests_.push_back(std::move(add_stop_request));
}
void JsonReader::AddBusBaseRequest(const json::Node& request) {
    AddBusRequest add_bus_request{};
    add_bus_request.name = request.AsDict().at("name").AsString();
    add_bus_request.is_roundtrip = request.AsDict().at("is_roundtrip").AsBool();
    const json::Array& stops = request.AsDict().at("stops").AsArray();
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

        catalogue.AddStopVertex(stop_from);
        
    }

    catalogue.AddBusWaitEdges();
    
    //add verices
    //add internal edges = bus-wait
    for(const auto& [name, stops, type] : add_bus_requests_) {
        BusType bus_type;
        if(type) {
            bus_type = BusType::CYCLED;
        } else {
            bus_type = BusType::ORDINARY;
        }
        catalogue.AddBus(name, stops, bus_type);
    }
    //add edges
}

renderer::RenderSettings JsonReader::ReadRenderSettingsFromJSON(const json::Document& document) const {
    renderer::RenderSettings settings;
    assert(document.GetRoot().IsDict());
    assert(document.GetRoot().AsDict().count("render_settings") != 0);
    const json::Node& json_settings = document.GetRoot().AsDict().at("render_settings").AsDict();
    
    settings.bus_label_font_size = json_settings.AsDict().at("bus_label_font_size").AsInt();
    settings.bus_label_offset.x = json_settings.AsDict().at("bus_label_offset").AsArray()[0].AsDouble();
    settings.bus_label_offset.y = json_settings.AsDict().at("bus_label_offset").AsArray()[1].AsDouble();
    
    for(const json::Node& color : json_settings.AsDict().at("color_palette").AsArray()) {
        settings.color_palette.push_back(ReadColor(color));
    }

    settings.height = json_settings.AsDict().at("height").AsDouble();
    settings.line_width = json_settings.AsDict().at("line_width").AsDouble();
    settings.padding = json_settings.AsDict().at("padding").AsDouble();
    settings.stop_label_font_size = json_settings.AsDict().at("stop_label_font_size").AsInt();
    settings.stop_label_offset.x = json_settings.AsDict().at("stop_label_offset").AsArray()[0].AsDouble();
    settings.stop_label_offset.y = json_settings.AsDict().at("stop_label_offset").AsArray()[1].AsDouble();
    settings.stop_radius = json_settings.AsDict().at("stop_radius").AsDouble();
    settings.underlayer_color = ReadColor(json_settings.AsDict().at("underlayer_color"));
    settings.underlayer_width = json_settings.AsDict().at("underlayer_width").AsDouble();
    settings.width = json_settings.AsDict().at("width").AsDouble();

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
    
    json::Array stat_requests = document_.GetRoot().AsDict().at("stat_requests").AsArray();

    for(const auto& stat_request : stat_requests) {
        std::string_view request_type = stat_request.AsDict().at("type").AsString();
        if(request_type == "Bus"sv) {

            ProcessBusStatRequest(handler, stat_request, answers_array);

        } else if(request_type == "Stop"sv) {
            
            ProcessStopInfoRequest(handler, stat_request, answers_array);

        } else if(request_type == "Map"sv) {
            
            ProcessMapRequest(handler, stat_request, answers_array);
        
        } else {
            throw std::logic_error("bad stat request");
        }
    }

    json::Document document{answers_array};
    return document;
}

void JsonReader::ProcessBusStatRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array) {
    std::optional<BusStat> bus_stat = handler.GetBusStat(stat_request.AsDict().at("name").AsString());
    int id = stat_request.AsDict().at("id").AsInt();
    answers_array.push_back(std::move(ConvertBusStatToJsonDict(id, bus_stat)));
}

void JsonReader::ProcessStopInfoRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array) {
    int id = stat_request.AsDict().at("id").AsInt();
    std::optional<StopInfo> stop_info = handler.GetStopInfo(stat_request.AsDict().at("name").AsString());
    answers_array.push_back(std::move(ConvertStopInfoToJsonDict(id, stop_info)));
}

void JsonReader::ProcessMapRequest(RequestHandler& handler, const json::Node& stat_request, json::Array& answers_array) {
    int id = stat_request.AsDict().at("id").AsInt();
    std::string map_as_string = std::move(handler.RenderMapToString());
    answers_array.push_back(std::move(ConvertMapToJsonDict(id, map_as_string)));
}

json::Node JsonReader::ConvertBusStatToJsonDict(int id, std::optional<BusStat> bus_stat) {

    if(bus_stat.has_value()) {
        // автобус существует
        auto stat = bus_stat.value();
        json::Node answer = 
            json::Builder{}.StartDict()
                .Key("request_id").Value(id)
                .Key("curvature").Value(stat.curvative)
                .Key("unique_stop_count").Value(static_cast<int>(stat.unique_stops_count))
                .Key("stop_count").Value(static_cast<int>(stat.stops_count))
                .Key("route_length").Value(static_cast<int>(stat.length))
                .EndDict().Build();
       
        return answer;
    } else {
        json::Node answer{
            json::Builder{}.StartDict()
                .Key("request_id").Value(id)
                .Key("error_message").Value("not found")
                .EndDict().Build()
        };
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
        json::Node answer{
            json::Builder{}.StartDict()
                .Key("request_id").Value(id)
                .Key("buses").Value(buses)
            .EndDict().Build()
        };
        return answer;
    } else {
        json::Node answer{
            json::Builder{}.StartDict()
                .Key("request_id").Value(id)
                .Key("error_message").Value("not found")
                .EndDict().Build()
        };
        return answer;
    }
}
json::Node JsonReader::ConvertMapToJsonDict(int id, std::string map_as_string) {
    json::Node answer{
            json::Builder{}.StartDict()
                .Key("request_id").Value(id)
                .Key("map").Value(std::move(map_as_string))
            .EndDict().Build()
        };
    return answer;
}

catalogue::RoutingSettings JsonReader::ReadRoutingSettings(const json::Document& document) const {
    catalogue::RoutingSettings settings;
    assert(document.GetRoot().IsDict());
    assert(document.GetRoot().AsDict().count("routing_settings") != 0);
    const json::Node& json_settings = document.GetRoot().AsDict().at("routing_settings").AsDict();
    settings.bus_wait_time = json_settings.AsDict().at("bus_wait_time").AsInt();
    settings.bus_velocity = json_settings.AsDict().at("bus_velocity").AsInt();
    return settings;
}

void JsonReader::SetRoutingSettings(catalogue::RoutingSettings settings, catalogue::TransportCatalogue& catalogue) const {
    catalogue.SetRoutingSettings(std::move(settings));
}
}
