#include "request_handler.h"
#include "json_builder.h"

#include <string_view>
#include <stdexcept>

#include <iostream>


namespace request_handler{

using namespace std;
using namespace transport_catalogue;

namespace details {

json::Node GenerateStopResult(int id, const set<const Bus*>* buses) {
	// Узел для возврата
	json::Builder build_result{};

	build_result.StartDict();
	build_result.Key("request_id"s).Value(id);

	if (buses == nullptr) {
		build_result.Key("error_message"s).Value("not found"s);
	}
	else {
		set<string_view> bus_names;

		for (auto& bus : *buses) {
			bus_names.insert(bus->name_);
		}

		build_result.Key("buses"s).StartArray();

		for (auto& bus_name : bus_names) {
			build_result.Value(string(bus_name));
		}

		build_result.EndArray();
	}

	build_result.EndDict();

	return build_result.Build();
}

json::Node GenerateBusResult(int id, std::optional<BusStat> bus_info) {

	// Узел для возврата
	json::Builder result{};

	if (bus_info.has_value()) {
		result
			.StartDict()
			.Key("request_id"s).Value(id)
			.Key("curvature").Value(bus_info.value().curvature)
			.Key("route_length"s).Value(bus_info.value().route_length)
			.Key("stop_count"s).Value(bus_info.value().stop_count)
			.Key("unique_stop_count"s).Value(bus_info.value().unique_stop_count)
			.EndDict().Build();
	}
	else {
		result
			.StartDict()
			.Key("request_id"s).Value(id)
			.Key("error_message"s).Value("not found"s)
			.EndDict().Build();
	}

	return result.Build();
}

json::Node GenerateMapResult(int id, svg::Document map) {
	// Рендер карты в строку
	ostringstream strm;
	map.Render(strm);

	// Узел для возврата
	json::Builder result{};

	result
		.StartDict()
		.Key("request_id"s).Value(id)
		.Key("map"s).Value(strm.str())
		.EndDict();

	return result.Build();
}

json::Node GenerateRouteList(int id, const transport_router::RouteResponce& route_responce){
	json::Array items_list;

	for (const auto& item : route_responce.items) {
		if (item.type == "Wait"s) {
			json::Builder wait_node{};

			wait_node.StartDict()
				.Key("type"s).Value("Wait"s)
				.Key("stop_name"s).Value(item.stop_name)
				.Key("time"s).Value(route_responce.bus_wait_time)
				.EndDict();

			items_list.push_back(wait_node.Build());
		}
		else if (item.type == "Bus"s) {
			json::Builder bus_node{};

			bus_node.StartDict()
				.Key("type"s).Value("Bus"s)
				.Key("bus"s).Value(item.bus_name)
				.Key("span_count"s).Value(item.span_count)
				.Key("time"s).Value(item.time)
				.EndDict();

			items_list.push_back(bus_node.Build());
		}
		else {
			throw logic_error("Unknown rote list item type"s);
		}
	}

	json::Builder result{};
	result.StartDict()
		.Key("request_id"s).Value(id)
		.Key("total_time"s).Value(route_responce.total_time)
		.Key("items"s).Value(move(items_list))
		.EndDict();

	return result.Build();
}

json::Node GenerateRouteResult(int id, optional<transport_router::RouteResponce> route_result) {
	if (route_result.has_value()) {
		return GenerateRouteList(id, route_result.value());
	}
	else {
		// Узел для возврата
		json::Builder result{};

		result.StartDict()
			.Key("request_id"s).Value(id)
			.Key("error_message"s).Value("not found"s)
			.EndDict();

		return result.Build();
	}
}


} // End of details

// Возвращает маршруты, проходящие через остановку (запрос Stop)
const set<const Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
	return catalogue_.GetBusesToStop(stop_name);
}

// Возвращает информацию о маршруте (запрос Bus)
std::optional<BusStat> RequestHandler::GetBusInfo(const std::string_view& bus_name) const {
	return catalogue_.GetBusInfo(bus_name);
}

