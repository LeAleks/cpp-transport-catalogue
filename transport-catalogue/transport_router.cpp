#include "transport_router.h"

#include <iostream>

#include <algorithm>

namespace transport_router {

using namespace std;

TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, RoutingSettings& routing_settings)
	: graph_(catalogue.GetStopList().size() + 1)
	, catalogue_(catalogue)
	, routing_settings_(routing_settings) {
	SetStopVertexId();
	SetRoutesToGraph();
}

void TransportRouter::SetRoutingSettings(RoutingSettings& settings) {
	routing_settings_ = settings;
}

// Получение свойств движения автобусов
RoutingSettings TransportRouter::GetRoutingSettings() const {
	return routing_settings_;
}

// Получение ссылки на граф
const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() {
	return graph_;
}

// Получение ссылки на свойства ребра по его EdgeId
const EdgeInfo& TransportRouter::GetEdgeInfo(graph::EdgeId id) const {
	return edge_id_to_edge_info_.at(id);
}

// Получение VertexId по названию остановки
graph::VertexId TransportRouter::GetVertexId(string_view stop_name) const {
	return stop_name_to_vertex_id_.at(stop_name);
}

// Получение времени перемещения между остановками
double TransportRouter::GetRunTime(graph::EdgeId id) const {
	return graph_.GetEdge(id).weight - routing_settings_.bus_wait_time;
}

double TransportRouter::GetDistance(const graph::Router<double>::RouteInfo& info) const {
	return info.weight;
}

void TransportRouter::SetStopVertexId() {
	// Проход по списку остановок в каталоге
	for (const auto& stop : catalogue_.GetStopList()) {
		// Проверка, что остановка еще не добавлена
		if (stop_name_to_vertex_id_.count(stop->name_) == 0) {
			// Если не добавлена, то создается пара имя - VertexID
			// VertexId = размеру списка до добавления остановки
			stop_name_to_vertex_id_[stop->name_] = stop_name_to_vertex_id_.size();
		}
	}
}

void TransportRouter::SetRoutesToGraph() {
	for (const auto& bus : catalogue_.GetBusList()) {
		// Нужно создать список остановок, в завивисимости от типа маршрута
		// Просто список - для кольцевого, удвоенный - для некольцевого
		// По этому списку и проходить при построении графа

		auto stop_list = bus->stops_;

		if (!bus->is_circle_) {
			stop_list.resize(bus->stops_.size() * 2 - 1);
			reverse(stop_list.begin(), stop_list.end());
			copy(bus->stops_.begin(), bus->stops_.end(), stop_list.begin());
		}

		// Хранилища названий остановок
		string second_stop, prev_stop;

		// Проход по списку остановок
		// Внешний цикл задает первую остановку для ребра. Внутренний - конечную

		// Проход по списку остановок
		// Внешний цикл задает первую остановку для ребра. Внутренний - конечную
		for (int start = 0; start < stop_list.size(); ++start) {
			double road_time_min = 0;
			int span_count = 0;
			for (int end = start + 1; end < stop_list.size(); ++end) {
				second_stop = stop_list[end]->name_;
				prev_stop = stop_list[end - 1]->name_;

				double distance = catalogue_.GetDistance(prev_stop, second_stop);
				double speed_m_per_min = routing_settings_.bus_velocity * 1000.0 / 60.0;
				road_time_min += distance / speed_m_per_min;

				graph::Edge<double> edge_to_add;
				edge_to_add.from = stop_name_to_vertex_id_[stop_list[start]->name_];
				edge_to_add.to = stop_name_to_vertex_id_[second_stop];
				edge_to_add.weight = road_time_min + routing_settings_.bus_wait_time;

				graph::EdgeId edge_id = graph_.AddEdge(std::move(edge_to_add));

				span_count++;
				edge_id_to_edge_info_[edge_id] = { bus->name_, stop_list[start]->name_, second_stop, span_count };
			}
		}
	}
}

} // End of transport_router