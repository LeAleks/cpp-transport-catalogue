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
	const transport_catalogue::Bus* bus;
	int span_count{};
	int distance{};
	double travel_time{};
	std::string_view stop_from;
};

class TransportRouter {
public:
	TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, RoutingSettings& routing_settings);

	void SetRoutingSettings(RoutingSettings& settings);

	// Загрузка информации из базы и заполение графа
	void FillGraph();

	// Получение ссылки на граф
	const graph::DirectedWeightedGraph<double>& GetGraph();

	// Получение ссылки на свойства ребра по его EdgeId
	const EdgeInfo& GetEdgeInfo(graph::EdgeId id) const;

	// Получение VertexId по названию остановки
	graph::VertexId GetVertexId(std::string_view stop_name) const;

	// Получение свойств движения автобусов
	RoutingSettings GetRoutingSettings() const;

private:
	graph::DirectedWeightedGraph<double> graph_;
	const transport_catalogue::TransportCatalogue& catalogue_;
	RoutingSettings routing_settings_;

	// Список остановок с присвоенным VertexId для каждой
	std::unordered_map<std::string_view, graph::VertexId> stops_vertex_id_;
	std::unordered_map<graph::EdgeId, EdgeInfo> edgeID_to_edge_info_;

	// Хеш функция для работы с парой VertexId
	struct HasherVertexId {
		size_t operator()(const std::pair<graph::VertexId, graph::VertexId>& stop_names) const {
			return size_t(stop_names.first) + 211 * size_t(stop_names.second);
		}
	};

	std::unordered_map<std::pair<graph::VertexId, graph::VertexId>, int, HasherVertexId> pair_idx_to_distance_;
};


} // End of transport_router