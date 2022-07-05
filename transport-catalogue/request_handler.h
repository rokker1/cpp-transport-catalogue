#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"

#include <optional>
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
using catalogue::TransportCatalogue;
using catalogue::BusStat;
using catalogue::StopInfo;
using BusPtr = Bus*;

class RequestHandler {
public:

    
    

    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, renderer::MapRenderer& renderer, graph::Router<BusRouteWeight>& router);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    
    // я художник я так вижу
    std::string RenderMapToString();


    // Возвращает информацию об остановке (запрос Stop)
    std::optional<StopInfo> GetStopInfo(const std::string_view& bus_name) const;

    std::optional<graph::Router<BusRouteWeight>::RouteInfo> GetRouteInfo(std::string_view stop_from, std::string_view stop_to) const;


private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;
    const graph::Router<BusRouteWeight>& router_;
};



