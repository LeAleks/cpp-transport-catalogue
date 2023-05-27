#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include "serialization.h"

#include <deque>
#include <string>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace json_reader {

struct InputQueries{
	std::deque<transport_catalogue::StopToAdd> stops_to_add;
	std::deque<transport_catalogue::BusToAdd> buses_to_add;
	std::deque<transport_catalogue::RequestInfo> requests;
	map_renderer::RenderSettings render_settings;
	transport_router::RoutingSettings routing_settings;
	// serialization_settings
	serialization::Settings ser_settings;
};

// Чтение json для транспортного каталога
InputQueries ReadTransportJson(std::istream& input);

}