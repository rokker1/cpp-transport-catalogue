#include "serialization.h"

namespace Serialize
{

catalogue::TransportCatalogue Deserializer::GetTransportCatalogue() const {
    catalogue::TransportCatalogue result;
   
    const tc_pb::transport_catalogue& pb_catalogue = pb_base_.cat();
    
    {
        // остановки
        std::deque<Stop> stops;
        std::map<std::string_view, const Stop*> stopname_to_stop;
        for(const auto& pb_stop : pb_catalogue.stops()) {
            Stop& emplaced = stops.emplace_back(
                Stop{
                    std::string(pb_stop.name()),
                    geo::Coordinates{
                        pb_stop.coordinates().lat(),
                        pb_stop.coordinates().lng()
                    },
                    pb_stop.id()
                }
            );

            stopname_to_stop[std::string_view(emplaced.name_)] = &emplaced;
        }

        result.SetStops(std::move(stops));
        result.SetStopnameToStop(std::move(stopname_to_stop));
    }
    {
        // автобусы
        std::deque<Bus> buses;
        std::map<std::string_view, const Bus*> busname_to_bus;
        for(const auto& pb_bus : pb_catalogue.buses()) {
            std::vector<const Stop*> stops;
            for(int stop_id : pb_bus.stops()) {
                assert(result.GetStops().at(stop_id).id == stop_id);
                stops.push_back(&(result.GetStops().at(stop_id)));
            }
            
            BusType bus_type;
            if(pb_bus.bus_type_cycled()) {
                bus_type = BusType::CYCLED;
            } else {
                bus_type = BusType::ORDINARY;
            }
            Bus current_bus{
                std::string(pb_bus.name()),
                std::move(stops),
                bus_type,
                pb_bus.id()
            };
            Bus& emlplaced = buses.emplace_back(std::move(current_bus));

            busname_to_bus[std::string_view(emlplaced.name_)] = &(emlplaced);
        }

        result.SetBuses(std::move(buses));
        result.SetBusnameToBus(std::move(busname_to_bus));
    }
    {
        // stops_to_buses_
        std::unordered_map<const Stop*, std::set<const Bus*>> stops_to_buses;

        for(const auto& stop_to_buses : pb_catalogue.stops_to_buses()) {
            int stop_id = stop_to_buses.stop_id();
            // по индексу находим указатель на остановку
            const Stop* stop_ptr = &(result.GetStops().at(stop_id));
            assert(stop_id == stop_ptr->id);

            std::set<const Bus*> current_stop_buses;
            for(int bus_id : stop_to_buses.bus_id()) {
                const Bus* bus_ptr = &(result.GetBuses().at(bus_id));
                assert(bus_id == bus_ptr->id);
                stops_to_buses[stop_ptr].insert(bus_ptr);
            }
        }

        result.SetStopsToBuses(std::move(stops_to_buses));
    }
    {
        // intervals_to_distance_
        std::unordered_map<std::pair<const Stop*, const Stop*>, uint64_t, catalogue::RouteDistanceHasher> intervals_to_distance;

        for(const auto& interval : pb_catalogue.intervals_to_distance()) {
            int from_id = interval.from_id();
            int to_id = interval.to_id();
            int64_t distance = interval.distance();

            const Stop* stop_ptr_from = &(result.GetStops().at(from_id));
            assert(from_id == stop_ptr_from->id);
            const Stop* stop_ptr_to = &(result.GetStops().at(to_id));
            assert(to_id == stop_ptr_to->id);

            intervals_to_distance[{stop_ptr_from, stop_ptr_to}] = distance;
        }

        result.SetIntervalsToDistance(std::move(intervals_to_distance));
    }
    return result;
}

catalogue::RoutingSettings Deserializer::GetRoutingSettings() const {
    catalogue::RoutingSettings result;

    result.bus_velocity = pb_base_.routing_settings().bus_velocity();
    result.bus_wait_time = pb_base_.routing_settings().bus_wait_time();

    return result;
}

renderer::RenderSettings Deserializer::GetRenderSettings() const {
    renderer::RenderSettings result;

    const tc_pb::RenderSettings& pb_render_settings = pb_base_.rendder_settings();

    result.width = pb_render_settings.width();
    result.height = pb_render_settings.height();
    result.padding = pb_render_settings.padding();
    result.line_width = pb_render_settings.line_width();
    result.stop_radius = pb_render_settings.stop_radius();

    result.bus_label_font_size = pb_render_settings.bus_label_font_size();
    result.bus_label_offset = {
        pb_render_settings.bus_label_offset().x(),
        pb_render_settings.bus_label_offset().y(),
    };

    result.stop_label_font_size = pb_render_settings.stop_label_font_size();
    result.stop_label_offset = {
        pb_render_settings.stop_label_offset().x(),
        pb_render_settings.stop_label_offset().y(),
    };

    result.underlayer_color = std::move(ExtractSVGColorFromPBColor(pb_render_settings.underlayer_color()));
    result.underlayer_width = pb_render_settings.underlayer_width();

    for(const auto& pb_color : pb_render_settings.color_palette()) {
        result.color_palette.push_back(ExtractSVGColorFromPBColor(pb_color));
    }

    return result;
}

svg::Color Deserializer::ExtractSVGColorFromPBColor(tc_pb::Color pb_color) {
    svg::Color result;

    if(pb_color.is_initialized()) {
        if(pb_color.has_string_color()) {
            result = pb_color.string_color();

        } else if(pb_color.has_rgb_color()) {

            result = svg::Rgb{
                static_cast<uint8_t>(pb_color.rgb_color().red()), 
                static_cast<uint8_t>(pb_color.rgb_color().green()), 
                static_cast<uint8_t>(pb_color.rgb_color().blue())
            };
        } else if(pb_color.has_rgba_color()) {

            result = svg::Rgba{
                static_cast<uint8_t>(pb_color.rgba_color().red()), 
                static_cast<uint8_t>(pb_color.rgba_color().green()), 
                static_cast<uint8_t>(pb_color.rgba_color().blue()), 
                pb_color.rgba_color().opacity()
            };
        } else {
            throw std::runtime_error("Bad value in protobuf Color!");
        }
    }

    return result;
}

} // namespace Serialize


