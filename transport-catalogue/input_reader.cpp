#include "input_reader.h"    

namespace transport
{
namespace input_reader
{
void Input_reader::AddInputQuery(std::istream& is) {
    std::string str;
    getline(is, str);

    size_t space_pos = str.find(" ");
    std::string_view operation = str.substr(0, space_pos);

    size_t query_begin = str.find_first_not_of(" ", space_pos);
    std::string query_text = str.substr(query_begin);
    if(operation == "Bus") {
        input_add_bus_queries_.emplace_back(std::move(ParseBusInputQuery(query_text)));
    } else if(operation == "Stop") {
        input_add_stop_queries_.emplace_back(std::move(ParseStopInput(query_text)));
    } else {
        return;
    }
}

const std::deque<AddStopQuery>& Input_reader::GetStopInputQueries() const {
    return input_add_stop_queries_;
}

const std::deque<AddBusQuery>& Input_reader::GetBusInputQueries() const {
    return input_add_bus_queries_;
}

AddBusQuery Input_reader::ParseBusInputQuery(std::string_view text) {
    AddBusQuery query;

    size_t semicol_pos = text.find(':');
    query.name_ = std::move(std::string(text.substr(0, text.find_last_not_of(" ", semicol_pos - 1) + 1))); 
    size_t value_begin = text.find_first_not_of(" ", semicol_pos + 1);
    text.remove_prefix(value_begin);
    BusType type = BusType::ORDINARY;
    size_t separator_pos = 0;
    char separator = '-';
    if(separator_pos = text.find(separator), separator_pos == text.npos) {
        separator = '>';
        type = BusType::CYCLED;
        separator_pos = text.find(separator);
    }
    query.type_ = type;

    while(separator_pos != text.npos) {
        size_t current_stop_end = text.find_last_not_of(" ", separator_pos - 1);
        std::string_view current_stop = text.substr(0, current_stop_end + 1);
        query.stops_.push_back(std::move(std::string(current_stop)));
        text.remove_prefix(text.find_first_not_of(" ", separator_pos + 1));
        separator_pos = text.find(separator);
    }
    std::string_view current_stop = text.substr(0, text.find_last_not_of(" ") + 1);
    query.stops_.push_back(std::move(std::string(current_stop)));

    return query;
}

AddStopQuery Input_reader::ParseStopInput(std::string_view text) {
    AddStopQuery query;
    size_t semicol_pos = text.find(':');
    
    query.name_ = std::move(std::string(text.substr(0, text.find_last_not_of(" ", semicol_pos - 1) + 1)));
    
    size_t lat_pos = text.find_first_not_of(" ", semicol_pos + 1);
    size_t comma_pos = text.find(',');
    size_t lng_pos = text.find_first_not_of(" ", comma_pos + 1);

    double lat = std::stod(std::string(text.substr(lat_pos, comma_pos)));
    double lng = std::stod(std::string(text.substr(lng_pos)));
    query.cordinates_.lat = lat;
    query.cordinates_.lng = lng;

    text.remove_prefix(std::min(text.size(), text.find(',', lng_pos)));

    while(!text.empty()) {
        auto [stop_name, distance_to] = ParseDistance(text);
        query.distances__.insert(std::move(std::pair<std::string, int>{std::string(stop_name), distance_to}));
    }

    return query;
}

std::tuple<std::string_view, int> Input_reader::ParseDistance(std::string_view& text) {
    text.remove_prefix(1);
    size_t next_sep_pos = text.find(',');
    int distance = std::stoi(std::string(text.substr(0, next_sep_pos)));

    size_t stop_name_begin_pos = text.find_first_not_of(" ", text.find("to") + 2);
    size_t stop_name_end_pos = text.find_last_not_of(" ", next_sep_pos - 1);
    std::string_view stop_name = text.substr(stop_name_begin_pos, stop_name_end_pos - stop_name_begin_pos + 1);
    
    text.remove_prefix(std::min(next_sep_pos, text.size()));
    
    return {stop_name, distance};
}    
void ReadInput(std::istream& in, transport::catalogue::Transport_catalogue& catalogue) {
 
    Input_reader reader;
    int query_count;
    std::string str;

    in >> query_count;
    getline(in, str);
    for(int i = 0; i < query_count; ++i) {
        reader.AddInputQuery(in);
    }

    auto stop_queries = reader.GetStopInputQueries();
    for(const auto& [name, coordinates, _] : stop_queries) {
        catalogue.AddStop(name, coordinates);
    }

    for(const auto& [name_from, _, distances] : stop_queries) {
        const Stop* stop_from = catalogue.FindStop(name_from);
        for(const auto& [name_to, distance] : distances) {
            const Stop* stop_to = catalogue.FindStop(name_to);
            catalogue.SetDistance({stop_from, stop_to}, distance);
        }
    }

    auto bus_queries = reader.GetBusInputQueries();
    for(const auto& [name, stops, type] : bus_queries) {
        catalogue.AddBus(name, stops, type);
    }
}   
} // namespace input_reader
} // namespace transport