// Получение маршрута между остановками
optional<transport_router::RouteResponce> RequestHandler::GetRoute(std::string_view stop_from, std::string_view stop_to) const {
	// Получение свойст движения автобусов, для расчета времени маршрута
	const transport_router::RoutingSettings routing_settings = transport_router_.GetRoutingSettings();

	// Получение VertexId остановок
	graph::VertexId idx_stop_from = transport_router_.GetVertexId(stop_from);
	graph::VertexId idx_stop_to = transport_router_.GetVertexId(stop_to);

	// Возврат информации о кратчайшем маршруте из маршрутизатора
	optional<graph::Router<double>::RouteInfo> route_info = router_.BuildRoute(idx_stop_from, idx_stop_to);

	if (route_info == nullopt) {
		return nullopt;
	}

	transport_router::RouteResponce result;

	// Получение ссылки на список ребер маршрута
	const vector<graph::EdgeId>& edges_list = route_info.value().edges;

	double total_route_time = 0.0;

	if (edges_list.empty()) {
		result.total_time = 0.0;
		return result;
	}
	
	for (const auto& edge_id : edges_list) {
		// Возврат информации о текущем ребре
		transport_router::EdgeInfo edge_info = transport_router_.GetEdgeInfo(edge_id);

		// Создание узла ожидания автобуса
		transport_router::RouteElement wait_elem;
		wait_elem.stop_name = edge_info.stop_from;
		wait_elem.time = routing_settings.bus_wait_time;
		wait_elem.type = "Wait"s;
		result.items.push_back(move(wait_elem));

		// Создание узла поездки на автобусе
		transport_router::RouteElement ride_elem;
		ride_elem.time = edge_info.travel_time - routing_settings.bus_wait_time;
		ride_elem.type = "Bus"s;
		ride_elem.bus_name = edge_info.bus->name_;
		ride_elem.span_count = edge_info.span_count;
		result.items.push_back(move(ride_elem));

		total_route_time += edge_info.travel_time;
	}

	result.total_time = total_route_time;
	result.bus_wait_time = routing_settings.bus_wait_time;

	return result;
}

// Этот метод будет нужен в следующей части итогового проекта
svg::Document RequestHandler::RenderMap() const{
	// Получение списка маршрутов из справочника
	const auto bus_list = catalogue_.GetBusList();

	// Передача данных и генерация документа с картой
	svg::Document bus_map = renderer_.GenerateMap(bus_list);

	return bus_map;
}


// Обработка списка запросов
 json::Document RequestHandler::GetJsonResponce(const std::deque<RequestInfo>& request_list) {
	
	 // Создаем корневой массив для вывода
	 json::Array response_output;

	 response_output.reserve(request_list.size());


	for (auto& request : request_list) {
		string_view request_type = request.request_items.at("type"s);

		if (request_type == "Stop"sv){
			const auto buses_list = GetBusesByStop(request.request_items.at("name"s));
			json::Node stop_result = details::GenerateStopResult(request.id, buses_list);
			response_output.push_back(stop_result);
		}
		else if (request_type == "Bus"sv) {
			auto bus_info = GetBusInfo(request.request_items.at("name"s));
			json::Node bus_result = details::GenerateBusResult(request.id, bus_info);
			response_output.push_back(bus_result);
		}
		else if (request_type == "Route"sv) {
			optional<transport_router::RouteResponce> route_result = GetRoute(request.request_items.at("from"s), request.request_items.at("to"s));

			json::Node router_result = details::GenerateRouteResult(request.id, route_result);
			response_output.push_back(router_result);
		}
		else if (request_type == "Map"sv) {
			json::Node map_result = details::GenerateMapResult(request.id, RenderMap());
			response_output.push_back(map_result);
		}
		else {
			throw logic_error("unknown request type"s);
		}
	}

	return json::Document(json::Builder{}.Value(response_output).Build());
}

} // End of request_handler
