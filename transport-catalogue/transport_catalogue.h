#pragma once
#include <algorithm>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <utility>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <functional>
#include <set>
#include "geo.h"

namespace transport
{
namespace catalogue
{

using geo::Coordinates;
using geo::ComputeDistance;
using std::string_view;
using std::vector;
using std::string;

struct Stop {
    std::string name_;
    Coordinates cordinates_;
};

enum class BusType {
    ORDINARY,
    CYCLED,
    MAX_BUS_TYPES,
};
struct Bus {
    std::string name_;
    std::vector<const Stop*> stops_;
    BusType bus_type_;
};

struct BusInfo {
    unsigned int stops_count = 0;
    unsigned int unique_stops_count = 0;
    uint64_t length = 0;
    double curvative = 0;

    bool IsEmpty() const {
        return stops_count == 0 && unique_stops_count == 0 && length < 1e-6 && curvative == 0;
    }
};

struct StopInfo {
    std::set<string_view> buses_;
    bool IsExsists = false; 
};

struct RouteDistanceHasher {
    size_t operator()(std::pair<const Stop*, const Stop*> p) const {
        std::hash<const void*> ptr_hasher{}; // объект - хешер
        static const size_t PRIME = 13;
        return PRIME * ptr_hasher(p.first) + ptr_hasher(p.second);
    }
};

class Transport_catalogue {
public:
    
    void AddBus(string_view name, const vector<string>& stops, BusType type);

    void AddStop(string_view name, Coordinates coordinates);

    const Stop* FindStop(std::string_view name) const;

    const Bus* FindBus(std::string_view name) const;

    BusInfo GetBusInfo(string_view name);

    StopInfo GetStopInfo(string_view stop_name);

    void PrintIntervals() const;

    void SetDistance(std::pair<const Stop*, const Stop*> p, uint64_t distance);
    
private:
    //все остановки в базе данных
    std::deque<Stop> stops_; 
    //индексация остановок -> указатель на остановку
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_; 

    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

    std::deque<BusInfo> bus_infos_;
    std::unordered_map<std::string_view, const BusInfo*> busname_to_businfo_;

    std::unordered_map<const Stop*, std::set<string_view>> stops_to_buses_;
    
    //контейнер длин между остановками
    std::unordered_map<std::pair<const Stop*, const Stop*>, uint64_t, RouteDistanceHasher> intervals_to_distance_;
    
    uint64_t GetDistance(std::pair<const Stop*, const Stop*> p) const;
    
    BusInfo ComputeBusInfo(string_view name);

    
};        
} // namespace catalogue

} // namespace transport

