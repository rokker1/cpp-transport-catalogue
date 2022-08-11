#pragma once

#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include <filesystem>
#include <fstream>
#include <iostream>

class Serializer {

public:
    Serializer() = delete;
    Serializer(const catalogue::TransportCatalogue& catalogue)
        : catalogue_(catalogue) {
            // Заполнить pb_catalogue_

            // остановки
            for (const auto& stop : catalogue_.GetStops()) {
                tc_pb::Stop pb_stop;
                pb_stop.set_id(stop.id);
                pb_stop.set_name(stop.name_);
                pb_stop.mutable_coordinates()->set_lat(stop.cordinates_.lat);
                pb_stop.mutable_coordinates()->set_lng(stop.cordinates_.lng);
                pb_catalogue_.mutable_stops()->Add(std::move(pb_stop));
            }

            //автобусы
            for(const auto& bus : catalogue_.GetBuses()) {
                tc_pb::Bus pb_bus;
                pb_bus.set_id(bus.id);
                pb_bus.set_name(bus.name_);
                
                for(const auto& stop : bus.stops_) {
                    pb_bus.add_stops(stop->id);
                }

                switch (bus.bus_type_)
                {
                case BusType::ORDINARY:
                    pb_bus.set_bus_type_cycled(false);
                    break;
                case BusType::CYCLED:
                    pb_bus.set_bus_type_cycled(true);
                    break;
                default:
                    break;
                }
                pb_catalogue_.mutable_buses()->Add(std::move(pb_bus));
            }

            // stops_to_buses_
            for(const auto& stop_to_buses : catalogue_.GetStopsToBuses()) {
                tc_pb::StopToBuses pb_stop_to_buses;                
                
                int stop_id = stop_to_buses.first->id;
                pb_stop_to_buses.set_stop_id(stop_id);

                const std::set<const Bus*>& buses = stop_to_buses.second;
                for(const auto& bus : buses) {
                    pb_stop_to_buses.mutable_bus_id()->Add(bus->id);
                }
                pb_catalogue_.mutable_stops_to_buses()->Add(std::move(pb_stop_to_buses));
            }

            //intervals_to_distance_
            for(const auto& interval_to_distance : catalogue_.GetIntervalsToDistance()) {
                tc_pb::IntervalToDistance pb_interval_to_distance;
                int from_id = interval_to_distance.first.first->id;
                int to_id = interval_to_distance.first.second->id;
                int64_t distance = interval_to_distance.second;
                pb_interval_to_distance.set_from_id(from_id);
                pb_interval_to_distance.set_to_id(to_id);
                pb_interval_to_distance.set_distance(distance);
                pb_catalogue_.mutable_intervals_to_distance()->Add(std::move(pb_interval_to_distance));
            }
        }

    void SaveTo(const std::filesystem::path& path) const {
        std::fstream out(path, std::ios::binary);

        pb_catalogue_.SerializePartialToOstream(&out);
    }
    
private:
    const catalogue::TransportCatalogue& catalogue_;
    tc_pb::transport_catalogue pb_catalogue_;
};