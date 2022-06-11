#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    BusStat stat = db_.GetBusInfo(bus_name);
    if(!stat.IsExsists) {
        return std::nullopt;
    } else {
        return stat;
    }
}

std::optional<StopInfo> RequestHandler::GetStopInfo(const std::string_view& stop_name) const {
    StopInfo info = db_.GetStopInfo(stop_name);
    if(info.IsExsists) {
        return info;
    } else {
        return std::nullopt;
    }
}


RequestHandler::RequestHandler(const TransportCatalogue& db, renderer::MapRenderer& renderer)
    : db_(db), renderer_(renderer) 
{
}

std::string RequestHandler::RenderMapToString() {
    std::deque<const Bus*> buses = db_.GetBusesSorted();
    renderer_.RenderRoutes(buses);

    const auto stops_to_buses = db_.GetStopsToBuses();
    const auto stopname_to_stops = db_.GetStopnameToStops();
    renderer_.RenderStops(stopname_to_stops, stops_to_buses);

    std::ostringstream out;
    renderer_.Render({out, 0, 0});

    return out.str();
}