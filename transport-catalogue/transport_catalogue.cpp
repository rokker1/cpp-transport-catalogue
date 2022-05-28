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
    busname_to_bus_[name] = &(*it);
}

void Transport_catalogue::AddStop(string_view name, double lat, double lng) {
    auto it = all_stops_.emplace(all_stops_.end(), std::move(Stop{string(name), Coordinates{lat, lng}}));
    stopname_to_stop_[name] = &(*it);
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
    if(busname_to_businfo__.count(name) == 0) {
        // статистики нет в базе
        return Compute_bus_info(name);
    } else {
        // статистика уже есть в базе
        return *(busname_to_businfo__.at(name));
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
        intervals_to_distance__[{stop_from, stop_to}] = distance;
        // маршрут в обратную сторону еще не задавался
        // или задается расстояние к одной и той же остановке
        if(intervals_to_distance__.count({stop_to, stop_from}) == 0
            || stop_from == stop_to) { 
            // значит, что длина маршрута в обе стороны равна
            intervals_to_distance__[{stop_to, stop_from}] = distance;
        }
    }        
}

void Transport_catalogue::Set_distance(std::pair<const Stop*, const Stop*> p, double d) {
    intervals_to_distance__[p] = d;
    // маршрут в обратную сторону еще не задавался
    // или задается расстояние к одной и той же остановке
    if(intervals_to_distance__.count({p.second, p.first}) == 0
        || p.first == p.second) { 
        // значит, что длина маршрута в обе стороны равна
        intervals_to_distance__[p] = d;
    }
}

uint64_t Transport_catalogue::Get_distance(std::pair<const Stop*, const Stop*> p) const {
    if(intervals_to_distance__.count(p)) {
        return intervals_to_distance__.at(p);
    } else {
        return 0u;
    }
}

BusInfo Transport_catalogue::Compute_bus_info(string_view name) {
    const Bus* bus{nullptr}; //указатель на автобус в деке
    unsigned int R = 0, U = 0;
    double L_geo = 0;
    uint64_t L_road = 0;
    double C = 0.0;
    if(bus = FindBus(name), bus) {
        // такой автобус существует
        //bus = FindBus(name);
        R = bus->stops_.size();

        std::unordered_set<string_view, std::hash<string_view>, std::equal_to<string_view>> unique_stops;
        for_each(bus->stops_.begin(), bus->stops_.end(), [&unique_stops](const Stop* stop){
            //std::cout << "Debug. \"" << stop->name_ << "\"." << std::endl;
            unique_stops.insert(string_view(stop->name_));
        });
        U = unique_stops.size();
        
        L_geo = std::transform_reduce(
            next(bus->stops_.begin()), bus->stops_.end(),
            bus->stops_.begin(),
            0.0,
            std::plus<>(),
            [](const Stop* s, const Stop* prev_s){
                if(s == prev_s) { return 0.0; } // между одной и той же остановкой нет расстояния
                return ComputeDistance(prev_s->cordinates_, s->cordinates_);
            }
        );

        L_road = std::transform_reduce(
            next(bus->stops_.begin()), bus->stops_.end(),
            bus->stops_.begin(),
            0ULL,
            std::plus<>(),
            [this, &bus](const Stop* s, const Stop* prev_s){
                if(bus->bus_type_ == BusType::CYCLED) {
                    return Get_distance({prev_s, s});
                } else if(bus->bus_type_ == BusType::ORDINARY) {
                    return Get_distance({prev_s, s}) + Get_distance({s, prev_s});
                } else {
                    uint64_t c = 0;
                    return c;
                }
                
            }
        );

        if(bus->bus_type_ == BusType::ORDINARY) { 
            L_geo *= 2;
            R = R * 2 - 1; 
        } //у обычного маршрута расстояние умножается на два

        C = static_cast<long double>(L_road) / static_cast<long double>(L_geo);

        // запишем рассчитанную информацию в базу
        auto it = bus_infos__.insert(bus_infos__.end(), std::move(BusInfo{R, U, L_road, C}));
        busname_to_businfo__[name] = &*it;
        // возвращаем рассчитанный объект со статистикой
        return *it;
    } else {
        // несуществующий автобус
        return {};
    }
}    
} // namespace catalogue

} // namespace transport

