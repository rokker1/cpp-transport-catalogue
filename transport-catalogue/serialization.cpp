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
    }



    return result;
}

} // namespace Serialize


