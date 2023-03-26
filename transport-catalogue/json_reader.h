#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace json_reader{

// Вход в функцию работы с json-файлами
void ReadJson(transport_catalogue::TransportCatalogue& catalog,
	map_renderer::MapRenderer& renderer,
	std::istream& input,
	std::ostream& output);

}