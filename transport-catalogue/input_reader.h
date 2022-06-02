#pragma once
#include <algorithm>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <utility>
#include <iostream>
#include <tuple>
#include <iomanip>
#include "transport_catalogue.h"
#include "geo.h"

namespace transport
{
namespace input_reader
{
using namespace catalogue;
using std::string_view;
using std::vector;
using std::string;
using geo::Coordinates;

enum class QueryType
{
    NewStop,
    NewBus,
    MAX_QUERY_TYPES,
};

struct AddStopQuery {
    std::string name_;
    Coordinates cordinates_;
    std::unordered_map<string, int> distances__;
};

struct AddBusQuery {
    std::string name_;
    std::vector<string> stops_;
    BusType type_;
};

class Input_reader {
    std::deque<AddStopQuery> input_add_stop_queries_;
    std::deque<AddBusQuery> input_add_bus_queries_;

public:
    void AddInputQuery(std::istream& is);

    const std::deque<AddStopQuery>& GetStopInputQueries() const;

    const std::deque<AddBusQuery>& GetBusInputQueries() const;

private:
    AddBusQuery ParseBusInputQuery(string_view text);

    AddStopQuery ParseStopInput(string_view text);

    std::tuple<string_view, int> ParseDistance(string_view text);
};        
} // namespace input_reader
} // namespace transport

