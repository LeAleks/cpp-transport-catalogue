#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#include <string>
#include <vector>
#include <string_view>
#include <deque>
#include <unordered_map>

#include "geo.h"


namespace transport_catalogue {

// Контейнер информации для одного зароса к каталогу
struct RequestInfo {
	int id;
	std::unordered_map<std::string, std::string> request_items;
};

// Запрос на добавление остановки
struct StopToAdd {
	std::string name;
	geo::Coordinates location;
	std::deque<std::pair<std::string, int>> distances_to_stops;
};

// Запрос на добавление автобуса
struct BusToAdd {
	bool is_circle;
	std::string name;
	std::deque<std::string> stops;
};


// Инфомарция об остановке
struct Stop {
	std::string name_;
	geo::Coordinates location_;
};

// Информация о маршруте
struct Bus {
	bool is_circle_ = false;
	std::string name_;
	std::vector<const Stop*> stops_;
};

struct StopsDistance {
	int distance_;
	std::string_view stop1_name_;
	std::string_view stop2_name_;
};



// Информация и маршруте (поиск)
struct BusStat {
	double curvature;
	int route_length;
	int stop_count;
	int unique_stop_count;
};

// Ответ на запрос по остановке
struct StopResponce{
	int id;
	bool was_found;
	std::vector<std::string_view> buses;
};


} // End of transport_catalog