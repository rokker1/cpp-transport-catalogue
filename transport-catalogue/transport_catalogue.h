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

    void AddStop(string_view name, double lat, double lng);

    const Stop* FindStop(std::string_view name) const;

    const Bus* FindBus(std::string_view name) const;

    BusInfo GetBusInfo(string_view name);

    StopInfo GetStopInfo(string_view stop_name);

    void AddDistances(string_view name, const std::unordered_map<std::string, int>& distances);

    // debug
    // void PrintIntervals() const {
    //     for(const auto& e : intervals_to_distance__) {
    //         std::cout << "From: \"" << (e.first.first)->name_ << "\" to \"" <<  (e.first.second)->name_ << "\". = " << e.second << "m. " << std::endl;
    //     }
    // }
    
private:
    std::deque<Stop> all_stops_; //все остановки в базе данных
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_; //индексация остановок -> указатель на остановку

    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

    std::deque<BusInfo> bus_infos__;
    std::unordered_map<std::string_view, const BusInfo*> busname_to_businfo__;

    std::unordered_map<const Stop*, std::set<string_view>> stops_to_buses_; // v.1
    
    //контейнер длин между остановками
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, RouteDistanceHasher> intervals_to_distance__;
    
    uint64_t Get_distance(std::pair<const Stop*, const Stop*> p) const;
    
    BusInfo Compute_bus_info(string_view name);

    void Set_distance(std::pair<const Stop*, const Stop*> p, double d);
};        
} // namespace catalogue

} // namespace transport

