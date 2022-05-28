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
    std::deque<AddStopQuery> input__add_stop_queries__;
    std::deque<AddBusQuery> input__add_bus_queries__;

public:
    void Add_input_query(std::istream& is);

    const std::deque<AddStopQuery>& Get_add_stop_queries() const;

    const std::deque<AddBusQuery>& Get_add_bus_queries() const;

private:
    AddBusQuery Parse_add_bus_query(string_view text);

    AddStopQuery Parse_Add_Stop_Query(string_view text);

    std::tuple<string_view, int> Parse_distance(string_view& text);
};        
} // namespace input_reader
} // namespace transport

