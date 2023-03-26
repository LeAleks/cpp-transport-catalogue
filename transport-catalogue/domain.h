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

#include "geo.h"


namespace transport_catalogue {

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

// Информация и маршруте (поиск)
struct BusStat {
	double curvature;
	int route_length;
	int stop_count;
	int unique_stop_count;
};

struct StopsDistance {
	int distance_;
	std::string_view stop1_name_;
	std::string_view stop2_name_;
};

} // End of transport_catalog