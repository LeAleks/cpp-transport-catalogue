#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "transport_router.h"

#include <deque>
#include <optional>
#include <set>

namespace request_handler{

 class RequestHandler {
 public:
     RequestHandler(
         transport_catalogue::TransportCatalogue& catalogue,
         map_renderer::MapRenderer& renderer,
         transport_router::TransportRouter& transport_router,
         graph::Router<double>& router
     )
         : catalogue_(catalogue)
         , renderer_(renderer)
         , transport_router_(transport_router)
         , router_(router) {}

     // Возвращает информацию о маршруте (запрос Bus)
     std::optional<transport_catalogue::BusStat> GetBusInfo(const std::string_view& bus_name) const;

     // Возвращает маршруты, проходящие через остановку (запрос Stop)
     const std::set<const transport_catalogue::Bus*>* GetBusesByStop(const std::string_view& stop_name) const;

     // Этот метод будет нужен в следующей части итогового проекта
     svg::Document RenderMap() const;

     // Получение маршрута между остановками
     std::optional<transport_router::RouteResponce> GetRoute(std::string_view stop_from, std::string_view stop_to) const;

     // Обработка списка запросов
     json::Document GetJsonResponce(const std::deque<transport_catalogue::RequestInfo>& request_list);

 private:
     // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
     const transport_catalogue::TransportCatalogue& catalogue_;
     const map_renderer::MapRenderer& renderer_;
     const transport_router::TransportRouter& transport_router_;
     const graph::Router<double> router_;
 };


 } // End of request_handler
