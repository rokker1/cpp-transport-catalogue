#include "transport_catalogue.h"

namespace catalogue {
void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string>& stops, BusType type) {
    std::vector<const Stop*> stops_ptr;
    auto it = buses_.insert(buses_.end(), std::move(Bus{std::string(name), stops_ptr, type}));
    
    for_each(stops.begin(), stops.end(), [&stops_ptr, &it, this](std::string_view stop_name){
        if(const Stop* stop_ptr = FindStop(stop_name)) { // если есть такая остановка в базе
            it->stops_.push_back(std::move(stop_ptr));
            //добавление в stops_to_buses_
            stops_to_buses_[stop_ptr].insert(&*it);
        }
        
    });
    busname_to_bus_[std::string_view{it->name_}] = &(*it);
}

void TransportCatalogue::AddStop(std::string_view name, Coordinates coordinates) {
    auto it = stops_.emplace(stops_.end(), std::move(Stop{std::string(name), coordinates}));
    stopname_to_stop_[std::string_view{it->name_}] = &(*it);
}

const Stop* TransportCatalogue::FindStop(std::string_view name) const {
    if(stopname_to_stop_.count(name) == 0) {
        return nullptr;
    }
    return stopname_to_stop_.at(name);
}

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
    if(busname_to_bus_.count(name) == 0) {
        return nullptr;
    } 
    return busname_to_bus_.at(name);
}

BusStat TransportCatalogue::GetBusInfo(std::string_view name) const {
    if(busname_to_businfo_.count(name) == 0) {
        // статистики нет в базе
        return ComputeBusInfo(name);
    } 
    // статистика уже есть в базе
    return *(busname_to_businfo_.at(name));
}

StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
    if(stopname_to_stop_.count(stop_name) != 0) {
        // остановка существует
        const Stop* stop_ptr = FindStop(stop_name);
        if(stops_to_buses_.count(stop_ptr) != 0) {
            //остановка есть и через нее едут автобусы
            StopInfo stop_info{{}, true};

            // после ревью появился лишний цикл здесь
            std::for_each(stops_to_buses_.at(stop_ptr).begin(),
                            stops_to_buses_.at(stop_ptr).end(),
                            [&](auto bus){
                                stop_info.buses_.insert(bus->name_);
                            });
            return stop_info;
        } else {
            return StopInfo{{}, true}; // остановка есть, а автобусов нет
        }
    } else {
        return StopInfo{{}, false}; // остановка не существует
    }
}

void TransportCatalogue::SetDistance(std::pair<const Stop*, const Stop*> p, uint64_t distance) {
    intervals_to_distance_[p] = distance;
    // маршрут в обратную сторону еще не задавался
    // или задается расстояние к одной и той же остановке
    if(intervals_to_distance_.count({p.second, p.first}) == 0
        || p.first == p.second) { 
        // значит, что длина маршрута в обе стороны равна
        intervals_to_distance_[{p.second, p.first}] = distance;
    }
}

uint64_t TransportCatalogue::GetDistance(std::pair<const Stop*, const Stop*> p) const {
    if(intervals_to_distance_.count(p)) {
        return intervals_to_distance_.at(p);
    } else {
        return 0u;
    }
}

