#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <string>
#include <deque>
#include <unordered_map>
#include <string_view>
#include <optional>

namespace transport_router {

struct RoutingSettings{
	int bus_wait_time;
	double bus_velocity;
};

struct RouteElement {
	std::string type;
	std::string stop_name;
	std::string bus_name;
	double time{};
	int span_count{};
};

struct RouteResponce {
	int id{};
	double total_time{};
	std::deque<RouteElement> items;
	double bus_wait_time{};
};

struct EdgeInfo {
	std::string bus_name;
	std::string start;
	std::string finish;
	int span_count = 0;
};


// Переделка класса для оптимизации алгоритма заполения графа


class TransportRouter {
public:
	TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, RoutingSettings& routing_settings);

	void SetRoutingSettings(RoutingSettings& settings);

	// Получение свойств движения автобусов
	RoutingSettings GetRoutingSettings() const;

	// Получение ссылки на граф
	const graph::DirectedWeightedGraph<double>& GetGraph();

	// Получение ссылки на свойства ребра по его EdgeId
	const EdgeInfo& GetEdgeInfo(graph::EdgeId id) const;

	// Получение VertexId по названию остановки
	graph::VertexId GetVertexId(std::string_view stop_name) const;

	// Получение времени перемещения между остановками
	double GetRunTime(graph::EdgeId id) const;

	// Метод возвращает значение длины ребра в double, вытаскивая его из 
	// шаблонной структуры RouteInfo
	double GetDistance(const graph::Router<double>::RouteInfo& info) const;

private:
	graph::DirectedWeightedGraph<double> graph_;
	const transport_catalogue::TransportCatalogue& catalogue_;
	RoutingSettings routing_settings_;

	std::unordered_map<std::string_view, graph::VertexId> stop_name_to_vertex_id_;
	std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_edge_info_;

	// Присвоение VertexId остановкам из каталога
	void SetStopVertexId();

	// Заполение графа маршрутами
	void SetRoutesToGraph();
};


} // End of transport_router