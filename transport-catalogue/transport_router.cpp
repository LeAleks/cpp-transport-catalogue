#include "transport_router.h"

#include <iostream>

namespace transport_router {

using namespace std;

TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, RoutingSettings& routing_settings)
	: graph_(catalogue.GetStopList().size() + 1)
	, catalogue_(catalogue)
	, routing_settings_(routing_settings) {}

void TransportRouter::SetRoutingSettings(RoutingSettings& settings) {
	routing_settings_ = settings;
}

void TransportRouter::FillGraph(){
	graph::VertexId current_id = 0;
	for (auto stop : catalogue_.GetStopList()) {
		stops_vertex_id_[stop->name_] = current_id;
		++current_id;
	}

	// Буффер для хранения и заполения свойств ребер перед переносом их в граф
	unordered_map<pair<graph::VertexId, graph::VertexId>, EdgeInfo, HasherVertexId> tmp_edges_to_vertexes;

	// Буфферы для хранения VertexId промежуточных остановок
	graph::VertexId idx_initial_stop = 0;
	graph::VertexId idx_prev_stop = 0;
	graph::VertexId idx_current_stop = 0;

	// Проход по списку маршрутов для построения графа каждого маршрута
	for (auto bus : catalogue_.GetBusList()) {


		for (auto it_outer = bus->stops_.begin(); it_outer + 1 != bus->stops_.end(); ++it_outer) {
			int span_count = 0;

			// Получение VertexId для "начальной" остановки маршрута
			string_view initial_stop_name = (*it_outer)->name_;
			idx_initial_stop = stops_vertex_id_.at(initial_stop_name);

			const int zero_stop_distance = 0;

			// Запись нулевого расстояния для одной и той же остановки
			pair_idx_to_distance_[{ idx_initial_stop, idx_initial_stop }] = zero_stop_distance;

			for (auto it_inner = it_outer + 1; it_inner != bus->stops_.end(); ++it_inner) {
				++span_count;

				string_view prev_stop_name = (*next(it_inner, -1))->name_;
				string_view current_stop_name = (*it_inner)->name_;

				idx_prev_stop = stops_vertex_id_.at(prev_stop_name);
				idx_current_stop = stops_vertex_id_.at(current_stop_name);

				int distance_prev_to_curr = catalogue_.GetDistance(prev_stop_name, current_stop_name);

				int distance_initial_to_curr = pair_idx_to_distance_.at({ idx_initial_stop, idx_prev_stop })
					+ distance_prev_to_curr;
				pair_idx_to_distance_[{ idx_initial_stop, idx_current_stop }] = distance_initial_to_curr;

				EdgeInfo edge_info;

				edge_info.bus = bus;
				edge_info.distance = distance_initial_to_curr;
				edge_info.span_count = span_count;
				edge_info.travel_time = 0.0;
				edge_info.stop_from = initial_stop_name;

				auto it_find = tmp_edges_to_vertexes.find({ idx_initial_stop, idx_current_stop });

				if (it_find == tmp_edges_to_vertexes.end()) {
					tmp_edges_to_vertexes[{ idx_initial_stop, idx_current_stop }] = edge_info;
				}
				else if (it_find->second.distance > distance_initial_to_curr) {
					tmp_edges_to_vertexes[{ idx_initial_stop, idx_current_stop }] = edge_info;
				}

				if (!bus->is_circle_) {
					int distance_current_prev = catalogue_.GetDistance(current_stop_name, prev_stop_name);

					int distance_current_to_initial = pair_idx_to_distance_.at({ idx_prev_stop, idx_initial_stop })
						+ distance_current_prev;

					pair_idx_to_distance_[{ idx_current_stop, idx_initial_stop }] = distance_current_to_initial;

					EdgeInfo edge_info_rev(edge_info);

					edge_info_rev.distance = distance_current_to_initial;
					edge_info_rev.stop_from = current_stop_name;

					auto it_find_rev = tmp_edges_to_vertexes.find({ idx_current_stop, idx_initial_stop });

					if (it_find_rev == tmp_edges_to_vertexes.end()) {
						tmp_edges_to_vertexes[{ idx_current_stop, idx_initial_stop }] = edge_info_rev;
					}
					else if (it_find_rev->second.distance > distance_current_to_initial) {
						tmp_edges_to_vertexes[{ idx_current_stop, idx_initial_stop }] = edge_info_rev;
					}
				}
			}
		}

		for (const auto& [stops_indexes, props] : tmp_edges_to_vertexes) {

			double bus_velocity_meters_per_minute = routing_settings_.bus_velocity * 1000 / 60;

			double travel_time = (double(props.distance) / bus_velocity_meters_per_minute) + double(routing_settings_.bus_wait_time);

			graph::EdgeId id = graph_.AddEdge({ stops_indexes.first, stops_indexes.second, travel_time });

			EdgeInfo edge_prop(props);
			edge_prop.travel_time = travel_time;

			edgeID_to_edge_info_.emplace(id, edge_prop);
		}
	}
}

// Получение ссылки на граф
const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() {
	return graph_;
}

// Получение ссылки на свойства ребра по его EdgeId
const EdgeInfo& TransportRouter::GetEdgeInfo(graph::EdgeId id) const {
	return edgeID_to_edge_info_.at(id);
}

// Получение VertexId по названию остановки
graph::VertexId TransportRouter::GetVertexId(string_view stop_name) const{
	return stops_vertex_id_.at(stop_name);
}

// Получение свойств движения автобусов
RoutingSettings TransportRouter::GetRoutingSettings() const {
	return routing_settings_;
}

} // End of transport_router