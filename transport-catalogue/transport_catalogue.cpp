#include "transport_catalogue.h"

namespace transport
{
namespace catalogue {
void Transport_catalogue::AddBus(string_view name, const vector<string>& stops, BusType type) {
    vector<const Stop*> stops_ptr;
    auto it = buses_.insert(buses_.end(), std::move(Bus{string(name), stops_ptr, type}));
    
    for_each(stops.begin(), stops.end(), [&stops_ptr, &it, this](string_view stop_name){
        if(const Stop* stop_ptr = FindStop(stop_name)) { // если есть такая остановка в базе
            it->stops_.push_back(std::move(stop_ptr));
            //добавление в stops_to_buses_
            stops_to_buses_[stop_ptr].insert(it->name_);
        }
        
    });
    busname_to_bus_[string_view{it->name_}] = &(*it);
}

void Transport_catalogue::AddStop(string_view name, double lat, double lng) {
    auto it = all_stops_.emplace(all_stops_.end(), std::move(Stop{string(name), Coordinates{lat, lng}}));
    
    stopname_to_stop_[string_view{it->name_}] = &(*it);
}

const Stop* Transport_catalogue::FindStop(std::string_view name) const {
    if(stopname_to_stop_.count(name) == 0) {
        return nullptr;
    } else {
        return stopname_to_stop_.at(name);
    }
}

const Bus* Transport_catalogue::FindBus(std::string_view name) const {
    if(busname_to_bus_.count(name) == 0) {
        return nullptr;
    } else {
        return busname_to_bus_.at(name);
    }
}

BusInfo Transport_catalogue::GetBusInfo(string_view name) {
    if(busname_to_businfo_.count(name) == 0) {
        // статистики нет в базе
        return ComputeBusInfo(name);
    } else {
        // статистика уже есть в базе
        return *(busname_to_businfo_.at(name));
    }
}

StopInfo Transport_catalogue::GetStopInfo(string_view stop_name) {
    if(stopname_to_stop_.count(stop_name) != 0) {
        // остановка существует
        const Stop* stop_ptr = FindStop(stop_name);
        if(stops_to_buses_.count(stop_ptr) != 0) {
            //остановка есть и через нее едут автобусы
            return StopInfo{stops_to_buses_.at(stop_ptr), true};
        } else {
            return StopInfo{{}, true}; // остановка есть, а автобусов нет
        }
    } else {
        return StopInfo{{}, false}; // остановка не существует
    }
}

void Transport_catalogue::AddDistances(string_view name, const std::unordered_map<std::string, int>& distances) {
    const Stop* stop_from = FindStop(name); // откуда
    for(const auto&[stop_to_name, distance] : distances) {
        const Stop* stop_to = FindStop(stop_to_name); // куда
        intervals_to_distance_[{stop_from, stop_to}] = distance;
        // маршрут в обратную сторону еще не задавался
        // или задается расстояние к одной и той же остановке
        if(intervals_to_distance_.count({stop_to, stop_from}) == 0
            || stop_from == stop_to) { 
            // значит, что длина маршрута в обе стороны равна
            intervals_to_distance_[{stop_to, stop_from}] = distance;
        }
    }        
}

void Transport_catalogue::SetDistance(std::pair<const Stop*, const Stop*> p, double d) {
    intervals_to_distance_[p] = d;
    // маршрут в обратную сторону еще не задавался
    // или задается расстояние к одной и той же остановке
    if(intervals_to_distance_.count({p.second, p.first}) == 0
        || p.first == p.second) { 
        // значит, что длина маршрута в обе стороны равна
        intervals_to_distance_[p] = d;
    }
}

uint64_t Transport_catalogue::GetDistance(std::pair<const Stop*, const Stop*> p) const {
    if(intervals_to_distance_.count(p)) {
        return intervals_to_distance_.at(p);
    } else {
        return 0u;
    }
}

BusInfo Transport_catalogue::ComputeBusInfo(string_view name) {
    const Bus* bus{nullptr}; //указатель на автобус в деке
    unsigned int stops_count = 0, unique_stops_count = 0;
    double length_geo = 0;
    uint64_t length_road = 0;
    double curvature = 0.0;
    if(bus = FindBus(name), bus) {
        // такой автобус существует
        //bus = FindBus(name);
        stops_count = bus->stops_.size();

        std::unordered_set<string_view, std::hash<string_view>, std::equal_to<string_view>> unique_stops;
        for_each(bus->stops_.begin(), bus->stops_.end(), [&unique_stops](const Stop* stop){
            //std::cout << "Debug. \"" << stop->name_ << "\"." << std::endl;
            unique_stops.insert(string_view(stop->name_));
        });
        unique_stops_count = unique_stops.size();
        
        length_geo = std::transform_reduce(
            next(bus->stops_.begin()), bus->stops_.end(),
            bus->stops_.begin(),
            0.0,
            std::plus<>(),
            [](const Stop* s, const Stop* prev_s){
                if(s == prev_s) { return 0.0; } // между одной и той же остановкой нет расстояния
                return ComputeDistance(prev_s->cordinates_, s->cordinates_);
            }
        );

        length_road = std::transform_reduce(
            next(bus->stops_.begin()), bus->stops_.end(),
            bus->stops_.begin(),
            0ULL,
            std::plus<>(),
            [this, &bus](const Stop* s, const Stop* prev_s){
                if(bus->bus_type_ == BusType::CYCLED) {
                    return GetDistance({prev_s, s});
                } else if(bus->bus_type_ == BusType::ORDINARY) {
                    return GetDistance({prev_s, s}) + GetDistance({s, prev_s});
                } else {
                    uint64_t c = 0;
                    return c;
                }
                
            }
        );

        if(bus->bus_type_ == BusType::ORDINARY) { 
            length_geo *= 2;
            stops_count = stops_count * 2 - 1; 
        } //у обычного маршрута расстояние умножается на два

        curvature = static_cast<long double>(length_road) / static_cast<long double>(length_geo);

        // запишем рассчитанную информацию в базу
        auto it = bus_infos_.insert(bus_infos_.end(), std::move(BusInfo{stops_count, unique_stops_count, length_road, curvature}));
        busname_to_businfo_[name] = &*it;
        // возвращаем рассчитанный объект со статистикой
        return *it;
    } else {
        // несуществующий автобус
        return {};
    }
}    
} // namespace catalogue

} // namespace transport