BusStat TransportCatalogue::ComputeBusInfo(std::string_view name) const {
    const Bus* bus{nullptr}; //указатель на автобус в деке
    unsigned int stops_count = 0, unique_stops_count = 0;
    double length_geo = 0;
    uint64_t length_road = 0;
    double curvature = 0.0;
    if(bus = FindBus(name), bus) {
        // такой автобус существует
        //bus = FindBus(name);
        stops_count = bus->stops_.size();

        std::unordered_set<std::string_view, std::hash<std::string_view>, std::equal_to<std::string_view>> unique_stops;
        for_each(bus->stops_.begin(), bus->stops_.end(), [&unique_stops](const Stop* stop){
            //std::cout << "Debug. \"" << stop->name_ << "\"." << std::endl;
            unique_stops.insert(std::string_view(stop->name_));
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


        // вариант с хранением статистики в базе каталога
        // запишем рассчитанную информацию в базу
        // auto it = bus_infos_.insert(bus_infos_.end(), std::move(BusStat{stops_count, unique_stops_count, length_road, curvature}));
        //busname_to_businfo_[name] = &*it;
        // возвращаем рассчитанный объект со статистикой
        // return *it;

        // вариант с возвратом промежуточной информации
        return BusStat{stops_count, unique_stops_count, length_road, curvature, true};
    } else {
        // несуществующий автобус
        return BusStat{};
    }
}  

void TransportCatalogue::PrintIntervals() const {
    for(const auto& e : intervals_to_distance_) {
        std::cout << "From: \"" << (e.first.first)->name_ << "\" to \"" <<  (e.first.second)->name_ << "\". = " << e.second << "m. " << std::endl;
    }
}  

std::deque<const Bus*> TransportCatalogue::GetBusesSorted() const {
    std::deque<const Bus*> result;
    for(const auto& [busname, bus] : busname_to_bus_) {
        result.push_back(bus);
    }
    return result;
}

const std::unordered_map<const Stop*, std::set<const Bus*>>& TransportCatalogue::GetStopsToBuses() const {
    return stops_to_buses_;
}

const std::map<std::string_view, const Stop*>& TransportCatalogue::GetStopnameToStops() const {
    return stopname_to_stop_;
}

TransportCatalogue::TransportCatalogue(RoutingSettings routing_settings)
    : routing_settings_(routing_settings) {
}
void TransportCatalogue::SetRoutingSettings(RoutingSettings routing_settings) {
    routing_settings_ = std::move(routing_settings);
}

void TransportCatalogue::AddStopVertex(const Stop* stop) {
    vertex_index_to_stop_.push_back(stop);
    vertex_index_to_stop_.push_back(stop);
    //Сохраняем только четные ид - приемные остановки, 
    //"исхожящие" вершины можно получить прибавив единицу к ид.
    stopname_to_vertex_id_.emplace(stop->name_, vertex_index_to_stop_.size() - 2);
}

void TransportCatalogue::AddBusWaitEdges() {
    //edge_index_to_bus_.resize(std::pow(stopname_to_stop_.size(), 3));
    //вызов конструктора графа для известного на данный момент числа вершин графа
    route_graph_ = std::move(graph::DirectedWeightedGraph<BusRouteWeight>(vertex_index_to_stop_.size()));
    for(graph::VertexId vertex_from_id = 0; vertex_from_id < vertex_index_to_stop_.size(); vertex_from_id += 2) {
        route_graph_.AddEdge({vertex_from_id, vertex_from_id + 1, routing_settings_.bus_wait_time});
        edge_index_to_bus_.push_back(nullptr);
    }
}

void TransportCatalogue::AddBusEdges(std::string_view name) {
    if(busname_to_bus_.count(name) == 0) {
        throw std::logic_error("No such bus");
    }
    const Bus* bus = busname_to_bus_.at(name);
    const std::vector<const Stop*>& stops = bus->stops_;

    if(bus->bus_type_ == BusType::CYCLED) {
        double current_distance = 0.0;
        int span_count = 0;
        for(auto it_from = stops.begin(); it_from != std::prev(stops.end()); ++it_from) {
            for(auto it_to = std::next(it_from); it_to != stops.end(); ++it_to) {
                current_distance = current_distance + GetDistance({*(std::prev(it_to)), *it_to});
                ++span_count;
                route_graph_.AddEdge({
                    GetStopVertexIndex((*it_from)->name_) + 1,
                    GetStopVertexIndex((*it_to)->name_),
                    {
                        current_distance / routing_settings_.bus_velocity,
                        span_count
                    }
                });
                edge_index_to_bus_.push_back(bus);
            }
            current_distance = 0.0;
            span_count = 0;
        }
    } else {
        double current_distance = 0.0;
        int span_count = 0;
        for(auto it_from = stops.begin(); it_from != std::prev(stops.end()); ++it_from) {
            for(auto it_to = std::next(it_from); it_to != stops.end(); ++it_to) {
                current_distance = current_distance + GetDistance({*(std::prev(it_to)), *it_to});
                ++span_count;
                route_graph_.AddEdge({
                    GetStopVertexIndex((*it_from)->name_) + 1,
                    GetStopVertexIndex((*it_to)->name_),
                    {
                        current_distance / routing_settings_.bus_velocity,
                        span_count
                    }
                });
                edge_index_to_bus_.push_back(bus);
            }
            current_distance = 0.0;
            span_count = 0;
        }
        
        for(auto it_from = stops.rbegin(); it_from != std::prev(stops.rend()); ++it_from) {
            for(auto it_to = std::next(it_from); it_to != stops.rend(); ++it_to) {
                current_distance = current_distance + GetDistance({*(std::prev(it_to)), *it_to});
                ++span_count;
                route_graph_.AddEdge({
                    GetStopVertexIndex((*it_from)->name_) + 1,
                    GetStopVertexIndex((*it_to)->name_),
                    {
                        current_distance / routing_settings_.bus_velocity,
                        span_count
                    }
                });
                edge_index_to_bus_.push_back(bus);
            }
            current_distance = 0.0;
            span_count = 0;
        }
    }
}


graph::VertexId TransportCatalogue::GetStopVertexIndex(std::string_view stop_name) const {
    if(stopname_to_vertex_id_.count(stop_name) == 0) {
        throw std::logic_error("Invalid stop name - can't find VertexId");
    }
    return stopname_to_vertex_id_.at(stop_name);
}

const Bus* TransportCatalogue::GetBusByEdgeIndex(graph::EdgeId edge_id) const {
    if(edge_id < edge_index_to_bus_.size()) {
        return edge_index_to_bus_[edge_id];
    } else {
        throw std::logic_error("Bad EdgeId requested!");
    }
}

const graph::Edge<BusRouteWeight>& TransportCatalogue::GetEdgeByIndex(graph::EdgeId edge_id) const {
    return route_graph_.GetEdge(edge_id);
}

int TransportCatalogue::ComputeRouteSpanBetweenGraphVertices(
                    const Bus* bus
                    , graph::VertexId vertex_from
                    , graph::VertexId vertex_to) const
{
    const Stop* stop_from = vertex_index_to_stop_[vertex_from];
    const Stop* stop_to = vertex_index_to_stop_[vertex_to];

    // вычислить расстояние между двумя указателями в объекте Bus
    auto it_from = std::find(bus->stops_.begin(), bus->stops_.end(), stop_from);
    auto it_to = std::find(it_from, bus->stops_.end(), stop_to);
    // неверный подход
    return std::distance(it_from, it_to); // заглушка
}

const Stop* TransportCatalogue::GetStopByVertexIndex(graph::VertexId vertex_id) const {
    if (vertex_id >= vertex_index_to_stop_.size()) {
        throw std::logic_error("Bad vartex id is passed, too big");
    }
    return vertex_index_to_stop_[vertex_id];
}

} // namespace catalogue


